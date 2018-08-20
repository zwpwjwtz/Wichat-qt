#include <QMessageBox>
#include <QMenu>

#include "friendlistwidget.h"
#include "ui_friendlistwidget.h"
#include "accountinfodialog.h"
#include "imageresource.h"

#define WICHAT_FRILIST_LIST_FIELD_ICON 0
#define WICHAT_FRILIST_LIST_FIELD_ICONPATH 1
#define WICHAT_FRILIST_LIST_FIELD_ID 2

#define WICHAT_FRILIST_MENU_FRIEND_OPEN 0
#define WICHAT_FRILIST_MENU_FRIEND_REMOVE 1
#define WICHAT_FRILIST_MENU_FRIEND_INFO 2


FriendListWidget::FriendListWidget(QWidget *parent) :
    AbstractListWidget(parent),
    ui(new Ui::FriendListWidget)
{
    ui->setupUi(this);
    ui->listFriend->setModel(&listModel);

    accountService = nullptr;
    contextMenu = new QMenu(ui->listFriend);
    listModel.setColumnCount(3);
    serviceBinded = false;
    needRefresh = false;   
}

FriendListWidget::~FriendListWidget()
{
    delete ui;
}

void FriendListWidget::bindService(Account* service)
{
    if (service == nullptr)
        return;

    if (serviceBinded)
        disconnect(accountService);

    accountService = service;
    serviceBinded = true;

    connect(accountService,
            SIGNAL(queryFriendListFinished(int,
                                           QList<Account::AccountListEntry>)),
            this,
            SLOT(onUpdateFriendListFinished(int,
                                            QList<Account::AccountListEntry>)));
    connect(accountService,
            SIGNAL(queryFriendRemarksFinished(int, QList<QString>)),
            this,
            SLOT(onUpdateFriendRemarksFinished(int,QList<QString>)));
    connect(accountService,
            SIGNAL(addFriendFinished(int, bool)),
            this,
            SLOT(onAddFriendFinished(int, bool)));
    connect(accountService,
            SIGNAL(removeFriendFinished(int, bool)),
            this,
            SLOT(onRemoveFriendFinished(int, bool)));
    connect(accountService,
            SIGNAL(queryFriendInfoFinished(int,
                                           QList<Account::AccountInfoEntry>)),
            this,
            SLOT(onGetFriendInfoFinished(int,
                                         QList<Account::AccountInfoEntry>)));
    connect(contextMenu,
            SIGNAL(triggered(QAction*)),
            this,
            SLOT(onContextMenuClicked(QAction*)));
}

bool FriendListWidget::refreshList()
{
    if (searchMode)
    {
        needRefresh = true;
        return false;
    }
    else
    {
        doRefreshList();
        return true;
    }
}

bool FriendListWidget::updateList()
{
    if (!serviceBinded)
        return false;
    int queryID;
    return accountService->queryFriendList(queryID);
}

bool FriendListWidget::updateEntryInfo()
{
    if (!serviceBinded)
        return false;
    int queryID;
    return accountService->queryFriendRemarks(infoList.keys(), queryID);
}

bool FriendListWidget::search(QString text)
{
    if (!searchMode)
        return false;

    for (int i=0; i<listModel.rowCount(); i++)
    {
        if (listModel.item(i, WICHAT_FRILIST_LIST_FIELD_ID)
                     ->text().indexOf(text) >= 0)
            ui->listFriend->setRowHidden(i, false);
        else
            ui->listFriend->setRowHidden(i, true);
    }
    return true;
}

bool FriendListWidget::hasMember(QString ID)
{
    for (int i=0; i<listModel.rowCount(); i++)
    {
        if (listModel.item(i, WICHAT_FRILIST_LIST_FIELD_ID)->text() == ID)
            return true;
    }
    return false;
}

void FriendListWidget::setSearchMode(bool isSearching)
{
    if (isSearching && !searchMode)
        setListEntryVisible(false);
    else if (searchMode)
        setListEntryVisible(true);
}

void FriendListWidget::showAccountInfo(QString ID)
{
    if (!serviceBinded)
        return;
    if (ID == accountService->ID())
    {
        // An info entry for own account
        Account::AccountInfoEntry info;
        info.ID = accountService->ID();
        info.offlineMsg = accountService->offlineMsg();
        showAccountInfoDialog(info);
    }
    else
    {
        int queryID;
        accountService->queryFriendInfo(ID, queryID);
    }
}

void FriendListWidget::setAccountIconPath(QString ID, QString iconPath)
{
    QString tempID;
    for (int i=0; i<listModel.rowCount(); i++)
    {
        tempID = listModel.item(i, WICHAT_FRILIST_LIST_FIELD_ID)->text();
        if (tempID == ID)
        {
            listModel.item(i, WICHAT_FRILIST_LIST_FIELD_ICON)
                     ->setIcon(QIcon(iconPath));
            listModel.item(i, WICHAT_FRILIST_LIST_FIELD_ICONPATH)
                     ->setText(iconPath);
        }
    }
}

