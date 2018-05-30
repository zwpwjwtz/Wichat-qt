#include <QDir>

#include "abstractsession.h"
#include "Private/abstractsession_p.h"

#define WICHAT_SESSION_CACHE_READ_PROBELEN 64


AbstractSession::AbstractSession()
{
    this->d_ptr = new AbstractSessionPrivate(this);
}

AbstractSession::AbstractSession(AbstractSessionPrivate* d)
{
    this->d_ptr = d;
}

AbstractSession::~AbstractSession()
{
    delete this->d_ptr;
}

AbstractSessionPrivate::AbstractSessionPrivate()
{
    this->q_ptr = nullptr;
}

AbstractSessionPrivate::AbstractSessionPrivate(AbstractSession *parent)
{
    this->q_ptr = parent;
}

QString AbstractSessionPrivate::getSessionFile(QString userDir,
                                               QString fileName)
{
    QDir dir(userDir);
    if (!dir.exists())
        return "";
    return dir.filePath(fileName);
}

QByteArray AbstractSessionPrivate::readUntil(QFile& file,
                                             const QByteArray delimiter)
{
    int p;
    QByteArray buffer, readBuffer;
    while (true)
    {
        if (file.atEnd())
            break;
        readBuffer = file.read(WICHAT_SESSION_CACHE_READ_PROBELEN);
        buffer.append(readBuffer);
        p = readBuffer.indexOf(delimiter);
        if (p >= 0)
        {
            file.seek(file.pos() - readBuffer.length()
                      + p + delimiter.length());
            buffer.chop(readBuffer.length() - p);
            break;
        }
    }
    return buffer;
}
