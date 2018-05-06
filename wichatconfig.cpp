#include <QCoreApplication>
#include <QDir>

#include "wichatconfig.h"
#include "Private/wichatconfig_p.h"

#define WICHAT_CONFIG_FILENAME_USER "preference.ini"

#define WICHAT_CONFIG_KEY_CONF_VER "Wichat_config_version"
#define WICHAT_CONFIG_VALUE_CONF_VER "1"
#define WICHAT_CONFIG_KEY_PROXY_HTTP "http_proxy"
#define WICHAT_CONFIG_KEY_SERV_ROOT "root_server"
#define WICHAT_CONFIG_KEY_LAST_USER "last_user"
#define WICHAT_CONFIG_KEY_RECORD_PATH "record_path"

#define WICHAT_CONFIG_KEY_LOGIN_STATE "login_state"
#define WICHAT_CONFIG_KEY_LOGIN_REM "remember_password"
#define WICHAT_CONFIG_KEY_SESSION_LAST "last_session"
#define WICHAT_CONFIG_KEY_PREF_FONT_FAMILY "font_family"
#define WICHAT_CONFIG_KEY_PREF_FONT_SIZE "font_size"
#define WICHAT_CONFIG_KEY_PREF_FONT_COLOR "font_color"
#define WICHAT_CONFIG_KEY_PREF_FONT_STYLE "font_style"
#define WICHAT_CONFIG_KEY_PREF_TEXT_ALIGN "text_align"
#define WICHAT_CONFIG_KEY_PREF_KEY_SEND "shortcut_send"
#define WICHAT_CONFIG_KEY_PREF_SCNSHOT "screen_shot_behavior"
#define WICHAT_CONFIG_KEY_PREF_NOTIFY "msg_notify_behavior"


WichatConfig::WichatConfig()
{
    this->d_ptr = new WichatConfigPrivate(this);
}

bool WichatConfig::loadConfigFile(QString path)
{
    Q_D(WichatConfig);

    if (path.isEmpty() || path == d->appConfig.fileName())
        return false;

    d->appConfig.sync();
    d->appConfig.setPath(QSettings::NativeFormat,
                         QSettings::UserScope,
                         path);
    if (d->appConfig.value(WICHAT_CONFIG_KEY_CONF_VER).toString().isEmpty())
        return false;
    else
        return true;
}

bool WichatConfig::saveConfigFile(QString path)
{
    Q_D(WichatConfig);

    if (path.isEmpty())
        return false;

    if (path != d->appConfig.fileName())
        d->appConfig.setPath(QSettings::NativeFormat,
                             QSettings::UserScope,
                             path);
    d->appConfig.sync();
    return true;
}

QString WichatConfig::httpProxy()
{
    Q_D(WichatConfig);
    return d->appConfig.value(WICHAT_CONFIG_KEY_PROXY_HTTP).toString();
}

void WichatConfig::setHttpProxy(QString proxyServer)
{
    Q_D(WichatConfig);
    d->appConfig.setValue(WICHAT_CONFIG_KEY_PROXY_HTTP, proxyServer);
}

QString WichatConfig::rootServer()
{
    Q_D(WichatConfig);
    return d->appConfig.value(WICHAT_CONFIG_KEY_SERV_ROOT).toString();
}

void WichatConfig::setRootServer(QString server)
{
    Q_D(WichatConfig);
    d->appConfig.setValue(WICHAT_CONFIG_KEY_SERV_ROOT, server);
}

QString WichatConfig::lastID()
{
    Q_D(WichatConfig);
    return d->appConfig.value(WICHAT_CONFIG_KEY_LAST_USER).toString();
}

void WichatConfig::setLastID(QString userID)
{
    Q_D(WichatConfig);
    d->appConfig.setValue(WICHAT_CONFIG_KEY_LAST_USER, userID);
}

QString WichatConfig::recordPath()
{
    Q_D(WichatConfig);
    QString path = d->appConfig.value(WICHAT_CONFIG_KEY_RECORD_PATH).toString();
    if (path.isEmpty())
        path = d->defaultRecordPath();
    return path;
}

void WichatConfig::setRecordPath(QString path)
{
    Q_D(WichatConfig);
    d->appConfig.setValue(WICHAT_CONFIG_KEY_RECORD_PATH, path);
}

QString WichatConfig::userDirectory(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return "";
    return d->currentUserDir;
}

int WichatConfig::lastLoginState(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return -1;
    return d->userConfig->value(WICHAT_CONFIG_KEY_LOGIN_STATE).toInt();
}

void WichatConfig::setLastLoginState(QString userID, int state)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;
    d->userConfig->setValue(WICHAT_CONFIG_KEY_LOGIN_STATE, state);
}

bool WichatConfig::rememberPassword(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return false;
    return d->userConfig->value(WICHAT_CONFIG_KEY_LOGIN_REM).toBool();
}

void WichatConfig::setRememberPassword(QString userID, bool remember)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;
    d->userConfig->setValue(WICHAT_CONFIG_KEY_LOGIN_REM, remember);
}

