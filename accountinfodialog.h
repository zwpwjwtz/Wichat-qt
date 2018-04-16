#ifndef ACCOUNTINFODIALOG_H
#define ACCOUNTINFODIALOG_H

#include <QDialog>
#include "account.h"

namespace Ui {
class AccountInfoDialog;
}

class AccountInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AccountInfoDialog(QWidget *parent = 0);
    ~AccountInfoDialog();

    QString ID;
    Account::OnlineState state;
    QString remarks;
    QString offlineMsg;

signals:
    void remarksChanged(QString ID, QString remarks);

protected:
    void showEvent(QShowEvent* event);
    void closeEvent(QCloseEvent* event);

private:
    Ui::AccountInfoDialog *ui;
};

#endif // ACCOUNTINFODIALOG_H
