#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QQueue>
#include "account.h"
#include "usersession.h"
#include "peersession.h"
#include "notification.h"


class QPushButton;
class QColorDialog;
class QTextBrowser;
class QTextEdit;
class QActionGroup;
class QSystemTrayIcon;

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
        taskRebuildConnection = 10
    };

    Ui::MainWindow *ui;
    QPushButton* buttonTabClose;
    QColorDialog* dialogColor;
    QMenu* menuFontStyle;
    QMenu* menuTextAlign;
    QMenu* menuSendOption;
    QMenu* menuSysTray;
    QSystemTrayIcon* sysTrayIcon;
    QActionGroup* groupTextAlign;
    QActionGroup* groupSendOption;
    QList<QTextBrowser*> browserList;
    QList<QTextEdit*> editorList;
    QStandardItemModel listFriendModel;
    UserSession userSessionList;
    PeerSession peerSessionList;
    QString userID;
    QQueue<TaskType> taskList;
    Notification noteList;
    bool manualExit;
    int lastHoveredTab;

    void addTask(TaskType task);
    void doTask();
    void applyFont();
    void applyUserSettings();
    int getSessionIndex(QString ID);
    int getSessionTabIndex(QString ID);
    void loadSession(QString ID, bool setTabActive = true);
    void loadSessionContent(QString ID);
    QByteArray renderHTML(const QByteArray& content);
    void addTab(QString ID);
    void loadTab();
    void refreshTab();
    void removeTab(QString ID);
    void changeSession();
    void changeState(Account::OnlineState state);
    void updateState();
    void updateCaption();
    void updateSysTrayMenu();
    void updateFriendList();
    QString getStateImagePath(QString ID);
    void getMessageList();
    void showNotification();
    void fixBrokenConnection();
    static QString stateToImagePath(int stateNumber, bool displayHide = false);

private slots:
    // Slots for asynchronous request
    void onChangeSessionFinished(int queryID, bool successful);
    void onChangeStateFinished(int queryID,
                               bool successful,
                               Account::OnlineState newState);

    // Customized slots for UI events
    void onMouseButtonRelease();
    void onMouseHoverEnter(QObject* watched, QHoverEvent* event);
    void onMouseHoverLeave(QObject* watched, QHoverEvent* event);
    void onKeyRelease(QObject* watched, QKeyEvent* event);
    void onSessionTabClose(bool checked);
    void onSysTrayIconClicked(int reason);
    void onSysTrayMenuClicked(QAction* action);

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
};

#endif // MAINWINDOW_H
