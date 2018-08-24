#include <QDesktopServices>
#include <QFileDialog>
#include <QImageReader>
#include <QScrollBar>

#include "sessionpresenter.h"
#include "ui_sessionpresenter.h"
#include "config_field.h"
#include "emoticonchooser.h"
#include "Modules/sessionmessagelist.h"
#include "Modules/htmlhelper.h"

#define WICHAT_SESSION_MENU_IMAGE_OPEN 1
#define WICHAT_SESSION_MENU_IMAGE_SAVEAS 2

#define WICHAT_SESSION_TIME_FORMAT "hh:mm:ss"
#define WICHAT_SESSION_DATETIME_FORMAT "yyyy-MM-dd hh:mm:ss"


SessionPresenter::SessionPresenter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionPresenter)
{
    ui->setupUi(this);
    menuBrowser = nullptr;

    isLoaded = false;
    isAvailable = true;
    emoticonListBinded = false;
    loadedHeadID = -1;
    loadedTailID = -1;

    connect(ui->textBrowser->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            this,
            SLOT(onBrowserPageScrolled(int)));
}

SessionPresenter::~SessionPresenter()
{
    delete ui;
}

void SessionPresenter::bindEmoticonList(EmoticonChooser* list)
{
    emoticonList = list;
    if (emoticonList == nullptr)
    {
        emoticonListBinded = false;
        return;
    }
    else
        emoticonListBinded = true;
}

QString SessionPresenter::ID()
{
    return sessionID;
}

void SessionPresenter::setID(QString ID)
{
    sessionID = ID;
}

void SessionPresenter::setUserID(QString ID)
{
    userID = ID;
}

bool SessionPresenter::available()
{
    return isAvailable;
}

void SessionPresenter::setAvailable(bool available)
{
    isAvailable = available;
}

bool SessionPresenter::loaded()
{
    return isLoaded;
}

void SessionPresenter::load(const SessionMessageList& list, int depth)
{
    messageList = &list;

    // Select a part of the session data to display
    int lastMessageID = list.last().ID;
    QList<SessionMessageList::MessageEntry> messages =
                                list.getMessageByRange(lastMessageID, -depth);
    if (messages.count() > 0)
    {
        loadedHeadID = messages.last().ID;
        loadedTailID = messages.first().ID;
    }

    // Load cached session content from session data
    QString buffer;
    buffer.append(HtmlHelper::getHTMLHeader(0));
    for (int i=0; i<messages.count(); i++)
        buffer.append(renderMessage(messages[i]));
    buffer.append(HtmlHelper::getHTMLFooter(0));

    // Set textBrowser unavailable (locked) to prevent event deadlock
    isAvailable = false;
    ui->textBrowser->setHtml(buffer);
    isAvailable = true;

    isLoaded = true;
}

void SessionPresenter::unload()
{
    ui->textBrowser->clear();
    isLoaded = false;
}

void SessionPresenter::refresh()
{
    // Load new messages (if any)
    int headID = messageList->last().ID;
    if (headID >= 0 && headID > loadedHeadID)
        loadMore(false);
}

void SessionPresenter::scrollToTop()
{
    ui->textBrowser->verticalScrollBar()->setValue(
                ui->textBrowser->verticalScrollBar()->minimum());
}

void SessionPresenter::scrollToBottom()
{
    ui->textBrowser->verticalScrollBar()->setValue(
                ui->textBrowser->verticalScrollBar()->maximum());
}

