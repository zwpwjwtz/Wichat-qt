#ifndef GROUP_H
#define GROUP_H

#include "abstractchat.h"

class GroupPrivate;

class Group : public AbstractChat
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Group)

public:
    static const int RecordSaltLen = 16;

    explicit Group();
    explicit Group(ServerConnection& server);
    ~Group();

    bool verify(QByteArray sessionID, QByteArray sessionKey);
    bool sendMessage(QString ID, QByteArray& content, int& queryID);
    bool getMessageList(int& queryID);
    bool getMessageList(QDateTime& lastTime, int& queryID);
    bool receiveMessage(QString ID, int &queryID);
    bool receiveMessage(QString ID, QDateTime& lastTime, int& queryID);

    void setGroupList(QList<QString> groupIDList);
};

#endif // GROUP_H
