#ifndef ABSTRACTSERVICE_P_H
#define ABSTRACTSERVICE_P_H

#include <QObject>
#include "../abstractenum.h"
#include "../encryptor.h"
#include "../requestmanager.h"


class AbstractService;
class ServerConnection;

class AbstractServicePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(AbstractService)
protected:
    AbstractService* q_ptr;

public:
    struct RequestInfo
    {
        int ID;
        AbstractEnum type;
    };

    RequestManager* server;
    QList<RequestInfo> requestList;
    static Encryptor encoder;

    AbstractServicePrivate(AbstractService* parent = nullptr,
                           ServerConnection* server = nullptr);
    ~AbstractServicePrivate();

    int getRequestIndexByID(int requestID);
    void addRequest(int requestID, AbstractEnum type);

    virtual void dispatchQueryRespone(int requestID) = 0;

    static QByteArray formatID(const QString& ID);
    static void parseMixedList(const QByteArray &data,
                               const QByteArray& fieldName,
                               QList<QByteArray>& list,
                               int* parsedLength = 0);

signals:
    void privateEvent(int eventType, int data);

protected slots:
    virtual void onRequestFinished(int requestID);
};

#endif // ABSTRACTSERVICE_P_H
