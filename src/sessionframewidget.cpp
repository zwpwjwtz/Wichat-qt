#include <QDir>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMenu>
#include <QStackedLayout>
#include <QLineEdit>
#include <QScrollBar>
#include <QColorDialog>
#include <QFileDialog>
#include <QDateTime>

#include "sessionframewidget.h"
#include "ui_sessionframewidget.h"
#include "config_field.h"
#include "emoticonchooser.h"
#include "friendlistwidget.h"
#include "grouplistwidget.h"
#include "imageresource.h"
#include "sessionpresenter.h"
#include "Modules/wichatconfig.h"
#include "Modules/account.h"
#include "Modules/conversation.h"
#include "Modules/group.h"
#include "Modules/notification.h"
#include "Modules/htmlhelper.h"

#ifdef Q_OS_WIN32
#define WICHAT_SESNFRAME_RESOURCE_DIR "Resource"
#else
#define WICHAT_SESNFRAME_RESOURCE_DIR "/usr/share"
#define WICHAT_SESNFRAME_RESOURCE_DIR_2 "/app/share"
#endif

#define WICHAT_SESNFRAME_FILE_FILTER_ALL "All (*.*)(*.*)"
#define WICHAT_SESNFRAME_FILE_FILTER_IMAGE "Image file " \
                                           "(*.gif *.jpg *.png)" \
                                           "(*.gif *.jpg *.png)"
#define WICHAT_SESNFRAME_FILE_FILTER_GIF "GIF file (*.gif)(*.gif)"
#define WICHAT_SESNFRAME_FILE_FILTER_JPG "JPEG file (*.jpg *.jpeg)(*.jpg *.jpeg)"
#define WICHAT_SESNFRAME_FILE_FILTER_PNG "PNG file (*.png)(*.png)"


SessionFrameWidget::SessionFrameWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionFrameWidget)
{
    ui->setupUi(this);

    ui->textBrowser->hide();
    ui->textEdit->hide();
    ui->tabSession->removeTab(0);
    ui->frameFont->hide();
    ui->frameTextGroup->setLayout(new QStackedLayout);
    ui->frameTextGroup->layout()->setMargin(0);
    emoticonList = new EmoticonChooser(this);
    emoticonList->hide();
    buttonTabClose = new QPushButton(QIcon(":/Icons/remove.png"),
                                     "");
    buttonTabClose->setFlat(true);
    buttonTabClose->setIconSize(QSize(16, 16));
    buttonTabClose->resize(20, 20);
    sessionTabBar = ui->tabSession->findChild<QTabBar*>();

    installEventFilter(this);
    sessionTabBar->installEventFilter(this);

    config = nullptr;
    account = nullptr;
    friendChat = nullptr;
    groupChat = nullptr;
    noteList = nullptr;
    dialogColor = nullptr;
    menuFontStyle = nullptr;
    menuTextAlign = nullptr;
    menuSendOption = nullptr;
    groupTextAlign = nullptr;
    groupSendOption = nullptr;
    friendChatBinded = false;
    groupChatBinded = false;
    notificationBinded = false;
    lastHoveredTab = -1;
    conversationLoginState = 0;

    connect(buttonTabClose,
            SIGNAL(clicked(bool)),
            this,
            SLOT(onSessionTabClose(bool)));
    connect(emoticonList,
            SIGNAL(emoticonClicked(QByteArray)),
            this,
            SLOT(onEmoticonClicked(QByteArray)));
}

SessionFrameWidget::~SessionFrameWidget()
{
    delete ui;
}

void SessionFrameWidget::init()
{
    static bool hasInited = false;
    if (hasInited)
        return;
    else
        hasInited = true;

    QDir resourceDir(WICHAT_SESNFRAME_RESOURCE_DIR);
#ifdef WICHAT_SESNFRAME_RESOURCE_DIR_2
    if (!resourceDir.exists())
        resourceDir.setPath(WICHAT_SESNFRAME_RESOURCE_DIR_2);
#endif
    if (!resourceDir.exists())
        resourceDir = QCoreApplication::applicationDirPath();
    emoticonList->setResourceDir(resourceDir.absolutePath());

    userSessionList.loadFromFile(config->userDirectory(userID));
    if (userSessionList.count() < 1)
        loadSession(userID, LocalDialog);
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
        if (sessionType(sessionIDList[i]) == GroupChat)
        {
            sessionLastTime = userSessionList.getSession(sessionIDList[i])
                                             .messageList->last().time;
            if (sessionLastTime > lastGroupMsgTime)
                lastGroupMsgTime = sessionLastTime;
        }
    }

    friendChat->setPeerSession(peerSessionList);
    groupChat->setPeerSession(peerSessionList);

    applyUserSettings();
}

void SessionFrameWidget::bindConfig(WichatConfig *configService)
{
    config = configService;
    if (config == nullptr)
        configBinded = false;
    else
        configBinded = true;
}

void SessionFrameWidget::bindAccount(Account* service)
{
    account = service;
    if (account == nullptr)
        accountBinded = false;
    else
    {
        accountBinded = true;
        userID = account->ID();
    }
}

