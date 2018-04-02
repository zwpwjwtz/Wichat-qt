#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>


class AccountPrivate;

class Account : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Account)
protected:
    AccountPrivate* d_ptr;

public:
    enum class VerifyError
    {
        Ok = 0,
        NetworkError = 1,
        VersionNotSupported = 2,
        IDFormatError = 3,
        PasswordFormatError = 4,
        VerificationFailed = 5,
        UnknownError = 255
    };
    enum class QueryType
    {
        None = 0,
        FriendListGet = 1,
        FriendRemarksGet = 2,
        FriendRemarksSet = 3,
        FriendInfoGet = 4
    };
    enum class QueryError
    {
        Ok = 0,
        NetworkError = 1,
        VersionNotSupported = 2,
        UnknownError = 255
    };
    enum class OnlineState
    {
        None = 0,
        Online = 1,
        Offline = 2,
        Busy = 4,
        Hide = 5,
    };
    struct FriendListEntry
    {
        QString ID;
        OnlineState state;
    };
    struct FriendInfoEntry
    {
        QString ID;
        QString remarks;
        QString offlineMsg;
    };

    static const int MaxIDLen = 8;
    static const int MaxPasswordLen = 16;
    static const int SessionLen = 16;
    static const int KeyLen = 16;
    static const int MaxOfflineMsgLen = 64;
    static const int MaxRemarkLen = 16;

    explicit Account();

    bool checkID(QString ID);
    bool checkPassword(QString password);
    VerifyError verify(QString ID, QString password);
    QString ID();
    bool setPassword(QString oldPassword, QString newPassword);
    bool resetSession();

    OnlineState state();
    bool setState(OnlineState newState);
    QString offlineMsg();
    bool setOfflineMsg(QString newMessage);

    bool queryFriendList();
    bool addFriend(QString ID);
    bool removeFriend(QString ID);
    bool queryFriendRemarks(QList<QString> IDs);
    bool setFriendRemarks(QString ID, QString remarks);
    bool queryFriendInfo(QString ID);

signals:
    bool queryFinished(int queryID, void* data);
    bool queryError(QueryError errorCode, int queryID);
};

#endif // ACCOUNT_H