QList<QString> FriendListWidget::getAccountIDList()
{
    return infoList.keys();
}

QString FriendListWidget::getFriendRemark(QString ID)
{
    return infoList.value(ID).remarks;
}

QString FriendListWidget::getAccountImagePath(QString ID)
{
    OnlineState state = infoList.value(ID).state;
    if (accountService && accountService->ID() == ID)
        return ImageResource::stateToImagePath(int(state), true);
    else
        return ImageResource::stateToImagePath(int(state));
}

void FriendListWidget::doRefreshList()
{
    listModel.removeRows(0, listModel.rowCount());

    QList<QStandardItem*> row;
    QString iconPath;

    // Add an entry for current user
    if (serviceBinded)
    {
        iconPath = ImageResource::stateToImagePath(int(accountService->state()),
                                                   true);
        row.append(new QStandardItem(QIcon(iconPath), accountService->ID()));
        row.append(new QStandardItem(iconPath));
        row.append(new QStandardItem(accountService->ID()));
        listModel.appendRow(row);
    }

    FriendInfoEntry friendInfo;
    QList<QString> friendIDList = infoList.keys();
    for (int i=0; i<friendIDList.count(); i++)
    {
        friendInfo = infoList[friendIDList[i]];
        iconPath = ImageResource::stateToImagePath(int(friendInfo.state));

        row.clear();
        if (friendInfo.remarks.isEmpty())
            row.append(new QStandardItem(QIcon(iconPath), friendIDList[i]));
        else
            row.append(new QStandardItem(QIcon(iconPath),
                                         QString("%1(%2)")
                                         .arg(friendInfo.remarks)
                                         .arg(friendIDList[i])));
        row.append(new QStandardItem(iconPath));
        row.append(new QStandardItem(friendIDList[i]));
        listModel.appendRow(row);
    }
    needRefresh = false;
}

void FriendListWidget::setListEntryVisible(bool visible)
{
    for (int i=0; i<listModel.rowCount(); i++)
        ui->listFriend->setRowHidden(i, !visible);
}

void FriendListWidget::showAccountInfoDialog(Account::AccountInfoEntry &info)
{
    AccountInfoDialog* dialog = new AccountInfoDialog((QWidget*)(parent()));
    connect(dialog,
            SIGNAL(remarksChanged(QString,QString)),
            this,
            SLOT(onFriendRemarksChanged(QString, QString)));
    connect(dialog,
            SIGNAL(offlineMsgChanged(QString, QString)),
            this,
            SLOT(onOfflineMsgChanged(QString, QString)));
    dialog->ID = info.ID;
    dialog->remarks = infoList.value(info.ID).remarks;
    if (serviceBinded)
    {
        dialog->remarksVisible = (info.ID != accountService->ID());
        dialog->remarksReadOnly = !(dialog->remarksVisible);
        dialog->offlineMsgReadOnly = dialog->remarksVisible;
    }
    else
    {
        dialog->remarksVisible = false;
        dialog->remarksReadOnly = true;
        dialog->offlineMsgReadOnly = false;
    }
    dialog->offlineMsg = info.offlineMsg;
    dialog->show();
}

void FriendListWidget::onUpdateFriendListFinished(int queryID,
                                    QList<Account::AccountListEntry> friends)
{
    Q_UNUSED(queryID)

    int i, j;
    bool found;

    // Remove non-existent entries
    QList<QString> friendIDList = infoList.keys();
    for (i=0; i<friendIDList.count(); i++)
    {
        found = false;
        for (j=0; j<friends.count(); j++)
        {
            if (friendIDList[i] == friends[j].ID)
            {
                found = true;
                break;
            }
        }
        if (!found)
            infoList.remove(friendIDList[i]);
    }

    // Update existing entries and create new entries
    FriendInfoEntry friendInfo;
    friendIDList = infoList.keys();
    for (i=0; i<friends.count(); i++)
    {
        if (friendIDList.indexOf(friends[i].ID) >= 0)
        {
            infoList[friends[i].ID].state = friends[i].state;
        }
        else
        {
            friendInfo.state = friends[i].state;
            friendInfo.remarks.clear();
            infoList.insert(friends[i].ID, friendInfo);
        }
    }
    emit listUpdated();
    refreshList();
}

void FriendListWidget::onUpdateFriendRemarksFinished(int queryID,
                                              QList<QString> remarks)
{
    Q_UNUSED(queryID)

    QList<QString> friendIDList = infoList.keys();
    for (int i=0; i<friendIDList.count(); i++)
    {
        if (i >= remarks.count())
        {
            // Exception: friend info list updated after query was sent
            // Try to resend this query
            updateList();
            return;
        }
        infoList[friendIDList[i]].remarks = remarks[i];
    }
    emit listUpdated();
    refreshList();
}

