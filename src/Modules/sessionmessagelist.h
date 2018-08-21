#ifndef SESSIONMESSAGELIST_H
#define SESSIONMESSAGELIST_H

#include <QObject>
#include <QDateTime>


class SessionMessageListPrivate;

class SessionMessageList
{
    Q_DECLARE_PRIVATE(SessionMessageList)
protected:
    SessionMessageListPrivate* d_ptr;

public:
    enum MessageType
    {
        TextMessage = 0,
        ImageAttachment = 1,
        FileAttachment = 2,
        UnknownMessageType = 255,
    };

    struct MessageEntry
    {
        int ID;
        QString source;
        QDateTime time;
        MessageType type;
        QByteArray content;
    };

    explicit SessionMessageList();
    ~SessionMessageList();

    int count();
    void clear();
    bool exist(int ID);

    MessageEntry first() const;
    MessageEntry last() const;

    QList<MessageEntry> getAll() const;
    MessageEntry getMessageByID(int ID) const;
    QList<MessageEntry> getMessageBySource(QString source) const;
    QList<MessageEntry> getMessageByType(MessageType type) const;
    QList<MessageEntry> getMessageByTime(QDateTime from, QDateTime to) const;
    QList<MessageEntry> getMessageByRange(int ID, int range) const;

    bool addMessage(MessageEntry &message);
    bool removeMessage(int messageID);
};

// Serializing and un-serializing operators
QDataStream& operator<<(QDataStream& stream, const SessionMessageList& list);
QDataStream& operator>>(QDataStream& stream, SessionMessageList& list);

#endif // SESSIONMESSAGELIST_H
