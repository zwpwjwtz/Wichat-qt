#include <QMessageBox>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QStackedLayout>
#include <QTextBrowser>
#include <QLineEdit>
#include <QScrollBar>
#include <QColorDialog>
#include <QMenu>
#include <QFileDialog>
#include <QDateTime>
#include <QImageReader>
#include <QDesktopServices>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutwindow.h"
#include "accountinfodialog.h"
#include "groupinfodialog.h"
#include "preferencedialog.h"
#include "systraynotification.h"
#include "emoticonchooser.h"
#include "global_objects.h"
#include "imageresource.h"
#include "Modules/account.h"
#include "Modules/conversation.h"
#include "Modules/group.h"
#include "Modules/wichatconfig.h"

#define WICHAT_MAIN_FONT_STYLE_BOLD "bold"
#define WICHAT_MAIN_FONT_STYLE_ITALIC "italic"
#define WICHAT_MAIN_FONT_STYLE_UNDERLINE "underline"
#define WICHAT_MAIN_FONT_STYLE_STRIKEOUT "strikeout"
#define WICHAT_MAIN_FONT_STYLE_OVERLINE "overline"

#define WICHAT_MAIN_TEXT_ALIGN_LEFT "left"
#define WICHAT_MAIN_TEXT_ALIGN_CENTER "center"
#define WICHAT_MAIN_TEXT_ALIGN_RIGHT "right"

#define WICHAT_MAIN_EDITOR_TEXTBOX_NAME "tC"
#define WICHAT_MAIN_EDITOR_SENDKEY_ENTER 0x01000004
#define WICHAT_MAIN_EDITOR_SENDKEY_CTRLENTER 0x01000004 + 0x04000000

#ifdef Q_OS_WIN32
#define WICHAT_MAIN_RESOURCE_DIR "Resource"
#else
#define WICHAT_MAIN_RESOURCE_DIR "/usr/share"
#define WICHAT_MAIN_RESOURCE_DIR_2 "/app/share"
#endif

#define WICHAT_MAIN_MENU_APP_PREFERENCE 3
#define WICHAT_MAIN_MENU_APP_ABOUT 4
#define WICHAT_MAIN_MENU_APP_QUIT 5

#define WICHAT_MAIN_FILE_FILTER_ALL "All (*.*)(*.*)"
#define WICHAT_MAIN_FILE_FILTER_IMAGE "Image file (*.gif *.jpg *.png)(*.gif *.jpg *.png)"
#define WICHAT_MAIN_FILE_FILTER_GIF "GIF file (*.gif)(*.gif)"
#define WICHAT_MAIN_FILE_FILTER_JPG "JPEG file (*.jpg *.jpeg)(*.jpg *.jpeg)"
#define WICHAT_MAIN_FILE_FILTER_PNG "PNG file (*.png)(*.png)"

#define WICHAT_MAIN_TIME_FORMAT "yyyy-MM-dd hh:mm:ss"

#define WICHAT_MAIN_TIMER_GET_FRIENDLIST 30
#define WICHAT_MAIN_TIMER_GET_FRIENDINFO 60
#define WICHAT_MAIN_TIMER_GET_GROUPLIST 120
#define WICHAT_MAIN_TIMER_GET_MSG 10
#ifdef QT_DEBUG
#define WICHAT_MAIN_TIMER_SHOW_NOTE 10
#else
#define WICHAT_MAIN_TIMER_SHOW_NOTE 2
#endif

Account::OnlineState Wichat_stateList[4] = {
        Account::OnlineState::Online,
        Account::OnlineState::Busy,
        Account::OnlineState::Hide,
        Account::OnlineState::Offline
};

QString Wichat_stateToString(Account::OnlineState state)
{
    QString stateString;

    switch (state)
    {
        case Account::OnlineState::Busy:
            stateString = "Busy";
            break;
        case Account::OnlineState::Hide:
            stateString = "Hide";
            break;
        case Account::OnlineState::Online:
            stateString = "Online";
            break;
        case Account::OnlineState::Offline:
        default:
            stateString = "Offline";
    }
    return stateString;
}

const QString Wichat_getHTMLHeader(int docType)
{
    // docType: 0=Content Window; 1=Input Window
    QString buffer("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"
                   "<html xmlns=\"http://www.w3.org/1999/xhtml\">"
                   "<head>"
                   "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
                   "<title></title>");
    switch (docType)
    {
        case 0:
            buffer.append("<style>body{width:360px;overflow:hidden}div,span,ul,li{marin:0px;padding:0px;list-style-type:none;font-size:15px}p{text-align:left}img{border:0px;max-width:100%;height:auto}.s{color:#339966}.r{color:#0066CC}.a{color:#000000}.c{margin:0px 0px 15px 5px;width:350px;word-wrap:break-word}</style></head><body>");
            break;
        case 1:
            buffer.append("<style>body{width:98%;margin:2px;overflow:hidden}div,span,ul,li{marin:0px;padding:0px;list-style-type:none}p{text-align:left;}textarea{width:100%;overflow:hidden;border:0px}</style></head><body><textarea type=text id=" WICHAT_MAIN_EDITOR_TEXTBOX_NAME " cols=38 rows=3>");
            break;
        default:;
    }
    return buffer;
}

