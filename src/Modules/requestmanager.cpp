#include "requestmanager.h"
#include "Private/requestmanager_p.h"


RequestManager::RequestManager()
{
    this->d_ptr = new RequestManagerPrivate(this);
}

RequestManager::RequestManager(ServerConnection& server)
{
    this->d_ptr = new RequestManagerPrivate(this, &server);
}

RequestManager::~RequestManager()
{
    delete this->d_ptr;
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
    d->encoder.encrypt(Encryptor::AES,
                       d->encoder.getCRC32(data).append(data),
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
                       result,
                       d->sessionKey,
                       temp);
    if (d->encoder.getCRC32(temp.mid(4)) != temp.left(4))
        return DecryptError;

    result = temp.mid(4);
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
        if (d->server->sendAsyncRequest(int(serverID),
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
                            d->server->sendRequest(int(serverID),
                                                  objectPath,
                                                  data,
                                                  result);
    if (errCode != ServerConnection::Ok)
        return RequestError(errCode);
    return Ok;
}

RequestManager::RequestError
RequestManager::getData(int requestID, QByteArray& buffer)
{
    Q_D(RequestManager);

    QByteArray data;
    ServerConnection::ConnectionStatus errCode =
                            d->server->getAsyncResponse(requestID, data);

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
    if (data.length() <= 4)
        return ReplyIncomplete;

    d->encoder.decrypt(Encryptor::AES,
                       data,
                       d->sessionKey,
                       data);
    if (d->encoder.getCRC32(data.mid(4)) != data.left(4))
        return DecryptError;
    else
    {
        buffer = data.mid(4);
        if (buffer.length() < 1)
            return Ok;
        switch (int(buffer[0]))
        {
            case WICHAT_SERVER_RESPONSE_SUCCESS:
                return Ok;
            case WICHAT_SERVER_RESPONSE_BUSY:
            case WICHAT_SERVER_RESPONSE_IN_MAINTANANCE:
                return NotAvailable;
            case WICHAT_SERVER_RESPONSE_INVALID:
                return RequestInvalid;
            case WICHAT_SERVER_RESPONSE_DEVICE_UNSUPPORTED:
                return VersionTooOld;
            default:
                return UnknownError;
        }
    }
}

void RequestManager::removeRequest(int requestID)
{
    Q_D(RequestManager);

    int index = d->getRecordIndexByID(requestID);
    if (index >= 0)
        d->requestList.removeAt(index);
}

RequestManagerPrivate::RequestManagerPrivate(RequestManager* parent,
                                             ServerConnection *server)
{
    this->q_ptr = parent;
    if (server)
    {
        this->server = server;
        defaultServer = false;
    }
    else
    {
        this->server = new ServerConnection;
        defaultServer = true;
    }
    connect(this->server,
            SIGNAL(requestFinished(int)),
            this,
            SLOT(onRequestFinished(int)));
}

RequestManagerPrivate::~RequestManagerPrivate()
{
    if (defaultServer)
        delete server;
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
    Q_Q(RequestManager);
    emit q->requestFinished(requestID);
}
