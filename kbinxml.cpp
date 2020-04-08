#include "kbinxml.h"
#include <QMap>
#include <QHash>
#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtXml>
#include <QTextCodec>
#include <QDebug>

#define WRITEDATATOXML(type, isArray)                                                                           \
QList<type> tmpList = isArray ? readData<type>(dataStream, nodeType.count) : readDataArray<type>(dataStream);   \
QString tmpString = genDataString<type>(tmpList);                                                               \
writer.writeCharacters(tmpString)

#define WRITEDATATOBIN(type, isArray)                                                                               \
QList<type> dataList;                                                                                               \
for(QString string : spiltedDataList)                                                                               \
{                                                                                                                   \
    bool ok;                                                                                                        \
    dataList.append(static_cast<type>(stringToVar<type>(string, &ok)));                                             \
    Q_ASSERT(ok);                                                                                                   \
}                                                                                                                   \
isArray ? appendData<type>(dataStream, dataList, parsedDataType.count) : appendDataArray<type>(dataStream, dataList)

#include "sixbit.h"
#include "formatid.h"

static const quint8 MAGIC_NUMBER = 0xA0;
static const quint8 SIG_COMPRESS = 0x42;
static const quint8 SIG_UNCOMPRESS = 0x45;

static const int INVAILD = -1;
static const int UNKNOWN = 0;

static QHash<quint8, QString> encodingFlag;
static QHash<QString, quint8> encodingFlagMap;

enum type {b = 1, s8, u8, s16, u16, s32, u32, s64, u64, f, d, ip4, UNIXtime, binary, str, node_start, attr, node_end, doc_end};
/*
typedef struct
{
    QString key;
    QString content;
} XMLAttrib;
*/
class dataType
{
public:
    //dataType();
    /*
    dataType(int size, int count, type varType, bool isVVar)
    {
        this->size = size;
        this->count = count;
        this->varType = varType;
        this->isVVar = isVVar;
    }
    dataType(dataType &&data)
    {
        this->size = data.size;
        this->count = data.count;
        this->isVVar = data.isVVar;
        this->varType = data.varType;
        this->arraySize = data.arraySize;
    }
    */
    int size = UNKNOWN;
    int count = UNKNOWN;
    type varType;
    int arraySize = UNKNOWN;
    bool isVVar;            //一些前缀是个v的变量，表示“共占用128bit（16字节）”

private:
    char padding[3];        //只是为了过掉警告，没有任何用处
};

/*
typedef struct
{
    dataType type;
    QList<XMLAttrib> attrib;
    QString content;
} node;
*/

void init( void )
{
    encodingFlag.insert(0x00, "SHIFT-JIS");
    encodingFlag.insert(0x20, "ASCII");
    encodingFlag.insert(0x40, "ISO-8859-1");
    encodingFlag.insert(0x60, "EUC-JP");
    encodingFlag.insert(0x80, "SHIFT-JIS");
    encodingFlag.insert(0xA0, "UTF-8");
    QHashIterator<quint8, QString> i(encodingFlag);
    while (i.hasNext())
    {
        i.next();
        encodingFlagMap.insert(i.value(), i.key());
    }
}

KBinXML::KBinXML()
{
    init();
}

