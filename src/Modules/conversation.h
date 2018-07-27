#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <QDateTime>
#include "abstractservice.h"


class PeerSession;
class ConversationPrivate;

class Conversation : public AbstractService
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Conversation)

public:
    enum class VerifyError
    {
        Ok = 0,
        NetworkError = 1,
        VersionNotSupported = 2,
        SessionFormatError = 3,
        KeyFormatError = 4,
        VerificationFailed = 5,
        UnknownError = 255
    };
    enum class QueryError
    {
        Ok = 0,
        NetworkError = 1,
        VersionNotSupported = 2,
        UnknownError = 255
    };
    struct MessageListEntry
    {
        QString ID;
    };
    struct MessageEntry
    {
        QString source;
        QDateTime time;
        int length;
        QByteArray content;
    };

    static const int RecordSaltLen = 16;

    explicit Conversation();
    explicit Conversation(ServerConnection& server);
    ~Conversation();

    void setUserDirectory(QString path);
    void setPeerSession(PeerSession& sessionList);
    bool verify(QByteArray sessionID, QByteArray sessionKey);
    QByteArray& keySalt();
    bool resetSession();
    bool sendMessage(QString ID, QByteArray& content, int& queryID);
    bool getMessageList();
    bool receiveMessage(QString ID, int& queryID);
    bool fixBrokenConnection(QString ID, int& queryID);

signals:
    void queryError(int queryID, Conversation::QueryError errorCode);
    void connectionBroken(QString ID);
    void verifyFinished(int queryID, Conversation::VerifyError errorCode);
    void resetSessionFinished(int queryID, bool successful);
    void sendMessageFinished(int queryID, bool successful);
    void getMessageListFinished(int queryID,
                                QList<Conversation::MessageListEntry>& msgList);
    void receiveMessageFinished(int queryID,
                                QList<Conversation::MessageEntry>& messages);
    void fixBrokenConnectionFinished(int queryID, bool successful);
};

#endif // CONVERSATION_H
