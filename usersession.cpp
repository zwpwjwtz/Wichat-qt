#include <QDir>

#include "usersession.h"
#include "Private/usersession_p.h"
#include "common.h"

#define WICHAT_SESSION_CACHE_FILENAME "session.dat"

#define WICHAT_SESSION_CACHE_FLAG_ACTIVE '\x01'
#define WICHAT_SESSION_CACHE_FLAG_INACTIVE '\x02'
#define WICHAT_SESSION_CACHE_DELIMITER "\x07\x07\x07\x07"


UserSession::UserSession()
{
    this->d_ptr = new UserSessionPrivate(this);
}

UserSession::UserSession(UserSessionPrivate* d)
{
    this->d_ptr = d;
}

bool UserSession::loadFromFile(QString userDir)
{
    Q_D(UserSession);

    QFile sessionFile(d->getSessionFile(userDir,
                                        WICHAT_SESSION_CACHE_FILENAME));
    if (sessionFile.isReadable())
        return false;

    QByteArray temp;
    SessionData newSession;
    d->sessionList.clear();
    sessionFile.open(QFile::ReadOnly);
    while (!sessionFile.atEnd())
    {
        // Read session ID
        temp = sessionFile.read(WICHAT_ACCOUNT_ID_MAXLEN).trimmed();
        if (sessionFile.atEnd())
            break;
        if (temp.length() < 1)
            continue;
        newSession.ID = temp;

        // Read active state
        temp = d->readUntil(sessionFile, QByteArray("\x00", 1));
        if (sessionFile.atEnd())
            break;
        if (temp.at(0) == WICHAT_SESSION_CACHE_FLAG_ACTIVE)
            newSession.active = true;
        else
            newSession.active = false;

        // Read input area and message display area
        newSession.input = d->readUntil(sessionFile,
                                        WICHAT_SESSION_CACHE_DELIMITER);
        newSession.cache = d->readUntil(sessionFile,
                                        WICHAT_SESSION_CACHE_DELIMITER);

        d->sessionList.push_back(newSession);
    }
    sessionFile.close();

    return true;
}

bool UserSession::saveToFile(QString userDir)
{
    Q_D(UserSession);

    QFile sessionFile(d->getSessionFile(userDir,
                                        WICHAT_SESSION_CACHE_FILENAME));
    if (sessionFile.isWritable())
        return false;

    QByteArray temp;
    sessionFile.open(QFile::WriteOnly);
    for (int i=0; i<d->sessionList.count(); i++)
    {
        if (d->sessionList[i].ID.isEmpty())
            continue;

        // Write session ID
        temp = d->sessionList[i].ID
                .leftJustified(WICHAT_ACCOUNT_ID_MAXLEN, '\0')
                .toLocal8Bit();
        sessionFile.write(temp);

        // Write active state
        temp.clear();
        if (d->sessionList[i].active)
            temp.append(WICHAT_SESSION_CACHE_FLAG_ACTIVE);
        else
            temp.append(WICHAT_SESSION_CACHE_FLAG_INACTIVE);
        temp.append('\0');
        sessionFile.write(temp);

        // Write input and message display area
        sessionFile.write(d->sessionList[i].input);
        sessionFile.write(WICHAT_SESSION_CACHE_DELIMITER);
        sessionFile.write(d->sessionList[i].cache);
        sessionFile.write(WICHAT_SESSION_CACHE_DELIMITER);
    }
    sessionFile.close();

    return true;
}

int UserSession::count()
{
    Q_D(UserSession);
    return d->sessionList.count();
}

bool UserSession::exists(QString sessionID)
{
    Q_D(UserSession);
    for (int i=0; i<d->sessionList.count(); i++)
        if (d->sessionList[i].ID == sessionID)
            return true;
    return false;
}

void UserSession::add(QString sessionID)
{
    Q_D(UserSession);
    if (sessionID.isEmpty())
        return;
    SessionData newSession;
    newSession.ID = sessionID;
    d->sessionList.push_back(newSession);
}

void UserSession::remove(QString sessionID)
{
    Q_D(UserSession);
    for (int i=0; i<d->sessionList.count(); i++)
    {
        if (d->sessionList[i].ID == sessionID)
        {
            d->sessionList.removeAt(i);
            i--;
        }
    }
}

void UserSession::removeAll()
{
    Q_D(UserSession);
    d->sessionList.clear();
}

UserSession::SessionData &UserSession::getSession(QString sessionID)
{
    Q_D(UserSession);
    for (int i=0; i<d->sessionList.count(); i++)
    {
        if (d->sessionList[i].ID == sessionID)
            return d->sessionList[i];
    }
    return d->emptySession;
}

UserSession::SessionData& UserSession::currentSession()
{
    Q_D(UserSession);
    int index = d->getActiveSession();
    if (index < 0)
        return d->emptySession;
    else
        return d->sessionList[index];
}

QList<QString> UserSession::sessionIdList()
{
    Q_D(UserSession);
    QList<QString> idList;
    for (int i=0; i<d->sessionList.count(); i++)
       idList.push_back(d->sessionList[i].ID);
    return idList;
}

UserSessionPrivate::UserSessionPrivate(UserSession *parent)
{
    this->q_ptr = parent;
}

UserSession::SessionData UserSessionPrivate::emptySession = {};

int UserSessionPrivate::getActiveSession()
{
    for (int i=0; i<sessionList.count(); i++)
    {
        if (sessionList[i].active)
            return i;
    }
    return -1;
}
