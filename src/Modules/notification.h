#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QDateTime>

class NotificationPrivate;

class Notification : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Notification)
protected:
    NotificationPrivate* d_ptr;

public:
    enum NoteEvent
    {
        none = 0,
        FriendAdd = 1,
        FriendDelete = 2,
        GotMsg = 3,
        GotGroupMsg = 4,
    };
    struct Note
    {
        NoteEvent type;
        QDateTime time;
        QString source;
        QString destination;
        int ID;
    };

    explicit Notification();
    ~Notification();

    int count();
    void append(const Note& note);
    const Note& peek(int noteID);
    QList<Note> getAll();
    void remove(int noteID);
    void clear();
    int getNewID();

signals:
    void newNote();
    void noteRemoved();
};

#endif // NOTIFICATION_H