KBinXML::KBinXML(QByteArray binaryData)
{
    QByteArray result;
    QXmlStreamWriter writer(&result);
    QDataStream stream(binaryData);
    QByteArray nodeData;
    QByteArray data;

    bool hasNodeLeft;

    init();
    binData = binaryData;
    stream.setVersion(QDataStream::Qt_5_12);
    stream.setByteOrder(QDataStream::BigEndian);

    if(!isKBin())
    {
        loaded = false;
        return;
    }

    {
        quint8 tempu8;
        stream >> tempu8;

        Q_ASSERT(tempu8 == MAGIC_NUMBER);

        stream >> tempu8;
        if(tempu8 == SIG_COMPRESS)
            isSixBitCoded = true;
        else if(tempu8 == SIG_UNCOMPRESS)
            isSixBitCoded = false;
        else
            qFatal("compress tag failed");

        stream >> tempu8;
        XMLEncoding = encodingFlag[tempu8];
        Q_ASSERT(not XMLEncoding.isEmpty());

        Q_ASSERT((tempu8 & 0xFF) != (static_cast<void>(stream >> tempu8), tempu8));
    }


    nodeData = readDataArray(stream);
    data = readDataArray(stream);

    QDataStream nodeStream(nodeData);
    QDataStream dataStream(data);

    hasNodeLeft = true;
    QTextCodec *codec = QTextCodec::codecForName(XMLEncoding.toLocal8Bit());

    writer.setCodec(codec);
    writer.setAutoFormatting(true);
    writer.writeStartDocument("1.0");

    while (hasNodeLeft && !nodeStream.atEnd())
    {
        quint8 nodeID;
        nodeStream >> nodeID;
        bool isArray = nodeID & 0x40;
        nodeID &= ~0x40;

        dataType nodeType = typePrase(formatID(nodeID).toString());

        if(!isArray)
            nodeType.arraySize = INVAILD;

        if(nodeType.varType == node_end)
        {
            writer.writeEndElement();
            continue;
        }
        if(nodeType.varType == doc_end)
        {
            writer.writeEndDocument();
            hasNodeLeft = false;
            continue;
        }

        QString nodeName;
        if(isSixBitCoded)
            nodeName = sixBit::decompress(nodeStream);
        else
        {
            quint8 len;
            nodeStream >> len;
            len = (len & ~0x40) + 1;
            nodeName = readRawData(nodeStream, len);
        }

        if(nodeType.varType == node_start)
        {
            writer.writeStartElement(nodeName);
            continue;
        }

        writer.writeAttribute("__type", formatID(nodeID).toString());

        if(nodeType.varType == attr)
        {
            QString value = readDataArray(dataStream);
            writer.writeAttribute(nodeName, value);
            continue;
        }

        if(!isArray)
            nodeType.arraySize = INVAILD;
        if(nodeType.varType == str)
        {
            QByteArray bytearray = readDataArray(dataStream);
            writer.writeCharacters(codec->toUnicode(bytearray));
        }
        else if(nodeType.varType == binary)
        {
            writer.writeCharacters(readDataArray(dataStream).toHex().toUpper());
        }
        else
        {
            switch (nodeType.varType)
            {
            case b:
            case s8:
            {
                WRITEDATATOXML(qint8, isArray);
            }
            break;
            case u8:
            {
                WRITEDATATOXML(quint8, isArray);
            }
            break;
            case s16:
            {
                WRITEDATATOXML(qint16, isArray);
            }
            break;
            case u16:
            {
                WRITEDATATOXML(quint16, isArray);
            }
            break;
            case s32:
            {
                WRITEDATATOXML(qint32, isArray);
            }
            break;
            case UNIXtime:
            {
                Q_ASSERT(nodeType.count == 1);
            }
                Q_FALLTHROUGH();
            case u32:
            {
                WRITEDATATOXML(quint32, isArray);
            }
            break;
            case s64:
            {
                WRITEDATATOXML(qint64, isArray);
            }
            break;
            case u64:
            {
                WRITEDATATOXML(quint64, isArray);
            }
            break;
            case f:
            {
                WRITEDATATOXML(float, isArray);
            }
            break;
            case d:
            {
                WRITEDATATOXML(double, isArray);
            }
            break;
            case ip4:
            {
                Q_ASSERT(nodeType.count == 1);

                QList<quint32> ipList = isArray ? readData<quint32>(dataStream, nodeType.count) : readDataArray<quint32>(dataStream);
                QStringList stringList;
                for(quint32 ipv4 : ipList)
                {
                    QString tmpString = QHostAddress(ipv4).toString();
                    stringList.append(tmpString);

                }
                QString tmpString = genDataString<QString>(stringList);
                writer.writeCharacters(tmpString);
            }
            break;
            case binary:
            case str:
            case node_start:
            case node_end:
            case attr:
            case doc_end:
                break;
            }
        }
        if(isArray)
        {
            /*
            switch (nodeType.varType)
            {
            case b:
            case s8:
            {
                WRITEDATAARRAYTOXML(qint8);
            }
            break;
            case u8:
            {
                WRITEDATAARRAYTOXML(quint8);
            }
            break;
            case s16:
            {
                WRITEDATAARRAYTOXML(qint16);
            }
            break;
            case u16:
            {
                WRITEDATAARRAYTOXML(quint16);
            }
            break;
            case s32:
            {
                WRITEDATAARRAYTOXML(qint32);
            }
            break;
            case UNIXtime:
            {
                Q_ASSERT(nodeType.count == 1);
            }
                Q_FALLTHROUGH();
            case u32:
            {
                WRITEDATAARRAYTOXML(quint32);
            }
            break;
            case s64:
            {
                WRITEDATAARRAYTOXML(qint64);
            }
            break;
            case u64:
            {
                WRITEDATAARRAYTOXML(quint64);
            }
            break;
            case f:
            {
                WRITEDATAARRAYTOXML(float);
            }
            break;
            case d:
            {
                WRITEDATAARRAYTOXML(double);
            }
            break;
            case ip4:
            {
                Q_ASSERT(nodeType.count == 1);
                QList<quint32> ipList = readDataArray<quint32>(dataStream);
                QStringList stringList;
                for(quint32 ipv4 : ipList)
                {
                    QString tmpString = QHostAddress(ipv4).toString();
                    stringList.append(tmpString);

                }
                QString tmpString = genDataString<QString>(stringList);
                writer.writeCharacters(tmpString);
            }
            break;
            case binary:
            {
                QByteArray binData = readDataArray(dataStream);
                writer.writeCharacters(binData.toHex().toUpper());
            }
            break;
            case str:
            {
                QString string = codec->toUnicode(readDataArray(dataStream));
                writer.writeCharacters(string);
            }
            break;
            case node_start:
            case node_end:
            case attr:
            case doc_end:
                break;
            }
            */
                writer.writeAttribute("__count", QString("%1")
                                                     .arg(readedDataSize / static_cast<quint32>(nodeType.size * nodeType.count)));
        }


    }

    xmlData = result;
    loaded = true;

}

