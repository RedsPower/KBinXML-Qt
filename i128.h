#ifndef I128_H
#define I128_H

#include <QObject>
#include <QList>
#include <cstdlib>

class i128
{
public:
    i128(quint64 a, quint64 b);

    QList<quint8> toint8s();
    QList<quint16> toint16s();
    QList<quint32> toint32s();
    QList<quint64> toint64s();
    QList<float> tofloats();
    QList<double> todoubles();
private:
    quint64 a;
    quint64 b;
};

#endif // I128_H
