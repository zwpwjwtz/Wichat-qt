#include <QInputDialog>
#include <QMessageBox>

#include "groupinfodialog.h"
#include "ui_groupinfodialog.h"
#include "global_objects.h"
#include "mainwindow.h"

#define WICHAT_GROUPINFO_MEMBERLIST_FIELD_ICON 0
#define WICHAT_GROUPINFO_MEMBERLIST_FIELD_ICONPATH 1
#define WICHAT_GROUPINFO_MEMBERLIST_FIELD_ID 2

#define WICHAT_GROUPINF_DATE_FORMAT "yyyy-MM-dd"


GroupInfoDialog::GroupInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GroupInfoDialog)
{
    ui->setupUi(this);
    ui->listMember->setModel(&listMemberModel);
    listMemberModel.setColumnCount(3);

    connect(&globalAccount,
            SIGNAL(getGroupMemberListFinished(int,
                                        QList<Account::AccountListEntry>&)),
            this,
            SLOT(onGetGroupMemberListFinished(int,
                                        QList<Account::AccountListEntry>&)));
    connect(&globalAccount,
            SIGNAL(addGroupMemeberFinished(int,bool)),
            this,
            SLOT(onAddGroupMemeberFinished(int,bool)));
    connect(&globalAccount,
            SIGNAL(removeGroupMemeberFinished(int,bool)),
            this,
            SLOT(onRemoveGroupMemeberFinished(int,bool)));
    connect(&globalAccount,
            SIGNAL(setGroupNameFinished(int,bool)),
            this,
            SLOT(onSetGroupNameFinished(int,bool)));
    connect(&globalAccount,
            SIGNAL(setGroupDescriptionFinished(int,bool)),
            this,
            SLOT(onSetGroupDescriptionFinished(int,bool)));
    connect(ui->labelGroupName,
            SIGNAL(textChanged(QString)),
            this,
            SLOT(onLabelGroupNameModified(QString)));
    connect(ui->labelDescription,
            SIGNAL(textChanged(QString)),
            this,
            SLOT(onLabelDescriptionModified(QString)));

    nameReadOnly = true;
    descriptionReadOnly = true;
}

GroupInfoDialog::~GroupInfoDialog()
{
    delete ui;
}

void GroupInfoDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    ui->labelID->setText(groupID);
    setWindowTitle(QString("Group Info - %1").arg(groupID));
    ui->labelCreationTime->setText(QString("Created in %1")
                                          .arg(creationTime.toString(
                                                WICHAT_GROUPINF_DATE_FORMAT)));
    ui->labelGroupName->setText(name);
    ui->labelGroupName->setTextEditable(userRole >=
                                    Account::GroupMemberRole::Administrator);
    ui->labelDescription->setText(description);
    ui->labelDescription->setTextEditable(userRole >=
                                    Account::GroupMemberRole::Administrator);

    ui->buttonAddMember->setEnabled(userRole >=
                                    Account::GroupMemberRole::Administrator);
    ui->buttonRemoveMember->setEnabled(ui->buttonAddMember->isEnabled());

    int queryID;
    globalAccount.getGroupMemberList(groupID, queryID);
}

void GroupInfoDialog::onAddGroupMemeberFinished(int queryID, bool successful)
{
    if (!successful)
        QMessageBox::critical(this, "Failed to add member",
                              QString("Failed to add member to group %1.\n"
                                      "Please make sure that you have the "
                                      "right permission to do that.")
                                     .arg(groupID));
    else
        globalAccount.getGroupMemberList(groupID, queryID);
}

void GroupInfoDialog::onRemoveGroupMemeberFinished(int queryID, bool successful)
{
    if (!successful)
        QMessageBox::critical(this, "Failed to remove member",
                              QString("Failed to remove member from group %1.\n"
                                      "Please make sure that you have the "
                                      "right permission to do that.")
                                     .arg(groupID));
    else
        globalAccount.getGroupMemberList(groupID, queryID);
}

