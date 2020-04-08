#ifndef KBINXMLQT_GLOBAL_H
#define KBINXMLQT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(KBINXMLQT_LIBRARY)
#  define KBINXMLQT_EXPORT Q_DECL_EXPORT
#else
#  define KBINXMLQT_EXPORT Q_DECL_IMPORT
#endif

#endif // KBINXMLQT_GLOBAL_H
