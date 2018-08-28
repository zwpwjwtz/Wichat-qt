#ifndef ONLINESTATE_H
#define ONLINESTATE_H

#include "abstractenum.h"
#include <QString>


class OnlineState : public AbstractEnum
{
public:
    static const int None = 0;
    static const int Online = 1;
    static const int Offline = 2;
    static const int Busy = 4;
    static const int Hide = 5;

    inline OnlineState()
    {
        value = None;
    }
    inline OnlineState(const int initValue)
    {
        value = initValue;
    }
    inline int operator =(const int a)
    {
        value = a;
        return value;
    }

    __attribute__((noinline)) QString toString()
    {
        switch (value)
        {
            case Busy:
                return "Busy";
            case Hide:
                return "Hide";
            case Online:
                return "Online";
            case Offline:
            default:
                return "Offline";
        }
    }
};

#endif // ONLINESTATE_H
