#include <QFile>
#include <QDir>
#include "common.h"
#include "abstractchat.h"
#include "Private/abstracchat_p.h"


QByteArray& AbstractChat::keySalt()
{
    Q_D(AbstractChat);
    return d->keySalt;
}

void AbstractChat::setUserDirectory(QString path)
{
    Q_D(AbstractChat);
    d->userDir = path;
}

void AbstractChat::setPeerSession(PeerSession& sessionList)
{
    Q_D(AbstractChat);
    d->sessionList = &sessionList;
}

bool AbstractChat::resetSession()
{
    // Currently not supported by server
    return false;
}

AbstractChatPrivate::AbstractChatPrivate(AbstractChat* parent,
                                         ServerConnection* server) :
    AbstractServicePrivate(parent, server){}

int AbstractChatPrivate::getAvailableQueryID()
{
    int queryID = 0;
    MessageTransaction* transaction;
    while (true)
    {
        queryID++;
        transaction = getTransactionByQueryID(queryID);
        if (transaction == nullptr)
            break;
    }
    return queryID;
}

AbstractChatPrivate::MessageTransaction*
AbstractChatPrivate::getTransactionByQueryID(int queryID)
{
    for (int i=0; i<sendingList.length(); i++)
    {
        if (sendingList[i].queryID == queryID)
            return &(sendingList[i]);
    }
    for (int i=0; i<receivingList.length(); i++)
    {
        if (receivingList[i].queryID == queryID)
            return &(receivingList[i]);
    }
    return nullptr;
}

AbstractChatPrivate::MessageTransaction*
AbstractChatPrivate::getTransactionByRequestID(int requestID)
{
    for (int i=0; i<sendingList.length(); i++)
    {
        if (sendingList[i].requestID == requestID)
            return &(sendingList[i]);
    }
    for (int i=0; i<receivingList.length(); i++)
    {
        if (receivingList[i].requestID == requestID)
            return &(receivingList[i]);
    }
    return nullptr;
}

void AbstractChatPrivate::removeTransaction(MessageTransaction* transaction)
{
    if (!transaction)
        return;

    if (transaction->data)
        delete transaction->data;
    if (transaction->messages)
        delete transaction->messages;

    int i;
    for (i=0; i< sendingList.count(); i++)
    {
        if (sendingList[i].queryID == transaction->queryID)
        {
            sendingList.removeAt(i);
            break;
        }
    }
    if (i < sendingList.count())
        return;

    for (i=0; i< receivingList.count(); i++)
    {
        if (receivingList[i].queryID == transaction->queryID)
        {
            receivingList.removeAt(i);
            break;
        }
    }
}

void AbstractChatPrivate::dataXMLize(const QByteArray& src, QByteArray& dest)
{
    dest.clear();
    int p, p1, p2, p3, p4;
    QFile file;
    QString fileName;
    QFileInfo fileInfo;
    p = 0;
    dest.clear();

    while (true)
    {
        p1 = src.indexOf("<file>", p);
        p2 = src.indexOf("</file>", p1);
        if (p1 < 0 || p2 < 0)
            break;
        dest.append(src.mid(p, p1 - p)); // Previous data before <D>

        // Parse file name
        p3 = src.indexOf("<name>", p1);
        p4 = src.indexOf("</name>");
        if (p3 >= 0 && p4 >= 0)
            fileName = src.mid(p3 + 6, p4 - p3 - 6);
        else
            fileName.clear();
        file.setFileName(fileName); // File Info

        // Parse file type
        p3 = src.indexOf("<type>", p1);
        p4 = src.indexOf("</type>");
        if (p3 < 0 || p4 < 0)
        {
            // Incomplet file tag, skip it
            p = p2 + 7;
            continue;
        }
        switch (src.mid(p3 + 6, p4 - p3 - 6).at(0))
        {
            case 'i':   // Image file
                if (!file.exists())
                    dest.append("<D t=i l=-1 >");
                else
                {
                    dest.append("<D t=i l=")
                        .append(QString::number(file.size()))
                        .append(" >");
                    file.open(QFile::ReadOnly);
                    dest.append(file.readAll());
                    file.close();
                }
                dest.append("</D>");
                break;
            case 'f':   // Other types of file
            default:
                if (!file.exists())
                    dest.append("<D t=f l=-1 >");
                else
                {
                    fileInfo.setFile(file);
                    dest.append("<D t=f l=")
                        .append(QString::number(file.size()))
                        .append(" n=")
                        .append(fileInfo.fileName().toLatin1())
                        .append(char(0)).append(" >");
                    file.open(QFile::ReadOnly);
                    dest.append(file.readAll());
                    file.close();
                }
                dest.append("</D>");
        }
        p = p2 + 7;
    }
    dest.append(src.mid(p)); // Last data after </D>
}

