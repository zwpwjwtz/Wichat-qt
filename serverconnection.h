#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QObject>

#define WICHAT_SERVER_RESPONSE_NONE 0
#define WICHAT_SERVER_RESPONSE_SUCCESS 1
#define WICHAT_SERVER_RESPONSE_BUSY 2
#define WICHAT_SERVER_RESPONSE_INVALID 3
#define WICHAT_SERVER_RESPONSE_DEVICE_UNSUPPORTED 4
#define WICHAT_SERVER_RESPONSE_FAILED 5
#define WICHAT_SERVER_RESPONSE_IN_MAINTANANCE 8


class ServerConnectionPrivate;

class ServerConnection : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ServerConnection)

public:
    enum ConnectionStatus
    {
        Ok = 0,
        CannotConnect = 1,
        ServerError = 2,
        NotAvailable = 3,
        VersionTooOld = 4,
        UnknownError = 255,
    };

    explicit ServerConnection();
    ~ServerConnection();
    bool setRootServer(QString serverName, int port);
    ConnectionStatus init(bool refresh = false);
    int getServerList();
    bool sendRequest(int serverID,
                     QString URL,
                     QByteArray& content,
                     QByteArray& buffer);

protected:
    ServerConnectionPrivate* d_ptr;
};

#endif // SERVERCONNECTION_H
