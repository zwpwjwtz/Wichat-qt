#ifndef AbstractSessionPRIVATE_H
#define AbstractSessionPRIVATE_H

#include <QtGlobal>


class QFile;

class AbstractSession;
class AbstractSessionPrivate
{
    Q_DECLARE_PUBLIC(AbstractSession)
protected:
    AbstractSession* q_ptr;

public:
    AbstractSessionPrivate();
    AbstractSessionPrivate(AbstractSession *parent);
    static QString getSessionFile(QString userDir, QString fileName);
    static QByteArray readUntil(QFile& file, const QByteArray delimiter);
};

#endif // AbstractSessionPRIVATE_H
