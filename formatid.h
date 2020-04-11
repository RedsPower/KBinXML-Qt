#ifndef FORMATID_H
#define FORMATID_H

#include <QHash>
#include <QSet>
#include "KBinXML-Qt_global.h"

class KBINXMLQT_EXPORT formatID
{
public:
    formatID(quint8 typeID);
    formatID(QString typeStr);
    quint8 toID() const;
    QString toString() const;

private:
    QHash<quint8, QString> keyMap;
    QHash<QString, quint8> strMap;

    void init();
    QString stringConvert(QString string);

    quint8 id;

};

#endif // FORMATID_H
