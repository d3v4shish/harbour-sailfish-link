#include "linkcommon.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QProcessEnvironment>
#include <QJsonValue>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>
#include <QSysInfo>
#include <QTextStream>
#include <QUuid>

namespace {

const char *AppName = "harbour-sailfish-link";
const qint64 MaxLogBytes = 512 * 1024;

QFileDevice::Permissions privateFilePermissions()
{
    return QFileDevice::ReadOwner | QFileDevice::WriteOwner;
}

QFileDevice::Permissions privateDirectoryPermissions()
{
    return QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner;
}

bool setPrivatePermissions(const QString &path,
                           QFileDevice::Permissions permissions,
                           QString *error)
{
    if (QFile::setPermissions(path, permissions))
        return true;

    if (error) {
        *error = QStringLiteral("Could not restrict permissions on %1").arg(path);
    }
    return false;
}

QString pathIn(const QString &base, const QString &name)
{
    QDir dir(base);
    return dir.filePath(name);
}

QString writableBase(QStandardPaths::StandardLocation location)
{
    const QString base = QStandardPaths::writableLocation(location);
    if (!base.isEmpty())
        return base;
    return QDir::homePath();
}

QJsonObject objectValue(const QJsonObject &parent, const QString &key)
{
    const QJsonValue value = parent.value(key);
    return value.isObject() ? value.toObject() : QJsonObject();
}

QString cleanUuid()
{
    QString value = QUuid::createUuid().toString();
    value.remove(QLatin1Char('{'));
    value.remove(QLatin1Char('}'));
    value.remove(QLatin1Char('-'));
    return value;
}

QString normalizedToken(const QString &token)
{
    QString value = token.trimmed().toLower();
    value.remove(QLatin1Char('-'));
    return value;
}

bool ensureObject(QJsonObject *config, const QString &key, const QJsonObject &defaults)
{
    bool changed = false;
    QJsonObject child = objectValue(*config, key);

    for (QJsonObject::const_iterator it = defaults.constBegin(); it != defaults.constEnd(); ++it) {
        if (!child.contains(it.key())) {
            child.insert(it.key(), it.value());
            changed = true;
        }
    }

    if (!config->value(key).isObject()) {
        changed = true;
    }

    config->insert(key, child);
    return changed;
}

bool ensureNonEmptyString(QJsonObject *object, const QString &key, const QString &fallback)
{
    const QString original = object->value(key).toString();
    const QString value = original.trimmed();
    if (!value.isEmpty()) {
        if (value != original) {
            object->insert(key, value);
            return true;
        }
        return false;
    }

    object->insert(key, fallback);
    return true;
}

bool isPairingToken(const QString &token)
{
    QString normalized = token.trimmed().toLower();
    normalized.remove(QLatin1Char('-'));
    if (normalized.size() != 32)
        return false;

    for (int i = 0; i < normalized.size(); ++i) {
        const QChar character = normalized.at(i);
        if (!character.isDigit() && (character < QLatin1Char('a') || character > QLatin1Char('f')))
            return false;
    }
    return true;
}

bool ensurePairingToken(QJsonObject *auth)
{
    if (isPairingToken(auth->value(QStringLiteral("pairing_token")).toString()))
        return false;

    auth->insert(QStringLiteral("pairing_token"), LinkCommon::generatePairingToken());
    return true;
}

void rotateLogIfNeeded()
{
    QFileInfo info(LinkCommon::logPath());
    if (!info.exists() || info.size() < MaxLogBytes)
        return;

    QFile::remove(LinkCommon::rotatedLogPath());
    QFile::rename(LinkCommon::logPath(), LinkCommon::rotatedLogPath());
}

}

namespace LinkCommon {

QString appName()
{
    return QString::fromLatin1(AppName);
}

QString configDir()
{
    return pathIn(writableBase(QStandardPaths::GenericConfigLocation), appName());
}

QString dataDir()
{
    return pathIn(writableBase(QStandardPaths::GenericDataLocation), appName());
}

QString logDir()
{
    return pathIn(dataDir(), QStringLiteral("logs"));
}

QString configPath()
{
    return pathIn(configDir(), QStringLiteral("config.json"));
}

QString statusPath()
{
    return pathIn(dataDir(), QStringLiteral("status.json"));
}

QString logPath()
{
    return pathIn(logDir(), QStringLiteral("link.log"));
}

QString rotatedLogPath()
{
    return pathIn(logDir(), QStringLiteral("link.log.1"));
}

bool ensureRuntimeDirs(QString *error)
{
    const QStringList dirs = QStringList() << configDir() << dataDir() << logDir();
    for (int i = 0; i < dirs.size(); ++i) {
        QDir dir(dirs.at(i));
        if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
            if (error)
                *error = QStringLiteral("Could not create %1").arg(dirs.at(i));
            return false;
        }
        if (!setPrivatePermissions(dirs.at(i), privateDirectoryPermissions(), error))
            return false;
    }
    return true;
}

