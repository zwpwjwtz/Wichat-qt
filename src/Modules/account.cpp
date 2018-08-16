#include "common.h"
#include "account.h"
#include "Private/account_p.h"

#define WICHAT_SERVER_PATH_ACCOUNT_LOGIN "/Account/log/login.php"
#define WICHAT_SERVER_PATH_ACCOUNT_ACTION "/Account/acc/action.php"
#define WICHAT_SERVER_PATH_ACCOUNT_FRIEND "/Account/acc/friend.php"
#define WICHAT_SERVER_PATH_GROUP_RELATION "/Account/group/relation.php"
#define WICHAT_SERVER_PATH_GROUP_ACTION "/Account/group/action.php"

#define WICHAT_SERVER_ACCOUNT_TIME_FORMAT "yyyy/MM/dd,HH:mm:ss"

#define WICHAT_ACCOUNT_STATE_DEFAULT 0
#define WICHAT_ACCOUNT_STATE_ONLINE 1
#define WICHAT_ACCOUNT_STATE_OFFLINE 2
//#define WICHAT_ACCOUNT_STATE_LOGOUT 3
#define WICHAT_ACCOUNT_STATE_BUSY 4
#define WICHAT_ACCOUNT_STATE_HIDE 5

#define WICHAT_RELATION_STATE_NONE 0
#define WICHAT_RELATION_STATE_WAITING 1
#define WICHAT_RELATION_STATE_ESTABLISHED 2
#define WICHAT_RELATION_STATE_BREAKING 3
//#define WICHAT_RELATION_STATE_REFUSED 4

#define WICHAT_ACCOUNT_INFO_SESSION_CHANGE 5
#define WICHAT_ACCOUNT_INFO_MSG_GET 6
#define WICHAT_ACCOUNT_INFO_MSG_SET 7
#define WICHAT_ACCOUNT_INFO_STATE_SET 8
#define WICHAT_ACCOUNT_INFO_PW_CHANGE 9

#define WICHAT_ACCOUNT_FRIEND_GETLIST 1
#define WICHAT_ACCOUNT_FRIEND_ADD 2
#define WICHAT_ACCOUNT_FRIEND_DEL 3
#define WICHAT_ACCOUNT_FRIEND_CHECK 4
#define WICHAT_ACCOUNT_FRIEND_GETINFO 10
#define WICHAT_ACCOUNT_FRIEND_GETNOTE 11
#define WICHAT_ACCOUNT_FRIEND_SETNOTE 12

#define WICHAT_ACCOUNT_FRIEND_OPTION_NORMAL 0
#define WICHAT_ACCOUNT_FRIEND_OPTION_GETDATE 1
#define WICHAT_ACCOUNT_FRIEND_OPTION_GETSTATE 2

#define WICHAT_ACCOUNT_GROUP_ADD_MEMBER 1
#define WICHAT_ACCOUNT_GROUP_DEL_MEMBER 2
#define WICHAT_ACCOUNT_GROUP_GET_MEMBER 3
#define WICHAT_ACCOUNT_GROUP_GET_NAME 4
#define WICHAT_ACCOUNT_GROUP_GET_INFO 5
#define WICHAT_ACCOUNT_GROUP_SET_NAME 6
#define WICHAT_ACCOUNT_GROUP_SET_DESCRIP 7
#define WICHAT_ACCOUNT_GROUP_DEL_GROUP 8

#define WICHAT_ACCOUNT_RESPONSE_PASSWORD_OK 1
#define WICHAT_ACCOUNT_RESPONSE_PASSWORD_INCORRECT 2
#define WICHAT_ACCOUNT_RESPONSE_GROUP_OK 0
#define WICHAT_ACCOUNT_RESPONSE_ACCOUNT_NOT_EXIST 3
#define WICHAT_ACCOUNT_RESPONSE_GROUP_NOT_EXIST 4
#define WICHAT_ACCOUNT_RESPONSE_GROUP_CANNOT_JOIN 5
#define WICHAT_ACCOUNT_RESPONSE_GROUP_NOT_MEMEBER 6
#define WICHAT_ACCOUNT_RESPONSE_GROUP_ALREADY_MEMEBER 7
#define WICHAT_ACCOUNT_RESPONSE_GROUP_NO_PERMISSION 8


