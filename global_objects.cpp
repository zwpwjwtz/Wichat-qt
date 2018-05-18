#include "global_objects.h"

#include "wichatconfig.h"
#include "serverconnection.h"
#include "account.h"
#include "conversation.h"
#include "mainwindow.h"
#include "preferencedialog.h"


WichatConfig globalConfig;
ServerConnection globalConnection;
Account globalAccount(globalConnection);
Conversation globalConversation(globalConnection);
MainWindow* globalMainWindow = nullptr;
PreferenceDialog* globalPreference = nullptr;
