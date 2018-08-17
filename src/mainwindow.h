#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QQueue>
#include <QTimer>
#include <QSystemTrayIcon>

#include "Modules/account.h"
#include "Modules/conversation.h"
#include "Modules/group.h"
#include "Modules/usersession.h"
#include "Modules/peersession.h"
#include "Modules/notification.h"
#include "Modules/sessionmessagelist.h"


class QPushButton;
class QColorDialog;
class QTextBrowser;
class QTextEdit;
class QActionGroup;
class AboutWindow;
class AccountInfoDialog;
class GroupInfoDialog;
class SystrayNotification;
class EmoticonChooser;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();
    void setID(QString ID);
    void showAccountInfo(QString ID);
    void showGroupInfo(QString groupID);

protected:
    void changeEvent(QEvent* event);
    void closeEvent(QCloseEvent* event);
    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);

private:
    enum TaskType
    {
        taskNone = 0,
        taskUpdateState = 1,
        taskChangeSession = 2,
        taskLogOut = 3,
        taskLogIn = 4,
        taskUpdateFriendList = 5,
        taskShowNotification = 6,
        taskUpdateAll = 7,
        taskGetMsgList = 8,
        taskReloadMsg = 9,
        taskRebuildConnection = 10,
        taskConversationLogin = 11,
        taskUpdateFriendInfo = 12,
        taskRefreshFriendList = 13,
        taskUpdateGroupList = 14,
        taskRefreshGroupList = 15,
        taskUpdateGroupInfo = 16
    };
    enum SessionType
    {
        LocalDialog = 1,
        FriendChat = 2,
        GroupChat = 3
    };

    Ui::MainWindow *ui;
    AboutWindow* aboutDialog;
    SystrayNotification* sysTrayNoteList;
    EmoticonChooser* emoticonList;
    QPushButton* buttonTabClose;
    QColorDialog* dialogColor;
    QMenu* menuApp;
    QMenu* menuFontStyle;
    QMenu* menuTextAlign;
    QMenu* menuSendOption;
    QMenu* menuSysTray;
    QSystemTrayIcon* sysTrayIcon;
    QActionGroup* groupTextAlign;
    QActionGroup* groupSendOption;
    QTabBar* tabBarSession;
    QList<QTextBrowser*> browserList;
    QList<QTextEdit*> editorList;

    QTimer timer;
    QString userID;
    QString lastConversation;
    QString lastFilePath;
    QString lastImageFilter;
    QDateTime lastGroupMsgTime;
    QQueue<TaskType> taskList;
    QQueue<QString> brokenConnectionList;
    QMap<int, QString> queryList;
    UserSession userSessionList;
    PeerSession peerSessionList;
    Notification noteList;
    bool manualExit;
    int conversationLoginState;
    int lastHoveredTab;
    int notificationState;
    static const int MaxEmotion = 66;

    void addTask(TaskType task);
    void doTask();
    void applyFont();
    void applyUserSettings();
    int getSessionIndex(QString sessionID);
    int getSessionTabIndex(QString sessionID);
    SessionType getSessionType(QString sessionID);
    QString getIDBySessionID(QString sessionID);
    QString getSessionIDByID(QString ID, SessionType type);
    void loadSession(QString sessionID, bool setTabActive = true);
    void loadSessionContent(QString sessionID);
    void syncSessionContent(QString sessionID, bool closeSession = false);
    void addTab(QString sessionID);
    void loadTab();
    void highlightSession(QString sessionID, bool highlight);
    void removeTab(QString sessionID);
    void changeSession();
    void changeState(Account::OnlineState state);
    void updateState();
    void updateCaption();
    void updateSysTrayMenu();
    QString getStateImagePath(QString sessionID);
    void conversationLogin();
    void getMessageList();
    bool sendMessage(QString content, QString sessionID);
    bool receiveMessage(QString sessionID);
    void showNotification();
    void fixBrokenConnection();
    QString addSenderInfo(const QString& content, QString ID);
    QString renderMessage(const SessionMessageList::MessageEntry& message,
                          bool fullHTML = false);

public:
    static QString extractHTMLTag(const QString& rawHTML, QString tagName);
    static QString getFileNameFromPath(QString filePath);

private slots:
    // Slots for asynchronous request
    void onChangeSessionFinished(int queryID, bool successful);
    void onChangeStateFinished(int queryID,
                               bool successful,
                               Account::OnlineState newState);
    void onFriendRequest(QString ID);
    void onFriendRemoved(QString ID);

    void onConversationQueryError(int queryID,
                                  AbstractChat::QueryError errCode);
    void onConnectionBroken(QString ID);
    void onConversationVerifyFinished(int queryID,
                                      AbstractChat::VerifyError errorCode);
    void onResetSessionFinished(int queryID, bool successful);
    void onSendMessageFinished(int queryID, bool successful);
    void onGetMessageListFinished(int queryID,
                                  QList<AbstractChat::MessageListEntry>& msgList);
    void onGetGroupMessageListFinished(int queryID,
                                  QList<AbstractChat::MessageListEntry>& msgList);
    void onReceiveMessageFinished(int queryID,
                                  QList<AbstractChat::MessageEntry>& messages);

    // Customized slots for UI events
    void onTimerTimeout();
    void onMouseButtonRelease();
    void onMouseHoverEnter(QObject* watched, QHoverEvent* event);
    void onMouseHoverLeave(QObject* watched, QHoverEvent* event);
    void onKeyRelease(QObject* watched, QKeyEvent* event);
    void onSessionTabClose(bool checked);
    void onListFriendUpdated();
    void onListFriendEntryClicked(QString ID);
    void onListGroupUpdated();
    void onListGroupEntryClicked(QString ID);
    void onSysTrayNoteClicked(const Notification::Note &note);
    void onSysTrayIconClicked(QSystemTrayIcon::ActivationReason reason);
    void onSysTrayMenuShowed();
    void onSysTrayMenuClicked(QAction* action);
    void onAppMenuClicked(QAction* action);
    void onEmoticonClicked(const QByteArray& emoticon);
    void onFontStyleMenuClicked(QAction* action);
    void onBrowserLinkClicked(const QUrl& url);
    void onTextAlignMenuClicked(QAction* action);
    void onSendOptionMenuClicked(QAction* action);

    // Auto-connected slots for UI events
    void on_buttonFont_clicked();
    void on_buttonTextColor_clicked();
    void on_buttonTextStyle_clicked();
    void on_buttonTextAlign_clicked();
    void on_comboFontFamily_currentTextChanged(const QString &arg1);
    void on_comboTextSize_currentTextChanged(const QString &arg1);
    void on_buttonSend_clicked();
    void on_buttonSendOpt_clicked();
    void on_tabSession_tabBarClicked(int index);
    void on_tabSession_currentChanged(int index);
    void on_buttonImage_clicked();
    void on_buttonFile_clicked();
    void on_textFriendSearch_textChanged(const QString &arg1);
    void on_buttonFriendAdd_clicked();
    void on_buttonEmotion_clicked();
    void on_buttonWichat_clicked();
    void on_comboState_currentIndexChanged(int index);
};

#endif // MAINWINDOW_H
