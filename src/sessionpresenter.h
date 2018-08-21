#ifndef SESSIONPRESENTER_H
#define SESSIONPRESENTER_H

#include <QWidget>
#include <Modules/sessionmessagelist.h>

#define WICHAT_SESNFRAME_CONTENT_LOAD_STEP 10


class EmoticonChooser;

namespace Ui {
class SessionPresenter;
}

class SessionPresenter : public QWidget
{
    Q_OBJECT

public:
    explicit SessionPresenter(QWidget *parent = 0);
    ~SessionPresenter();
    void bindEmoticonList(EmoticonChooser* list);

    QString ID();
    void setID(QString ID);
    void setUserID(QString ID);
    bool available();
    void setAvailable(bool available);
    bool loaded();

    void load(const SessionMessageList &list,
              int depth = WICHAT_SESNFRAME_CONTENT_LOAD_STEP);
    void unload();
    void refresh();

    void scrollToTop();
    void scrollToBottom();

signals:
    void browserLinkClicked(const QUrl& url, QString sessionID);

private:
    Ui::SessionPresenter *ui;
    const SessionMessageList* messageList;
    EmoticonChooser* emoticonList;
    QString sessionID;
    QString userID;
    int loadedHeadID;
    int loadedTailID;
    bool isAvailable;
    bool isLoaded;
    bool emoticonListBinded;

    void loadMore(bool olderMessages = true);
    QString renderMessage(const SessionMessageList::MessageEntry& message,
                          bool fullHTML = false);
    QString fallbackStringFromCode(const QByteArray& emoticon);
    static QString getFileNameFromPath(QString filePath);

private slots:
    void onBrowserPageScrolled(int pos);
    void on_textBrowser_anchorClicked(const QUrl &arg1);
};

#endif // SESSIONPRESENTER_H