KBinXML::KBinXML(QString xml)
{
    init();
    xmlData = xml.toUtf8();
    isSixBitCoded = true;
}

QString KBinXML::toXML()
{
    return xmlData;
}

QByteArray KBinXML::toBin(QString targetCodec)
{
    Q_ASSERT(encodingFlagMap.keys().contains(targetCodec));
    QTextCodec *outputCodec = QTextCodec::codecForName(targetCodec.toLocal8Bit());
    //QXmlStreamReader reader(xmlData);
    QDomDocument doc;

    QByteArray result;
    QByteArray nodeArray;
    QByteArray dataArray;
    QDataStream nodeStream(nodeArray);
    QDataStream dataStream(dataArray);
    QDataStream resultStream(result);

    //XMLEncoding = reader.documentEncoding().toString();
    doc.setContent(xmlData);
    QDomNode root = doc.documentElement();

    nodeStream.setVersion(QDataStream::Qt_5_12);
    nodeStream.setByteOrder(QDataStream::BigEndian);
    dataStream.setVersion(QDataStream::Qt_5_12);
    dataStream.setByteOrder(QDataStream::BigEndian);
    resultStream.setVersion(QDataStream::Qt_5_12);
    resultStream.setByteOrder(QDataStream::BigEndian);

    resultStream << static_cast<quint8>(MAGIC_NUMBER);
    if(isSixBitCoded)
        resultStream << static_cast<char>(SIG_COMPRESS);
    else
        resultStream << static_cast<char>(SIG_UNCOMPRESS);
    resultStream << static_cast<char>(encodingFlagMap[targetCodec]);
    resultStream << static_cast<char>(encodingFlagMap[targetCodec] ^ 0xFF);
/*
    while(!reader.atEnd())
    {
        QXmlStreamReader::TokenType tokentype = reader.readNext();
        if(tokentype == QXmlStreamReader::Comment)
            continue;
        if(tokentype == QXmlStreamReader::StartElement)
        {
            if(reader.attributes().hasAttribute("__type"))
                nodeStream << formatID(reader.attributes().value("__type").toString()).toID();
            else
                nodeStream << formatID("void").toID();

            dataType DataType = typePrase(reader.attributes().value("__type").toString());

            if(reader.attributes().hasAttribute("__count"))
                DataType.arraySize = reader.attributes().value("__count").toInt();
            else
            {

                DataType.arraySize = INVAILD;
            }

            QStringList varList;
            if(DataType.varType not_eq str and DataType.varType not_eq binary)
                varList = reader.readElementText().split(' ');
            if(DataType.arraySize == INVAILD)
            {
                Q_ASSERT(varList.count() == DataType.count);
                switch (DataType.varType)
                {
                case b:
                case s8:
                {
                    WRITEDATATOBIN(qint8, toInt);
                }
                break;
                case u8:
                {
                    WRITEDATATOBIN(quint8, toUInt);
                }
                break;
                case s16:
                {
                    WRITEDATATOBIN(qint16, toInt);
                }
                break;
                case u16:
                {
                    WRITEDATATOBIN(quint16, toUInt);
                }
                break;
                case s32:
                {
                    WRITEDATATOBIN(qint32, toInt);
                }
                break;
                case UNIXtime:
                    Q_ASSERT(DataType.count == 1);
                    Q_FALLTHROUGH();
                case u32:
                {
                    WRITEDATATOBIN(quint32, toUInt);
                }
                break;
                case s64:
                {
                    WRITEDATATOBIN(qint64, toInt);
                }
                break;
                case u64:
                {
                    WRITEDATATOBIN(quint64, toUInt);
                }
                break;
                case f:
                {
                    WRITEDATATOBIN(float, toFloat);
                }
                break;
                case d:
                {
                    WRITEDATATOBIN(double, toDouble);
                }
                break;
                case ip4:
                {
                    QList<quint32> ipList;
                    for(QString string : varList)
                    {
                        ipList.append(QHostAddress(string).toIPv4Address());
                    }
                    if(ipList.count() == 1)
                        appendData(dataStream, ipList);

                }
                break;
                case binary:
                case str:
                {
                    DataType.arraySize = UNKNOWN;
                }
                break;
                case attr:
                case node_start:
                case node_end:
                case doc_end:
                    break;
                }
            }
            else
            {

                switch (DataType.varType)
                {

                }
            }

            if(isSixBitCoded)
                nodeStream << sixBit::compress(codec->fromUnicode(reader.name()));
            else
            {
                nodeStream << static_cast<quint8>(codec->fromUnicode(reader.name()).length());
                writeRawData(nodeStream, codec->fromUnicode(reader.name()));
            }

            QXmlStreamAttributes attrs = reader.attributes();
            for(QXmlStreamAttribute attr : attrs)
            {
                if(attr.name() != "__type" && attr.name() != "__count")
                {
                    if(isSixBitCoded)
                        nodeStream << sixBit::compress(codec->fromUnicode(attr.name()));
                    else
                    {
                        nodeStream << static_cast<quint8>(codec->fromUnicode(reader.name()).length());
                        writeRawData(nodeStream, codec->fromUnicode(reader.name()));
                    }
                    appendDataArray(dataStream, codec->fromUnicode(attr.value()));
                }
            }
        }
        if(tokentype == QXmlStreamReader::EndElement)
            nodeStream << static_cast<quint8>(0xFE);
        if(tokentype == QXmlStreamReader::EndDocument)
        {
            nodeStream << static_cast<quint8>(0xFF);
            writePaddingBytes(nodeStream, nodeArray);
            writePaddingBytes(dataStream, dataArray);
        }
    }
    if(reader.hasError())
    {
        Q_ASSERT(false);
        return nullptr;
    }
    else
    {

        return binData;
    }
*/

    for(QDomNode node = root.previousSibling();not node.isNull(); node = node.previousSibling())
    {
        if(node.nodeType() == QDomNode::ProcessingInstructionNode)
        {
            QDomProcessingInstruction instruction = node.toProcessingInstruction();
            if(instruction.target() == "xml")
            {
                QStringList list = instruction.data().split(' ');
                for(QString string : list)
                {
                    QStringList tempList = string.split("=");
                    if(tempList.contains("encoding"))
                    {
                        XMLEncoding = tempList.at(1);
                        XMLEncoding.remove('\"');
                    }
                }
            }
        }
    }
    if(XMLEncoding.isNull())
        XMLEncoding = "UTF-8";

    //QTextCodec *inputCodec = QTextCodec::codecForName(XMLEncoding.toLocal8Bit());

    processNodes(root, nodeStream, dataStream, outputCodec);
    nodeStream << static_cast<quint8>(formatID("xmlEnd").toID() | 0x40);
    writePaddingBytes(nodeStream, nodeArray);
    writePaddingBytes(dataStream, dataArray);

    resultStream << dataArray;
    resultStream << nodeStream;

    binData = result;
    return result;
}

