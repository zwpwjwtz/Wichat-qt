#ifndef CONVERSATION_P_H
#define CONVERSATION_P_H

#include <QQueue>

#include "conversation.h"
#include "encryptor.h"
#include "requestmanager.h"
#include "peersession.h"


class Conversation;

class ConversationPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Conversation)
protected:
    Conversation* q_ptr;

public:
    enum class ServerObject
    {
        RecordLogin = 1,
        RecordGet = 2,
        RecordAction = 3
    };
    enum class RequestType
    {
        None = 0,
        Verify = 1,
        ResetSession = 2,
        SendMessage = 3,
        GetMessageList = 4,
        ReceiveMessage = 5,
        FixConnection = 6
    };
    enum PrivateEventType
    {
        RequestFinished = 1,
        SendingFailed = 2,
        ReceivingFailed = 3
    };
    struct RequestInfo
    {
        int ID;
        RequestType type;
    };
    struct MessageTransaction
    {
        QString target;
        QByteArray* data;
        qint32 pos;
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
    Encryptor encoder;
    RequestManager* server;
    QList<RequestInfo> requestList;
    QString userDir;
    PeerSession* sessionList;
    QQueue<MessageTransaction> sendingList;
    QQueue<MessageTransaction> receivingList;

    ConversationPrivate(Conversation* parent = 0, ServerConnection* server = 0);
    ~ConversationPrivate();
    int getRequestIndexByID(int requestID);
    void addRequest(int requestID, RequestType type);
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
    static QByteArray formatID(QString ID);
    static QString serverObjectToPath(ServerObject objectID);

signals:
    void privateEvent(int eventType, int data);

protected slots:
    void onRequestFinished(int requestID);
};

#endif // CONVERSATION_P_H