Account::Account()
{
    this->d_ptr = new AccountPrivate(this);
    connect(d_ptr,
            SIGNAL(privateEvent(int, int)),
            this,
            SLOT(onPrivateEvent(int, int)));
}

Account::Account(ServerConnection& server)
{
    this->d_ptr = new AccountPrivate(this, &server);
    connect(d_ptr,
            SIGNAL(privateEvent(int, int)),
            this,
            SLOT(onPrivateEvent(int, int)));
}

Account::~Account()
{
    delete this->d_ptr;
}

bool Account::checkID(QString ID)
{
    int length = ID.length();
    if (length < 1 || length > MaxIDLen)
        return false;
    for (int i=0; i<length; i++)
    {
        char ch = ID[i].toLatin1();
        if (ch < '0' || ch > '9')
            return false;
    }
    return true;
}

bool Account::checkPassword(QString password)
{
    int length = password.length();
    if (length < 1 || length > MaxPasswordLen)
        return false;
    for (int i=0; i<length; i++)
    {
        char ch = password[i].toLatin1();
        if (!((ch >= '0' && ch <= '9') ||
              (ch >= 'A' && ch <= 'Z') ||
              (ch >= 'a' && ch <= 'z')))
            return false;
    }
    return true;
}

bool Account::verify(QString ID, QString password)
{
    Q_D(Account);
    if (!checkID(ID))
        emit verifyFinished(VerifyError::IDFormatError);
    if (!checkPassword(password))
        emit verifyFinished(VerifyError::PasswordFormatError);

    d->loginID = ID;
    d->loginPassword = d->encoder.getSHA256(password.toLatin1());
    return d->processLogin(0);
}

QString Account::ID()
{
    Q_D(Account);
    return d->currentID;
}

QByteArray Account::sessionID()
{
    Q_D(Account);
    return d->currentSession;
}

QByteArray Account::sessionKey()
{
    Q_D(Account);
    return d->sessionKey;
}

bool Account::setPassword(QString oldPassword, QString newPassword)
{
    Q_D(Account);
    if (!checkPassword(oldPassword) || !checkPassword(newPassword))
        return false;

    int queryID;
    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(WICHAT_ACCOUNT_INFO_PW_CHANGE))
            .append(char(qrand() * 256));
    bufferIn.append(oldPassword.toLatin1()).append(char(0));
    bufferIn.append(newPassword.toLatin1()).append(char(0));
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::AccountAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::SetPassword);
    return true;
}

bool Account::resetSession(int& queryID)
{
    Q_D(Account);
    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(WICHAT_ACCOUNT_INFO_SESSION_CHANGE))
            .append(char(qrand() * 256));
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::AccountAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::ResetSession);
    return true;
}

Account::OnlineState Account::state()
{
    Q_D(Account);
    return d->currentState;
}

bool Account::setState(OnlineState newState, int& queryID)
{
    Q_D(Account);
    if (d->currentState == OnlineState::None)
        return false;

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(WICHAT_ACCOUNT_INFO_STATE_SET))
            .append(char(newState));
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::AccountAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::SetState);
    return true;
}

QString Account::offlineMsg()
{
    Q_D(Account);
    return d->currentOfflineMsg;
}

bool Account::setOfflineMsg(QString newMessage, int &queryID)
{
    Q_D(Account);
    newMessage.truncate(MaxOfflineMsgLen - 2);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(WICHAT_ACCOUNT_INFO_MSG_SET))
            .append(char(qrand() * 256));
    bufferIn.append("<MSG>")
            .append(newMessage.toUtf8())
            .append("</MSG>");
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::AccountAction),
                            false, &queryID))
        return false;

    d->currentOfflineMsg = newMessage;

    d->addRequest(queryID, AccountPrivate::RequestType::SetOfflineMsg);
    return true;
}