bool KBinXML::isLoaded() const
{
    return loaded;
}

bool KBinXML::isKBin()
{
    return isKBin(binData);
}

bool KBinXML::isKBin(QByteArray data)
{
    QString xmlEncoding;
    init();

    if(static_cast<quint8>(data.at(0)) != MAGIC_NUMBER)
        return false;

    if(static_cast<quint8>(data.at(1)) == SIG_COMPRESS || static_cast<quint8>(data.at(1)) == SIG_UNCOMPRESS)
        ;
    else
        return false;

    if(encodingFlag.keys().contains(static_cast<quint8>(data.at(2))))
        return false;

    if(static_cast<quint8>(data.at(2) != (static_cast<quint8>(data.at(3)) ^ static_cast<quint8>(0xFF))))
        return false;

    return true;
}
/*
QByteArray KBinXML::readDataFromStream(QDataStream &stream)
{
    quint32 size;
    stream >> size;
    return readDataFromStream(stream, size);
}
*/
QByteArray KBinXML::readRawData(QDataStream &stream, quint32 size)
{
    quint8 tmp;
    QByteArray result;
    for(quint32 i = 0; i < size; ++i)
    {
        stream >> tmp;
        result = result.append(static_cast<char>(tmp));
    }
    return result;
}

void KBinXML::writeRawData(QDataStream &stream, QByteArray data)
{
    for(int i = 0; i < data.size(); ++i)
    {
        stream << static_cast<quint8>(data.at(i));
    }
}
/*
QByteArray KBinXML::readData(QDataStream &stream)
{
    QByteArray result = readDataFromStream(stream);
    readPaddingBytes(stream, static_cast<quint32>(result.size()));
    return result;
}
*/
/*
QString KBinXML::getNodeName(QDataStream &stream)
{
    QString name;

    if(isSixBitCoded)
        name = sixBit::decompress(stream);
    else
    {
        quint8 nameLen;
        stream >> nameLen;
        readRawData(stream, nameLen);
    }
    return name;
}
*/
dataType KBinXML::typePrase(QString typeString)
{
    dataType result;

    //排除特殊情况
    if(typeString == "void")
    {
        result.size = INVAILD;
        result.count = INVAILD;
        result.isVVar = false;
        result.varType = type::node_start;
    }

    if(typeString == "bin")
    {
        result.size = UNKNOWN;
        result.count = INVAILD;
        result.isVVar = false;
        result.varType = type::binary;
    }

    if(typeString == "str")
    {
        result.size = UNKNOWN;
        result.count = INVAILD;
        result.isVVar = false;
        result.varType = type::str;
    }

    if(typeString == "ip4")
    {
        result.size = sizeof (quint32);
        result.count = 1;
        result.isVVar = false;
        result.varType = type::ip4;
    }

    if(typeString == "time")
    {
        result.size = sizeof (quint32);
        result.count = 1;
        result.isVVar = false;
        result.varType = type::UNIXtime;
    }
    if(typeString == "attr")
    {
        result.size = UNKNOWN;
        result.count = INVAILD;
        result.isVVar = false;
        result.varType = type::attr;
    }
    if(typeString == "nodeEnd")
    {
        result.size = INVAILD;
        result.count = INVAILD;
        result.isVVar = false;
        result.varType = type::node_end;
    }
    if(typeString == "xmlEnd")
    {
        result.size = INVAILD;
        result.count = INVAILD;
        result.isVVar = false;
        result.varType = type::doc_end;
    }
    if(typeString == "void" || typeString == "bin" || typeString == "str"
        || typeString == "ip4" || typeString == "time" || typeString == "attr"
        || typeString == "nodeEnd" || typeString == "xmlEnd")
        return result;

    if(typeString[0] == 'v')
    {
        result.isVVar = true;
        typeString = typeString.right(typeString.length() - 1);
    }
    if(typeString.front().isDigit())
    {
        if(typeString.front() >= '2' && typeString.front() <= '4')
        {
            QString count;
            count = typeString.at(0);
            result.count = count.toInt();
            typeString = typeString.right(typeString.length() - 1);
        }
        else
        {
            return dataType();
        }
    }

    if(typeString == "s8")
        result.varType = s8;
    if(typeString == "u8")
        result.varType = u8;
    if(typeString == "s16")
        result.varType = s16;
    if(typeString == "u16")
        result.varType = u16;
    if(typeString == "s32")
        typeString = s32;
    if(typeString == "u32")
        result.varType = u32;
    if(typeString == "s64")
        result.varType = s64;
    if(typeString == "u64")
        result.varType = u64;
    if(typeString == "b")
        result.varType = b;
    if(typeString == "f")
        result.varType = f;
    if(typeString == "d")
        result.varType = d;

    if(result.varType >= b && result.varType <= d)
    {
        if(result.varType == b || result.varType == s8 || result.varType == u8)
            result.size = sizeof (quint8);
        if(result.varType == s16 || result.varType == u16)
            result.size = sizeof (quint16);
        if(result.varType == s32 || result.varType == u32)
            result.size = sizeof (quint32);
        if(result.varType == s64 || result.varType == u64)
            result.size = sizeof (quint64);
        if(result.varType == f)
            result.size = sizeof (float);     //这里应为32bit（4字节）
        if(result.varType == d)
            result.size = sizeof (double);    //这里应为64bit（8字节）

        if(result.count == UNKNOWN)
            result.count = 1;
    }

    if(result.isVVar)
    {
        result.count = 128/8 / result.size;
    }

    return result;
}

