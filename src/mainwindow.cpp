#include <QMessageBox>
#include <QCloseEvent>
#include <QMenu>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutwindow.h"
#include "accountinfodialog.h"
#include "groupinfodialog.h"
#include "preferencedialog.h"
#include "systraynotification.h"
#include "global_objects.h"
#include "imageresource.h"
#include "Modules/account.h"
#include "Modules/group.h"
#include "Modules/wichatconfig.h"

#define WICHAT_MAIN_MENU_APP_PREFERENCE 3
#define WICHAT_MAIN_MENU_APP_ABOUT 4
#define WICHAT_MAIN_MENU_APP_QUIT 5

#define WICHAT_MAIN_TIMER_WAIT_LOGOUT 3000
#define WICHAT_MAIN_TIMER_GET_FRIENDLIST 30
#define WICHAT_MAIN_TIMER_GET_FRIENDINFO 60
#define WICHAT_MAIN_TIMER_GET_GROUPLIST 120
#define WICHAT_MAIN_TIMER_GET_MSG 10
#ifdef QT_DEBUG
#define WICHAT_MAIN_TIMER_SHOW_NOTE 10
#else
#define WICHAT_MAIN_TIMER_SHOW_NOTE 2
#endif


OnlineState Wichat_Main_stateList[4] = {
    OnlineState::Online,
    OnlineState::Busy,
    OnlineState::Hide,
    OnlineState::Offline
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->labelFriendSearch->setAttribute(Qt::WA_TransparentForMouseEvents);
    for (int i=0; i<4; i++)
        ui->comboState->addItem(QIcon(ImageResource::stateToImagePath(
                                                    Wichat_Main_stateList[i],
                                                    true)),
                                Wichat_Main_stateList[i].toString(),
                                Wichat_Main_stateList[i].value);

    menuSysTray = new QMenu();
    sysTrayIcon = new QSystemTrayIcon(this);
    sysTrayIcon->setContextMenu(menuSysTray);
    sysTrayNoteList = new SystrayNotification;

    connect(&globalAccount,
            SIGNAL(resetSessionFinished(int, bool)),
            this,
            SLOT(onChangeSessionFinished(int, bool)));
    connect(&globalAccount,
            SIGNAL(setStateFinished(int, bool, OnlineState)),
            this,
            SLOT(onChangeStateFinished(int,bool, OnlineState)));
    connect(&globalAccount,
            SIGNAL(friendRequest(QString)),
            this,
            SLOT(onFriendRequest(QString)));
    connect(&globalAccount,
            SIGNAL(friendRemoved(QString)),
            this,
            SLOT(onFriendRemoved(QString)));
    connect(ui->listFriend,
            SIGNAL(listUpdated()),
            this,
            SLOT(onListFriendUpdated()));
    connect(ui->listFriend,
            SIGNAL(entryClicked(QString)),
            this,
            SLOT(onListFriendEntryClicked(QString)));
    connect(ui->listGroup,
            SIGNAL(listUpdated()),
            this,
            SLOT(onListGroupUpdated()));
    connect(ui->listGroup,
            SIGNAL(entryClicked(QString)),
            this,
            SLOT(onListGroupEntryClicked(QString)));
    connect(ui->frameSession,
            SIGNAL(currentSessionChanged(QString)),
            this,
            SLOT(onFrameSessionIDChanged(QString)));
    connect(sysTrayIcon,
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,
            SLOT(onSysTrayIconClicked(QSystemTrayIcon::ActivationReason)));
    connect(sysTrayNoteList,
            SIGNAL(noteClicked(const Notification::Note&)),
            this,
            SLOT(onSysTrayNoteClicked(const Notification::Note&)));
    connect(menuSysTray,
            SIGNAL(aboutToShow()),
            this,
            SLOT(onSysTrayMenuShowed()));
    connect(menuSysTray,
            SIGNAL(triggered(QAction*)),
            this,
            SLOT(onSysTrayMenuClicked(QAction*)));
    connect(&timer,
            SIGNAL(timeout()),
            this,
            SLOT(onTimerTimeout()));

    aboutDialog = nullptr;
    menuApp = nullptr;
    timer.setInterval(1000);
    manualExit = false;
    notificationState = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    static bool hasInited = false;
    if (hasInited)
        return;

    userID = globalAccount.ID();    

    ui->frameSession->bindConfig(&globalConfig);
    ui->frameSession->bindAccount(&globalAccount);
    ui->frameSession->bindFriendChatService(&globalConversation);
    ui->frameSession->bindGroupChatService(&globalGroup);
    ui->frameSession->bindNotificationService(&noteList);
    ui->frameSession->bindFriendList(ui->listFriend);
    ui->frameSession->bindGroupList(ui->listGroup);
    ui->listFriend->bindService(&globalAccount);
    ui->listGroup->bindService(&globalAccount);

    resizeEvent(0); // Trigger resizing manually
    sysTrayIcon->show();
    ui->frameSession->init();
    if (ui->frameSession->currentSessionID().isEmpty())
        ui->frameSession->hide();
    else
        ui->frameSession->show();

    addTask(taskUpdateAll);
    timer.start();

    hasInited = true;
}

