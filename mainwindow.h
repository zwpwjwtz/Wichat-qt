#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QQueue>
#include <QTimer>
#include <QSystemTrayIcon>

#include "account.h"
#include "conversation.h"
#include "usersession.h"
#include "peersession.h"
#include "notification.h"


class QPushButton;
class QColorDialog;
class QTextBrowser;
class QTextEdit;
class QActionGroup;
class AccountInfoDialog;
class SystrayNotification;

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

protected:
    void changeEvent(QEvent* event);
    void closeEvent(QCloseEvent* event);
    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);

private:
    typedef Account::OnlineState OnlineState;
    typedef Account::AccountListEntry AccountListEntry;
    typedef Account::AccountInfoEntry AccountInfoEntry;
    typedef Conversation::MessageListEntry MessageListEntry;

    enum TaskType
    {
        taskNone = 0,
        taskUpdateState = 1,
        taskChangeSession = 2,
        taskLogOut = 3,
        taskLogIn = 4,
        taskUpdateFriList = 5,
        taskShowNotification = 6,
        taskUpdateAll = 7,
        taskGetMsgList = 8,
        taskReloadMsg = 9,
        taskRebuildConnection = 10,
        taskConversationLogin = 11
    };

    Ui::MainWindow *ui;
    AccountInfoDialog* accountInfo;
    SystrayNotification* sysTrayNoteList;
    QPushButton* buttonTabClose;
    QColorDialog* dialogColor;
    QMenu* menuFontStyle;
    QMenu* menuTextAlign;
    QMenu* menuSendOption;
    QMenu* menuFriendOption;
    QMenu* menuSysTray;
    QSystemTrayIcon* sysTrayIcon;
    QActionGroup* groupTextAlign;
    QActionGroup* groupSendOption;
    QTabBar* tabBarSession;
    QList<QTextBrowser*> browserList;
    QList<QTextEdit*> editorList;

    QStandardItemModel listFriendModel;
    QTimer timer;
    QString userID;
    QString lastConversation;
    QString lastFilePath;
    QString lastImageFilter;
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
    int getSessionIndex(QString ID);
    int getSessionTabIndex(QString ID);
    void loadSession(QString ID, bool setTabActive = true);
    void loadSessionContent(QString ID);
    void syncSessionContent(QString ID, bool closeSession = false);
    QString renderHTML(const QByteArray& content);
    void addTab(QString ID);
    void loadTab();
    void refreshTab();
    void highlightSession(QString ID, bool highlight);
    void removeTab(QString ID);
    void changeSession();
    void changeState(Account::OnlineState state);
    void updateState();
    void updateCaption();
    void updateSysTrayMenu();
    void updateFriendList();
    QString getStateImagePath(QString ID);
    bool isFriend(QString ID);
    void conversationLogin();
    void getMessageList();
    bool sendMessage(QString content, QString sessionID);
    bool receiveMessage(QString sessionID);
    void showNotification();
    void fixBrokenConnection();
    static QString addSenderInfo(const QString& content, QString ID);
    static QString extractHTMLTag(const QString& rawHTML, QString tagName);
    static QString stateToImagePath(int stateNumber, bool displayHide = false);
    static QString getFileNameFromPath(QString filePath);

private slots:
    // Slots for asynchronous request
    void onChangeSessionFinished(int queryID, bool successful);
    void onChangeStateFinished(int queryID,
                               bool successful,
                               OnlineState newState);
    void onUpdateFriendListFinished(int queryID,
                                    QList<AccountListEntry> friends);
    void onAddFriendFinished(int queryID, bool successful);
    void onRemoveFriendFinished(int queryID, bool successful);
    void onGetFriendInfoFinished(int queryID,
                                 QList<AccountInfoEntry> infoList);
    void onFriendRequest(QString ID);
    void onFriendRemoved(QString ID);
    void onConnectionBroken(QString ID);
    void onConversationVerifyFinished(int queryID,
                                      Conversation::VerifyError errorCode);
    void onResetSessionFinished(int queryID, bool successful);
    void onSendMessageFinished(int queryID, bool successful);
    void onGetMessageListFinished(int queryID,
                                  QList<MessageListEntry> msgList);
    void onReceiveMessageFinished(int queryID, QByteArray& content);

    // Customized slots for UI events
    void onTimerTimeout();
    void onMouseButtonRelease();
    void onMouseHoverEnter(QObject* watched, QHoverEvent* event);
    void onMouseHoverLeave(QObject* watched, QHoverEvent* event);
    void onKeyRelease(QObject* watched, QKeyEvent* event);
    void onSessionTabClose(bool checked);
    void onSysTrayNoteClicked(const Notification::Note &note);
    void onSysTrayIconClicked(QSystemTrayIcon::ActivationReason reason);
    void onSysTrayMenuShowed();
    void onSysTrayMenuClicked(QAction* action);
    void onListFriendMenuClicked(QAction* action);
    void onFriendRemarksChanged(QString ID, QString remarks);

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
    void on_listFriend_doubleClicked(const QModelIndex &index);
    void on_listFriend_customContextMenuRequested(const QPoint &pos);
    void on_buttonImage_clicked();
    void on_buttonFile_clicked();
    void on_textFriendSearch_textChanged(const QString &arg1);
    void on_buttonFriendAdd_clicked();
};

#endif // MAINWINDOW_H
