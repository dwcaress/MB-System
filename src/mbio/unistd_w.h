/*--------------------------------------------------------------------
 *    The MB-system:  unistd_w.h
 *
 *    Windows / MSVC substitute for POSIX <unistd.h>. MSVC ships no
 *    <unistd.h>; this provides the small subset MB-System sources rely on.
 *    Selected by mb_define.h (and at each call site):
 *
 *        #ifdef _WIN32
 *        #include "unistd_w.h"
 *        #else
 *        #include <unistd.h>
 *        #endif
 *
 *    The broader POSIX compatibility shim (clock_gettime, strcasecmp, sockets,
 *    sigaction, getopt, etc.) lives in mb_define.h under #ifdef _WIN32.
 *--------------------------------------------------------------------*/
#ifndef UNISTD_W_H
#define UNISTD_W_H

#include <direct.h>
#include <process.h>
#include <io.h>

#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif

#ifdef _WIN32
#  ifndef R_OK
#    define R_OK 04
#    define W_OK 02
#    define X_OK 01
#    define F_OK 00
#  endif
#endif

#endif /* UNISTD_W_H */
