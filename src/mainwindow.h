#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QQueue>
#include <QTimer>
#include <QSystemTrayIcon>

#include "Modules/account.h"
#include "Modules/notification.h"

class AboutWindow;
class AccountInfoDialog;
class GroupInfoDialog;
class SystrayNotification;
class SessionFrameWidget;

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
    void showAccountInfo(QString ID);
    void showGroupInfo(QString groupID);

protected:
    void changeEvent(QEvent* event);
    void closeEvent(QCloseEvent* event);
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

    Ui::MainWindow *ui;
    AboutWindow* aboutDialog;
    SystrayNotification* sysTrayNoteList;
    QSystemTrayIcon* sysTrayIcon;
    QMenu* menuApp;
    QMenu* menuSysTray;

    QTimer timer;
    QTimer iconFlashTimer;
    QIcon blankIcon;
    QIcon currentSysTrayIcon;
    QString userID;
    QQueue<TaskType> taskList;
    QMap<int, QString> queryList;
    Notification noteList;
    bool manualExit;
    bool sysTrayIconVisible;
    int notificationState;

    void addTask(TaskType task);
    void doTask();
    void changeSession();
    void changeState(OnlineState state);
    void updateState();
    void updateCaption();
    void updateSysTrayMenu();
    void showNotification();
    void setFlashIcon(bool flashing);

public:
    static QString getFileNameFromPath(QString filePath);

private slots:
    // Slots for asynchronous request
    void onChangeSessionFinished(int queryID, bool successful);
    void onChangeStateFinished(int queryID,
                               bool successful,
                               OnlineState newState);
    void onFriendRequest(QString ID);
    void onFriendRemoved(QString ID);    

    // Customized slots for UI events
    void onTimerTimeout();
    void onIconFlashTimerTimeout();
    void onListFriendUpdated();
    void onListFriendEntryClicked(QString ID);
    void onListGroupUpdated();
    void onListGroupEntryClicked(QString ID);
    void onFrameSessionIDChanged(QString ID);
    void onSysTrayNoteClicked(const Notification::Note &note);
    void onSysTrayIconClicked(QSystemTrayIcon::ActivationReason reason);
    void onSysTrayMenuShowed();
    void onSysTrayMenuClicked(QAction* action);
    void onAppMenuClicked(QAction* action);

    // Auto-connected slots for UI events
    void on_textFriendSearch_textChanged(const QString &arg1);
    void on_buttonFriendAdd_clicked();
    void on_buttonWichat_clicked();
    void on_comboState_activated(int index);
};

#endif // MAINWINDOW_H
