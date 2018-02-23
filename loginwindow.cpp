#include <QDesktopWidget>
#include "loginwindow.h"
#include "ui_loginwindow.h"

LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->height() - height()) / 2);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}
