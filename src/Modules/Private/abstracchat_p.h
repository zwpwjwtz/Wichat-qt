#ifndef ABSTRACTCHAT_P_H
#define ABSTRACTCHAT_P_H

#include <QQueue>

#include "abstractservice_p.h"
#include "../abstractchat.h"
#include "../peersession.h"


class AbstractChat;

class AbstractChatPrivate : public AbstractServicePrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(AbstractChat)

public:
    struct MessageTransaction
    {
        QString target;
        QDateTime time;
        QByteArray messageID;
        QByteArray* data;
        QList<AbstractChat::MessageEntry>* messages;
        qint32 pos;
        bool multiPart;
        int queryID;    // For high-level callbacks; constant
        int requestID;  // For low-level callbacks; variable
    };

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

    AbstractChatPrivate(AbstractChat* parent, ServerConnection* server);

    virtual bool processSendList() = 0;
    virtual bool processReceiveList() = 0;

    int getAvailableQueryID();
    MessageTransaction* getTransactionByQueryID(int queryID);
    MessageTransaction* getTransactionByRequestID(int requestID);
    void removeTransaction(MessageTransaction* transaction);

    void dataXMLize(const QByteArray& src, QByteArray& dest);
    void dataUnxmlize(const QByteArray& src,
                      QByteArray& dest,
                      QString cacheDir);

    static void parseMessageList(QByteArray& data,
                                 QByteArray listType,
                                 QList<AbstractChat::MessageListEntry> &list);
};

#endif // ABSTRACTCHAT_P_H
