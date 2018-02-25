#ifndef ENCRYPTORPRIVATE_H
#define ENCRYPTORPRIVATE_H

#include <QObject>
#include <QCryptographicHash>

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

    unsigned int crc32_string(const char * text,long length);
    bool BlowFish(char* bufferIn,
                                    long inLength,
                                    char* key,
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
    void Blowfish_BytesToBlock(unsigned char *buf, Blowfish_SBlock *b);
    void Blowfish_BlockToBytes(Blowfish_SBlock *b, unsigned char *buf);
    void Blowfish_EncryptBlock(Blowfish *blowfish, Blowfish_SBlock *block);
    void Blowfish_DecryptBlock(Blowfish *blowfish, Blowfish_SBlock *block);
    int Blowfish_Init(Blowfish *blowfish, unsigned char *ucKey, int keysize);

protected:
    Encryptor* q_ptr;
};

#endif // ENCRYPTORPRIVATE_H
