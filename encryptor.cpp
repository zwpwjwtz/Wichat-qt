#include "encryptor.h"
#include "encryptorprivate.h"

Encryptor::Encryptor()
{
    this->d_ptr = new EncryptorPrivate;
}

QByteArray Encryptor::getCRC32(QByteArray& source)
{
    Q_D(Encryptor);

    unsigned int ret = d->crc32_string(source.data(), source.size());
    return QByteArray()
            .append(ret & 0xFF)
            .append((ret >> 2) & 0xFF)
            .append((ret >> 4) & 0xFF)
            .append((ret >> 6) & 0xFF);
}

QString Encryptor::getSHA1(QByteArray& source, bool toDec)
{
    Q_D(Encryptor);

    QString result;
    d->result = d->hasher.hash(source, QCryptographicHash::Sha1);
    if (toDec)
    {
        for (int i=0; i<d->result.size(); i++)
            result.append(char(int(d->result[i]) + 48));
    }
    else
        result = d->result;
    return result;
}

bool Encryptor::encrypt(Algorithm algo,
                        const QByteArray& source,
                        const QByteArray& key,
                        QByteArray& result)
{
    Q_D(Encryptor);

    bool ret = false;
    qint64 sourceLen = source.length();
    char* resultByte = nullptr;

    switch (algo)
    {
        case none:
        case CSC:
            break;
        case Blowfish:
            resultByte = new char[sourceLen];
            ret = d->BlowFish(source.constData(), sourceLen,
                              key.constData(),
                              resultByte, sourceLen,
                              1);
            if (ret)
                result.fromRawData(resultByte, sourceLen);
            break;
        case AES:
            break;
        default:;
    }

    if (resultByte != nullptr)
        delete resultByte;
    return ret;
}
bool Encryptor::decrypt(Algorithm algo,
                        const QByteArray& source,
                        const QByteArray& key,
                        QByteArray& result)
{
    Q_D(Encryptor);

    bool ret = false;
    qint64 sourceLen = source.length();
    char* resultByte = nullptr;

    switch (algo)
    {
        case none:
        case CSC:
            break;
        case Blowfish:
            resultByte = new char[sourceLen];
            ret = d->BlowFish(source.constData(), sourceLen,
                              key.constData(),
                              resultByte, sourceLen,
                              2);
            if (ret)
                result.fromRawData(resultByte, sourceLen);
            break;
        case AES:
            break;
        default:;
    }

    if (resultByte != nullptr)
        delete resultByte;
    return ret;
}

