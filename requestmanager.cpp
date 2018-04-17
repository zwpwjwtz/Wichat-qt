#include "requestmanager.h"
#include "Private/requestmanager_p.h"


RequestManager::RequestManager()
{
    this->d_ptr = new RequestManagerPrivate(this);
    connect(d_ptr,
            &RequestManagerPrivate::privateEvent,
            this,
            &RequestManager::onPrivateEvent);
}

void RequestManager::setSessionInfo(QString sessionID, QByteArray key)
{
    Q_D(RequestManager);

    d->currentSession = sessionID;
    d->sessionKey = key;
}

RequestManager::RequestError
RequestManager::sendData(const QByteArray& data,
                         QByteArray& result,
                         int serverID,
                         QString objectPath,
                         bool synchronous,
                         int* requestID)
{
    Q_D(RequestManager);

    QByteArray temp, bufferIn;
    result.clear();

    bufferIn.append(d->currentSession);
    bufferIn.append(d->encoder.getCRC32(data));
    d->encoder.encrypt(Encryptor::AES,
                       data,
                       d->sessionKey,
                       temp);
    bufferIn.append(temp);

    RequestError errCode = sendRawData(temp, result,
                                       serverID, objectPath,
                                       synchronous, requestID);
    if (!synchronous || errCode != Ok)
        return errCode;

    d->encoder.decrypt(Encryptor::AES,
                       temp.mid(4),
                       d->sessionKey,
                       result);
    if (d->encoder.getCRC32(result) != temp.mid(0, 4))
        return DecryptError;
    else
        return Ok;
}

RequestManager::RequestError
RequestManager::sendRawData(const QByteArray& data,
                           QByteArray& result,
                           int serverID,
                           QString objectPath,
                           bool synchronous,
                           int* requestID)
{
    Q_D(RequestManager);

    if (!synchronous)
    {
        if (d->server.sendAsyncRequest(serverID,
                                       objectPath,
                                       const_cast<QByteArray&>(data),
                                       *requestID))
            return Ok;
        else
            return CannotConnect;
    }

    ServerConnection::ConnectionStatus errCode =
                            d->server.sendRequest(serverID,
                                                  objectPath,
                                                  const_cast<QByteArray&>(data),
                                                  result);
    if (errCode != ServerConnection::Ok)
        return RequestError(errCode);
    if (result.length() < 4)
        return ReplyIncomplete;
}

RequestManager::RequestError
RequestManager::getData(int requestID, QByteArray& buffer)
{
    Q_D(RequestManager);

    QByteArray data;
    ServerConnection::ConnectionStatus errCode =
                            d->server.getAsyncResponse(requestID, data);

    if (errCode != ServerConnection::Ok)
        return RequestError(errCode);
    if (data.length() < 4)
        return ReplyIncomplete;

    QByteArray crc32 = data.left(4);
    d->encoder.decrypt(Encryptor::AES,
                               data.mid(4),
                               d->sessionKey,
                               data);
    if (d->encoder.getCRC32(data) != crc32)
        return DecryptError;
    else
    {
        buffer = data;
        return Ok;
    }
}

RequestManager::RequestType RequestManager::getRecordType(int requestID)
{
    Q_D(RequestManager);

    for (int i=0; i<d->requestList.count(); i++)
    {
        if (d->requestList[i].requestID == requestID)
        {
            return d->requestList[i].type;
        }
    }
}

void RequestManager::setRecordType(int type, int requestID)
{
    Q_D(RequestManager);

    RequestManagerPrivate::RequestRecord record;
    record.type = type;
    record.requestID = requestID;
    d->requestList.push_back(record);
}

void RequestManager::removeRequestRecord(int requestID)
{
    Q_D(RequestManager);

    for (int i=0; i<d->requestList.count(); i++)
    {
        if (d->requestList[i].requestID == requestID)
        {
            d->requestList.removeAt(i);
            break;
        }
    }
}

void RequestManager::onPrivateEvent(int eventType, int data)
{
    switch (RequestManagerPrivate::PrivateEventType(eventType))
    {
        case RequestManagerPrivate::RequestFinished:
        {
            emit onRequestFinished(data);
            break;
        }
        default:;
    }
}

RequestManagerPrivate::RequestManagerPrivate(RequestManager* parent)
{
    this->q_ptr = parent;
    connect(&server,
            SIGNAL(onRequestFinished(int)),
            this,
            SLOT(onRequestFinished(int)));
}

void RequestManagerPrivate::onRequestFinished(int requestID)
{
    emit privateEvent(RequestFinished, requestID);
}
