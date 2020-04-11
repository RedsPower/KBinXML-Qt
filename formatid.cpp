#include "formatid.h"
#include <QDebug>

void formatID::init()
{
    /*
    keyMap.insert(1, "void");
    keyMap.insert(2, "s8");
    keyMap.insert(3, "u8");
    keyMap.insert(4, "s16");
    keyMap.insert(5, "u16");
    keyMap.insert(6, "s32");
    keyMap.insert(7, "u32");
    keyMap.insert(8, "s64");
    keyMap.insert(9, "u64");
    keyMap.insert(10, "bin");
    keyMap.insert(11, "str");
    keyMap.insert(12, "ip4");
    keyMap.insert(13, "time");
    keyMap.insert(14, "f");
    keyMap.insert(15, "d");
    keyMap.insert(16, "2s8");
    keyMap.insert(17, "2u8");
    keyMap.insert(18, "2s16");
    keyMap.insert(19, "2u16");
    keyMap.insert(20, "2s32");
    keyMap.insert(21, "2u32");
    keyMap.insert(22, "2s64");
    keyMap.insert(23, "2u64");
    keyMap.insert(24, "2f");
    keyMap.insert(25, "2d");
    */
    quint8 i = 1;
    keyMap.insert(i, "void");
    ++i;
    for(int j = 1; j <= 4; ++j)
    {
        for (int n = 0; n < 10; ++i,++n)
        {
            QString typeString;

            if(j == 1 && n > 7 and i < 14)
            {
                if(i == 10)
                    typeString = "bin";
                else if(i == 11)
                    typeString = "str";
                else if(i == 12)
                    typeString = "ip4";
                else if(i == 13)
                    typeString = "time";
                else
                    ++n;
                --n;
            }
            else
            {
                switch(n)
                {
                case 0:
                    typeString = "s8";
                    break;
                case 1:
                    typeString = "u8";
                    break;
                case 2:
                    typeString = "s16";
                    break;
                case 3:
                    typeString = "u16";
                    break;
                case 4:
                    typeString = "s32";
                    break;
                case 5:
                    typeString = "u32";
                    break;
                case 6:
                    typeString = "s64";
                    break;
                case 7:
                    typeString = "u64";
                    break;
                case 8:
                    typeString = "f";
                    break;
                case 9:
                    typeString = "d";
                    break;
                }
            }

            if(j != 1)
                typeString = QString("%1").arg(j) + typeString;

            keyMap.insert(i, typeString);

        }
    }
    keyMap.insert(46, "attr");
    // no 47
    keyMap.insert(48, "vs8");
    keyMap.insert(49, "vu8");
    keyMap.insert(50, "vs16");
    keyMap.insert(51, "vu16");
    keyMap.insert(52, "b");
    keyMap.insert(53, "2b");
    keyMap.insert(54, "3b");
    keyMap.insert(55, "4b");
    keyMap.insert(56, "vb");

    //------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------

    QHash<quint8, QString>::const_iterator iterator;
    for(iterator = keyMap.cbegin(); iterator != keyMap.cend(); ++iterator)
    {
        strMap.insert(iterator.value(), iterator.key());
    }

    //------------------------------------------------------------------------------------------------

    keyMap.insert(0xBE, "nodeEnd");
    keyMap.insert(0xBF, "xmlEnd");
    qDebug() << keyMap;
}

QString formatID::stringConvert(QString string)
{
    if(string == "string")
        string = "str";
    if(string == "binary")
        string = "bin";
    if(string == "float")
        string = "f";
    if(string == "double")
        string = "d";
    if(string == "bool")
        string = "b";

    if(string == "vs32" or string == "vu32")
        string[0] = '4';
    if(string == "vs64" or string == "vu64")
        string[0] = '2';
    if(string == "vf")
        string[0] = '4';
    if(string == "vd")
        string[0] = '2';

    return string;
}

formatID::formatID(quint8 typeID)
{
    init();
    id = typeID;
}

formatID::formatID(QString typeStr)
{
    init();
    typeStr = stringConvert(typeStr);
    id = strMap[typeStr];
}

quint8 formatID::toID() const
{
    return id;
}

QString formatID::toString() const
{
    return keyMap[id];
}
