#include <QDir>
#include <QFile>
#include "emoticonchooser.h"
#include "ui_emoticonchooser.h"

#define WICHAT_EMOTICON_SPEC_FILE ":/Emoticon/Unicode/5.0/emoji-test.txt"

#define WICHAT_EMOJI_RESOURCE_DIR "emoticons/Twemoji/72x72"
#define WICHAT_EMOJI_RESOURCE_IMAGE_SUFFIX ".png"

#define WICHAT_EMOJI_PREFERRED_MAX 10


EmoticonChooser::EmoticonChooser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmoticonChooser)
{
    ui->setupUi(this);
    ui->listPreferred->setModel(&listPreferredModel);
    ui->listChooser->setModel(&listChooserModel);
    ui->listClassifier->setModel(&listClassifierModel);

    emojiResourceDir = QCoreApplication::applicationDirPath()
                        .append(WICHAT_EMOJI_RESOURCE_DIR);

    listPreferredModel.setColumnCount(1);
    listChooserModel.setColumnCount(1);
    listClassifierModel.setColumnCount(1);
    emojiList.parseSpecFile(WICHAT_EMOTICON_SPEC_FILE,
                            EmojiSpecParser::UnicodeEmojiSpec);

    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
}

EmoticonChooser::~EmoticonChooser()
{
    delete ui;
}

void EmoticonChooser::init()
{
    static bool initialized = false;
    if (initialized)
        return;

    loadImages();
    initialized = true;
}

void EmoticonChooser::setResourceDir(QString dirPath)
{
    QDir resourceDir(dirPath);
    if (!resourceDir.exists())
        return;

    resourceDir.filePath(WICHAT_EMOJI_RESOURCE_DIR);
//    emojiResourceDir = QDir::cleanPath(resourceDir.path()
//                                       .append(QDir::separator())
//                                       .append(WICHAT_EMOJI_RESOURCE_DIR));
    emojiResourceDir = resourceDir.absoluteFilePath(WICHAT_EMOJI_RESOURCE_DIR);
}

void EmoticonChooser::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    init();
    if (listPreferredModel.rowCount() < 1)
    {
        ui->framePreferred->hide();
        expandList();
    }
    else
        ui->framePreferred->show();
}

void EmoticonChooser::expandList()
{
//    resize(width(),
//           ui->framePreferred->height() +
//           ui->frameChooser->height());
    ui->frameChooser->show();
    ui->buttonExpand->setText("▲");
    listExpanded = true;
}

void EmoticonChooser::collapseList()
{
//    resize(width(),
//           ui->framePreferred->height());
    ui->frameChooser->hide();
    ui->buttonExpand->setText("▼");
    listExpanded = false;
}

void EmoticonChooser::loadImages()
{
    if (emojiList.count() < 1)
        return;
    QDir resourceDir(emojiResourceDir);
    if (!resourceDir.exists())
        return;

    listClassifierModel.clear();
    listChooserModel.clear();
    listPreferredModel.clear();

    int i, j, k;
    QList<QString> groups = emojiList.getGroupNames();
    QList<QByteArray> emojiCodeList;
    QByteArray tempCode;
    QString emojiFileName;
    QFile emojiFile;
    QStandardItem* emojiIcon;
    QList<QStandardItem*> emojiIconGroup;
    for (i=0; i<groups.count(); i++)
    {
        emojiIconGroup.clear();

        // Get a list of unicode of emojis
        // Note that this list is in inversed order
        emojiCodeList = emojiList.getCodeByGroup(groups[i]);
        for (j=emojiCodeList.count() - 1; j>=0; j--)
        {
            // Invert emoji hex unicode (because it is of little endian)
            tempCode.clear();
            for (k=emojiCodeList[j].length() - 1; k>=0; k--)
                tempCode.append(emojiCodeList[j].at(k));

            // Get emoji file name from hex code, then trim leading zeros.
            emojiFileName = tempCode.toHex()
                            .append(WICHAT_EMOJI_RESOURCE_IMAGE_SUFFIX);
            if (emojiFileName.at(0) == '0')
                emojiFileName.remove(0, 1);
            emojiFile.setFileName(resourceDir.filePath(emojiFileName));
            if (!emojiFile.exists())
                continue;

            emojiIcon = new QStandardItem(QIcon(emojiFile.fileName()), "");
            emojiIcon->setData(emojiCodeList[j]);
            emojiIconGroup.append(emojiIcon);
        }
        if (emojiIconGroup.count() > 0)
        {
            listChooserModel.appendColumn(emojiIconGroup);

            // Use the first emoji as an icon of its group
            listClassifierModel.appendRow(
                        new QStandardItem(emojiIconGroup[0]->icon(), ""));
        }
    }
    currentGroup = 0;
}

void EmoticonChooser::on_buttonExpand_clicked()
{
    if (listExpanded)
        collapseList();
    else
        expandList();
}
void EmoticonChooser::on_listChooser_clicked(const QModelIndex &index)
{
    const QStandardItem* item = listChooserModel.item(index.row(), currentGroup);
    emit emoticonClicked(item->data().toByteArray());

    // See if the selected emoticon already exists in preferred list
    for (int i=0; i<listPreferredModel.rowCount(); i++)
    {
        if (listPreferredModel.item(i, 0)->data() == item->data())
            return;
    }

    // Add the selected emoticon to preferred list
    QStandardItem* newItem = new QStandardItem(item->icon(), item->text());
    newItem->setData(item->data());
    listPreferredModel.insertRow(0, newItem);
    if (listPreferredModel.rowCount() > WICHAT_EMOJI_PREFERRED_MAX)
        listPreferredModel.removeRow(WICHAT_EMOJI_PREFERRED_MAX);
}

void EmoticonChooser::on_listClassifier_clicked(const QModelIndex &index)
{
    currentGroup = index.row();
    ui->listChooser->setModelColumn(currentGroup);
}

void EmoticonChooser::on_listPreferred_clicked(const QModelIndex &index)
{
    int row = index.row();
    QStandardItem* item = listPreferredModel.takeItem(row, 0);
    emit emoticonClicked(item->data().toByteArray());

    // Move the selected emoticon to the first place
    listPreferredModel.removeRow(row);
    listPreferredModel.insertRow(0, item);
}
