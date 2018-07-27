#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "abstractservice.h"

class AccountPrivate;

class Account : public AbstractService
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Account)

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
    struct AccountListEntry
    {
        QString ID;
        OnlineState state;
    };
    struct AccountInfoEntry
    {
        QString ID;
        QString offlineMsg;
    };

    static const int MaxPasswordLen = 16;
    static const int MaxOfflineMsgLen = 64;
    static const int MaxRemarkLen = 16;

    explicit Account();
    explicit Account(ServerConnection& server);
    ~Account();

    static bool checkID(QString ID);
    static bool checkPassword(QString password);

    bool verify(QString ID, QString password);
    bool resetSession(int& queryID);

    QString ID();
    bool setPassword(QString oldPassword, QString newPassword);
    QByteArray sessionID();
    QByteArray sessionKey();
    OnlineState state();
    bool setState(OnlineState newState, int& queryID);
    QString offlineMsg();
    bool setOfflineMsg(QString newMessage, int& queryID);

    bool queryFriendList(int& queryID);
    bool addFriend(QString ID, int& queryID);
    bool removeFriend(QString ID, int& queryID);
    bool queryFriendRemarks(QList<QString> IDs, int& queryID);
    bool setFriendRemarks(QString ID, QString remarks, int& queryID);
    bool queryFriendInfo(QString ID, int& queryID);

signals:
    void queryError(int queryID, QueryError errorCode);
    void verifyFinished(VerifyError errorCode);
    void resetSessionFinished(int queryID, bool successful);
    void setPasswordFinished(int queryID, bool successful);
    void setStateFinished(int queryID, bool successful, OnlineState newState);
    void setOfflineMsgFinished(int queryID, bool successful);
    void queryFriendListFinished(int queryID, QList<AccountListEntry> friends);
    void addFriendFinished(int queryID, bool successful);
    void removeFriendFinished(int queryID, bool successful);
    void queryFriendRemarksFinished(int queryID, QList<QString> remarks);
    void setFriendRemarksFinished(int queryID, bool successful);
    void queryFriendInfoFinished(int queryID, QList<AccountInfoEntry> infos);
    void friendRequest(QString ID);
    void friendRemoved(QString ID);
};

#endif // ACCOUNT_H
