#ifndef GLOBAL_OBJECTS_H
#define GLOBAL_OBJECTS_H

class WichatConfig;
class ServerConnection;
class Account;
class Conversation;
class MainWindow;
class PreferenceDialog;

extern WichatConfig globalConfig;
extern ServerConnection globalConnection;
extern Account globalAccount;
extern Conversation globalConversation;

extern MainWindow* globalMainWindow;
extern PreferenceDialog* globalPreference;

#endif // GLOBAL_OBJECTS_H