bool Account::queryFriendList(int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(WICHAT_ACCOUNT_FRIEND_GETLIST))
            .append(char(WICHAT_ACCOUNT_FRIEND_OPTION_GETSTATE));
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::FriendAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::GetFriendList);
    return true;
}

bool Account::addFriend(QString ID, int &queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(WICHAT_ACCOUNT_FRIEND_ADD))
            .append(char(qrand() * 256));
    bufferIn.append("<IDList><ID>")
            .append(d->formatID(ID))
            .append("</ID></IDList>");
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                  AccountPrivate::ServerObject::FriendAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::AddFriend);
    return true;
}

bool Account::removeFriend(QString ID, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(WICHAT_ACCOUNT_FRIEND_DEL))
            .append(char(qrand() * 256));
    bufferIn.append("<IDList><ID>")
            .append(d->formatID(ID))
            .append("</ID></IDList>");
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::FriendAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::RemoveFriend);
    return true;
}

bool Account::queryFriendRemarks(QList<QString> IDs, int &queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(WICHAT_ACCOUNT_FRIEND_GETNOTE))
            .append(char(qrand() * 256));
    bufferIn.append("<IDList>");
    for (int i=0; i<IDs.count(); i++)
    {
        bufferIn.append("<ID>")
                .append(d->formatID(IDs[i]))
                .append("</ID>");
    }
    bufferIn.append("</IDList>");
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::FriendAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::GetFriendRemarks);
    return true;
}

bool Account::setFriendRemarks(QString ID, QString remarks, int &queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    qint16 remarksLength = remarks.toUtf8().length() + 1;
    bufferIn.append(char(WICHAT_ACCOUNT_FRIEND_SETNOTE))
            .append(char(qrand() * 256));
    bufferIn.append(d->formatID(ID))
            .append(QByteArray::fromRawData((char*)(&remarksLength), 2))
            .append(remarks.toUtf8()).append(char(0));
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::FriendAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::SetFriendRemarks);
    return true;
}

bool Account::queryFriendInfo(QString ID, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(WICHAT_ACCOUNT_FRIEND_GETINFO))
            .append(char(WICHAT_ACCOUNT_FRIEND_OPTION_GETDATE));
    bufferIn.append("<IDList><ID>")
            .append(d->formatID(ID))
            .append("</ID></IDList>");
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::FriendAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::GetFriendInfo);
    return true;
}

bool Account::getGroupList(int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(1)).append(char(qrand()));
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupRelation),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::GetGroupList);
    return true;
}

bool Account::joinGroup(QString groupID, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(2)).append(char(qrand()));
    bufferIn.append(d->formatID(groupID));
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupRelation),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::JoinGroup);
    return true;
}

bool Account::quitGroup(QString groupID, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(3)).append(char(qrand()));
    bufferIn.append(d->formatID(groupID));
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupRelation),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::QuitGroup);
    return true;
}

bool Account::addGroupMemeber(QString groupID, QString memberID, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(1)).append(char(qrand()));
    bufferIn.append(d->formatID(groupID));
    bufferIn.append("<IDList><ID>")
            .append(d->formatID(memberID))
            .append("</ID></IDList>");
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupAction),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::AddGroupMember);
    return true;
}

bool Account::removeGroupMemeber(QString groupID, QString memberID,int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(2)).append(char(qrand()));
    bufferIn.append(d->formatID(groupID));
    bufferIn.append("<IDList><ID>")
            .append(d->formatID(memberID))
            .append("</ID></IDList>");
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupAction),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::RemoveGroupMember);
    return true;
}

bool Account::getGroupMemberList(QString groupID, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(3)).append(char(qrand()));
    bufferIn.append(d->formatID(groupID));
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupAction),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::GetGroupMember);
    return true;
}

bool Account::getGroupNames(QList<QString>& groupIDList, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(4)).append(char(qrand()));
    bufferIn.append(QByteArray(MaxIDLen, '0'));
    bufferIn.append("<IDList>");
    for (int i=0; i<groupIDList.count(); i++)
        bufferIn.append("<ID>").append(groupIDList[i]).append("</ID>");
    bufferIn.append("</IDList>");
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupAction),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::GetGroupName);
    return true;
}

