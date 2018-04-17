#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include <QObject>

class RequestManagerPrivate;
class RequestManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RequestManager)
protected:
    RequestManagerPrivate* d_ptr;

public:
    typedef int RequestType;
    enum RequestError
    {
        Ok = 0,
        CannotConnect = 1,
        ServerError = 2,
        NotAvailable = 3,
        VersionTooOld = 4,
        ReplyIncomplete = 128,
        CannotDecrypt = 128,
        DecryptError = 129,
        UnknownError = 255,
    };

    explicit RequestManager();
    void setSessionInfo(QString sessionID, QByteArray key);
    RequestError sendRawData(const QByteArray& data,
                             QByteArray& result,
                             int serverID,
                             QString objectPath,
                             bool synchronous = true,
                             int* requestID = nullptr);
    RequestError sendData(const QByteArray& data,
                          QByteArray& result,
                          int serverID,
                          QString objectPath,
                          bool synchronous = true,
                          int* requestID = nullptr);
    RequestError getData(int requestID, QByteArray& buffer);
    RequestType getRecordType(int requestID);
    void setRecordType(int requestID, RequestType type);
    void removeRequestRecord(int requestID);

signals:
    void onRequestFinished(int requestID);

private slots:
    void onPrivateEvent(int eventType, int data);
};

#endif // REQUESTMANAGER_H
