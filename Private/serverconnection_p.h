#ifndef SERVERCONNECTION_P_H
#define SERVERCONNECTION_P_H

#include <QObject>
#include <QList>

class ServerConnection;

class ServerConnectionPrivate: QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(ServerConnection)

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

    QString rootServer;
    int rootServerPort;
    QList<QString> AccServerList;
    QList<QString> RecServerList;
    QByteArray iBuffer, oBuffer;

    bool hasInited = false;

    ServerConnectionPrivate();
    int httpRequest(QString strHostName,
                    int intPort,
                    QString strUrl,
                    QString strMethod,
                    QByteArray& bytePostData,
                    int lngPostDataLen,
                    QByteArray& byteReceive);

protected:
    ServerConnection* q_ptr;
};

#endif // SERVERCONNECTION_P_H
