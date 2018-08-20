#ifndef ACCOUNTINFODIALOG_H
#define ACCOUNTINFODIALOG_H

#include <QDialog>
#include "Modules/onlinestate.h"
#include "editablelabel.h"


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
    OnlineState state;
    QString remarks;
    bool remarksVisible;
    bool remarksReadOnly;
    QString offlineMsg;
    bool offlineMsgReadOnly;

signals:
    void remarksChanged(QString ID, QString remarks);
    void offlineMsgChanged(QString ID, QString remarks);

protected:
    void showEvent(QShowEvent* event);
    void closeEvent(QCloseEvent* event);

private:
    Ui::AccountInfoDialog *ui;
};

#endif // ACCOUNTINFODIALOG_H
