#include "systraynotification.h"
#include "ui_systraynotification.h"

#define WICHAT_TRAYNOTE_NOTELIST_FIELD_ID 0
#define WICHAT_TRAYNOTE_NOTELIST_FIELD_SOURCE 1
#define WICHAT_TRAYNOTE_NOTELIST_FIELD_TYPE 2
#define WICHAT_TRAYNOTE_NOTELIST_FIELD_TEXT 3
#define WICHAT_TRAYNOTE_NOTELIST_FIELD_COUNT 4


SystrayNotification::SystrayNotification(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystrayNotification)
{
    ui->setupUi(this);
    listNoteModel.setColumnCount(5);
    ui->listNote->setModel(&listNoteModel);
    ui->listNote->setColumnHidden(WICHAT_TRAYNOTE_NOTELIST_FIELD_ID, true);
    ui->listNote->setColumnHidden(WICHAT_TRAYNOTE_NOTELIST_FIELD_SOURCE, true);
    ui->listNote->setColumnHidden(WICHAT_TRAYNOTE_NOTELIST_FIELD_TYPE, true);
    ui->listNote->setColumnWidth(WICHAT_TRAYNOTE_NOTELIST_FIELD_TEXT, 240);
    ui->listNote->setColumnWidth(WICHAT_TRAYNOTE_NOTELIST_FIELD_COUNT, 30);
    ui->listNote->setStyleSheet("background-color: transparent;");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
}

SystrayNotification::~SystrayNotification()
{
    delete ui;
}

void SystrayNotification::activate()
{
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();  // for MacOS
    activateWindow(); // for Windows
}

void SystrayNotification::addNote(const Notification::Note &note)
{
    if (noteExists(note.ID))
        return;
    else
        noteList.append(note);

    // See if any notification of the same type exists
    for (int i=0; i<listNoteModel.rowCount(); i++)
    {
        if (listNoteModel.item(i, WICHAT_TRAYNOTE_NOTELIST_FIELD_SOURCE)
                ->text() == note.source &&
            listNoteModel.item(i, WICHAT_TRAYNOTE_NOTELIST_FIELD_SOURCE)
                ->text().toInt() == int(note.type))
        {
            // Add notification counter by 1
            int count = listNoteModel.item(i, WICHAT_TRAYNOTE_NOTELIST_FIELD_COUNT)->text().toInt();
            count++;
            listNoteModel.item(i, WICHAT_TRAYNOTE_NOTELIST_FIELD_COUNT)->setText(QString::number(count));
            return;
        }
    }

    // Otherwise, create new notification entry
    QList<QStandardItem*> newRow;
    QStandardItem* item;

    item = new QStandardItem(QString::number(note.ID));
    newRow.append(item);
    item = new QStandardItem(note.source);
    newRow.append(item);
    item = new QStandardItem(QString::number(int(note.type)));
    newRow.append(item);
    switch (note.type) {
        case Notification::NoteEvent::FriendAdd:
            item = new QStandardItem(QIcon(":/Icons/add.png"),
                                     "Friend request");
            break;
        case Notification::NoteEvent::FriendDelete:
            item = new QStandardItem(QIcon(":/Icons/remove.png"),
                                     "Friend removed");
            break;
        case Notification::NoteEvent::GotMsg:
            item = new QStandardItem(QIcon(":/Icons/conversation.png"),
                                     note.source);
            break;
        case Notification::NoteEvent::GotGroupMsg:
            item = new QStandardItem(QIcon(":/Icons/group.png"),
                                     note.source);
            break;
        default:;
    }
    newRow.append(item);
    item = new QStandardItem("1");
    newRow.append(item);

    listNoteModel.appendRow(newRow);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    adjustListSize();
}

void SystrayNotification::removeNote(int noteID)
{
    for (int i=0; i<listNoteModel.rowCount(); i++)
    {
        if (listNoteModel.item(i, WICHAT_TRAYNOTE_NOTELIST_FIELD_ID)
                ->text().toInt() == noteID)
        {
            listNoteModel.removeRow(i);
        }
    }
    noteList.remove(noteID);
    adjustListSize();
}

int SystrayNotification::removeNotes(Notification::NoteEvent noteType,
                                      QString source)
{
    int removedCount = 0;
    QList<Notification::Note> tempList = noteList.getAll();
    for (int i=0; i<tempList.count(); i++)
    {
        if (!source.isEmpty() && source != tempList[i].source)
            continue;
        if (tempList[i].type == noteType)
        {
            noteList.remove(tempList[i].ID);
            removedCount++;
        }
    }
    return removedCount;
}

int SystrayNotification::countNote()
{
    return noteList.count();
}

void SystrayNotification::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    hide();
}

void SystrayNotification::adjustListSize()
{
    int y1, y2;
    y1 = ui->listNote->visualRect(listNoteModel.index(0,
                                                      WICHAT_TRAYNOTE_NOTELIST_FIELD_TEXT))
                     .top();
    y2 = ui->listNote->visualRect(listNoteModel
                                  .index(listNoteModel.rowCount() - 1,
                                         WICHAT_TRAYNOTE_NOTELIST_FIELD_TEXT))
                     .bottom();
    resize(width(),
           ui->labelNoteList->height()
           + y2 - y1
           + ui->frameNoteCtrl->height() + 10);
    ui->listNote->resize(ui->listNote->width(),
                         y2 - y1);
    ui->frameNoteCtrl->move(0, ui->labelNoteList->height() + y2 + 5);
}

bool SystrayNotification::noteExists(int noteID)
{
    for (int i=0; i<noteList.count(); i++)
    {
        if (noteList.peek(noteID).ID == noteID)
            return true;
    }
    return false;
}

void SystrayNotification::on_listNote_clicked(const QModelIndex &index)
{
    int noteID = listNoteModel.item(index.row(),
                                    WICHAT_TRAYNOTE_NOTELIST_FIELD_ID)
                              ->text().toInt();
    emit noteClicked(noteList.peek(noteID));
}

void SystrayNotification::on_labelShowAll_linkActivated(const QString &link)
{
    Q_UNUSED(link)
    int noteID;
    for (int i=0; i<listNoteModel.rowCount(); i++)
    {
        noteID = listNoteModel.item(i, WICHAT_TRAYNOTE_NOTELIST_FIELD_ID)
                               ->text().toInt();
        emit noteClicked(noteList.peek(noteID));
    }
}
