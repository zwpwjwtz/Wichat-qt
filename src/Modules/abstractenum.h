#ifndef ABSTRACTENUM_H
#define ABSTRACTENUM_H


class AbstractEnum
{
public:
    int value;

    inline AbstractEnum()
    {
        value = 0;
    }
    inline AbstractEnum(const int& initValue)
    {
        // Attention: rewrite is needed when inherited
        value = initValue;
    }
    inline bool operator ==(const AbstractEnum& a)
    {
        return value == a.value;
    }
    inline bool operator ==(const int& a)
    {
        return value == a;
    }
    inline operator int()
    {
        return value;
    }
    inline int operator =(const int& a)
    {
        // Attention: rewrite is needed when inherited
        value = a;
        return value;
    }
};

#endif // ABSTRACTENUM_H
