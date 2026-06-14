/*
 * mb_win_compat.h - MSVC compatibility shim, force-included by CMake on WIN32+MSVC.
 *
 * Provides POSIX functions/macros missing from the MSVC runtime so that POSIX-
 * flavoured MB-System sources compile without #ifdef sprinkling in each file.
 *
 * Only compiled when _MSC_VER is defined. Safe to include from C and C++.
 */
#ifndef MB_WIN_COMPAT_H
#define MB_WIN_COMPAT_H

#if defined(_MSC_VER)

/* Expose M_PI, M_E etc from <math.h> — MSVC gates them behind this. */
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

/* Suppress Windows.h's min/max macros that clobber std::numeric_limits<>::max()
   and similar; trim rarely-used Win32 sub-APIs to speed parsing.
   NOGDI suppresses wingdi.h's ERROR macro (collides with cproj.h ERROR define). */
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOGDI
#define NOGDI
#endif

#include <time.h>     /* struct timespec (VS2015+) */
#include <sys/types.h>/* mode_t (POSIX), via _CRT_NONSTDC_NO_DEPRECATE */
#include <sys/stat.h> /* _stat / S_IFDIR / S_IFREG */
#include <direct.h>   /* _mkdir */
#include <BaseTsd.h>  /* SSIZE_T */
#include <stdbool.h>  /* bool / true / false for C TUs that forget to include it */
#include <stdint.h>   /* uint32_t / int64_t / etc */
#include <inttypes.h> /* PRIu32 / PRId64 / PRIx32 etc — POSIX format macros */
#include <stdio.h>    /* stderr / stdout / FILE */
#include <malloc.h>   /* _alloca */
#include <io.h>       /* _access, R_OK/W_OK constants live in unistd.h shim */
#include <winsock2.h> /* struct timeval, sockets — MUST precede windows.h on MSVC */
#include <ws2tcpip.h>
#include <windows.h>

/* POSIX alloca() — MSVC has _alloca (in malloc.h). Used as a portable
   stand-in for C99 variable-length arrays, which MSVC does not support. */
#ifndef alloca
#define alloca(n) _alloca(n)
#endif

/* POSIX ssize_t — MSVC has SSIZE_T (BaseTsd.h) but not the lowercase POSIX name. */
#ifndef _SSIZE_T_DEFINED
typedef SSIZE_T ssize_t;
#define _SSIZE_T_DEFINED
#endif

/* POSIX mode_t — UCRT's <sys/types.h> exposes mode_t inconsistently (depends
   on NO_OLDNAMES / _CRT_DECLARE_NONSTDC_NAMES). Provide a guarded typedef
   matching iowrap-posix.h's _MODE_T_DEFINED guard so the two don't collide. */
#ifndef _MODE_T_DEFINED
typedef int mode_t;
#define _MODE_T_DEFINED
#endif
/* pthreads-win32 semaphore.h gates its own mode_t typedef on HAVE_MODE_T;
   announce ours so it doesn't redefine. */
#ifndef HAVE_MODE_T
#define HAVE_MODE_T
#endif

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME  0
#endif
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* clock_gettime() emulation using Win32 high-precision time. */
static __inline int mb_win_clock_gettime(int clk_id, struct timespec *ts)
{
    (void)clk_id;
    FILETIME ft;
    ULARGE_INTEGER ull;
    /* 100-ns intervals between 1601-01-01 and 1970-01-01 Unix epoch. */
    const unsigned long long EPOCH_DIFF_100NS = 116444736000000000ULL;
    GetSystemTimePreciseAsFileTime(&ft);
    ull.LowPart  = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    unsigned long long t = ull.QuadPart - EPOCH_DIFF_100NS;
    ts->tv_sec  = (time_t)(t / 10000000ULL);
    ts->tv_nsec = (long)((t % 10000000ULL) * 100);
    return 0;
}

#define clock_gettime(clk, ts) mb_win_clock_gettime((clk), (ts))

