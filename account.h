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
    bool resetSession(int& queryID);

    QString ID();
    bool setPassword(QString oldPassword, QString newPassword);
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

private:
    void dispatchQueryRespone(int queryID);

private slots:
    void onPrivateEvent(int eventType, int data);
};

#endif // ACCOUNT_H
