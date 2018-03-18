#ifndef ACCOUNT_P_H
#define ACCOUNT_P_H

#include <QObject>

#include "account.h"
#include "encryptor.h"
#include "serverconnection.h"


class AccountPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Account)
protected:
    Account* q_ptr;

public:
    enum ServerObject
    {
        Action_Acc_Change = 1,
        Action_Fri_Change = 2,
        Action_Msg_Exchange = 3,
        Action_Msg_Get_List = 4,
        Action_Msg_Get_Key = 5,
    };

    QString currentID;
    QByteArray currentSession;
    QByteArray sessionKey;
    qint64 sessionValidTime;
    Account::OnlineState currentState;
    QString currentOfflineMsg;

    QString keySalt;
    Encryptor encoder;
    ServerConnection server;

    AccountPrivate(Account* parent = 0);
    bool exchangeData(const QByteArray& data,
                 QByteArray& result,
                 ServerObject object);
    QByteArray formatID(QString ID);
    Account::OnlineState intToOnlineState(int var);
};

#endif // ACCOUNT_P_H
