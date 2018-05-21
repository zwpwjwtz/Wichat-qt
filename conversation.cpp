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

#define WICHAT_SERVER_RESPONSE_RES_OK 0
#define WICHAT_SERVER_RESPONSE_RES_NOT_EXIST 1
#define WICHAT_SERVER_RESPONSE_RES_SIZE_TOO_LARGE 2
#define WICHAT_SERVER_RESPONSE_RES_EOF 3
#define WICHAT_SERVER_RESPONSE_RES_OUT_RANGE 4

#define WICHAT_SESSION_FILE_CACHE_DIR "cache"

#define WICHAT_CONVERS_EVENT_REQUEST_FINISHED 1
#define WICHAT_CONVERS_EVENT_SEND_FAILED 2
#define WICHAT_CONVERS_EVENT_RECEIVE_FAILED 3


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

QByteArray& Conversation::keySalt()
{
    Q_D(Conversation);
    return d->keySalt;
}

void Conversation::setUserDirectory(QString path)
{
    Q_D(Conversation);
    d->userDir = path;
}

void Conversation::setPeerSession(PeerSession& sessionList)
{
    Q_D(Conversation);
    d->sessionList = &sessionList;
}

bool Conversation::resetSession()
{
    // Currently not supported by server
    return false;
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
    transaction.currentMessageLength = 0;
    transaction.queryID = d->getAvailableQueryID();
    d->sendingList.enqueue(transaction);

    queryID = transaction.queryID;
    d->processSendList();
    return true;
}