/* POSIX stat-mode predicates (MSVC has only _S_IFDIR etc., no S_IS* macros). */
#ifndef S_ISDIR
#define S_ISDIR(m)  (((m) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(m)  (((m) & _S_IFMT) == _S_IFREG)
#endif
/* Windows has no symlink concept at stat-mode level; always false. */
#ifndef S_ISLNK
#define S_ISLNK(m)  (0)
#endif

/* POSIX getopt — prebuilt lib supplied via ConfigUser.cmake (GETOPT_LIBRARY).
   Force-included so utilities don't need explicit #include <getopt.h>. */
#include <getopt.h>

/* POSIX realpath(path, resolved) — MSVC has _fullpath with arg order
   (resolved, path, max_len). Wrap. NULL resolved => malloc'd buffer (PATH_MAX).*/
#ifndef realpath
#define realpath(path, resolved) _fullpath((resolved), (path), _MAX_PATH)
#endif

/* POSIX sleep(seconds) — Windows has Sleep(milliseconds) (note capital S). */
#ifndef sleep
#define sleep(s) Sleep((s) * 1000)
#endif
#ifndef usleep
#define usleep(us) Sleep((us) / 1000)
#endif

/* POSIX popen / pclose — MSVC has _popen / _pclose with same signatures. */
#ifndef popen
#define popen  _popen
#endif
#ifndef pclose
#define pclose _pclose
#endif

/* POSIX strcasecmp / strncasecmp — MSVC has _stricmp / _strnicmp. */
#include <string.h>
#ifndef strcasecmp
#define strcasecmp  _stricmp
#endif
#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

/* POSIX strtok_r — MSVC has strtok_s with same signature semantics. */
#ifndef strtok_r
#define strtok_r(str, delim, saveptr) strtok_s((str), (delim), (saveptr))
#endif

/* POSIX file-open flags missing from MSVC <fcntl.h>: NONBLOCK/SYNC/DSYNC.
   Windows file I/O is blocking + synchronous by default; treat as no-op. */
#include <fcntl.h>
#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif
#ifndef O_SYNC
#define O_SYNC 0
#endif
#ifndef O_DSYNC
#define O_DSYNC 0
#endif

/* access() mode flags — already in unistd.h stub but define here too so TUs
   that pull mb_win_compat.h via /FI but never #include <unistd.h> still see them. */
#ifndef F_OK
#define F_OK 00
#define R_OK 04
#define W_OK 02
#define X_OK 01
#endif

/* fcntl() commands — only matter for socket non-blocking toggling; not used at
   build time, just need to parse. Stub as 0 so source compiles; runtime call
   would need ioctlsocket replacement, but most call paths are gated to POSIX. */
#ifndef F_GETFL
#define F_GETFL 0
#endif
#ifndef F_SETFL
#define F_SETFL 0
#endif

/* POSIX stdio recursive lock — Windows CRT has _lock_file / _unlock_file. */
#ifndef flockfile
#define flockfile(f)   _lock_file(f)
#endif
#ifndef funlockfile
#define funlockfile(f) _unlock_file(f)
#endif

/* MSG_NOSIGNAL — POSIX flag to send() that suppresses SIGPIPE. Windows has no
   SIGPIPE, so 0 is correct. */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/* SO_REUSEPORT — Linux extension; map to SO_REUSEADDR on Windows. */
#ifndef SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

/* MSG_DONTWAIT — POSIX flag for non-blocking I/O. Winsock has no equivalent;
   the right way is ioctlsocket(FIONBIO). Map to 0 so code compiles; callers
   that want non-blocking must set it via msock_set_blocking() instead. */
#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

/* O_NOCTTY — POSIX open flag (don't acquire controlling tty); no-op on Win. */
#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif

/* SIGSTOP / SIGTSTP / SIGCONT — POSIX job-control signals; not on Win. */
#ifndef SIGSTOP
#define SIGSTOP 19
#endif
#ifndef SIGTSTP
#define SIGTSTP 20
#endif
#ifndef SIGCONT
#define SIGCONT 18
#endif

