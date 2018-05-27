#ifndef EMOJISPECPARSERPRIVATE_H
#define EMOJISPECPARSERPRIVATE_H

#include <QObject>
#include <QMap>


class EmojiSpecParser;

class EmojiSpecParserPrivate : QObject
{
    Q_DECLARE_PUBLIC(EmojiSpecParser)
protected:
    EmojiSpecParser* q_ptr;

public:
    struct EmojiEntry
    {
        int ID;
        int codeLength;
        char* unicode;
        QString fullName;
        int groupIndex;
        int subGroupIndex;
        bool qualified;
    };

    QString fileName;
    int emojiCount;
    QList<EmojiEntry> emojiList;
    QList<QString> groupList;
    QList<QString> subGroupList;
    QMap<int, int> groupMap; // Group index => Emoji indexes
    QMap<int, int> subGroupMap; // Group index => Sub-group indexes

    EmojiSpecParserPrivate(EmojiSpecParser* parent = nullptr);
    void removeAll();

    bool parseUnicodeSpecFile(const QString& fileName);
    QList<EmojiEntry> parseUnicodeSpecGroup(const QByteArray& content);
    QList<EmojiEntry> parseUnicodeSpecSubGroup(const QByteArray& content,
                                               const int groupIndex);
    EmojiEntry parseUnicodeSpecRecord(const QByteArray& content);
};

#endif // EMOJISPECPARSERPRIVATE_H
