#include <QtTest/QtTest>
#include <QCryptographicHash>
#include "../encryptor.h"

class TestEncryptor : public QObject
{
    Q_OBJECT

public:
    Encryptor encoder;

    QByteArray input;
    QByteArray output;

    const char* testVector1 = "\x6c\x1f\x8d\x84\x4a\xbe\x5a\xf3";
    const char* testVector2 = "\xeb\x5e\x0b\x35\x23\xac\xa8\x02"
                              "\xfe\x40\x33\x1f\x2f\x71\x94\x6d";
    const char* testVector3 = "\xad\xb9\x2e\xa3\x33\x42\xd2\x79";
    const char* testVector4 = "\xc1\xa6\xa3\x27\x79\xfc\x88\x8a";

private slots:
    void getCRC32();
    void getSHA1();
    void getSHA256();
    void getHMAC();

    void encrypt();
    void decrypt();

    void fuse();
    void fuse_R();
    void byteXOR();
};

void TestEncryptor::getCRC32()
{
    QCOMPARE(encoder.getCRC32(testVector1),
             QByteArray("\xd5\xea\x31\xca"));
}

void TestEncryptor::getSHA1()
{
    QCOMPARE(encoder.getSHA1(testVector1),
             QByteArray::fromRawData(
                        "\x88\x66\x46\x9f\x68\x08\xda\xa8"
                        "\xf9\xea\x1f\x8f\x21\x00\xb9\xcd"
                        "\x94\x12\x4c\x87", 20));
}

void TestEncryptor::getSHA256()
{
    QCOMPARE(QCryptographicHash::hash(testVector1, QCryptographicHash::Sha256),
             QByteArray("\x9a\x48\x77\xb7\x1e\x6a\x41\xf2"
                        "\x45\x18\xc8\xe2\x15\x21\x32\x4c"
                        "\xd0\xe2\x29\x35\x0c\x49\x0e\x4a"
                        "\xc2\x6a\xaa\xe6\xd7\x11\x85\x99"));
}

void TestEncryptor::getHMAC()
{
    QCOMPARE(encoder.getHMAC(testVector1, testVector2),
             QByteArray("\xed\x0a\x13\xec\xa9\x55\x9d\x06"
                        "\xd2\x99\x7e\x58\xee\xa0\x47\xc1"
                        "\x8b\x71\x99\x73\xfc\x34\x24\x3e"
                        "\x08\x9d\x48\xb8\x0e\x1c\x1e\xb5"));
}

void TestEncryptor::encrypt()
{
    QByteArray result;
    encoder.encrypt(Encryptor::AES,
                    testVector1,
                    QByteArray(testVector2).append(testVector2),
                    result);
    output = result;
}

void TestEncryptor::decrypt()
{
    QByteArray result;
    encoder.decrypt(Encryptor::AES,
                    output,
                    QByteArray(testVector2).append(testVector2),
                    result);
    QCOMPARE(result,
             QByteArray(testVector1));
}

void TestEncryptor::fuse()
{
    QCOMPARE(encoder.fuse(testVector1, testVector2),
             QByteArray(testVector3));
}

void TestEncryptor::fuse_R()
{
    QCOMPARE(encoder.fuse_R(testVector3, testVector2),
             QByteArray(testVector1));
}

void TestEncryptor::byteXOR()
{
    QCOMPARE(encoder.byteXOR(testVector1, testVector3),
             QByteArray(testVector4));
}

QTEST_MAIN(TestEncryptor)
#include "test_encryptor.moc"
