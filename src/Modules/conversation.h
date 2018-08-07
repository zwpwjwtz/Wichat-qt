#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <QDateTime>
#include "abstractchat.h"


class ConversationPrivate;

class Conversation : public AbstractChat
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Conversation)

public:
    static const int RecordSaltLen = 16;

    explicit Conversation();
    explicit Conversation(ServerConnection& server);
    ~Conversation();

    bool verify(QByteArray sessionID, QByteArray sessionKey);
    bool sendMessage(QString ID, QByteArray& content, int& queryID);
    bool getMessageList();
    bool receiveMessage(QString ID, int& queryID);
    bool fixBrokenConnection(QString ID, int& queryID);

signals:
    void connectionBroken(QString ID);
    void fixBrokenConnectionFinished(int queryID, bool successful);
};

#endif // CONVERSATION_H