template<typename T>
QList<T> KBinXML::readData(QDataStream &stream, int count)
{
    Q_ASSERT(count > 0);
    QList<T> result;

    if(singleDataSize == 0)
        singleDataSize = sizeof (T) * static_cast<unsigned int>(count);
    else
    {
        if(sizeof (T) * static_cast<size_t>(count) != singleDataSize)
        {
            readPaddingBytes(stream);
        }
    }
    for(int i = 0; i < count; ++i)
    {
        T tmp;
        stream >> tmp;
        result.append(tmp);
    }
    readedDataSize += sizeof (T) * static_cast<size_t>(count);
    return result;
}

template<typename T>
QList<T> KBinXML::readDataArray(QDataStream &stream)
{
    quint32 dataSize;
    QList<T> result;

    readPaddingBytes(stream);
    stream >> dataSize;

    for(quint32 i = 0; i < dataSize / sizeof (T); ++i)
    {
        T tmp;
        stream >> tmp;
        result.append(tmp);
    }
    readedDataSize = sizeof (quint32) + dataSize;
    readPaddingBytes(stream);
    return result;
}

QByteArray KBinXML::readDataArray(QDataStream &stream)
{
    //quint32 dataSize;
    QByteArray result;

    readPaddingBytes(stream);
    stream >> result;
    readedDataSize = static_cast<quint32>(result.size());
    readPaddingBytes(stream);
    return result;
}