QJsonObject readJson(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.exists())
        return QJsonObject();
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("Could not open %1: %2").arg(path, file.errorString());
        return QJsonObject();
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error)
            *error = QStringLiteral("Invalid JSON in %1: %2").arg(path, parseError.errorString());
        return QJsonObject();
    }

    return document.object();
}

bool writeJson(const QString &path, const QJsonObject &object, QString *error)
{
    QFileInfo info(path);
    QDir dir(info.absolutePath());
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        if (error)
            *error = QStringLiteral("Could not create %1").arg(info.absolutePath());
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QStringLiteral("Could not write %1: %2").arg(path, file.errorString());
        return false;
    }

    const QByteArray json = QJsonDocument(object).toJson(QJsonDocument::Indented);
    if (file.write(json) != json.size()) {
        if (error)
            *error = QStringLiteral("Could not write %1: %2").arg(path, file.errorString());
        return false;
    }
    if (!file.commit()) {
        if (error)
            *error = QStringLiteral("Could not commit %1: %2").arg(path, file.errorString());
        return false;
    }

    if (!setPrivatePermissions(path, privateFilePermissions(), error))
        return false;

    return true;
}

QString generateDeviceId()
{
    QString value = QUuid::createUuid().toString();
    value.remove(QLatin1Char('{'));
    value.remove(QLatin1Char('}'));
    return value;
}

QString generatePairingToken()
{
    const QString raw = cleanUuid();
    QStringList groups;
    for (int i = 0; i < 32; i += 4)
        groups << raw.mid(i, 4);
    return groups.join(QLatin1Char('-'));
}

QString tokenHint(const QString &token)
{
    const QString value = normalizedToken(token);
    if (value.size() <= 6)
        return value;
    return value.right(6);
}

QString utcNow()
{
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
}

QString defaultDeviceName()
{
    QString name = QSysInfo::machineHostName();
    if (name.trimmed().isEmpty())
        name = QProcessEnvironment::systemEnvironment().value(QStringLiteral("HOSTNAME"));
    if (name.trimmed().isEmpty())
        name = QStringLiteral("Sailfish device");
    return name.trimmed();
}

QString defaultSshUser()
{
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString user = env.value(QStringLiteral("USER"));
    if (user.trimmed().isEmpty())
        user = env.value(QStringLiteral("LOGNAME"));
    if (user.trimmed().isEmpty())
        user = QStringLiteral("defaultuser");
    return user.trimmed();
}

QJsonObject defaultConfig()
{
    QJsonObject device;
    device.insert(QStringLiteral("id"), generateDeviceId());
    device.insert(QStringLiteral("name"), defaultDeviceName());

    QJsonObject discovery;
    discovery.insert(QStringLiteral("enabled"), false);
    discovery.insert(QStringLiteral("announcement_port"), 45177);
    discovery.insert(QStringLiteral("announcement_interval_seconds"), 15);
    discovery.insert(QStringLiteral("broadcast"), true);
    discovery.insert(QStringLiteral("multicast_group"), QStringLiteral("239.255.77.77"));

    QJsonObject ssh;
    ssh.insert(QStringLiteral("port"), 22);
    ssh.insert(QStringLiteral("user"), defaultSshUser());

    QJsonObject webcam;
    webcam.insert(QStringLiteral("port"), 8090);
    webcam.insert(QStringLiteral("mjpeg_path"), QStringLiteral("/stream.mjpeg"));
    webcam.insert(QStringLiteral("status_path"), QStringLiteral("/status.json"));

    QJsonObject lls;
    lls.insert(QStringLiteral("port"), 8091);
    lls.insert(QStringLiteral("control_path"), QStringLiteral("/control"));
    lls.insert(QStringLiteral("status_path"), QStringLiteral("/status"));

    QJsonObject auth;
    auth.insert(QStringLiteral("pairing_token"), generatePairingToken());

    QJsonObject config;
    config.insert(QStringLiteral("config_version"), 1);
    config.insert(QStringLiteral("first_run"), true);
    config.insert(QStringLiteral("device"), device);
    config.insert(QStringLiteral("discovery"), discovery);
    config.insert(QStringLiteral("ssh"), ssh);
    config.insert(QStringLiteral("webcam"), webcam);
    config.insert(QStringLiteral("lls"), lls);
    config.insert(QStringLiteral("auth"), auth);
    return config;
}

