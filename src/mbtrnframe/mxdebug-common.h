#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef MXDEBUG_CXX_H
#define MXDEBUG_CXX_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mx_module_s{
    int id;
    char *name;
    int level;
    bool suspend;
}mx_module_t;

typedef enum{
    MXMTHREAD = -17,
    MXMFRAME,
    MXMERR,
    MXMBBUF,
    MXMCBUF,
    MXMFILE,
    MXMLIST,
    MXMLOG,
    MXMMEM,
    MXMQUEUE,
    MXMSOCK,
    MXMSTATS,
    MXMTIME,
    MXERROR,
    MXWARN,
    MXDEBUG,
    MXINFO,
    MX_APP_RANGE = 0
}mx_id_t;

#define MXD_BOOL2CH(x) (x ? 'Y' : 'N')
// feature macros (may be compiled out)
// enabled by default
#ifndef WITHOUT_PTHREAD_MUTEX
#define MUTEX_INIT(pmtx,a) pthread_mutex_init(pmtx,a)
#define MUTEX_TRYLOCK(pmtx) pthread_mutex_trylock(pmtx)
#define MUTEX_LOCK(pmtx) pthread_mutex_lock(pmtx)
#define MUTEX_UNLOCK(pmtx) pthread_mutex_unlock(pmtx)
#define MUTEX_DESTROY(pmtx) pthread_mutex_destroy(pmtx)
#else
#define MUTEX_INIT(pmtx, a) 0 
#define MUTEX_TRYLOCK(pmtx)
#define MUTEX_LOCK(pmtx)
#define MUTEX_UNLOCK(pmtx)
#define MUTEX_DESTROY(pmtx)
#endif

#ifndef WITH_FLOCK
#define FILE_LOCK(f) flockfile(f)
#define FILE_UNLOCK(f) funlockfile(f)
#else
#define FILE_LOCK(f)
#define FILE_UNLOCK(f)
#endif

#ifdef __cplusplus
}
#endif

#endif
