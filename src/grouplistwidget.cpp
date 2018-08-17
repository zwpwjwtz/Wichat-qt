#include <QMessageBox>
#include <QMenu>

#include "grouplistwidget.h"
#include "ui_grouplistwidget.h"
#include "groupinfodialog.h"
#include "imageresource.h"

#define WICHAT_GRPLIST_LIST_FIELD_ICON 0
#define WICHAT_GRPLIST_LIST_FIELD_ICONPATH 1
#define WICHAT_GRPLIST_LIST_FIELD_ID 2
#define WICHAT_GRPLIST_MENU_GROUP_OPEN 6
#define WICHAT_GRPLIST_MENU_GROUP_QUIT 7
#define WICHAT_GRPLIST_MENU_GROUP_INFO 8


GroupListWidget::GroupListWidget(QWidget *parent) :
    AbstractListWidget(parent),
    ui(new Ui::GroupListWidget)
{
    ui->setupUi(this);
    ui->listGroup->setModel(&listModel);

    accountService = nullptr;
    contextMenu = new QMenu(ui->listGroup);
    listModel.setColumnCount(3);
    serviceBinded = false;
    needRefresh = false;
}

GroupListWidget::~GroupListWidget()
{
    delete ui;
}

void GroupListWidget::bindService(Account* service)
{
    if (service == nullptr)
        return;

    if (serviceBinded)
        disconnect(accountService);

    accountService = service;
    serviceBinded = true;

    connect(accountService,
            SIGNAL(getGroupListFinished(int,QList<Account::GroupListEntry>&)),
            this,
            SLOT(onUpdateGroupListFinished(int,QList<Account::GroupListEntry>&)));
    connect(accountService,
            SIGNAL(getGroupNameFinished(int,QList<QString>&)),
            this,
            SLOT(onGetGroupNamesFinished(int,QList<QString>&)));
    connect(accountService,
            SIGNAL(getGroupInfoFinished(int,Account::GroupInfoEntry&)),
            this,
            SLOT(onGetGroupInfoFinished(int,Account::GroupInfoEntry&)));
    connect(contextMenu,
            SIGNAL(triggered(QAction*)),
            this,
            SLOT(onContextMenuClicked(QAction*)));
}

bool GroupListWidget::refreshList()
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

bool GroupListWidget::updateList()
{
    if (!serviceBinded)
        return false;
    int queryID;
    return accountService->getGroupList(queryID);
}

bool GroupListWidget::updateEntryInfo()
{
    if (!serviceBinded)
        return false;
    int queryID;
    QList<QString> groupIDList(infoList.keys());
    return accountService->getGroupNames(groupIDList, queryID);
}

bool GroupListWidget::search(QString text)
{
    if (!searchMode)
        return false;

    for (int i=0; i<listModel.rowCount(); i++)
    {
        if (listModel.item(i, WICHAT_GRPLIST_LIST_FIELD_ID)
                     ->text().indexOf(text) >= 0)
            ui->listGroup->setRowHidden(i, false);
        else
            ui->listGroup->setRowHidden(i, true);
    }
    return true;
}

bool GroupListWidget::hasMember(QString ID)
{
    for (int i=0; i<listModel.rowCount(); i++)
    {
        if (listModel.item(i, WICHAT_GRPLIST_LIST_FIELD_ID)->text() == ID)
            return true;
    }
    return false;
}

void GroupListWidget::setSearchMode(bool isSearching)
{
    if (isSearching && !searchMode)
        setListEntryVisible(false);
    else if (searchMode)
        setListEntryVisible(true);
}

void GroupListWidget::showGroupInfo(QString groupID)
{
    if (!serviceBinded)
        return;
    int queryID;
    accountService->getGroupInfo(groupID, queryID);
}

QList<QString> GroupListWidget::getGroupIDList()
{
    return infoList.keys();
}

QString GroupListWidget::getGroupName(QString ID)
{
    return infoList.value(ID).name;
}

QString GroupListWidget::getGroupImagePath(QString ID)
{
    Q_UNUSED(ID)
    return ImageResource::getGroupImagePath();
}

void GroupListWidget::doRefreshList()
{
    listModel.removeRows(0, listModel.rowCount());

    QList<QStandardItem*> row;
    QString iconPath(":/Icons/group.png");

    GroupInfoEntry groupInfo;
    QList<QString> groupIDList = infoList.keys();
    for (int i=0; i<groupIDList.count(); i++)
    {
        groupInfo = infoList[groupIDList[i]];

        row.clear();
        if (groupInfo.name.isEmpty())
            row.append(new QStandardItem(QIcon(iconPath), groupIDList[i]));
        else
            row.append(new QStandardItem(QIcon(iconPath),
                                         QString("%1(%2)")
                                         .arg(groupInfo.name)
                                         .arg(groupIDList[i])));
        row.append(new QStandardItem(iconPath));
        row.append(new QStandardItem(groupIDList[i]));
        listModel.appendRow(row);
    }
    needRefresh = false;
}

void GroupListWidget::setListEntryVisible(bool visible)
{
    for (int i=0; i<listModel.rowCount(); i++)
        ui->listGroup->setRowHidden(i, !visible);
}

void GroupListWidget::showGroupInfoDialog(Account::GroupInfoEntry& info)
{
    GroupInfoDialog* dialog = new GroupInfoDialog((QWidget*)(parent()));
    dialog->groupID = info.ID;
    dialog->memberCount = info.memberCount;
    dialog->creationTime = info.creationTime;
    dialog->userRole = info.role;
    dialog->name = info.name;
    dialog->description = info.description;
    dialog->show();
}

