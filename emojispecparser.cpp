#include <QFile>
#include <QTemporaryFile>
#include <QVector>
#include "emojispecparser.h"
#include "Private/emojispecparserprivate.h"

#define EMOJI_SPEC_UNICODE_DELIM_RECORD "\n"
#define EMOJI_SPEC_UNICODE_DELIM_GROUP "# group: "
#define EMOJI_SPEC_UNICODE_DELIM_SUBGROUP "# subgroup: "
#define EMOJI_SPEC_UNICODE_DELIM_QUALIFIED ";"
#define EMOJI_SPEC_UNICODE_DELIM_CODE "# "
#define EMOJI_SPEC_UNICODE_DELIM_FULLNAME " "

#define EMOJI_SPEC_UNICODE_VAR_NONQUALIFIED "non-fully-qualified"

#define EMOJI_SPEC_UNICODE_IGNORE_LINE "subtotal:"
#define EMOJI_SPEC_UNICODE_EOF "#EOF"


EmojiSpecParser::EmojiSpecParser()
{
    this->d_ptr = new EmojiSpecParserPrivate(this);
}

EmojiSpecParser::~EmojiSpecParser()
{
    clear();
    delete this->d_ptr;
}

bool EmojiSpecParser::parseSpec(const QByteArray &content, SpecType type)
{
    // Dump content to a temporary file
    QTemporaryFile tempFile;
    if (!tempFile.open())
        return false;
    tempFile.write(content);
    tempFile.close();

    return parseSpecFile(tempFile.fileName(), type);
}

bool EmojiSpecParser::parseSpecFile(const QString& fileName,
                                    SpecType type)
{
    Q_D(EmojiSpecParser);

    bool successful = false;
    switch (type)
    {
        case EmojiSpecParser::UnicodeEmojiSpec:
            successful = d->parseUnicodeSpecFile(fileName);
            break;
        default:;
    }
    return successful;
}

int EmojiSpecParser::count()
{
    Q_D(EmojiSpecParser);
    return d->emojiCount;
}

void EmojiSpecParser::clear()
{
    Q_D(EmojiSpecParser);
    d->fileName.clear();
    d->removeAll();
}

bool EmojiSpecParser::existsCode(const QByteArray& unicode)
{
    Q_D(EmojiSpecParser);
    for (int i=0; i<d->emojiList.count(); i++)
    {
        if (d->emojiList[i].unicode == unicode)
            return true;
    }
    return false;
}

QByteArray EmojiSpecParser::getCode(QString name)
{
    Q_D(EmojiSpecParser);
    for (int i=0; i<d->emojiList.count(); i++)
    {
        if (d->emojiList[i].fullName == name)
            return QByteArray(d->emojiList[i].unicode,
                              d->emojiList[i].codeLength);
    }
    return QByteArray();
}

const QList<QByteArray> EmojiSpecParser::getCodeByGroup(QString groupName)
{
    Q_D(EmojiSpecParser);
    QList<QByteArray> codeList;
    int index = d->groupList.indexOf(groupName);
    if (index < 0)
        return codeList;

    QList<int> emojiIndexes = d->groupMap.values(index);
    for (int i=0; i<emojiIndexes.count(); i++)
    {
        index = emojiIndexes[i];
        codeList.append(QByteArray(d->emojiList[index].unicode,
                                   d->emojiList[index].codeLength));
    }
    return codeList;
}

const QList<QByteArray> EmojiSpecParser::getCodeBySubGroup(QString subGroupName)
{
    Q_D(EmojiSpecParser);
    QList<QByteArray> codeList;
    int index = d->subGroupList.indexOf(subGroupName);
    if (index < 0)
        return codeList;

    for (int i=0; i<d->emojiList.count(); i++)
    {
        if (d->emojiList[i].subGroupIndex == index)
            codeList.append(QByteArray(d->emojiList[i].unicode,
                                       d->emojiList[i].codeLength));
    }
    return codeList;
}

bool EmojiSpecParser::existsName(const QString name)
{
    Q_D(EmojiSpecParser);
    for (int i=0; i<d->emojiList.count(); i++)
    {
        if (d->emojiList[i].fullName == name)
            return true;
    }
    return false;
}

