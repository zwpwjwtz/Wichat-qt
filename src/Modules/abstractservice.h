#ifndef ABSTRACTSERVICE_H
#define ABSTRACTSERVICE_H

#include <QObject>


class AbstractServicePrivate;
class ServerConnection;

class AbstractService : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractService)
protected:
    AbstractServicePrivate* d_ptr;

public:
    static const int MaxIDLen = 8;
    static const int SessionLen = 16;
    static const int KeyLen = 16;

    AbstractService();

protected slots:
    virtual void onPrivateEvent(int eventType, int data);
};

#endif // ABSTRACTSERVICE_H
