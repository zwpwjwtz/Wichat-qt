#ifndef GROUPLISTWIDGET_H
#define GROUPLISTWIDGET_H

#include "abstractlistwidget.h"
#include "Modules/account.h"


namespace Ui {
class GroupListWidget;
}

class QAction;
class QMenu;

class GroupListWidget : public AbstractListWidget
{
    Q_OBJECT

public:
    struct GroupInfoEntry
    {
        QString name;
    };

    explicit GroupListWidget(QWidget *parent = 0);
    ~GroupListWidget();
    void bindService(Account* service);

    bool refreshList();
    bool updateList();
    bool updateEntryInfo();
    bool search(QString text);

    bool hasMember(QString ID);
    void setSearchMode(bool isSearching);

    void joinGroup(QString ID);
    void showGroupInfo(QString groupID);
    QList<QString> getGroupIDList();
    QString getGroupName(QString ID);
    QString getGroupImagePath(QString ID);

private:
    Ui::GroupListWidget *ui;
    Account* accountService;
    QMap<QString, GroupInfoEntry> infoList;
    QMap<int, QString> queryList;
    bool serviceBinded;
    bool needRefresh;

    void doRefreshList();
    void setListEntryVisible(bool visible);
    void showGroupInfoDialog(Account::GroupInfoEntry& info);

private slots:
    void onUpdateGroupListFinished(int queryID,
                                   QList<Account::GroupListEntry>& groups);
    void onGetGroupNamesFinished(int queryID, QList<QString>& names);
    void onGetGroupInfoFinished(int queryID, Account::GroupInfoEntry& info);
    void onJoinGroupFinished(int queryID, bool successful);
    void onQuitGroupFinished(int queryID, bool successful);

    // Customized slots for UI events
    void onContextMenuClicked(QAction* action);

    // Auto-connected slots for UI events
    void on_listGroup_doubleClicked(const QModelIndex &index);
    void on_listGroup_customContextMenuRequested(const QPoint &pos);
};

#endif // GROUPLISTWIDGET_H
