#include <QtNetwork>
#include "common.h"
#include "serverconnection.h"
#include "Private/serverconnection_p.h"

#define WICHAT_CONNECTION_DNS_GET_ACC 1
#define WICHAT_CONNECTION_DNS_GET_REC 2
#define WICHAT_CONNECTION_DNS_GET_WEB 4

#define WICHAT_CONNECTION_DNS_PARSE_OK 0
#define WICHAT_CONNECTION_DNS_PARSE_NO_NETWORK 1
#define WICHAT_CONNECTION_DNS_PARSE_NO_RESPONSE 2
#define WICHAT_CONNECTION_DNS_PARSE_SERVER_ERROR 3
#define WICHAT_CONNECTION_DNS_PARSE_VER_ERROR 4
#define WICHAT_CONNECTION_DNS_PARSE_UNKOWN_ERROR -1

#define WICHAT_CONNECTION_REQUEST_PROP_ID "request_id"
#define WICHAT_CONNECTION_REQUEST_PROP_METHOD "request_method"
#define WICHAT_CONNECTION_REQUEST_PROP_DATA "request_data"
#define WICHAT_CONNECTION_REQUEST_PROP_REDIRECT "request_redirected"


ServerConnection::ServerConnection()
{
    this->d_ptr = new ServerConnectionPrivate(this);
}

ServerConnection::~ServerConnection()
{
    delete this->d_ptr;
}

bool ServerConnection::setRootServer(QString hostName, int port)
{
    Q_D(ServerConnection);
    if (hostName.isEmpty() || port < 0)
        return false;
    d->rootServer = hostName;
    d->rootServerPort = port;
    return true;
}

bool ServerConnection::getServerInfo(int serverID, QString &hostName, int &port)
{
    Q_D(ServerConnection);

    ServerConnectionPrivate::ServerInfo server = d->selectServer(serverID);
    hostName = server.hostName;
    port = server.port;
    return (!hostName.isEmpty());
}

ServerConnection::ConnectionStatus ServerConnection::init(bool refresh)
{
    Q_D(ServerConnection);

    ConnectionStatus status = Ok;

    if (d->hasInited && !refresh)
        return status;

    switch (d->getServerList())
    {
        case WICHAT_CONNECTION_DNS_PARSE_OK:
            d->hasInited = true;
            break;
        case WICHAT_CONNECTION_DNS_PARSE_NO_NETWORK:
            status = CannotConnect;
            break;
        case WICHAT_CONNECTION_DNS_PARSE_SERVER_ERROR:
            status = ServerError;
            break;
        case WICHAT_CONNECTION_DNS_PARSE_NO_RESPONSE:
            status = NotAvailable;
            break;
        case WICHAT_CONNECTION_DNS_PARSE_VER_ERROR:
            status = VersionTooOld;
            break;
        default:
            status = UnknownError;
    }

    return status;
}

/* Return value:
 * 0=OK;
 * 1=No Networking;
 * 2=Server No Response;
 * 3=Server Error;
 * 4=Version Error;
 * -1=Other Unknown Error
 */
int ServerConnectionPrivate::getServerList()
{
    int result = 0;
    int received;
    int parsedLength;
    int pos;
    QByteArray iBuffer, oBuffer;

    iBuffer = QueryHeader;
    iBuffer.append(WICHAT_CLIENT_DEVICE)
           .append(WICHAT_CONNECTION_DNS_GET_ACC
                   | WICHAT_CONNECTION_DNS_GET_REC
                   | WICHAT_CONNECTION_DNS_GET_WEB);

    received = httpRequest(rootServer, rootServerPort,
                              "/Root/query/index.php", "POST",
                              iBuffer,
                              oBuffer);
    if (received < 1)
        return WICHAT_CONNECTION_DNS_PARSE_NO_RESPONSE;
    else if (oBuffer.left(ResponseHeaderLen) != ResponseHeader)
        return WICHAT_CONNECTION_DNS_PARSE_SERVER_ERROR;
    else
        pos = ResponseHeaderLen;

    result = parseDNSResponse(oBuffer.mid(pos),
                              AccServerList,
                              parsedLength);
    if (result != WICHAT_CONNECTION_DNS_PARSE_OK)
        return result;
    pos += parsedLength;

    result = parseDNSResponse(oBuffer.mid(pos),
                              RecServerList,
                              parsedLength);
    if (result != WICHAT_CONNECTION_DNS_PARSE_OK)
        return result;
    pos += parsedLength;

    result = parseDNSResponse(oBuffer.mid(pos),
                              WebServerList,
                              parsedLength);
    return result;
}

