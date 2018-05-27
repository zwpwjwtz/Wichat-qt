#ifndef NOTIFICATIONPRIVATE_H
#define NOTIFICATIONPRIVATE_H

#include <QQueue>

#include "../notification.h"


class NotificationPrivate : QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Notification)
protected:
    Notification* q_ptr;

public:
    static Notification::Note emptyNote;
    QQueue<Notification::Note> noteQueue;

    NotificationPrivate(Notification* parent);
};

#endif // NOTIFICATIONPRIVATE_H