void SessionPresenter::loadMore(bool olderMessages)
{
    int start, stop;

    if (olderMessages)
    {
        stop = loadedTailID;
        start = stop - WICHAT_SESNFRAME_CONTENT_LOAD_STEP;
        if (start < messageList->first().ID)
            start = messageList->first().ID;
    }
    else
    {
        start = loadedHeadID + 1;
        stop = start + WICHAT_SESNFRAME_CONTENT_LOAD_STEP;
        if (stop > messageList->last().ID + 1)
            stop = messageList->last().ID + 1;
    }
    if (start >= stop)
        return;

    QString htmlBuffer;
    SessionMessageList::MessageEntry message;
    for (int i=start; i<stop; i++)
    {
        message = messageList->getMessageByID(i);
        if (message.ID >= 0)
            htmlBuffer.append(renderMessage(message));
    }
    if (!htmlBuffer.isEmpty())
    {
        htmlBuffer.prepend(HtmlHelper::getHTMLHeader(0));
        htmlBuffer.append(HtmlHelper::getHTMLFooter(0));
    }

    isAvailable = false;
    if (olderMessages)
    {
        int oldPageHeight = ui->textBrowser->verticalScrollBar()->maximum();
        ui->textBrowser->moveCursor(QTextCursor::Start);
        ui->textBrowser->insertHtml(htmlBuffer);
        ui->textBrowser->verticalScrollBar()->setValue(
              ui->textBrowser->verticalScrollBar()->maximum() - oldPageHeight);
        loadedTailID = start;
    }
    else
    {
        ui->textBrowser->append(htmlBuffer);
        loadedHeadID = stop - 1;
    }
    isAvailable = true;
}

QString SessionPresenter::renderMessage(
                                const SessionMessageList::MessageEntry& message,
                                bool fullHTML)
{
    int p1, p2;
    QString temp;
    QString header("<div class=%SENDER%><b>%ID%</b>&nbsp;&nbsp;%TIME%</div>");
    QString result(message.content);

    // Deail with sender's ID
    if (message.source == userID)
        header.replace("%SENDER%", "s");
    else
        header.replace("%SENDER%", "r");
    header.replace("%ID%", message.source);

    // Deail with date/time (UTC => Local Time)
    QDateTime localTime(message.time.toLocalTime());
    if (localTime.date().daysTo(QDate::currentDate()) < 1)
        header.replace("%TIME%",
                       localTime.toString(WICHAT_SESSION_TIME_FORMAT));
    else
        header.replace("%TIME%",
                       localTime.toString(WICHAT_SESSION_DATETIME_FORMAT));

    // Deal with emoticon
    QByteArray emoticon;
    p2 = 0;
    while(true)
    {
        p1 = result.indexOf("<emoticon>emoji:", p2);
        p2 = result.indexOf("</emoticon>", p1 + 15);
        if (p1 < 0 || p2 < 0)
            break;

        emoticon = QByteArray::fromHex(result.mid(p1 + 15, p2 - p1 - 15)
                                             .toUtf8());
        if (emoticonListBinded)
            temp = QString("<img src=\"%1\" alt=\"%2\" class=\"emoji\" />")
                    .arg(emoticonList->getImagePathFromCode(emoticon))
                    .arg(emoticonList->getImageNameFromCode(emoticon));
        else
            temp = fallbackStringFromCode(emoticon);
        result.replace(p1, p2 - p1 + 11, temp);
    }

    // Deal with image
    p2 = 0;
    while(true)
    {
        p1 = result.indexOf("<file><type>i</type><name>", p2);
        p2 = result.indexOf("</name></file>", p1);
        if (p1 < 0 || p2 < 0)
            break;
        QString fileName = result.mid(p1 + 26, p2 - p1 - 26);
        QImageReader image(fileName);
        int displayWidth = image.size().width();
        if (displayWidth > ui->textBrowser->width())
            displayWidth = ui->textBrowser->width() - 20;
        temp = QString("<img src=\"")
                    .append(fileName)
                    .append("\" alt=Image width=%1 />")
                    .arg(QString::number(displayWidth));

        result.replace(p1, p2 - p1, temp);
    }

    // Deal with file
    p2 = 0;
    while(true)
    {
        p1 = result.indexOf("<file><type>f</type><name>", p2);
        p2 = result.indexOf("</name></file>", p1);
        if (p1 < 0 || p2 < 0)
            break;
        QString fileName = result.mid(p1 + 26, p2 - p1 - 26);

        QString noteText;
        if (message.source == userID)
            noteText = QString("You sent file \"%1\" to him/her.")
                              .arg(getFileNameFromPath(fileName));
        else
            noteText = QString("He/she sent file \"%1\" to you.")
                              .arg(getFileNameFromPath(fileName));
        temp = QString("<div style=\"width:300px;border:1px solid;\">"
                          "<div style=\"float:right\">")
                    .append(noteText)
                    .append("<a href=\"")
                    .append(fileName)
                    .append("\" target=_blank>View the file</a></div></div>");

        result.replace(p1, p2 - p1, temp);
    }

    result.prepend(header);
    if (fullHTML)
    {
        result.prepend(HtmlHelper::getHTMLHeader(0));
        result.append(HtmlHelper::getHTMLFooter(0));
    }
    return result;
}

