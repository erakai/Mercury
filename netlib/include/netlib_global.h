#ifndef NETLIB_GLOBAL_H
#define NETLIB_GLOBAL_H

#include <QTCore/QtGlobal>

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
  #if defined(NETLIB_LIBRARY)
    #define NETLIB_EXPORT Q_DECL_EXPORT
  #else
    #define NETLIB_EXPORT Q_DECL_IMPORT
  #endif
#else
  #define NETLIB_EXPORT
#endif

#endif // NETLIB_GLOBAL_H
