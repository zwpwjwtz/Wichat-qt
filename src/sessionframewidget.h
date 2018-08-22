#ifndef SESSIONFRAMEWIDGET_H
#define SESSIONFRAMEWIDGET_H

#include <QWidget>
#include <QMap>
#include <QQueue>
#include "Modules/usersession.h"
#include "Modules/peersession.h"
#include "Modules/sessionmessagelist.h"
#include "Modules/abstractchat.h"


class QActionGroup;
class QColorDialog;
class QMenu;
class QPushButton;
class QTabBar;
class QTextEdit;
class WichatConfig;
class Account;
class Conversation;
class Group;
class Notification;
class EmoticonChooser;
class FriendListWidget;
class GroupListWidget;
class SessionPresenter;

namespace Ui {
class SessionFrameWidget;
}

class SessionFrameWidget : public QWidget
{
    Q_OBJECT

public:
    enum SessionType
    {
        LocalDialog = 1,
        FriendChat = 2,
        GroupChat = 3
    };

    explicit SessionFrameWidget(QWidget* parent = 0);
    ~SessionFrameWidget();
    void init();
    void bindConfig(WichatConfig* configService);
    void bindAccount(Account* service);
    void bindFriendChatService(Conversation* service);
    void bindGroupChatService(Group* service);
    void bindNotificationService(Notification* service);
    void bindFriendList(FriendListWidget* widget);
    void bindGroupList(GroupListWidget* widget);

    QString currentSessionID();
    bool existSession(QString sessionID);
    void loadSession(QString sessionID, bool setTabActive = true);
    void loadSession(QString ID, SessionType type, bool setTabActive = true);
    void highlightSession(QString sessionID, bool highlight);
    SessionType sessionType(QString sessionID);
    QString getIDBySessionID(QString sessionID);
    QString getSessionIDByID(QString ID, SessionType type);

    QString getStateImagePath(QString sessionID);

    void setTabText(QString sessionID, QString text);
    void setTabIconPath(QString sessionID, QString iconPath);
    void conversationLogin();
    void getMessageList();
    bool sendMessage(QString content, QString sessionID);
    bool receiveMessage(QString sessionID);
    void showNotification();
    void fixBrokenConnection();

protected:
    void closeEvent(QCloseEvent* event);
    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent* event);

signals:
    void needLogin();
    void connectionBroken(QString sessionID);
    void currentSessionChanged(QString sessionID);

private:
    Ui::SessionFrameWidget *ui;
    WichatConfig* config;
    Account* account;
    Conversation* friendChat;
    Group* groupChat;
    Notification* noteList;
    EmoticonChooser* emoticonList;
    FriendListWidget* friendList;
    GroupListWidget* groupList;

    QColorDialog* dialogColor;
    QMenu* menuFontStyle;
    QMenu* menuTextAlign;
    QMenu* menuSendOption;
    QPushButton* buttonTabClose;
    QActionGroup* groupTextAlign;
    QActionGroup* groupSendOption;
    QTabBar* sessionTabBar;
    QList<SessionPresenter*> browserList;
    QList<QTextEdit*> editorList;

    QString userID;
    QString lastConversation;
    QString lastFilePath;
    QString lastImageFilter;
    QDateTime lastGroupMsgTime;
    QMap<int, QString> queryList;
    QQueue<QString> brokenConnectionList;
    UserSession userSessionList;
    PeerSession peerSessionList;
    int conversationLoginState;
    int lastHoveredTab;
    bool configBinded;
    bool accountBinded;
    bool friendChatBinded;
    bool groupChatBinded;
    bool notificationBinded;
    bool friendListBinded;
    bool groupListBinded;

    void applyFont();
    void applyUserSettings();
    int getSessionIndex(QString sessionID);
    int getSessionTabIndex(QString sessionID);
    void loadSessionContent(QString sessionID);
    void syncSessionContent(QString sessionID, bool closeSession = false);
    void addTab(QString sessionID);
    void loadTab();
    void removeTab(QString sessionID);
    QString addSenderInfo(const QString& content, QString ID);

private slots:
    // Slots for asynchronous request
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
    void onMouseButtonRelease();
    void onMouseHoverEnter(QObject* watched, QHoverEvent* event);
    void onMouseHoverLeave(QObject* watched, QHoverEvent* event);
    void onKeyRelease(QObject* watched, QKeyEvent* event);
    void onSessionTabClose(bool checked);
    void onEmoticonClicked(const QByteArray& emoticon);
    void onFontStyleMenuClicked(QAction* action);
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
    void on_buttonEmotion_clicked();
};

#endif // SESSIONFRAMEWIDGET_H
