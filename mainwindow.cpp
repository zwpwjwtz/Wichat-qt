#include <QMessageBox>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QStackedLayout>
#include <QTextBrowser>
#include <QLineEdit>
#include <QScrollBar>
#include <QColorDialog>
#include <QMenu>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global_objects.h"

#define WICHAT_MAIN_FRILIST_FIELD_ICON 0
#define WICHAT_MAIN_FRILIST_FIELD_ICONPATH 1
#define WICHAT_MAIN_FRILIST_FIELD_ID 2

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

void Wichat_addHTMLHeader(QByteArray& buffer, int docType)
{
    // docType: 0=Content Window; 1=Input Window
    buffer.append("<!DOCTYPE html PUBLIC ""-//W3C//DTD HTML 4.01 Transitional//EN"" ""http://www.w3.org/TR/html4/loose.dtd""><html xmlns=""http://www.w3.org/1999/xhtml""><head><meta http-equiv=""Content-Type"" content=""text/html; charset=utf-8"" /><title></title>");
    switch (docType)
    {
        case 0:
            buffer.append("<style>div,span,ul,li{marin:0px;padding:0px;list-style-type:none;font-size:15px}p{text-align:left}img{border:0px;max-width:100%;height:auto}.s{color:#339966}.r{color:#0066CC}.a{color:#000000}.c{margin:0px 0px 15px 5px;width:350px;word-wrap:break-word}</style></head><body style=""width:360px;overflow:hidden;"">");
            break;
        case 1:
            buffer.append("<style>div,span,ul,li{marin:0px;padding:0px;list-style-type:none}p{text-align:left;}</style></head><body style=""width:98%;margin:2px;overflow:hidden""><textarea type=text id=" WICHAT_MAIN_EDITOR_TEXTBOX_NAME " cols=38 rows=3 style=""width:100%;overflow:hidden;border:0px;"">");
            break;
        default:;
    }
}

void Wichat_addHTMLFooter(QByteArray& buffer,int docType)
{
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
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textBrowser->hide();
    ui->textEdit->hide();
    ui->tabSession->removeTab(0);
    ui->frameFont->hide();
    ui->frameTextGroup->setLayout(new QStackedLayout);
    buttonTabClose = new QPushButton(QIcon(":/Icons/remove.ico"),
                                     "");
    buttonTabClose->setGeometry(0, 0, 10, 10);
    connect(buttonTabClose,
            SIGNAL(clicked(bool)),
            this,
            SLOT(onSessionTabClose()));

    installEventFilter(this);
    ui->listFriend->installEventFilter(this);
    ui->tabSession->tabBar()->installEventFilter(this);

    manualExit = false;
    dialogColor = nullptr;
    menuFontStyle = nullptr;
    menuTextAlign = nullptr;
    menuSendOption = nullptr;
    groupTextAlign = nullptr;
    groupSendOption = nullptr;
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
        loadSession(userID);
    else
    {
        loadTab();
        loadSession(userSessionList.currentSession().ID);
    }
    applyUserSettings();

    hasInited = true;
}