const QString Wichat_getHTMLFooter(int docType)
{
    QString buffer;
    switch (docType)
    {
        case 0:
            buffer.append("</body></html>");
            break;
        case 1:
            buffer.append("</textarea></body></html>");
            break;
        default:;
    }
    return buffer;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textBrowser->hide();
    ui->textEdit->hide();
    ui->labelFriendSearch->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->tabSession->removeTab(0);
    ui->frameFont->hide();
    ui->frameMsg->hide();
    ui->frameTextGroup->setLayout(new QStackedLayout);
    ui->frameTextGroup->layout()->setMargin(0);
    for (int i=0; i<4; i++)
        ui->comboState->addItem(QIcon(ImageResource::stateToImagePath(
                                                    int(Wichat_stateList[i]),
                                                    true)),
                                Wichat_stateToString(Wichat_stateList[i]),
                                int(Wichat_stateList[i]));

    menuSysTray = new QMenu();
    sysTrayIcon = new QSystemTrayIcon(this);
    sysTrayIcon->setContextMenu(menuSysTray);
    sysTrayNoteList = new SystrayNotification;
    emoticonList = new EmoticonChooser(this);
    emoticonList->hide();
    buttonTabClose = new QPushButton(QIcon(":/Icons/remove.png"),
                                     "");
    buttonTabClose->setFlat(true);
    buttonTabClose->setIconSize(QSize(16, 16));
    buttonTabClose->resize(20, 20);
    tabBarSession = ui->tabSession->findChild<QTabBar*>();

    connect(&globalAccount,
            SIGNAL(resetSessionFinished(int, bool)),
            this,
            SLOT(onChangeSessionFinished(int, bool)));
    connect(&globalAccount,
            SIGNAL(setStateFinished(int, bool, Account::OnlineState)),
            this,
            SLOT(onChangeStateFinished(int,bool, Account::OnlineState)));
    connect(&globalAccount,
            SIGNAL(friendRequest(QString)),
            this,
            SLOT(onFriendRequest(QString)));
    connect(&globalAccount,
            SIGNAL(friendRemoved(QString)),
            this,
            SLOT(onFriendRemoved(QString)));
    connect(&globalConversation,
            SIGNAL(queryError(int, AbstractChat::QueryError)),
            this,
            SLOT(onConversationQueryError(int, AbstractChat::QueryError)));
    connect(&globalConversation,
            SIGNAL(connectionBroken(QString)),
            this,
            SLOT(onConnectionBroken(QString)));
    connect(&globalConversation,
            SIGNAL(verifyFinished(int, AbstractChat::VerifyError)),
            this,
            SLOT(onConversationVerifyFinished(int, AbstractChat::VerifyError)));
    connect(&globalConversation,
            SIGNAL(resetSessionFinished(int, bool)),
            this,
            SLOT(onResetSessionFinished(int, bool)));
    connect(&globalConversation,
            SIGNAL(sendMessageFinished(int, bool)),
            this,
            SLOT(onSendMessageFinished(int, bool)));
    connect(&globalConversation,
            SIGNAL(getMessageListFinished(int,
                                          QList<AbstractChat::MessageListEntry>&)),
            this,
            SLOT(onGetMessageListFinished(int,
                                          QList<AbstractChat::MessageListEntry>&)));
    connect(&globalConversation,
            SIGNAL(receiveMessageFinished(int,
                                          QList<AbstractChat::MessageEntry>&)),
            this,
            SLOT(onReceiveMessageFinished(int, QList<AbstractChat::MessageEntry>&)));
    connect(&globalGroup,
            SIGNAL(queryError(int, AbstractChat::QueryError)),
            this,
            SLOT(onConversationQueryError(int, AbstractChat::QueryError)));
    connect(&globalGroup,
            SIGNAL(verifyFinished(int, AbstractChat::VerifyError)),
            this,
            SLOT(onConversationVerifyFinished(int, AbstractChat::VerifyError)));
    connect(&globalGroup,
            SIGNAL(resetSessionFinished(int, bool)),
            this,
            SLOT(onResetSessionFinished(int, bool)));
    connect(&globalGroup,
            SIGNAL(sendMessageFinished(int, bool)),
            this,
            SLOT(onSendMessageFinished(int, bool)));
    connect(&globalGroup,
            SIGNAL(getMessageListFinished(int,
                                          QList<AbstractChat::MessageListEntry>&)),
            this,
            SLOT(onGetGroupMessageListFinished(int,
                                          QList<AbstractChat::MessageListEntry>&)));
    connect(&globalGroup,
            SIGNAL(receiveMessageFinished(int,
                                          QList<AbstractChat::MessageEntry>&)),
            this,
            SLOT(onReceiveMessageFinished(int, QList<AbstractChat::MessageEntry>&)));
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
    connect(buttonTabClose,
            SIGNAL(clicked(bool)),
            this,
            SLOT(onSessionTabClose(bool)));
    connect(sysTrayIcon,
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,
            SLOT(onSysTrayIconClicked(QSystemTrayIcon::ActivationReason)));
    connect(sysTrayNoteList,
            SIGNAL(noteClicked(const Notification::Note&)),
            this,
            SLOT(onSysTrayNoteClicked(const Notification::Note&)));
    connect(emoticonList,
            SIGNAL(emoticonClicked(QByteArray)),
            this,
            SLOT(onEmoticonClicked(QByteArray)));
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

    installEventFilter(this);
    ui->listFriend->installEventFilter(this);
    tabBarSession->installEventFilter(this);

    aboutDialog = nullptr;
    dialogColor = nullptr;
    menuApp = nullptr;
    menuFontStyle = nullptr;
    menuTextAlign = nullptr;
    menuSendOption = nullptr;
    groupTextAlign = nullptr;
    groupSendOption = nullptr;
    timer.setInterval(1000);
    manualExit = false;
    lastHoveredTab = -1;
    conversationLoginState = 0;
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

    userSessionList.loadFromFile(globalConfig.userDirectory(userID));
    if (userSessionList.count() < 1)
        loadSession(getSessionIDByID(userID, LocalDialog));
    else
    {
        loadTab();
        loadSession(userSessionList.currentSession().ID, false);
    }

    QDateTime sessionLastTime;
    QList<QString> sessionIDList = userSessionList.sessionIdList();
    lastGroupMsgTime = QDateTime::fromMSecsSinceEpoch(0);
    for (int i=0; i<sessionIDList.count(); i++)
    {
        if (getSessionType(sessionIDList[i]) == GroupChat)
        {
            sessionLastTime = userSessionList.getSession(sessionIDList[i])
                                             .messageList->last().time;
            if (sessionLastTime > lastGroupMsgTime)
                lastGroupMsgTime = sessionLastTime;
        }
    }

    globalConversation.setPeerSession(peerSessionList);
    globalGroup.setPeerSession(peerSessionList);

    QDir resourceDir(WICHAT_MAIN_RESOURCE_DIR);
#ifdef WICHAT_MAIN_RESOURCE_DIR_2
    if (!resourceDir.exists())
        resourceDir.setPath(WICHAT_MAIN_RESOURCE_DIR_2);
#endif
    if (!resourceDir.exists())
        resourceDir = QCoreApplication::applicationDirPath();
    emoticonList->setResourceDir(resourceDir.absolutePath());

    applyUserSettings();

    ui->listFriend->bindService(&globalAccount);
    ui->listGroup->bindService(&globalAccount);

    resizeEvent(0); // Trigger resizing manually
    sysTrayIcon->show();

    addTask(taskUpdateAll);
    timer.start();

    hasInited = true;
}