/* struct timezone — legacy POSIX (gettimeofday 2nd arg); MSVC has no definition. */
#ifndef _STRUCT_TIMEZONE_DEFINED
#define _STRUCT_TIMEZONE_DEFINED
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif

/* gettimeofday — POSIX; not in MSVC runtime. Use GetSystemTimePreciseAsFileTime. */
static __inline int mb_win_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME ft;
    ULARGE_INTEGER ui;
    /* Windows epoch (1601-01-01) to Unix epoch (1970-01-01) = 11644473600 sec. */
    static const unsigned long long EPOCH_DIFF = 116444736000000000ULL;
    if (tv) {
        GetSystemTimePreciseAsFileTime(&ft);
        ui.LowPart = ft.dwLowDateTime;
        ui.HighPart = ft.dwHighDateTime;
        ui.QuadPart -= EPOCH_DIFF;
        tv->tv_sec  = (long)(ui.QuadPart / 10000000ULL);
        tv->tv_usec = (long)((ui.QuadPart / 10ULL) % 1000000ULL);
    }
    if (tz) {
        tz->tz_minuteswest = 0;
        tz->tz_dsttime = 0;
    }
    return 0;
}
#define gettimeofday(tv, tz) mb_win_gettimeofday((tv), (tz))

/* Winsock setsockopt/getsockopt take const char* for optval; POSIX takes
   const void*. Wrap with cast so POSIX-flavoured call sites compile. The
   wrapper bodies are defined BEFORE the macros so they call the real WS2 fn. */
static __inline int mb_win_setsockopt(SOCKET s, int level, int optname, const void *optval, int optlen)
{
    return setsockopt(s, level, optname, (const char *)optval, optlen);
}
static __inline int mb_win_getsockopt(SOCKET s, int level, int optname, void *optval, int *optlen)
{
    return getsockopt(s, level, optname, (char *)optval, optlen);
}
#define setsockopt(s, lev, opt, val, len)  mb_win_setsockopt((s), (lev), (opt), (val), (len))
#define getsockopt(s, lev, opt, val, len)  mb_win_getsockopt((s), (lev), (opt), (val), (len))

/* POSIX shutdown how-flags — winsock has SD_RECEIVE/SD_SEND/SD_BOTH. */
#ifndef SHUT_RD
#define SHUT_RD   SD_RECEIVE
#endif
#ifndef SHUT_WR
#define SHUT_WR   SD_SEND
#endif
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif

/* POSIX signals — Windows <signal.h> has SIGINT/TERM/etc but no SIGHUP/SIGPIPE,
   and no struct sigaction. Provide minimal shim so signal-setup boilerplate
   in mbtrn utilities parses (sigaction() forwards to signal()). */
#include <signal.h>
#ifndef SIGHUP
#define SIGHUP  1
#endif
#ifndef SIGQUIT
#define SIGQUIT 3
#endif
#ifndef SIGKILL
#define SIGKILL 9
#endif
#ifndef SIGPIPE
#define SIGPIPE 13
#endif
#ifndef SIGALRM
#define SIGALRM 14
#endif
#ifndef SIGCHLD
#define SIGCHLD 17
#endif
#ifndef SIGUSR1
#define SIGUSR1 30
#endif
#ifndef SIGUSR2
#define SIGUSR2 31
#endif

#ifndef MB_WIN_SIGACTION_DEFINED
#define MB_WIN_SIGACTION_DEFINED
typedef int sigset_t;
struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
};
static __inline int sigemptyset(sigset_t *s)             { if (s) *s = 0; return 0; }
static __inline int sigfillset(sigset_t *s)              { if (s) *s = ~0; return 0; }
static __inline int sigaddset(sigset_t *s, int n)        { (void)n; if (s) *s |= 1; return 0; }
static __inline int sigdelset(sigset_t *s, int n)        { (void)n; if (s) *s &= ~1; return 0; }
static __inline int sigismember(const sigset_t *s, int n){ (void)n; return s ? (*s & 1) : 0; }
static __inline int sigaction(int sig, const struct sigaction *act, struct sigaction *old)
{
    (void)old;
    if (act && act->sa_handler) signal(sig, act->sa_handler);
    return 0;
}
#endif