void KBinXML::readPaddingBytes(QDataStream &stream, int size)
{
    Q_ASSERT(size > 0);
    for(quint32 i = readedDataSize % static_cast<quint32>(size); i % static_cast<quint32>(size) > 0; ++i)
    {
        quint8 padding;
        stream >> padding;
        Q_ASSERT(padding == 0);
    }
    singleDataSize = 0;
    readedDataSize = 0;
}

void KBinXML::writePaddingBytes(QDataStream &stream, int size)
{
    for(quint32 i = writedDataSize % static_cast<quint32>(size); i % static_cast<quint32>(size); ++i)
    {
        stream << static_cast<quint8>('\0');
    }
    singleDataSize = 0;
    writedDataSize = 0;
}

void KBinXML::writePaddingBytes(QDataStream &stream, QByteArray data, int size)
{
    for(int i = data.size() % size; i % size != 0; ++i)
    {
        stream << static_cast<quint8>(0);
    }
}

template<class T>
QString KBinXML::genDataString(QList<T> list)
{
    bool firstTimeExec = true;
    QString tmpString;
    for(int i = 0; i < list.size(); ++i)
    {
        if(not firstTimeExec)
            tmpString.append(' ');
        tmpString.append(QString("%1").arg(list.at(i)));
        firstTimeExec = false;
    }
    return tmpString;
}

