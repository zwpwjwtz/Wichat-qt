#ifndef PEERSESSION_H
#define PEERSESSION_H

#include "abstractsession.h"


class PeerSessionPrivate;

class PeerSession : public AbstractSession
{
    Q_DECLARE_PRIVATE(PeerSession)
protected:
    PeerSession(PeerSessionPrivate* d);

public:
    struct SessionKeyData : public AbstractSessionData
    {
        QString ID;
        QByteArray sendersKey;
        QByteArray receiversKey;
        qint64 updateTime;
    };

    explicit PeerSession();
    ~PeerSession();

    int count();
    bool exists(QString sessionID);
    void add(QString sessionID);
    void remove(QString sessionID);
    void removeAll();
    SessionKeyData& getSession(QString sessionID);
    bool loadSessionKey(QString userDir, QByteArray& keySalt);
    bool saveSessionKey(QString userDir, QByteArray& keySalt);
};

#endif // PEERSESSION_H
