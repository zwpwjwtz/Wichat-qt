#ifndef FRIENDLISTWIDGET_H
#define FRIENDLISTWIDGET_H

#include "abstractlistwidget.h"
#include "Modules/account.h"


namespace Ui {
class FriendListWidget;
}

class QAction;
class QMenu;

class FriendListWidget : public AbstractListWidget
{
    Q_OBJECT

public:
    struct FriendInfoEntry
    {
        QString remarks;
        OnlineState state;
    };

    explicit FriendListWidget(QWidget* parent = 0);
    ~FriendListWidget();
    void bindService(Account* service);

    bool refreshList();
    bool updateList();
    bool updateEntryInfo();
    bool search(QString text);

    bool hasMember(QString ID);
    void setSearchMode(bool isSearching);

    void showAccountInfo(QString ID);
    void setAccountIconPath(QString ID, QString iconPath);
    QList<QString> getAccountIDList();
    QString getFriendRemark(QString ID);
    QString getAccountImagePath(QString ID);

signals:
    void offlineMessageChanged(QString text);

private:
    Ui::FriendListWidget *ui;
    Account* accountService;
    QMap<QString, FriendInfoEntry> infoList;
    QMap<int, QString> queryList;
    bool serviceBinded;
    bool needRefresh;

    void doRefreshList();
    void setListEntryVisible(bool visible);
    void showAccountInfoDialog(Account::AccountInfoEntry &info);

private slots:
    void onUpdateFriendListFinished(int queryID,
                                    QList<Account::AccountListEntry> friends);
    void onUpdateFriendRemarksFinished(int queryID, QList<QString> remarks);
    void onGetFriendInfoFinished(int queryID,
                                 QList<Account::AccountInfoEntry> infoList);
    void onAddFriendFinished(int queryID, bool successful);
    void onRemoveFriendFinished(int queryID, bool successful);
    void onFriendRemarksChanged(QString ID, QString remarks);
    void onOfflineMsgChanged(QString ID, QString offlineMsg);

    // Customized slots for UI events
    void onContextMenuClicked(QAction* action);

    // Auto-connected slots for UI events
    void on_listFriend_doubleClicked(const QModelIndex &index);
    void on_listFriend_customContextMenuRequested(const QPoint &pos);
};

#endif // FRIENDLISTWIDGET_H