void MainWindow::showAccountInfo(QString ID)
{
    ui->listFriend->showAccountInfo(ID);
}

void MainWindow::showGroupInfo(QString groupID)
{
    ui->listGroup->showGroupInfo(groupID);
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if (isMaximized())
            updateCaption();
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!manualExit)
    {
        if (QMessageBox::warning(this, "Exit Wichat",
                                 "Sure to exit WiChat?",
                                 QMessageBox::Yes | QMessageBox::No)
            != QMessageBox::Yes)
        {
            event->ignore();
            return;
        }
    }
    if (!ui->frameSession->isInterruptable())
    {
        if (QMessageBox::warning(this, "Exit WiChat",
                                 "One or more messages is waiting to be sent. "
                                 "Quitting now may lead to incomplete messages "
                                 "on the other side. \n"
                                 "Do you still want to quit WiChat?",
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No)
            != QMessageBox::Yes)
        {
            event->ignore();
            return;
        }
    }

    // Log out before exit
    changeState(OnlineState::Offline);

    // Wait for logout process to finish,
    // but do not wait for too long
    QTimer timer;
    QEventLoop loop(this);
    connect(&globalAccount,
            SIGNAL(setStateFinished(int,bool,OnlineState)),
            &loop,
            SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.setSingleShot(true);
    timer.start(WICHAT_MAIN_TIMER_WAIT_LOGOUT);
    loop.exec();

    if (aboutDialog)
        aboutDialog->close();
    ui->frameSession->close();
    sysTrayIcon->hide();
    sysTrayNoteList->close();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    ui->textFriendSearch->resize(ui->frameFriendSearch->width(),
                                 ui->textFriendSearch->height());
    ui->labelFriendSearch->resize(ui->textFriendSearch->width() - 5,
                                  ui->labelFriendSearch->height());
}

void MainWindow::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)
    init();
}

void MainWindow::addTask(TaskType task)
{
    taskList.enqueue(task);
}

void MainWindow::doTask()
{
    if (taskList.isEmpty())
        return;

    TaskType task = taskList.dequeue();
    switch (task)
    {
        case taskChangeSession:
            changeSession();
            break;
        case taskUpdateState:
            updateState();
            updateCaption();
            break;
        case taskShowNotification:
            showNotification();
            break;
        case taskUpdateFriendList:
            ui->listFriend->updateList();
            break;
        case taskUpdateAll:
            addTask(taskUpdateState);
            addTask(taskUpdateFriendList);
            addTask(taskUpdateFriendInfo);
            addTask(taskUpdateGroupList);
            addTask(taskGetMsgList);
            break;
        case taskGetMsgList:
            ui->frameSession->getMessageList();
            break;
        case taskRebuildConnection:
            ui->frameSession->fixBrokenConnection();
            break;
        case taskUpdateFriendInfo:
            ui->listFriend->updateEntryInfo();
            break;
        case taskUpdateGroupList:
            ui->listGroup->updateList();
            break;
        case taskUpdateGroupInfo:
            ui->listGroup->updateEntryInfo();
            break;
        default:;
    }
}

