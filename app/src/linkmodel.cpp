#include "linkmodel.h"

#include "linkcommon.h"

#include <QJsonDocument>
#include <QJsonValue>

namespace {

bool validPort(int port)
{
    return port > 0 && port <= 65535;
}

QString trimmedOrFallback(const QString &value, const QString &fallback)
{
    const QString trimmed = value.trimmed();
    return trimmed.isEmpty() ? fallback : trimmed;
}

}

LinkModel::LinkModel(QObject *parent)
    : QObject(parent)
{
    load();
}

QJsonObject LinkModel::object(const QJsonObject &parent, const QString &key) const
{
    const QJsonValue value = parent.value(key);
    return value.isObject() ? value.toObject() : QJsonObject();
}

void LinkModel::load()
{
    QString error;
    m_config = LinkCommon::ensureConfig(&error);
    if (!error.isEmpty())
        m_message = error;

    QString statusError;
    m_status = LinkCommon::readJson(LinkCommon::statusPath(), &statusError);
    if (!statusError.isEmpty())
        m_message = statusError;
}

bool LinkModel::saveConfig(QJsonObject config, const QString &successMessage)
{
    QString error;
    if (!LinkCommon::saveConfig(config, &error)) {
        m_message = error;
        emit changed();
        return false;
    }

    LinkCommon::appendLog(QStringLiteral("INFO"), successMessage);
    m_message = successMessage;
    load();
    emit changed();
    return true;
}

QString LinkModel::configPath() const
{
    return LinkCommon::configPath();
}

QString LinkModel::statusPath() const
{
    return LinkCommon::statusPath();
}

QString LinkModel::logPath() const
{
    return LinkCommon::logPath();
}

QString LinkModel::generatedSize() const
{
    return LinkCommon::bytesText(LinkCommon::generatedFilesSize());
}

bool LinkModel::firstRun() const
{
    return m_config.value(QStringLiteral("first_run")).toBool(false);
}

bool LinkModel::enabled() const
{
    return object(m_config, QStringLiteral("discovery")).value(QStringLiteral("enabled")).toBool(false);
}

QString LinkModel::deviceId() const
{
    return object(m_config, QStringLiteral("device")).value(QStringLiteral("id")).toString();
}

QString LinkModel::deviceName() const
{
    return object(m_config, QStringLiteral("device")).value(QStringLiteral("name")).toString();
}

QString LinkModel::pairingToken() const
{
    return object(m_config, QStringLiteral("auth")).value(QStringLiteral("pairing_token")).toString();
}

QString LinkModel::tokenHint() const
{
    return LinkCommon::tokenHint(pairingToken());
}

QString LinkModel::sshUser() const
{
    return object(m_config, QStringLiteral("ssh")).value(QStringLiteral("user")).toString();
}

int LinkModel::sshPort() const
{
    return object(m_config, QStringLiteral("ssh")).value(QStringLiteral("port")).toInt(22);
}

int LinkModel::announcementPort() const
{
    return object(m_config, QStringLiteral("discovery")).value(QStringLiteral("announcement_port")).toInt(45177);
}

QString LinkModel::statusState() const
{
    return m_status.value(QStringLiteral("state")).toString(QStringLiteral("not started"));
}

QString LinkModel::statusSummary() const
{
    if (m_status.isEmpty())
        return QStringLiteral("No service status has been written yet.");

    const QString state = statusState();
    const QString ip = currentIp();
    const QString iface = interfaceName();
    if (state == QStringLiteral("announced"))
        return QStringLiteral("Announcing %1 on %2.").arg(ip, iface);
    if (state == QStringLiteral("disabled"))
        return QStringLiteral("Discovery is disabled.");
    if (state == QStringLiteral("unavailable"))
        return QStringLiteral("Discovery is unavailable: %1").arg(lastError());
    if (state == QStringLiteral("error"))
        return QStringLiteral("Discovery error: %1").arg(lastError());
    return state;
}

QString LinkModel::currentIp() const
{
    return m_status.value(QStringLiteral("ip")).toString();
}

QString LinkModel::interfaceName() const
{
    return m_status.value(QStringLiteral("interface")).toString();
}

QString LinkModel::lastAnnounceAt() const
{
    return m_status.value(QStringLiteral("last_announce_at")).toString();
}

QString LinkModel::lastError() const
{
    return m_status.value(QStringLiteral("last_error")).toString();
}

QString LinkModel::payloadPreview() const
{
    const QJsonValue payload = m_status.value(QStringLiteral("payload"));
    if (!payload.isObject())
        return QString();
    return QString::fromUtf8(QJsonDocument(payload.toObject()).toJson(QJsonDocument::Indented));
}

QString LinkModel::message() const
{
    return m_message;
}

void LinkModel::refresh()
{
    load();
    if (m_message.isEmpty())
        m_message = QStringLiteral("Refreshed.");
    emit changed();
}

void LinkModel::setDiscoveryEnabled(bool enabled)
{
    QJsonObject config = m_config;
    QJsonObject discovery = object(config, QStringLiteral("discovery"));
    discovery.insert(QStringLiteral("enabled"), enabled);
    config.insert(QStringLiteral("discovery"), discovery);

    saveConfig(config, enabled ? QStringLiteral("Discovery enabled.") : QStringLiteral("Discovery disabled."));
}

void LinkModel::saveSettings(const QString &deviceName,
                             const QString &sshUser,
                             int sshPort,
                             int announcementPort,
                             bool enabled)
{
    if (!validPort(sshPort) || !validPort(announcementPort)) {
        m_message = QStringLiteral("Ports must be between 1 and 65535.");
        emit changed();
        return;
    }

    QJsonObject config = m_config;

    QJsonObject device = object(config, QStringLiteral("device"));
    device.insert(QStringLiteral("name"), trimmedOrFallback(deviceName, LinkCommon::defaultDeviceName()));
    config.insert(QStringLiteral("device"), device);

    QJsonObject ssh = object(config, QStringLiteral("ssh"));
    ssh.insert(QStringLiteral("user"), trimmedOrFallback(sshUser, LinkCommon::defaultSshUser()));
    ssh.insert(QStringLiteral("port"), sshPort);
    config.insert(QStringLiteral("ssh"), ssh);

    QJsonObject discovery = object(config, QStringLiteral("discovery"));
    discovery.insert(QStringLiteral("announcement_port"), announcementPort);
    discovery.insert(QStringLiteral("enabled"), enabled);
    config.insert(QStringLiteral("discovery"), discovery);

    saveConfig(config, QStringLiteral("Settings saved."));
}

void LinkModel::rotatePairingToken()
{
    QJsonObject config = m_config;
    QJsonObject auth = object(config, QStringLiteral("auth"));
    auth.insert(QStringLiteral("pairing_token"), LinkCommon::generatePairingToken());
    config.insert(QStringLiteral("auth"), auth);

    saveConfig(config, QStringLiteral("Pairing token rotated."));
}

void LinkModel::clearGeneratedFiles()
{
    QString error;
    if (!LinkCommon::clearGeneratedFiles(&error)) {
        m_message = error;
        emit changed();
        return;
    }

    m_message = QStringLiteral("Generated status and logs cleared.");
    load();
    emit changed();
}

void LinkModel::markFirstRunSeen()
{
    QJsonObject config = m_config;
    config.insert(QStringLiteral("first_run"), false);
    saveConfig(config, QStringLiteral("First-run help dismissed."));
}
