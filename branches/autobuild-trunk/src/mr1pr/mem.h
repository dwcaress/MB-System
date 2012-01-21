/*--------------------------------------------------------------------
 *    The MB-system:	mem.h			3/7/2003
 *	$Id: mem.h 1770 2009-10-19 17:16:39Z caress $
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
 * $Log: mem.h,v $
 * Revision 5.1  2003/04/17 21:12:03  caress
 * Release 5.0.beta30
 *
 * Revision 5.0  2003/03/11 19:09:14  caress
 * Initial version.
 *
 *
 *
 *--------------------------------------------------------------------*/
/*
 *	Copyright (c) 1998 by University of Hawaii.
 */

#ifndef __MEM__
#define __MEM__

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

int		memalloc(MemType **, unsigned int *, unsigned int, unsigned int);
int		memallocsh(MemType **, int *, unsigned int *, unsigned int,
			   unsigned int);
void		memmaxalloc(unsigned long);
/*void		swapbytes(void *, unsigned int);*/

}

#else

extern int	memalloc(MemType **, unsigned int *, unsigned int, unsigned int);
extern int	memallocsh(MemType **, int *, unsigned int *, unsigned int,
			   unsigned int);
extern void	memmaxalloc(unsigned long);
extern void	swapbytes(void *, unsigned int);

#endif /* defined(c_plusplus) || defined(__cplusplus) */

#endif /* __MEM__ */
