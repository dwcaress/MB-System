/*--------------------------------------------------------------------
 *    The MB-system:	mr1pr.h				3/7/2003
 *	$Id: mr1pr.h,v 5.0 2003/03/11 19:09:14 caress Exp $
 *
 *    Copyright (c) 2003 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* This source code is part of the MR1PR library used to read and write
 * swath sonar data in the MR1PR format devised and used by the 
 * Hawaii Mapping Research Group of the University of Hawaii.
 * This source code was made available by Roger Davis of the
 * University of Hawaii under the GPL. Minor modifications have
 * been made to the version distributed here as part of MB-System.
 *
 * Author:	Roger Davis (primary author)
 * Author:	D. W. Caress (MB-System revisions)
 * Date:	March 7, 2003 (MB-System revisions)
 * $Log: mr1pr.h,v $
 * Revision 5.0  2003/03/11 19:09:14  caress
 * Initial version.
 *
 *
 *
 *--------------------------------------------------------------------*/
/*
 *	Copyright (c) 1991 by University of Hawaii.
 */

/*
 *	mr1pr.h --
 *	Hawaii MR1 post-processing software definitions.
 */

#ifndef __MR1PR__
#define __MR1PR__

#include "mr1pr_defines.h"

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {

int			mr1_appendlog(MR1File *, char **);
int			mr1_appendstr(char **, char *);
int			mr1_copypng(int, XDR *, XDR *, int);
int			mr1_getpngdataptrs(Ping *, float *,
				float **, float **, float **, float **,
				float **, float **, float **, float **);
int			mr1_isnand(double);
int			mr1_isnanf(float);
int			mr1_mrkget(void *, int, int);
void *			mr1_mrkmemalloc(int);
void			mr1_mrkset(void *, int, int, int);
double			mr1_nand();
float			mr1_nanf();
unsigned int		mr1_pngdatabufsz(Ping *);
float *			mr1_pngmemalloc(Ping *);
int			mr1_pngrealloc(Ping *, float **, unsigned int *);
int			mr1_rdmrfhdr(MR1File *, XDR *);
int			mr1_rdpng(Ping *, float **, XDR *, int);
int			mr1_rdpngdata(Ping *, float *, XDR *);
int			mr1_rdpnghdr(Ping *, XDR *, int);
int			mr1_replacestr(char **, char *);
int			mr1_seekpng(int, XDR *, int);
int			mr1_seekpngdata(Ping *, XDR *);
int			mr1_tmparse(char *, int, double *);
int			mr1_wrmrfhdr(MR1File *, XDR *);
int			mr1_wrpng(Ping *, float *, XDR *);
int			mr1_wrpngdata(Ping *, float *, XDR *);
int			mr1_wrpnghdr(Ping *, XDR *);

}

#else

extern int		mr1_appendlog(MR1File *, char **);
extern int		mr1_appendstr(char **, char *);
extern int		mr1_copypng(int, XDR *, XDR *, int);
extern int		mr1_getpngdataptrs(Ping *, float *,
				float **, float **, float **, float **,
				float **, float **, float **, float **);
extern int		mr1_isnand(double);
extern int		mr1_isnanf(float);
extern int		mr1_mrkget(void *, int, int);
extern void *		mr1_mrkmemalloc(int);
extern void		mr1_mrkset(void *, int, int, int);
extern double		mr1_nand();
extern float		mr1_nanf();
extern unsigned int	mr1_pngdatabufsz(Ping *);
extern float *		mr1_pngmemalloc(Ping *);
extern int		mr1_pngrealloc(Ping *, float **, unsigned int *);
extern int		mr1_rdmrfhdr(MR1File *, XDR *);
extern int		mr1_rdpng(Ping *, float **, XDR *, int);
extern int		mr1_rdpngdata(Ping *, float *, XDR *);
extern int		mr1_rdpnghdr(Ping *, XDR *, int);
extern int		mr1_replacestr(char **, char *);
extern int		mr1_seekpng(int, XDR *, int);
extern int		mr1_seekpngdata(Ping *, XDR *);
/*extern int		mr1_tmparse(char *, int, double *);*/
extern int		mr1_wrmrfhdr(MR1File *, XDR *);
extern int		mr1_wrpng(Ping *, float *, XDR *);
extern int		mr1_wrpngdata(Ping *, float *, XDR *);
extern int		mr1_wrpnghdr(Ping *, XDR *);

#endif /* defined(c_plusplus) || defined(__cplusplus) */

#endif /* __MR1PR__ */
