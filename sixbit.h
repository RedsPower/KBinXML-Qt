#ifndef SIXBIT_H
#define SIXBIT_H

#include <QObject>

class sixBit
{
public:
    sixBit();
    static QByteArray compress(QByteArray data);
    static QByteArray decompress(QDataStream &stream);
private:
    static QByteArray QBitAToQByteA(QBitArray data);
    static QBitArray QByteAToQBitA(QByteArray data);
};

#endif // SIXBIT_H
