#ifndef EMOJISPECPARSER_H
#define EMOJISPECPARSER_H

/*
 * Parser of emoji specification files
 * See corresponding spec files for mmore details:
 *
 * Unicode emoji test data (5.0):
 *      http://http://unicode.org/Public/emoji/5.0/emoji-test.txt
*/

#include <QObject>


class EmojiSpecParserPrivate;

class EmojiSpecParser : public QObject
{
    Q_DECLARE_PRIVATE(EmojiSpecParser)
protected:
    EmojiSpecParserPrivate* d_ptr;

public:
    enum SpecType
    {
        none = 0,
        UnicodeEmojiSpec = 1
    };

    explicit EmojiSpecParser();
    ~EmojiSpecParser();

    bool parseSpec(const QByteArray& content, SpecType type);
    bool parseSpecFile(const QString& fileName, SpecType type);

    int count();
    void clear();

    bool existsCode(const QByteArray& unicode);
    QByteArray getCode(QString name);
    const QList<QByteArray> getCodeByGroup(QString groupName);
    const QList<QByteArray> getCodeBySubGroup(QString subGroupName);

    bool existsName(const QString name);
    QString getName(const QByteArray& unicode);

    const QList<QString> getGroupNames();
    const QList<QString> getSubGroupNames(QString groupName);
};

#endif // EMOJISPECPARSER_H
