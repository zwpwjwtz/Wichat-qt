#include "common.h"
#include "account.h"
#include "Private/account_p.h"

#define WICHAT_SERVER_PATH_ACCOUNT_LOGIN "/Account/log/login.php"
#define WICHAT_SERVER_PATH_ACCOUNT_ACTION "/Account/acc/action.php"
#define WICHAT_SERVER_PATH_RECORD_ACTION "/Record/query/action.php"
#define WICHAT_SERVER_PATH_RECORD_GET "/Record/query/get.php"

#define WICHAT_ACCOUNT_STATE_DEFAULT 0
#define WICHAT_ACCOUNT_STATE_ONLINE 1
#define WICHAT_ACCOUNT_STATE_OFFLINE 2
//#define WICHAT_ACCOUNT_STATE_LOGOUT 3
#define WICHAT_ACCOUNT_STATE_BUSY 4
#define WICHAT_ACCOUNT_STATE_HIDE 5

#define WICHAT_RELATION_STATE_NONE 0
#define WICHAT_RELATION_STATE_WAITING 1
#define WICHAT_RELATION_STATE_ESTABLISHED 2
#define WICHAT_RELATION_STATE_BREAKING 3
//#define WICHAT_RELATION_STATE_REFUSED 4


Account::Account()
{
    this->d_ptr = new AccountPrivate;
}

bool Account::checkID(QString ID)
{
    int length = ID.length();
    if (length < 1 || length > MaxIDLen)
        return false;
    for (int i=0; i<length; i++)
    {
        char ch = ID[i].toLatin1();
        if (ch < '0' || ch > '9')
            return false;
    }
    return true;
}

bool Account::checkPassword(QString password)
{
    int length = password.length();
    if (length < 1 || length > MaxPasswordLen)
        return false;
    for (int i=0; i<length; i++)
    {
        char ch = password[i].toLatin1();
        if (!((ch >= '0' && ch <= '9') ||
              (ch >= 'A' && ch <= 'Z') ||
              (ch >= 'a' && ch <= 'z')))
            return false;
    }
    return true;
}

Account::VerifyError Account::verify(QString ID, QString password)
{
    Q_D(Account);
    if (!checkID(ID))
        return VerifyError::IDFormatError;
    if (!checkPassword(password))
        return VerifyError::PasswordFormatError;

    // Build pre-login request
    QByteArray bufferIn, bufferOut;
    QByteArray hashedPassword = d->encoder.getSHA256(password.toLatin1())
                                          .left(KeyLen);
    QByteArray tempKey = d->encoder.genKey("", true).left(KeyLen);
    QByteArray tempKey2;
    d->encoder.encrypt(Encryptor::AES,
                       tempKey,
                       hashedPassword,
                       tempKey2);
    bufferIn.append(char(WICHAT_CLIENT_DEVICE)).append(char(1));
    bufferIn.append(d->encoder.fuse(d->formatID(ID), tempKey2));
    bufferIn.append(tempKey2);

    // Send pre-login request
    if (!d->server.sendRequest(1, WICHAT_SERVER_PATH_ACCOUNT_LOGIN,
                               bufferIn, bufferOut))
        return VerifyError::NetworkError;
    if (bufferOut[0] == char(WICHAT_SERVER_RESPONSE_DEVICE_UNSUPPORTED))
        return VerifyError::VersionNotSupported;
    if (bufferOut[0] != char(WICHAT_SERVER_RESPONSE_SUCCESS))
        return VerifyError::UnknownError;

    // Extract pre-encryption key
    bufferOut.remove(0, 1);
    d->encoder.decrypt(Encryptor::AES,
                       bufferOut,
                       hashedPassword,
                       tempKey2);
    tempKey = d->encoder.byteXOR(tempKey, tempKey2);


    // Build login request
    bufferIn.clear();
    bufferIn.append(hashedPassword);
    bufferIn.append(char(OnlineState::Online));
    d->encoder.encrypt(Encryptor::AES, bufferIn, tempKey, bufferOut);
    bufferIn.clear();
    bufferIn.append(char(WICHAT_CLIENT_DEVICE)).append(char(2));
    bufferIn.append(d->encoder.getHMAC(d->formatID(ID), tempKey)
                              .left(MaxIDLen));
    bufferIn.append(bufferOut);

    // Send login request
    if (!d->server.sendRequest(1, WICHAT_SERVER_PATH_ACCOUNT_LOGIN,
                               bufferIn, bufferOut))
        return VerifyError::NetworkError;
    if (bufferOut[0] != char(WICHAT_SERVER_RESPONSE_SUCCESS))
        return VerifyError::VerificationFailed;

    // Extract session and account information
    d->currentSession = bufferOut.mid(1, SessionLen);
    d->sessionValidTime = *((qint16*)(bufferOut.mid(KeyLen + 1, 2).data()));
    d->currentState = d->intToOnlineState(bufferOut.mid(KeyLen + 3, 1)[1]);
    bufferOut.remove(0, SessionLen + 4);
    d->encoder.decrypt(Encryptor::AES,
                       bufferOut,
                       tempKey,
                       bufferIn);
    d->sessionKey = bufferIn.mid(0, KeyLen);
    d->currentOfflineMsg = bufferIn.mid(KeyLen);
    d->currentID = ID;

    return VerifyError::Ok;
}

