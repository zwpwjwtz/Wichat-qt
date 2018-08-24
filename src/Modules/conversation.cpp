#include <QFile>
#include <QDir>
#include "common.h"
#include "conversation.h"
#include "Private/conversation_p.h"

#define WICHAT_SERVER_PATH_RECORD_LOGIN "/Record/log/login.php"
#define WICHAT_SERVER_PATH_RECORD_ACTION "/Record/query/action.php"
#define WICHAT_SERVER_PATH_RECORD_GET "/Record/query/get.php"

#define WICHAT_SERVER_RECORD_OBJID_SERVER "10000"
#define WICHAT_SERVER_RECORD_TIME_FORMAT "yyyy/MM/dd,HH:mm:ss"
#define WICHAT_SERVER_RECORD_ID_LEN 16

#define WICHAT_SERVER_RESPONSE_RES_OK 0
#define WICHAT_SERVER_RESPONSE_RES_NOT_EXIST 1
#define WICHAT_SERVER_RESPONSE_RES_SIZE_TOO_LARGE 2
#define WICHAT_SERVER_RESPONSE_RES_EOF 3
#define WICHAT_SERVER_RESPONSE_RES_OUT_RANGE 4

#define WICHAT_SESSION_FILE_CACHE_DIR "cache"


Conversation::Conversation()
{
    this->d_ptr = new ConversationPrivate(this);
    connect(d_ptr,
            SIGNAL(privateEvent(int, int)),
            this,
            SLOT(onPrivateEvent(int, int)));
}

Conversation::Conversation(ServerConnection &server)
{
    this->d_ptr = new ConversationPrivate(this, &server);
    connect(d_ptr,
            SIGNAL(privateEvent(int, int)),
            this,
            SLOT(onPrivateEvent(int, int)));
}

Conversation::~Conversation()
{
    delete this->d_ptr;
}

bool Conversation::verify(QByteArray sessionID, QByteArray sessionKey)
{
    Q_D(Conversation);

    if (sessionID.length() != SessionLen)
    {
        emit verifyFinished(-1, VerifyError::SessionFormatError);
        return false;
    }
    if (sessionKey.length() != KeyLen)
    {
        emit verifyFinished(-1, VerifyError::KeyFormatError);
        return false;
    }
    if (d->loggedin)
    {
        emit verifyFinished(-1, VerifyError::Ok);
        return true;
    }

    int queryID;
    QByteArray bufferIn, bufferOut;
    d->server->setSessionInfo(sessionID, sessionKey);
    d->tempLoginKey = d->encoder.genKey(KeyLen);
    bufferIn.append(d->tempLoginKey);
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                                ConversationPrivate::ServerObject::RecordLogin),
                            false, &queryID)
                == RequestManager::CannotConnect)
        return false;

    d->tempLoginKey = d->encoder.getHMAC(sessionKey, d->tempLoginKey)
                                .left(KeyLen);
    d->tempSession = sessionID;
    d->addRequest(queryID, ConversationPrivate::RequestType::Verify);
    return true;
}

bool Conversation::sendMessage(QString ID, QByteArray &content, int& queryID)
{
    Q_D(Conversation);

    if (!d->loggedin || !d->sessionList)
        return false;

    // Create new session key if no previous session is found
    if (!d->sessionList->exists(ID))
    {
        d->sessionList->add(ID);
        d->sessionList->getSession(ID).sendersKey = d->encoder.genKey(KeyLen);
    }

    QByteArray* bufferIn = new QByteArray;
    QByteArray temp;
    d->dataXMLize(content, temp);
    /*
     * TODO: End-to-end encryption
    d->encoder.decrypt(Encryptor::Blowfish,
    d->encoder.encrypt(Encryptor::Blowfish,
                       temp,
                       d->sessionList->getSession(ID).sendersKey,
                       *bufferIn);
    */
    bufferIn->append(temp);

    // TODO: Reset sending key positively & randomly

    ConversationPrivate::MessageTransaction transaction;
    transaction.target = ID;
    transaction.data = bufferIn;
    transaction.messages = nullptr;
    transaction.pos = 0;
    transaction.queryID = d->getAvailableQueryID();
    d->sendingList.enqueue(transaction);

    queryID = transaction.queryID;
    d->processSendList();
    return true;
}

