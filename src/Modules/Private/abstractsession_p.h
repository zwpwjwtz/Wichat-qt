#ifndef AbstractSessionPRIVATE_H
#define AbstractSessionPRIVATE_H

#include <QObject>
#include <QFile>


class AbstractSession;
class AbstractSessionPrivate : public QObject
{
    Q_OBJECT
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