/* POSIX setenv — MSVC has _putenv_s with (name, value); ignore overwrite. */
#include <stdlib.h>
static __inline int mb_win_setenv(const char *name, const char *value, int overwrite)
{
    (void)overwrite;
    if (!name || !value) return -1;
    return _putenv_s(name, value);
}
#define setenv(n, v, o) mb_win_setenv((n), (v), (o))
#define unsetenv(n)     _putenv_s((n), "")

/* POSIX readlink — no Windows symlink semantics worth emulating; stub to -1. */
static __inline long mb_win_readlink_stub(const char *path, char *buf, unsigned long bufsiz)
{
    (void)path; (void)buf; (void)bufsiz;
    return -1;
}
#define readlink(p, b, n) mb_win_readlink_stub((p), (b), (n))

/* POSIX mode-bit constants. MSVC defines only _S_IREAD/_S_IWRITE and a few
   _S_I* aliases; the POSIX names (and group/other variants) are missing.
   Map missing ones onto user bits — Windows ACLs ignore group/other anyway. */
#ifndef S_IRUSR
#define S_IRUSR _S_IREAD
#endif
#ifndef S_IWUSR
#define S_IWUSR _S_IWRITE
#endif
#ifndef S_IXUSR
#define S_IXUSR 0
#endif
#ifndef S_IRWXU
#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#endif
#ifndef S_IRGRP
#define S_IRGRP 0
#endif
#ifndef S_IWGRP
#define S_IWGRP 0
#endif
#ifndef S_IXGRP
#define S_IXGRP 0
#endif
#ifndef S_IRWXG
#define S_IRWXG 0
#endif
#ifndef S_IROTH
#define S_IROTH 0
#endif
#ifndef S_IWOTH
#define S_IWOTH 0
#endif
#ifndef S_IXOTH
#define S_IXOTH 0
#endif
#ifndef S_IRWXO
#define S_IRWXO 0
#endif

/* localtime_r(time_t*, struct tm*) -> MSVC has localtime_s(struct tm*, time_t*) with swapped args. */
#define localtime_r(tp, tmp)  (localtime_s((tmp), (tp)) == 0 ? (tmp) : NULL)
#define gmtime_r(tp, tmp)     (gmtime_s((tmp), (tp))    == 0 ? (tmp) : NULL)

/* POSIX mkdir(path, mode); MSVC has _mkdir(path) only. Ignore mode on Windows. */
#define mkdir(path, mode)  _mkdir(path)

/* POSIX file ops missing on MSVC — implement via Win32 / CRT equivalents.
   Used by mbtrnframe (mfile.c, mtime.c, msocket.c) and a few utilities. */
#include <stdarg.h>
static __inline int mb_win_ftruncate(int fd, long long length)
{
    return _chsize_s(fd, length) == 0 ? 0 : -1;
}
#define ftruncate(fd, len) mb_win_ftruncate((fd), (long long)(len))

static __inline int mb_win_fsync(int fd) { return _commit(fd); }
#define fsync(fd) mb_win_fsync(fd)

static __inline int mb_win_vdprintf(int fd, const char *fmt, va_list ap)
{
    char buf[4096];
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    if (n < 0) return -1;
    if (n > (int)sizeof(buf)) n = (int)sizeof(buf);
    return _write(fd, buf, (unsigned)n);
}
#define vdprintf(fd, fmt, ap) mb_win_vdprintf((fd), (fmt), (ap))

/* fcntl — only used for nonblocking sockets in mbtrn code; ioctlsocket would be
   the proper replacement but call sites tolerate failure. Stub to 0. */
static __inline int mb_win_fcntl_stub(int fd, int cmd, ...) { (void)fd; (void)cmd; return 0; }
#define fcntl mb_win_fcntl_stub

