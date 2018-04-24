#ifndef REQUESTMANAGER_P_H
#define REQUESTMANAGER_P_H

#include <QObject>

#include "encryptor.h"
#include "serverconnection.h"


class RequestManager;

class RequestManagerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(RequestManager)
protected:
    RequestManager* q_ptr;

public:
    typedef int RecordType;

    struct RequestRecord
    {
        int requestID;
        RecordType type;
        bool rawData;
    };

    enum PrivateEventType
    {
        RequestFinished = 1
    };

    QByteArray currentSession;
    QByteArray sessionKey;
    Encryptor encoder;
    ServerConnection server;
    QList<RequestRecord> requestList;

    RequestManagerPrivate(RequestManager *parent = 0);
    int getRecordIndexByID(int requestID);

signals:
    void privateEvent(PrivateEventType eventType, int data);

private slots:
    void onRequestFinished(int requestID);
};

#endif // REQUESTMANAGER_P_H
