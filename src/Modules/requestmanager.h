#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include <QObject>

class ServerConnection;

class RequestManagerPrivate;
class RequestManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RequestManager)
protected:
    RequestManagerPrivate* d_ptr;

public:
    enum ServerType
    {
        RootServer = 0,
        AccountServer = 1,
        RecordServer = 2,
    };

    enum RequestError
    {
        Ok = 0,
        CannotConnect = 1,
        ServerError = 2,
        NotAvailable = 3,
        VersionTooOld = 4,
        ReplyIncomplete = 128,
        RequestInvalid = 129,
        DecryptError = 130,
        UnknownError = 255,
    };

    explicit RequestManager();
    explicit RequestManager(ServerConnection& server);
    ~RequestManager();
    void setSessionInfo(QByteArray sessionID, QByteArray key);
    RequestError sendRawData(const QByteArray& data,
                             QByteArray& result,
                             ServerType serverID,
                             QString objectPath,
                             bool synchronous = true,
                             int* requestID = nullptr);
    RequestError sendData(const QByteArray& data,
                          QByteArray& result,
                          ServerType serverID,
                          QString objectPath,
                          bool synchronous = true,
                          int* requestID = nullptr);
    RequestError getData(int requestID, QByteArray& buffer);
    void removeRequest(int requestID);

signals:
    void requestFinished(int requestID);
};

#endif // REQUESTMANAGER_H
