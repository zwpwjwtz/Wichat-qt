#include "global_objects.h"

WichatConfig globalConfig;
RequestManager globalConnection;
Account globalAccount(globalConnection);
Conversation globalConversation(globalConnection);
MainWindow* globalMainWindow = nullptr;