void SessionFrameWidget::bindFriendChatService(Conversation* service)
{
    if (friendChatBinded && friendChat != service)
        disconnect(friendChat);

    friendChat = service;
    if (friendChat == nullptr)
    {
        friendChatBinded = false;
        return;
    }
    else
        friendChatBinded = true;

    connect(friendChat,
            SIGNAL(queryError(int, AbstractChat::QueryError)),
            this,
            SLOT(onConversationQueryError(int, AbstractChat::QueryError)));
    connect(friendChat,
            SIGNAL(connectionBroken(QString)),
            this,
            SLOT(onConnectionBroken(QString)));
    connect(friendChat,
            SIGNAL(verifyFinished(int, AbstractChat::VerifyError)),
            this,
            SLOT(onConversationVerifyFinished(int, AbstractChat::VerifyError)));
    connect(friendChat,
            SIGNAL(resetSessionFinished(int, bool)),
            this,
            SLOT(onResetSessionFinished(int, bool)));
    connect(friendChat,
            SIGNAL(sendMessageFinished(int, bool)),
            this,
            SLOT(onSendMessageFinished(int, bool)));
    connect(friendChat,
            SIGNAL(getMessageListFinished(int,
                                    QList<AbstractChat::MessageListEntry>&)),
            this,
            SLOT(onGetMessageListFinished(int,
                                    QList<AbstractChat::MessageListEntry>&)));
    connect(friendChat,
            SIGNAL(receiveMessageFinished(int,
                                          QList<AbstractChat::MessageEntry>&)),
            this,
            SLOT(onReceiveMessageFinished(int,
                                          QList<AbstractChat::MessageEntry>&)));
}

void SessionFrameWidget::bindGroupChatService(Group *service)
{
    if (groupChatBinded && groupChat != service)
        disconnect(groupChat);

    groupChat = service;
    if (groupChat == nullptr)
    {
        groupChatBinded = false;
        return;
    }
    else
        groupChatBinded = true;

    connect(groupChat,
            SIGNAL(queryError(int, AbstractChat::QueryError)),
            this,
            SLOT(onConversationQueryError(int, AbstractChat::QueryError)));
    connect(groupChat,
            SIGNAL(verifyFinished(int, AbstractChat::VerifyError)),
            this,
            SLOT(onConversationVerifyFinished(int, AbstractChat::VerifyError)));
    connect(groupChat,
            SIGNAL(resetSessionFinished(int, bool)),
            this,
            SLOT(onResetSessionFinished(int, bool)));
    connect(groupChat,
            SIGNAL(sendMessageFinished(int, bool)),
            this,
            SLOT(onSendMessageFinished(int, bool)));
    connect(groupChat,
            SIGNAL(getMessageListFinished(int,
                                    QList<AbstractChat::MessageListEntry>&)),
            this,
            SLOT(onGetGroupMessageListFinished(int,
                                    QList<AbstractChat::MessageListEntry>&)));
    connect(groupChat,
            SIGNAL(receiveMessageFinished(int,
                                          QList<AbstractChat::MessageEntry>&)),
            this,
            SLOT(onReceiveMessageFinished(int,
                                          QList<AbstractChat::MessageEntry>&)));
}

void SessionFrameWidget::bindNotificationService(Notification *service)
{
    noteList = service;
    if (noteList == nullptr)
    {
        notificationBinded = false;
        return;
    }
    else
        notificationBinded = true;
}

void SessionFrameWidget::bindFriendList(FriendListWidget* widget)
{
    friendList = widget;
    if (friendList == nullptr)
        friendListBinded = false;
    else
        friendListBinded = true;
}

void SessionFrameWidget::bindGroupList(GroupListWidget* widget)
{
    groupList = widget;
    if (groupList == nullptr)
        groupListBinded = false;
    else
        groupListBinded = true;
}

QString SessionFrameWidget::currentSessionID()
{
    return lastConversation;
}

bool SessionFrameWidget::existSession(QString sessionID)
{
    return (getSessionTabIndex(sessionID) >= 0);
}

// Load new session, and make it visible to user
void SessionFrameWidget::loadSession(QString sessionID, bool setTabActive)
{
    init();
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
    if (notificationBinded)
    {
        bool receivedMsg = false;
        QList<Notification::Note> notes = noteList->getAll();
        for (int i=0; i<notes.count(); i++)
        {
            if (notes[i].source == sessionID &&
                (notes[i].type == Notification::GotMsg ||
                 notes[i].type == Notification::GotGroupMsg))
            {
                noteList->remove(notes[i].ID);
                receivedMsg = true;
            }
        }
        if (receivedMsg && sessionID != userID)
            receiveMessage(sessionID);
    }

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
    if (!browserList[index]->available())
        return;
    loadSessionContent(sessionID);
    ((QStackedLayout*)(ui->frameTextGroup->layout()))->setCurrentIndex(index);
    lastConversation = sessionID;
}

