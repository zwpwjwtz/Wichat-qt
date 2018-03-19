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

    static const int MinSeedLength = 8;
    static const int MinKeyLength = 16;
    static const int MaxKeyLength = 128;

    explicit Encryptor();

    QByteArray getCRC32(const QByteArray &source);
    QByteArray getSHA1(const QByteArray& source, bool toDec = false);
    QByteArray getHMAC(const QByteArray& source,
                       QByteArray key,
                       bool toDec = false);

    bool encrypt(Algorithm algo,
                 const QByteArray& source,
                 const QByteArray& key,
                 QByteArray& result);
    bool decrypt(Algorithm algo,
                 const QByteArray& source,
                 const QByteArray& key,
                 QByteArray& result);
    QByteArray genKey(QString seed, bool hex = false);
    QByteArray fuse(const QByteArray& str, QByteArray delta, int base = 128);
    QByteArray fuse_R(const QByteArray &str, QByteArray delta, int base = 128);
    QByteArray byteXOR(const QByteArray& array1, const QByteArray& array2);

protected:
    EncryptorPrivate* d_ptr;
};

#endif // ENCRYPTOR_H
