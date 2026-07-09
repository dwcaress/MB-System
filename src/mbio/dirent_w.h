/*--------------------------------------------------------------------
 *    The MB-system:  dirent_w.h
 *
 *    Minimal Windows / MSVC substitute for POSIX <dirent.h> — just the
 *    opendir / readdir / closedir subset MB-System utilities use, on top of
 *    the Win32 FindFirstFile API. Selected like unistd_w.h:
 *
 *        #ifdef _WIN32
 *        #include "dirent_w.h"
 *        #else
 *        #include <dirent.h>
 *        #endif
 *--------------------------------------------------------------------*/
#ifndef DIRENT_W_H
#define DIRENT_W_H

#if defined(_WIN32)

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef NAME_MAX
#define NAME_MAX 260
#endif

struct dirent {
    char d_name[NAME_MAX + 1];
};

typedef struct {
    HANDLE           h;
    WIN32_FIND_DATAA fd;
    int              first;
    struct dirent    de;
} DIR;

static __inline DIR *opendir(const char *path)
{
    if (!path || !*path) { errno = ENOENT; return NULL; }
    size_t n = strlen(path);
    char *pat = (char *)malloc(n + 3);
    if (!pat) { errno = ENOMEM; return NULL; }
    memcpy(pat, path, n);
    /* append "\*" (or "*" if the path already ends in a separator) */
    if (n && (path[n - 1] == '/' || path[n - 1] == '\\')) { pat[n] = '*'; pat[n + 1] = '\0'; }
    else { pat[n] = '\\'; pat[n + 1] = '*'; pat[n + 2] = '\0'; }
    DIR *d = (DIR *)calloc(1, sizeof(DIR));
    if (!d) { free(pat); errno = ENOMEM; return NULL; }
    d->h = FindFirstFileA(pat, &d->fd);
    free(pat);
    if (d->h == INVALID_HANDLE_VALUE) { free(d); errno = ENOENT; return NULL; }
    d->first = 1;
    return d;
}

static __inline struct dirent *readdir(DIR *d)
{
    if (!d) return NULL;
    if (d->first) d->first = 0;
    else if (!FindNextFileA(d->h, &d->fd)) return NULL;
    strncpy(d->de.d_name, d->fd.cFileName, NAME_MAX);
    d->de.d_name[NAME_MAX] = '\0';
    return &d->de;
}

static __inline int closedir(DIR *d)
{
    if (!d) return -1;
    if (d->h != INVALID_HANDLE_VALUE) FindClose(d->h);
    free(d);
    return 0;
}

#endif /* _WIN32 */

#endif /* DIRENT_W_H */
