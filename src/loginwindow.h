#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include "Modules/account.h"

class ServerConfigDialog;

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    typedef Account::VerifyError VerifyError;
    explicit LoginWindow(QWidget *parent = 0);
    ~LoginWindow();

protected:
    void keyPressEvent(QKeyEvent * event);

private:
    Ui::LoginWindow *ui;
    ServerConfigDialog* serverConfig;
    bool loggingIn;
    void showLoginProgress();
    void updateRootServer();

private slots:
    void onAccountVerifyFinished(VerifyError errorCode);
    void on_buttonLogin_clicked();
    void on_buttonSettings_clicked();
    void on_labelRegister_linkActivated(const QString &link);
};

#endif // LOGINWINDOW_H
