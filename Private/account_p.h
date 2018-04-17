#ifndef ACCOUNT_P_H
#define ACCOUNT_P_H

#include <QObject>

#include "account.h"
#include "encryptor.h"
#include "requestmanager.h"


class AccountPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Account)
protected:
    Account* q_ptr;

public:
    enum class ServerObject
    {
        AccountLogin = 1,
        AccountAction = 2,
        FriendAction = 3
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
        RequestFinished = 1
    };

    QString currentID;
    QByteArray currentSession;
    QByteArray sessionKey;
    qint64 sessionValidTime;
    Account::OnlineState currentState;
    QString currentOfflineMsg;
    Encryptor encoder;
    RequestManager server;

    AccountPrivate(Account* parent = 0);
    void parseAccountList(QByteArray& data,
                          QByteArray listType,
                          QList<Account::AccountListEntry>& list);
    void parseMixedList(QByteArray& data,
                        QByteArray fieldName,
                        QList<QByteArray>& list);
    bool processReplyData(RequestType type, QByteArray& data);
    QByteArray formatID(QString ID);
    Account::OnlineState intToOnlineState(int var);
    QString serverObjectToPath(ServerObject objectID);

signals:
    void privateEvent(PrivateEventType eventType, int data);

protected slots:
    void onRequestFinished(int requestID);
};

#endif // ACCOUNT_P_H