ServerConnection::ConnectionStatus
ServerConnection::sendRequest(int serverID,
                              QString URL,
                              const QByteArray& content,
                              QByteArray& buffer)
{
    Q_D(ServerConnection);

    QByteArray iBuffer;
    ServerConnectionPrivate::ServerInfo server = d->selectServer(serverID);
    if (server.hostName.isEmpty())
        return CannotConnect;

    iBuffer = d->QueryHeader;
    iBuffer.append(content);

    int received = 0;
#ifdef QT_DEBUG
    received = d->httpRequest(server.hostName, server.port,
                              URL, d->MethodPost,
                              iBuffer,
                              buffer);
#else
    for (int i = 1; i < d->MaxRequestCount; i++)
    {
        received = d->httpRequest(server.hostName, 80,
                                  URL, d->MethodPost,
                                  iBuffer,
                                  buffer);
        if (received > 0) break;
    }
#endif
    if (received > d->ResponseHeaderLen)
    {
        if (buffer.indexOf(d->ResponseHeader) == 0)
        {
            buffer.remove(0, d->ResponseHeaderLen);
            return Ok;
        }
    }
    return CannotConnect;
}

bool ServerConnection::sendAsyncRequest(int serverID,
                                        QString URL,
                                        const QByteArray& content,
                                        int& requestID)
{
    Q_D(ServerConnection);

    QByteArray iBuffer;
    ServerConnectionPrivate::ServerInfo server = d->selectServer(serverID);
    if (server.hostName.isEmpty())
        return false;

    iBuffer = d->QueryHeader;
    iBuffer.append(content);

    requestID = d->httpRequest(server.hostName, server.port,
                               URL, d->MethodPost,
                               iBuffer,
                               iBuffer,
                               false);
    if (requestID != 0)
        return true;
    else
        return false;
}

ServerConnection::ConnectionStatus
ServerConnection::getAsyncResponse(int requestID, QByteArray& buffer)
{
    Q_D(ServerConnection);

    ServerConnection::ConnectionStatus errCode = Ok;
    int requestIndex = d->requestIdList.indexOf(requestID);
    if (requestIndex >= 0)
    {
        QNetworkReply* reply = d->reponseList[requestIndex];
        d->reponseList.removeAt(requestIndex);
        d->requestIdList.removeAt(requestIndex);
        QNetworkReply::NetworkError replyErrCode = reply->error();
        if (replyErrCode != QNetworkReply::NetworkError::NoError)
            errCode = CannotConnect;
        else
        {
            buffer = reply->readAll();
            if (buffer.length() > d->ResponseHeaderLen)
            {
                if (buffer.indexOf(d->ResponseHeader) == 0)
                    buffer.remove(0, d->ResponseHeaderLen);
            }
            else
                errCode = ServerError;
        }
        reply->deleteLater();
    }
    else
        errCode = UnknownError;

    return errCode;
}

ServerConnectionPrivate::ServerConnectionPrivate(ServerConnection* parent)
{
    this->q_ptr = parent;
    rootServer = DefaultRootServer;
    rootServerPort = 80;
    connect(&network,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(onHttpRequestFinished(QNetworkReply*)));
}

ServerConnectionPrivate::ServerInfo
ServerConnectionPrivate::selectServer(int serverID)
{
    Q_Q(ServerConnection);

    ServerInfo server;
    server.port = 0;
    if (q->init() != ServerConnection::ConnectionStatus::Ok)
        return server;
    switch (serverID)
    {
        case WICHAT_SERVER_ID_ROOT:
            server.hostName = rootServer;
            server.port = rootServerPort;
            break;
        case WICHAT_SERVER_ID_ACCOUNT:
            if (!AccServerList.isEmpty())
                server = AccServerList[0];
            break;
        case WICHAT_SERVER_ID_RECORD:
            if (!RecServerList.isEmpty())
                server = RecServerList[0];
            break;
        case WICHAT_SERVER_ID_WEB:
            if (!WebServerList.isEmpty())
                server = WebServerList[0];
            break;
        default:;
    }
    return server;
}

int ServerConnectionPrivate::httpRequest(QString strHostName,
                                         int intPort,
                                         QString strUrl,
                                         QString strMethod,
                                         QByteArray& bytePostData,
                                         QByteArray& byteReceive,
                                         bool boolSync)
{
#ifndef IS_LOCAL_SERVER
    if (network.networkAccessible() == QNetworkAccessManager::NotAccessible)
        return -1;
#endif

    QUrl url;
    url.setScheme("http");
    url.setHost(strHostName);
    if (intPort != 80)
        url.setPort(intPort);
    url.setPath(strUrl);

    QNetworkRequest request;
    QNetworkReply* reply;
    request.setUrl(url);
    request.setRawHeader("User-agent", BrowserAgent);
    if (strMethod == MethodPost)
        reply = network.post(request, bytePostData);
    else
        reply = network.get(request);

    if (!boolSync)
    {
        reply->setProperty(WICHAT_CONNECTION_REQUEST_PROP_METHOD, strMethod);
        reply->setProperty(WICHAT_CONNECTION_REQUEST_PROP_DATA, bytePostData);
        return waitHttpRequest(reply, false);
    }
    else
        waitHttpRequest(reply);

    QNetworkReply::NetworkError errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError)
    {
        reply->deleteLater();
        return 0;
    }

    // Deal with redirect (when scheme is changed)
    QNetworkReply* newReply = dealHttpRedirect(reply,
                                               strMethod,
                                               bytePostData,
                                               boolSync);
    if (newReply != reply)
    {
        reply->deleteLater();
        reply = newReply;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        reply->deleteLater();
        return 0;
    }

    // Read reply content
    int p = 0;
    QByteArray buffer;
    byteReceive.clear();
    while(true)
    {
        buffer = reply->readAll();
        if (buffer.size() < 1) break;
        byteReceive.append(buffer);
        p += buffer.size();
    }

    reply->deleteLater();
    return p;
}