QString EmojiSpecParser::getName(const QByteArray& unicode)
{
    Q_D(EmojiSpecParser);
    for (int i=0; i<d->emojiList.count(); i++)
    {
        if (strncmp(d->emojiList[i].unicode,
                    unicode.constData(),
                    unicode.length()) == 0)
            return d->emojiList[i].fullName;
    }
    return "";
}

const QList<QString> EmojiSpecParser::getGroupNames()
{
    Q_D(EmojiSpecParser);
    return d->groupList;
}

const QList<QString> EmojiSpecParser::getSubGroupNames(QString groupName)
{
    Q_D(EmojiSpecParser);
    QList<QString> subGroups;
    QList<int> subGroupIndexes = d->subGroupMap.values(
                                            d->groupList.indexOf(groupName));
    for (int i=0; i<subGroupIndexes.count(); i++)
        subGroups.append(d->subGroupList[subGroupIndexes[i]]);

    return subGroups;
}

EmojiSpecParserPrivate::EmojiSpecParserPrivate(EmojiSpecParser* parent)
{
    this->q_ptr = parent;
    emojiCount = 0;
}

void EmojiSpecParserPrivate::removeAll()
{
    if (emojiList.count() > 0)
    {
        for (int i=0; i<emojiList.count(); i++)
        {
            if (emojiList[i].unicode != nullptr)
                delete emojiList[i].unicode;
        }
        emojiCount = 0;
        emojiList.clear();
        groupMap.clear();
        subGroupMap.clear();
    }
}

bool EmojiSpecParserPrivate::parseUnicodeSpecFile(const QString &fileName)
{
    QFile specFile(fileName);
    QByteArray buffer, content;
    int p = 0;

    specFile.open(QFile::ReadOnly);
    if (specFile.error() != QFile::NoError)
        return false;

    // Find spec file body
    while (!specFile.atEnd())
    {
        buffer = specFile.readLine();
        p = buffer.indexOf(EMOJI_SPEC_UNICODE_DELIM_GROUP);
        if (p >= 0)
            break;
    }
    if (p < 0)
        return false;
    else
        specFile.seek(specFile.pos() - buffer.length());

    removeAll();
    buffer.clear();

    // Parse each emoji group
    bool foundDeliminator = false;
    while (!specFile.atEnd())
    {
        buffer = specFile.readLine();
        if (buffer.indexOf(EMOJI_SPEC_UNICODE_IGNORE_LINE) >= 0)
            continue;
        if (buffer.indexOf(EMOJI_SPEC_UNICODE_EOF) >= 0)
            break;
        content.append(buffer);
        p = buffer.indexOf(EMOJI_SPEC_UNICODE_DELIM_GROUP);
        if (p >= 0)
        {
            if (foundDeliminator)
            {
                emojiList.append(parseUnicodeSpecGroup(content));
                content.clear();
                specFile.seek(specFile.pos() - buffer.length());
                foundDeliminator = false;
            }
            else
                foundDeliminator = true;
        }
    }
    if (content.length() > 0)
        emojiList.append(parseUnicodeSpecGroup(content));

    specFile.close();
    return true;
}

QList<EmojiSpecParserPrivate::EmojiEntry>
EmojiSpecParserPrivate::parseUnicodeSpecGroup(const QByteArray &content)
{
    QList<EmojiSpecParserPrivate::EmojiEntry> tempList;
    int p1 = content.indexOf(EMOJI_SPEC_UNICODE_DELIM_GROUP);
    int p2 = content.indexOf("\n", p1);
    if (p1 < 0 || p2 < 0)
        return tempList;

    int groupIndex = groupList.count();

    int dimLength = strlen(EMOJI_SPEC_UNICODE_DELIM_GROUP);
    QString groupName = content.mid(p1 + dimLength, p2 - p1 - dimLength);
    groupList.append(groupName);

    // Parse each subgroup
    p1 = p2;
    while (p2 >= 0)
    {
        p2 = content.indexOf(EMOJI_SPEC_UNICODE_DELIM_SUBGROUP, p1 + 1);
        if (p2 >= 0)
        {
            tempList.append(parseUnicodeSpecSubGroup(content.mid(p1, p2 - p1),
                                                      groupIndex));
            p1 = p2;
        }
        else
            break;
    }
    if (p2 >= 0 && p2 < content.length() - 1)
        tempList.append(parseUnicodeSpecSubGroup(content.mid(p2 + 1),
                                                  groupIndex));

    return tempList;
}

