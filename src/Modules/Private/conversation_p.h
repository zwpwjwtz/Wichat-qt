#ifndef CONVERSATION_P_H
#define CONVERSATION_P_H

#include <QQueue>

#include "abstractservice_p.h"
#include "../conversation.h"
#include "../peersession.h"


class Conversation;

class ConversationPrivate : public AbstractServicePrivate
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
        inline ServerObject(const int& initValue){value = initValue;}
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
        inline RequestType(const int& initValue){value = initValue;}
    };
    enum PrivateEventType
    {
        RequestFinished = 1,
        SendingFailed = 2,
        ReceivingFailed = 3
    };

    struct MessageTransaction
    {
        QString target;
        QByteArray messageID;
        QByteArray* data;
        QList<Conversation::MessageEntry>* messages;
        qint32 pos;
        qint32 currentMessageLength;
        bool multiPart;
        int queryID;    // For high-level callbacks; constant
        int requestID;  // For low-level callbacks; variable
    };

    static const int MaxMsgBlock = 50 * 1024 - 16;

    QString currentID;
    bool loggedin;
    QByteArray currentSession;
    QByteArray sessionKey;
    QByteArray tempLoginKey;
    QByteArray tempSession;
    qint64 sessionStartTime;
    qint64 sessionValidTime;
    QByteArray keySalt;
    QString userDir;
    PeerSession* sessionList;
    QQueue<MessageTransaction> sendingList;
    QQueue<MessageTransaction> receivingList;

    ConversationPrivate(Conversation* parent = 0, ServerConnection* server = 0);

    virtual void dispatchQueryRespone(int requestID);
    bool processReplyData(RequestType type, QByteArray& data);
    bool processSendList();
    bool processReceiveList();

    int getAvailableQueryID();
    MessageTransaction* getTransactionByQueryID(int queryID);
    MessageTransaction* getTransactionByRequestID(int requestID);
    void removeTransaction(MessageTransaction* transaction);

    void dataXMLize(const QByteArray& src, QByteArray& dest);
    void dataUnxmlize(const QByteArray& src,
                      QByteArray& dest,
                      QString cacheDir);

    static void parseAccountList(QByteArray& data,
                                 QByteArray listType,
                                 QList<Conversation::MessageListEntry> &list);
    static QString serverObjectToPath(ServerObject objectID);
};

#endif // CONVERSATION_P_H