void MainWindow::changeSession()
{
    int queryID;
    globalAccount.resetSession(queryID);
}

void MainWindow::changeState(OnlineState state)
{
    int queryID;
    globalAccount.setState(state, queryID);
}

void MainWindow::updateState()
{
    int currentState = globalAccount.state();

    ui->frameSession->setTabIconPath(
            ui->frameSession->getSessionIDByID(userID,
                                            SessionFrameWidget::LocalDialog),
            ImageResource::stateToImagePath(currentState, true));
    sysTrayIcon->setIcon(QIcon(ImageResource::stateToImagePath(currentState,
                                                               true)));
    ui->listFriend->setAccountIconPath(userID,
                                       ImageResource::stateToImagePath(
                                                        currentState, true));
    for (int i=0; i<ui->comboState->count(); i++)
    {
        if (ui->comboState->itemData(i).toInt() == currentState)
            ui->comboState->setCurrentIndex(i);
    }
}

void MainWindow::updateCaption()
{
    QString title;
    QString currentSession = ui->frameSession->currentSessionID();
    SessionFrameWidget::SessionType type =
                                    ui->frameSession->sessionType(currentSession);
    if ((currentSession == userID && type == SessionFrameWidget::LocalDialog)
        || currentSession.isEmpty())
    {
        title.append('[')
             .append(globalAccount.state().toString())
             .append(']');
        if (!globalAccount.offlineMsg().isEmpty())
            title.append('-').append(globalAccount.offlineMsg());
    }
    else
    {
        if (type == SessionFrameWidget::GroupChat)
            title = QString("Session with Group %1")
                    .arg(ui->frameSession->getIDBySessionID(currentSession));
        else
            title = QString("Session with %1")
                    .arg(ui->frameSession->getIDBySessionID(currentSession));
    }
    setWindowTitle(title);
}

void MainWindow::updateSysTrayMenu()
{
    QList<QAction*> actionList = menuSysTray->actions();
    if (actionList.isEmpty())
    {
        // Add a title in the tray menu
        QAction* action = new QAction(userID, menuSysTray);
        action->setData(0);
        menuSysTray->addAction(action);
        menuSysTray->addSeparator();

        // Add other options for switching online state
        for (int i=0; i<4; i++)
        {
            action = new QAction(menuSysTray);
            action->installEventFilter(this);
            action->setIcon(QIcon(ImageResource::stateToImagePath(
                                                int(Wichat_Main_stateList[i]),
                                                true)));
            action->setText(Wichat_Main_stateList[i].toString());
            action->setData(Wichat_Main_stateList[i].value);
            menuSysTray->addAction(action);
        }

        // Add an entry for quitting program
        menuSysTray->addSeparator();
        action = new QAction(QIcon(":/Icons/exit.png"),
                             "Quit", menuSysTray);
        action->setData(-1);
        menuSysTray->addAction(action);

        actionList = menuSysTray->actions();
    }
}

