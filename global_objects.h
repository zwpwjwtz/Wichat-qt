#ifndef GLOBAL_OBJECTS_H
#define GLOBAL_OBJECTS_H

#include "wichatconfig.h"
#include "requestmanager.h"
#include "account.h"
#include "conversation.h"
#include "mainwindow.h"

extern WichatConfig globalConfig;
extern RequestManager globalConnection;
extern Account globalAccount;
extern Conversation globalConversation;

extern MainWindow* globalMainWindow;

#endif // GLOBAL_OBJECTS_H
