#ifndef ENCRYPTORPRIVATE_H
#define ENCRYPTORPRIVATE_H

#include <QObject>
#include <QCryptographicHash>
#include "opensslpp/include/opensslpp/random.h"

#define BLOWFISH_MAX_PBLOCK_SIZE 18
#define BLOWFISH_MAX_SBLOCK_XSIZE 4
#define BLOWFISH_MAX_SBLOCK_YSIZE 256
#define BLOWFISH_MAX_KEY_SIZE 56


class Encryptor;

class EncryptorPrivate : QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Encryptor)

public:
    QCryptographicHash hasher{QCryptographicHash::Sha1};
    QByteArray result;
    static std::unique_ptr<opensslpp::Random> randGenerator;
    static constexpr const char* DefaultDelta =
                                "`-jvDj34hjG]vb 0-r 32-ug11`JWaepoj 1#@f12?#";

    unsigned int crc32_string(const char * text,long length);
    bool BlowFish(const char* bufferIn,
                  long inLength,
                  const char* key,
                  char* bufferOut,
                  long outLength,
                  int mode);

private:
    struct Blowfish_SBlock
    {
        unsigned int m_uil; /*Hi*/
        unsigned int m_uir; /*Lo*/
    };
    struct Blowfish
    {
        Blowfish_SBlock m_oChain;
        unsigned int m_auiP[BLOWFISH_MAX_PBLOCK_SIZE];
        unsigned int m_auiS[BLOWFISH_MAX_SBLOCK_XSIZE][BLOWFISH_MAX_SBLOCK_YSIZE];
    };

    unsigned char Blowfish_Byte(unsigned int ui);
    unsigned int Blowfish_F(unsigned int auiS[][BLOWFISH_MAX_SBLOCK_YSIZE], unsigned int ui);
    void Blowfish_BytesToBlock(const unsigned char *buf, Blowfish_SBlock *b);
    void Blowfish_BlockToBytes(Blowfish_SBlock *b, unsigned char *buf);
    void Blowfish_EncryptBlock(Blowfish *blowfish, Blowfish_SBlock *block);
    void Blowfish_DecryptBlock(Blowfish *blowfish, Blowfish_SBlock *block);
    int Blowfish_Init(Blowfish *blowfish, const unsigned char *ucKey, int keysize);

    int factorNumber(int number);
    QByteArray charVectorToQByteArray(std::vector<unsigned char> var);
    std::vector<unsigned char> qByteArrayToCharVector(const QByteArray& var);


protected:
    Encryptor* q_ptr;
};

#endif // ENCRYPTORPRIVATE_H
