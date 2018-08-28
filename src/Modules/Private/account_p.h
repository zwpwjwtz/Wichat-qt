#ifndef ACCOUNT_P_H
#define ACCOUNT_P_H

#include <QObject>

#include "abstractservice_p.h"
#include "../account.h"


class AccountPrivate : public AbstractServicePrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Account)

public:
    class ServerObject : public AbstractEnum
    {
    public:
        static const int AccountLogin = 1;
        static const int AccountAction = 2;
        static const int FriendAction = 3;
        static const int GroupRelation = 4;
        static const int GroupAction = 5;
        inline ServerObject(const int initValue){value = initValue;}
    };

    class RequestType : public AbstractEnum
    {
    public:
        static const int None = 0;
        static const int ResetSession = 1;
        static const int SetPassword = 2;
        static const int SetState = 3;
        static const int SetOfflineMsg = 4;
        static const int GetFriendList = 5;
        static const int AddFriend = 6;
        static const int RemoveFriend = 7;
        static const int GetFriendRemarks = 8;
        static const int SetFriendRemarks = 9;
        static const int GetFriendInfo = 10;
        static const int Login = 11;
        static const int GetGroupList = 12;
        static const int JoinGroup = 13;
        static const int QuitGroup = 14;
        static const int AddGroupMember = 15;
        static const int RemoveGroupMember = 16;
        static const int GetGroupMember = 17;
        static const int GetGroupName = 18;
        static const int GetGroupInfo = 19;
        static const int SetGroupName = 20;
        static const int SetGroupDescription = 21;
        static const int DeleteGroup = 22;
        inline RequestType(const int initValue){value = initValue;}
    };

    QString loginID;
    QByteArray loginPassword;
    QByteArray loginKey;

    QString currentID;
    QByteArray currentSession;
    QByteArray sessionKey;
    qint64 sessionValidTime;
    OnlineState currentState;
    OnlineState expectedState;
    QString currentOfflineMsg;

    AccountPrivate(Account* parent = 0, ServerConnection* server = 0);

    virtual void dispatchQueryRespone(int requestID);
    bool processLogin(int requestID);
    bool processReplyData(RequestType type, QByteArray& data);

    static void parseAccountList(QByteArray& data,
                                 QByteArray listType,
                                 QList<Account::AccountListEntry>& list);
    static void parseGroupList(QByteArray& data,
                               QList<Account::GroupListEntry>& list);
    static OnlineState intToOnlineState(int var);
    static QString serverObjectToPath(ServerObject objectID);
};

#endif // ACCOUNT_P_H
