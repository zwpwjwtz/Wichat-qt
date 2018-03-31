#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

class ServerConfigDialog;

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = 0);
    ~LoginWindow();

private slots:
    void on_buttonLogin_clicked();

    void on_buttonSettings_clicked();

private:
    Ui::LoginWindow *ui;
    ServerConfigDialog* serverConfig;
};

#endif // LOGINWINDOW_H
