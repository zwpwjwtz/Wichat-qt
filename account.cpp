#include "common.h"
#include "account.h"
#include "Private/account_p.h"

#define WICHAT_SERVER_PATH_ACCOUNT_LOGIN "/Account/log/login.php"
#define WICHAT_SERVER_PATH_ACCOUNT_ACTION "/Account/acc/action.php"

#define WICHAT_ACCOUNT_STATE_DEFAULT 0
#define WICHAT_ACCOUNT_STATE_ONLINE 1
#define WICHAT_ACCOUNT_STATE_OFFLINE 2
//#define WICHAT_ACCOUNT_STATE_LOGOUT 3
#define WICHAT_ACCOUNT_STATE_BUSY 4
#define WICHAT_ACCOUNT_STATE_HIDE 5

#define WICHAT_ACCOUNT_PASSWORD_OK 1
#define WICHAT_ACCOUNT_PASSWORD_INCORRECT 1

#define WICHAT_RELATION_STATE_NONE 0
#define WICHAT_RELATION_STATE_WAITING 1
#define WICHAT_RELATION_STATE_ESTABLISHED 2
#define WICHAT_RELATION_STATE_BREAKING 3
//#define WICHAT_RELATION_STATE_REFUSED 4

#define WICHAT_ACCOUNT_EVENT_REQUEST_FINISHED 1


Account::Account()
{
    this->d_ptr = new AccountPrivate(this);
    connect(d_ptr,
            SIGNAL(privateEvent(int, int)),
            this,
            SLOT(onPrivateEvent(int, int)));
}

Account::Account(ServerConnection &server)
{
    this->d_ptr = new AccountPrivate(this, &server);
    connect(d_ptr,
            SIGNAL(privateEvent(int, int)),
            this,
            SLOT(onPrivateEvent(int, int)));
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
    bufferIn.append(char(9)).append(char(qrand() * 256));
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
    bufferIn.append(char(5)).append(char(qrand() * 256));
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
    bufferIn.append(char(8)).append(char(newState));
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
    bufferIn.append(char(7)).append(char(qrand() * 256));
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
    bufferIn.append(char(1)).append(char(2));
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::AccountAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::GetFriendList);
    return true;
}

bool Account::addFriend(QString ID, int &queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(2)).append(char(qrand() * 256));
    bufferIn.append("<IDList><ID>")
            .append(d->formatID(ID))
            .append("</ID></IDList>");
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                  AccountPrivate::ServerObject::AccountAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::AddFriend);
    return true;
}

bool Account::removeFriend(QString ID, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(3)).append(char(qrand() * 256));
    bufferIn.append("<IDList><ID>")
            .append(d->formatID(ID))
            .append("</ID></IDList>");
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::AccountAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::RemoveFriend);
    return true;
}

bool Account::queryFriendRemarks(QList<QString> IDs, int &queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(11)).append(char(qrand() * 256));
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
                                AccountPrivate::ServerObject::AccountAction),
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
    bufferIn.append(char(12)).append(char(qrand() * 256));
    bufferIn.append(d->formatID(ID))
            .append(QByteArray::fromRawData((char*)(&remarksLength), 2))
            .append(remarks.toUtf8()).append(char(0));
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::AccountAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::SetFriendRemarks);
    return true;
}

bool Account::queryFriendInfo(QString ID, int& queryID)
{
    Q_D(Account);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(10)).append(char(1));
    bufferIn.append("<IDList><ID>")
            .append(d->formatID(ID))
            .append("</ID></IDList>");
    if (RequestManager::Ok !=
        d->server->sendData(bufferIn, bufferOut,
                            RequestManager::AccountServer,
                            d->serverObjectToPath(
                                AccountPrivate::ServerObject::AccountAction),
                            false, &queryID))
        return false;

    d->addRequest(queryID, AccountPrivate::RequestType::GetFriendInfo);
    return true;
}