bool Conversation::getMessageList(int& queryID)
{
    Q_D(Conversation);

    if (!d->loggedin)
        return false;

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(1)).append(char(qrand()));
    if (d->server->sendData(bufferIn, bufferOut,
                            RequestManager::RecordServer,
                            d->serverObjectToPath(
                               ConversationPrivate::ServerObject::RecordGet),
                            false, &queryID)
            == RequestManager::CannotConnect)
    return false;

    d->addRequest(queryID, ConversationPrivate::RequestType::GetMessageList);
    return true;
}

bool Conversation::receiveMessage(QString ID, int& queryID)
{
    Q_D(Conversation);

    if (!d->loggedin || !d->sessionList)
        return false;

    ConversationPrivate::MessageTransaction transaction;
    transaction.target = ID;
    transaction.data = nullptr;
    transaction.messages = new QList<MessageEntry>;
    transaction.pos = 0;
    transaction.multiPart = false;
    transaction.queryID = d->getAvailableQueryID();
    d->receivingList.enqueue(transaction);

    queryID = transaction.queryID;
    d->processReceiveList();
    return true;
}

bool Conversation::fixBrokenConnection(QString ID, int& queryID)
{
    Q_D(Conversation);

    if (!d->loggedin || !d->sessionList)
        return false;

    qint32 sendLength = 32;
    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(0)).append(char(2));
    bufferIn.append(d->formatID(ID));
    bufferIn.append(QByteArray::fromRawData((const char*)(&sendLength), 4));
    bufferIn.append(char(1)).append(char(0));
    bufferIn.append(char(127)).append(char(255))
            .append(char(127)).append(char(255))
            .append(char(WICHAT_CLIENT_DEVICE)).append(char(4));
    for (int i=0; i<26; i++)
        bufferIn.append(char(qrand()));

    if (d->server->sendData(bufferIn, bufferOut,
                        RequestManager::RecordServer,
                        d->serverObjectToPath(
                            ConversationPrivate::ServerObject::RecordAction),
                        false, &queryID)
            != RequestManager::Ok)
        return false;

    d->addRequest(queryID, ConversationPrivate::RequestType::FixConnection);
    return true;
}

