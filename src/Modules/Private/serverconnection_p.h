#ifndef SERVERCONNECTION_P_H
#define SERVERCONNECTION_P_H

#include <QNetworkAccessManager>

class QNetworkRequest;
class QNetworkReply;

class ServerConnection;

class ServerConnectionPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(ServerConnection)
protected:
    ServerConnection* q_ptr;

public:
    const char* AcceptAll = "*/*\0";
    const char* BrowserAgent = "Mozilla/4.0\0";
    const char* MethodPost = "POST";
    const char* MethodGet = "GET";
    const int MaxRequestCount = 2;

#ifdef IS_LOCAL_SERVER
    const char* DefaultRootServer = "127.0.0.1.";
#else
    const char* DefaultRootServer = "dns.wichat.org";
#endif

    const char* QueryHeader = "WiChatCQ";
    const int QueryHeaderLen = 8;
    const int QueryGetAcc = 1;
    const int QueryGetRec = 2;
    const char* ResponseHeader = "WiChatSR";
    const int ResponseHeaderLen = 8;

    QNetworkAccessManager network;
    QString rootServer;
    int rootServerPort;
    QList<QString> AccServerList;
    QList<QString> RecServerList;
    QList<int> requestIdList;
    QList<QNetworkReply*> reponseList;
    bool hasInited = false;

    ServerConnectionPrivate(ServerConnection* parent);
    int getServerList();
    QString selectServer(int serverID);
    int httpRequest(QString strHostName,
                    int intPort,
                    QString strUrl,
                    QString strMethod,
                    QByteArray& bytePostData,
                    QByteArray& byteReceive,
                    bool boolSync = true);

protected:
    int waitHttpRequest(QNetworkReply* reply, bool synchronous = true);
    QNetworkReply* dealHttpRedirect(QNetworkReply* reply,
                                    QString method,
                                    QByteArray& data,
                                    bool synchronous = true);

protected slots:
    void onHttpRequestFinished(QNetworkReply* reply);
};

#endif // SERVERCONNECTION_P_H
