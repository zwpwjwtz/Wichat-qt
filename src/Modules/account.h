#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QDateTime>
#include "abstractservice.h"
#include "onlinestate.h"


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
    enum class GroupMemberRole
    {
        Default = 0,
        Administrator = 1,
        Super = 2
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
    struct GroupListEntry
    {
        QString ID;
    };
    struct GroupInfoEntry
    {
        QString ID;
        QString name;
        QString description;
        QDateTime creationTime;
        GroupMemberRole role;
        int type;
        int memberCount;
    };

    static const int MaxPasswordLen = 16;
    static const int MaxOfflineMsgLen = 64;
    static const int MaxRemarkLen = 16;

    explicit Account();
    explicit Account(ServerConnection& server);
    ~Account();

    static bool checkID(QString ID);
    static bool checkPassword(QString password);

    bool verify(QString ID, QString password, OnlineState loginState);
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

    bool getGroupList(int& queryID);
    bool joinGroup(QString groupID, int& queryID);
    bool quitGroup(QString groupID, int& queryID);
    bool addGroupMemeber(QString groupID, QString memberID, int& queryID);
    bool removeGroupMemeber(QString groupID, QString memberID, int& queryID);
    bool getGroupMemberList(QString groupID, int& queryID);
    bool getGroupNames(QList<QString>& groupIDList, int& queryID);
    bool getGroupInfo(QString groupID, int& queryID);
    bool setGroupName(QString groupID, QString name, int& queryID);
    bool setGroupDescription(QString groupID, QString text, int& queryID);
    bool deleteGroup(QString groupID, int& queryID);

signals:
    void queryError(int queryID, QueryError errorCode);
    void verifyFinished(VerifyError errorCode);
    void resetSessionFinished(int queryID, bool successful);
    void setPasswordFinished(int queryID, bool successful);
    void setStateFinished(int queryID,
                          bool successful,
                          OnlineState newState);
    void setOfflineMsgFinished(int queryID, bool successful);

    void queryFriendListFinished(int queryID,
                                 QList<Account::AccountListEntry> friends);
    void addFriendFinished(int queryID, bool successful);
    void removeFriendFinished(int queryID, bool successful);
    void queryFriendRemarksFinished(int queryID, QList<QString> remarks);
    void setFriendRemarksFinished(int queryID, bool successful);
    void queryFriendInfoFinished(int queryID,
                                 QList<Account::AccountInfoEntry> infos);
    void friendRequest(QString ID);
    void friendRemoved(QString ID);

    void getGroupListFinished(int queryID,
                              QList<Account::GroupListEntry>& groupList);
    void joinGroupFinished(int queryID, bool successful);
    void quitGroupFinished(int queryID, bool successful);
    void addGroupMemeberFinished(int queryID, bool successful);
    void removeGroupMemeberFinished(int queryID, bool successful);
    void getGroupMemberListFinished(int queryID,
                                    QList<Account::AccountListEntry>& members);
    void getGroupNameFinished(int queryID, QList<QString>& name);
    void getGroupInfoFinished(int queryID, Account::GroupInfoEntry& groupInfo);
    void setGroupNameFinished(int queryID, bool successful);
    void setGroupDescriptionFinished(int queryID, bool successful);
    void deleteGroupFinished(int queryID, bool successful);
};

#endif // ACCOUNT_H