void MainWindow::setID(QString ID)
{
    userID = ID;
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

    // Log out before exit
    changeState(Account::OnlineState::Offline);
    QEventLoop loop(this);
    connect(&globalAccount,
            SIGNAL(setStateFinished(int,bool,Account::OnlineState)),
            &loop,
            SLOT(quit()));
    loop.exec();

    if (aboutDialog)
        aboutDialog->close();
    sysTrayIcon->hide();
    sysTrayNoteList->close();
    emoticonList->hide();

    syncSessionContent("");
    userSessionList.saveToFile(globalConfig.userDirectory(userID));
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type())
    {
        case QEvent::MouseButtonRelease:
            onMouseButtonRelease();
            break;
//        case QEvent::HoverEnter:
        case QEvent::HoverMove:
            onMouseHoverEnter(watched, (QHoverEvent*)(event));
            break;
        case QEvent::HoverLeave:
            onMouseHoverLeave(watched, (QHoverEvent*)(event));
            break;
        case QEvent::KeyRelease:
            onKeyRelease(watched, (QKeyEvent*)(event));
            break;
        default:;
    }
    return QObject::eventFilter(watched, event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    ui->tabSession->resize(ui->tabSession->width(),
                           ui->frameTextCtrl->y() - ui->tabSession->y());
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
            getMessageList();
            break;
        case taskReloadMsg:
            loadSessionContent(lastConversation);
            break;
        case taskRebuildConnection:
            fixBrokenConnection();
            break;
        case taskConversationLogin:
            conversationLogin();
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

void MainWindow::applyFont()
{
    QFont font;
    QString temp;
    temp = globalConfig.prefFontStyle(userID);
    if (temp.contains(WICHAT_MAIN_FONT_STYLE_BOLD))
        font.setBold(true);
    if (temp.contains(WICHAT_MAIN_FONT_STYLE_ITALIC))
        font.setItalic(true);
    if (temp.contains(WICHAT_MAIN_FONT_STYLE_UNDERLINE))
        font.setUnderline(true);
    if (temp.contains(WICHAT_MAIN_FONT_STYLE_STRIKEOUT))
        font.setStrikeOut(true);
    if (temp.contains(WICHAT_MAIN_FONT_STYLE_OVERLINE))
        font.setOverline(true);

    temp = globalConfig.prefFontFamily(userID);
    if (!temp.isEmpty())
        font.setFamily(globalConfig.prefFontFamily(userID));

    font.setPointSize(globalConfig.prefFontSize(userID).toInt());

    for (int i=0; i<editorList.count(); i++)
    {
        editorList[i]->setFont(font);
        editorList[i]->setTextColor(globalConfig.prefFontColor(userID));
        temp = globalConfig.prefTextAlign(userID);
        if (temp == WICHAT_MAIN_TEXT_ALIGN_CENTER)
            editorList[i]->setAlignment(Qt::AlignCenter);
        else if (temp == WICHAT_MAIN_TEXT_ALIGN_RIGHT)
            editorList[i]->setAlignment(Qt::AlignRight);
        else
            editorList[i]->setAlignment(Qt::AlignLeft);
    }
}

void MainWindow::applyUserSettings()
{
    globalConversation.setUserDirectory(globalConfig.userDirectory(userID));
    globalGroup.setUserDirectory(globalConfig.userDirectory(userID));

    int sendKey = globalConfig.prefSendKey(userID);
    if (!(sendKey == WICHAT_MAIN_EDITOR_SENDKEY_ENTER ||
          sendKey == WICHAT_MAIN_EDITOR_SENDKEY_CTRLENTER))
        globalConfig.setPrefSendKey(userID, WICHAT_MAIN_EDITOR_SENDKEY_ENTER);
}

int MainWindow::getSessionIndex(QString sessionID)
{
    for (int i=0; i<browserList.count(); i++)
    {
        if (browserList[i]->property("ID").toString() == sessionID)
            return i;
    }
    return -1;
}

int MainWindow::getSessionTabIndex(QString sessionID)
{
    for (int i=0; i<ui->tabSession->count(); i++)
    {
        if (((QTextBrowser*)(ui->tabSession->widget(i)))
                                    ->property("ID").toString() == sessionID)
            return i;
    }
    return -1;
}

MainWindow::SessionType MainWindow::getSessionType(QString sessionID)
{
    if (sessionID.indexOf('L') == 0)
        return LocalDialog;
    else if (sessionID.indexOf('G') == 0)
        return GroupChat;
    else
        return FriendChat;
}

QString MainWindow::getIDBySessionID(QString sessionID)
{
    if (sessionID.indexOf('L') == 0 || sessionID.indexOf('G') == 0)
        return sessionID.mid(1);
    else
        return sessionID;
}

QString MainWindow::getSessionIDByID(QString ID, SessionType type)
{
    if (type == LocalDialog)
        return ID.prepend('L');
    else if (type == GroupChat)
        return ID.prepend('G');
    else
        return ID;
}

// Load new session, and make it visible to user
void MainWindow::loadSession(QString sessionID, bool setTabActive)
{
    if (sessionID.isEmpty())
        return;

    int index = getSessionTabIndex(sessionID);
    if (index == -1)
    {
        // Create new session
        if (!userSessionList.exists(sessionID))
            userSessionList.add(sessionID);
        addTab(sessionID);
    }

    // Clear possible notifications of in-coming messages
    bool receivedMsg = false;
    QList<Notification::Note> notes = noteList.getAll();
    for (int i=0; i<notes.count(); i++)
    {
        if (notes[i].source == sessionID &&
            (notes[i].type == Notification::GotMsg ||
             notes[i].type == Notification::GotGroupMsg))
        {
            noteList.remove(notes[i].ID);
            receivedMsg = true;
        }
    }
    if (receivedMsg && sessionID != userID)
        receiveMessage(sessionID);

    // Change session tab index if necessary
    index = getSessionTabIndex(sessionID);
    if (setTabActive)
    {
        ui->tabSession->setCurrentIndex(index);
    }
    if (lastConversation == sessionID)
        return;
    highlightSession(sessionID, false);

    // Save old session if necessary
    UserSession::SessionData& oldSession =
                                userSessionList.getSession(lastConversation);
    if (!oldSession.ID.isEmpty())
    {
        int oldIndex = getSessionIndex(oldSession.ID);
        if (oldIndex >= 0)// && oldIndex != ui->tabSession->currentIndex())
        {
            syncSessionContent(oldSession.ID);
        }
    }

    // Load and activate the session
    index = getSessionIndex(sessionID);
    if (!browserList[index]->property("Initialized").toBool())
        return;
    if (!browserList[index]->property("Loaded").toBool())
    {
        loadSessionContent(sessionID);
        browserList[index]->setProperty("Loaded", true);
    }
    ((QStackedLayout*)(ui->frameTextGroup->layout()))->setCurrentIndex(index);
    lastConversation = sessionID;

    updateCaption();
}

void MainWindow::loadSessionContent(QString sessionID)
{
    UserSession::SessionData& session = userSessionList.getSession(sessionID);
    session.active = true;

    // Load cached session content from session data
    int tabIndex = getSessionIndex(sessionID);
    QString buffer;
    QList<SessionMessageList::MessageEntry> messages =
                                    session.messageList->getAll();
    buffer.append(Wichat_getHTMLHeader(0));
    for (int i=0; i<messages.count(); i++)
        buffer.append(renderMessage(messages[i]));
    buffer.append(Wichat_getHTMLFooter(0));
    browserList[tabIndex]->setHtml(buffer);

    // Load text input area from session data
    buffer.clear();
    buffer.append(session.input);
    editorList[tabIndex]->setHtml(buffer);
    applyFont();

    // Process possible events (redrawing etc.)
    QCoreApplication::processEvents();

    // Scroll both area to the end
    browserList[tabIndex]->verticalScrollBar()->setValue(
                    browserList[tabIndex]->verticalScrollBar()->maximum());
    editorList[tabIndex]->verticalScrollBar()->setValue(
                    editorList[tabIndex]->verticalScrollBar()->maximum());
}

void MainWindow::syncSessionContent(QString sessionID, bool closeSession)
{
    int index;

    if (sessionID.isEmpty())
    {
        index = ui->tabSession->currentIndex();
        if (index < 0)
           return;
        sessionID = browserList[index]->property("ID").toString();
    }

    index = getSessionIndex(sessionID);
    UserSession::SessionData& session = userSessionList.getSession(sessionID);
    if (session.ID == sessionID)
    {
        // Store content in input box to session cache
        session.input = editorList[index]->toHtml().toUtf8();
        session.active = !closeSession;
    }
}


void MainWindow::addTab(QString sessionID)
{
    QTextBrowser* newBrowser = new QTextBrowser;
    QTextEdit* newEditor = new QTextEdit;

    newBrowser->setProperty("ID", sessionID);
    newBrowser->setProperty("Initialized", false);
    newBrowser->setProperty("Loaded", false);
    newBrowser->setOpenLinks(false);
    newBrowser->setGeometry(ui->textBrowser->geometry());
    newEditor->setProperty("ID", sessionID);
    newEditor->setGeometry(ui->textEdit->geometry());

    browserList.push_back(newBrowser);
    editorList.push_back(newEditor);

    connect(newBrowser,
            SIGNAL(anchorClicked(const QUrl&)),
            this,
            SLOT(onBrowserLinkClicked(const QUrl&)));

    QString tabText;
    if (getSessionType(sessionID) == FriendChat)
        tabText = ui->listFriend->getFriendRemark(getIDBySessionID(sessionID));
    if (tabText.isEmpty())
        tabText = getIDBySessionID(sessionID);
    ui->tabSession->addTab(newBrowser,
                           QIcon(getStateImagePath(sessionID)),
                           tabText);
    ui->frameTextGroup->layout()->addWidget(newEditor);
    ui->frameMsg->show();

    newBrowser->setProperty("Initialized", true);
}

// Initialize session tab with given session list
// Do not touch any session content
void MainWindow::loadTab()
{
    QList<QString> sessionIdList = userSessionList.sessionIdList();

    ui->tabSession->clear();

    for (int i=0; i<sessionIdList.count(); i++)
    {
        if (userSessionList.getSession(sessionIdList[i]).active)
            addTab(sessionIdList[i]);
    }
}

void MainWindow::highlightSession(QString sessionID, bool highlight)
{
    int index = getSessionTabIndex(sessionID);
    if (index < 0)
        return;
    if (highlight)
    {
        tabBarSession->setTabTextColor(index, QColor("#FFA07A"));
        setWindowTitle(QString("Message from %1")
                              .arg(getIDBySessionID(sessionID)));
        QApplication::alert(this);
    }
    else
    {
        tabBarSession->setTabTextColor(index, QColor(""));
        updateCaption();
    }
}

void MainWindow::removeTab(QString sessionID)
{
    syncSessionContent(sessionID, true);
    lastConversation.clear();

    // Actually close the tab
    int index = getSessionIndex(sessionID);
    disconnect(browserList[index],
               SIGNAL(anchorClicked(const QUrl&)),
               this,
               SLOT(onBrowserLinkClicked(const QUrl&)));
    browserList.removeAt(index);
    delete editorList[index];
    editorList.removeAt(index);
    ui->tabSession->removeTab(getSessionTabIndex(sessionID));
    if (editorList.count() < 1)
    {
        ui->frameMsg->hide();

        // Trigger resizing manually
        QCoreApplication::processEvents();
        resizeEvent(0);
    }
}

void MainWindow::changeSession()
{
    int queryID;
    globalAccount.resetSession(queryID);
}

void MainWindow::changeState(Account::OnlineState state)
{
    int queryID;
    globalAccount.setState(state, queryID);
}

void MainWindow::updateState()
{
    ui->tabSession->setTabIcon(getSessionTabIndex(userID),
                               QIcon(ImageResource::stateToImagePath(
                                            int(globalAccount.state()), true)));
    sysTrayIcon->setIcon(QIcon(ImageResource::stateToImagePath(
                                   int(globalAccount.state()), true)));
    ui->listFriend->setAccountIconPath(userID,
                                       ImageResource::stateToImagePath(
                                            int(globalAccount.state()), true));
}

void MainWindow::updateCaption()
{
    QString title;
    if (lastConversation == userID || lastConversation.isEmpty())
    {
        title.append('[')
             .append(Wichat_stateToString(globalAccount.state()))
             .append(']');
        if (!globalAccount.offlineMsg().isEmpty())
            title.append('-').append(globalAccount.offlineMsg());
    }
    else
        title = QString("Session with %1")
                .arg(getIDBySessionID(lastConversation));
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
                                                   int(Wichat_stateList[i]),
                                                   true)));
            action->setText(Wichat_stateToString(Wichat_stateList[i]));
            action->setData(int(Wichat_stateList[i]));
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