void AbstractChatPrivate::dataUnxmlize(const QByteArray& src,
                                       QByteArray& dest,
                                       QString cacheDir)
{
    int p, p1, p2, p3;
    QFile file;
    QString fileName;
    qint64 fileLength;
    QByteArray fileContent;
    p = 0;
    dest.clear();

    while (true)
    {
        p1 = src.indexOf("<D t=", p);
        p2 = src.indexOf('>', p1);
        if (p1 < 0 || p2 < 0)
            break;
        dest.append(src.mid(p, p1 - p)); // Previous data before <D>

        // Parse file length
        p3 = src.indexOf("l=", p1);
        if (p3 > 0)
            fileLength = QString(src.mid(p3 + 2,
                                         src.indexOf(' ', p3 + 2) - p3 - 2))
                                .toInt();
        else
            fileLength = -1;
        if (fileLength <= 0)
        {
            // Incomplet file info, skip it
            p = p2 + 1;
            continue;
        }

        // Parse file name
        p3 = src.indexOf("n=", p1);
        if (p3 > 0 && p3 < p2)
            fileName = src.mid(p3 + 2, src.indexOf(' ', p3 + 2));
        else
            fileName.clear();

        QDir fileDir(cacheDir);
        if (!fileDir.exists())
            fileDir.mkpath(cacheDir);

        switch (src.at(p1 + 5))
        {
            case 'i':
                fileContent = src.mid(p2 + 1, fileLength);
                fileName = QString(encoder.getSHA256(fileContent).toHex())
                                  .prepend('/').prepend(cacheDir);
                file.setFileName(fileName);
                file.open(QFile::WriteOnly);
                file.write(fileContent);
                file.close();
                dest.append("<file><type>i</type><name>")
                    .append(fileName)
                    .append("</name></file>");
                break;
            case 'f':
            default:
                if (fileName.isEmpty())
                    break;
                file.setFileName(fileName.prepend('/').prepend(cacheDir));
                file.open(QFile::WriteOnly);
                file.write(src.mid(p2 + 1, fileLength));
                file.close();
                dest.append("<file><type>f</type><name>")
                    .append(fileName)
                    .append("</name></file>");
        }
        p = p2 + fileLength;
        p3 = src.indexOf("</D>", p);
        if (p3 < 0)
            break;
        else
            p = p3 + 4;
    }
    dest.append(src.mid(p));
}

void AbstractChatPrivate::parseAccountList(QByteArray& data,
                                           QByteArray listType,
                                   QList<AbstractChat::MessageListEntry>& list)
{
    int p1, p2, p3, pE;
    AbstractChat::MessageListEntry account;
    if (listType.isEmpty())
        p1 = data.indexOf("<IDList>");
    else
        p1 = data.indexOf(QByteArray("<IDList t=")
                          .append(listType)
                          .append(">"));
    pE = data.indexOf("</IDList>");
    if (p1 >= 0 && pE > 16)
    {
        p3 = p2 = p1;
        while (true)
        {
            p1 = data.indexOf("<ID", p1 + 1);
            p2 = data.indexOf(">", p1 + 1);
            p3 = data.indexOf("</ID>", p2 + 1);
            if (p1 < 0 || p1 < 0 || p1 < 0 || p1 > pE)
                break;

            account.ID = QString(data.mid(p2 + 1, p3 - p2 - 1)).trimmed();
            list.append(account);
        }
    }
}
