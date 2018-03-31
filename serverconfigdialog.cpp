#include "serverconfigdialog.h"
#include "ui_serverconfigdialog.h"

ServerConfigDialog::ServerConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerConfigDialog)
{
    ui->setupUi(this);

    if (!WichatRootServer.isEmpty())
        ui->textRootServer->setText(WichatRootServer);
    if (WichatRootServerPort != 0)
        ui->textRootPort->setText(QString::number(WichatRootServerPort));
}

ServerConfigDialog::~ServerConfigDialog()
{
    delete ui;
}

void ServerConfigDialog::on_buttonBox_accepted()
{
    WichatRootServer = ui->textRootPort->text();
    WichatRootServerPort = ui->textRootPort->text().toInt();
    if (WichatRootServerPort <= 0)
        WichatRootServerPort = 80;
}
