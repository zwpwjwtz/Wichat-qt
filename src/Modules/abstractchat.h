#ifndef ABSTRACTCHAT_H
#define ABSTRACTCHAT_H

#include <QDateTime>
#include "abstractservice.h"


class PeerSession;
class AbstractChatPrivate;

class AbstractChat : public AbstractService
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractChat)

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

    virtual void setUserDirectory(QString path);
    virtual void setPeerSession(PeerSession& sessionList);
    virtual QByteArray& keySalt();
    virtual bool resetSession();
    virtual bool verify(QByteArray sessionID, QByteArray sessionKey) = 0;
    virtual bool sendMessage(QString ID, QByteArray& content, int& queryID) = 0;
    virtual bool getMessageList() = 0;
    virtual bool receiveMessage(QString ID, int& queryID) = 0;

signals:
    void queryError(int queryID, AbstractChat::QueryError errorCode);
    void verifyFinished(int queryID, AbstractChat::VerifyError errorCode);
    void resetSessionFinished(int queryID, bool successful);
    void sendMessageFinished(int queryID, bool successful);
    void getMessageListFinished(int queryID,
                                QList<AbstractChat::MessageListEntry>& msgList);
    void receiveMessageFinished(int queryID,
                                QList<AbstractChat::MessageEntry>& messages);
};

#endif // ABSTRACTCHAT_H
