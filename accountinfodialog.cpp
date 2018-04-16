#include "accountinfodialog.h"
#include "ui_accountinfodialog.h"

AccountInfoDialog::AccountInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AccountInfoDialog)
{
    ui->setupUi(this);
}

AccountInfoDialog::~AccountInfoDialog()
{
    delete ui;
}

void AccountInfoDialog::showEvent(QShowEvent* event)
{
    ui->labelID->setText(ID);
    if (state == Account::OnlineState::Offline)
        ui->labelOnlineState->setText("Not online");
    else
        ui->labelOnlineState->setText("Online");
    ui->labelOfflineMsg->setText(offlineMsg);
    ui->textRemarks->setText(remarks);
    setWindowTitle(QString("Friend Info - ").append(ID));
}

void AccountInfoDialog::closeEvent(QCloseEvent* event)
{
    if (ui->textRemarks->text() != remarks)
    {
        remarks = ui->textRemarks->text();
        emit remarksChanged(ID, remarks);
    }
}
