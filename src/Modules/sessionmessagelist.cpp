#include <QDataStream>

#include "sessionmessagelist.h"
#include "Private/sessionemessagelist_p.h"

#define WICHAT_SESSION_MSGLIST_SERIAL_MAGIC "WichatMesgList"
#define WICHAT_SESSION_MSGLIST_SERIAL_VER "\x02\x00"


SessionMessageList::SessionMessageList()
{
    this->d_ptr = new SessionMessageListPrivate(this);
}

SessionMessageList::~SessionMessageList()
{
    delete this->d_ptr;
}

int SessionMessageList::count()
{
    Q_D(SessionMessageList);
    return d->messageIDList.count();
}

void SessionMessageList::clear()
{
    Q_D(SessionMessageList);

    d->messageIDList.clear();
    d->messageList.clear();
}

QList<SessionMessageList::MessageEntry> SessionMessageList::getAll() const
{
    const Q_D(SessionMessageList);
    return d->messageList;
}

SessionMessageList::MessageEntry
SessionMessageList::getMessageByID(int ID) const
{
    const Q_D(SessionMessageList);

    int index = d->messageIDList.indexOf(ID);
    if (index < 0)
        return SessionMessageListPrivate::emptyMessage;
    else
        return d->messageList[index];
}

QList<SessionMessageList::MessageEntry>
SessionMessageList::getMessageBySource(QString source) const
{
    const Q_D(SessionMessageList);

    MessageEntry message;
    QList<MessageEntry> tempList;
    for (int i=0; i<d->messageList.count(); i++)
    {
        message = d->messageList[i];
        if (message.source == source)
            tempList.append(message);
    }
    return tempList;
}

QList<SessionMessageList::MessageEntry>
SessionMessageList::getMessageByType(MessageType type) const
{
    const Q_D(SessionMessageList);

    MessageEntry message;
    QList<MessageEntry> tempList;
    for (int i=0; i<d->messageList.count(); i++)
    {
        message = d->messageList[i];
        if (message.type == type)
            tempList.append(message);
    }
    return tempList;
}

QList<SessionMessageList::MessageEntry>
SessionMessageList::getMessageByTime(QDateTime from, QDateTime to) const
{
    const Q_D(SessionMessageList);

    MessageEntry message;
    QList<MessageEntry> tempList;
    for (int i=0; i<d->messageList.count(); i++)
    {
        message = d->messageList[i];
        if (message.time >= from && message.time <= to)
            tempList.append(message);
    }
    return tempList;
}

QList<SessionMessageList::MessageEntry>
SessionMessageList::getMessageByRange(int ID, int range) const
{
    const Q_D(SessionMessageList);

    QList<SessionMessageList::MessageEntry> tempList;
    int index = d->messageIDList.indexOf(ID);
    if (index >= 0)
    {
        int start = index, stop = index + range;
        if (stop < 0)
            stop = 0;
        else if (stop >= d->messageList.count())
            stop = d->messageList.count();
        if (start > stop)
        {
            start = stop;
            stop = index + 1;
        }
        while (start < stop)
        {
            tempList.append(d->messageList[start]);
            start++;
        }
    }
    return tempList;
}

SessionMessageList::MessageEntry SessionMessageList::first() const
{
    const Q_D(SessionMessageList);
    if (d->messageList.count() > 0)
        return d->messageList.first();
    else
        return d->emptyMessage;
}

SessionMessageList::MessageEntry SessionMessageList::last() const
{
    const Q_D(SessionMessageList);
    if (d->messageList.count() > 0)
        return d->messageList.last();
    else
        return d->emptyMessage;
}

bool SessionMessageList::addMessage(MessageEntry& message)
{
    Q_D(SessionMessageList);

    message.ID = d->getAvailableID();
    if (message.ID == 0)
        return false;

    d->messageList.append(message);
    d->messageIDList.append(message.ID);
    return true;
}

bool SessionMessageList::removeMessage(int messageID)
{
    Q_D(SessionMessageList);

    int index = d->messageIDList.indexOf(messageID);
    if (index < 0)
        return false;

    d->messageIDList.removeAt(index);
    d->messageList.removeAt(index);
    return true;
}

QDataStream& operator<<(QDataStream& stream, const SessionMessageList& list)
{
    QByteArray buffer;

    buffer = QByteArray::fromRawData(WICHAT_SESSION_MSGLIST_SERIAL_MAGIC,
                                     strlen(WICHAT_SESSION_MSGLIST_SERIAL_MAGIC));
    stream << buffer;

    buffer = QByteArray::fromRawData(WICHAT_SESSION_MSGLIST_SERIAL_VER,
                                     strlen(WICHAT_SESSION_MSGLIST_SERIAL_VER));
    stream << buffer;

    QList<SessionMessageList::MessageEntry> tempList =list.getAll();
    stream << tempList.count();
    for (int i=0; i<tempList.count(); i++)
    {
        stream << tempList[i].ID;
        stream << tempList[i].time;
        stream << int(tempList[i].type);
        stream << tempList[i].source;
        stream << tempList[i].content;
    }

    return stream;
}

QDataStream& operator>>(QDataStream& stream, SessionMessageList& list)
{
    int listLength;
    QByteArray buffer;
    SessionMessageList::MessageEntry message;

    stream >> buffer;
    if (buffer != WICHAT_SESSION_MSGLIST_SERIAL_MAGIC)
        return stream;

    stream >> buffer;
    if (buffer != WICHAT_SESSION_MSGLIST_SERIAL_VER)
        return stream;

    stream >> listLength;
    if (listLength < 0)
        return stream;

    list.clear();
    for (int i=0; i<listLength; i++)
    {
        int tempInt;
        stream >> message.ID;
        stream >> message.time;
        stream >> tempInt;
        message.type = SessionMessageList::MessageType(tempInt);
        stream >> message.source;
        stream >> message.content;

        list.addMessage(message);
    }

    return stream;
}

SessionMessageList::MessageEntry SessionMessageListPrivate::emptyMessage = {};

SessionMessageListPrivate::SessionMessageListPrivate(SessionMessageList *parent)
{
    this->q_ptr = parent;

    emptyMessage.ID = -1;
    emptyMessage.type = SessionMessageList::UnknownMessageType;
    emptyMessage.time = QDateTime::fromMSecsSinceEpoch(0);
}

int SessionMessageListPrivate::getAvailableID()
{
    int ID;
    if (messageIDList.count() < 1)
        ID = 1;
    else
        ID = messageIDList.last() + 1;

    while (messageIDList.contains(ID) && ID != 0)
        ID++;
    return ID;
}
