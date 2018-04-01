#ifndef GLOBAL_OBJECTS_H
#define GLOBAL_OBJECTS_H

#include "wichatconfig.h"
#include "serverconnection.h"
#include "account.h"
#include "mainwindow.h"

extern WichatConfig globalConfig;
extern ServerConnection globalConnection;
extern Account globalAccount;

extern MainWindow* globalMainWindow;

#endif // GLOBAL_OBJECTS_H