void FriendListWidget::onAddFriendFinished(int queryID, bool successful)
{
    QString ID = queryList[queryID];
    if (ID.isEmpty())
        return;
    if (successful)
        QMessageBox::information(this, "Friend request sent",
                                 QString("Your friend request has been sent.\n"
                                         "Please wait respose from %1.")
                                 .arg(ID));
    else
        QMessageBox::warning(this, "Adding friend failed",
                             QString("Cannot add %1 to your friend list.")
                             .arg(ID));
}

void FriendListWidget::onRemoveFriendFinished(int queryID, bool successful)
{
    QString ID = queryList[queryID];
    if (ID.isEmpty())
        return;
    if (!successful)
        QMessageBox::warning(this, "Removing friend failed",
                             QString("Cannot remove %1 from your friend list.")
                             .arg(ID));
    else
    {
        emit listUpdated();
        refreshList();
    }
}

void FriendListWidget::onGetFriendInfoFinished(int queryID,
                                     QList<Account::AccountInfoEntry> infoList)
{
    Q_UNUSED(queryID)
    if (infoList.count() < 1)
        return;

    showAccountInfoDialog(infoList[0]);
}

void FriendListWidget::onFriendRemarksChanged(QString ID, QString remarks)
{
    if (!serviceBinded)
        return;
    int queryID;
    accountService->setFriendRemarks(ID, remarks, queryID);
}

void FriendListWidget::onOfflineMsgChanged(QString ID, QString offlineMsg)
{
    if (!serviceBinded)
        return;
    if (ID == accountService->ID())
    {
        int queryID;
        accountService->setOfflineMsg(offlineMsg, queryID);
    }
    emit offlineMessageChanged(offlineMsg);
}

void FriendListWidget::onContextMenuClicked(QAction* action)
{
    QModelIndexList index = ui->listFriend->selectionModel()->selectedIndexes();
    if (index.count() < 1)
        return;
    QString firstID = listModel.item(index[0].row(),
                                     WICHAT_FRILIST_LIST_FIELD_ID)
                                     ->text();

    switch(action->data().toInt())
    {
        case WICHAT_FRILIST_MENU_FRIEND_OPEN:
        {
            on_listFriend_doubleClicked(index[0]);
            break;
        }
        case WICHAT_FRILIST_MENU_FRIEND_REMOVE:
        {
            if (QMessageBox::information(this, "Remove a friend",
                                         QString("Do you really want to remove %1 "
                                                 "from your friend list?")
                                                .arg(firstID),
                                         QMessageBox::Yes | QMessageBox::No)
                            != QMessageBox::Yes)
                return;
            if (serviceBinded)
            {
                int queryID;
                accountService->removeFriend(firstID, queryID);
                queryList[queryID] = firstID;
            }
            break;
        }
        case WICHAT_FRILIST_MENU_FRIEND_INFO:
        {
            showAccountInfo(firstID);
            break;
        }
        default:;
    }
}

void FriendListWidget::on_listFriend_doubleClicked(const QModelIndex &index)
{
    QString ID = listModel.item(index.row(),
                                WICHAT_FRILIST_LIST_FIELD_ID)->text();
    emit entryClicked(ID);
}

void FriendListWidget::on_listFriend_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos)

    static QList<QAction*> actionList;
    if (actionList.count() < 1)
    {
        // Action: WICHAT_FRILIST_MENU_FRIEND_OPEN
        QAction* action = new QAction(contextMenu);
        action->setText("Open dialog");
        action->setData(WICHAT_FRILIST_MENU_FRIEND_OPEN);
        action->setIcon(QIcon(":/Icons/conversation.png"));
        contextMenu->addAction(action);

        // Action: WICHAT_FRILIST_MENU_FRIEND_REMOVE
        action = new QAction(contextMenu);
        action->setText("Remove this friend");
        action->setData(WICHAT_FRILIST_MENU_FRIEND_REMOVE);
        action->setIcon(QIcon(":/Icons/remove.png"));
        contextMenu->addAction(action);

        // Action: WICHAT_FRILIST_MENU_FRIEND_INFO
        action = new QAction(contextMenu);
        action->setText("View information");
        action->setData(WICHAT_FRILIST_MENU_FRIEND_INFO);
        action->setIcon(QIcon(":/Icons/information.png"));
        contextMenu->addAction(action);

        actionList = contextMenu->actions();
    }

    QModelIndexList index = ui->listFriend->selectionModel()->selectedIndexes();
    if (index.count() < 1)
        return;
    QString firstID = listModel.item(index[0].row(),
                                     WICHAT_FRILIST_LIST_FIELD_ID)
                                     ->text();
    if (accountService && firstID == accountService->ID())
        actionList[WICHAT_FRILIST_MENU_FRIEND_REMOVE]->setVisible(false);
    else
        actionList[WICHAT_FRILIST_MENU_FRIEND_REMOVE]->setVisible(true);

    contextMenu->popup(QCursor::pos());
}