void GroupListWidget::onUpdateGroupListFinished(int queryID,
                                        QList<Account::GroupListEntry>& groups)
{
    Q_UNUSED(queryID)

    int i, j;
    bool found;

    // Remove non-existent entries
    QList<QString> groupIDList = infoList.keys();
    for (i=0; i<groupIDList.count(); i++)
    {
        found = false;
        for (j=0; j<groups.count(); j++)
        {
            if (groupIDList[i] == groups[j].ID)
            {
                found = true;
                break;
            }
        }
        if (!found)
            infoList.remove(groupIDList[i]);
    }

    // Update existing entries and create new entries
    GroupInfoEntry groupInfo;
    groupIDList = infoList.keys();
    for (i=0; i<groups.count(); i++)
    {
        if (groupIDList.indexOf(groups[i].ID) < 0)
        {
            groupInfo.name.clear();
            infoList.insert(groups[i].ID, groupInfo);
        }
    }
    refreshList();
    emit listUpdated();
}

void GroupListWidget::onJoinGroupFinished(int queryID, bool successful)
{
    QString ID = queryList[queryID];
    if (successful)
        QMessageBox::information(this, "Group join request sent",
                                 QString("Your join request has been sent.\n"
                                         "Please wait respose from the"
                                         "group administrator.").arg(ID));
    else
        QMessageBox::warning(this, "Joining group failed",
                             QString("Unable to join group %1.").arg(ID));
}

void GroupListWidget::onQuitGroupFinished(int queryID, bool successful)
{
    QString ID = queryList[queryID];
    if (ID.isEmpty())
        return;
    if (!successful)
        QMessageBox::warning(this, "Quiting group failed",
                             QString("Unable to quit group %1. Please contact "
                                     "administrator for help.").arg(ID));
    else
        updateList();
}

void GroupListWidget::onGetGroupNamesFinished(int queryID,
                                              QList<QString>& names)
{
    Q_UNUSED(queryID)

    QList<QString> groupIDList = infoList.keys();
    for (int i=0; i<groupIDList.count(); i++)
    {
        if (i >= names.count())
        {
            // Exception: group info list updated after query was sent
            // Try to resend this query
            updateList();
            return;
        }
        infoList[groupIDList[i]].name = names[i];
    }
    emit listUpdated();
    refreshList();
}

void GroupListWidget::onGetGroupInfoFinished(int queryID,
                                        Account::GroupInfoEntry& info)
{
    Q_UNUSED(queryID)
    showGroupInfoDialog(info);
}

void GroupListWidget::onContextMenuClicked(QAction* action)
{
    QModelIndexList index = ui->listGroup->selectionModel()->selectedIndexes();
    if (index.count() < 1)
        return;
    QString firstID = listModel.item(index[0].row(),
                                     WICHAT_GRPLIST_LIST_FIELD_ID)
                                     ->text();

    switch(action->data().toInt())
    {
        case WICHAT_GRPLIST_MENU_GROUP_OPEN:
        {
            on_listGroup_doubleClicked(index[0]);
            break;
        }
        case   WICHAT_GRPLIST_MENU_GROUP_QUIT:
        {
            if (QMessageBox::information(this, "Quit a group",
                                         QString("Do you really want to quit "
                                                 "group %1 ?").arg(firstID),
                                         QMessageBox::Yes | QMessageBox::No)
                            != QMessageBox::Yes)
                return;
            if (serviceBinded)
            {
                int queryID;
                accountService->quitGroup(firstID, queryID);
                queryList[queryID] = firstID;
            }
            break;
        }
        case WICHAT_GRPLIST_MENU_GROUP_INFO:
        {
            if (serviceBinded)
            {
                int queryID;
                accountService->getGroupInfo(firstID, queryID);
            }
            break;
        }
        default:;
    }
}

void GroupListWidget::on_listGroup_doubleClicked(const QModelIndex& index)
{
    QString ID = listModel.item(index.row(),
                                WICHAT_GRPLIST_LIST_FIELD_ID)->text();
    emit entryClicked(ID);
}

void GroupListWidget::on_listGroup_customContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)

    static QList<QAction*> actionList;
    if (actionList.count() < 1)
    {
        // Action: WICHAT_GRPLIST_MENU_GROUP_OPEN
        QAction* action = new QAction(contextMenu);
        action->setText("Open dialog");
        action->setData(WICHAT_GRPLIST_MENU_GROUP_OPEN);
        action->setIcon(QIcon(":/Icons/conversation.png"));
        contextMenu->addAction(action);

        // Action: WICHAT_GRPLIST_MENU_GROUP_QUIT
        action = new QAction(contextMenu);
        action->setText("Quit this group");
        action->setData(  WICHAT_GRPLIST_MENU_GROUP_QUIT);
        action->setIcon(QIcon(":/Icons/exit.png"));
        contextMenu->addAction(action);

        // Action: WICHAT_GRPLIST_MENU_GROUP_INFO
        action = new QAction(contextMenu);
        action->setText("View information");
        action->setData(WICHAT_GRPLIST_MENU_GROUP_INFO);
        action->setIcon(QIcon(":/Icons/information.png"));
        contextMenu->addAction(action);

        actionList = contextMenu->actions();
    }

    contextMenu->popup(QCursor::pos());
}