bool Account::getGroupInfo(QString groupID, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(5)).append(char(qrand()));
    bufferIn.append(d->formatID(groupID));
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupAction),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::GetGroupInfo);
    return true;
}

bool Account::setGroupName(QString groupID, QString name, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(6)).append(char(qrand()));
    bufferIn.append(d->formatID(groupID)).append(name.toUtf8());
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupAction),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::SetGroupName);
    return true;
}

bool Account::setGroupDescription(QString groupID, QString text, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(7)).append(char(qrand()));
    bufferIn.append(d->formatID(groupID)).append(text.toUtf8());
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupAction),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::SetGroupDescription);
    return true;
}

bool Account::deleteGroup(QString groupID, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(8)).append(char(qrand()));
    bufferIn.append(d->formatID(groupID));
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               AccountPrivate::ServerObject::GroupAction),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, AccountPrivate::RequestType::DeleteGroup);
    return true;
}

AccountPrivate::AccountPrivate(Account *parent, ServerConnection *server) :
    AbstractServicePrivate(parent, server){}

void AccountPrivate::dispatchQueryRespone(int requestID)
{
    Q_Q(Account);

    bool successful;
    QByteArray data;
    static QList<QByteArray> tempList;
    static QList<Account::AccountListEntry> idList;

    int requestIndex = getRequestIndexByID(requestID);
    RequestType requestType(requestList[requestIndex].type);
    if (requestType == RequestType::Login)
    {
        processLogin(requestID);
        requestList.removeAt(requestIndex);
        return;
    }

    RequestManager::RequestError errCode = server->getData(requestID, data);
    if (errCode == RequestManager::Ok)
    {
        successful = processReplyData(requestType, data);
        switch (requestType)
        {
            // For some operations, the first byte (indicating if
            // it is successful or not) has already been checked in
            // processReplyData(), and no further checking is needed
            // So we simply pass the value of "successful" to these signals
            case AccountPrivate::RequestType::ResetSession:
            {
                successful &= data.length() > Account::KeyLen;
                if (successful)
                {
                    currentSession = data.left(Account::SessionLen);
                    sessionKey = data.mid(Account::SessionLen, Account::KeyLen);
                    server->setSessionInfo(currentSession, sessionKey);
                }
                emit q->resetSessionFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::SetPassword:
            {
                emit q->setPasswordFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::SetState:
            {
                if (successful)
                    currentState = Account::OnlineState(data.at(0));
                emit q->setStateFinished(requestID,
                                         successful,
                                         currentState);
                break;
            }
            case AccountPrivate::RequestType::SetOfflineMsg:
            {
                emit q->setOfflineMsgFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::GetFriendList:
            {
                parseAccountList(data, "b", idList);
                for (int i=0; i<idList.count(); i++)
                    emit q->friendRemoved(idList[i].ID);
                parseAccountList(data, "w", idList);
                for (int i=0; i<idList.count(); i++)
                    emit q->friendRequest(idList[i].ID);
                parseAccountList(data, "c", idList);
                emit q->queryFriendListFinished(requestID, idList);
                break;
            }
            case AccountPrivate::RequestType::AddFriend:
            {
                parseAccountList(data, "f", idList);
                successful &= idList.isEmpty();
                emit q->addFriendFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::RemoveFriend:
            {
                parseAccountList(data, "f", idList);
                successful &= idList.isEmpty();
                emit q->removeFriendFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::GetFriendRemarks:
            {
                QList<QString> remarkList;
                parseMixedList(data, "NOTE", tempList);
                for (int i=0; i<tempList.count(); i++)
                    remarkList.push_back(tempList[i]);
                emit q->queryFriendRemarksFinished(requestID, remarkList);
                break;
            }
            case AccountPrivate::RequestType::SetFriendRemarks:
            {
                emit q->setFriendRemarksFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::GetFriendInfo:
            {
                QList<Account::AccountInfoEntry> accountInfoList;
                Account::AccountInfoEntry accountInfo;
                parseMixedList(data, "ID", tempList);
                for (int i=0; i<tempList.count(); i++)
                {
                    accountInfo.ID = tempList[i];
                    accountInfoList.push_back(accountInfo);
                }
                parseMixedList(data, "MSG", tempList);
                for (int i=0; i<tempList.count(); i++)
                {
                    accountInfoList[i].offlineMsg = tempList[i];
                }
                emit q->queryFriendInfoFinished(requestID, accountInfoList);
                break;
            }
            case AccountPrivate::RequestType::GetGroupList:
            {
                QList<Account::GroupListEntry> groupList;
                parseGroupList(data, groupList);
                emit q->getGroupListFinished(requestID, groupList);
                break;
            }
            case AccountPrivate::RequestType::AddGroupMember:
            {
                parseAccountList(data, "f", idList);
                successful &= idList.isEmpty();
                emit q->addGroupMemeberFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::RemoveGroupMember:
            {
                parseAccountList(data, "f", idList);
                successful &= idList.isEmpty();
                emit q->removeGroupMemeberFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::GetGroupMember:
            {
                parseAccountList(data, "", idList);
                emit q->getGroupMemberListFinished(requestID, idList);
                break;
            }
            case AccountPrivate::RequestType::GetGroupName:
            {
                QList<QString> nameList;
                parseMixedList(data, "NAME", tempList);
                for (int i=0; i<tempList.count(); i++)
                    nameList.push_back(tempList[i]);
                emit q->getGroupNameFinished(requestID, nameList);
                break;
            }
            case AccountPrivate::RequestType::GetGroupInfo:
            {
                Account::GroupInfoEntry groupInfo;
                parseMixedList(data, "ID", tempList);
                groupInfo.ID = tempList[0];
                parseMixedList(data, "COUNT", tempList);
                groupInfo.memberCount = tempList[0].toInt();
                parseMixedList(data, "TIME", tempList);
                groupInfo.creationTime = QDateTime::fromString(tempList[0],
                                            WICHAT_SERVER_ACCOUNT_TIME_FORMAT);
                parseMixedList(data, "NAME", tempList);
                groupInfo.name = QString::fromUtf8(tempList[0]);
                parseMixedList(data, "DESC", tempList);
                groupInfo.description = QString::fromUtf8(tempList[0]);
                parseMixedList(data, "ROLE", tempList);
                groupInfo.role = Account::GroupMemberRole(tempList[0].toInt());
                emit q->getGroupInfoFinished(requestID, groupInfo);
                break;
            }
            case AccountPrivate::RequestType::SetGroupName:
            {
                emit q->setGroupNameFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::SetGroupDescription:
            {
                emit q->setGroupDescriptionFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::DeleteGroup:
            {
                emit q->deleteGroupFinished(requestID, successful);
                break;
            }
            default:
                emit q->queryError(requestID,
                                   Account::QueryError::UnknownError);
        }
    }
    else if (errCode == RequestManager::VersionTooOld)
        emit q->queryError(requestID, Account::QueryError::VersionNotSupported);
    else if (errCode == RequestManager::CannotConnect)
        emit q->queryError(requestID, Account::QueryError::NetworkError);
    else
        emit q->queryError(requestID, Account::QueryError::UnknownError);

    requestList.removeAt(requestIndex);
}

bool AccountPrivate::processLogin(int requestID)
{
    Q_Q(Account);

    static int loginState = 0;
    QByteArray bufferIn, bufferOut;
    QByteArray tempKey;
    RequestManager::RequestError errCode;

    if (loginState == 0 || loginState == 3) // Not logged in | logged in
    {
    // Build pre-login request
    tempKey = encoder.getHMAC(loginPassword, loginID.toLatin1())
                           .left(Account::KeyLen);
    loginKey = encoder.genKey(Account::KeyLen);

    encoder.encrypt(Encryptor::AES,
                    loginKey,
                    tempKey,
                    bufferOut);
    bufferIn.append(char(WICHAT_CLIENT_DEVICE)).append(char(1));
    bufferIn.append(encoder.fuse(formatID(loginID), bufferOut));
    bufferIn.append(bufferOut);

    // Send pre-login reques
    if (server->sendRawData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            serverObjectToPath(ServerObject::AccountLogin),
                            false, &requestID)
             != RequestManager::Ok)
         return false;

    addRequest(requestID, RequestType::Login);
    loginState = 1;
    }
    else if (loginState == 1) // Stage 1 finished
    {
    loginState = 0;
    errCode = server->getData(requestID, bufferOut);
    if (errCode == RequestManager::CannotConnect)
        emit q->verifyFinished(Account::VerifyError::NetworkError);
    if (errCode == RequestManager::VersionTooOld)
        emit q->verifyFinished(Account::VerifyError::VersionNotSupported);
    if (errCode != RequestManager::Ok)
        return false;

    if (bufferOut.length() < Account::KeyLen)
    {
        emit q->verifyFinished(Account::VerifyError::VerificationFailed);
        return false;
    }

    // Extract pre-encryption key
    bufferOut.remove(0, 1);
    tempKey = encoder.getHMAC(loginPassword, loginID.toLatin1())
                           .left(Account::KeyLen);
    encoder.decrypt(Encryptor::AES,
                    bufferOut,
                    tempKey,
                    bufferIn);
    loginKey = encoder.byteXOR(loginKey, bufferIn);


    // Build login request
    bufferIn.clear();
    bufferIn.append(loginPassword.left(Account::KeyLen));
    bufferIn.append(char(Account::OnlineState::Online));
    encoder.encrypt(Encryptor::AES, bufferIn, loginKey, bufferOut);
    bufferIn.clear();
    bufferIn.append(char(WICHAT_CLIENT_DEVICE)).append(char(2));
    bufferIn.append(encoder.getHMAC(formatID(loginID), loginKey)
                           .left(Account::MaxIDLen));
    bufferIn.append(bufferOut);

    // Send login request
    if (server->sendRawData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            serverObjectToPath(ServerObject::AccountLogin),
                            false, &requestID)
            != RequestManager::Ok)
        return false;

    addRequest(requestID, RequestType::Login);
    loginState = 2;
    }
    else if (loginState == 2) // Stage 2 finished
    {
    loginState = 0;
    errCode = server->getData(requestID, bufferOut);
    if (errCode == RequestManager::CannotConnect)
    {
        emit q->verifyFinished(Account::VerifyError::NetworkError);
        return false;
    }
    if (errCode != RequestManager::Ok || bufferOut.length() < Account::KeyLen)
    {
        emit q->verifyFinished(Account::VerifyError::VerificationFailed);
        return false;
    }

    encoder.decrypt(Encryptor::AES,
                    bufferOut.mid(1 + Account::SessionLen),
                    loginKey,
                    bufferIn);
    if (bufferIn.left(4) != encoder.getCRC32(bufferIn.mid(4)))
        emit q->verifyFinished(Account::VerifyError::UnknownError);

    // Extract session and account information
    currentSession = bufferOut.mid(1, Account::SessionLen);
    bufferIn.remove(0, 4);
    currentState = intToOnlineState(bufferIn.at(1));
    sessionValidTime = *((qint16*)(bufferIn.mid(2, 2).data()));
    sessionKey = bufferIn.mid(4, Account::KeyLen);
    currentOfflineMsg = QString(bufferIn.mid(4 + Account::KeyLen)).trimmed();
    currentID = loginID;

    server->setSessionInfo(currentSession, sessionKey);
    loginState = 3;
    emit q->verifyFinished(Account::VerifyError::Ok);
    }

    return true;
}

bool AccountPrivate::processReplyData(RequestType type, QByteArray& data)
{
    if (data.length() < 2)
        return false;
    switch (type)
    {
        case AccountPrivate::RequestType::SetPassword:
            if (data[1] != char(WICHAT_ACCOUNT_RESPONSE_PASSWORD_OK))
                return false;
        case AccountPrivate::RequestType::SetState:
            data.remove(0, 1);
            break;
        case AccountPrivate::RequestType::ResetSession:
        case AccountPrivate::RequestType::SetOfflineMsg:
        case AccountPrivate::RequestType::GetFriendList:
        case AccountPrivate::RequestType::AddFriend:
        case AccountPrivate::RequestType::RemoveFriend:
        case AccountPrivate::RequestType::GetFriendRemarks:
        case AccountPrivate::RequestType::SetFriendRemarks:
        case AccountPrivate::RequestType::GetFriendInfo:
            data.remove(0, 2);
            break;
        default:;
    }
    return true;
}

void AccountPrivate::parseAccountList(QByteArray& data,
                                      QByteArray listType,
                                      QList<Account::AccountListEntry>& list)
{
    int p1, p2, p3, pS, pE;
    unsigned char accountState;
    Account::AccountListEntry account;

    list.clear();
    if (listType.isEmpty())
        p1 = data.indexOf("<IDList>");
    else
        p1 = data.indexOf(QByteArray("<IDList t=")
                          .append(listType)
                          .append(">"));
    pE = data.indexOf("</IDList>", p1);
    if (p1 >= 0 && pE >= 0)
    {
        p3 = p2 = p1;
        while (true)
        {
            p1 = data.indexOf("<ID", p1 + 1);
            p2 = data.indexOf(">", p1 + 1);
            p3 = data.indexOf("</ID>", p2 + 1);
            if (p1 < 0 || p1 < 0 || p1 < 0 || p1 > pE)
                break;

            account.ID = QString(data.mid(p2 + 1, p3 - p2 - 1)).trimmed();
            pS = data.indexOf("s=", p1);
            if (pS + 2 < p2)
                accountState = data.at(pS + 2) - '0';
            else
                accountState = 0;
            account.state = Account::OnlineState(accountState);
            list.append(account);
        }
    }
}

void AccountPrivate::parseGroupList(QByteArray& data,
                                    QList<Account::GroupListEntry>& list)
{
    int p1, p2, p3, pE;
    Account::GroupListEntry group;

    list.clear();
    p1 = data.indexOf("<IDList>");
    pE = data.indexOf("</IDList>", p1);
    if (p1 >= 0 && pE >= 0)
    {
        p3 = p2 = p1;
        while (true)
        {
            p1 = data.indexOf("<ID", p1 + 1);
            p2 = data.indexOf(">", p1 + 1);
            p3 = data.indexOf("</ID>", p2 + 1);
            if (p1 < 0 || p1 < 0 || p1 < 0 || p1 > pE)
                break;

            group.ID = QString(data.mid(p2 + 1, p3 - p2 - 1)).trimmed();
            list.append(group);
        }
    }
}

Account::OnlineState AccountPrivate::intToOnlineState(int var)
{
    switch (var)
    {
        case WICHAT_ACCOUNT_STATE_ONLINE:
            return Account::OnlineState::Online;
        case WICHAT_ACCOUNT_STATE_OFFLINE:
            return Account::OnlineState::Offline;
        case WICHAT_ACCOUNT_STATE_BUSY:
            return Account::OnlineState::Busy;
        case WICHAT_ACCOUNT_STATE_HIDE:
            return Account::OnlineState::Hide;
        default:
            return Account::OnlineState::None;
    }
}

QString AccountPrivate::serverObjectToPath(ServerObject objectID)
{
    switch (objectID)
    {
        case ServerObject::AccountLogin:
            return WICHAT_SERVER_PATH_ACCOUNT_LOGIN;
            break;
        case ServerObject::AccountAction:
            return WICHAT_SERVER_PATH_ACCOUNT_ACTION;
            break;
        case ServerObject::FriendAction:
            return WICHAT_SERVER_PATH_ACCOUNT_FRIEND;
            break;
        case ServerObject::GroupRelation:
            return WICHAT_SERVER_PATH_GROUP_RELATION;
            break;
        case ServerObject::GroupAction:
            return WICHAT_SERVER_PATH_GROUP_ACTION;
            break;
        default:
            return "";
    }
}
