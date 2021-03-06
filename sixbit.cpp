#include "sixbit.h"
#include <QDataStream>
#include <QBitArray>
#include <QDebug>

static const QString charmap("0123456789:ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");

sixBit::sixBit()
{

}

QByteArray sixBit::compress(const QByteArray &data)
{
    QBitArray bitArray;
    QByteArray result;
    QByteArray codedData;
    for(qint8 i : data)
    {
        codedData.append(static_cast<char>(charmap.indexOf(i)));
    }

    bitArray.resize((data.length() * 6));
    for(int i = 0; i < data.length(); ++i)
    {
        for(int j = 0; j < 6; ++j)
            bitArray[6 * i + j] = (static_cast<quint8>(codedData[i]) >> (6 - j - 1)) & 1;
    }

    result += static_cast<char>(data.length());
    result += QBitAToQByteA(bitArray);
    return result;
}

QByteArray sixBit::decompress(QDataStream &stream)
{
    QByteArray result;
    QBitArray bitArray;
    QByteArray content;

    stream.setVersion(QDataStream::Qt_5_12);
    stream.setByteOrder(QDataStream::BigEndian);

    quint8 len;
    stream >> len;

    int bitlen = len * 6;
    int bytelen = (bitlen + 7) / 8;

    for(int i = 0; i < bytelen; ++i)
    {
        quint8 temp;
        stream >> temp;
        content.append(static_cast<char>(temp));
    }

    bitArray = QByteAToQBitA(content);
    bitArray.resize(bitlen);

    for(int i = 0; i < len; ++i)
    {
        char temp = 0;
        for(int j = 0; j < 6; ++j)
        {
            temp |= ((bitArray[6 * i + j] ? 1 : 0) << (6 - j - 1));
        }
        result.append(charmap[temp]);
    }

    return result;
}

QByteArray sixBit::QBitAToQByteA(QBitArray data)
{
    QByteArray result;
    int padding = 0;
    if(data.size() % 8 != 0)
        padding = 8 - (data.size() % 8);
    data.resize(data.size() + padding);

    for(int i = 0; i < data.size() / 8; ++i)
    {
        quint8 tmp = 0;
        for(int j = 0; j < 8; ++j)
        {
            tmp |= data[8 * i + j] << (8 - j - 1);
        }
        result.append(static_cast<char>(tmp));
    }
    return result;
}

QBitArray sixBit::QByteAToQBitA(QByteArray data)
{
    QBitArray result(data.length() * 8);
    for(int i = 0; i < data.length(); ++i)
        for(int j = 0; j < 8; ++j)
            result[8 * i + j] = (data.at(i) >> (8 - j - 1)) & 1;

    return result;
}