void MainWindow::showNotification()
{
    if (noteList.count() < 1)
        return;

    bool clearNote;
    static QString sessionID;
    static QList<Notification::Note> tempNoteList;
    tempNoteList = noteList.getAll();
    for (int i=0; i<tempNoteList.count(); i++)
    {
        clearNote = true; // The read note is removed by default
        switch (tempNoteList[i].type)
        {
            case Notification::FriendAdd:
            case Notification::FriendDelete:
                sysTrayNoteList->addNote(tempNoteList[i]);
                break;
            case Notification::GotMsg:
            case Notification::GotGroupMsg:
                if (tempNoteList[i].type == Notification::GotGroupMsg)
                    sessionID = ui->frameSession->
                                getSessionIDByID(tempNoteList[i].source,
                                                 SessionFrameWidget::GroupChat);
                else
                    sessionID = ui->frameSession->
                                getSessionIDByID(tempNoteList[i].source,
                                                SessionFrameWidget::FriendChat);
                if (sessionID == ui->frameSession->currentSessionID())
                    ui->frameSession->receiveMessage(sessionID);
                else
                {
                    if (ui->frameSession->existSession(sessionID))
                    {
                        // Session exists, highlight it
                        clearNote = false;
                        ui->frameSession->highlightSession(sessionID, true);
                    }
                    else // Otherwise, show notification only
                        sysTrayNoteList->addNote(tempNoteList[i]);
                }
                break;
            default:
                break;
        }
        if (clearNote)
            noteList.remove(tempNoteList[i].ID);
    }

    if (sysTrayNoteList->countNote() > 0)
    {
        // Check static indicator to avoid unnecessary operations
        // as this function is called frequently by timer
        if (notificationState == 0)
        {
            sysTrayIcon->setIcon(QIcon(":/Icons/notification.png"));
            notificationState = 1;
        }
    }
}

void MainWindow::onChangeSessionFinished(int queryID, bool successful)
{
    Q_UNUSED(queryID)
    if (!successful)
        addTask(taskLogOut);
}

void MainWindow::onChangeStateFinished(int queryID,
                                       bool successful,
                                       OnlineState newState)
{
    Q_UNUSED(queryID)
    Q_UNUSED(newState)
    if (!successful)
        QMessageBox::critical(this, "Wichat error",
                              "Cannot set online state.");
    else
        addTask(taskUpdateState);
}

void MainWindow::onFriendRequest(QString ID)
{
    Notification::Note note;
    note.ID = noteList.getNewID();
    note.source = ID;
    note.destination = userID;
    note.type = Notification::FriendAdd;
    noteList.append(note);
}

void MainWindow::onFriendRemoved(QString ID)
{
    Notification::Note note;
    note.ID = noteList.getNewID();
    note.source = ID;
    note.destination = userID;
    note.type = Notification::FriendDelete;
    noteList.append(note);
}

void MainWindow::onTimerTimeout()
{
    static int count;
    count++;

    if (count % WICHAT_MAIN_TIMER_GET_FRIENDLIST == 0)
        addTask(taskUpdateFriendList);
    if (count % WICHAT_MAIN_TIMER_GET_FRIENDINFO == 0)
        addTask(taskUpdateFriendInfo);
    if (count % WICHAT_MAIN_TIMER_GET_GROUPLIST == 10)
        addTask(taskUpdateGroupList);
    if (count % WICHAT_MAIN_TIMER_GET_MSG == 0)
        addTask(taskGetMsgList);
    if (count % WICHAT_MAIN_TIMER_SHOW_NOTE == 0)
        addTask(taskShowNotification);

    doTask();
}

void MainWindow::onListFriendUpdated()
{
    // Update friend remarks and icons in session tab
    QString ID, sessionID, iconPath, remarks;
    QList<QString> friendIDList = ui->listFriend->getAccountIDList();
    for (int i=0; i<friendIDList.count(); i++)
    {
        ID = friendIDList[i];
        sessionID = ui->frameSession->getSessionIDByID(ID,
                                                SessionFrameWidget::FriendChat);
        if (ui->frameSession->existSession(sessionID))
        {
            iconPath = ui->listFriend->getAccountImagePath(ID);
            ui->frameSession->setTabIconPath(sessionID, iconPath);

            remarks = ui->listFriend->getFriendRemark(ID);
            if (remarks.isEmpty())
                ui->frameSession->setTabText(sessionID, ID);
            else
                ui->frameSession->setTabText(sessionID, remarks);
        }
    }
}

