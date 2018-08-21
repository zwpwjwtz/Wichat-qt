#ifndef EMOTICONCHOOSER_H
#define EMOTICONCHOOSER_H

#include <QWidget>
#include <QStandardItemModel>
#include "Modules/emojispecparser.h"


class QShowEvent;

namespace Ui {
class EmoticonChooser;
}

class EmoticonChooser : public QWidget
{
    Q_OBJECT

public:
    explicit EmoticonChooser(QWidget *parent = 0);
    ~EmoticonChooser();

    void init();
    void setResourceDir(QString dirPath);
    QString getImageNameFromCode(const QByteArray& emoticon);
    QString getImagePathFromCode(const QByteArray& emoticon);

signals:
    void emoticonClicked(const QByteArray& emoticon);

protected:
    void showEvent(QShowEvent* event);

private:
    Ui::EmoticonChooser *ui;
    EmojiSpecParser emojiList;
    QStandardItemModel listPreferredModel;
    QStandardItemModel listChooserModel;
    QStandardItemModel listClassifierModel;
    QString emojiResourceDir;
    bool listExpanded;
    int currentGroup;

    void expandList();
    void collapseList();

    void loadImages();
    QString getEmojiFileName(const QByteArray& code);

private slots:
    void on_buttonExpand_clicked();
    void on_listChooser_clicked(const QModelIndex &index);
    void on_listClassifier_clicked(const QModelIndex &index);
    void on_listPreferred_clicked(const QModelIndex &index);
};

#endif // EMOTICONCHOOSER_H
