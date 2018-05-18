#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QDialog>

namespace Ui {
class PreferenceDialog;
}

class PreferenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferenceDialog(QWidget *parent = 0);
    ~PreferenceDialog();

protected:
    void showEvent(QShowEvent* event);

private:
    Ui::PreferenceDialog *ui;

    void loadSettings();

private slots:
    void onSetPasswordFinished(int queryID, bool successful);

    void on_radioDefaultRecordPath_clicked();
    void on_radioCustomizedRecordPath_clicked();
    void on_buttonRecordPath_clicked();
    void on_buttonChangePassword_clicked();
};

#endif // PREFERENCEDIALOG_H
