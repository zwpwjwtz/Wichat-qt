#ifndef CONVERSATION_P_H
#define CONVERSATION_P_H

#include <QQueue>

#include "abstracchat_p.h"
#include "../conversation.h"


class Conversation;

class ConversationPrivate : public AbstractChatPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Conversation)

public:
    class ServerObject : public AbstractEnum
    {
    public:
        static const int RecordLogin = 1;
        static const int RecordGet = 2;
        static const int RecordAction = 3;
        inline ServerObject(const int initValue){value = initValue;}
    };
    class RequestType : public AbstractEnum
    {
    public:
        static const int None = 0;
        static const int Verify = 1;
        static const int ResetSession = 2;
        static const int SendMessage = 3;
        static const int GetMessageList = 4;
        static const int ReceiveMessage = 5;
        static const int FixConnection = 6;
        inline RequestType(const int initValue){value = initValue;}
    };
    enum PrivateEventType
    {
        RequestFinished = 1,
        SendingFailed = 2,
        ReceivingFailed = 3
    };

    static const int MaxMsgBlock = 50 * 1024 - 16;

    ConversationPrivate(Conversation* parent = 0, ServerConnection* server = 0);

    virtual void dispatchQueryRespone(int requestID);
    bool processReplyData(RequestType type, QByteArray& data);
    bool processSendList();
    bool processReceiveList();

    static QString serverObjectToPath(ServerObject objectID);
};

#endif // CONVERSATION_P_H