void SessionFrameWidget::loadSession(QString ID,
                                     SessionFrameWidget::SessionType type,
                                     bool setTabActive)
{
    loadSession(getSessionIDByID(ID, type), setTabActive);
}

void SessionFrameWidget::highlightSession(QString sessionID, bool highlight)
{
    int index = getSessionTabIndex(sessionID);
    if (index < 0)
        return;
    if (highlight)
    {
        sessionTabBar->setTabTextColor(index, QColor("#FFA07A"));
        setWindowTitle(QString("Message from %1")
                              .arg(getIDBySessionID(sessionID)));
        QApplication::alert(this);
    }
    else
    {
        sessionTabBar->setTabTextColor(index, QColor(""));
    }
}

SessionFrameWidget::SessionType
SessionFrameWidget::sessionType(QString sessionID)
{
    if (sessionID.indexOf('L') == 0)
        return LocalDialog;
    else if (sessionID.indexOf('G') == 0)
        return GroupChat;
    else
        return FriendChat;
}

QString SessionFrameWidget::getIDBySessionID(QString sessionID)
{
    if (sessionID.indexOf('L') == 0 || sessionID.indexOf('G') == 0)
        return sessionID.mid(1);
    else
        return sessionID;
}

QString SessionFrameWidget::getSessionIDByID(QString ID, SessionType type)
{
    if (type == LocalDialog)
        return ID.prepend('L');
    else if (type == GroupChat)
        return ID.prepend('G');
    else
        return ID;
}

void SessionFrameWidget::setTabText(QString sessionID, QString text)
{
    int index = getSessionTabIndex(sessionID);
    if (index >= 0)
        sessionTabBar->setTabText(index, text);
}

void SessionFrameWidget::setTabIconPath(QString sessionID, QString iconPath)
{
    int index = getSessionTabIndex(sessionID);
    if (index >= 0)
        sessionTabBar->setTabIcon(index, QIcon(iconPath));
}

int SessionFrameWidget::getSessionIndex(QString sessionID)
{
    for (int i=0; i<browserList.count(); i++)
    {
        if (browserList[i]->ID() == sessionID)
            return i;
    }
    return -1;
}

int SessionFrameWidget::getSessionTabIndex(QString sessionID)
{
    for (int i=0; i<ui->tabSession->count(); i++)
    {
        if (((SessionPresenter*)(ui->tabSession->widget(i)))
                                    ->ID() == sessionID)
            return i;
    }
    return -1;
}

void SessionFrameWidget::conversationLogin()
{
    if (!accountBinded)
        return;
    switch (conversationLoginState)
    {
        case 0: // Not logged in
            friendChat->verify(account->sessionID(),
                               account->sessionKey());
            groupChat->verify(account->sessionID(),
                              account->sessionKey());
            conversationLoginState = 1;
        case 1: // Waiting for server response
            // Do not use QEventloop here, as it might provoke deadlook
            break;
        case 2: // Logged in successfully
        default:
            break;
    }
}

void SessionFrameWidget::getMessageList()
{
    int queryID;
    conversationLogin();
    friendChat->getMessageList(queryID);
    groupChat->getMessageList(lastGroupMsgTime, queryID);
}

bool SessionFrameWidget::sendMessage(QString content, QString sessionID)
{
#ifndef QT_DEBUG
    if (account->state() == OnlineState::None ||
        account->state() == OnlineState::Offline)
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
    SessionType messageType(sessionType(sessionID));
    QString ID(getIDBySessionID(sessionID));
    QByteArray buffer(addSenderInfo(content, userID).toUtf8());
    bool successful;
    if (ID != userID)
    {
        if (messageType == FriendChat &&
            friendListBinded && !friendList->hasMember(ID))
        {
            QMessageBox::warning(this, "Unable send message",
                                 QString("You are not friend to %1.\n"
                                 "Please add %1 to your friend list before "
                                 "sending any message.")
                                 .arg(ID).arg(ID));
            return false;
        }
        else if (messageType == GroupChat &&
                 groupListBinded && !groupList->hasMember(ID))
        {
            QMessageBox::warning(this, "Unable send message",
                              QString("You are not in group %1.\n"
                              "Please join %1 before sending any message.")
                              .arg(ID).arg(ID));
            return false;
        }

        conversationLogin();
        if (messageType == GroupChat)
            successful = groupChat->sendMessage(ID, buffer, queryID);
        else
            successful = friendChat->sendMessage(ID, buffer, queryID);
        if (!successful)
        {
            QMessageBox::warning(this, "Cannot send message",
                                 "Wichat is currently unable to send message.");
            return false;
        }
        else
        {
            QueryInfo query;
            query.sessionID = sessionID;
            query.type = QueryType::SendMessage;
            query.cancellable = false;
            queryList[queryID] = query;
        }
    }

    SessionMessageList::MessageEntry sessionMessage;
    sessionMessage.time = QDateTime::currentDateTimeUtc();
    sessionMessage.type = SessionMessageList::TextMessage;
    sessionMessage.source = userID;
    sessionMessage.content = buffer;
    userSessionList.getSession(sessionID).messageList
                                         ->addMessage(sessionMessage);

    browserList[sessionIndex]->refresh();
    return true;
}

