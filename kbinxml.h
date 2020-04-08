#ifndef KBINXML_H
#define KBINXML_H

#include "KBinXML-Qt_global.h"
#include <QByteArray>
#include <QString>
#include <QDataStream>
#include <QHostAddress>
#include <QtXml>

class dataType;

class KBINXMLQT_EXPORT KBinXML
{
public:
    KBinXML();
    KBinXML(QByteArray binaryData);
    KBinXML(QString xml);
    QString toXML();
    QByteArray toBin(QString targetCodec = "SHIFT-JIS");
    bool isLoaded() const;

private:
    bool isKBin();
    static bool isKBin(QByteArray data);

    //QByteArray readDataFromStream(QDataStream &stream);
    QByteArray readRawData(QDataStream &stream, quint32 len);
    void writeRawData(QDataStream &stream, QByteArray data);
    //QByteArray readData(QDataStream &stream);
    void readPaddingBytes(QDataStream &stream, int size = 4);
    void writePaddingBytes(QDataStream &stream, int size = 4);
    void writePaddingBytes(QDataStream &stream, QByteArray data, int size = 4);

    dataType typePrase(QString typeString);

    template <typename T> QList<T> readData(QDataStream &stream, int count = 1);
    template <typename T> QList<T> readDataArray(QDataStream &stream);
    QByteArray readDataArray(QDataStream &stream);

    template <class T> QString genDataString(QList<T> list);
    template <> QString genDataString(QList<float> list);
    //QString genDataString(QList<float> list);

    template <class T> void appendData(QDataStream &stream, QList<T> list, int count = 1);
    template <class T> void appendDataArray(QDataStream &stream, QList<T> data);
    void appendDataArray(QDataStream &stream, QByteArray data);

    template <typename T> T stringToVar(QString string, bool *ok);

    template <class T> QList<T> praseDataString(QString string);

    void processNodes(QDomNode node, QDataStream &nodeStream, QDataStream &dataStream, QTextCodec *outputCodec);

    QByteArray xmlData;
    QByteArray binData;
    bool loaded;
    bool isSixBitCoded;
    QString XMLEncoding;

    size_t singleDataSize = 0;
    quint32 readedDataSize = 0;
    quint32 writedDataSize = 0;


};

#endif // KBINXML_H