void MainWindow::setID(QString ID)
{
    userID = ID;
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
            event->ignore();
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type())
    {
        case QEvent::MouseButtonRelease:
            onMouseButtonRelease();
            break;
        case QEvent::HoverEnter:
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
    TaskType task = taskList.dequeue();
    switch (task)
    {
        case taskChangeSession:
            changeSession();
            break;
        case taskUpdateState:
            updateState();
            refreshTab();
        break;
        case taskShowNotification:
            updateFriendList();
            refreshTab();
            showNotification();
            break;
        case taskUpdateFriList:
            updateFriendList();
            refreshTab();
            break;
        case taskUpdateAll:
            addTask(taskGetMsgList);
            addTask(taskUpdateFriList);
            break;
        case taskGetMsgList:
            QCoreApplication::processEvents();
            getMessageList();
            showNotification();
            break;
        case taskReloadMsg:
            loadSessionContent(userSessionList.currentSession().ID);
            break;
        case taskRebuildConnection:
            fixBrokenConnection();
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
    applyFont();
}

int MainWindow::getSessionIndex(QString ID)
{
    for (int i=0; i<browserList.count(); i++)
    {
        if (browserList[i]->property("ID").toString() == ID)
            return i;
    }
    return -1;
}

int MainWindow::getSessionTabIndex(QString ID)
{
    for (int i=0; i<ui->tabSession->count(); i++)
    {
        if (((QTextBrowser*)(ui->tabSession->widget(i)))
                                        ->property("ID").toString() == ID)
            return i;
    }
    return -1;
}

void MainWindow::loadSession(QString ID, bool setTabActive)
{
    if (ID.isEmpty())
        return;

    int index = getSessionTabIndex(ID);
    if (index == -1)
    {
        // Create new session
        if (!userSessionList.exists(ID))
            userSessionList.add(ID);
        addTab(ID);
    }

    index = getSessionTabIndex(ID);
    if (setTabActive)
        ui->tabSession->setTabEnabled(index, true);
    else
    {
        loadSessionContent(ID);
        showNotification();
        setWindowTitle(QString("Session with %1").arg(ID));
    }

    updateCaption();
}

void MainWindow::loadSessionContent(QString ID)
{
    UserSession::SessionData& session = userSessionList.currentSession();
    if (session.ID == ID)
        return;
    else
        session.active = false;

    // Clear possible notifications of in-coming messages
    session = userSessionList.getSession(ID);
    const Notification::Note* note;
    bool receivedMsg = false;
    for (int i=0; i<noteList.count(); i++)
    {
        note = &noteList.peek(i);
        if (note->source == ID && note->type == Notification::GotMsg)
        {
            noteList.remove(i);
            receivedMsg = true;
        }
    }

    if (receivedMsg && ID != userID)
    {
        // TODO: receiveMessage(ID)
    }

    // Load cached session content from session data
    int tabIndex = getSessionTabIndex(ID);
    QByteArray buffer;
    Wichat_addHTMLHeader(buffer, 0);
    buffer.append(session.cache);
    Wichat_addHTMLFooter(buffer, 0);
    browserList[tabIndex]->setHtml(renderHTML(buffer));

    // Load text input area from session data
    buffer.clear();
    //Wichat_addHTMLHeader(buffer, 1);
    buffer.append(session.input);
    //Wichat_addHTMLFooter(buffer, 1);
    editorList[tabIndex]->setHtml(renderHTML(buffer));

    // Process possible events (redrawing etc.)
    QCoreApplication::processEvents();

    // Scroll both area to the end
    browserList[tabIndex]->verticalScrollBar()->setValue(
                    browserList[tabIndex]->verticalScrollBar()->maximum());
    editorList[tabIndex]->verticalScrollBar()->setValue(
                    editorList[tabIndex]->verticalScrollBar()->maximum());

    session.active = true;
}

QByteArray MainWindow::renderHTML(const QByteArray& content)
{
    QByteArray result;
    // TODO
    return result;
}

void MainWindow::addTab(QString ID)
{
    QTextBrowser* newBrowser = new QTextBrowser;
    QTextEdit* newEditor = new QTextEdit;

    newBrowser->setProperty("ID", ID);
    newEditor->setProperty("ID", ID);
    newBrowser->resize(ui->textBrowser->size());
    newEditor->resize(ui->textEdit->size());

    browserList.push_back(newBrowser);
    editorList.push_back(newEditor);
    ui->tabSession->addTab(newBrowser,
                           QIcon(getStateImagePath(ID)),
                           ID);
    ui->frameTextGroup->layout()->addWidget(newEditor);

}

void MainWindow::loadTab()
{
    QList<QString> sessionIdList = userSessionList.sessionIdList();

    ui->tabSession->clear();

    // Add a private session window
    addTab(userID);

    // Add other session windows for each peer
    for (int i=0; i<sessionIdList.count(); i++)
    {
        if (sessionIdList[i] != userID)
            addTab(sessionIdList[i]);
    }
}

void MainWindow::refreshTab()
{
    for (int i=0; i<ui->tabSession->count(); i++)
    {
        ui->tabSession->setTabIcon(i, QIcon(getStateImagePath(
                                                ui->tabSession->tabText(i))));
    }
}

void MainWindow::removeTab(QString ID)
{
    // Store content in message area and input box to session cache
    int index = getSessionIndex(ID);
    UserSession::SessionData& session = userSessionList.getSession(ID);
    session.active = false;
    session.cache = browserList[index]->toHtml().toLatin1();
    session.input = editorList[index]->toHtml().toLatin1();

    // Actually close the tab
    index = getSessionTabIndex(ID);
    ui->tabSession->removeTab(index);
}

void MainWindow::changeSession()
{
    if (!globalAccount.resetSession())
        addTask(taskLogOut);
}

void MainWindow::updateState()
{

}

void MainWindow::updateCaption()
{
    QString title;
    if (userSessionList.currentSession().ID == userID)
        title.append('[')
             .append(Wichat_stateToString(globalAccount.state()))
             .append(']');
    if (!globalAccount.offlineMsg().isEmpty())
        title.append('-').append(globalAccount.offlineMsg());
    setWindowTitle(title);
}

void MainWindow::updateFriendList()
{

}

QString MainWindow::getStateImagePath(QString ID)
{
    QString image;
    if (ID == userID)
        image = stateToImagePath(int(globalAccount.state()), true);
    else
    {
        for (int i=0; i<listFriendModel.rowCount(); i++)
        {
            if (listFriendModel.item(i, WICHAT_MAIN_FRILIST_FIELD_ID)
                                ->data().toString() == ID)
            {
                image = listFriendModel.item(i,
                                             WICHAT_MAIN_FRILIST_FIELD_ICONPATH)
                                        ->data().toString();
                break;
            }
        }
    }
    return image;
}

void MainWindow::getMessageList()
{

}

void MainWindow::showNotification()
{

}

void MainWindow::fixBrokenConnection()
{

}

QString MainWindow::stateToImagePath(int stateNumber, bool displayHide)
{
    QString path = ":/Icons/";
    switch (Account::OnlineState(stateNumber))
    {
        case Account::OnlineState::Online:
            path.append("green");
            break;
        case Account::OnlineState::Busy:
            path.append("yellow");
            break;
        case Account::OnlineState::Hide:
            if (displayHide)
                path.append("blue");
            else
                path.append("grey");
            break;
        case Account::OnlineState::Offline:
        default:
            path.append("grey");
    }
    path.append(".ico");
    return path;
}

void MainWindow::onMouseButtonRelease()
{
    if (!ui->frameFont->hasFocus())
    ui->frameFont->hide();
}

void MainWindow::onMouseHoverEnter(QObject* watched, QHoverEvent* event)
{
    if (watched == ui->tabSession->tabBar())
    {
        int index = ui->tabSession->tabBar()->tabAt(QCursor::pos()
                                                    - pos()
                                                    - QPoint(0, 50));
        if (index < 0)
            return;

        lastHoveredTab = index;
        ui->tabSession->tabBar()->setTabButton(index,
                                               QTabBar::RightSide,
                                               buttonTabClose);
    }
}

void MainWindow::onMouseHoverLeave(QObject* watched, QHoverEvent* event)
{
    if (watched == ui->tabSession->tabBar() && lastHoveredTab >= 0)
    {
        ui->tabSession->tabBar()->setTabButton(lastHoveredTab,
                                               QTabBar::RightSide,
                                               nullptr);
        lastHoveredTab = -1;
    }
}

void MainWindow::onKeyRelease(QObject* watched, QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        if (editorList[getSessionIndex(
                    userSessionList.currentSession().ID)]->hasFocus())
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
}

void MainWindow::onSessionTabClose(bool checked)
{
    int index = ui->tabSession->tabBar()->tabAt(QCursor::pos() - pos());
    ui->tabSession->tabBar()->setTabButton(index,
                                           QTabBar::RightSide,
                                           nullptr);
    removeTab(ui->tabSession->widget(index)->property("ID").toString());
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
    QList<QString> fontStyleArgs;

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
    }
    else
    {
        // Parse font style arguments from user config
        fontStyleArgs = globalConfig.prefFontStyle(userID)
                                    .split(',').toVector().toList();
        actionList = menuFontStyle->actions();
        for (int i=0; i<actionList.count(); i++)
        {
            if (fontStyleArgs.contains(actionList[i]->data().toString()))
                actionList[0]->setChecked(true);
            else
                actionList[0]->setChecked(false);
        }
    }

    menuFontStyle->exec(QCursor::pos());

    // Store new style to user config
    fontStyleArgs.clear();
    for (int i=0; i< actionList.count(); i++)
    {
        if (actionList[i]->isChecked())
            fontStyleArgs.push_back(actionList[i]->data().toString());
    }
    globalConfig.setPrefFontFamily(userID, fontStyleArgs.join(','));
    applyFont();
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
    }
    else
    {
        // Parse align style from user config
        alignMode = globalConfig.prefTextAlign(userID);
        actionList = groupTextAlign->actions();
        if (alignMode == WICHAT_MAIN_TEXT_ALIGN_CENTER)
            actionList[1]->setChecked(true);
        else if (alignMode == WICHAT_MAIN_TEXT_ALIGN_RIGHT)
            actionList[2]->setChecked(true);
        else
            actionList[0]->setChecked(true);
    }

    menuTextAlign->exec(QCursor::pos());

    // Store new style to user config
    alignMode.clear();
    for (int i=0; i< actionList.count(); i++)
    {
        if (actionList[i]->isChecked())
        {
            alignMode = actionList[i]->data().toString();
            break;
        }
    }
    globalConfig.setPrefTextAlign(userID, alignMode);
    applyFont();
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
#ifndef QT_DEBUG
    if (globalAccount.state() == Account::OnlineState::None ||
        globalAccount.state() == Account::OnlineState::Offline ||)
    {
        QMessageBox::warning(this, "Please log in first",
                             "You are off-line now. "
                             "Please log in to send messages.");
        return;
    }
#endif

    QString sessionID = userSessionList.currentSession().ID;
    int sessionIndex = getSessionIndex(sessionID);
    if (sessionIndex < 0)
        return;

    // TODO: sendMessage(ID, content)

    browserList[sessionIndex]->append(editorList[sessionIndex]->toHtml());
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
    }
    else
    {
        // Parse send option from user config
        keyCode = globalConfig.prefSendKey(userID);
        actionList = groupSendOption->actions();
        if (keyCode == WICHAT_MAIN_EDITOR_SENDKEY_CTRLENTER)
            actionList[1]->setChecked(true);
        else
            actionList[0]->setChecked(true);
    }

    menuSendOption->exec(QCursor::pos());

    // Store new style to user config
    keyCode = WICHAT_MAIN_EDITOR_SENDKEY_ENTER;
    for (int i=0; i< actionList.count(); i++)
    {
        if (actionList[i]->isChecked())
        {
            keyCode = actionList[i]->data().toInt();
            break;
        }
    }
    globalConfig.setPrefSendKey(userID, keyCode);
}

void MainWindow::on_tabSession_tabBarClicked(int index)
{
    onMouseHoverEnter(ui->tabSession->tabBar(), nullptr);
}