void MainWindow::onListFriendEntryClicked(QString ID)
{
    if (ID == userID)
        ui->frameSession->loadSession(ID, SessionFrameWidget::LocalDialog);
    else
        ui->frameSession->loadSession(ID, SessionFrameWidget::FriendChat);
}

void MainWindow::onListGroupEntryClicked(QString ID)
{
    ui->frameSession->loadSession(ID, SessionFrameWidget::GroupChat);
}

void MainWindow::onListGroupUpdated()
{
    // Update group names and icons in session tab
    QString ID, sessionID, iconPath, groupName;
    QList<QString> groupIDList = ui->listGroup->getGroupIDList();
    for (int i=0; i<groupIDList.count(); i++)
    {
        ID = groupIDList[i];
        sessionID = ui->frameSession->getSessionIDByID(ID,
                                                SessionFrameWidget::GroupChat);
        if (ui->frameSession->existSession(sessionID))
        {
            iconPath = ui->listGroup->getGroupImagePath(ID);
            ui->frameSession->setTabIconPath(sessionID, iconPath);

            groupName = ui->listGroup->getGroupName(ID);
            if (groupName.isEmpty())
                ui->frameSession->setTabText(sessionID, ID);
            else
                ui->frameSession->setTabText(sessionID, groupName);
        }
    }
    globalGroup.setGroupList(groupIDList);
}

void MainWindow::onFrameSessionIDChanged(QString ID)
{
    Q_UNUSED(ID)
    updateCaption();
}

void MainWindow::onSysTrayNoteClicked(const Notification::Note& note)
{
    int queryID;
    switch (note.type)
    {
        case Notification::FriendAdd:
            if (QMessageBox::question(this, "New friend request",
                                      QString("%1 wants to add you as friend. "
                                      "Do you accept this request?")
                                      .arg(note.source),
                                      QMessageBox::Yes | QMessageBox::No)
                    == QMessageBox::Yes)
                globalAccount.addFriend(note.source, queryID);
            else
                globalAccount.removeFriend(note.source, queryID);
            break;
        case Notification::FriendDelete:
            QMessageBox::information(this, "Friend removed",
                                     QString("%1 remove you from his/hers "
                                             "friend list.")
                                     .arg(note.source));
            globalAccount.removeFriend(note.source, queryID);
            break;
        case Notification::GotMsg:
            ui->frameSession->loadSession(note.source,
                                          SessionFrameWidget::FriendChat,
                                          true);
            setWindowState(Qt::WindowActive);
            break;
        case Notification::GotGroupMsg:
            ui->frameSession->loadSession(note.source,
                                          SessionFrameWidget::GroupChat,
                                          true);
            setWindowState(Qt::WindowActive);
            break;
        default:;
    }
    sysTrayNoteList->removeNote(note.ID);

    if (sysTrayNoteList->countNote() < 1)
    {
        notificationState = 0;
        sysTrayNoteList->hide();
        addTask(taskUpdateState);
    }
}

void MainWindow::onSysTrayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Context:
            // This does not work
            // updateSysTrayMenu();
            break;
        case QSystemTrayIcon::DoubleClick:
        case QSystemTrayIcon::Trigger:
            if (sysTrayNoteList->countNote() > 0)
            {
                sysTrayNoteList->show();
                sysTrayNoteList->activate();
                QRect iconGeo = sysTrayIcon->geometry();
                if (iconGeo.y() - sysTrayNoteList->height() <= 0)
                    sysTrayNoteList->move(iconGeo.x() -
                                          sysTrayNoteList->width() / 2,
                                          iconGeo.y() +
                                          sysTrayNoteList->height());
                else
                    sysTrayNoteList->move(iconGeo.x() -
                                          sysTrayNoteList->width() / 2,
                                          iconGeo.y() -
                                          sysTrayNoteList->height());
            }
            else
                setWindowState(Qt::WindowActive);
            break;
        default:;
    }
}