void ConversationPrivate::dispatchQueryRespone(int requestID)
{
    Q_Q(Conversation);

    QByteArray data;
    bool successful;
    int requestIndex = getRequestIndexByID(requestID);
    ConversationPrivate::RequestType requestType(requestList[requestIndex].type);
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
            case ConversationPrivate::RequestType::Verify:
            {
                sessionKey = tempLoginKey;
                currentSession = tempSession;
                keySalt = data.left(Conversation::RecordSaltLen);
                server->setSessionInfo(currentSession, sessionKey);
                loggedin = true;
                emit q->verifyFinished(requestID, Conversation::VerifyError::Ok);
                break;
            }
            case ConversationPrivate::RequestType::ResetSession:
            {
                successful &= data.length() > Conversation::KeyLen;
                if (successful)
                {
                    currentSession = data.left(Conversation::SessionLen);
                    sessionKey = data.mid(Conversation::SessionLen,
                                          Conversation::KeyLen);
                    server->setSessionInfo(currentSession, sessionKey);
                }
                emit q->resetSessionFinished(requestID, successful);
                break;
            }
            case ConversationPrivate::RequestType::SendMessage:
            {
                ConversationPrivate::MessageTransaction* transaction =
                                        getTransactionByRequestID(requestID);
                if (!transaction)
                {
                    emit q->queryError(requestID, Conversation::QueryError::UnknownError);
                    break;
                }
                if (data[0] != char(WICHAT_SERVER_RESPONSE_RES_OK))
                {
                    emit q->queryError(requestID,
                                       Conversation::QueryError::UnknownError);
                    removeTransaction(transaction);
                    break;
                }
                if (transaction->pos == 0)
                {
                    // Store message ID assigned by server
                    transaction->messageID =
                                    data.mid(1, WICHAT_SERVER_RECORD_ID_LEN);
                }
                if (transaction->pos + MaxMsgBlock <
                    transaction->data->length())
                {
                    // Continue to send the rest part of the message
                    transaction->pos += MaxMsgBlock;
                    processSendList();
                    break;
                }
                emit q->sendMessageFinished(transaction->queryID, successful);
                removeTransaction(transaction);
                break;
            }
            case ConversationPrivate::RequestType::GetMessageList:
            {
                QList<Conversation::MessageListEntry> idList;
                parseMessageList(data, "v", idList);
                emit q->getMessageListFinished(requestID, idList);
                break;
            }
            case ConversationPrivate::RequestType::ReceiveMessage:
            {
                ConversationPrivate::MessageTransaction* transaction =
                                        getTransactionByRequestID(requestID);
                if (!transaction)
                {
                    emit q->queryError(requestID, Conversation::QueryError::UnknownError);
                    removeTransaction(transaction);
                    break;
                }
                if (data[0] == char(WICHAT_SERVER_RESPONSE_RES_SIZE_TOO_LARGE))
                {
                    // Try to query resource with multiple request
                    if (transaction->multiPart)
                    {
                        emit q->queryError(requestID, Conversation::QueryError::UnknownError);
                        removeTransaction(transaction);
                    }
                    else
                    {
                        transaction->multiPart = true;
                        processReceiveList();
                    }
                    break;
                }
                if (data[0] != char(WICHAT_SERVER_RESPONSE_RES_OK) &&
                    data[0] != char(WICHAT_SERVER_RESPONSE_RES_EOF))
                {
                    emit q->queryError(requestID, Conversation::QueryError::UnknownError);
                    removeTransaction(transaction);
                    break;
                }

                int i, pos = 1;
                int parsedLength;
                int readLength;
                QList<QByteArray> tempList;
                parseMixedList(data, "SRC", tempList, &parsedLength);
                pos += parsedLength;
                if (tempList.length() > 0)
                {
                    // Append the remaining content to the last record
                    // Create a empty record if necessary
                    if (transaction->messages->count() > 0)
                    {
                        readLength = transaction->messages->last().length -
                                    transaction->messages->last()
                                                         .content.length();
                        transaction->messages->last().content.append(
                                                    data.mid(pos, readLength));
                        transaction->pos += readLength;
                        pos += readLength;
                    }

                    // Then create new records for the rest content
                    QList<Conversation::MessageEntry> newMessageList;
                    for (i=0; i<tempList.count(); i++)
                    {
                        newMessageList.push_back(Conversation::MessageEntry());
                        newMessageList[i].source = tempList[i];
                        newMessageList[i].length = 0;
                    }

                    // Then parse records' time and length
                    parseMixedList(data, "TIME", tempList, &parsedLength);
                    for (i=0; i<tempList.count(); i++)
                        newMessageList[i].time =
                                    QDateTime::fromString(QString(tempList[i]),
                                            WICHAT_SERVER_RECORD_TIME_FORMAT);
                    parseMixedList(data, "LEN", tempList, &parsedLength);
                    for (i=0; i<tempList.count(); i++)
                        newMessageList[i].length = QString(tempList[i]).toInt();

                    // Finally parse records' content
                    for (i=0; i<newMessageList.count(); i++)
                    {
                        if (pos + newMessageList[i].length >= data.length())
                            readLength = data.length() - pos;
                        else
                            readLength = newMessageList[i].length;
                        newMessageList[i].content = data.mid(pos, readLength);
                        transaction->pos += readLength;
                        pos += readLength;
                    }
                    transaction->messages->append(newMessageList);
                }
                else
                {
                    // Simply append content to the last record
                    if (transaction->messages->count() > 0)
                        transaction->messages->last().content.append(
                                                                data.mid(pos));
                    transaction->pos += data.length() - pos;
                }
                if (transaction->multiPart &&
                    data[0] != char(WICHAT_SERVER_RESPONSE_RES_EOF))
                {
                    // Continue to receive the rest part of the message
                    processReceiveList();
                    break;
                }
                /*
                 * TODO: End-to-end encryption
                encoder.decrypt(Encryptor::Blowfish,
                                   data,
                                   sessionList->getSession(
                                            transaction->target).receiversKey,
                                   *transaction->data);
                */
                QString cacheDir(userDir);
                cacheDir.append('/').append(WICHAT_SESSION_FILE_CACHE_DIR);
                for (i=0; i<transaction->messages->count(); i++)
                {
                    dataUnxmlize((*transaction->messages)[i].content, data, cacheDir);
                    (*transaction->messages)[i].content = data;
                }
                emit q->receiveMessageFinished(transaction->queryID,
                                            *(transaction->messages));
                removeTransaction(transaction);
                break;
            }
            case ConversationPrivate::RequestType::FixConnection:
            {
                emit q->fixBrokenConnectionFinished(requestID, successful);
                break;
            }
            default:
                emit q->queryError(requestID,
                                   Conversation::QueryError::UnknownError);
        }
    }
    else if (errCode == RequestManager::VersionTooOld)
        emit q->queryError(requestID, Conversation::QueryError::VersionNotSupported);
    else if (errCode == RequestManager::CannotConnect)
        emit q->queryError(requestID, Conversation::QueryError::NetworkError);
    else
        emit q->queryError(requestID, Conversation::QueryError::UnknownError);

    requestList.removeAt(requestIndex);
}

