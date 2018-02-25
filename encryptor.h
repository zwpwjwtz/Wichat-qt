#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <QObject>

class EncryptorPrivate;

class Encryptor : QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Encryptor)

public:
    enum Algorithm
    {
        none = 0,
        CSC = 1,
        Blowfish = 2,
        AES = 3
    };

    explicit Encryptor();

    QByteArray getCRC32(QByteArray& source);
    QString getSHA1(QByteArray &source, bool toDec = false);

    bool encrypt(Algorithm algo,
                 QByteArray& source,
                 QByteArray& key,
                 QByteArray& result);
    bool decrypt(Algorithm algo,
                 QByteArray& source,
                 QByteArray& key,
                 QByteArray& result);

protected:
    EncryptorPrivate* d_ptr;
};

#endif // ENCRYPTOR_H
