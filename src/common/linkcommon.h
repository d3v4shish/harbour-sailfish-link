#ifndef LINKCOMMON_H
#define LINKCOMMON_H

#include <QJsonObject>
#include <QString>

namespace LinkCommon {

QString appName();
QString configDir();
QString dataDir();
QString logDir();
QString configPath();
QString statusPath();
QString logPath();
QString rotatedLogPath();

bool ensureRuntimeDirs(QString *error = 0);
QJsonObject readJson(const QString &path, QString *error = 0);
bool writeJson(const QString &path, const QJsonObject &object, QString *error = 0);
QJsonObject defaultConfig();
QJsonObject ensureConfig(QString *error = 0);
bool saveConfig(const QJsonObject &config, QString *error = 0);

QString generateDeviceId();
QString generatePairingToken();
QString tokenHint(const QString &token);
QString utcNow();
QString defaultDeviceName();
QString defaultSshUser();
QString bytesText(qint64 bytes);
qint64 generatedFilesSize();
bool clearGeneratedFiles(QString *error = 0);
void appendLog(const QString &level, const QString &message);

}

#endif
