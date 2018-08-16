#ifndef GROUPINFODIALOG_H
#define GROUPINFODIALOG_H

#include <QDialog>
#include <QStandardItemModel>

#include "editablelabel.h"
#include "Modules/account.h"


namespace Ui {
class GroupInfoDialog;
}

class GroupInfoDialog : public QDialog
{
    Q_OBJECT

public:
    QString groupID;
    int memberCount;
    QDateTime creationTime;
    Account::GroupMemberRole userRole;
    QString name;
    bool nameReadOnly;
    QString description;
    bool descriptionReadOnly;

    QStandardItemModel listMemberModel;

    explicit GroupInfoDialog(QWidget *parent = 0);
    ~GroupInfoDialog();

protected:
    void showEvent(QShowEvent* event);

private:
    Ui::GroupInfoDialog *ui;

private slots:
    // Slots for asynchronous request
    void onAddGroupMemeberFinished(int queryID, bool successful);
    void onRemoveGroupMemeberFinished(int queryID, bool successful);
    void onSetGroupNameFinished(int queryID, bool successful);
    void onSetGroupDescriptionFinished(int queryID, bool successful);
    void onGetGroupMemberListFinished(int queryID,
                                    QList<Account::AccountListEntry>& members);

    // Customized slots for UI events
    void onLabelGroupNameModified(QString text);
    void onLabelDescriptionModified(QString text);

    // Auto-connected slots for UI events
    void on_buttonRemoveMember_clicked();
    void on_buttonAddMember_clicked();
    void on_buttonGetMemberInfo_clicked();
};

#endif // GROUPINFODIALOG_H