bool SessionFrameWidget::receiveMessage(QString sessionID)
{
    int queryID;
    QString ID(getIDBySessionID(sessionID));
    SessionType type(sessionType(sessionID));

    conversationLogin();
    if (type == FriendChat)
        friendChat->receiveMessage(ID, queryID);
    else if (type == GroupChat)
    {
        groupChat->receiveMessage(ID, lastGroupMsgTime, queryID);
        lastGroupMsgTime = QDateTime::currentDateTimeUtc();
    }
    else
        return false;

    QueryInfo query;
    query.sessionID = sessionID;
    query.type = QueryType::ReceiveMessage;
    query.cancellable = true;
    queryList[queryID] = query;
    return true;
}


void SessionFrameWidget::fixBrokenConnection()
{
    int queryID;
    QString sessionID = brokenConnectionList.dequeue();
    QString ID = getIDBySessionID(sessionID);

    conversationLogin();
    friendChat->fixBrokenConnection(ID, queryID);

    QueryInfo query;
    query.sessionID = sessionID;
    query.type = QueryType::FixConnection;
    query.cancellable = false;
    queryList[queryID] = query;
}

bool SessionFrameWidget::isInterruptable()
{
    bool interruptable = true;
    QMap<int, QueryInfo>::ConstIterator i;
    for (i=queryList.begin(); i!=queryList.end(); i++)
    {
        if (!(i->cancellable))
        {
            interruptable = false;
            break;
        }
    }
    return interruptable;
}

void SessionFrameWidget::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event)
    emoticonList->hide();

    syncSessionContent("");
    userSessionList.saveToFile(config->userDirectory(userID));
}

bool SessionFrameWidget::eventFilter(QObject *watched, QEvent *event)
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

void SessionFrameWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    ui->tabSession->resize(ui->tabSession->width(),
                           ui->frameTextCtrl->y() - ui->tabSession->y());
}

void SessionFrameWidget::applyFont()
{
    QFont font;
    QString temp;
    temp = config->prefFontStyle(userID);
    if (temp.contains(WICHAT_SESNFRAME_FONT_STYLE_BOLD))
        font.setBold(true);
    if (temp.contains(WICHAT_SESNFRAME_FONT_STYLE_ITALIC))
        font.setItalic(true);
    if (temp.contains(WICHAT_SESNFRAME_FONT_STYLE_UNDERLINE))
        font.setUnderline(true);
    if (temp.contains(WICHAT_SESNFRAME_FONT_STYLE_STRIKEOUT))
        font.setStrikeOut(true);
    if (temp.contains(WICHAT_SESNFRAME_FONT_STYLE_OVERLINE))
        font.setOverline(true);

    temp = config->prefFontFamily(userID);
    if (!temp.isEmpty())
        font.setFamily(config->prefFontFamily(userID));

    font.setPointSize(config->prefFontSize(userID).toInt());

    for (int i=0; i<editorList.count(); i++)
    {
        editorList[i]->setFont(font);
        editorList[i]->setTextColor(config->prefFontColor(userID));
        temp = config->prefTextAlign(userID);
        if (temp == WICHAT_SESNFRAME_TEXT_ALIGN_CENTER)
            editorList[i]->setAlignment(Qt::AlignCenter);
        else if (temp == WICHAT_SESNFRAME_TEXT_ALIGN_RIGHT)
            editorList[i]->setAlignment(Qt::AlignRight);
        else
            editorList[i]->setAlignment(Qt::AlignLeft);
    }
}

void SessionFrameWidget::applyUserSettings()
{
    friendChat->setUserDirectory(config->userDirectory(userID));
    groupChat->setUserDirectory(config->userDirectory(userID));

    int sendKey = config->prefSendKey(userID);
    if (!(sendKey == WICHAT_SESNFRAME_EDITOR_SENDKEY_ENTER ||
          sendKey == WICHAT_SESNFRAME_EDITOR_SENDKEY_CTRLENTER))
        config->setPrefSendKey(userID, WICHAT_SESNFRAME_EDITOR_SENDKEY_ENTER);
}

QString SessionFrameWidget::getStateImagePath(QString sessionID)
{
    QString image;
    if (getIDBySessionID(sessionID) == userID)
        image = ImageResource::stateToImagePath(int(account->state()),
                                                true);
    else switch (sessionType(sessionID))
    {
        case LocalDialog:
            image = ImageResource::stateToImagePath(
                                        int(OnlineState::Online));
            break;
        case GroupChat:
            image = groupList->getGroupImagePath(getIDBySessionID(sessionID));
            break;
        case FriendChat:
            image = friendList->getAccountImagePath(getIDBySessionID(sessionID));
            break;
        default:
            image = ImageResource::stateToImagePath(
                                        int(OnlineState::Offline));
    }
    return image;
}

