#include <QDesktopWidget>
#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "serverconfigdialog.h"


LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    serverConfig = nullptr;

    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->height() - height()) / 2);
}

LoginWindow::~LoginWindow()
{
    delete ui;
    if (serverConfig)
        delete serverConfig;
}

void LoginWindow::on_buttonLogin_clicked()
{

}

void LoginWindow::on_buttonSettings_clicked()
{
    if (!serverConfig)
        serverConfig = new ServerConfigDialog;
    serverConfig->exec();
}