QString MainWindow::getStateImagePath(QString sessionID)
{
    QString image;
    if (getIDBySessionID(sessionID) == userID)
        image = ImageResource::stateToImagePath(int(globalAccount.state()),
                                                true);
    else switch (getSessionType(sessionID))
    {
        case LocalDialog:
            image = ImageResource::stateToImagePath(
                                        int(Account::OnlineState::Online));
            break;
        case GroupChat:
            image = ImageResource::getGroupImagePath();
            break;
        case FriendChat:
            image = ui->listFriend->getAccountImagePath(
                                                getIDBySessionID(sessionID));
            break;
        default:
            image = ImageResource::stateToImagePath(
                                        int(Account::OnlineState::Offline));
    }
    return image;
}


void MainWindow::conversationLogin()
{
    switch (conversationLoginState)
    {
        case 0: // Not logged in
            globalConversation.verify(globalAccount.sessionID(),
                                  globalAccount.sessionKey());
            globalGroup.verify(globalAccount.sessionID(),
                               globalAccount.sessionKey());
            conversationLoginState = 1;
        case 1: // Waiting for server response
            // Do not use QEventloop here, as it might provoke deadlook
            break;
        case 2: // Logged in successfully
        default:
            break;
    }
}

void MainWindow::getMessageList()
{
    int queryID;
    conversationLogin();
    globalConversation.getMessageList(queryID);
    globalGroup.getMessageList(lastGroupMsgTime, queryID);
}