void SessionFrameWidget::loadSessionContent(QString sessionID)
{
    int tabIndex;
    for (tabIndex=0; tabIndex<browserList.count(); tabIndex++)
    {
        if (browserList[tabIndex]->ID() == sessionID)
            break;
    }
    if (tabIndex >= browserList.count() || !browserList[tabIndex]->available())
        return;

    UserSession::SessionData& session = userSessionList.getSession(sessionID);
    session.active = true;

    // Do not load a session more than once
    if (browserList[tabIndex]->loaded())
        return;
    else
        browserList[tabIndex]->load(*(session.messageList));

    // Load text input area from session data
    editorList[tabIndex]->setHtml(QString::fromUtf8(session.input));
    applyFont();

    // Process possible events (redrawing etc.)
    QCoreApplication::processEvents();

    // Scroll both area to their propre positions
    browserList[tabIndex]->scrollToBottom();
    editorList[tabIndex]->verticalScrollBar()->setValue(
                    editorList[tabIndex]->verticalScrollBar()->minimum());
}

void SessionFrameWidget::syncSessionContent(QString sessionID, bool closeSession)
{
    int index;

    if (sessionID.isEmpty())
    {
        index = ui->tabSession->currentIndex();
        if (index < 0)
           return;
        sessionID = browserList[index]->ID();
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

void SessionFrameWidget::addTab(QString sessionID)
{
    SessionPresenter* newBrowser = new SessionPresenter;
    QTextEdit* newEditor = new QTextEdit;

    newBrowser->setID(sessionID);
    newBrowser->setUserID(userID);
    newBrowser->setAvailable(false);
    newBrowser->setGeometry(ui->textBrowser->geometry());
    newBrowser->bindEmoticonList(emoticonList);
    newEditor->setProperty("ID", sessionID);
    newEditor->setGeometry(ui->textEdit->geometry());

    browserList.push_back(newBrowser);
    editorList.push_back(newEditor);

    QString tabText;
    if (sessionType(sessionID) == GroupChat)
        tabText = groupList->getGroupName(getIDBySessionID(sessionID));
    else
        tabText = friendList->getFriendRemark(getIDBySessionID(sessionID));
    if (tabText.isEmpty())
        tabText = getIDBySessionID(sessionID);
    ui->tabSession->addTab(newBrowser,
                           QIcon(getStateImagePath(sessionID)),
                           tabText);
    ui->frameTextGroup->layout()->addWidget(newEditor);
    show();

    newBrowser->setAvailable(true);
}

// Initialize session tab with given session list
// Do not touch any session content
void SessionFrameWidget::loadTab()
{
    QList<QString> sessionIdList = userSessionList.sessionIdList();

    ui->tabSession->clear();

    for (int i=0; i<sessionIdList.count(); i++)
    {
        if (userSessionList.getSession(sessionIdList[i]).active)
            addTab(sessionIdList[i]);
    }
}

void SessionFrameWidget::removeTab(QString sessionID)
{
    syncSessionContent(sessionID, true);
    lastConversation.clear();

    // Actually close the tab
    int index = getSessionIndex(sessionID);
    browserList.removeAt(index);
    delete editorList[index];
    editorList.removeAt(index);
    ui->tabSession->removeTab(getSessionTabIndex(sessionID));
    if (editorList.count() < 1)
    {
        hide();

        // Trigger resizing manually
        QCoreApplication::processEvents();
        resizeEvent(0);
    }
}

QString SessionFrameWidget::addSenderInfo(const QString& content, QString ID)
{
    QString configString;
    QString styleStringHeader, styleStringFooter;

    // Set up <font> tag
    configString = config->prefFontFamily(ID);
    if (!configString.isEmpty())
        styleStringHeader.append("face=\"")
                        .append(configString)
                        .append("\"");
    configString = config->prefFontColor(ID);
    if (!configString.isEmpty())
        styleStringHeader.append(" color=\"")
                        .append(configString)
                        .append("\"");
    configString = config->prefFontSize(ID);
    if (!configString.isEmpty())
        styleStringHeader.append("style=\"font-size:")
                        .append(configString)
                        .append("pt\"");
    if (!styleStringHeader.isEmpty())
    {
        styleStringHeader.prepend("<font ").append('>');
        styleStringFooter = "</font>";
    }
    configString = config->prefFontStyle(ID);
    if (configString.contains(WICHAT_SESNFRAME_FONT_STYLE_BOLD))
    {
        styleStringHeader.append("<b>");
        styleStringFooter.prepend("</b>");
    }
    if (configString.contains(WICHAT_SESNFRAME_FONT_STYLE_ITALIC))
    {
        styleStringHeader.append("<i>");
        styleStringFooter.prepend("</i>");
    }
    if (configString.contains(WICHAT_SESNFRAME_FONT_STYLE_UNDERLINE))
    {
        styleStringHeader.append("<u>");
        styleStringFooter.prepend("</u>");
    }
    if (configString.contains(WICHAT_SESNFRAME_FONT_STYLE_STRIKEOUT))
    {
        styleStringHeader.append("<del>");
        styleStringFooter.prepend("</del>");
    }
    if (configString.contains(WICHAT_SESNFRAME_FONT_STYLE_OVERLINE))
    {
        styleStringHeader.append("<span style=\"text-decoration:overline\">");
        styleStringFooter.prepend("</span>");
    }

    // Set up alignment tag
    configString = config->prefTextAlign(ID);
    if (configString == WICHAT_SESNFRAME_TEXT_ALIGN_CENTER)
        styleStringHeader.prepend(" align=center>");
    else if (configString == WICHAT_SESNFRAME_TEXT_ALIGN_RIGHT)
        styleStringHeader.prepend(" align=right>");
    else
        styleStringHeader.prepend(">");

    styleStringHeader.prepend("<div class=c");
    styleStringFooter.append("</div>");

    return styleStringHeader.append(content).append(styleStringFooter);
}

void SessionFrameWidget::onConversationQueryError(int queryID,
                                          AbstractChat::QueryError errCode)
{
    Q_UNUSED(queryID)
    Q_UNUSED(errCode)
    conversationLoginState = 0;
}

void SessionFrameWidget::onConnectionBroken(QString ID)
{
    // TODO: deal with group session
    brokenConnectionList.enqueue(ID);
    emit connectionBroken(ID);
}

void SessionFrameWidget::onConversationVerifyFinished(int queryID,
                                            AbstractChat::VerifyError errorCode)
{
    Q_UNUSED(queryID)
    if (errorCode == AbstractChat::VerifyError::Ok)
    {
        conversationLoginState = 2;
        peerSessionList.loadSessionKey(config->userDirectory(userID),
                                       friendChat->keySalt());
    }
    else
        conversationLoginState = 0;
}

void SessionFrameWidget::onResetSessionFinished(int queryID, bool successful)
{
    Q_UNUSED(queryID)
    if (!successful)
        emit needLogin();
}

void SessionFrameWidget::onSendMessageFinished(int queryID, bool successful)
{
    QString destination = queryList.value(queryID).sessionID;
    queryList.remove(queryID);
    if (successful)
        return;

    if (!destination.isEmpty())
        QMessageBox::warning(this, "Failed to send message",
                             QString("Wichat failed to send message to %1.\n"
                             "Please check your network, then try again.")
                             .arg(getIDBySessionID(destination)));
}

void SessionFrameWidget::onGetMessageListFinished(int queryID,
                                QList<AbstractChat::MessageListEntry>& msgList)
{
    Q_UNUSED(queryID)
    if (!notificationBinded)
        return;

    Notification::Note newNote;
    newNote.destination = userID;
    newNote.type = Notification::GotMsg;
    newNote.time = QDateTime::currentDateTime();
    for (int i=0; i<msgList.count(); i++)
    {
        newNote.source = msgList[i].ID;
        newNote.ID = noteList->getNewID();
        noteList->append(newNote);
    }
}

void SessionFrameWidget::onGetGroupMessageListFinished(int queryID,
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
        newNote.ID = noteList->getNewID();
        noteList->append(newNote);
    }
}

void SessionFrameWidget::onReceiveMessageFinished(int queryID,
                                    QList<AbstractChat::MessageEntry>& messages)
{
    QString sourceID = queryList.value(queryID).sessionID;
    queryList.remove(queryID);
    if (sourceID.isEmpty())
        return;

    SessionMessageList::MessageEntry sessionMessage;
    SessionMessageList* messageList =
                            userSessionList.getSession(sourceID).messageList;
    for (int i=0; i<messages.count(); i++)
    {
        if (messages[i].source == userID)
        {
            messages.removeAt(i);
            i--;
            continue;
        }
        sessionMessage.source = messages[i].source;
        sessionMessage.time = messages[i].time;
        sessionMessage.type = SessionMessageList::TextMessage;
        sessionMessage.content = messages[i].content;
        messageList->addMessage(sessionMessage);
    }
    if (messages.count() > 0)
        browserList[getSessionIndex(sourceID)]->refresh();
}

void SessionFrameWidget::onMouseButtonRelease()
{
    ui->frameFont->hide();
    emoticonList->hide();
}

void SessionFrameWidget::onMouseHoverEnter(QObject* watched, QHoverEvent* event)
{
    Q_UNUSED(event)
    if (watched == sessionTabBar && lastHoveredTab < 0)
    {
        int index = sessionTabBar->tabAt(
                                sessionTabBar->mapFromGlobal(QCursor::pos()));
        if (index < 0)
            return;

        lastHoveredTab = index;
        sessionTabBar->setTabButton(index, QTabBar::RightSide, buttonTabClose);
    }
}

void SessionFrameWidget::onMouseHoverLeave(QObject* watched, QHoverEvent* event)
{
    Q_UNUSED(event)
    if (watched == sessionTabBar && lastHoveredTab >= 0)
    {
        sessionTabBar->setTabButton(lastHoveredTab,
                                               QTabBar::RightSide,
                                               nullptr);
        lastHoveredTab = -1;
    }
}

void SessionFrameWidget::onKeyRelease(QObject* watched, QKeyEvent* event)
{
    Q_UNUSED(watched)
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        if (lastConversation.isEmpty())
            return;
        if (editorList[getSessionIndex(lastConversation)]->hasFocus())
        {
            if ((config->prefSendKey(userID) ==
                                WICHAT_SESNFRAME_EDITOR_SENDKEY_ENTER &&
                 event->modifiers() == Qt::NoModifier) ||
                (config->prefSendKey(userID) ==
                                WICHAT_SESNFRAME_EDITOR_SENDKEY_CTRLENTER &&
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

void SessionFrameWidget::onSessionTabClose(bool checked)
{
    Q_UNUSED(checked)
    int index = sessionTabBar->tabAt(
                                sessionTabBar->mapFromGlobal(QCursor::pos()));
    sessionTabBar->setTabButton(index,
                                           QTabBar::RightSide,
                                           nullptr);

    SessionPresenter* browser = (SessionPresenter*)(ui->tabSession->widget(index));
    removeTab(browser->ID());
}


void SessionFrameWidget::onEmoticonClicked(const QByteArray &emoticon)
{
    QImage image(emoticonList->getImagePathFromCode(emoticon));
    QTextEdit* editor = editorList[getSessionIndex(lastConversation)];
    QTextCursor cursor = editor->textCursor();
    int lineHeight = config->prefFontSize(userID).toInt() * 2;
    if (lineHeight < 10)
        lineHeight = 10;
    cursor.insertImage(image.scaledToHeight(lineHeight),
                       emoticon.toHex().prepend("[emoji]").append("[/emoji]"));
}

void SessionFrameWidget::onFontStyleMenuClicked(QAction* action)
{
    // Store new style to user config
    QStringList fontStyleArgs(config->prefFontStyle(userID)
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
    config->setPrefFontStyle(userID, fontStyleArgs.join(","));
    applyFont();
}

void SessionFrameWidget::onTextAlignMenuClicked(QAction* action)
{
    // Store new style to user config
    QString alignMode;
    if (action->isChecked())
        alignMode = action->data().toString();
    config->setPrefTextAlign(userID, alignMode);
    applyFont();
}

void SessionFrameWidget::onSendOptionMenuClicked(QAction* action)
{
    // Store new style to user config
    int keyCode = WICHAT_SESNFRAME_EDITOR_SENDKEY_ENTER;
    if (action->isChecked())
        keyCode = action->data().toInt();
    config->setPrefSendKey(userID, keyCode);
}

void SessionFrameWidget::on_buttonFont_clicked()
{
    if (ui->frameFont->isVisible())
        ui->frameFont->hide();
    else
        ui->frameFont->show();
}

void SessionFrameWidget::on_buttonTextColor_clicked()
{
    if (!dialogColor)
        dialogColor = new QColorDialog;
    dialogColor->setCurrentColor(QColor(config->prefFontColor(userID)));
    dialogColor->exec();

    config->setPrefFontColor(userID,
                                  dialogColor->currentColor().name());
    applyFont();
}

void SessionFrameWidget::on_buttonTextStyle_clicked()
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
        actionList[0]->setData(WICHAT_SESNFRAME_FONT_STYLE_BOLD);
        fontStyle.setBold(true);
        actionList[0]->setFont(fontStyle);
        fontStyle.setBold(false);

        actionList.append(menuFontStyle->addAction("Italic"));
        actionList[1]->setCheckable(true);
        actionList[1]->setData(WICHAT_SESNFRAME_FONT_STYLE_ITALIC);
        fontStyle.setItalic(true);
        actionList[1]->setFont(fontStyle);
        fontStyle.setItalic(false);

        actionList.append(menuFontStyle->addAction("Underline"));
        actionList[2]->setCheckable(true);
        actionList[2]->setData(WICHAT_SESNFRAME_FONT_STYLE_UNDERLINE);
        fontStyle.setUnderline(true);
        actionList[2]->setFont(fontStyle);
        fontStyle.setUnderline(false);

        actionList.append(menuFontStyle->addAction("StrikeOut"));
        actionList[3]->setCheckable(true);
        actionList[3]->setData(WICHAT_SESNFRAME_FONT_STYLE_STRIKEOUT);
        fontStyle.setStrikeOut(true);
        actionList[3]->setFont(fontStyle);
        fontStyle.setStrikeOut(false);

        actionList.append(menuFontStyle->addAction("Overline"));
        actionList[4]->setCheckable(true);
        actionList[4]->setData(WICHAT_SESNFRAME_FONT_STYLE_OVERLINE);
        fontStyle.setOverline(true);
        actionList[4]->setFont(fontStyle);
        fontStyle.setOverline(false);

        connect(menuFontStyle,
                SIGNAL(triggered(QAction*)),
                this,
                SLOT(onFontStyleMenuClicked(QAction*)));
    }

        // Parse font style arguments from user config
        fontStyleArgs = config->prefFontStyle(userID)
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

void SessionFrameWidget::on_buttonTextAlign_clicked()
{
    QList<QAction*> actionList;
    QString alignMode;

    if (!menuTextAlign)
    {
        // Create align style menu with icons
        menuTextAlign = new QMenu;

        actionList.append(menuTextAlign->addAction("Left justify"));
        actionList[0]->setCheckable(true);
        actionList[0]->setData(WICHAT_SESNFRAME_TEXT_ALIGN_LEFT);
        actionList[0]->setIcon(QIcon(":/Icons/format-justify-left.png"));

        actionList.append(menuTextAlign->addAction("Center justify"));
        actionList[1]->setCheckable(true);
        actionList[1]->setData(WICHAT_SESNFRAME_TEXT_ALIGN_CENTER);
        actionList[1]->setIcon(QIcon(":/Icons/format-justify-center.png"));

        actionList.append(menuTextAlign->addAction("Right justify"));
        actionList[2]->setCheckable(true);
        actionList[2]->setData(WICHAT_SESNFRAME_TEXT_ALIGN_RIGHT);
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
        alignMode = config->prefTextAlign(userID);
        actionList = groupTextAlign->actions();
        if (alignMode == WICHAT_SESNFRAME_TEXT_ALIGN_CENTER)
            actionList[1]->setChecked(true);
        else if (alignMode == WICHAT_SESNFRAME_TEXT_ALIGN_RIGHT)
            actionList[2]->setChecked(true);
        else
            actionList[0]->setChecked(true);

    menuTextAlign->popup(QCursor::pos());
}

void SessionFrameWidget::on_comboFontFamily_currentTextChanged(const QString &arg1)
{
    config->setPrefFontFamily(userID, arg1);
    applyFont();
}

void SessionFrameWidget::on_comboTextSize_currentTextChanged(const QString &arg1)
{
    config->setPrefFontSize(userID, arg1);
    applyFont();
}

void SessionFrameWidget::on_buttonSend_clicked()
{
    QString content, newContent;
    int sessionIndex = getSessionIndex(lastConversation);

    // Deal with emoticons
    content = editorList[sessionIndex]->toHtml();
    newContent = content.replace("<img src=\"[emoji]", "&lt;emoticon&gt;emoji:")
                        .replace("[/emoji]\" />", "&lt;/emoticon&gt;");
    editorList[sessionIndex]->setHtml(newContent);

    content = editorList[sessionIndex]->toPlainText();
    if (sendMessage(content, lastConversation))
        editorList[sessionIndex]->clear();
}

void SessionFrameWidget::on_buttonSendOpt_clicked()
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
        actionList[0]->setData(WICHAT_SESNFRAME_EDITOR_SENDKEY_ENTER);

        actionList.append(menuSendOption->addAction("Send with Ctrl+Enter key"));
        actionList[1]->setCheckable(true);
        actionList[1]->setData(WICHAT_SESNFRAME_EDITOR_SENDKEY_CTRLENTER);

        groupSendOption = new QActionGroup(menuSendOption);
        for (int i=0; i<actionList.count(); i++)
            groupSendOption->addAction(actionList[i]);

        connect(menuSendOption,
                SIGNAL(triggered(QAction*)),
                this,
                SLOT(onSendOptionMenuClicked(QAction*)));
    }

        // Parse send option from user config
        keyCode = config->prefSendKey(userID);
        actionList = groupSendOption->actions();
        if (keyCode == WICHAT_SESNFRAME_EDITOR_SENDKEY_CTRLENTER)
            actionList[1]->setChecked(true);
        else
            actionList[0]->setChecked(true);

    menuSendOption->popup(QCursor::pos());
}

void SessionFrameWidget::on_tabSession_tabBarClicked(int index)
{
    Q_UNUSED(index)
    onMouseHoverEnter(sessionTabBar, nullptr);
}

void SessionFrameWidget::on_tabSession_currentChanged(int index)
{
    if (index < 0)
    {
        emit currentSessionChanged("");
        return;
    }

    SessionPresenter* browser = (SessionPresenter*)(ui->tabSession->widget(index));
    QString ID = browser->ID();
    loadSession(ID, false);
    emit currentSessionChanged(ID);
}

void SessionFrameWidget::on_buttonImage_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,
                         "Send image",
                         lastFilePath,
                         QString(WICHAT_SESNFRAME_FILE_FILTER_IMAGE).append(";;")
                         .append(WICHAT_SESNFRAME_FILE_FILTER_GIF).append(";;")
                         .append(WICHAT_SESNFRAME_FILE_FILTER_JPG).append(";;")
                         .append(WICHAT_SESNFRAME_FILE_FILTER_PNG),
                         &lastImageFilter);
    if (path.isEmpty())
        return;

    QString content("<file><type>i</type><name>%FILE%</name></file>");
    content.replace("%FILE%", path);
    sendMessage(content, lastConversation);
    lastFilePath = path;
}

void SessionFrameWidget::on_buttonFile_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,
                         "Send file",
                         lastFilePath,
                         QString(WICHAT_SESNFRAME_FILE_FILTER_ALL));
    if (path.isEmpty())
        return;

    QString content("<file><type>f</type><name>%FILE%</name></file>");
    content.replace("%FILE%", path);
    sendMessage(content, lastConversation);
    lastFilePath = path;
}


void SessionFrameWidget::on_buttonEmotion_clicked()
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
