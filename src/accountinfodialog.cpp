#include "accountinfodialog.h"
#include "ui_accountinfodialog.h"

AccountInfoDialog::AccountInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AccountInfoDialog)
{
    ui->setupUi(this);

    remarksVisible = true;
    offlineMsgReadOnly = false;
}

AccountInfoDialog::~AccountInfoDialog()
{
    delete ui;
}

void AccountInfoDialog::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    ui->labelID->setText(ID);

    if (state == OnlineState::Offline)
        ui->labelOnlineState->setText("Not online");
    else
        ui->labelOnlineState->setText("Online");

    ui->textOfflineMsg->setText(offlineMsg);
    ui->textOfflineMsg->setTextEditable(!offlineMsgReadOnly);

    ui->textRemarks->setText(remarks);
    ui->textRemarks->setVisible(remarksVisible);
    ui->textRemarks->setTextEditable(!remarksReadOnly);

    setWindowTitle(QString("Friend Info - ").append(ID));
}

void AccountInfoDialog::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event)
    if (ui->textRemarks->text() != remarks)
    {
        remarks = ui->textRemarks->text();
        emit remarksChanged(ID, remarks);
    }
    if (ui->textOfflineMsg->text() != offlineMsg)
    {
        offlineMsg = ui->textOfflineMsg->text();
        emit offlineMsgChanged(ID, offlineMsg);
    }
}
