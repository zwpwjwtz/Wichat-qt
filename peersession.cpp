#include <QDir>

#include "peersession.h"
#include "peersessionprivate.h"
#include "common.h"

#define WICHAT_SESSION_KEY_FILENAME "session2.dat"


PeerSession::PeerSession()
{
    this->d_ptr = new PeerSessionPrivate(this);
}

PeerSession::PeerSession(PeerSessionPrivate* d)
{
    this->d_ptr = d;
}

PeerSession::SessionKeyData PeerSessionPrivate::emptyKey = {};

int PeerSession::count()
{
    Q_D(PeerSession);
    return d->sessionKeyList.count();
}

bool PeerSession::exists(QString sessionID)
{
    Q_D(PeerSession);
    for (int i=0; i<d->sessionKeyList.count(); i++)
        if (d->sessionKeyList[i].ID == sessionID)
            return true;
    return false;
}

void PeerSession::add(QString sessionID)
{
    Q_D(PeerSession);
    if (sessionID.isEmpty())
        return;
    SessionKeyData newSession;
    newSession.ID = sessionID;
    d->sessionKeyList.push_back(newSession);
}

void PeerSession::remove(QString sessionID)
{
    Q_D(PeerSession);
    for (int i=0; i<d->sessionKeyList.count(); i++)
    {
        if (d->sessionKeyList[i].ID == sessionID)
        {
            d->sessionKeyList.removeAt(i);
            i--;
        }
    }
}

void PeerSession::removeAll()
{
    Q_D(PeerSession);
    d->sessionKeyList.clear();
}

PeerSession::SessionKeyData& PeerSession::getSession(QString sessionID)
{
    Q_D(PeerSession);
    for (int i=0; i<d->sessionKeyList.count(); i++)
    {
        if (d->sessionKeyList[i].ID == sessionID)
            return d->sessionKeyList[i];
    }
    return d->emptyKey;
}

PeerSessionPrivate::PeerSessionPrivate(PeerSession *parent)
{
    this->q_ptr = parent;
}

bool PeerSession::loadSessionKey(QString userDir, QByteArray& keySalt)
{
    Q_D(PeerSession);
    QFile sessionFile(d->getSessionFile(userDir,
                                        WICHAT_SESSION_KEY_FILENAME));
    if (sessionFile.isReadable())
        return false;

    QByteArray plain;
    bool ok = d->enc.decrypt(Encryptor::Blowfish,
                             sessionFile.readAll(),
                             keySalt,
                             plain);
    if (!ok)
        return false;

    int p1 = 0, p2 = 0;
    SessionKeyData newSession;
    while (p2 < plain.length() - 1)
    {
        p2 = plain.indexOf('\0', p1);
        newSession.ID = plain.mid(p1, p2 - p1);
        p2 = plain.indexOf('\0', p1);
        newSession.sendsKey = plain.mid(p1, p2 - p1);
        p2 = plain.indexOf('\0', p1);
        newSession.receiversKey = plain.mid(p1, p2 - p1);
        p2 = plain.indexOf('\0', p1);
        newSession.updateTime = plain.mid(p1, p2 - p1).toInt(&ok);

        if (newSession.ID.isEmpty() ||
            newSession.sendsKey.isEmpty() ||
            newSession.receiversKey.isEmpty() ||
            !ok)
        {
            ok = false;
            break;
        }
        d->sessionKeyList.push_back(newSession);
    }
    sessionFile.close();
    return ok;
}

bool PeerSession::saveSessionKey(QString userDir, QByteArray& keySalt)
{
    Q_D(PeerSession);
    QFile sessionFile(d->getSessionFile(userDir,
                                        WICHAT_SESSION_KEY_FILENAME));
    if (sessionFile.isWritable())
        return false;

    QByteArray plain;
    for (int i=0; i<d->sessionKeyList.count(); i++)
        plain.append(d->sessionKeyList[i].ID).append('\0')
             .append(d->sessionKeyList[i].sendsKey).append('\0')
             .append(d->sessionKeyList[i].receiversKey).append('\0')
             .append(d->sessionKeyList[i].updateTime).append('\0');
    plain.append('\0');

    QByteArray cipher;
    bool ok = d->enc.encrypt(Encryptor::Blowfish,
                             plain,
                             keySalt,
                             cipher);
    if (ok)
        sessionFile.write(cipher);
    sessionFile.close();
    return ok;
}
