#ifndef KBINXML_H
#define KBINXML_H

#include "KBinXML-Qt_global.h"
#include <QByteArray>
#include <QString>
#include <QDataStream>
#include <QHostAddress>
#include <QtXml>
#include "sixbit.h"

//namespace kbin {

class dataType;

const int KBinPaddingSize = 4;

class KBINXMLQT_EXPORT KBinXML
{
public:
    KBinXML();
    KBinXML(QByteArray binaryData);
    KBinXML(QString xml);
    QString toXML();
    QByteArray toBin(QString targetCodec = "Shift-JIS");
    bool isLoaded() const;
    QByteArray testFunc(QString testStr = "Shift-JIS");

private:
    bool isKBin();
    static bool isKBin(const QByteArray &data);

    //QByteArray readDataFromStream(QDataStream &stream);
    QByteArray readRawData(QDataStream &stream, quint32 size);
    void writeRawData(QDataStream &stream, QByteArray data);
    //QByteArray readData(QDataStream &stream);
    void readPaddingBytes(QDataStream &stream, int size = KBinPaddingSize);
    void writePaddingBytes(QDataStream &stream, int size = KBinPaddingSize);
    void writePaddingBytes(QDataStream &stream, QByteArray data, int size = KBinPaddingSize);

    dataType typePrase(QString typeString);

    template <typename T> QList<T> readData(QDataStream &stream, int count = 1);
    template <typename T> QList<T> readDataArray(QDataStream &stream);
    QByteArray readDataArray(QDataStream &stream);

    template <typename T> QString genDataString(QList<T> list);
    //QString genDataString(QList<float> list);

    template <typename T> void appendData(QDataStream &stream, QList<T> list, int count = 1);
    template <typename T> void appendDataArray(QDataStream &stream, QList<T> data);
    void appendDataArray(QDataStream &stream, QByteArray data);

    template <typename T> T stringToVar(QString string, bool *ok);

    template <typename T> QList<T> praseDataString(QString string);

    void processNodes(QDomNode node, QDataStream &nodeStream, QDataStream &dataStream);

    void writeNodeName(QDataStream &stream, QString nodeName);

    QByteArray xmlData;
    QByteArray binData;
    bool loaded;
    bool isSixBitCoded;
    QString XMLEncoding;

    QTextCodec *xmlCodec;

    size_t singleDataSize = 0;
    quint32 readedDataSize = 0;
    quint32 writedDataSize = 0;


};
template <> QString KBinXML::genDataString(QList<float> list);
//}

#endif // KBINXML_H
