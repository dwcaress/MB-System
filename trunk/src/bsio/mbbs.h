/*--------------------------------------------------------------------
 *    The MB-system:	mbbs.h	3/3/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2014 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* This source code is part of the mbbsio library used to read and write
 * swath sonar data in the bsio format devised and used by the
 * Hawaii Mapping Research Group of the University of Hawaii.
 * This source code was made available by Roger Davis of the
 * University of Hawaii under the GPL. Minor modifications have
 * been made to the version distributed here as part of MB-System.
 *
 * Author:	Roger Davis (primary author)
 * Author:	D. W. Caress (MB-System revisions)
 * Date:	March 3, 2014 (MB-System revisions)
 *
 *--------------------------------------------------------------------*/
/*
 *	Copyright (c) 2005 by University of Hawaii.
 */

/*
 *	bs.h --
 *	Hawaii Mapping Research Group BS file processing definitions.
 */

#ifndef __MBBS__
#define __MBBS__

#include "mbbs_defines.h"

#define mbbs_pngvisible(f)	((int) (!(((unsigned int) f) & PNG_HIDE) && \
				        !(((unsigned int) f) & PNG_LOWQUALITY)))
#define mbbs_pngmscvisible(f)	((int) (!(((unsigned int) f) & PNG_MSCHIDE) && \
					!(((unsigned int) f) & PNG_HIDE) && \
				        !(((unsigned int) f) & PNG_LOWQUALITY)))

extern unsigned long mbbs_iobytecnt;

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {

int			mbbs_appendlog(BSFile *, char **);
int			mbbs_appendstr(char **, char *);
void			mbbs_cal2jul(struct tm *);
int			mbbs_copypng(int, XDR *, XDR *, int);
int			mbbs_freebsfmem(BSFile *);
int			mbbs_getpngdataptrs(Ping *, MemType *, PingData *);
int			mbbs_isnand(double);
int			mbbs_isnanf(float);
void			mbbs_jul2cal(struct tm *);
int			mbbs_leapyr(struct tm *);
int			mbbs_mrkget(void *, int, int);
void *			mbbs_mrkmemalloc(int);
void			mbbs_mrkset(void *, int, int, int);
double			mbbs_nand();
float			mbbs_nanf();
int			mbbs_pngdatabufsz(Ping *, unsigned long long *);
MemType *		mbbs_pngmemalloc(Ping *);
int			mbbs_pngrealloc(Ping *, MemType **, unsigned int *);
int			mbbs_rdbsfhdr(BSFile *, XDR *);
int			mbbs_rdpng(Ping *, MemType **, XDR *, int);
int			mbbs_rdpngdata(Ping *, MemType *, XDR *);
int			mbbs_rdpnghdr(Ping *, XDR *, int);
int			mbbs_rdpngpddata(Ping *, PingData *, XDR *);
int			mbbs_rdversion(FILE *, int *);
int			mbbs_replacestr(char **, char *);
int			mbbs_seekpng(int, XDR *, int);
int			mbbs_seekpngdata(Ping *, XDR *);
int			mbbs_setgmttz();
int			mbbs_setswradius(int, FILE *, long, int, unsigned int, float);
int			mbbs_splitfile(char *, char *, char *, int, char *);
int			mbbs_striptail(char *, char);
int			mbbs_tmparse(char *, int, double *);
int			mbbs_tmparsegmttz(char *, int, double *);
int			mbbs_wrbsfhdr(BSFile *, XDR *);
int			mbbs_wrfflagsclrbits(FILE *, unsigned int);
int			mbbs_wrfflagssetbits(FILE *, unsigned int);
int			mbbs_wrpflagsclrbits(int, FILE *, long, unsigned int);
int			mbbs_wrpflags(int, FILE *, long, unsigned int);
int			mbbs_wrpflagssetbits(int, FILE *, long, unsigned int);
int			mbbs_wrpng(Ping *, MemType *, XDR *);
int			mbbs_wrpngdata(Ping *, MemType *, XDR *);
int			mbbs_wrpngpddata(Ping *, PingData *, XDR *);
int			mbbs_wrpnghdr(Ping *, XDR *);
int			mbbs_wrsllc(int, FILE *, long, double, double, float);
int			mbbs_wrtll(int, FILE *, long, double, double);
int			mbbs_wrtllc(int, FILE *, long, double, double, float);
int			mbbs_xdrstring(XDR *, char **, unsigned long *);

}

#else

extern int		mbbs_appendlog(BSFile *, char **);
extern int		mbbs_appendstr(char **, char *);
extern void		mbbs_cal2jul(struct tm *);
extern int		mbbs_copypng(int, XDR *, XDR *, int);
extern int		mbbs_freebsfmem(BSFile *);
extern int		mbbs_getpngdataptrs(Ping *, MemType *, PingData *);
extern int		mbbs_isnand(double);
extern int		mbbs_isnanf(float);
extern void		mbbs_jul2cal(struct tm *);
extern int		mbbs_leapyr(struct tm *);
extern int		mbbs_mrkget(void *, int, int);
extern void *		mbbs_mrkmemalloc(int);
extern void		mbbs_mrkset(void *, int, int, int);
extern double		mbbs_nand();
extern float		mbbs_nanf();
extern int		mbbs_pngdatabufsz(Ping *, unsigned long long *);
extern MemType *	mbbs_pngmemalloc(Ping *);
extern int		mbbs_pngrealloc(Ping *, MemType **, unsigned int *);
extern int		mbbs_rdbsfhdr(BSFile *, XDR *);
extern int		mbbs_rdpng(Ping *, MemType **, XDR *, int);
extern int		mbbs_rdpngdata(Ping *, MemType *, XDR *);
extern int		mbbs_rdpnghdr(Ping *, XDR *, int);
extern int		mbbs_rdpngpddata(Ping *, PingData *, XDR *);
extern int		mbbs_rdversion(FILE *, int *);
extern int		mbbs_replacestr(char **, char *);
extern int		mbbs_seekpng(int, XDR *, int);
extern int		mbbs_seekpngdata(Ping *, XDR *);
extern int		mbbs_setgmttz();
extern int		mbbs_setswradius(int, FILE *, long, int, unsigned int, float);
extern int		mbbs_splitfile(char *, char *, char *, int, char *);
extern int		mbbs_striptail(char *, char);
extern int		mbbs_tmparse(char *, int, double *);
extern int		mbbs_tmparsegmttz(char *, int, double *);
extern int		mbbs_wrbsfhdr(BSFile *, XDR *);
extern int		mbbs_wrfflagsclrbits(FILE *, unsigned int);
extern int		mbbs_wrfflagssetbits(FILE *, unsigned int);
extern int		mbbs_wrpflagsclrbits(int, FILE *, long, unsigned int);
extern int		mbbs_wrpflags(int, FILE *, long, unsigned int);
extern int		mbbs_wrpflagssetbits(int, FILE *, long, unsigned int);
extern int		mbbs_wrpng(Ping *, MemType *, XDR *);
extern int		mbbs_wrpngdata(Ping *, MemType *, XDR *);
extern int		mbbs_wrpnghdr(Ping *, XDR *);
extern int		mbbs_wrpngpddata(Ping *, PingData *, XDR *);
extern int		mbbs_wrsllc(int, FILE *, long, double, double, float);
extern int		mbbs_wrtll(int, FILE *, long, double, double);
extern int		mbbs_wrtllc(int, FILE *, long, double, double, float);
extern int		mbbs_xdrstring(XDR *, char **, unsigned long *);

#endif /* defined(c_plusplus) || defined(__cplusplus) */

#endif /* __MBBS__ */