ConversationPrivate::ConversationPrivate(Conversation* parent,
                                         ServerConnection* server) :
    AbstractChatPrivate(parent, server){}

bool ConversationPrivate::processReplyData(RequestType type, QByteArray& data)
{
    if (data.length() < 2)
        return false;
    switch (type)
    {
        case RequestType::SendMessage:
        case RequestType::ReceiveMessage:
            data.remove(0, 1);
            break;
        case RequestType::Verify:
        case RequestType::ResetSession:
        case RequestType::GetMessageList:
        case RequestType::FixConnection:
            data.remove(0, 2);
            break;
        default:;
    }
    return true;
}

bool ConversationPrivate::processSendList()
{
    if (sendingList.isEmpty())
        return false;

    QByteArray bufferIn, bufferOut;
    MessageTransaction& transaction = sendingList.first();
    qint32 dataLength = transaction.data->length();
    qint32 sendLength;
    bool isEOF;
    if (dataLength < MaxMsgBlock)
    {
        transaction.multiPart = false;
        sendLength = dataLength;
        isEOF = false;
    }
    else
    {
        transaction.multiPart = true;
        if (dataLength > transaction.pos + MaxMsgBlock)
        {
            sendLength = MaxMsgBlock;
            isEOF = false;
        }
        else
        {
            sendLength = dataLength - transaction.pos;
            isEOF = true;
        }
    }
    if (transaction.pos == 0)
        transaction.messageID = QByteArray::fromRawData("\x00", 1)
                                        .repeated(WICHAT_SERVER_RECORD_ID_LEN);
    bufferIn.append(char(2)).append(char(2));
    bufferIn.append(formatID(transaction.target));
    bufferIn.append(QByteArray::fromRawData((const char*)(&sendLength), 4));
    if (isEOF)
        bufferIn.append(char(1));
    else
        bufferIn.append(char(0));
    if (transaction.multiPart)
        bufferIn.append(char(1));
    else
        bufferIn.append(char(0));
    bufferIn.append(transaction.messageID);
    bufferIn.append(transaction.data->mid(transaction.pos, sendLength));

    if (server->sendData(bufferIn, bufferOut,
                         RequestManager::RecordServer,
                         serverObjectToPath(ServerObject::RecordAction),
                         false, &transaction.requestID)
            != RequestManager::Ok)
    {
        emit privateEvent(ConversationPrivate::SendingFailed, transaction.queryID);
        removeTransaction(&transaction);
        return false;
    }
    addRequest(transaction.requestID, RequestType::SendMessage);
    return true;
}

bool ConversationPrivate::processReceiveList()
{
    if (receivingList.isEmpty())
        return false;

    qint32 receiveLength = MaxMsgBlock;
    QByteArray bufferIn, bufferOut;
    MessageTransaction& transaction = receivingList.first();
    bufferIn.append(char(1));
    if (transaction.target == WICHAT_SERVER_RECORD_OBJID_SERVER)
        bufferIn.append(char(1));
    else
        bufferIn.append(char(2));
    bufferIn.append(formatID(transaction.target));
    bufferIn.append(QByteArray::fromRawData((const char*)(&receiveLength), 4));
    if (transaction.multiPart)
        bufferIn.append(char(0)).append(char(transaction.pos / MaxMsgBlock));
    else
        bufferIn.append(char(1)).append(char(0));

    if (server->sendData(bufferIn, bufferOut,
                         RequestManager::RecordServer,
                         serverObjectToPath(ServerObject::RecordAction),
                         false, &transaction.requestID)
            != RequestManager::Ok)
    {
        emit privateEvent(ReceivingFailed, transaction.queryID);
        removeTransaction(&transaction);
        return false;
    }
    addRequest(transaction.requestID, RequestType::ReceiveMessage);
    return true;
}

QString ConversationPrivate::serverObjectToPath(ServerObject objectID)
{
    switch (objectID)
    {
        case ServerObject::RecordLogin:
            return WICHAT_SERVER_PATH_RECORD_LOGIN;
            break;
        case ServerObject::RecordGet:
            return WICHAT_SERVER_PATH_RECORD_GET;
            break;
        case ServerObject::RecordAction:
            return WICHAT_SERVER_PATH_RECORD_ACTION;
            break;
        default:
            return "";
    }
}
