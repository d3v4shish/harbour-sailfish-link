#ifndef LINKMODEL_H
#define LINKMODEL_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class LinkModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath NOTIFY changed)
    Q_PROPERTY(QString statusPath READ statusPath NOTIFY changed)
    Q_PROPERTY(QString logPath READ logPath NOTIFY changed)
    Q_PROPERTY(QString generatedSize READ generatedSize NOTIFY changed)
    Q_PROPERTY(bool firstRun READ firstRun NOTIFY changed)
    Q_PROPERTY(bool enabled READ enabled NOTIFY changed)
    Q_PROPERTY(QString deviceId READ deviceId NOTIFY changed)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY changed)
    Q_PROPERTY(QString pairingToken READ pairingToken NOTIFY changed)
    Q_PROPERTY(QString tokenHint READ tokenHint NOTIFY changed)
    Q_PROPERTY(QString sshUser READ sshUser NOTIFY changed)
    Q_PROPERTY(int sshPort READ sshPort NOTIFY changed)
    Q_PROPERTY(int announcementPort READ announcementPort NOTIFY changed)
    Q_PROPERTY(QString statusState READ statusState NOTIFY changed)
    Q_PROPERTY(QString statusSummary READ statusSummary NOTIFY changed)
    Q_PROPERTY(QString currentIp READ currentIp NOTIFY changed)
    Q_PROPERTY(QString interfaceName READ interfaceName NOTIFY changed)
    Q_PROPERTY(QString lastAnnounceAt READ lastAnnounceAt NOTIFY changed)
    Q_PROPERTY(QString lastError READ lastError NOTIFY changed)
    Q_PROPERTY(QString payloadPreview READ payloadPreview NOTIFY changed)
    Q_PROPERTY(QString message READ message NOTIFY changed)

public:
    explicit LinkModel(QObject *parent = 0);

    QString configPath() const;
    QString statusPath() const;
    QString logPath() const;
    QString generatedSize() const;
    bool firstRun() const;
    bool enabled() const;
    QString deviceId() const;
    QString deviceName() const;
    QString pairingToken() const;
    QString tokenHint() const;
    QString sshUser() const;
    int sshPort() const;
    int announcementPort() const;
    QString statusState() const;
    QString statusSummary() const;
    QString currentIp() const;
    QString interfaceName() const;
    QString lastAnnounceAt() const;
    QString lastError() const;
    QString payloadPreview() const;
    QString message() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setDiscoveryEnabled(bool enabled);
    Q_INVOKABLE void saveSettings(const QString &deviceName,
                                  const QString &sshUser,
                                  int sshPort,
                                  int announcementPort,
                                  bool enabled);
    Q_INVOKABLE void rotatePairingToken();
    Q_INVOKABLE void clearGeneratedFiles();
    Q_INVOKABLE void markFirstRunSeen();

signals:
    void changed();

private:
    void load();
    bool saveConfig(QJsonObject config, const QString &successMessage);
    QJsonObject object(const QJsonObject &parent, const QString &key) const;

    QJsonObject m_config;
    QJsonObject m_status;
    QString m_message;
};

#endif
