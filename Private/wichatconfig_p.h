#ifndef WICHATCONFIGPRIVATE_H
#define WICHATCONFIGPRIVATE_H

#include <QSettings>

class WichatConfig;

class WichatConfigPrivate : QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(WichatConfig)
    WichatConfig* q_ptr;

public:
    WichatConfigPrivate(WichatConfig* parent);
    QSettings appConfig{"Wichat", "Desktop"};
    QSettings userConfig;
    QString currentUserID;
    QString currentUserDir;

    bool switchUser(QString userID);
};

#endif // WICHATCONFIGPRIVATE_H