void MainWindow::onSysTrayMenuShowed()
{
    updateSysTrayMenu();
}

void MainWindow::onSysTrayMenuClicked(QAction* action)
{
    int state = action->data().toInt();
    if (state == -1)
    {
        manualExit = true;
        close();
    }
    else if (state > 0)
        changeState(OnlineState(action->data().toInt()));
}

void MainWindow::onAppMenuClicked(QAction* action)
{
    switch(action->data().toInt())
    {
        case WICHAT_MAIN_MENU_APP_PREFERENCE:
        {
            if (!globalPreference)
                globalPreference = new PreferenceDialog;
            globalPreference->exec();
            break;
        }
        case WICHAT_MAIN_MENU_APP_ABOUT:
        {
            if (!aboutDialog)
                aboutDialog = new AboutWindow(this);
            aboutDialog->show();
            break;
        }
        case WICHAT_MAIN_MENU_APP_QUIT:
        {
            manualExit = true;
            close();
            break;
        }
        default:;
    }
}

void MainWindow::on_textFriendSearch_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
    {
        ui->labelFriendSearch->show();
        ui->listFriend->setSearchMode(false);
    }
    else
    {
        ui->labelFriendSearch->hide();
        ui->listFriend->search(arg1);
    }
}

void MainWindow::on_buttonFriendAdd_clicked()
{
    QString ID = ui->textFriendSearch->text();
    if (ID.isEmpty())
        return;

    int queryID;
    if (ui->tabChatObjectList->currentIndex() == 0)
    {
    if (QMessageBox::question(this, "Add friend",
                              QString("Do you want to add %1 as friend?")
                              .arg(ID),
                              QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::Yes)
    {
        globalAccount.addFriend(ID, queryID);
        queryList[queryID] = ID;
    }
    }
    else
    {
        if (QMessageBox::question(this, "Join group",
                                  QString("Do you want to join group %1?")
                                  .arg(ID),
                                  QMessageBox::Yes | QMessageBox::No)
                == QMessageBox::Yes)
        {
            globalAccount.joinGroup(ID, queryID);
            queryList[queryID] = ID;
        }
    }
}

void MainWindow::on_buttonWichat_clicked()
{
    if (!menuApp)
    {
        menuApp = new QMenu(this);

        // Action: WICHAT_MAIN_MENU_APP_PREFERENCE
        QAction* action = new QAction(menuApp);
        action->setText("Settings");
        action->setData(WICHAT_MAIN_MENU_APP_PREFERENCE);
        action->setIcon(QIcon(":/Icons/advanced.png"));
        menuApp->addAction(action);

        // Action: WICHAT_MAIN_MENU_APP_ABOUT
        action = new QAction(menuApp);
        action->setText("About Wichat");
        action->setData(WICHAT_MAIN_MENU_APP_ABOUT);
        action->setIcon(QIcon(":/Icons/Wichat.png"));
        menuApp->addAction(action);

        menuApp->addSeparator();

        // Action: WICHAT_MAIN_MENU_APP_QUIT
        action = new QAction(menuApp);
        action->setText("Quit");
        action->setData(WICHAT_MAIN_MENU_APP_QUIT);
        action->setIcon(QIcon(":/Icons/exit.png"));
        menuApp->addAction(action);

        connect(menuApp,
                SIGNAL(triggered(QAction*)),
                this,
                SLOT(onAppMenuClicked(QAction*)));
    }
    menuApp->popup(QCursor::pos());
}

void MainWindow::on_comboState_activated(int index)
{
    bool ok;
    int state = ui->comboState->itemData(index).toInt(&ok);
    if (ok)
        changeState(OnlineState(state));
}
