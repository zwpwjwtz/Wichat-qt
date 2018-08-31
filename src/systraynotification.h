#ifndef SYSTRAYNOTIFICATION_H
#define SYSTRAYNOTIFICATION_H

#include <QWidget>
#include <QStandardItemModel>
#include "Modules/notification.h"


namespace Ui {
class SystrayNotification;
}

class SystrayNotification : public QWidget
{
    Q_OBJECT

public:
    explicit SystrayNotification(QWidget *parent = 0);
    ~SystrayNotification();

    void activate();
    void addNote(const Notification::Note& note);
    void removeNote(int noteID);
    int removeNotes(Notification::NoteEvent noteType,
                    QString source = QString());
    int countNote();

signals:
    void noteClicked(const Notification::Note& note);
    void listUpdated();

protected:
    void focusOutEvent(QFocusEvent * event);
    void resizeEvent(QResizeEvent* event);

private:
    Ui::SystrayNotification *ui;
    QStandardItemModel listNoteModel;
    Notification noteList;
    static const int NoteListMaxHeight = 640;

    void adjustListSize();
    bool noteExists(int noteID);

private slots:
    void on_listNote_clicked(const QModelIndex &index);
    void on_labelShowAll_linkActivated(const QString &link);
};

#endif // SYSTRAYNOTIFICATION_H