void Account::dispatchQueryRespone(int requestID)
{
    Q_D(Account);

    bool successful;
    QByteArray data;
    QList<QByteArray> tempList;

    int requestIndex = d->getRequestIndexByID(requestID);
    AccountPrivate::RequestType requestType = d->requestList[requestIndex].type;
    if (requestType == AccountPrivate::RequestType::Login)
    {
        d->processLogin(requestID);
        d->requestList.removeAt(requestIndex);
        return;
    }

    RequestManager::RequestError errCode = d->server->getData(requestID, data);
    if (errCode == RequestManager::Ok)
    {
        successful = d->processReplyData(requestType, data);
        switch (requestType)
        {
            // For some operations, the first byte (indicating if
            // it is successful or not) has already been checked in
            // d->processReplyData(), and no further checking is needed
            // So we simply pass the value of "successful" to these signals
            case AccountPrivate::RequestType::ResetSession:
            {
                successful &= data.length() > KeyLen;
                if (successful)
                {
                    d->currentSession = data.left(SessionLen);
                    d->sessionKey = data.mid(SessionLen, KeyLen);
                    d->server->setSessionInfo(d->currentSession, d->sessionKey);
                }
                emit resetSessionFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::SetPassword:
            {
                emit setPasswordFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::SetState:
            {
                if (successful)
                    d->currentState = OnlineState(data.at(0));
                emit setStateFinished(requestID, successful, d->currentState);
                break;
            }
            case AccountPrivate::RequestType::SetOfflineMsg:
            {
                emit setOfflineMsgFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::GetFriendList:
            {
                QList<AccountListEntry> idList;
                d->parseAccountList(data, "b", idList);
                for (int i=0; i<idList.count(); i++)
                    emit friendRemoved(idList[i].ID);
                d->parseAccountList(data, "w", idList);
                for (int i=0; i<idList.count(); i++)
                    emit friendRequest(idList[i].ID);
                d->parseAccountList(data, "c", idList);
                emit queryFriendListFinished(requestID, idList);
                break;
            }
            case AccountPrivate::RequestType::AddFriend:
            {
                QList<AccountListEntry> idList;
                d->parseAccountList(data, "f", idList);
                successful &= idList.isEmpty();
                emit addFriendFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::RemoveFriend:
            {
                QList<AccountListEntry> idList;
                d->parseAccountList(data, "f", idList);
                successful &= idList.isEmpty();
                emit removeFriendFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::GetFriendRemarks:
            {
                QList<QString> remarkList;
                d->parseMixedList(data, "NOTE", tempList);
                for (int i=0; i<tempList.count(); i++)
                    remarkList.push_back(tempList[i]);
                emit queryFriendRemarksFinished(requestID, remarkList);
                break;
            }
            case AccountPrivate::RequestType::SetFriendRemarks:
            {
                emit setFriendRemarksFinished(requestID, successful);
                break;
            }
            case AccountPrivate::RequestType::GetFriendInfo:
            {
                QList<AccountInfoEntry> accountInfoList;
                AccountInfoEntry accountInfo;
                d->parseMixedList(data, "ID", tempList);
                for (int i=0; i<tempList.count(); i++)
                {
                    accountInfo.ID = tempList[i];
                    accountInfoList.push_back(accountInfo);
                }
                d->parseMixedList(data, "MSG", tempList);
                for (int i=0; i<tempList.count(); i++)
                {
                    accountInfoList[i].offlineMsg = tempList[i];
                }
                emit queryFriendInfoFinished(requestID, accountInfoList);
                break;
            }
            default:
                emit queryError(requestID, QueryError::UnknownError);
        }
    }
    else if (errCode == RequestManager::VersionTooOld)
        emit queryError(requestID, QueryError::VersionNotSupported);
    else if (errCode == RequestManager::CannotConnect)
        emit queryError(requestID, QueryError::NetworkError);
    else
        emit queryError(requestID, QueryError::UnknownError);

    d->requestList.removeAt(requestIndex);
}

void Account::onPrivateEvent(int eventType, int data)
{
    Q_D(Account);

    switch (eventType)
    {
        case WICHAT_ACCOUNT_EVENT_REQUEST_FINISHED:
            if (d->getRequestIndexByID(data) >= 0)
                dispatchQueryRespone(data);
            break;
        default:;
    }
}

AccountPrivate::AccountPrivate(Account* parent, ServerConnection* server)
{
    this->q_ptr = parent;
    if (server)
        this->server = new RequestManager(*server);
    else
        this->server = new RequestManager;
    connect(this->server,
            SIGNAL(requestFinished(int)),
            this,
            SLOT(onRequestFinished(int)));
}

AccountPrivate::~AccountPrivate()
{
    delete this->server;
}

int AccountPrivate::getRequestIndexByID(int requestID)
{
    for (int i=0; i<requestList.count(); i++)
    {
        if (requestList[i].ID == requestID)
            return i;
    }
    return -1;
}

void AccountPrivate::addRequest(int requestID, RequestType type)
{
    RequestInfo request;
    request.ID = requestID;
    request.type = type;
    requestList.append(request);
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
                            serverObjectToPath(
                                AccountPrivate::ServerObject::AccountLogin),
                            false, &requestID)
             != RequestManager::Ok)
         return false;

    addRequest(requestID, AccountPrivate::RequestType::Login);
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
                            serverObjectToPath(
                                AccountPrivate::ServerObject::AccountLogin),
                            false, &requestID)
            != RequestManager::Ok)
        return false;

    addRequest(requestID, AccountPrivate::RequestType::Login);
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
        case RequestType::SetPassword:
            if (data[1] != char(WICHAT_ACCOUNT_PASSWORD_OK))
                return false;
        case RequestType::SetState:
            data.remove(0, 1);
            break;
        case RequestType::ResetSession:
        case RequestType::SetOfflineMsg:
        case RequestType::GetFriendList:
        case RequestType::AddFriend:
        case RequestType::RemoveFriend:
        case RequestType::GetFriendRemarks:
        case RequestType::SetFriendRemarks:
        case RequestType::GetFriendInfo:
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

void AccountPrivate::parseMixedList(QByteArray& data,
                                    QByteArray fieldName,
                                    QList<QByteArray>& list)
{
    int p1, p2, pE, length;
    QByteArray fieldBegin, fieldEnd;
    p1 = data.indexOf("<MList>");
    pE = data.indexOf("</MList>");
    fieldBegin.append('<').append(fieldName).append('>');
    fieldEnd.append("</").append(fieldName).append('>');
    list.clear();
    if (p1 >= 0 && pE > 13)
    {
        p2 = p1;
        while (true)
        {
            p1 = data.indexOf(fieldBegin, p1 + 1);
            p2 = data.indexOf(fieldEnd, p2 + 1);
            if (p1 < 0 || p2 < 0)
                break;

            length = p2 - p1 - fieldBegin.length();
            if (length > 0)
                list.append(data.mid(p1 + fieldBegin.length(), length));
            else
                list.append(QByteArray());
        }
    }
}

QByteArray AccountPrivate::formatID(QString ID)
{
    return ID.leftJustified(Account::MaxIDLen, '\0', true).toLatin1();
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
        case ServerObject::FriendAction:
            return WICHAT_SERVER_PATH_ACCOUNT_ACTION;
            break;
        default:
            return "";
    }
}

void AccountPrivate::onRequestFinished(int requestID)
{
    emit privateEvent(WICHAT_ACCOUNT_EVENT_REQUEST_FINISHED, requestID);
}
