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

void RequestManager::setSessionInfo(QByteArray sessionID, QByteArray key)
{
    Q_D(RequestManager);

    d->currentSession = sessionID;
    d->sessionKey = key;
}

RequestManager::RequestError
RequestManager::sendData(const QByteArray& data,
                         QByteArray& result,
                         ServerType serverID,
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

    RequestError errCode = sendRawData(bufferIn, result,
                                       serverID, objectPath,
                                       synchronous, requestID);
    if (errCode != Ok)
        return errCode;
    if (!synchronous)
    {
        d->requestList[d->getRecordIndexByID(*requestID)].rawData = false;
        return errCode;
    }

    if (result.length() <= 4)
        return ReplyIncomplete;
    d->encoder.decrypt(Encryptor::AES,
                       temp.mid(4),
                       d->sessionKey,
                       result);
    if (d->encoder.getCRC32(result) != temp.mid(0, 4))
        return DecryptError;
    switch (int(result[0]))
    {
        case WICHAT_SERVER_RESPONSE_SUCCESS:
            return Ok;
        case WICHAT_SERVER_RESPONSE_BUSY:
        case WICHAT_SERVER_RESPONSE_IN_MAINTANANCE:
            return NotAvailable;
        case WICHAT_SERVER_RESPONSE_INVALID:
            return UnknownError;
        case WICHAT_SERVER_RESPONSE_DEVICE_UNSUPPORTED:
            return VersionTooOld;
        default:
            return UnknownError;
    }
}

RequestManager::RequestError
RequestManager::sendRawData(const QByteArray& data,
                           QByteArray& result,
                           ServerType serverID,
                           QString objectPath,
                           bool synchronous,
                           int* requestID)
{
    Q_D(RequestManager);

    if (!synchronous)
    {
        if (d->server.sendAsyncRequest(int(serverID),
                                       objectPath,
                                       data,
                                       *requestID))
        {
            RequestManagerPrivate::RequestRecord newRequest;
            newRequest.requestID = *requestID;
            newRequest.rawData = true;
            d->requestList.push_back(newRequest);
            return Ok;
        }
        else
            return CannotConnect;
    }

    ServerConnection::ConnectionStatus errCode =
                            d->server.sendRequest(int(serverID),
                                                  objectPath,
                                                  data,
                                                  result);
    if (errCode != ServerConnection::Ok)
        return RequestError(errCode);
    switch (int(result[0]))
    {
        case WICHAT_SERVER_RESPONSE_SUCCESS:
            return Ok;
        case WICHAT_SERVER_RESPONSE_BUSY:
        case WICHAT_SERVER_RESPONSE_IN_MAINTANANCE:
            return NotAvailable;
        case WICHAT_SERVER_RESPONSE_INVALID:
            return UnknownError;
        case WICHAT_SERVER_RESPONSE_DEVICE_UNSUPPORTED:
            return VersionTooOld;
        default:
            return UnknownError;
    }
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

    int recordIndex = d->getRecordIndexByID(requestID);
    if (recordIndex < 0)
        return UnknownError;
    if (d->requestList[recordIndex].rawData)
    {
        buffer = data;
        d->requestList.removeAt(recordIndex);
        return Ok;
    }

     d->requestList.removeAt(recordIndex);
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

void RequestManager::removeRequest(int requestID)
{
    Q_D(RequestManager);

    int index = d->getRecordIndexByID(requestID);
    if (index >= 0)
        d->requestList.removeAt(index);
}

void RequestManager::onPrivateEvent(int eventType, int data)
{
    Q_D(RequestManager);

    switch (RequestManagerPrivate::PrivateEventType(eventType))
    {
        case RequestManagerPrivate::RequestFinished:
        {
            if (d->getRecordIndexByID(data) >= 0)
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

int RequestManagerPrivate::getRecordIndexByID(int requestID)
{
    for (int i=0; i<requestList.count(); i++)
    {
        if (requestList[i].requestID == requestID)
        {
            return i;
        }
    }
    return -1;
}

void RequestManagerPrivate::onRequestFinished(int requestID)
{
    emit privateEvent(RequestFinished, requestID);
}
