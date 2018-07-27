#include "abstractservice.h"
#include "Private/abstractservice_p.h"

#define WICHAT_SERVICE_EVENT_REQUEST_FINISHED 1


AbstractService::AbstractService()
{
    this->d_ptr = 0;
}

void AbstractService::onPrivateEvent(int eventType, int data)
{
    Q_D(AbstractService);

    if (d == 0)
        return;

    switch (eventType)
    {
        case WICHAT_SERVICE_EVENT_REQUEST_FINISHED:
            if (d->getRequestIndexByID(data) >= 0)
                d->dispatchQueryRespone(data);
            break;
        default:;
    }
}

AbstractServicePrivate::AbstractServicePrivate(AbstractService* parent,
                                               ServerConnection* server)
{
    this->q_ptr = parent;
    if (server)
        this->server = new RequestManager(*server);
    else
        this->server = new RequestManager;
    connect(this->server,
            SIGNAL(requestFinished(int)),
            this,
            SLOT(onRequestFinished(int)));
}

AbstractServicePrivate::~AbstractServicePrivate()
{
    delete this->server;
}

Encryptor AbstractServicePrivate::encoder;

int AbstractServicePrivate::getRequestIndexByID(int requestID)
{
    for (int i=0; i<requestList.count(); i++)
    {
        if (requestList[i].ID == requestID)
            return i;
    }
    return -1;
}

void AbstractServicePrivate::addRequest(int requestID, AbstractEnum type)
{
    RequestInfo request;
    request.ID = requestID;
    request.type = type;
    requestList.append(request);
}

QByteArray AbstractServicePrivate::formatID(const QString& ID)
{
    return ID.leftJustified(AbstractService::MaxIDLen, '\0', true).toLatin1();
}

void AbstractServicePrivate::parseMixedList(const QByteArray& data,
                                    const QByteArray& fieldName,
                                    QList<QByteArray>& list,
                                    int* parsedLength)
{
    int p1, p2, pE, length;
    QByteArray fieldBegin, fieldEnd;
    p1 = data.indexOf("<MList>");
    pE = data.indexOf("</MList>");
    fieldBegin.append('<').append(fieldName).append('>');
    fieldEnd.append("</").append(fieldName).append('>');
    list.clear();
    if (p1 >= 0 && pE > 13)
    {
        p2 = p1;
        while (true)
        {
            p1 = data.indexOf(fieldBegin, p1 + 1);
            p2 = data.indexOf(fieldEnd, p2 + 1);
            if (p1 < 0 || p2 < 0)
                break;

            length = p2 - p1 - fieldBegin.length();
            if (length > 0)
                list.append(data.mid(p1 + fieldBegin.length(), length));
            else
                list.append(QByteArray());
        }
        if (parsedLength)
            *parsedLength = pE - data.indexOf("<MList>") + 9;
    }
    else
    {
        if (parsedLength)
            *parsedLength = 0;
    }
}

void AbstractServicePrivate::onRequestFinished(int requestID)
{
    emit privateEvent(WICHAT_SERVICE_EVENT_REQUEST_FINISHED, requestID);
}
