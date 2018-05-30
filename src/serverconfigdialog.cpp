#include "serverconfigdialog.h"
#include "ui_serverconfigdialog.h"

ServerConfigDialog::ServerConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerConfigDialog)
{
    ui->setupUi(this);

    setFixedSize(width(), height());
    WichatRootServer = "dns.wichat.net";
    WichatRootServerPort = 80;
}

ServerConfigDialog::~ServerConfigDialog()
{
    delete ui;
}

void ServerConfigDialog::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    if (!WichatRootServer.isEmpty())
        ui->textRootServer->setText(WichatRootServer);
    if (WichatRootServerPort != 0)
        ui->textRootPort->setText(QString::number(WichatRootServerPort));
}

void ServerConfigDialog::on_buttonBox_accepted()
{
    WichatRootServer = ui->textRootServer->text();
    WichatRootServerPort = ui->textRootPort->text().toInt();
    if (WichatRootServerPort < 0)
        WichatRootServerPort = 0;
}