/* nanosleep — millisecond resolution via Win32 Sleep. */
static __inline int mb_win_nanosleep(const struct timespec *req, struct timespec *rem)
{
    (void)rem;
    if (!req) return -1;
    DWORD ms = (DWORD)(req->tv_sec * 1000ULL + req->tv_nsec / 1000000ULL);
    Sleep(ms);
    return 0;
}
#define nanosleep(req, rem) mb_win_nanosleep((req), (rem))

/* clock_getres / clock_setres — report 1 ns resolution; setres no-op. */
static __inline int mb_win_clock_getres(int clk, struct timespec *ts)
{
    (void)clk;
    if (ts) { ts->tv_sec = 0; ts->tv_nsec = 1; }
    return 0;
}
#define clock_getres(clk, ts) mb_win_clock_getres((clk), (ts))

static __inline int mb_win_clock_setres(int clk, struct timespec *ts)
{
    (void)clk; (void)ts; return 0;
}
#define clock_setres(clk, ts) mb_win_clock_setres((clk), (ts))

/* Minimal POSIX strptime — handles the format specifiers used by mbtrn
   (%Y %j %H %M %S %m %d). Returns pointer past last char consumed, or NULL. */
static __inline char *mb_win_strptime(const char *s, const char *fmt, struct tm *tm)
{
    while (*fmt && *s) {
        if (*fmt == '%' && fmt[1]) {
            int val = 0, n = 0;
            char c = fmt[1];
            if (c == 'Y') { if (sscanf(s, "%4d%n", &val, &n) != 1) return NULL; tm->tm_year = val - 1900; }
            else if (c == 'm') { if (sscanf(s, "%2d%n", &val, &n) != 1) return NULL; tm->tm_mon  = val - 1; }
            else if (c == 'd') { if (sscanf(s, "%2d%n", &val, &n) != 1) return NULL; tm->tm_mday = val; }
            else if (c == 'H') { if (sscanf(s, "%2d%n", &val, &n) != 1) return NULL; tm->tm_hour = val; }
            else if (c == 'M') { if (sscanf(s, "%2d%n", &val, &n) != 1) return NULL; tm->tm_min  = val; }
            else if (c == 'S') { if (sscanf(s, "%2d%n", &val, &n) != 1) return NULL; tm->tm_sec  = val; }
            else if (c == 'j') {
                if (sscanf(s, "%3d%n", &val, &n) != 1) return NULL;
                tm->tm_yday = val - 1;
                /* Convert day-of-year → month/day (needed by mktime). */
                static const int mdays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
                int year = tm->tm_year + 1900;
                int leap = (year%4==0 && (year%100!=0 || year%400==0)) ? 1 : 0;
                int d = val, m = 0;
                while (m < 12) {
                    int dim = mdays[m] + ((m==1) ? leap : 0);
                    if (d <= dim) break;
                    d -= dim; m++;
                }
                tm->tm_mon = m; tm->tm_mday = d;
            }
            else return NULL;
            s += n; fmt += 2;
        } else if (*fmt == *s || *fmt == ' ') {
            while (*fmt == ' ') fmt++;
            while (*s == ' ') s++;
            if (*fmt == *s) { fmt++; s++; }
        } else return NULL;
    }
    return (char *)s;
}
#define strptime(s, fmt, tm) mb_win_strptime((s), (fmt), (tm))

/* strsignal — POSIX; not in MSVC runtime. Return signal number as static string. */
static __inline const char *mb_win_strsignal(int sig)
{
    static char buf[32];
    _snprintf_s(buf, sizeof(buf), _TRUNCATE, "signal %d", sig);
    return buf;
}
#define strsignal(sig) mb_win_strsignal(sig)

/* No symlinks on plain Windows builds. lstat == stat; symlink() not supported. */
#define lstat(path, sb)    stat((path), (sb))
static __inline int mb_win_symlink_unsupported(const char *target, const char *linkpath)
{
    (void)target; (void)linkpath;
    return -1;  /* errno left untouched; caller treats nonzero as failure */
}
#define symlink(t, l)  mb_win_symlink_unsupported((t), (l))

#ifdef __cplusplus
}
#endif

#endif /* _MSC_VER */

#endif /* MB_WIN_COMPAT_H */