QJsonObject ensureConfig(QString *error)
{
    if (!ensureRuntimeDirs(error))
        return QJsonObject();

    const QFileInfo configInfo(configPath());
    QString readError;
    QJsonObject config = readJson(configPath(), &readError);
    if (!readError.isEmpty()) {
        if (error)
            *error = readError;
        return QJsonObject();
    }

    if (!configInfo.exists()) {
        config = defaultConfig();
        if (!writeJson(configPath(), config, error))
            return QJsonObject();
        return config;
    }

    QJsonObject defaults = defaultConfig();
    bool changed = false;

    if (config.value(QStringLiteral("config_version")).toInt() != 1) {
        config.insert(QStringLiteral("config_version"), 1);
        changed = true;
    }
    if (!config.contains(QStringLiteral("first_run"))) {
        config.insert(QStringLiteral("first_run"), false);
        changed = true;
    }

    changed = ensureObject(&config, QStringLiteral("device"), objectValue(defaults, QStringLiteral("device"))) || changed;
    changed = ensureObject(&config, QStringLiteral("discovery"), objectValue(defaults, QStringLiteral("discovery"))) || changed;
    changed = ensureObject(&config, QStringLiteral("ssh"), objectValue(defaults, QStringLiteral("ssh"))) || changed;
    changed = ensureObject(&config, QStringLiteral("webcam"), objectValue(defaults, QStringLiteral("webcam"))) || changed;
    changed = ensureObject(&config, QStringLiteral("lls"), objectValue(defaults, QStringLiteral("lls"))) || changed;
    changed = ensureObject(&config, QStringLiteral("auth"), objectValue(defaults, QStringLiteral("auth"))) || changed;

    QJsonObject device = objectValue(config, QStringLiteral("device"));
    changed = ensureNonEmptyString(&device, QStringLiteral("id"), generateDeviceId()) || changed;
    changed = ensureNonEmptyString(&device, QStringLiteral("name"), defaultDeviceName()) || changed;
    config.insert(QStringLiteral("device"), device);

    QJsonObject ssh = objectValue(config, QStringLiteral("ssh"));
    changed = ensureNonEmptyString(&ssh, QStringLiteral("user"), defaultSshUser()) || changed;
    config.insert(QStringLiteral("ssh"), ssh);

    QJsonObject auth = objectValue(config, QStringLiteral("auth"));
    changed = ensurePairingToken(&auth) || changed;
    config.insert(QStringLiteral("auth"), auth);

    if (changed && !writeJson(configPath(), config, error))
        return QJsonObject();

    return config;
}

bool saveConfig(const QJsonObject &config, QString *error)
{
    if (!ensureRuntimeDirs(error))
        return false;
    return writeJson(configPath(), config, error);
}

QString bytesText(qint64 bytes)
{
    if (bytes < 1024)
        return QStringLiteral("%1 B").arg(bytes);
    if (bytes < 1024 * 1024)
        return QStringLiteral("%1 KiB").arg(QString::number(bytes / 1024.0, 'f', 1));
    return QStringLiteral("%1 MiB").arg(QString::number(bytes / (1024.0 * 1024.0), 'f', 1));
}

qint64 generatedFilesSize()
{
    qint64 total = 0;
    const QStringList paths = QStringList() << statusPath() << logPath() << rotatedLogPath();
    for (int i = 0; i < paths.size(); ++i) {
        QFileInfo info(paths.at(i));
        if (info.exists())
            total += info.size();
    }
    return total;
}

bool clearGeneratedFiles(QString *error)
{
    bool ok = true;
    const QStringList paths = QStringList() << statusPath() << logPath() << rotatedLogPath();
    for (int i = 0; i < paths.size(); ++i) {
        QFile file(paths.at(i));
        if (!file.exists())
            continue;
        if (!file.remove()) {
            ok = false;
            if (error && error->isEmpty())
                *error = QStringLiteral("Could not remove %1: %2").arg(paths.at(i), file.errorString());
        }
    }
    return ok;
}

void appendLog(const QString &level, const QString &message)
{
    QString error;
    if (!ensureRuntimeDirs(&error))
        return;

    rotateLogIfNeeded();

    QFile file(logPath());
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return;

    if (!QFile::setPermissions(logPath(), privateFilePermissions()))
        return;

    QTextStream out(&file);
    out << utcNow() << " [" << level << "] " << message << "\n";
}

}
