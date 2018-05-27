#ifndef SESSIONEMESSAGELIST_P_H
#define SESSIONEMESSAGELIST_P_H

#include "../sessionmessagelist.h"


class SessionMessageList;

class SessionMessageListPrivate
{
    Q_DECLARE_PUBLIC(SessionMessageList)
protected:
    SessionMessageList* q_ptr;

public:
    QList<int> messageIDList;
    QList<SessionMessageList::MessageEntry> messageList;

    SessionMessageListPrivate(SessionMessageList* parent);
    int getAvailableID();
};

#endif // SESSIONEMESSAGELIST_P_H
