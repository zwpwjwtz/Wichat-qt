#ifndef USERSESSIONPRIVATE_H
#define USERSESSIONPRIVATE_H

#include <QObject>
#include <QFile>

#include "abstractsession_p.h"
#include "usersession.h"

class UserSessionPrivate : public AbstractSessionPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(UserSession)
protected:
    UserSession* q_ptr;

public:
    static UserSession::SessionData emptySession; // const
    QList<UserSession::SessionData> sessionList;

    UserSessionPrivate(UserSession *parent);
    int getActiveSession();
};

#endif // USERSESSIONPRIVATE_H
