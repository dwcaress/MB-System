#include <stdbool.h>
#include "mxdebug-common.h"

// C output API
#ifndef WITHOUT_MXDEBUG

#define MX_INFO(fmt, ...) {mxd_mdprint(MXINFO, fmt, __VA_ARGS__);}
#define MX_WARN(fmt, ...) {mxd_mdprint(MXWARN, fmt, __VA_ARGS__);}
#define MX_DEBUG(fmt, ...) {mxd_mdprint(MXDEBUG, fmt, __VA_ARGS__);}
// TODO: make error print w/ func, line
#define MX_ERROR(fmt, ...) {mxd_mdprint(MXERROR, fmt, __VA_ARGS__);}

#define MX_INFO_MSG(fmt, ...) {mxd_mdprint(MXINFO, fmt);}
#define MX_WARN_MSG(fmt, ...) {mxd_mdprint(MXWARN, fmt);}
#define MX_DEBUG_MSG(fmt, ...) {mxd_mdprint(MXDEBUG, fmt);}
#define MX_ERROR_MSG(fmt, ...) {mxd_mdprint(MXERROR, fmt);}

// always
#define MX_PRINT(fmt, ...) {mxd_dprint(fmt, __VA_ARGS__);}
#define MX_MSG(fmt, ...) {mxd_dprint(fmt);}
// if module defined
#define MX_DPRINT(mid, fmt, ...) {mxd_mdprint(mid, fmt, __VA_ARGS__);}
#define MX_DMSG(mid, fmt, ...) {mxd_mdprint(mid, fmt);}
// if module level != 0
#define MX_MPRINT(mid, fmt, ...) {mxd_nzprint(mid, fmt, __VA_ARGS__);}
#define MX_MMSG(mid, fmt, ...) {mxd_nzprint(mid, fmt);}
// if module level <= N
#define MX_LPRINT(mid, N, fmt, ...) {mxd_geprint(mid, N, fmt, __VA_ARGS__);}
#define MX_LMSG(mid, N, fmt, ...) {mxd_geprint(mid, N, fmt);}
// if B is true
#define MX_BPRINT(B, fmt, ...) {if(B)mxd_dprint(fmt, __VA_ARGS__);}
#define MX_BMSG(B, fmt, ...) {if(B)mxd_dprint(fmt);}
// if module defined and B is true
#define MX_MBPRINT(mid, B, fmt, ...) {if(B)mxd_mdprint(mid, fmt, __VA_ARGS__);}
#define MX_MBMSG(mid, B, fmt, ...) {if(B)mxd_mdprint(mid, fmt);}
// trace
#define MX_TRACE()  {fprintf(stderr, "%s:%d\n", __func__, __LINE__);}
//#define MX_MTRACE(m)  {fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, m);}

//// a hack for iostreams
//#define MX_DCERR(sfmt) if(debug()){ std::cerr << sfmt; }
//#define MX_MCERR(m, sfmt) if(debug()){ std::cerr << std::endl << m << sfmt; }
//#define MX_CCERR(c, sfmt) if(c){ std::cerr << sfmt; }
//#define MX_DOSTREAM(os, sfmt) if(debug()){ os << sfmt; }
//#define MX_COSTREAM(os, c, sfmt) if(c){ os << sfmt; }

#else
#define MX_INFO(fmt, ...)
#define MX_WARN(fmt, ...)
#define MX_DEBUG(fmt, ...)
#define MX_ERROR(fmt, ...)

#define MX_PRINT(mid, fmt, ...)
#define MX_MSG(mid, fmt, ...)
#define MX_MPRINT(mid, fmt, ...)
#define MX_MMSG(mid, fmt, ...)
#define MX_LPRINT(mid, l, fmt, ...)
#define MX_LMSG(mid, l, fmt, ...)
#define MX_BPRINT(B, fmt, ...)
#define MX_BMSG(B, fmt, ...)
#define MX_MBPRINT(mid, B, fmt, ...)
#define MX_MBMSG(mid, B, fmt, ...)
#define MX_TRACE()
#define MX_MTRACE()

// a hack for iostreams
//#define MX_DCERR(sfmt)
//#define MX_CCERR(c, sfmt)
//#define MX_MCERR(m, sfmt)
//#define MX_DOSTREAM(os, sfmt)
//#define MX_COSTREAM(os,c, sfmt)

#endif

// C API
extern void mxd_setModule(int id, int level, bool suspend, const char *name);
extern void mxd_removeModule(int id);
extern void mxd_autoNewline(bool enable);
extern void mxd_release();
extern void mxd_destroy();
extern void mxd_show();
extern void mxd_fshow(FILE *fp, int indent);
extern char *mxd_name(int id);
extern void mxd_setName(int id, const char *name);
extern int mxd_level(int id);
extern void mxd_setLevel(int id, int level);
extern bool mxd_testModule(int id, int level);
extern bool mxd_nTestModule(int *id, int len, int level);
extern int mxd_size();
extern bool mxd_hasID(int id);
extern void mxd_suspend(int id, bool suspend);
extern void mxd_nSuspend(int *id, int len, bool suspend);
extern bool mxd_suspended(int id);
extern mx_module_t *mxd_save(int id);
extern void mxd_restore(int id, mx_module_t *src);
extern void mxd_dprint(const char *fmt, ...);
extern void mxd_mdprint(int id, const char *fmt, ...);
extern void mxd_geprint(int id, int lvl, const char *fmt, ...);
extern void mxd_nzprint(int id, const char *fmt, ...);
