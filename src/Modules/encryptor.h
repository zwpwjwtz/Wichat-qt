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

    static QByteArray getCRC32(const QByteArray &source);
    static QByteArray getSHA256(const QByteArray& source, bool toDec = false);
    static QByteArray getHMAC(const QByteArray& source,
                              QByteArray key,
                              bool toDec = false);

    static bool encrypt(Algorithm algo,
                        const QByteArray& source,
                        const QByteArray& key,
                        QByteArray& result);
    static bool decrypt(Algorithm algo,
                        const QByteArray& source,
                        const QByteArray& key,
                        QByteArray& result);
    static QByteArray genKey(int length, bool toDec = false);

    static QByteArray fuse(const QByteArray& str, QByteArray delta, int base = 128);
    static QByteArray fuse_R(const QByteArray &str, QByteArray delta, int base = 128);
    static QByteArray byteXOR(const QByteArray& array1, const QByteArray& array2);

protected:
    EncryptorPrivate* d_ptr;
};

#endif // ENCRYPTOR_H