int ServerConnectionPrivate::waitHttpRequest(QNetworkReply* reply,
                                              bool synchronous)
{
    if (!synchronous)
    {
        int requestID = 1;
        while (requestIdList.contains(requestID) || requestID == 0)
        {
            requestID = qrand();
        }
        reply->setProperty(WICHAT_CONNECTION_REQUEST_PROP_ID, requestID);
        reponseList.push_back(reply);
        requestIdList.push_back(requestID);
        return requestID;
    }
    else
    {
        QEventLoop loop;
        connect(&network, SIGNAL(finished(QNetworkReply*)),
                &loop, SLOT(quit()));
        loop.exec();
        return 0;
    }
}

QNetworkReply* ServerConnectionPrivate::dealHttpRedirect(QNetworkReply* reply,
                                                         QString method,
                                                         QByteArray& data,
                                                         bool synchronous)
{
    // Check if redirection has been performed
    if (!reply->property(WICHAT_CONNECTION_REQUEST_PROP_REDIRECT).isNull())
        return reply;

    QNetworkRequest request = reply->request();
    QUrl originalURL = request.url();
    QUrl redirectedURL(reply->attribute(
                       QNetworkRequest::RedirectionTargetAttribute).toUrl());
    if (redirectedURL.host() == originalURL.host() &&
        redirectedURL.path() == originalURL.path() &&
        redirectedURL.scheme() != originalURL.scheme())
    {
        QNetworkReply* newReply;
        request.setUrl(redirectedURL);
        if (method == MethodPost)
            newReply = network.post(request, data);
        else
            newReply = network.get(request);
        reply->setProperty(WICHAT_CONNECTION_REQUEST_PROP_REDIRECT, true);
        waitHttpRequest(reply, synchronous);
        return newReply;
    }
    return reply;
}

int ServerConnectionPrivate::parseDNSResponse(const QByteArray& rawData,
                                              QList<ServerInfo>& serverList,
                                              int& parsedLength)
{
    int result;
    int serverCount;
    bool parseIntOK;
    QList<QByteArray> strList;
    ServerInfo server;

    parsedLength = 0;
    serverList.clear();
    switch (rawData[0])
    {
        case WICHAT_SERVER_RESPONSE_NONE:
        case WICHAT_SERVER_RESPONSE_BUSY:
            result = WICHAT_CONNECTION_DNS_PARSE_NO_RESPONSE;
            break;
        case WICHAT_SERVER_RESPONSE_SUCCESS:
            serverCount = int(rawData[1]);
            if (serverCount < 1)
            {
                result = WICHAT_CONNECTION_DNS_PARSE_SERVER_ERROR;
                break;
            }
            strList = rawData.mid(2).split('\0');
            parsedLength += 2;
            for (int i=0; i<strList.count(); i++)
            {
                // Parse the first field: host name
                server.hostName = strList[i].trimmed();
                parsedLength += strList[i].length() + 1;
                if (server.hostName.isEmpty())
                    break;

                // Parse the second field: port
                if (i + 1 < strList.count())
                {
                    i++;
                    server.port = strList[i].toInt(&parseIntOK);
                    parsedLength += strList[i].length() + 1;
                    if (!parseIntOK)
                        server.port = 80;
                }

                serverList.push_back(server);
                serverCount--;
                if (serverCount <= 0)
                    break;
            }
            result = WICHAT_CONNECTION_DNS_PARSE_OK;
            break;
        case WICHAT_SERVER_RESPONSE_DEVICE_UNSUPPORTED:
            result = WICHAT_CONNECTION_DNS_PARSE_VER_ERROR;
            break;
        default:
            result = WICHAT_CONNECTION_DNS_PARSE_UNKOWN_ERROR;
    }
    return result;
}

void ServerConnectionPrivate::onHttpRequestFinished(QNetworkReply* reply)
{
    Q_Q(ServerConnection);

    // Use property "ID" to check if the reply is a synchronous one
    int requestID = reply->property(WICHAT_CONNECTION_REQUEST_PROP_ID).toInt();
    if (requestID > 0)
    {
        QString method = reply->property(
                            WICHAT_CONNECTION_REQUEST_PROP_ID).toString();
        QByteArray postData = reply->property(
                            WICHAT_CONNECTION_REQUEST_PROP_DATA).toByteArray();

        if (dealHttpRedirect(reply, method, postData, false) != reply)
        {
            // Wait until the next call
            return;
        }

        emit q->requestFinished(requestID);
    }
}
