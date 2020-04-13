#ifndef SIXBIT_H
#define SIXBIT_H

#include "KBinXML-Qt_global.h"
#include <QObject>

class KBINXMLQT_EXPORT sixBit
{
public:
    sixBit();
    static QByteArray compress(const QByteArray &data);
    static QByteArray decompress(QDataStream &stream);
private:
    static QByteArray QBitAToQByteA(QBitArray data);
    static QBitArray QByteAToQBitA(QByteArray data);
};

#endif // SIXBIT_H
