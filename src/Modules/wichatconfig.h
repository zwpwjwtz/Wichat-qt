#ifndef WICHATCONFIG_H
#define WICHATCONFIG_H

#include <QObject>
#include <QVector>

class WichatConfigPrivate;

class WichatConfig
{
    Q_DECLARE_PRIVATE(WichatConfig)
    WichatConfigPrivate* d_ptr;

public:
    explicit WichatConfig();
    ~WichatConfig();

    // Application configuration
    bool loadConfigFile(QString path);
    bool saveConfigFile(QString path);
    QString httpProxy();
    void setHttpProxy(QString proxyServer);
    QString rootServer();
    void setRootServer(QString server);
    QString lastID();
    void setLastID(QString userID);
    QString recordPath();
    void setRecordPath(QString path);
    bool isDefaultRecordPath();

    // User configuration
    QString userDirectory(QString userID);
    int lastLoginState(QString userID);
    void setLastLoginState(QString userID, int state);
    bool rememberPassword(QString userID);
    void setRememberPassword(QString userID, bool remember);
    QVector<QString> lastSessions(QString userID);
    void setLastSession(QString userID, QVector<QString> lastSessions);
    QString prefFontFamily(QString userID);
    void setPrefFontFamily(QString userID, QString family);
    QString prefFontSize(QString userID);
    void setPrefFontSize(QString userID, QString size);
    QString prefFontColor(QString userID);
    void setPrefFontColor(QString userID, QString color);
    QString prefFontStyle(QString userID);
    void setPrefFontStyle(QString userID, QString style);
    QString prefTextAlign(QString userID);
    void setPrefTextAlign(QString userID, QString alignMode);
    int prefSendKey(QString userID);
    void setPrefSendKey(QString userID, int keyCode);
    int prefScreenShot(QString userID);
    void setPrefScreenShot(QString userID, int hidden);
    int prefNotify(QString userID);
    void setPrefNotify(QString userID, int notifyLevel);
};

#endif // WICHATCONFIG_H
