#include "global_objects.h"

#include "Modules/wichatconfig.h"
#include "Modules/serverconnection.h"
#include "Modules/account.h"
#include "Modules/conversation.h"
#include "Modules/group.h"
#include "mainwindow.h"
#include "preferencedialog.h"


WichatConfig globalConfig;
ServerConnection globalConnection;
Account globalAccount(globalConnection);
Conversation globalConversation(globalConnection);
Group globalGroup(globalConnection);
MainWindow* globalMainWindow = nullptr;
PreferenceDialog* globalPreference = nullptr;
