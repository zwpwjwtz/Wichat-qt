#include "notification.h"
#include "Private/notification_p.h"

Notification::Notification()
{
    this->d_ptr = new NotificationPrivate(this);
}

Notification::~Notification()
{
    delete this->d_ptr;
}

int Notification::count()
{
    Q_D(Notification);
    return d->noteQueue.count();
}

void Notification::append(const Note& note)
{
    Q_D(Notification);
    d->noteQueue.push_back(note);
    emit newNote(note.ID);
}

const Notification::Note& Notification::peek(int noteID)
{
    Q_D(Notification);
    for (int i=0; i<d->noteQueue.count(); i++)
    {
        if (d->noteQueue[i].ID == noteID)
            return d->noteQueue[i];
    }
    return d->emptyNote;
}

QList<Notification::Note> Notification::getAll()
{
    Q_D(Notification);
    QList<Note> noteList;
    for (int i=0; i<d->noteQueue.count(); i++)
    {
        noteList.push_back(d->noteQueue[i]);
    }
    return noteList;
}

void Notification::remove(int noteID)
{
    Q_D(Notification);
    for (int i=0; i<d->noteQueue.count(); i++)
    {
        if (d->noteQueue[i].ID == noteID)
        {
            d->noteQueue.removeAt(i);
            break;
        }
    }
    emit noteRemoved(noteID);
}

void Notification::clear()
{
    Q_D(Notification);
    d->noteQueue.clear();
}

int Notification::getNewID()
{
    int newID = 0;
    while (true)
    {
        newID++;
        if (peek(newID).destination.isEmpty())
            break;
    }
    return newID;
}

Notification::Note NotificationPrivate::emptyNote = {};

NotificationPrivate::NotificationPrivate(Notification* parent)
{
    this->q_ptr = parent;
}
