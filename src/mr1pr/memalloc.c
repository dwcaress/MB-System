/*--------------------------------------------------------------------
 *    The MB-system:	memalloc.c	3/7/2003
 *	$Id: memalloc.c,v 5.0 2003-03-11 19:09:14 caress Exp $
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
 * $Log: not supported by cvs2svn $
 *
 *
 *--------------------------------------------------------------------*/
/*
 *	Copyright (c) 1992 by University of Hawaii.
 */

/* Various system dependent defines */
#ifdef SUN
#define Free			(void) free
#define MemType			char
#define MemSizeType		unsigned int
#define MemCopy(m0, m1, n)	bcopy((char *) (m0), (char *) (m1), (int) (n))
#define MemZero(m, n)		bzero((char *) (m), (int) (n))
#else
#define Free			free
#define MemType			void
#define MemSizeType		size_t
#define MemCopy(m0, m1, n)	(void) memmove((void *) (m1), (void *) (m0), (size_t) (n))
#define MemZero(m, n)		(void) memset((void *) (m), (int) 0, (size_t) (n))
#endif

#include "mem.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* maximum allocation size allowed (0 => no limit) */
static unsigned long mem_maxallocsz= 0;

#ifndef WINNT
static key_t mem_key= (key_t) IPC_PRIVATE;
static int mem_shmflag= 0666;
#endif /* WINNT */

void
memmaxalloc(unsigned long m)
{
	mem_maxallocsz= m;

	return;
}

int
memalloc(MemType **buf, unsigned int *bufsz, unsigned int nobj, unsigned int objsz)
{
	if (buf == (MemType **) 0)
		return MEM_BADARG;
	if (bufsz == (unsigned int *) 0)
		return MEM_BADARG;

	/* sufficient memory already allocated? */
	if (*bufsz >= nobj) {
		if ((*buf != (MemType *) 0) && (nobj != 0) && (objsz != 0))
			MemZero(*buf, nobj*objsz);
		return MEM_SUCCESS;
	}

	if ((mem_maxallocsz != 0) && (mem_maxallocsz < (unsigned long) (nobj*objsz)))
		return MEM_OOB;

	/* free any existing memory */
	if (*buf != (MemType *) 0)
		free((MemType *) *buf);
	*bufsz= 0;

	/* allocate new memory */
	if ((*buf= (MemType *) calloc((MemSizeType) nobj, (MemSizeType) objsz)) == (MemType *) 0)
		return MEM_CALLOC;
	*bufsz= nobj;

	return MEM_SUCCESS;
}

#ifndef WINNT
int
memallocsh(MemType **buf, int *shmid, unsigned int *bufsz, unsigned int nobj, unsigned int objsz)
{
	if (buf == (MemType **) 0)
		return MEM_BADARG;
	if (shmid == (int *) 0)
		return MEM_BADARG;
	if (bufsz == (unsigned int *) 0)
		return MEM_BADARG;

	/* sufficient memory already allocated? */
	if (*bufsz >= nobj) {
		if ((*buf != (MemType *) 0) && (nobj != 0) && (objsz != 0))
			MemZero(*buf, nobj*objsz);
		return MEM_SUCCESS;
	}

	if ((mem_maxallocsz != 0) && (mem_maxallocsz < (unsigned long) (nobj*objsz)))
		return MEM_OOB;

	/* free any existing memory */
	if ((*buf != (MemType *) 0) && (*shmid != MEM_SHMNULLID)) {
		if (shmdt((MemType *) *buf) < 0)
			return MEM_SHMDET;
		*buf= (MemType *) 0;
		*bufsz= 0;
		if (shmctl(*shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
			*shmid= MEM_SHMNULLID;
			return MEM_SHMRM;
		}
	}
	*buf= (MemType *) 0;
	*bufsz= 0;
	*shmid= MEM_SHMNULLID;

	/* allocate new memory */
	if ((*shmid= shmget(mem_key, (MemSizeType) (nobj*objsz), mem_shmflag)) < 0) {
		*shmid= MEM_SHMNULLID;
		return MEM_SHMGET;
	}
	if ((*buf= (MemType *) shmat(*shmid, (MemType *) 0, (int) 0)) == (MemType *) -1) {
		(void) shmctl(*shmid, IPC_RMID, (struct shmid_ds *) 0);
		*shmid= MEM_SHMNULLID;
		return MEM_SHMATT;
	}
	*bufsz= nobj;

	return MEM_SUCCESS;
}
#endif /* WINNT */