bool Account::setPassword(QString oldPassword, QString newPassword)
{
    Q_D(Account);
    if (!checkPassword(oldPassword) || !checkPassword(newPassword))
        return false;
    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(9)).append(char(qrand() * 256));
    bufferIn.append(d->encoder.getSHA256(oldPassword.toLatin1()).left(KeyLen));
    bufferIn.append(d->encoder.getSHA256(newPassword.toLatin1()).left(KeyLen));
    if (!d->exchangeData(bufferIn, bufferOut, d->Action_Acc_Change))
        return false;
    if (bufferOut[0] != char(WICHAT_SERVER_RESPONSE_SUCCESS))
        return false;
    return true;
}

bool Account::resetSession()
{
    Q_D(Account);
    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(5)).append(char(qrand() * 256));
    if (!d->exchangeData(bufferIn, bufferOut, d->Action_Acc_Change))
        return false;
    if (bufferOut[0] != char(WICHAT_SERVER_RESPONSE_SUCCESS))
        return false;
    d->encoder.decrypt(Encryptor::AES,
                       bufferOut,
                       d->sessionKey,
                       bufferIn);
    if (bufferIn.length() < KeyLen)
        return false;
    d->sessionKey = bufferIn.left(KeyLen);
    return true;
}

Account::OnlineState Account::state()
{
    Q_D(Account);
    return d->currentState;
}

bool Account::setState(OnlineState newState)
{
    Q_D(Account);
    if (d->currentState == OnlineState::None)
        return false;

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(8)).append(char(newState));
    if (!d->exchangeData(bufferIn, bufferOut, d->Action_Acc_Change))
        return false;
    if (bufferOut.length() < 1 ||
        bufferOut[0] != char(WICHAT_SERVER_RESPONSE_SUCCESS))
        return false;

    d->currentState = newState;
    return true;
}

QString Account::offlineMsg()
{
    Q_D(Account);
    return d->currentOfflineMsg;
}

bool Account::setOfflineMsg(QString newMessage)
{
    Q_D(Account);
    newMessage.truncate(MaxOfflineMsgLen - 2);

    QByteArray bufferIn, bufferOut;
    bufferIn.append(char(7)).append(char(qrand() * 256));
    bufferIn.append("<MSG>")
            .append(newMessage.toLatin1())
            .append("</MSG>");
    if (!d->exchangeData(bufferIn, bufferOut, d->Action_Acc_Change))
        return false;
    if (bufferOut.length() < 1 ||
        bufferOut[0] != char(WICHAT_SERVER_RESPONSE_SUCCESS))
        return false;

    d->currentOfflineMsg = newMessage;
    return true;
}

bool Account::queryFriendList()
{
    return true;
}

bool Account::addFriend(QString ID)
{
    return true;
}

bool Account::removeFriend(QString ID)
{
    return true;
}

bool Account::queryFriendRemarks(QList<QString> IDs)
{
    return true;
}

bool Account::setFriendRemarks(QString ID, QString remarks)
{
    return true;
}

bool Account::queryFriendInfo(QString ID)
{
    return true;
}

AccountPrivate::AccountPrivate(Account* parent)
{
    this->q_ptr = parent;
}

bool AccountPrivate::exchangeData(const QByteArray& data,
                                  QByteArray& result,
                                  ServerObject object)
{
    QByteArray temp, bufferIn;
    result.clear();
    switch (object)
    {
        case Action_Acc_Change:
        case Action_Fri_Change:
            bufferIn.append(currentSession);
            bufferIn.append(encoder.getCRC32(data));
            encoder.encrypt(Encryptor::AES,
                            data,
                            sessionKey,
                            temp);
            bufferIn.append(temp);
            if (!server.sendRequest(1,
                                    WICHAT_SERVER_PATH_ACCOUNT_ACTION,
                                    bufferIn,
                                    temp))
                return false;
            break;
        default:
            return false;
    }
    if (temp.length() < 4)
        return false;
    encoder.decrypt(Encryptor::AES,
                    temp.mid(4),
                    sessionKey,
                    result);
    if (encoder.getCRC32(result) != temp.mid(0, 4))
        return false;
    else
        return true;
}



QByteArray AccountPrivate::formatID(QString ID)
{
    Q_Q(Account);
    return ID.leftJustified(q->MaxIDLen, '\0', true).toLatin1();
}

Account::OnlineState AccountPrivate::intToOnlineState(int var)
{
    switch (var)
    {
        case WICHAT_ACCOUNT_STATE_ONLINE:
            return Account::OnlineState::Online;
        case WICHAT_ACCOUNT_STATE_OFFLINE:
            return Account::OnlineState::Offline;
        case WICHAT_ACCOUNT_STATE_BUSY:
            return Account::OnlineState::Busy;
        case WICHAT_ACCOUNT_STATE_HIDE:
            return Account::OnlineState::Hide;
        default:
            return Account::OnlineState::None;
    }
}
