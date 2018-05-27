#ifndef PEERSESSIONPRIVATE_H
#define PEERSESSIONPRIVATE_H

#include "abstractsession_p.h"
#include "../peersession.h"
#include "../encryptor.h"


class PeerSessionPrivate : public AbstractSessionPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(PeerSession)
protected:
    PeerSession* q_ptr;
    static PeerSession::SessionKeyData emptyKey;

public:
    Encryptor enc;
    QList<PeerSession::SessionKeyData> sessionKeyList;
    explicit PeerSessionPrivate(PeerSession* parent);
};

#endif // PEERSESSIONPRIVATE_H