bool MainWindow::sendMessage(QString content, QString sessionID)
{
#ifndef QT_DEBUG
    if (globalAccount.state() == Account::OnlineState::None ||
        globalAccount.state() == Account::OnlineState::Offline)
    {
        QMessageBox::warning(this, "Please log in first",
                             "You are off-line now. "
                             "Please log in to send messages.");
        return false;
    }
#endif

    int sessionIndex = getSessionIndex(sessionID);
    if (sessionIndex < 0)
        return false;

    int queryID;
    SessionType messageType(getSessionType(sessionID));
    QString ID(getIDBySessionID(sessionID));
    QByteArray buffer(addSenderInfo(content, userID).toUtf8());
    bool successful;
    if (ID != userID)
    {
        if (messageType == FriendChat && !ui->listFriend->hasMember(ID))
        {
            QMessageBox::warning(this, "Unable send message",
                                 QString("You are not friend to %1.\n"
                                 "Please add %1 to your friend list before "
                                 "sending any message.")
                                 .arg(ID).arg(ID));
            return false;
        }

        conversationLogin();
        if (messageType == GroupChat)
            successful = globalGroup.sendMessage(ID, buffer, queryID);
        else
            successful = globalConversation.sendMessage(ID, buffer, queryID);
        if (!successful)
        {
            QMessageBox::warning(this, "Cannot send message",
                                 "Wichat is currently unable to send message.");
            return false;
        }
        else
            queryList[queryID] = sessionID;
    }

    SessionMessageList::MessageEntry sessionMessage;
    sessionMessage.time = QDateTime::currentDateTimeUtc();
    sessionMessage.type = SessionMessageList::TextMessage;
    sessionMessage.source = userID;
    sessionMessage.content = buffer;
    userSessionList.getSession(sessionID).messageList
                                         ->addMessage(sessionMessage);

    browserList[sessionIndex]->append(renderMessage(sessionMessage, true));
    return true;
}