QString SessionPresenter::fallbackStringFromCode(const QByteArray& emoticon)
{
    const uint* unicode = (const uint*)(emoticon.constData());
    int unicodeLength = emoticon.length() / sizeof(uint) +
                        (emoticon.length() % sizeof(uint) > 0);
    return QString::fromUcs4(unicode, unicodeLength);
}

QString SessionPresenter::getFileNameFromPath(QString filePath)
{
    char pathSep;
    if (filePath.indexOf('/') >= 0)
        pathSep  = '/';
    else if (filePath.indexOf('\\') >= 0)
        pathSep  = '\\';
    else
        return filePath;
    return filePath.mid(filePath.lastIndexOf(pathSep) + 1);
}

void SessionPresenter::onBrowserPageScrolled(int pos)
{
    if (!isAvailable)
        return;
    if (pos <= 0 || pos >= ui->textBrowser->verticalScrollBar()->maximum())
        loadMore(pos <= 0);
}

void SessionPresenter::onBrowserMenuClicked(QAction* action)
{
    bool ok;
    int actionIndex = action->data().toInt(&ok);
    if (!ok)
        return;

    QTextCharFormat character = ui->textBrowser->textCursor().charFormat();
    switch (actionIndex)
    {
        case WICHAT_SESSION_MENU_IMAGE_OPEN:
            if (character.isImageFormat())
                on_textBrowser_anchorClicked(
                        QUrl::fromLocalFile(character.toImageFormat().name()));
            break;
        case WICHAT_SESSION_MENU_IMAGE_SAVEAS:
            if (character.isImageFormat())
            {
                lastFilePath = character.toImageFormat().name();
                QString path;
                path = QFileDialog::getSaveFileName(this,
                                                    "Save image file as",
                                                    lastFilePath);
                if (!path.isEmpty())
                    QFile::copy(lastFilePath, path);
            }
            break;
        default:;
    }
}

void SessionPresenter::on_textBrowser_anchorClicked(const QUrl &arg1)
{
    QDesktopServices::openUrl(arg1);
}

void SessionPresenter::on_textBrowser_customContextMenuRequested(const QPoint &pos)
{
    if (menuBrowser)
    {
        disconnect(menuBrowser);
        delete menuBrowser;
    }

    menuBrowser = ui->textBrowser->createStandardContextMenu(pos);
    //menuBrowser->setParent((QWidget*)(parent()));

    QTextCursor cursor = ui->textBrowser->textCursor();
    if (cursor.charFormat().isImageFormat())
    {
        QAction* newAction = new QAction(menuBrowser);
        newAction->setText("View image");
        newAction->setData(WICHAT_SESSION_MENU_IMAGE_OPEN);
        menuBrowser->addAction(newAction);

        newAction = new QAction(menuBrowser);
        newAction->setText("Save image as");
        newAction->setData(WICHAT_SESSION_MENU_IMAGE_SAVEAS);
        menuBrowser->addAction(newAction);
    }

    connect(menuBrowser,
            SIGNAL(triggered(QAction*)),
            this,
            SLOT(onBrowserMenuClicked(QAction*)));
    menuBrowser->popup(pos);
}
