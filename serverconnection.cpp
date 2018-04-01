#include <QtNetwork>
#include "common.h"
#include "serverconnection.h"
#include "Private/serverconnection_p.h"

ServerConnection::ServerConnection()
{
    this->d_ptr = new ServerConnectionPrivate;
}

ServerConnection::~ServerConnection()
{
    delete this->d_ptr;
}

bool ServerConnection::setRootServer(QString serverName, int port)
{
    Q_D(ServerConnection);
    if (serverName.isEmpty() || port < 0)
        return false;
    d->rootServer = serverName;
    d->rootServerPort = port;
    return true;
}

ServerConnection::ConnectionStatus ServerConnection::init(bool refresh)
{
    Q_D(ServerConnection);

    ConnectionStatus status = Ok;

    if (d->hasInited && !refresh)
        return status;

    switch (getServerList())
    {
        case 0:
            d->hasInited = true;
            break;
        case 1:
            status = CannotConnect;
        case 2:
            status = ServerError;
        case 3:
            status = NotAvailable;
        case 4:
            status = VersionTooOld;
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
int ServerConnection::getServerList()
{
    Q_D(ServerConnection);

    int result = 0;
    int received;
    QList<QByteArray> strList;

    d->iBuffer.clear();
    d->iBuffer = d->QueryHeader;
    d->iBuffer.append(WICHAT_CLIENT_DEVICE).append(d->QueryGetAcc);

    received = d->httpRequest(d->rootServer, d->rootServerPort,
                              "/Root/query/index.php", "POST",
                              d->iBuffer, d->iBuffer.length(),
                              d->oBuffer);
    if (received < 1)
        return 1;
    switch (d->oBuffer[d->ResponseHeaderLen])
    {
        case WICHAT_SERVER_RESPONSE_NONE:
        case WICHAT_SERVER_RESPONSE_BUSY:
            result = 2;
            break;
        case WICHAT_SERVER_RESPONSE_SUCCESS:
            if (int(d->oBuffer[d->ResponseHeaderLen + 1]) < 1)
            {
                result = 3;
                break;
            }
            strList = d->oBuffer.mid(d->ResponseHeaderLen + 2).split('\0');
            if (strList.count() > 0)
            {
                for (int i=0; i<strList.count(); i++)
                    d->AccServerList.push_back(strList[i]);
            }
            else
                result = 3;
            break;
        case WICHAT_SERVER_RESPONSE_DEVICE_UNSUPPORTED:
            result = 4;
            break;
        default:
            result = -1;
    }
    if (result != 0)
        return result;

    d->iBuffer[d->QueryHeaderLen + 1] = d->QueryGetRec;
    received = d->httpRequest(d->rootServer, d->rootServerPort,
                              "/Root/query/index.php", d->MethodPost,
                              d->iBuffer, d->iBuffer.length(),
                              d->oBuffer);
    if (received < 1)
        return 1;
    switch(d->oBuffer[d->ResponseHeaderLen])
    {
        case WICHAT_SERVER_RESPONSE_NONE:
        case WICHAT_SERVER_RESPONSE_INVALID:
            result = 2;
            break;
        case WICHAT_SERVER_RESPONSE_SUCCESS:
            if (int(d->oBuffer[d->ResponseHeaderLen + 1]) < 1)
            {
                result = 3;
                break;
            }
            strList = d->oBuffer.mid(d->ResponseHeaderLen + 2).split('\0');
            if (strList.count() > 0)
            {
                for (int i=0; i<strList.count(); i++)
                    d->RecServerList.push_back(strList[i]);
            }
            else
                result = 3;
            break;
        case WICHAT_SERVER_RESPONSE_BUSY:
        case WICHAT_SERVER_RESPONSE_IN_MAINTANANCE:
            result = 3;
            break;
        default:
            result = -1;
    }

    return result;
}

bool ServerConnection::sendRequest(int serverID,
                                   QString URL,
                                   QByteArray& content,
                                   QByteArray& buffer)
{
    Q_D(ServerConnection);

    if (init() != ConnectionStatus::Ok)
        return false;

    QString server;
    if (serverID == 1)
    {
        if (d->AccServerList.isEmpty()) return false;
        server = d->AccServerList[0].trimmed();
    }
    else
    {
        if (d->RecServerList.isEmpty()) return false;
        server = d->RecServerList[0].trimmed();
    }

    d->iBuffer = d->QueryHeader;
    d->iBuffer.append(content);

    int received;
#ifdef QT_DEBUG
    received = d->httpRequest(server, 80,
                              URL, d->MethodPost,
                              d->iBuffer, d->iBuffer.length(),
                              buffer);
#else
    for (int i = 1; i < d->MaxRequestCount; i++)
    {
        received = d->httpRequest(server, 80,
                                  URL, d->MethodPost,
                                  d->iBuffer, d->iBuffer.length(),
                                  buffer);
        if (received > 0) break;
    }
#endif
    if (received > d->ResponseHeaderLen)
    {
        if (buffer.indexOf(d->ResponseHeader) == 0)
        {
            buffer.remove(0, d->ResponseHeaderLen);
            return true;
        }
    }
    return false;
}

ServerConnectionPrivate::ServerConnectionPrivate()
{
    rootServer = DefaultRootServer;
    rootServerPort = 80;
}

int ServerConnectionPrivate::httpRequest(QString strHostName,
                                         int intPort,
                                         QString strUrl,
                                         QString strMethod,
                                         QByteArray& bytePostData,
                                         int lngPostDataLen,
                                         QByteArray& byteReceive)
{
    QNetworkAccessManager network;
    if (network.networkAccessible() != QNetworkAccessManager::Accessible)
        return -1;

    QUrl url;
    url.setScheme("http");
    url.setHost(strHostName);
    url.setPort(intPort);
    url.setPath(strUrl);

    QNetworkRequest request;
    QNetworkReply* reply;
    QEventLoop loop;
    request.setUrl(url);
    request.setRawHeader("User-agent", BrowserAgent);
    if (strMethod == MethodPost)
        reply = network.post(request, bytePostData.left(lngPostDataLen));
    else
        reply = network.get(request);
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
        return 0;

    // Deal with redirect (when scheme is changed)
    QUrl redirectedURL(reply->attribute(
                           QNetworkRequest::RedirectionTargetAttribute).toUrl());
    if (redirectedURL.host() == url.host() &&
        redirectedURL.path() == url.path() &&
        redirectedURL.scheme() != url.scheme())
    {
        request.setUrl(redirectedURL);
        if (strMethod == MethodPost)
            reply = network.post(request, bytePostData.left(lngPostDataLen));
        else
            reply = network.get(request);
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
    }

    if (reply->error() != QNetworkReply::NoError)
        return 0;

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

    return p;
}
