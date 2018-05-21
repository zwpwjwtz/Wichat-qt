#ifndef USERSESSION_H
#define USERSESSION_H

#include "abstractsession.h"


class SessionMessageList;

class UserSessionPrivate;

class UserSession : public AbstractSession
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(UserSession)
protected:
    UserSession(UserSessionPrivate* d);

public:
    struct SessionData : public AbstractSessionData
    {
        QString ID;
        bool active;
        SessionMessageList* messageList;
        QByteArray input;
    };

    explicit UserSession();
    bool loadFromFile(QString userDir);
    bool saveToFile(QString userDir);
    int count();
    bool exists(QString sessionID);
    void add(QString sessionID);
    void remove(QString sessionID);
    void removeAll();
    SessionData& getSession(QString sessionID);
    SessionData& currentSession();
    QList<QString> sessionIdList();
};

#endif // USERSESSION_H