//unsigned int __crc32_table[255];
/*ulPolynomial = 0x04c11db7*/
unsigned int  __crc32_table[256] =
{
   0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
   0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
   0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
   0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
   0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
   0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
   0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
   0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
   0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
   0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
   0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
   0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
   0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
   0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
   0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
   0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
   0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
   0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
   0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
   0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
   0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
   0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
   0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
   0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
   0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
   0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
   0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
   0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
   0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
   0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
   0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
   0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
   0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
   0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
   0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
   0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
   0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
   0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
   0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
   0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
   0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
   0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
   0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
   0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
   0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
   0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
   0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
   0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
   0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
   0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
   0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
   0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
   0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
   0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
   0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
   0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
   0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
   0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
   0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
   0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
   0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
   0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
   0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
   0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

/*Initialization with a fixed string which consists of the hexadecimal digits of PI (less the initial 3)
  P-array, 18 32-bit subkeys*/
const unsigned int scm_auiInitP[BLOWFISH_MAX_PBLOCK_SIZE] = {
    0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344,
    0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89,
    0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
    0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917,
    0x9216d5d9, 0x8979fb1b
};
/*Four 32-bit S-boxes with 256 entries each*/
const unsigned int scm_auiInitS[BLOWFISH_MAX_SBLOCK_XSIZE][BLOWFISH_MAX_SBLOCK_YSIZE] = {
    /*0*/
    {0xd1310ba6, 0x98dfb5ac, 0x2ffd72db, 0xd01adfb7,
     0xb8e1afed, 0x6a267e96, 0xba7c9045, 0xf12c7f99,
     0x24a19947, 0xb3916cf7, 0x0801f2e2, 0x858efc16,
     0x636920d8, 0x71574e69, 0xa458fea3, 0xf4933d7e,
     0x0d95748f, 0x728eb658, 0x718bcd58, 0x82154aee,
     0x7b54a41d, 0xc25a59b5, 0x9c30d539, 0x2af26013,
     0xc5d1b023, 0x286085f0, 0xca417918, 0xb8db38ef,
     0x8e79dcb0, 0x603a180e, 0x6c9e0e8b, 0xb01e8a3e,
     0xd71577c1, 0xbd314b27, 0x78af2fda, 0x55605c60,
     0xe65525f3, 0xaa55ab94, 0x57489862, 0x63e81440,
     0x55ca396a, 0x2aab10b6, 0xb4cc5c34, 0x1141e8ce,
     0xa15486af, 0x7c72e993, 0xb3ee1411, 0x636fbc2a,
     0x2ba9c55d, 0x741831f6, 0xce5c3e16, 0x9b87931e,
     0xafd6ba33, 0x6c24cf5c, 0x7a325381, 0x28958677,
     0x3b8f4898, 0x6b4bb9af, 0xc4bfe81b, 0x66282193,
     0x61d809cc, 0xfb21a991, 0x487cac60, 0x5dec8032,
     0xef845d5d, 0xe98575b1, 0xdc262302, 0xeb651b88,
     0x23893e81, 0xd396acc5, 0x0f6d6ff3, 0x83f44239,
     0x2e0b4482, 0xa4842004, 0x69c8f04a, 0x9e1f9b5e,
     0x21c66842, 0xf6e96c9a, 0x670c9c61, 0xabd388f0,
     0x6a51a0d2, 0xd8542f68, 0x960fa728, 0xab5133a3,
     0x6eef0b6c, 0x137a3be4, 0xba3bf050, 0x7efb2a98,
     0xa1f1651d, 0x39af0176, 0x66ca593e, 0x82430e88,
     0x8cee8619, 0x456f9fb4, 0x7d84a5c3, 0x3b8b5ebe,
     0xe06f75d8, 0x85c12073, 0x401a449f, 0x56c16aa6,
     0x4ed3aa62, 0x363f7706, 0x1bfedf72, 0x429b023d,
     0x37d0d724, 0xd00a1248, 0xdb0fead3, 0x49f1c09b,
     0x075372c9, 0x80991b7b, 0x25d479d8, 0xf6e8def7,
     0xe3fe501a, 0xb6794c3b, 0x976ce0bd, 0x04c006ba,
     0xc1a94fb6, 0x409f60c4, 0x5e5c9ec2, 0x196a2463,
     0x68fb6faf, 0x3e6c53b5, 0x1339b2eb, 0x3b52ec6f,
     0x6dfc511f, 0x9b30952c, 0xcc814544, 0xaf5ebd09,
     0xbee3d004, 0xde334afd, 0x660f2807, 0x192e4bb3,
     0xc0cba857, 0x45c8740f, 0xd20b5f39, 0xb9d3fbdb,
     0x5579c0bd, 0x1a60320a, 0xd6a100c6, 0x402c7279,
     0x679f25fe, 0xfb1fa3cc, 0x8ea5e9f8, 0xdb3222f8,
     0x3c7516df, 0xfd616b15, 0x2f501ec8, 0xad0552ab,
     0x323db5fa, 0xfd238760, 0x53317b48, 0x3e00df82,
     0x9e5c57bb, 0xca6f8ca0, 0x1a87562e, 0xdf1769db,
     0xd542a8f6, 0x287effc3, 0xac6732c6, 0x8c4f5573,
     0x695b27b0, 0xbbca58c8, 0xe1ffa35d, 0xb8f011a0,
     0x10fa3d98, 0xfd2183b8, 0x4afcb56c, 0x2dd1d35b,
     0x9a53e479, 0xb6f84565, 0xd28e49bc, 0x4bfb9790,
     0xe1ddf2da, 0xa4cb7e33, 0x62fb1341, 0xcee4c6e8,
     0xef20cada, 0x36774c01, 0xd07e9efe, 0x2bf11fb4,
     0x95dbda4d, 0xae909198, 0xeaad8e71, 0x6b93d5a0,
     0xd08ed1d0, 0xafc725e0, 0x8e3c5b2f, 0x8e7594b7,
     0x8ff6e2fb, 0xf2122b64, 0x8888b812, 0x900df01c,
     0x4fad5ea0, 0x688fc31c, 0xd1cff191, 0xb3a8c1ad,
     0x2f2f2218, 0xbe0e1777, 0xea752dfe, 0x8b021fa1,
     0xe5a0cc0f, 0xb56f74e8, 0x18acf3d6, 0xce89e299,
     0xb4a84fe0, 0xfd13e0b7, 0x7cc43b81, 0xd2ada8d9,
     0x165fa266, 0x80957705, 0x93cc7314, 0x211a1477,
     0xe6ad2065, 0x77b5fa86, 0xc75442f5, 0xfb9d35cf,
     0xebcdaf0c, 0x7b3e89a0, 0xd6411bd3, 0xae1e7e49,
     0x00250e2d, 0x2071b35e, 0x226800bb, 0x57b8e0af,
     0x2464369b, 0xf009b91e, 0x5563911d, 0x59dfa6aa,
     0x78c14389, 0xd95a537f, 0x207d5ba2, 0x02e5b9c5,
     0x83260376, 0x6295cfa9, 0x11c81968, 0x4e734a41,
     0xb3472dca, 0x7b14a94a, 0x1b510052, 0x9a532915,
     0xd60f573f, 0xbc9bc6e4, 0x2b60a476, 0x81e67400,
     0x08ba6fb5, 0x571be91f, 0xf296ec6b, 0x2a0dd915,
     0xb6636521, 0xe7b9f9b6, 0xff34052e, 0xc5855664,
     0x53b02d5d, 0xa99f8fa1, 0x08ba4799, 0x6e85076a},
    /*1*/
    {0x4b7a70e9, 0xb5b32944, 0xdb75092e, 0xc4192623,
     0xad6ea6b0, 0x49a7df7d, 0x9cee60b8, 0x8fedb266,
     0xecaa8c71, 0x699a17ff, 0x5664526c, 0xc2b19ee1,
     0x193602a5, 0x75094c29, 0xa0591340, 0xe4183a3e,
     0x3f54989a, 0x5b429d65, 0x6b8fe4d6, 0x99f73fd6,
     0xa1d29c07, 0xefe830f5, 0x4d2d38e6, 0xf0255dc1,
     0x4cdd2086, 0x8470eb26, 0x6382e9c6, 0x021ecc5e,
     0x09686b3f, 0x3ebaefc9, 0x3c971814, 0x6b6a70a1,
     0x687f3584, 0x52a0e286, 0xb79c5305, 0xaa500737,
     0x3e07841c, 0x7fdeae5c, 0x8e7d44ec, 0x5716f2b8,
     0xb03ada37, 0xf0500c0d, 0xf01c1f04, 0x0200b3ff,
     0xae0cf51a, 0x3cb574b2, 0x25837a58, 0xdc0921bd,
     0xd19113f9, 0x7ca92ff6, 0x94324773, 0x22f54701,
     0x3ae5e581, 0x37c2dadc, 0xc8b57634, 0x9af3dda7,
     0xa9446146, 0x0fd0030e, 0xecc8c73e, 0xa4751e41,
     0xe238cd99, 0x3bea0e2f, 0x3280bba1, 0x183eb331,
     0x4e548b38, 0x4f6db908, 0x6f420d03, 0xf60a04bf,
     0x2cb81290, 0x24977c79, 0x5679b072, 0xbcaf89af,
     0xde9a771f, 0xd9930810, 0xb38bae12, 0xdccf3f2e,
     0x5512721f, 0x2e6b7124, 0x501adde6, 0x9f84cd87,
     0x7a584718, 0x7408da17, 0xbc9f9abc, 0xe94b7d8c,
     0xec7aec3a, 0xdb851dfa, 0x63094366, 0xc464c3d2,
     0xef1c1847, 0x3215d908, 0xdd433b37, 0x24c2ba16,
     0x12a14d43, 0x2a65c451, 0x50940002, 0x133ae4dd,
     0x71dff89e, 0x10314e55, 0x81ac77d6, 0x5f11199b,
     0x043556f1, 0xd7a3c76b, 0x3c11183b, 0x5924a509,
     0xf28fe6ed, 0x97f1fbfa, 0x9ebabf2c, 0x1e153c6e,
     0x86e34570, 0xeae96fb1, 0x860e5e0a, 0x5a3e2ab3,
     0x771fe71c, 0x4e3d06fa, 0x2965dcb9, 0x99e71d0f,
     0x803e89d6, 0x5266c825, 0x2e4cc978, 0x9c10b36a,
     0xc6150eba, 0x94e2ea78, 0xa5fc3c53, 0x1e0a2df4,
     0xf2f74ea7, 0x361d2b3d, 0x1939260f, 0x19c27960,
     0x5223a708, 0xf71312b6, 0xebadfe6e, 0xeac31f66,
     0xe3bc4595, 0xa67bc883, 0xb17f37d1, 0x018cff28,
     0xc332ddef, 0xbe6c5aa5, 0x65582185, 0x68ab9802,
     0xeecea50f, 0xdb2f953b, 0x2aef7dad, 0x5b6e2f84,
     0x1521b628, 0x29076170, 0xecdd4775, 0x619f1510,
     0x13cca830, 0xeb61bd96, 0x0334fe1e, 0xaa0363cf,
     0xb5735c90, 0x4c70a239, 0xd59e9e0b, 0xcbaade14,
     0xeecc86bc, 0x60622ca7, 0x9cab5cab, 0xb2f3846e,
     0x648b1eaf, 0x19bdf0ca, 0xa02369b9, 0x655abb50,
     0x40685a32, 0x3c2ab4b3, 0x319ee9d5, 0xc021b8f7,
     0x9b540b19, 0x875fa099, 0x95f7997e, 0x623d7da8,
     0xf837889a, 0x97e32d77, 0x11ed935f, 0x16681281,
     0x0e358829, 0xc7e61fd6, 0x96dedfa1, 0x7858ba99,
     0x57f584a5, 0x1b227263, 0x9b83c3ff, 0x1ac24696,
     0xcdb30aeb, 0x532e3054, 0x8fd948e4, 0x6dbc3128,
     0x58ebf2ef, 0x34c6ffea, 0xfe28ed61, 0xee7c3c73,
     0x5d4a14d9, 0xe864b7e3, 0x42105d14, 0x203e13e0,
     0x45eee2b6, 0xa3aaabea, 0xdb6c4f15, 0xfacb4fd0,
     0xc742f442, 0xef6abbb5, 0x654f3b1d, 0x41cd2105,
     0xd81e799e, 0x86854dc7, 0xe44b476a, 0x3d816250,
     0xcf62a1f2, 0x5b8d2646, 0xfc8883a0, 0xc1c7b6a3,
     0x7f1524c3, 0x69cb7492, 0x47848a0b, 0x5692b285,
     0x095bbf00, 0xad19489d, 0x1462b174, 0x23820e00,
     0x58428d2a, 0x0c55f5ea, 0x1dadf43e, 0x233f7061,
     0x3372f092, 0x8d937e41, 0xd65fecf1, 0x6c223bdb,
     0x7cde3759, 0xcbee7460, 0x4085f2a7, 0xce77326e,
     0xa6078084, 0x19f8509e, 0xe8efd855, 0x61d99735,
     0xa969a7aa, 0xc50c06c2, 0x5a04abfc, 0x800bcadc,
     0x9e447a2e, 0xc3453484, 0xfdd56705, 0x0e1e9ec9,
     0xdb73dbd3, 0x105588cd, 0x675fda79, 0xe3674340,
     0xc5c43465, 0x713e38d8, 0x3d28f89e, 0xf16dff20,
     0x153e21e7, 0x8fb03d4a, 0xe6e39f2b, 0xdb83adf7},
    /*2*/
    {0xe93d5a68, 0x948140f7, 0xf64c261c, 0x94692934,
     0x411520f7, 0x7602d4f7, 0xbcf46b2e, 0xd4a20068,
     0xd4082471, 0x3320f46a, 0x43b7d4b7, 0x500061af,
     0x1e39f62e, 0x97244546, 0x14214f74, 0xbf8b8840,
     0x4d95fc1d, 0x96b591af, 0x70f4ddd3, 0x66a02f45,
     0xbfbc09ec, 0x03bd9785, 0x7fac6dd0, 0x31cb8504,
     0x96eb27b3, 0x55fd3941, 0xda2547e6, 0xabca0a9a,
     0x28507825, 0x530429f4, 0x0a2c86da, 0xe9b66dfb,
     0x68dc1462, 0xd7486900, 0x680ec0a4, 0x27a18dee,
     0x4f3ffea2, 0xe887ad8c, 0xb58ce006, 0x7af4d6b6,
     0xaace1e7c, 0xd3375fec, 0xce78a399, 0x406b2a42,
     0x20fe9e35, 0xd9f385b9, 0xee39d7ab, 0x3b124e8b,
     0x1dc9faf7, 0x4b6d1856, 0x26a36631, 0xeae397b2,
     0x3a6efa74, 0xdd5b4332, 0x6841e7f7, 0xca7820fb,
     0xfb0af54e, 0xd8feb397, 0x454056ac, 0xba489527,
     0x55533a3a, 0x20838d87, 0xfe6ba9b7, 0xd096954b,
     0x55a867bc, 0xa1159a58, 0xcca92963, 0x99e1db33,
     0xa62a4a56, 0x3f3125f9, 0x5ef47e1c, 0x9029317c,
     0xfdf8e802, 0x04272f70, 0x80bb155c, 0x05282ce3,
     0x95c11548, 0xe4c66d22, 0x48c1133f, 0xc70f86dc,
     0x07f9c9ee, 0x41041f0f, 0x404779a4, 0x5d886e17,
     0x325f51eb, 0xd59bc0d1, 0xf2bcc18f, 0x41113564,
     0x257b7834, 0x602a9c60, 0xdff8e8a3, 0x1f636c1b,
     0x0e12b4c2, 0x02e1329e, 0xaf664fd1, 0xcad18115,
     0x6b2395e0, 0x333e92e1, 0x3b240b62, 0xeebeb922,
     0x85b2a20e, 0xe6ba0d99, 0xde720c8c, 0x2da2f728,
     0xd0127845, 0x95b794fd, 0x647d0862, 0xe7ccf5f0,
     0x5449a36f, 0x877d48fa, 0xc39dfd27, 0xf33e8d1e,
     0x0a476341, 0x992eff74, 0x3a6f6eab, 0xf4f8fd37,
     0xa812dc60, 0xa1ebddf8, 0x991be14c, 0xdb6e6b0d,
     0xc67b5510, 0x6d672c37, 0x2765d43b, 0xdcd0e804,
     0xf1290dc7, 0xcc00ffa3, 0xb5390f92, 0x690fed0b,
     0x667b9ffb, 0xcedb7d9c, 0xa091cf0b, 0xd9155ea3,
     0xbb132f88, 0x515bad24, 0x7b9479bf, 0x763bd6eb,
     0x37392eb3, 0xcc115979, 0x8026e297, 0xf42e312d,
     0x6842ada7, 0xc66a2b3b, 0x12754ccc, 0x782ef11c,
     0x6a124237, 0xb79251e7, 0x06a1bbe6, 0x4bfb6350,
     0x1a6b1018, 0x11caedfa, 0x3d25bdd8, 0xe2e1c3c9,
     0x44421659, 0x0a121386, 0xd90cec6e, 0xd5abea2a,
     0x64af674e, 0xda86a85f, 0xbebfe988, 0x64e4c3fe,
     0x9dbc8057, 0xf0f7c086, 0x60787bf8, 0x6003604d,
     0xd1fd8346, 0xf6381fb0, 0x7745ae04, 0xd736fccc,
     0x83426b33, 0xf01eab71, 0xb0804187, 0x3c005e5f,
     0x77a057be, 0xbde8ae24, 0x55464299, 0xbf582e61,
     0x4e58f48f, 0xf2ddfda2, 0xf474ef38, 0x8789bdc2,
     0x5366f9c3, 0xc8b38e74, 0xb475f255, 0x46fcd9b9,
     0x7aeb2661, 0x8b1ddf84, 0x846a0e79, 0x915f95e2,
     0x466e598e, 0x20b45770, 0x8cd55591, 0xc902de4c,
     0xb90bace1, 0xbb8205d0, 0x11a86248, 0x7574a99e,
     0xb77f19b6, 0xe0a9dc09, 0x662d09a1, 0xc4324633,
     0xe85a1f02, 0x09f0be8c, 0x4a99a025, 0x1d6efe10,
     0x1ab93d1d, 0x0ba5a4df, 0xa186f20f, 0x2868f169,
     0xdcb7da83, 0x573906fe, 0xa1e2ce9b, 0x4fcd7f52,
     0x50115e01, 0xa70683fa, 0xa002b5c4, 0x0de6d027,
     0x9af88c27, 0x773f8641, 0xc3604c06, 0x61a806b5,
     0xf0177a28, 0xc0f586e0, 0x006058aa, 0x30dc7d62,
     0x11e69ed7, 0x2338ea63, 0x53c2dd94, 0xc2c21634,
     0xbbcbee56, 0x90bcb6de, 0xebfc7da1, 0xce591d76,
     0x6f05e409, 0x4b7c0188, 0x39720a3d, 0x7c927c24,
     0x86e3725f, 0x724d9db9, 0x1ac15bb4, 0xd39eb8fc,
     0xed545578, 0x08fca5b5, 0xd83d7cd3, 0x4dad0fc4,
     0x1e50ef5e, 0xb161e6f8, 0xa28514d9, 0x6c51133c,
     0x6fd5c7e7, 0x56e14ec4, 0x362abfce, 0xddc6c837,
     0xd79a3234, 0x92638212, 0x670efa8e, 0x406000e0},
    /*3*/
    {0x3a39ce37, 0xd3faf5cf, 0xabc27737, 0x5ac52d1b,
     0x5cb0679e, 0x4fa33742, 0xd3822740, 0x99bc9bbe,
     0xd5118e9d, 0xbf0f7315, 0xd62d1c7e, 0xc700c47b,
     0xb78c1b6b, 0x21a19045, 0xb26eb1be, 0x6a366eb4,
     0x5748ab2f, 0xbc946e79, 0xc6a376d2, 0x6549c2c8,
     0x530ff8ee, 0x468dde7d, 0xd5730a1d, 0x4cd04dc6,
     0x2939bbdb, 0xa9ba4650, 0xac9526e8, 0xbe5ee304,
     0xa1fad5f0, 0x6a2d519a, 0x63ef8ce2, 0x9a86ee22,
     0xc089c2b8, 0x43242ef6, 0xa51e03aa, 0x9cf2d0a4,
     0x83c061ba, 0x9be96a4d, 0x8fe51550, 0xba645bd6,
     0x2826a2f9, 0xa73a3ae1, 0x4ba99586, 0xef5562e9,
     0xc72fefd3, 0xf752f7da, 0x3f046f69, 0x77fa0a59,
     0x80e4a915, 0x87b08601, 0x9b09e6ad, 0x3b3ee593,
     0xe990fd5a, 0x9e34d797, 0x2cf0b7d9, 0x022b8b51,
     0x96d5ac3a, 0x017da67d, 0xd1cf3ed6, 0x7c7d2d28,
     0x1f9f25cf, 0xadf2b89b, 0x5ad6b472, 0x5a88f54c,
     0xe029ac71, 0xe019a5e6, 0x47b0acfd, 0xed93fa9b,
     0xe8d3c48d, 0x283b57cc, 0xf8d56629, 0x79132e28,
     0x785f0191, 0xed756055, 0xf7960e44, 0xe3d35e8c,
     0x15056dd4, 0x88f46dba, 0x03a16125, 0x0564f0bd,
     0xc3eb9e15, 0x3c9057a2, 0x97271aec, 0xa93a072a,
     0x1b3f6d9b, 0x1e6321f5, 0xf59c66fb, 0x26dcf319,
     0x7533d928, 0xb155fdf5, 0x03563482, 0x8aba3cbb,
     0x28517711, 0xc20ad9f8, 0xabcc5167, 0xccad925f,
     0x4de81751, 0x3830dc8e, 0x379d5862, 0x9320f991,
     0xea7a90c2, 0xfb3e7bce, 0x5121ce64, 0x774fbe32,
     0xa8b6e37e, 0xc3293d46, 0x48de5369, 0x6413e680,
     0xa2ae0810, 0xdd6db224, 0x69852dfd, 0x09072166,
     0xb39a460a, 0x6445c0dd, 0x586cdecf, 0x1c20c8ae,
     0x5bbef7dd, 0x1b588d40, 0xccd2017f, 0x6bb4e3bb,
     0xdda26a7e, 0x3a59ff45, 0x3e350a44, 0xbcb4cdd5,
     0x72eacea8, 0xfa6484bb, 0x8d6612ae, 0xbf3c6f47,
     0xd29be463, 0x542f5d9e, 0xaec2771b, 0xf64e6370,
     0x740e0d8d, 0xe75b1357, 0xf8721671, 0xaf537d5d,
     0x4040cb08, 0x4eb4e2cc, 0x34d2466a, 0x0115af84,
     0xe1b00428, 0x95983a1d, 0x06b89fb4, 0xce6ea048,
     0x6f3f3b82, 0x3520ab82, 0x011a1d4b, 0x277227f8,
     0x611560b1, 0xe7933fdc, 0xbb3a792b, 0x344525bd,
     0xa08839e1, 0x51ce794b, 0x2f32c9b7, 0xa01fbac9,
     0xe01cc87e, 0xbcc7d1f6, 0xcf0111c3, 0xa1e8aac7,
     0x1a908749, 0xd44fbd9a, 0xd0dadecb, 0xd50ada38,
     0x0339c32a, 0xc6913667, 0x8df9317c, 0xe0b12b4f,
     0xf79e59b7, 0x43f5bb3a, 0xf2d519ff, 0x27d9459c,
     0xbf97222c, 0x15e6fc2a, 0x0f91fc71, 0x9b941525,
     0xfae59361, 0xceb69ceb, 0xc2a86459, 0x12baa8d1,
     0xb6c1075e, 0xe3056a0c, 0x10d25065, 0xcb03a442,
     0xe0ec6e0e, 0x1698db3b, 0x4c98a0be, 0x3278e964,
     0x9f1f9532, 0xe0d392df, 0xd3a0342b, 0x8971f21e,
     0x1b0a7441, 0x4ba3348c, 0xc5be7120, 0xc37632d8,
     0xdf359f8d, 0x9b992f2e, 0xe60b6f47, 0x0fe3f11d,
     0xe54cda54, 0x1edad891, 0xce6279cf, 0xcd3e7e6f,
     0x1618b166, 0xfd2c1d05, 0x848fd2c5, 0xf6fb2299,
     0xf523f357, 0xa6327623, 0x93a83531, 0x56cccd02,
     0xacf08162, 0x5a75ebb5, 0x6e163697, 0x88d273cc,
     0xde966292, 0x81b949d0, 0x4c50901b, 0x71c65614,
     0xe6c6c7bd, 0x327a140a, 0x45e1d006, 0xc3f27b9a,
     0xc9aa53fd, 0x62a80f00, 0xbb25bfe2, 0x35bdd2f6,
     0x71126905, 0xb2040222, 0xb6cbcf7c, 0xcd769c2b,
     0x53113ec0, 0x1640e3d3, 0x38abbd60, 0x2547adf0,
     0xba38209c, 0xf746ce76, 0x77afa1c5, 0x20756060,
     0x85cbfe4e, 0x8ae88dd8, 0x7aaaf9b0, 0x4cf9aa7e,
     0x1948c25c, 0x02fb8a8c, 0x01c36ae4, 0xd6ebe1f9,
     0x90d4f869, 0xa65cdea0, 0x3f09252d, 0xc208e69f,
     0xb74e6132, 0xce77e25b, 0x578fdfe3, 0x3ac372e6}
};

unsigned int EncryptorPrivate::crc32_string(const char * text, long length)
{
        // Start out with all bits set high.
        unsigned int crc=0xffffffff;
        long i;

        // Perform the algorithm on each character in the string,
        // using the lookup table values.
        for(i=0;i < length;++i) {
            crc=((crc >> 8) & 0x00ffffff) ^ __crc32_table[(unsigned char)((crc & 0xFF) ^ text[i])];
        }

        // Exclusive OR the result with the beginning value.
        return crc ^ 0xffffffff;
}

/*Extract low order byte*/
unsigned char EncryptorPrivate::Blowfish_Byte(unsigned int ui)
{
    return (unsigned char)(ui & 0xff);
}
/*Function F*/
unsigned int EncryptorPrivate::Blowfish_F(unsigned int auiS[][BLOWFISH_MAX_SBLOCK_YSIZE], unsigned int ui)
{
    return ((auiS[0][Blowfish_Byte(ui>>24)] + auiS[1][Blowfish_Byte(ui>>16)]) ^ auiS[2][Blowfish_Byte(ui>>8)]) + auiS[3][Blowfish_Byte(ui)];
}
/*Semi-Portable Byte Shuffling*/
void EncryptorPrivate::Blowfish_BytesToBlock(unsigned const char *buf,
                                             Blowfish_SBlock *b)
{
    unsigned int y;
    unsigned const char *p = buf;
    /*Left*/
    b->m_uil = 0;
    y = *p++;
    y <<= 24;
    b->m_uil |= y;
    y = *p++;
    y <<= 16;
    b->m_uil |= y;
    y = *p++;
    y <<= 8;
    b->m_uil |= y;
    y = *p++;
    b->m_uil |= y;
    /*Right*/
    b->m_uir = 0;
    y = *p++;
    y <<= 24;
    b->m_uir |= y;
    y = *p++;
    y <<= 16;
    b->m_uir |= y;
    y = *p++;
    y <<= 8;
    b->m_uir |= y;
    y = *p++;
    b->m_uir |= y;
}
void EncryptorPrivate::Blowfish_BlockToBytes(Blowfish_SBlock *b, unsigned char *buf)
{
    unsigned int y;
    unsigned char *p = buf+8;
    /*Right*/
    y = b->m_uir;
    *--p = Blowfish_Byte(y);
    y = b->m_uir >> 8;
    *--p = Blowfish_Byte(y);
    y = b->m_uir >> 16;
    *--p = Blowfish_Byte(y);
    y = b->m_uir >> 24;
    *--p = Blowfish_Byte(y);
    /*Left*/
    y = b->m_uil;
    *--p = Blowfish_Byte(y);
    y = b->m_uil >> 8;
    *--p = Blowfish_Byte(y);
    y = b->m_uil >> 16;
    *--p = Blowfish_Byte(y);
    y = b->m_uil >> 24;
    *--p = Blowfish_Byte(y);
}
/****************************************************************************************/
/*Sixteen Round Encipher of Block*/
void EncryptorPrivate::Blowfish_EncryptBlock(Blowfish *blowfish, Blowfish_SBlock *block)
{
    unsigned int uiLeft = block->m_uil;
    unsigned int uiRight = block->m_uir;
    uiLeft ^= blowfish->m_auiP[0];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[1];  uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[2];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[3];  uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[4];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[5];  uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[6];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[7];  uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[8];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[9];  uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[10];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[11]; uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[12];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[13]; uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[14];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[15]; uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[16];
    uiRight ^= blowfish->m_auiP[17];
    block->m_uil = uiRight;
    block->m_uir = uiLeft;
}
/*Sixteen Round Decipher of SBlock*/
void EncryptorPrivate::Blowfish_DecryptBlock(Blowfish *blowfish, Blowfish_SBlock *block)
{
    unsigned int uiLeft = block->m_uil;
    unsigned int uiRight = block->m_uir;
    uiLeft ^= blowfish->m_auiP[17];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[16]; uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[15];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[14]; uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[13];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[12]; uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[11];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[10]; uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[9];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[8];  uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[7];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[6];  uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[5];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[4];  uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[3];
    uiRight ^= Blowfish_F(blowfish->m_auiS, uiLeft)^blowfish->m_auiP[2];  uiLeft ^= Blowfish_F(blowfish->m_auiS, uiRight)^blowfish->m_auiP[1];
    uiRight ^= blowfish->m_auiP[0];
    block->m_uil = uiRight;
    block->m_uir = uiLeft;
}
int EncryptorPrivate::Blowfish_Init(Blowfish *blowfish, unsigned const char *ucKey, int keysize)
{
    unsigned int i, j, k, n;
    unsigned char aucLocalKey[BLOWFISH_MAX_KEY_SIZE];
    const unsigned char *p = aucLocalKey;
    unsigned int x = 0;
    int iCount = 0;
    Blowfish_SBlock block = {0, 0}; //all-zero block
    if (keysize<1) return -1;
    /*Check the Key - the key length should be between 1 and 56 bytes*/
    if(keysize<0 || keysize>BLOWFISH_MAX_KEY_SIZE) return -1;
    memcpy(aucLocalKey, ucKey, keysize);
    /*Reflexive Initialization of the Blowfish.
      Generating the Subkeys from the Key flood P and S boxes with PI*/
    memcpy(blowfish->m_auiP, scm_auiInitP, sizeof(blowfish->m_auiP));
    memcpy(blowfish->m_auiS, scm_auiInitS, sizeof(blowfish->m_auiS));
    /*Load P boxes with key bytes
      Repeatedly cycle through the key bits until the entire P array has been XORed with key bits*/
    for(i=0; i<BLOWFISH_MAX_PBLOCK_SIZE; i++)
    {
        x=0;
        for(n=4; n--; )
        {
            /*int iVal = (int)(*p);*/
            x <<= 8;
            x |= *(p++);
            iCount++;
            if(iCount == keysize)
            {
                /*All bytes used, so recycle bytes*/
                iCount = 0;
                p = aucLocalKey;
            }
        }
        blowfish->m_auiP[i] ^= x;
    }
    /*Reflect P and S boxes through the evolving Blowfish*/
    for(i=0; i<BLOWFISH_MAX_PBLOCK_SIZE; )
        Blowfish_EncryptBlock(blowfish, &block), blowfish->m_auiP[i++] = block.m_uil, blowfish->m_auiP[i++] = block.m_uir;
    for(j=0; j<BLOWFISH_MAX_SBLOCK_XSIZE; j++)
        for(k=0; k<BLOWFISH_MAX_SBLOCK_YSIZE; )
            Blowfish_EncryptBlock(blowfish, &block), blowfish->m_auiS[j][k++] = block.m_uil, blowfish->m_auiS[j][k++] = block.m_uir;
    return 1;
}

bool EncryptorPrivate::BlowFish(const char* bufferIn,
                                long inLength,
                                const char* key,
                                char* bufferOut,
                                long outLength,
                                int mode)
{
    //Mode:0=none;1=Encrypt;2=Decrpyt.
    if (bufferIn==0 || bufferOut==0 || key==0) return false;
    if (inLength % 8>0 || outLength<inLength) return false;
    if (mode<1 || mode>2) return true;
    Blowfish data;
    if (Blowfish_Init(&data,(unsigned const char*)key,16)!=1) return false;
    long i=0;
    Blowfish_SBlock work;
    do
    {
        Blowfish_BytesToBlock((unsigned const char*)bufferIn+i, &work);
        if (mode==1)
            Blowfish_EncryptBlock(&data, &work);
        else
            Blowfish_DecryptBlock(&data, &work);
        Blowfish_BlockToBytes(&work, (unsigned char*)bufferOut+i);
        i+=8;
    }while(i<inLength);

    return true;
}
