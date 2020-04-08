#include "i128.h"

i128::i128(quint64 a, quint64 b)
{
    this->a = a;
    this->b = b;
}

QList<quint8> i128::toint8s()
{
    QList<quint8> result;
    for(int i = 0; i < 8; ++i)
    {
        result.append((a >> (8 * (8 - i - 1))) & 0xFF);
    }
    for(int i = 0; i < 8; ++i)
    {
        result.append((b >> (8 * (8 - i - 1))) & 0xFF);
    }
    return result;
}

QList<quint16> i128::toint16s()
{
    QList<quint16> result;
    for(int i = 0; i < 4; ++i)
    {
        result.append((a >> (16 * (4 - i - 1))) & 0xFFFF);
    }
    for(int i = 0; i < 4; ++i)
    {
        result.append((b >> (16 * (4 - i - 1))) & 0xFFFF);
    }
    return result;
}

QList<quint32> i128::toint32s()
{
    QList<quint32> result;
    result.append(a >> 32);
    result.append(a & 0xFFFFFFFF);
    result.append(b >> 32);
    result.append(b & 0xFFFFFFFF);
    return result;
}

QList<quint64> i128::toint64s()
{
    QList<quint64> result;
    result.append(a);
    result.append(b);
    return result;
}

QList<float> i128::tofloats()
{
    QList<quint32> temp;
    QList<float> result;
    temp = toint32s();
    for(int i = 0; i < temp.size(); ++i)
    {
        memcpy(&result[i], &temp[i], sizeof (float));
    }
    return result;
}

QList<double> i128::todoubles()
{
    QList<quint64> temp;
    QList<double> result;
    temp = toint64s();
    for(int i = 0; i < temp.size(); ++i)
    {
        memcpy(&result[i], &temp[i], sizeof (double));
    }
    return result;
}
