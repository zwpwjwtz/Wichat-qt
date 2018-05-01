#ifndef GLOBAL_OBJECTS_H
#define GLOBAL_OBJECTS_H

class WichatConfig;
class ServerConnection;
class Account;
class Conversation;
class MainWindow;

extern WichatConfig globalConfig;
extern ServerConnection globalConnection;
extern Account globalAccount;
extern Conversation globalConversation;

extern MainWindow* globalMainWindow;

#endif // GLOBAL_OBJECTS_H