QVector<QString> WichatConfig::lastSessions(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return QVector<QString>();
    return d->userConfig->value(WICHAT_CONFIG_KEY_SESSION_LAST)
                        .toString().split(';').toVector();
}

void WichatConfig::setLastSession(QString userID, QVector<QString> lastSessions)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;

    QString sessionIdList;
    for (int i=0; i<lastSessions.count(); i++)
        sessionIdList.append(lastSessions[i]).append(';');
    if (sessionIdList[sessionIdList.length() - 1] == ';')
        sessionIdList.chop(1);
    d->userConfig->setValue(WICHAT_CONFIG_KEY_SESSION_LAST, sessionIdList);
}

QString WichatConfig::prefFontFamily(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return "";
    return d->userConfig->value(WICHAT_CONFIG_KEY_PREF_FONT_FAMILY).toString();
}

void WichatConfig::setPrefFontFamily(QString userID, QString family)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;
    d->userConfig->setValue(WICHAT_CONFIG_KEY_PREF_FONT_FAMILY, family);
}

QString WichatConfig::prefFontSize(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return "";
    return d->userConfig->value(WICHAT_CONFIG_KEY_PREF_FONT_SIZE).toString();
}

void WichatConfig::setPrefFontSize(QString userID, QString size)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;
    d->userConfig->setValue(WICHAT_CONFIG_KEY_PREF_FONT_SIZE, size);
}

QString WichatConfig::prefFontColor(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return "";
    return d->userConfig->value(WICHAT_CONFIG_KEY_PREF_FONT_COLOR).toString();
}

void WichatConfig::setPrefFontColor(QString userID, QString color)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;
    d->userConfig->setValue(WICHAT_CONFIG_KEY_PREF_FONT_COLOR, color);
}

QString WichatConfig::prefFontStyle(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return "";
    return d->userConfig->value(WICHAT_CONFIG_KEY_PREF_FONT_STYLE).toString();
}

void WichatConfig::setPrefFontStyle(QString userID, QString style)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;
    d->userConfig->setValue(WICHAT_CONFIG_KEY_PREF_FONT_STYLE, style);
}

QString WichatConfig::prefTextAlign(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return "";
    return d->userConfig->value(WICHAT_CONFIG_KEY_PREF_TEXT_ALIGN).toString();
}

void WichatConfig::setPrefTextAlign(QString userID, QString alignMode)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;
    d->userConfig->setValue(WICHAT_CONFIG_KEY_PREF_TEXT_ALIGN, alignMode);
}

int WichatConfig::prefSendKey(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return -1;
    return d->userConfig->value(WICHAT_CONFIG_KEY_PREF_KEY_SEND).toInt();
}

void WichatConfig::setPrefSendKey(QString userID, int keyCode)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;
    d->userConfig->setValue(WICHAT_CONFIG_KEY_PREF_KEY_SEND, keyCode);
}

int WichatConfig::prefScreenShot(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return -1;
    return d->userConfig->value(WICHAT_CONFIG_KEY_PREF_SCNSHOT).toInt();
}

void WichatConfig::setPrefScreenShot(QString userID, int hidden)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;
    d->userConfig->setValue(WICHAT_CONFIG_KEY_PREF_SCNSHOT, hidden);
}

int WichatConfig::prefNotify(QString userID)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return -1;
    return d->userConfig->value(WICHAT_CONFIG_KEY_PREF_NOTIFY).toInt();
}

void WichatConfig::setPrefNotify(QString userID, int notifyLevel)
{
    Q_D(WichatConfig);
    if (!d->switchUser(userID))
        return;
    d->userConfig->setValue(WICHAT_CONFIG_KEY_PREF_NOTIFY, notifyLevel);
}

WichatConfigPrivate::WichatConfigPrivate(WichatConfig* parent)
{
    this->q_ptr = parent;
    this->userConfig = nullptr;
}

bool WichatConfigPrivate::switchUser(QString userID)
{
    if (userID == currentUserID)
        return true;

    QString tempPath = appConfig.value(WICHAT_CONFIG_KEY_RECORD_PATH)
                                .toString();
    if (tempPath.isEmpty())
        tempPath = defaultRecordPath();

    // Enter data directory
    QDir userDir(tempPath);
    if (!userDir.exists())
    {
        if (!userDir.mkpath("."))
            return false;
    }

    // Enter user record directory
    if (!userDir.cd(userID))
    {
        if (!userDir.mkdir(userID))
            return false;
        else
            userDir.cd(userID);
    }

    currentUserDir = userDir.path();
    if (!userConfig)
        delete userConfig;
    userConfig = new QSettings(QString(currentUserDir)
                               .append('/')
                               .append(WICHAT_CONFIG_FILENAME_USER),
                               QSettings::NativeFormat,
                               this);
    return true;
}

QString WichatConfigPrivate::defaultRecordPath()
{
#ifdef Q_OS_WIN32
    return QCoreApplication::applicationDirPath();
#else
    QString appName = QCoreApplication::applicationName();
#ifdef Q_OS_MACOS
    return appName.prepend("/Library/Application Support/")
                  .prepend(QDir::homePath());
#else
    return appName.prepend("/.local/share/")
                  .prepend(QDir::homePath());
#endif
#endif
}
