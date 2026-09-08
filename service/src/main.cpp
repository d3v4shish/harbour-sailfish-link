#include "linkcommon.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonValue>
#include <QNetworkAddressEntry>
#include <QNetworkInterface>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>

#include <algorithm>

namespace {

struct LanAddress
{
    QString interfaceName;
    QString ip;
    QHostAddress broadcast;
    QNetworkInterface networkInterface;
    int score;
};

QJsonObject objectValue(const QJsonObject &parent, const QString &key)
{
    const QJsonValue value = parent.value(key);
    return value.isObject() ? value.toObject() : QJsonObject();
}

int boundedPort(const QJsonObject &object, const QString &key, int fallback)
{
    const int port = object.value(key).toInt(fallback);
    if (port < 1 || port > 65535)
        return fallback;
    return port;
}

int boundedIntervalSeconds(const QJsonObject &discovery)
{
    const int seconds = discovery.value(QStringLiteral("announcement_interval_seconds")).toInt(15);
    if (seconds < 5)
        return 5;
    if (seconds > 300)
        return 300;
    return seconds;
}

bool isPrivateIpv4(const QHostAddress &address)
{
    if (address.protocol() != QAbstractSocket::IPv4Protocol)
        return false;

    const quint32 ip = address.toIPv4Address();
    if ((ip & 0xff000000U) == 0x0a000000U)
        return true;
    if ((ip & 0xfff00000U) == 0xac100000U)
        return true;
    if ((ip & 0xffff0000U) == 0xc0a80000U)
        return true;
    return false;
}

bool isMulticastIpv4(const QHostAddress &address)
{
    if (address.protocol() != QAbstractSocket::IPv4Protocol)
        return false;

    return (address.toIPv4Address() & 0xf0000000U) == 0xe0000000U;
}

int interfaceScore(const QString &name)
{
    const QString lower = name.toLower();
    if (lower.startsWith(QStringLiteral("wlan")) ||
            lower.startsWith(QStringLiteral("wlp")) ||
            lower.contains(QStringLiteral("wifi"))) {
        return 0;
    }
    if (lower.startsWith(QStringLiteral("eth")) ||
            lower.startsWith(QStringLiteral("en"))) {
        return 1;
    }
    return 2;
}

LanAddress selectLanAddress(QString *error)
{
    QList<LanAddress> addresses;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (int i = 0; i < interfaces.size(); ++i) {
        const QNetworkInterface iface = interfaces.at(i);
        const QNetworkInterface::InterfaceFlags flags = iface.flags();
        if (!(flags & QNetworkInterface::IsUp) ||
                !(flags & QNetworkInterface::IsRunning) ||
                (flags & QNetworkInterface::IsLoopBack)) {
            continue;
        }

        const QList<QNetworkAddressEntry> entries = iface.addressEntries();
        for (int j = 0; j < entries.size(); ++j) {
            const QHostAddress address = entries.at(j).ip();
            if (!isPrivateIpv4(address))
                continue;

            LanAddress lan;
            lan.interfaceName = iface.name();
            lan.ip = address.toString();
            lan.broadcast = entries.at(j).broadcast();
            lan.networkInterface = iface;
            lan.score = interfaceScore(iface.name());
            addresses.append(lan);
        }
    }

    if (addresses.isEmpty()) {
        if (error)
            *error = QStringLiteral("No active RFC1918 IPv4 LAN address found.");
        return LanAddress();
    }

    std::sort(addresses.begin(), addresses.end(), [](const LanAddress &left, const LanAddress &right) {
        if (left.score != right.score)
            return left.score < right.score;
        if (left.interfaceName != right.interfaceName)
            return left.interfaceName < right.interfaceName;
        return left.ip < right.ip;
    });

    return addresses.first();
}

QString endpointUrl(const QString &ip, const QJsonObject &config, const QString &sectionName,
                    const QString &pathName, int defaultPort, const QString &defaultPath)
{
    const QJsonObject section = objectValue(config, sectionName);
    QString path = section.value(pathName).toString(defaultPath);
    if (!path.startsWith(QLatin1Char('/')))
        path.prepend(QLatin1Char('/'));
    return QStringLiteral("http://%1:%2%3").arg(ip).arg(boundedPort(section, QStringLiteral("port"), defaultPort)).arg(path);
}

QJsonObject buildPayload(const QJsonObject &config, const LanAddress &lan)
{
    const QString now = LinkCommon::utcNow();
    const QJsonObject device = objectValue(config, QStringLiteral("device"));
    const QJsonObject sshConfig = objectValue(config, QStringLiteral("ssh"));
    const QJsonObject auth = objectValue(config, QStringLiteral("auth"));

    QJsonObject devicePayload;
    devicePayload.insert(QStringLiteral("id"), device.value(QStringLiteral("id")).toString());
    devicePayload.insert(QStringLiteral("name"), device.value(QStringLiteral("name")).toString());

    QJsonObject networkPayload;
    networkPayload.insert(QStringLiteral("ip"), lan.ip);

    QJsonObject sshPayload;
    sshPayload.insert(QStringLiteral("host"), lan.ip);
    sshPayload.insert(QStringLiteral("port"), boundedPort(sshConfig, QStringLiteral("port"), 22));
    sshPayload.insert(QStringLiteral("user"), sshConfig.value(QStringLiteral("user")).toString(LinkCommon::defaultSshUser()));

    QJsonObject webcamPayload;
    webcamPayload.insert(QStringLiteral("mjpeg_url"),
                         endpointUrl(lan.ip, config, QStringLiteral("webcam"), QStringLiteral("mjpeg_path"),
                                     8090, QStringLiteral("/stream.mjpeg")));
    webcamPayload.insert(QStringLiteral("status_url"),
                         endpointUrl(lan.ip, config, QStringLiteral("webcam"), QStringLiteral("status_path"),
                                     8090, QStringLiteral("/status.json")));

    QJsonObject llsPayload;
    llsPayload.insert(QStringLiteral("control_url"),
                      endpointUrl(lan.ip, config, QStringLiteral("lls"), QStringLiteral("control_path"),
                                  8091, QStringLiteral("/control")));
    llsPayload.insert(QStringLiteral("status_url"),
                      endpointUrl(lan.ip, config, QStringLiteral("lls"), QStringLiteral("status_path"),
                                  8091, QStringLiteral("/status")));

    QJsonObject authPayload;
    authPayload.insert(QStringLiteral("token_hint"),
                       LinkCommon::tokenHint(auth.value(QStringLiteral("pairing_token")).toString()));

    QJsonObject payload;
    payload.insert(QStringLiteral("version"), 1);
    payload.insert(QStringLiteral("device"), devicePayload);
    payload.insert(QStringLiteral("announced_at"), now);
    payload.insert(QStringLiteral("network"), networkPayload);
    payload.insert(QStringLiteral("ssh"), sshPayload);
    payload.insert(QStringLiteral("webcam"), webcamPayload);
    payload.insert(QStringLiteral("lls"), llsPayload);
    payload.insert(QStringLiteral("auth"), authPayload);
    return payload;
}

void writeStatus(bool enabled,
                 bool eligible,
                 const QString &state,
                 const LanAddress &lan,
                 int announcementPort,
                 const QString &lastError,
                 const QJsonObject &payload)
{
    const QJsonObject previous = LinkCommon::readJson(LinkCommon::statusPath());
    const QString now = LinkCommon::utcNow();

    QJsonObject status;
    status.insert(QStringLiteral("status_version"), 1);
    status.insert(QStringLiteral("updated_at"), now);
    status.insert(QStringLiteral("enabled"), enabled);
    status.insert(QStringLiteral("eligible"), eligible);
    status.insert(QStringLiteral("state"), state);
    status.insert(QStringLiteral("interface"), lan.interfaceName);
    status.insert(QStringLiteral("ip"), lan.ip);
    status.insert(QStringLiteral("announcement_port"), announcementPort);
    status.insert(QStringLiteral("last_error"), lastError);

    if (state == QStringLiteral("announced")) {
        status.insert(QStringLiteral("last_announce_at"), now);
        status.insert(QStringLiteral("last_announced_ip"), lan.ip);
        status.insert(QStringLiteral("payload"), payload);
    } else {
        status.insert(QStringLiteral("last_announce_at"),
                      previous.value(QStringLiteral("last_announce_at")).toString());
        status.insert(QStringLiteral("last_announced_ip"),
                      previous.value(QStringLiteral("last_announced_ip")).toString());
    }

    QString error;
    if (!LinkCommon::writeJson(LinkCommon::statusPath(), status, &error)) {
        LinkCommon::appendLog(QStringLiteral("ERROR"), error);
        return;
    }

    const QString oldKey = previous.value(QStringLiteral("state")).toString() +
            QLatin1Char('|') + previous.value(QStringLiteral("ip")).toString() +
            QLatin1Char('|') + previous.value(QStringLiteral("last_error")).toString();
    const QString newKey = state + QLatin1Char('|') + lan.ip + QLatin1Char('|') + lastError;
    if (oldKey != newKey) {
        QString message = QStringLiteral("state=%1 ip=%2 interface=%3").arg(state, lan.ip, lan.interfaceName);
        if (!lastError.isEmpty())
            message.append(QStringLiteral(" error=%1").arg(lastError));
        const QString level = state == QStringLiteral("error") ? QStringLiteral("ERROR") :
                (lastError.isEmpty() ? QStringLiteral("INFO") : QStringLiteral("WARN"));
        LinkCommon::appendLog(level, message);
    }
}

bool sendDatagram(QUdpSocket *socket, const QByteArray &datagram, const QHostAddress &address, quint16 port, QString *error)
{
    const qint64 sent = socket->writeDatagram(datagram, address, port);
    if (sent == datagram.size())
        return true;

    if (error && error->isEmpty())
        *error = QStringLiteral("UDP send to %1:%2 failed: %3").arg(address.toString()).arg(port).arg(socket->errorString());
    return false;
}

int runCycle(QUdpSocket *socket)
{
    QString configError;
    const QJsonObject config = LinkCommon::ensureConfig(&configError);
    const QJsonObject discovery = objectValue(config, QStringLiteral("discovery"));
    const int intervalSeconds = boundedIntervalSeconds(discovery);
    const int announcementPort = boundedPort(discovery, QStringLiteral("announcement_port"), 45177);
    const bool enabled = discovery.value(QStringLiteral("enabled")).toBool(false);

    if (!configError.isEmpty() || config.isEmpty()) {
        writeStatus(false, false, QStringLiteral("error"), LanAddress(), announcementPort, configError, QJsonObject());
        return intervalSeconds;
    }

    if (!enabled) {
        writeStatus(false, false, QStringLiteral("disabled"), LanAddress(), announcementPort, QString(), QJsonObject());
        return intervalSeconds;
    }

    QString lanError;
    const LanAddress lan = selectLanAddress(&lanError);
    if (!lanError.isEmpty()) {
        writeStatus(true, false, QStringLiteral("unavailable"), lan, announcementPort, lanError, QJsonObject());
        return intervalSeconds;
    }

    const QJsonObject payload = buildPayload(config, lan);
    const QByteArray datagram = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QString sendError;
    bool sent = false;

    if (discovery.value(QStringLiteral("broadcast")).toBool(true)) {
        if (lan.broadcast.isNull()) {
            sendError = QStringLiteral("No IPv4 broadcast address is available on %1.")
                    .arg(lan.interfaceName);
        } else {
            sent = sendDatagram(socket, datagram, lan.broadcast,
                                static_cast<quint16>(announcementPort), &sendError) || sent;
        }
    }

    const QString group = discovery.value(QStringLiteral("multicast_group")).toString();
    if (!group.trimmed().isEmpty()) {
        const QHostAddress multicastAddress(group.trimmed());
        if (!isMulticastIpv4(multicastAddress)) {
            if (sendError.isEmpty()) {
                sendError = QStringLiteral("Invalid IPv4 multicast group: %1")
                        .arg(group.trimmed());
            }
        } else {
            socket->setMulticastInterface(lan.networkInterface);
            socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
            sent = sendDatagram(socket, datagram, multicastAddress,
                                static_cast<quint16>(announcementPort), &sendError) || sent;
        }
    }

    if (!sent) {
        writeStatus(true, true, QStringLiteral("error"), lan, announcementPort, sendError, QJsonObject());
        return intervalSeconds;
    }

    writeStatus(true, true, QStringLiteral("announced"), lan, announcementPort, sendError, payload);
    return intervalSeconds;
}

}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("OfficeFake"));
    app.setApplicationName(QStringLiteral("harbour-sailfish-link"));

    LinkCommon::appendLog(QStringLiteral("INFO"), QStringLiteral("discovery service starting"));

    QUdpSocket socket;
    QTimer timer;

    const auto cycle = [&]() {
        const int intervalSeconds = runCycle(&socket);
        timer.setInterval(intervalSeconds * 1000);
    };

    cycle();
    QObject::connect(&timer, &QTimer::timeout, &app, cycle);
    timer.start();

    return app.exec();
}