bool MainWindow::receiveMessage(QString sessionID)
{
    int queryID;
    QString ID(getIDBySessionID(sessionID));
    SessionType type(getSessionType(sessionID));

    conversationLogin();
    if (type == FriendChat)
        globalConversation.receiveMessage(ID, queryID);
    else if (type == GroupChat)
    {
        globalGroup.receiveMessage(ID, lastGroupMsgTime, queryID);
        lastGroupMsgTime = QDateTime::currentDateTimeUtc();
    }
    else
        return false;

    queryList[queryID] = sessionID;
    return true;
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
                    sessionID = getSessionIDByID(tempNoteList[i].source,
                                                 GroupChat);
                else
                    sessionID = getSessionIDByID(tempNoteList[i].source,
                                                 FriendChat);
                if (sessionID == lastConversation)
                    receiveMessage(lastConversation);
                else
                {
                    int index = getSessionIndex(sessionID);
                    if (index >= 0) // Session exists, highlight it
                    {
                        clearNote = false;
                        highlightSession(sessionID, true);
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

void MainWindow::fixBrokenConnection()
{
    int queryID;
    QString sessionID = brokenConnectionList.dequeue();
    QString ID = getIDBySessionID(sessionID);

    conversationLogin();
    globalConversation.fixBrokenConnection(ID, queryID);
    queryList[queryID] = sessionID;
}

QString MainWindow::addSenderInfo(const QString& content, QString ID)
{
    QString configString;
    QString styleStringHeader, styleStringFooter;

    // Set up <font> tag
    configString = globalConfig.prefFontFamily(ID);
    if (!configString.isEmpty())
        styleStringHeader.append("face=\"")
                        .append(configString)
                        .append("\"");
    configString = globalConfig.prefFontColor(ID);
    if (!configString.isEmpty())
        styleStringHeader.append(" color=\"")
                        .append(configString)
                        .append("\"");
    configString = globalConfig.prefFontSize(ID);
    if (!configString.isEmpty())
        styleStringHeader.append("style=\"font-size:")
                        .append(configString)
                        .append("pt\"");
    if (!styleStringHeader.isEmpty())
    {
        styleStringHeader.prepend("<font ").append('>');
        styleStringFooter = "</font>";
    }
    configString = globalConfig.prefFontStyle(ID);
    if (configString.contains(WICHAT_MAIN_FONT_STYLE_BOLD))
    {
        styleStringHeader.append("<b>");
        styleStringFooter.prepend("</b>");
    }
    if (configString.contains(WICHAT_MAIN_FONT_STYLE_ITALIC))
    {
        styleStringHeader.append("<i>");
        styleStringFooter.prepend("</i>");
    }
    if (configString.contains(WICHAT_MAIN_FONT_STYLE_UNDERLINE))
    {
        styleStringHeader.append("<u>");
        styleStringFooter.prepend("</u>");
    }
    if (configString.contains(WICHAT_MAIN_FONT_STYLE_STRIKEOUT))
    {
        styleStringHeader.append("<del>");
        styleStringFooter.prepend("</del>");
    }
    if (configString.contains(WICHAT_MAIN_FONT_STYLE_OVERLINE))
    {
        styleStringHeader.append("<span style=\"text-decoration:overline\">");
        styleStringFooter.prepend("</span>");
    }

    // Set up alignment tag
    configString = globalConfig.prefTextAlign(ID);
    if (configString == WICHAT_MAIN_TEXT_ALIGN_CENTER)
        styleStringHeader.prepend(" align=center>");
    else if (configString == WICHAT_MAIN_TEXT_ALIGN_RIGHT)
        styleStringHeader.prepend(" align=right>");
    else
        styleStringHeader.prepend(">");

    styleStringHeader.prepend("<div class=c");
    styleStringFooter.append("</div>");

    return styleStringHeader.append(content).append(styleStringFooter);
}

QString MainWindow::renderMessage(const SessionMessageList::MessageEntry& message,
                                  bool fullHTML)
{
    int p1, p2;
    QString temp;
    QString header("<div class=%SENDER%><b>%ID%</b>&nbsp;&nbsp;%TIME%</div>");
    QString result(message.content);

    if (message.source == userID)
        header.replace("%SENDER%", "s");
    else
        header.replace("%SENDER%", "r");
    header.replace("%ID%", message.source);
    header.replace("%TIME%", message.time.toString(WICHAT_MAIN_TIME_FORMAT));

    // Deal with emoticon
    p2 = 0;
    while(true)
    {
        p1 = result.indexOf("<emotion>", p2);
        p2 = result.indexOf("</emotion>", p1);
        if (p1 < 0 || p2 < 0)
            break;
        int emotionIndex = int(QString(result.mid(p1 + 9, p2 - p1 - 9))
                                      .toInt());
        // TODO: parse emoticon

        result.replace(p1, p2 - p1, temp);
    }

    // Deal with image
    p2 = 0;
    while(true)
    {
        p1 = result.indexOf("<file><type>i</type><name>", p2);
        p2 = result.indexOf("</name></file>", p1);
        if (p1 < 0 || p2 < 0)
            break;
        QString fileName = result.mid(p1 + 26, p2 - p1 - 26);
        QImageReader image(fileName);
        int displayWidth = image.size().width();
        if (displayWidth > ui->textBrowser->width())
            displayWidth = ui->textBrowser->width() - 20;
        temp = QString("<img src=\"")
                    .append(fileName)
                    .append("\" alt=Image width=%1 />")
                    .arg(QString::number(displayWidth));

        result.replace(p1, p2 - p1, temp);
    }

    // Deal with file
    p2 = 0;
    while(true)
    {
        p1 = result.indexOf("<file><type>f</type><name>", p2);
        p2 = result.indexOf("</name></file>", p1);
        if (p1 < 0 || p2 < 0)
            break;
        QString fileName = result.mid(p1 + 26, p2 - p1 - 26);

        QString noteText;
        if (message.source == userID)
            noteText = QString("You sent file \"%1\" to him/her.")
                              .arg(getFileNameFromPath(fileName));
        else
            noteText = QString("He/she sent file \"%1\" to you.")
                              .arg(getFileNameFromPath(fileName));
        temp = QString("<div style=\"width:300px;border:1px solid;\">"
                          "<div style=\"float:right\">")
                    .append(noteText)
                    .append("<a href=\"")
                    .append(fileName)
                    .append("\" target=_blank>View the file</a></div></div>");

        result.replace(p1, p2 - p1, temp);
    }

    result.prepend(header);
    if (fullHTML)
    {
        result.prepend(Wichat_getHTMLHeader(0));
        result.append(Wichat_getHTMLFooter(0));
    }
    return result;
}

QString MainWindow::extractHTMLTag(const QString& rawHTML, QString tagName)
{
    int p1, p2, p3;
    QString tagBegin(tagName), tagEnd(tagName);
    tagBegin.prepend('<');
    tagEnd.prepend("</").append('>');

    p1 = rawHTML.indexOf(tagBegin, 0, Qt::CaseInsensitive);
    p2 = rawHTML.indexOf('>', p1);
    p3 = rawHTML.indexOf(tagEnd, 0, Qt::CaseInsensitive);
    if (p1 >= 0 && p2 > p1 && p3 > p2)
        return rawHTML.mid(p2 + 1, p3 - p2 - 1);
    else
        return "";
}


QString MainWindow::getFileNameFromPath(QString filePath)
{
    char pathSep;
    if (filePath.indexOf('/') >= 0)
        pathSep  = '/';
    else if (filePath.indexOf('\\') >= 0)
        pathSep  = '\\';
    else
        return filePath;
    return filePath.mid(filePath.lastIndexOf(pathSep) + 1);
}

void MainWindow::onChangeSessionFinished(int queryID, bool successful)
{
    Q_UNUSED(queryID)
    if (!successful)
        addTask(taskLogOut);
}

void MainWindow::onChangeStateFinished(int queryID,
                                       bool successful,
                                       Account::OnlineState newState)
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

void MainWindow::onConversationQueryError(int queryID,
                                          AbstractChat::QueryError errCode)
{
    Q_UNUSED(queryID)
    Q_UNUSED(errCode)
    conversationLoginState = 0;
}

void MainWindow::onConnectionBroken(QString ID)
{
    brokenConnectionList.enqueue(ID);
    addTask(taskRebuildConnection);
}

void MainWindow::onConversationVerifyFinished(int queryID,
                                            AbstractChat::VerifyError errorCode)
{
    Q_UNUSED(queryID)
    if (errorCode == AbstractChat::VerifyError::Ok)
    {
        conversationLoginState = 2;
        peerSessionList.loadSessionKey(globalConfig.userDirectory(userID),
                                       globalConversation.keySalt());
    }
    else
        conversationLoginState = 0;
}

void MainWindow::onResetSessionFinished(int queryID, bool successful)
{
    Q_UNUSED(queryID)
    if (!successful)
        addTask(taskConversationLogin);
}

void MainWindow::onSendMessageFinished(int queryID, bool successful)
{
    if (successful)
        return;

    QString destination = queryList.value(queryID);
    queryList.remove(queryID);
    if (!destination.isEmpty())
        QMessageBox::warning(this, "Failed to send message",
                             QString("Wichat failed to send message to %1.\n"
                             "Please check your network, then try again.")
                             .arg(getIDBySessionID(destination)));
}

void MainWindow::onGetMessageListFinished(int queryID,
                                          QList<AbstractChat::MessageListEntry>& msgList)
{
    Q_UNUSED(queryID)
    Notification::Note newNote;
    newNote.destination = userID;
    newNote.type = Notification::GotMsg;
    newNote.time = QDateTime::currentDateTime();
    for (int i=0; i<msgList.count(); i++)
    {
        newNote.source = msgList[i].ID;
        newNote.ID = noteList.getNewID();
        noteList.append(newNote);
    }
}

void MainWindow::onGetGroupMessageListFinished(int queryID,
                                QList<AbstractChat::MessageListEntry>& msgList)
{
    Q_UNUSED(queryID)
    Notification::Note newNote;
    newNote.destination = userID;
    newNote.type = Notification::GotGroupMsg;
    newNote.time = QDateTime::currentDateTime();
    for (int i=0; i<msgList.count(); i++)
    {
        newNote.source = msgList[i].ID;
        newNote.ID = noteList.getNewID();
        noteList.append(newNote);
    }
}

void MainWindow::onReceiveMessageFinished(int queryID,
                                          QList<AbstractChat::MessageEntry>& messages)
{
    QString sourceID = queryList.value(queryID);
    if (sourceID.isEmpty())
        return;
    queryList.remove(queryID);

    QString htmlBuffer;
    SessionMessageList::MessageEntry sessionMessage;
    SessionMessageList* messageList =
                            userSessionList.getSession(sourceID).messageList;
    for (int i=0; i<messages.count(); i++)
    {
        if (messages[i].source == userID)
            continue;
        sessionMessage.source = messages[i].source;
        sessionMessage.time = messages[i].time;
        sessionMessage.type = SessionMessageList::TextMessage;
        sessionMessage.content = messages[i].content;
        messageList->addMessage(sessionMessage);

        htmlBuffer.append(renderMessage(sessionMessage));
    }
    if (!htmlBuffer.isEmpty())
    {
        htmlBuffer.prepend(Wichat_getHTMLHeader(0));
        htmlBuffer.append(Wichat_getHTMLFooter(0));
        browserList[getSessionIndex(sourceID)]->append(htmlBuffer);
    }
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

void MainWindow::onMouseButtonRelease()
{
    ui->frameFont->hide();
    emoticonList->hide();
}

void MainWindow::onMouseHoverEnter(QObject* watched, QHoverEvent* event)
{
    Q_UNUSED(event)
    if (watched == tabBarSession && lastHoveredTab < 0)
    {
        int index = tabBarSession->tabAt(
                                tabBarSession->mapFromGlobal(QCursor::pos()));
        if (index < 0)
            return;

        lastHoveredTab = index;
        tabBarSession->setTabButton(index, QTabBar::RightSide, buttonTabClose);
    }
}

void MainWindow::onMouseHoverLeave(QObject* watched, QHoverEvent* event)
{
    Q_UNUSED(event)
    if (watched == tabBarSession && lastHoveredTab >= 0)
    {
        tabBarSession->setTabButton(lastHoveredTab,
                                               QTabBar::RightSide,
                                               nullptr);
        lastHoveredTab = -1;
    }
}

void MainWindow::onKeyRelease(QObject* watched, QKeyEvent* event)
{
    Q_UNUSED(watched)
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        if (lastConversation.isEmpty())
            return;
        if (editorList[getSessionIndex(lastConversation)]->hasFocus())
        {
            if ((globalConfig.prefSendKey(userID) ==
                                WICHAT_MAIN_EDITOR_SENDKEY_ENTER &&
                 event->modifiers() == Qt::NoModifier) ||
                (globalConfig.prefSendKey(userID) ==
                                WICHAT_MAIN_EDITOR_SENDKEY_CTRLENTER &&
                 event->modifiers() == Qt::ControlModifier))
            {
                on_buttonSend_clicked();
            }
        }
    }
    else if (event->key() == Qt::Key_Escape)
    {
        onMouseButtonRelease();
    }
}

void MainWindow::onSessionTabClose(bool checked)
{
    Q_UNUSED(checked)
    int index = tabBarSession->tabAt(
                                tabBarSession->mapFromGlobal(QCursor::pos()));
    tabBarSession->setTabButton(index,
                                           QTabBar::RightSide,
                                           nullptr);
    removeTab(ui->tabSession->widget(index)->property("ID").toString());
    updateCaption();
}


void MainWindow::onListFriendUpdated()
{
    // Update friend remarks and icons in session tab
    int index;
    QString ID, iconPath, remarks;
    QList<QString> friendIDList = ui->listFriend->getAccountIDList();
    for (int i=0; i<friendIDList.count(); i++)
    {
        ID = friendIDList[i];
        index = getSessionTabIndex(getSessionIDByID(ID, FriendChat));
        if (index >= 0)
        {
            iconPath = ui->listFriend->getAccountImagePath(ID);
            ui->tabSession->setTabIcon(index, QIcon(iconPath));

            remarks = ui->listFriend->getFriendRemark(ID);
            if (remarks.isEmpty())
                ui->tabSession->setTabText(index, ID);
            else
                ui->tabSession->setTabText(index, remarks);
        }
    }
}

void MainWindow::onListFriendEntryClicked(QString ID)
{
    if (ID == userID)
        loadSession(getSessionIDByID(ID, LocalDialog));
    else
        loadSession(getSessionIDByID(ID, FriendChat));
}

void MainWindow::onListGroupEntryClicked(QString ID)
{
    loadSession(getSessionIDByID(ID, GroupChat));
}

void MainWindow::onListGroupUpdated()
{
    // Update group names and icons in session tab
    int index;
    QString ID, iconPath, groupName;
    QList<QString> groupIDList = ui->listGroup->getGroupIDList();
    for (int i=0; i<groupIDList.count(); i++)
    {
        ID = groupIDList[i];
        index = getSessionTabIndex(getSessionIDByID(ID, GroupChat));
        if (index >= 0)
        {
            iconPath = ui->listGroup->getGroupImagePath(ID);
            ui->tabSession->setTabIcon(index, QIcon(iconPath));

            groupName = ui->listGroup->getGroupName(ID);
            if (groupName.isEmpty())
                ui->tabSession->setTabText(index, ID);
            else
                ui->tabSession->setTabText(index, groupName);
        }
    }
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
            loadSession(getSessionIDByID(note.source, FriendChat), true);
            setWindowState(Qt::WindowActive);
            break;
        case Notification::GotGroupMsg:
            loadSession(getSessionIDByID(note.source, GroupChat), true);
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
        changeState(Account::OnlineState(action->data().toInt()));
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

void MainWindow::onEmoticonClicked(const QByteArray &emoticon)
{
    const uint* unicode = (const uint*)(emoticon.constData());
    int unicodeLength = emoticon.length() / sizeof(uint) +
                        (emoticon.length() % sizeof(uint) > 0);
    editorList[getSessionIndex(lastConversation)]->insertHtml(
                                            QString::fromUcs4(unicode,
                                                              unicodeLength));
}

void MainWindow::onFontStyleMenuClicked(QAction* action)
{
    // Store new style to user config
    QStringList fontStyleArgs(globalConfig.prefFontStyle(userID)
                              .split(',', QString::SkipEmptyParts)
                              .toVector().toList());
    QString arg(action->data().toString());
    if (action->isChecked())
    {
        if (!fontStyleArgs.contains(arg))
            fontStyleArgs.push_back(arg);
    }
    else
    {
        if (fontStyleArgs.contains(arg))
            fontStyleArgs.removeAll(arg);
    }
    globalConfig.setPrefFontStyle(userID, fontStyleArgs.join(","));
    applyFont();
}

void MainWindow::onBrowserLinkClicked(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

void MainWindow::onTextAlignMenuClicked(QAction* action)
{
    // Store new style to user config
    QString alignMode;
    if (action->isChecked())
        alignMode = action->data().toString();
    globalConfig.setPrefTextAlign(userID, alignMode);
    applyFont();
}

void MainWindow::onSendOptionMenuClicked(QAction* action)
{
    // Store new style to user config
    int keyCode = WICHAT_MAIN_EDITOR_SENDKEY_ENTER;
    if (action->isChecked())
        keyCode = action->data().toInt();
    globalConfig.setPrefSendKey(userID, keyCode);
}

void MainWindow::on_buttonFont_clicked()
{
    if (ui->frameFont->isVisible())
        ui->frameFont->hide();
    else
        ui->frameFont->show();
}

void MainWindow::on_buttonTextColor_clicked()
{
    if (!dialogColor)
        dialogColor = new QColorDialog;
    dialogColor->setCurrentColor(QColor(globalConfig.prefFontColor(userID)));
    dialogColor->exec();

    globalConfig.setPrefFontColor(userID,
                                  dialogColor->currentColor().name());
    applyFont();
}

void MainWindow::on_buttonTextStyle_clicked()
{
    QList<QAction*> actionList;
    QFont fontStyle;
    QStringList fontStyleArgs;

    if (!menuFontStyle)
    {
        // Create font style menu with previewed effect
        menuFontStyle = new QMenu;

        actionList.append(menuFontStyle->addAction("Bold"));
        actionList[0]->setCheckable(true);
        actionList[0]->setData(WICHAT_MAIN_FONT_STYLE_BOLD);
        fontStyle.setBold(true);
        actionList[0]->setFont(fontStyle);
        fontStyle.setBold(false);

        actionList.append(menuFontStyle->addAction("Italic"));
        actionList[1]->setCheckable(true);
        actionList[1]->setData(WICHAT_MAIN_FONT_STYLE_ITALIC);
        fontStyle.setItalic(true);
        actionList[1]->setFont(fontStyle);
        fontStyle.setItalic(false);

        actionList.append(menuFontStyle->addAction("Underline"));
        actionList[2]->setCheckable(true);
        actionList[2]->setData(WICHAT_MAIN_FONT_STYLE_UNDERLINE);
        fontStyle.setUnderline(true);
        actionList[2]->setFont(fontStyle);
        fontStyle.setUnderline(false);

        actionList.append(menuFontStyle->addAction("StrikeOut"));
        actionList[3]->setCheckable(true);
        actionList[3]->setData(WICHAT_MAIN_FONT_STYLE_STRIKEOUT);
        fontStyle.setStrikeOut(true);
        actionList[3]->setFont(fontStyle);
        fontStyle.setStrikeOut(false);

        actionList.append(menuFontStyle->addAction("Overline"));
        actionList[4]->setCheckable(true);
        actionList[4]->setData(WICHAT_MAIN_FONT_STYLE_OVERLINE);
        fontStyle.setOverline(true);
        actionList[4]->setFont(fontStyle);
        fontStyle.setOverline(false);

        connect(menuFontStyle,
                SIGNAL(triggered(QAction*)),
                this,
                SLOT(onFontStyleMenuClicked(QAction*)));
    }

        // Parse font style arguments from user config
        fontStyleArgs = globalConfig.prefFontStyle(userID)
                                    .split(',', QString::SkipEmptyParts)
                                    .toVector().toList();
        actionList = menuFontStyle->actions();
        for (int i=0; i<actionList.count(); i++)
        {
            if (fontStyleArgs.contains(actionList[i]->data().toString()))
                actionList[i]->setChecked(true);
            else
                actionList[i]->setChecked(false);
        }

    menuFontStyle->popup(QCursor::pos());
}

void MainWindow::on_buttonTextAlign_clicked()
{
    QList<QAction*> actionList;
    QString alignMode;

    if (!menuTextAlign)
    {
        // Create align style menu with icons
        menuTextAlign = new QMenu;

        actionList.append(menuTextAlign->addAction("Left justify"));
        actionList[0]->setCheckable(true);
        actionList[0]->setData(WICHAT_MAIN_TEXT_ALIGN_LEFT);
        actionList[0]->setIcon(QIcon(":/Icons/format-justify-left.png"));

        actionList.append(menuTextAlign->addAction("Center justify"));
        actionList[1]->setCheckable(true);
        actionList[1]->setData(WICHAT_MAIN_TEXT_ALIGN_CENTER);
        actionList[1]->setIcon(QIcon(":/Icons/format-justify-center.png"));

        actionList.append(menuTextAlign->addAction("Right justify"));
        actionList[2]->setCheckable(true);
        actionList[2]->setData(WICHAT_MAIN_TEXT_ALIGN_RIGHT);
        actionList[2]->setIcon(QIcon(":/Icons/format-justify-right.png"));

        groupTextAlign = new QActionGroup(menuTextAlign);
        for (int i=0; i<actionList.count(); i++)
            groupTextAlign->addAction(actionList[i]);

        connect(menuTextAlign,
                SIGNAL(triggered(QAction*)),
                this,
                SLOT(onTextAlignMenuClicked(QAction*)));
    }

        // Parse align style from user config
        alignMode = globalConfig.prefTextAlign(userID);
        actionList = groupTextAlign->actions();
        if (alignMode == WICHAT_MAIN_TEXT_ALIGN_CENTER)
            actionList[1]->setChecked(true);
        else if (alignMode == WICHAT_MAIN_TEXT_ALIGN_RIGHT)
            actionList[2]->setChecked(true);
        else
            actionList[0]->setChecked(true);

    menuTextAlign->popup(QCursor::pos());
}

void MainWindow::on_comboFontFamily_currentTextChanged(const QString &arg1)
{
    globalConfig.setPrefFontFamily(userID, arg1);
    applyFont();
}

void MainWindow::on_comboTextSize_currentTextChanged(const QString &arg1)
{
    globalConfig.setPrefFontSize(userID, arg1);
    applyFont();
}

void MainWindow::on_buttonSend_clicked()
{
    QString content;
    int sessionIndex = getSessionIndex(lastConversation);
    content = editorList[sessionIndex]->toPlainText();
    if (sendMessage(content, lastConversation))
        editorList[sessionIndex]->clear();
}

void MainWindow::on_buttonSendOpt_clicked()
{
    QList<QAction*> actionList;
    int keyCode;

    if (!menuSendOption)
    {
        // Create send option menu
        menuSendOption = new QMenu;
        groupSendOption = new QActionGroup(menuSendOption);

        actionList.append(menuSendOption->addAction("Send with Enter key"));
        actionList[0]->setCheckable(true);
        actionList[0]->setData(WICHAT_MAIN_EDITOR_SENDKEY_ENTER);

        actionList.append(menuSendOption->addAction("Send with Ctrl+Enter key"));
        actionList[1]->setCheckable(true);
        actionList[1]->setData(WICHAT_MAIN_EDITOR_SENDKEY_CTRLENTER);

        groupSendOption = new QActionGroup(menuSendOption);
        for (int i=0; i<actionList.count(); i++)
            groupSendOption->addAction(actionList[i]);

        connect(menuSendOption,
                SIGNAL(triggered(QAction*)),
                this,
                SLOT(onSendOptionMenuClicked(QAction*)));
    }

        // Parse send option from user config
        keyCode = globalConfig.prefSendKey(userID);
        actionList = groupSendOption->actions();
        if (keyCode == WICHAT_MAIN_EDITOR_SENDKEY_CTRLENTER)
            actionList[1]->setChecked(true);
        else
            actionList[0]->setChecked(true);

    menuSendOption->popup(QCursor::pos());
}

void MainWindow::on_tabSession_tabBarClicked(int index)
{
    Q_UNUSED(index)
    onMouseHoverEnter(tabBarSession, nullptr);
}

void MainWindow::on_tabSession_currentChanged(int index)
{
    if (index < 0)
        return;

    QString ID = ui->tabSession->widget(index)->property("ID").toString();
    loadSession(ID, false);
}

void MainWindow::on_buttonImage_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,
                         "Send image",
                         lastFilePath,
                         QString(WICHAT_MAIN_FILE_FILTER_IMAGE).append(";;")
                         .append(WICHAT_MAIN_FILE_FILTER_GIF).append(";;")
                         .append(WICHAT_MAIN_FILE_FILTER_JPG).append(";;")
                         .append(WICHAT_MAIN_FILE_FILTER_PNG),
                         &lastImageFilter);
    if (path.isEmpty())
        return;

    QString content("<file><type>i</type><name>%FILE%</name></file>");
    content.replace("%FILE%", path);
    sendMessage(content, lastConversation);
    lastFilePath = path;
}

void MainWindow::on_buttonFile_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,
                         "Send file",
                         lastFilePath,
                         QString(WICHAT_MAIN_FILE_FILTER_ALL));
    if (path.isEmpty())
        return;

    QString content("<file><type>f</type><name>%FILE%</name></file>");
    content.replace("%FILE%", path);
    sendMessage(content, lastConversation);
    lastFilePath = path;
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

void MainWindow::on_buttonEmotion_clicked()
{
    if (emoticonList->isVisible())
        emoticonList->hide();
    else
    {
        emoticonList->move(ui->frameTextCtrl->pos() +
                           ui->buttonEmotion->pos() -
                           QPoint(0, emoticonList->height()));
        emoticonList->show();
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

void MainWindow::on_comboState_currentIndexChanged(int index)
{
    bool ok;
    int state = ui->comboState->itemData(index).toInt(&ok);
    if (ok)
        changeState(Account::OnlineState(state));
}