template<>
QString KBinXML::genDataString<float>(QList<float> list)
{
    bool firstTimeExec = true;
    QString tmpString;
    for(int i = 0; i < list.size(); ++i)
    {
        if(!firstTimeExec)
            tmpString.append(' ');
        tmpString.append(QString("%1").arg(static_cast<double>(list.at(i))));
        firstTimeExec = false;
    }
    return tmpString;
}

template<class T>
void KBinXML::appendData(QDataStream &stream, QList<T> list, int count)
{
    Q_ASSERT(list.count() == count);

    if(singleDataSize == 0)
        singleDataSize = sizeof (T) * static_cast<size_t>(count);
    else
    {
        if(sizeof (T) * static_cast<size_t>(count) != singleDataSize)
        {
            writePaddingBytes(stream);
            singleDataSize = 0;
        }
    }

    for(int i = 0; i == list.count(); ++i)
    {
        stream << list.at(i);
    }
    writedDataSize += sizeof (T) * static_cast<size_t>(list.size());
}

template<class T>
void KBinXML::appendDataArray(QDataStream &stream, QList<T> data)
{
    writePaddingBytes(stream);
    stream << static_cast<quint32>(data.size());
    for(int i = 0; i < data.size(); ++i)
    {
        stream << data.at(i);
    }
    writedDataSize += sizeof (quint32) + static_cast<size_t>(data.size()) * sizeof (T);
    writePaddingBytes(stream);
}

void KBinXML::appendDataArray(QDataStream &stream, QByteArray data)
{
    writePaddingBytes(stream);
    stream << data;
    writedDataSize += sizeof (quint32) + static_cast<quint32>(data.size());
    writePaddingBytes(stream);
}

