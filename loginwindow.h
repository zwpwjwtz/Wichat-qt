#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include "account.h"

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

private slots:
    void onAccountVerifyFinished(VerifyError errorCode);
    void on_buttonLogin_clicked();
    void on_buttonSettings_clicked();

private:
    Ui::LoginWindow *ui;
    ServerConfigDialog* serverConfig;
};

#endif // LOGINWINDOW_H
