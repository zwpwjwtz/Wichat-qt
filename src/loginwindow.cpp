#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMessageBox>

#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "serverconfigdialog.h"
#include "global_objects.h"
#include "mainwindow.h"
#include "Modules/wichatconfig.h"
#include "Modules/serverconnection.h"
#include "Modules/account.h"


LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    ui->textID->setText(globalConfig.lastID());
    serverConfig = nullptr;
    loggingIn = false;

    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->height() - height()) / 2);

    connect(&globalAccount,
            SIGNAL(verifyFinished(VerifyError)),
            this,
            SLOT(onAccountVerifyFinished(VerifyError)));

    showLoginProgress();
}

LoginWindow::~LoginWindow()
{
    delete ui;
    if (serverConfig)
        delete serverConfig;
}

void LoginWindow::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
        ui->buttonLogin->animateClick();
}

void LoginWindow::showLoginProgress()
{
    if (loggingIn)
    {
        ui->textState->setText("Logging in...");
        ui->buttonLogin->setText("Cancel");
        ui->frameInput->hide();
        ui->frameProgress->show();
    }
    else
    {
        ui->textState->clear();
        ui->buttonLogin->setEnabled(true);
        ui->buttonLogin->setText("Login");
        ui->frameInput->show();
        ui->frameProgress->hide();
    }
}

void LoginWindow::onAccountVerifyFinished(VerifyError errorCode)
{
    if (!loggingIn)
        return;

    loggingIn = false;
    showLoginProgress();
    switch (errorCode)
    {
        case Account::VerifyError::Ok:
            this->hide();
            if (!globalMainWindow)
                globalMainWindow = new MainWindow;
            globalMainWindow->show();
            break;
        case Account::VerifyError::IDFormatError:
            QMessageBox::critical(this, "Incorrect ID format",
                                  "The account ID that you provided should have"
                                  " a length between 1 and 7, and contain only "
                                  "digits and/or letters.");
            break;
        case Account::VerifyError::PasswordFormatError:
            QMessageBox::critical(this, "Incorrect password format",
                                  "The account ID that you provided should have"
                                  " a length between 1~16.");
            break;
        case Account::VerifyError::NetworkError:
            QMessageBox::critical(this, "Network error",
                                  "Wichat cannot connect to specified servers. "
                                  "Please check your configuration, "
                                  "or try again later.");
            break;
        case Account::VerifyError::VersionNotSupported:
            QMessageBox::critical(this, "Version not supported",
                                  "This version of Wichat is no longer "
                                  "supported by the server. Please update to "
                                  "a newer version.");
            break;
        case Account::VerifyError::VerificationFailed:
            QMessageBox::critical(this, "Authentication failed",
                                  "The account ID or password that you provided"
                                  " do not match. Please try again.");
            break;
        case Account::VerifyError::UnknownError:
        default:
            QMessageBox::critical(this, "Unknown error occurred",
                                  "Wichat encountered an unknown error.\n"
                                  "You may need to check program integrity, or "
                                  "report this error to the developper.");
    }
}

void LoginWindow::on_buttonLogin_clicked()
{
    if (loggingIn)
    {
        // Cancel log in
        loggingIn = false;
        showLoginProgress();
        return;
    }

    globalConfig.setLastID(ui->textID->text());

    QList<QString> rootServer = globalConfig.rootServer().split(',')
                                                         .toVector().toList();
    if (rootServer.count() >= 2)
        globalConnection.setRootServer(rootServer[0], rootServer[1].toInt());

    loggingIn = true;
    showLoginProgress();
    if (!globalAccount.verify(ui->textID->text(), ui->textPassword->text()))
    {
        // Assuming network problem
        onAccountVerifyFinished(Account::VerifyError::NetworkError);
    }
}

void LoginWindow::on_buttonSettings_clicked()
{
    if (!serverConfig)
        serverConfig = new ServerConfigDialog;

    QStringList rootServerInfo = globalConfig.rootServer().split(',');
    if (rootServerInfo.count() > 0)
        serverConfig->WichatRootServer = rootServerInfo[0];
    else
        rootServerInfo.append("");
    if (rootServerInfo.count() > 1)
        serverConfig->WichatRootServerPort = rootServerInfo[1].toInt();
    else
        rootServerInfo.append("");

    serverConfig->exec();

    rootServerInfo[0] = serverConfig->WichatRootServer;
    rootServerInfo[1] = QString::number(serverConfig->WichatRootServerPort);
    globalConfig.setRootServer(rootServerInfo.join(","));
}