void KBinXML::processNodes(QDomNode node, QDataStream &nodeStream, QDataStream &dataStream, QTextCodec *outputCodec)
{
    if(node.isElement())
    {
        bool isArray = false;
        QString nodeTypeString;
        QDomElement element = node.toElement();
        if(not element.isNull())
        {
            if(element.attributes().contains("__type"))
            {
                nodeTypeString = element.attribute("__type");
            }
            else
            {
                if(element.childNodes().count() == 1 and element.firstChild().isText())
                {
                    nodeTypeString = "str";
                }
                else
                {
                    nodeTypeString = "void";
                }
            }

            if(element.hasAttribute("__count"))
            {
                isArray = true;
            }

            nodeStream << (formatID(nodeTypeString).toID() | (isArray ? 0x40 : 0));

            QString nodeName = element.tagName();
            if(isSixBitCoded)
            {
                nodeStream << sixBit::compress(outputCodec->fromUnicode(nodeName));
            }
            else
            {
                nodeStream << static_cast<quint8>((nodeName.length() - 1) | 0x40);
                writeRawData(nodeStream, outputCodec->fromUnicode(nodeName));
            }

            if(nodeTypeString != "void")
            {
                if(element.childNodes().count() == 1 and element.firstChild().isText())
                {
                    bool isArray;
                    QString data = element.firstChildElement().toText().data();
                    dataType parsedDataType = typePrase(nodeTypeString);
                    if(element.hasAttribute("__count"))
                        parsedDataType.arraySize = element.attribute("__count").toInt();
                    else
                        parsedDataType.arraySize = INVAILD;
                    if(parsedDataType.arraySize == INVAILD)
                        isArray = false;
                    else
                        isArray = true;

                    if(parsedDataType.varType == binary)
                    {
                        appendDataArray(dataStream, QByteArray::fromHex(data.toUtf8()));
                    }
                    else if(parsedDataType.varType == str)
                    {
                        appendDataArray(dataStream, outputCodec->fromUnicode(data));
                    }
                    else
                    {
                        QStringList spiltedDataList = data.split(' ');
                        if((parsedDataType.size * parsedDataType.count * (parsedDataType.arraySize == INVAILD ? 1 : parsedDataType.arraySize))
                            != spiltedDataList.count())
                            qFatal("attr __count not fetch to arg");

                        switch(parsedDataType.varType)
                        {
                        case b:
                        case s8:
                        {
                            WRITEDATATOBIN(qint8, isArray);
                        }
                        break;
                        case u8:
                        {
                            WRITEDATATOBIN(quint8, isArray);
                        }
                        break;
                        case s16:
                        {
                            WRITEDATATOBIN(qint16, isArray);
                        }
                        break;
                        case u16:
                        {
                            WRITEDATATOBIN(quint16, isArray);
                        }
                        break;
                        case s32:
                        {
                            WRITEDATATOBIN(qint32, isArray);
                        }
                        break;
                        case UNIXtime:
                        {
                            Q_ASSERT(parsedDataType.count == 1);
                        }
                        Q_FALLTHROUGH();
                        case u32:
                        {
                            WRITEDATATOBIN(quint32, isArray);
                        }
                        break;
                        case s64:
                        {
                            WRITEDATATOBIN(quint64, isArray);
                        }
                        break;
                        case u64:
                        {
                            WRITEDATATOBIN(quint64, isArray);
                        }
                        break;
                        case f:
                        {
                            WRITEDATATOBIN(float, isArray);
                        }
                        break;
                        case d:
                        {
                            WRITEDATATOBIN(double, isArray);
                        }
                        break;
                        case ip4:
                        {
                            QList<quint32> ipNumList;
                            for(QString ipString : spiltedDataList)
                            {
                                bool ok;
                                ipNumList.append(QHostAddress(ipString).toIPv4Address(&ok));
                                Q_ASSERT(ok);
                            }
                            isArray ? appendData<quint32>(dataStream, ipNumList) : appendDataArray(dataStream, ipNumList);
                        }
                        break;
                        case node_start:
                        case node_end:
                        case doc_end:
                        case attr:
                        case str:
                        case binary:
                            break;
                        }


                    }
                }
            }

            auto attrList = element.attributes();
            for(int i = 0; i < attrList.count(); ++i)
            {
                QDomNode attribNode = attrList.item(i);
                auto attrib = attribNode.toAttr();
                if(attrib.name() != "__type" and attrib.name() != "__count" and attrib.name() != "__type")
                {
                    nodeStream << formatID("attr").toID();
                    if(isSixBitCoded)
                    {
                        nodeStream << sixBit::compress(outputCodec->fromUnicode(attrib.name()));
                    }
                    else
                    {
                        nodeStream << static_cast<quint8>((attrib.name().length() - 1) | 0x40);
                        writeRawData(nodeStream, outputCodec->fromUnicode(attrib.name()));
                    }
                    appendDataArray(dataStream, outputCodec->fromUnicode(attrib.value()));
                }
            }


            for(auto childNode = element.firstChildElement(); not childNode.isNull(); childNode.nextSiblingElement())
                processNodes(childNode, nodeStream, dataStream, outputCodec);
            nodeStream << static_cast<quint8>(formatID("nodeEnd").toID() | 0x40);
        }
    }
}

template<typename T>
T KBinXML::stringToVar(QString string, bool *ok)
{
    T result = 0;
    auto &varTypeID = typeid (T);
    *ok = false;
    if(varTypeID == typeid (qint8) or varTypeID == typeid (short))
    {
        result = static_cast<T>(string.toShort(ok));
    }
    if(varTypeID == typeid (quint8) or varTypeID == typeid (unsigned short))
    {
        result = static_cast<T>(string.toUShort(ok));
    }
    if(varTypeID == typeid (int))
    {
        result = static_cast<T>(string.toInt(ok));
    }
    if(varTypeID == typeid (unsigned int))
    {
        result = static_cast<T>(string.toUInt(ok));
    }
    if(varTypeID == typeid (long))
    {
        result = static_cast<T>(string.toLong(ok));
    }
    if(varTypeID == typeid (unsigned long))
    {
        result = static_cast<T>(string.toULong(ok));
    }
    if(varTypeID == typeid (long long))
    {
        result = static_cast<T>(string.toLongLong(ok));
    }
    if(varTypeID == typeid (unsigned long long))
    {
        result = static_cast<T>(string.toULongLong(ok));
    }
    if(varTypeID == typeid (float))
    {
        result = static_cast<T>(string.toFloat(ok));
    }
    if(varTypeID == typeid (double))
    {
        result = static_cast<T>(string.toDouble(ok));
    }
    return result;
}