QList<EmojiSpecParserPrivate::EmojiEntry>
EmojiSpecParserPrivate::parseUnicodeSpecSubGroup(const QByteArray &content,
                                                 const int groupIndex)
{
    QList<EmojiSpecParserPrivate::EmojiEntry> tempList;
    int p1 = content.indexOf(EMOJI_SPEC_UNICODE_DELIM_SUBGROUP);
    int p2 = content.indexOf("\n", p1);
    if (p1 < 0 || p2 < 0)
        return tempList;

    // Create a map from the subgroup to the group
    int subGroupIndex = subGroupList.count();
    subGroupMap.insertMulti(groupIndex, subGroupIndex);

    int dimLength = strlen(EMOJI_SPEC_UNICODE_DELIM_SUBGROUP);
    QString subGroupName = content.mid(p1 + dimLength, p2 - p1 - dimLength);
    subGroupList.append(subGroupName);

    // Parse each record
    EmojiEntry record;
    p1 = p2 + 1;
    dimLength = strlen(EMOJI_SPEC_UNICODE_DELIM_RECORD);
    while (p2 >= 0)
    {
        p2 = content.indexOf(EMOJI_SPEC_UNICODE_DELIM_RECORD, p1 + 1);
        if (p2 >= 0)
        {
            record = parseUnicodeSpecRecord(content.mid(p1 + dimLength,
                                                        p2 - p1 - dimLength));
            if (record.unicode)
            {
                record.ID = ++emojiCount;
                record.groupIndex = groupIndex;
                record.subGroupIndex = subGroupIndex;
                tempList.append(record);

                // Create maps from the emoji entry to the group
                groupMap.insertMulti(groupIndex, emojiCount - 1);
            }
            p1 = p2 + 1;
        }
        else
            break;
    }
    // Do not bother reading the rest part of "content",
    // as we assume that each subgroup is terminated by a deliminator ("\n")

    return tempList;
}

EmojiSpecParserPrivate::EmojiEntry
EmojiSpecParserPrivate::parseUnicodeSpecRecord(const QByteArray &content)
{
    int p1, p2;
    QByteArray buffer;
    EmojiSpecParserPrivate::EmojiEntry record;

    // Parse property of qualification
    p1 = content.indexOf(EMOJI_SPEC_UNICODE_DELIM_QUALIFIED);
    p2 = content.indexOf(EMOJI_SPEC_UNICODE_DELIM_CODE, p1 + 1);
    if (p1 >= 0)
    {
        buffer = content.mid(p1 + 1, p2 - p1);
        if (buffer.indexOf(EMOJI_SPEC_UNICODE_VAR_NONQUALIFIED) < 0)
            record.qualified = true;
        else
            record.qualified = false;
    }

    // Parse emoji code
    record.unicode = nullptr;
    p1 = p2;
    int dimLength = strlen(EMOJI_SPEC_UNICODE_DELIM_CODE);
    int codeLength;
    QVector<uint> codeBuffer;
    p2 = content.indexOf(EMOJI_SPEC_UNICODE_DELIM_FULLNAME, p1 + dimLength);
    if (p1 >= 0)
    {
        // Original format: UTF-8
        // Convert it to unicode (reduced UCS-4 / UTF-32)
        buffer = content.mid(p1 + dimLength, p2 - p1 - dimLength);
        codeBuffer = QString::fromUtf8(buffer).toUcs4();
        codeLength = codeBuffer.length() * sizeof(uint);
        buffer = QByteArray::fromRawData((const char*)(codeBuffer.data()),
                                         codeLength);

        // Find and remove leading zeros by byte (char)
        while (buffer.at(codeLength - 1) == '\0' && codeLength >= 0)
            codeLength--;
        if (codeLength >= 0)
        {
            record.unicode = new char[codeLength];
            record.codeLength = codeLength;
            strncpy(record.unicode, buffer.constData(), codeLength);
        }
    }

    // Parse full name
    record.fullName = content.mid(p2 +
                                  strlen(EMOJI_SPEC_UNICODE_DELIM_FULLNAME));

    return record;
}
