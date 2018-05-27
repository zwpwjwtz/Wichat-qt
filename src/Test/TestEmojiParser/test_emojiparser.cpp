#include <QtTest/QtTest>
#include "../emojispecparser.h"

class TestEmojiParser : public QObject
{
    Q_OBJECT

public:
    EmojiSpecParser parser;

    QByteArray input;
    QByteArray output;

    int testEmojiCount = 3204;
    const char* testFile = "../../Emoticon/Unicode/5.0/emoji-test.txt";
    const char* testEmojiName = "face with rolling eyes";
    const char* testEmojiCode = "\x44\xf6\x01";
    const int testEmojiCodeLength = 3;

private slots:
    void countEmoji();
    void getNameByCode();
    void getCodeByName();
};

void TestEmojiParser::countEmoji()
{
    parser.parseSpecFile(testFile, EmojiSpecParser::UnicodeEmojiSpec);
    QCOMPARE(parser.count(),
             testEmojiCount);
}

void TestEmojiParser::getCodeByName()
{
    parser.parseSpecFile(testFile, EmojiSpecParser::UnicodeEmojiSpec);
    QCOMPARE(parser.getCode(testEmojiName).toHex(),
             QByteArray(testEmojiCode).toHex());
}

void TestEmojiParser::getNameByCode()
{
    parser.parseSpecFile(testFile, EmojiSpecParser::UnicodeEmojiSpec);
    QCOMPARE(parser.getName(QByteArray(testEmojiCode,
                                       testEmojiCodeLength)),
             QString(testEmojiName));
}

QTEST_MAIN(TestEmojiParser)
#include "test_emojiparser.moc"
