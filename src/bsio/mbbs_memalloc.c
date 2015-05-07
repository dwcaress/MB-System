/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_memalloc.c	3/3/2014
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
 *	Copyright (c) 1992 by University of Hawaii.
 */

#include <string.h>
#include <stdlib.h>

#include "mbbs_defines.h"
#include "mbbs_mem.h"

/* maximum allocation size allowed (0 => no limit) */
static unsigned long mem_maxallocsz= 0;

#ifndef WIN32
static key_t mem_key= (key_t) IPC_PRIVATE;
#endif
static int mem_shmflag= 0666;

void
mbbs_memmaxalloc(unsigned long m)
{
	mem_maxallocsz= m;

	return;
}

int
mbbs_memalloc(MemType **buf, unsigned int *bufsz, unsigned int nobj, size_t objsz)
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
		CFree((MemType *) *buf);
	*bufsz= 0;

	/* allocate new memory */
	if ((*buf= (MemType *) calloc((MemSizeType) nobj, (MemSizeType) objsz)) == (MemType *) 0)
		return MEM_CALLOC;
	*bufsz= nobj;

	return MEM_SUCCESS;
}

#ifndef WIN32
int
mbbs_memallocsh(MemType **buf, int *shmid, unsigned int *bufsz, unsigned int nobj, size_t objsz)
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
#endif
