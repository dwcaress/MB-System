#ifndef MBGUI_GLOBAL_H
#define MBGUI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MBGUI_LIBRARY)
#  define MBGUI_EXPORT Q_DECL_EXPORT
#else
#  define MBGUI_EXPORT Q_DECL_IMPORT
#endif

#endif // MBGUI_GLOBAL_H
