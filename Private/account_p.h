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
        Action_Acc_Action = 1,
        Action_Fri_Action = 2,
        Action_Msg_Exchange = 3,
        Action_Msg_Get_List = 4,
        Action_Msg_Get_Key = 5,
    };

    enum class RequestType
    {
        None = 0,
        ResetSession = 1,
        SetPassword = 2,
        SetState = 3,
        SetOfflineMsg = 4,
        GetFriendList = 5,
        AddFriend = 6,
        RemoveFriend = 7,
        GetFriendRemarks = 8,
        SetFriendRemarks = 9,
        GetFriendInfo = 10
    };

    enum PrivateEventType
    {
        requestFinished = 1
    };

    struct RequestRecord
    {
        RequestType type;
        int requestID;
    };

    QString currentID;
    QByteArray currentSession;
    QByteArray sessionKey;
    qint64 sessionValidTime;
    Account::OnlineState currentState;
    QString currentOfflineMsg;

    Encryptor encoder;
    ServerConnection server;
    QList<RequestRecord> requestList;

    AccountPrivate(Account* parent = 0);
    bool exchangeData(const QByteArray& data,
                      QByteArray& result,
                      ServerObject object,
                      bool synchronous = true,
                      int* requestID = nullptr);
    void addRequestRecord(RequestType type, int requestID);
    void removeRequestRecord(int requestID);
    bool processReplyData(RequestType type, QByteArray& data);
    void parseAccountList(QByteArray& data,
                          QByteArray listType,
                          QList<Account::AccountListEntry>& list);
    void parseMixedList(QByteArray& data,
                        QByteArray fieldName,
                        QList<QByteArray>& list);
    QByteArray formatID(QString ID);
    Account::OnlineState intToOnlineState(int var);

signals:
    void privateEvent(PrivateEventType eventType, int data);

private slots:
    void onRequestFinished(int requestID);
};

#endif // ACCOUNT_P_H