void GroupInfoDialog::onSetGroupNameFinished(int queryID, bool successful)
{
    Q_UNUSED(queryID)
    if (!successful)
        QMessageBox::critical(this, "Failed to set group name",
                              QString("Failed to set the name of group %1.\n"
                                      "You may not have the permission to "
                                      "modify group information").arg(groupID));
}

void GroupInfoDialog::onSetGroupDescriptionFinished(int queryID,bool successful)
{
    Q_UNUSED(queryID)
    if (!successful)
        QMessageBox::critical(this, "Failed to set group description",
                              QString("Failed to set description text for group"
                                      "%1\n. You may not have the permission "
                                      "to modify group information")
                                     .arg(groupID));
}

void GroupInfoDialog::onGetGroupMemberListFinished(int queryID,
                                    QList<Account::AccountListEntry>& members)
{
    Q_UNUSED(queryID)
    listMemberModel.removeRows(0, listMemberModel.rowCount());

    QList<QStandardItem*> row;
    QString iconPath;

    int onlineCount = 0;
    Account::AccountListEntry member;
    for (int i=0; i<members.count(); i++)
    {
        member = members[i];
        if (member.state == Account::OnlineState::Online ||
            member.state == Account::OnlineState::Busy)
            onlineCount++;
        iconPath = globalMainWindow->stateToImagePath(int(member.state));

        row.clear();
        row.append(new QStandardItem(QIcon(iconPath), member.ID));
        row.append(new QStandardItem(iconPath));
        row.append(new QStandardItem(member.ID));
        listMemberModel.appendRow(row);
    }

    ui->tabGroupInfo->setTabText(1, QString("Group members (%1/%2)")
                                        .arg(QString::number(onlineCount))
                                        .arg(QString::number(members.count())));
}

void GroupInfoDialog::onLabelGroupNameModified(QString text)
{
    int queryID;
    globalAccount.setGroupName(groupID, text, queryID);
}

void GroupInfoDialog::onLabelDescriptionModified(QString text)
{
    int queryID;
    globalAccount.setGroupDescription(groupID, text, queryID);
}

void GroupInfoDialog::on_buttonAddMember_clicked()
{
    QString ID = QInputDialog::getText(this, "Add group member",
                                       "Input the ID of the account that you "
                                       "want to add to this group:");
    if (!Account::checkID(ID))
    {
        QMessageBox::critical(this, "ID format error",
                              "The ID that you input is not correct.");
        return;
    }

    int queryID;
    globalAccount.addGroupMemeber(groupID, ID, queryID);
}

void GroupInfoDialog::on_buttonRemoveMember_clicked()
{
    QModelIndexList index = ui->listMember->selectionModel()->selectedIndexes();
    if (index.count() < 1)
        return;

    QList<QString> IDList;
    for (int i=0; i<index.count(); i++)
    {
        if (index[i].column() == WICHAT_GROUPINFO_MEMBERLIST_FIELD_ID)
            IDList.append(listMemberModel.item(index[i].row(),
                                          WICHAT_GROUPINFO_MEMBERLIST_FIELD_ID)
                                         ->text());
    }

    if (IDList.count() == 1)
    {
        if (QMessageBox::warning(this, "Remove group member",
                                 QString("Are you sure to remove %1 "
                                         "from this group?").arg(IDList[0]),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No)
                != QMessageBox::Yes)
            return;
    }
    else
    {
        if (QMessageBox::warning(this, "Remove group member",
                                 QString("Are you sure to remove %1 members"
                                         "from this group?")
                                        .arg(QString::number(IDList.count())),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No)
                != QMessageBox::Yes)
            return;
    }

    int queryID;
    for (int i=0; i<IDList.count(); i++)
        globalAccount.removeGroupMemeber(groupID, IDList[i], queryID);
}

void GroupInfoDialog::on_buttonGetMemberInfo_clicked()
{
    QModelIndexList index = ui->listMember->selectionModel()->selectedIndexes();
    if (index.count() != 3)
        return;

    QString ID = listMemberModel.item(
                            index[WICHAT_GROUPINFO_MEMBERLIST_FIELD_ID].row(),
                            WICHAT_GROUPINFO_MEMBERLIST_FIELD_ID)
                                ->text();
    globalMainWindow->showAccountInfo(ID);
}