bool Conversation::getMessageList()
{
    Q_D(Conversation);

    if (!d->loggedin)
        return false;

    int queryID;
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
    transaction.currentMessageLength = 0;
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

void Conversation::dispatchQueryRespone(int requestID)
{
    Q_D(Conversation);

    QByteArray data;
    bool successful;
    int requestIndex = d->getRequestIndexByID(requestID);
    ConversationPrivate::RequestType requestType =
                                            d->requestList[requestIndex].type;
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
            case ConversationPrivate::RequestType::Verify:
            {
                d->sessionKey = d->tempLoginKey;
                d->currentSession = d->tempSession;
                d->keySalt = data.left(RecordSaltLen);
                d->server->setSessionInfo(d->currentSession, d->sessionKey);
                d->loggedin = true;
                emit verifyFinished(requestID, VerifyError::Ok);
                break;
            }
            case ConversationPrivate::RequestType::ResetSession:
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
            case ConversationPrivate::RequestType::SendMessage:
            {
                ConversationPrivate::MessageTransaction* transaction =
                                        d->getTransactionByRequestID(requestID);
                if (!transaction)
                {
                    emit queryError(requestID, QueryError::UnknownError);
                    break;
                }
                if (transaction->pos + d->MaxMsgBlock <
                    transaction->data->length())
                {
                    // Continue to send the rest part of the message
                    transaction->pos += d->MaxMsgBlock;
                    d->processSendList();
                    break;
                }
                emit sendMessageFinished(transaction->queryID, successful);
                d->removeTransaction(transaction);
                break;
            }
            case ConversationPrivate::RequestType::GetMessageList:
            {
                QList<MessageListEntry> idList;
                d->parseAccountList(data, "v", idList);
                emit getMessageListFinished(requestID, idList);
                break;
            }
            case ConversationPrivate::RequestType::ReceiveMessage:
            {
                ConversationPrivate::MessageTransaction* transaction =
                                        d->getTransactionByRequestID(requestID);
                if (!transaction)
                {
                    emit queryError(requestID, QueryError::UnknownError);
                    d->removeTransaction(transaction);
                    break;
                }
                if (data[0] == char(WICHAT_SERVER_RESPONSE_RES_SIZE_TOO_LARGE))
                {
                    // Try to query resource with multiple request
                    if (transaction->multiPart)
                    {
                        emit queryError(requestID, QueryError::UnknownError);
                        d->removeTransaction(transaction);
                    }
                    else
                    {
                        transaction->multiPart = true;
                        d->processReceiveList();
                    }
                    break;
                }
                if (data[0] != char(WICHAT_SERVER_RESPONSE_RES_OK) &&
                    data[0] != char(WICHAT_SERVER_RESPONSE_RES_EOF))
                {
                    emit queryError(requestID, QueryError::UnknownError);
                    d->removeTransaction(transaction);
                    break;
                }

                int i, pos;
                int readLength;
                QList<QByteArray> tempList;
                d->parseMixedList(data, "SRC", tempList, &pos);
                pos += 1;
                if (tempList.length() > 0)
                {
                    // Append the remaining content to the last record
                    // Create a empty record if necessary
                    if (transaction->messages->count() > 0)
                    {
                        readLength = transaction->currentMessageLength -
                                    transaction->messages->last()
                                                         .content.length();
                        transaction->messages->last().content.append(
                                                    data.mid(pos, readLength));
                        pos += readLength;
                    }

                    // Then create new records for the rest content
                    QList<MessageEntry> newMessageList;
                    for (i=0; i<tempList.count(); i++)
                    {
                        newMessageList.push_back(MessageEntry());
                        newMessageList[i].source = tempList[i];
                        newMessageList[i].length = 0;
                    }

                    // Then parse records' time and length
                    d->parseMixedList(data, "TIME", tempList, &pos);
                    for (i=0; i<tempList.count(); i++)
                        newMessageList[i].time =
                                    QDateTime::fromString(QString(tempList[i]),
                                            WICHAT_SERVER_RECORD_TIME_FORMAT);
                    d->parseMixedList(data, "LEN", tempList, &pos);
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
                        pos += readLength;
                        if (pos >= data.length())
                            break;
                    }
                    transaction->messages->append(newMessageList);
                }
                else
                {
                    // Simply append content to the last record
                    transaction->messages->last().content.append(data.mid(pos));
                    transaction->currentMessageLength += data.length() - pos;

                }
                transaction->pos += data.length() - 1;
                if (transaction->multiPart &&
                    data[0] != char(WICHAT_SERVER_RESPONSE_RES_EOF))
                {
                    // Continue to receive the rest part of the message
                    d->processReceiveList();
                    break;
                }
                /*
                 * TODO: End-to-end encryption
                d->encoder.decrypt(Encryptor::Blowfish,
                                   data,
                                   d->sessionList->getSession(
                                            transaction->target).receiversKey,
                                   *transaction->data);
                */
                QString cacheDir(d->userDir);
                cacheDir.append('/').append(WICHAT_SESSION_FILE_CACHE_DIR);
                for (i=0; i<transaction->messages->count(); i++)
                {
                    d->dataUnxmlize((*transaction->messages)[i].content, data, cacheDir);
                    (*transaction->messages)[i].content = data;
                }
                emit receiveMessageFinished(transaction->queryID,
                                            *(transaction->messages));
                d->removeTransaction(transaction);
                break;
            }
            case ConversationPrivate::RequestType::FixConnection:
            {
                emit fixBrokenConnectionFinished(requestID, successful);
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


void Conversation::onPrivateEvent(int eventType, int data)
{
    Q_D(Conversation);

    switch (eventType)
    {
        case WICHAT_CONVERS_EVENT_REQUEST_FINISHED:
            if (d->getRequestIndexByID(data) >= 0)
                dispatchQueryRespone(data);
            break;
        default:;
    }
}

ConversationPrivate::ConversationPrivate(Conversation* parent,
                                         ServerConnection* server)
{
    this->q_ptr = parent;
    if (server)
        this->server = new RequestManager(*server);
    else
        this->server = new RequestManager;
    sessionList = nullptr;
    connect(this->server,
            SIGNAL(requestFinished(int)),
            this,
            SLOT(onRequestFinished(int)));
}

ConversationPrivate::~ConversationPrivate()
{
    delete this->server;
}

int ConversationPrivate::getRequestIndexByID(int requestID)
{
    for (int i=0; i<requestList.count(); i++)
    {
        if (requestList[i].ID == requestID)
            return i;
    }
    return -1;
}

void ConversationPrivate::addRequest(int requestID, RequestType type)
{
    RequestInfo request;
    request.ID = requestID;
    request.type = type;
    requestList.append(request);
}

bool ConversationPrivate::processReplyData(RequestType type, QByteArray& data)
{
    if (data.length() < 2)
        return false;
    switch (type)
    {
        case RequestType::ReceiveMessage:
            data.remove(0, 1);
            break;
        case RequestType::Verify:
        case RequestType::ResetSession:
        case RequestType::SendMessage:
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
    bufferIn.append(transaction.data->mid(transaction.pos, sendLength));

    if (server->sendData(bufferIn, bufferOut,
                         RequestManager::RecordServer,
                         serverObjectToPath(ServerObject::RecordAction),
                         false, &transaction.requestID)
            != RequestManager::Ok)
    {
        emit privateEvent(SendingFailed, transaction.queryID);
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

int ConversationPrivate::getAvailableQueryID()
{
    int queryID = 0;
    MessageTransaction* transaction;
    while (true)
    {
        queryID++;
        transaction = getTransactionByQueryID(queryID);
        if (transaction == nullptr)
            break;
    }
    return queryID;
}

ConversationPrivate::MessageTransaction*
ConversationPrivate::getTransactionByQueryID(int queryID)
{
    for (int i=0; i<sendingList.length(); i++)
    {
        if (sendingList[i].queryID == queryID)
            return &(sendingList[i]);
    }
    for (int i=0; i<receivingList.length(); i++)
    {
        if (receivingList[i].queryID == queryID)
            return &(receivingList[i]);
    }
    return nullptr;
}

ConversationPrivate::MessageTransaction*
ConversationPrivate::getTransactionByRequestID(int requestID)
{
    for (int i=0; i<sendingList.length(); i++)
    {
        if (sendingList[i].requestID == requestID)
            return &(sendingList[i]);
    }
    for (int i=0; i<receivingList.length(); i++)
    {
        if (receivingList[i].requestID == requestID)
            return &(receivingList[i]);
    }
    return nullptr;
}

void ConversationPrivate::removeTransaction(MessageTransaction* transaction)
{
    if (!transaction)
        return;

    if (transaction->data)
        delete transaction->data;
    if (transaction->messages)
        delete transaction->messages;

    int i;
    for (i=0; i< sendingList.count(); i++)
    {
        if (sendingList[i].queryID == transaction->queryID)
        {
            sendingList.removeAt(i);
            break;
        }
    }
    if (i < sendingList.count())
        return;

    for (i=0; i< receivingList.count(); i++)
    {
        if (receivingList[i].queryID == transaction->queryID)
        {
            receivingList.removeAt(i);
            break;
        }
    }
}

QByteArray ConversationPrivate::formatID(QString ID)
{
    return ID.leftJustified(Conversation::MaxIDLen, '\0', true).toLatin1();
}

void ConversationPrivate::dataXMLize(const QByteArray& src, QByteArray& dest)
{
    dest.clear();
    int p, p1, p2, p3, p4;
    QFile file;
    QString fileName;
    QFileInfo fileInfo;
    p = 0;
    dest.clear();

    while (true)
    {
        p1 = src.indexOf("<file>", p);
        p2 = src.indexOf("</file>", p1);
        if (p1 < 0 || p2 < 0)
            break;
        dest.append(src.mid(p, p1 - p)); // Previous data before <D>

        // Parse file name
        p3 = src.indexOf("<name>", p1);
        p4 = src.indexOf("</name>");
        if (p3 >= 0 && p4 >= 0)
            fileName = src.mid(p3 + 6, p4 - p3 - 6);
        else
            fileName.clear();
        file.setFileName(fileName); // File Info

        // Parse file type
        p3 = src.indexOf("<type>", p1);
        p4 = src.indexOf("</type>");
        if (p3 < 0 || p4 < 0)
        {
            // Incomplet file tag, skip it
            p = p2 + 7;
            continue;
        }
        switch (src.mid(p3 + 6, p4 - p3 - 6).at(0))
        {
            case 'i':   // Image file
                if (!file.exists())
                    dest.append("<D t=i l=-1 >");
                else
                {
                    dest.append("<D t=i l=")
                        .append(QString::number(file.size()))
                        .append(" >");
                    file.open(QFile::ReadOnly);
                    dest.append(file.readAll());
                    file.close();
                }
                dest.append("</D>");
                break;
            case 'f':   // Other types of file
            default:
                if (!file.exists())
                    dest.append("<D t=f l=-1 >");
                else
                {
                    fileInfo.setFile(file);
                    dest.append("<D t=f l=")
                        .append(QString::number(file.size()))
                        .append(" n=")
                        .append(fileInfo.fileName().toLatin1())
                        .append(char(0)).append(" >");
                    file.open(QFile::ReadOnly);
                    dest.append(file.readAll());
                    file.close();
                }
                dest.append("</D>");
        }
        p = p2 + 7;
    }
    dest.append(src.mid(p)); // Last data after </D>
}

void ConversationPrivate::dataUnxmlize(const QByteArray& src,
                                       QByteArray& dest,
                                       QString cacheDir)
{
    int p, p1, p2, p3;
    QFile file;
    QString fileName;
    qint64 fileLength;
    QByteArray fileContent;
    p = 0;
    dest.clear();

    while (true)
    {
        p1 = src.indexOf("<D t=", p);
        p2 = src.indexOf('>', p1);
        if (p1 < 0 || p2 < 0)
            break;
        dest.append(src.mid(p, p1 - p)); // Previous data before <D>

        // Parse file length
        p3 = src.indexOf("l=", p1);
        if (p3 > 0)
            fileLength = QString(src.mid(p3 + 2,
                                         src.indexOf(' ', p3 + 2) - p3 - 2))
                                .toInt();
        else
            fileLength = -1;
        if (fileLength <= 0)
        {
            // Incomplet file info, skip it
            p = p2 + 1;
            continue;
        }

        // Parse file name
        p3 = src.indexOf("n=", p1);
        if (p3 > 0 && p3 < p2)
            fileName = src.mid(p3 + 2, src.indexOf(' ', p3 + 2));
        else
            fileName.clear();

        QDir fileDir(cacheDir);
        if (!fileDir.exists())
            fileDir.mkpath(cacheDir);

        switch (src.at(p1 + 5))
        {
            case 'i':
                fileContent = src.mid(p2 + 1, fileLength);
                fileName = QString(encoder.getSHA256(fileContent).toHex())
                                  .prepend('/').prepend(cacheDir);
                file.setFileName(fileName);
                file.open(QFile::WriteOnly);
                file.write(fileContent);
                file.close();
                dest.append("<file><type>i</type><name>")
                    .append(fileName)
                    .append("</name></file>");
                break;
            case 'f':
            default:
                if (fileName.isEmpty())
                    break;
                file.setFileName(fileName.prepend('/').prepend(cacheDir));
                file.open(QFile::WriteOnly);
                file.write(src.mid(p2 + 1, fileLength));
                file.close();
                dest.append("<file><type>f</type><name>")
                    .append(fileName)
                    .append("</name></file>");
        }
        p = p2 + fileLength;
        p3 = src.indexOf("</D>", p);
        if (p3 < 0)
            break;
        else
            p = p3 + 4;
    }
    dest.append(src.mid(p));
}

void ConversationPrivate::parseAccountList(QByteArray& data,
                                           QByteArray listType,
                                   QList<Conversation::MessageListEntry>& list)
{
    int p1, p2, p3, pE;
    Conversation::MessageListEntry account;
    if (listType.isEmpty())
        p1 = data.indexOf("<IDList>");
    else
        p1 = data.indexOf(QByteArray("<IDList t=")
                          .append(listType)
                          .append(">"));
    pE = data.indexOf("</IDList>");
    if (p1 >= 0 && pE > 16)
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
            list.append(account);
        }
    }
}

void ConversationPrivate::parseMixedList(QByteArray& data,
                                         QByteArray fieldName,
                                         QList<QByteArray>& list,
                                         int* parsedLength)
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
        if (parsedLength)
            *parsedLength = pE - data.indexOf("<MList>") + 9;
    }
    else
    {
        if (parsedLength)
            *parsedLength = 0;
    }
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

void ConversationPrivate::onRequestFinished(int requestID)
{
    emit privateEvent(WICHAT_CONVERS_EVENT_REQUEST_FINISHED, requestID);
}
