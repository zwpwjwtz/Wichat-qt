#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

#include "preferencedialog.h"
#include "ui_preferencedialog.h"
#include "global_objects.h"
#include "Modules/account.h"
#include "Modules/wichatconfig.h"


PreferenceDialog::PreferenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog)
{
    ui->setupUi(this);
    connect(&globalAccount,
            SIGNAL(setPasswordFinished(int, bool)),
            this,
            SLOT(onSetPasswordFinished(int, bool)));
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}

void PreferenceDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    loadSettings();
}

void PreferenceDialog::loadSettings()
{
    if (globalConfig.isDefaultRecordPath())
    {
        ui->radioDefaultRecordPath->setChecked(true);
        ui->labelRecordPath->clear();
    }
    else
    {
        ui->radioCustomizedRecordPath->setChecked(true);
        ui->labelRecordPath->setText(globalConfig.recordPath());
    }

}

void PreferenceDialog::onSetPasswordFinished(int queryID, bool successful)
{
    Q_UNUSED(queryID)
    if (successful)
        QMessageBox::information(this, "Password changed",
                                 "Password is successfully change. "
                                 "Please re-login with your new password.");
    else
        QMessageBox::warning(this, "Password not changed",
                             "Cannot modify account password. "
                             "Please verify your input, then try it again.");
}

void PreferenceDialog::on_radioDefaultRecordPath_clicked()
{
    ui->buttonRecordPath->setEnabled(false);
    globalConfig.setRecordPath("");
    ui->labelRecordPath->setText(globalConfig.recordPath());
}

void PreferenceDialog::on_radioCustomizedRecordPath_clicked()
{
    ui->buttonRecordPath->setEnabled(true);
}

void PreferenceDialog::on_buttonRecordPath_clicked()
{
    QString path;
    path = QFileDialog::getExistingDirectory(this, "Set user data directory",
                                             ui->labelRecordPath->text());
    if (!path.isEmpty())
    {
        ui->labelRecordPath->setText(path);
        globalConfig.setRecordPath(path);
    }
}

void PreferenceDialog::on_buttonChangePassword_clicked()
{
    bool ok;
    QString oldPassword, newPassword;

    oldPassword = QInputDialog::getText(this, "Modify account password",
                                        "Old password:",
                                        QLineEdit::PasswordEchoOnEdit,
                                        "", &ok);
    if (!ok || oldPassword.isEmpty())
        return;
    newPassword = QInputDialog::getText(this, "Modify account password",
                                        "New password:",
                                        QLineEdit::PasswordEchoOnEdit,
                                        "", &ok);
    if (!ok || newPassword.isEmpty())
        return;

    globalAccount.setPassword(oldPassword, newPassword);
}
