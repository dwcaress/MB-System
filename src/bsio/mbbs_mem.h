/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_mem.h	3/3/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2015 by
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
 *	Copyright (c) 1998 by University of Hawaii.
 */

#ifndef __MBBS_MEM__
#define __MBBS_MEM__

#include <sys/types.h>
#ifndef WIN32
#include <sys/shm.h>
#endif

#define MEM_SUCCESS	(0)
#define MEM_BADARG	(1)
#define MEM_CALLOC	(2)
#define MEM_OOB		(3)
#define MEM_SHMGET	(4)
#define MEM_SHMATT	(5)
#define MEM_SHMDET	(6)
#define MEM_SHMRM	(7)

#define MEM_SHMNULLID	(-1)

#if defined(c_plusplus) || defined(__cplusplus)

extern "C" {

int		mbbs_memalloc(MemType **, unsigned int *, unsigned int, size_t);
int		mbbs_memallocsh(MemType **, int *, unsigned int *, unsigned int,
			   size_t);
void		mbbs_memmaxalloc(unsigned long);
void		mbbs_revbytes(void *, unsigned int);
void		mbbs_swapbytes(void *, unsigned int);

}

#else

extern int	mbbs_memalloc(MemType **, unsigned int *, unsigned int, size_t);
extern int	mbbs_memallocsh(MemType **, int *, unsigned int *, unsigned int,
			   size_t);
extern void	mbbs_memmaxalloc(unsigned long);
extern void	mbbs_revbytes(void *, unsigned int);
extern void	mbbs_swapbytes(void *, unsigned int);

#endif /* defined(c_plusplus) || defined(__cplusplus) */

#endif /* __MBBS_MEM__ */
