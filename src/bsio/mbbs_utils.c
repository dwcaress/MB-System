/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_utils.c	3/3/2014
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
 *	Copyright (c) 2010 by University of Hawaii.
 */

/*
   utils.c --
   Higher-level utilities for Hawaii Mapping Research Group BS files.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "mbbs_defines.h"

extern int	mbbs_appendstr(char **, char *);
extern int	mbbs_copypng(int, XDR *, XDR *, int);
extern int	mbbs_freebsfmem(BSFile *);
extern int	mbbs_rdbsfhdr(BSFile *, XDR *);
extern int	mbbs_wrbsfhdr(BSFile *, XDR *);

int
mbbs_splitfile(char *dirnm, char *bsfnm0, char *bsfnm1, int pngid, char *logprefix)
{
	int chngdir, origdirfd, err;
	char tmpfilenm[80], prefix[80], newlogtail[120];
	char *lp, *tailstr;
	FILE *ifp, *ofp;
	XDR xdri, xdro;
	BSFile bsfi, bsfo;

	if (sizeof(int) < 4)
		return BS_BADARCH;

	if ((bsfnm0 == (char *) 0) ||
	    ((int) strlen(bsfnm0) == 0) ||
	    (bsfnm1 == (char *) 0) ||
	    ((int) strlen(bsfnm1) == 0) ||
	    (pngid < 0))
		return BS_BADARG;

	if ((dirnm == (char *) 0) ||
	    ((int) strlen(dirnm) == 0))
		chngdir= 0;
	else
		chngdir= 1;

	if (chngdir) {
		if ((origdirfd= open(".", O_RDONLY)) < 0)
			return BS_OPEN;
		if (chdir(dirnm) < 0) {
			(void) close(origdirfd);
			return BS_CHDIR;
		}
	}

	/* rename original file to temporary name */
	(void) strcpy(tmpfilenm, "BSLIBsplittmp");
	if (access(tmpfilenm, F_OK) == 0) {
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return BS_ACCESS;
	}
	if (rename(bsfnm0, tmpfilenm) < 0) {
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return BS_RENAME;
	}

	if ((ifp= fopen(tmpfilenm, "r")) == (FILE *) 0) {
		(void) rename(tmpfilenm, bsfnm0);
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return BS_OPEN;
	}
	xdrstdio_create(&xdri, ifp, XDR_DECODE);
	if ((err= mbbs_rdbsfhdr(&bsfi, &xdri)) != BS_SUCCESS) {
		(void) fclose(ifp);
		(void) rename(tmpfilenm, bsfnm0);
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return err;
	}
	if (pngid >= bsfi.bsf_count) {
		(void) mbbs_freebsfmem(&bsfi);
		(void) fclose(ifp);
		(void) rename(tmpfilenm, bsfnm0);
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return BS_BADARG;
	}
	MemCopy(&bsfi, &bsfo, sizeof(BSFile));

	/* these strings now belong to the output header! */
	bsfi.bsf_srcfilenm= (char *) 0;
	bsfi.bsf_log= (char *) 0;

	if ((logprefix == (char *) 0) ||
	    ((int) strlen(logprefix) == 0)) {
		(void) sprintf(prefix, "BSLIB::bs_split()");
		lp= prefix;
	}
	else if ((int) strlen(logprefix) > 50) {
		(void) strncpy(prefix, logprefix, 47);
		prefix[47]= '\0';
		(void) strcat(prefix, "...");
		lp= prefix;
	}
	else
		lp= logprefix;
	if ((int) strlen(bsfo.bsf_log) > 0)
		(void) sprintf(newlogtail, "\n%s [ BreakFile @ Ping%1d HEAD ] ;", lp, pngid);
	else
		(void) sprintf(newlogtail, "%s [ BreakFile @ Ping%1d HEAD ] ;", lp, pngid);
	if ((err= mbbs_appendstr(&(bsfo.bsf_log), newlogtail)) != BS_SUCCESS) {
		(void) mbbs_freebsfmem(&bsfo);
		(void) fclose(ifp);
		(void) rename(tmpfilenm, bsfnm0);
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return err;
	}

	/* eventually we will replace the "HEAD ] ;" substring just
	   appended to the log with "TAIL ] ;", so locate it now */
	tailstr= bsfo.bsf_log;
	tailstr+= (int) strlen(bsfo.bsf_log);
	for ( ; *tailstr != 'H'; tailstr--);

	/* copy first part of original file */
	if ((ofp= fopen(bsfnm0, "w")) == (FILE *) 0) {
		(void) mbbs_freebsfmem(&bsfo);
		(void) fclose(ifp);
		(void) rename(tmpfilenm, bsfnm0);
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return BS_OPEN;
	}
	xdrstdio_create(&xdro, ofp, XDR_ENCODE);
	bsfo.bsf_count= pngid;
	if ((err= mbbs_wrbsfhdr(&bsfo, &xdro)) != BS_SUCCESS) {
		(void) fclose(ofp);
		(void) mbbs_freebsfmem(&bsfo);
		(void) fclose(ifp);
		(void) rename(tmpfilenm, bsfnm0);
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return err;
	}
	if ((err= mbbs_copypng(bsfo.bsf_count, &xdri, &xdro, bsfi.bsf_version)) != BS_SUCCESS) {
		(void) fclose(ofp);
		(void) mbbs_freebsfmem(&bsfo);
		(void) fclose(ifp);
		(void) rename(tmpfilenm, bsfnm0);
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return err;
	}
	xdr_destroy(&xdro);
	(void) fclose(ofp);

	/* copy second part of original file */
	if ((ofp= fopen(bsfnm1, "w")) == (FILE *) 0) {
		(void) mbbs_freebsfmem(&bsfo);
		(void) fclose(ifp);
		(void) rename(tmpfilenm, bsfnm0);
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return BS_OPEN;
	}
	xdrstdio_create(&xdro, ofp, XDR_ENCODE);
	bsfo.bsf_count= bsfi.bsf_count-pngid;
	(void) strcpy(tailstr, "TAIL ] ;");
	if ((err= mbbs_wrbsfhdr(&bsfo, &xdro)) != BS_SUCCESS) {
		(void) fclose(ofp);
		(void) mbbs_freebsfmem(&bsfo);
		(void) fclose(ifp);
		(void) rename(tmpfilenm, bsfnm0);
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return err;
	}
	if ((err= mbbs_copypng(bsfo.bsf_count, &xdri, &xdro, bsfi.bsf_version)) != BS_SUCCESS) {
		(void) fclose(ofp);
		(void) mbbs_freebsfmem(&bsfo);
		(void) fclose(ifp);
		(void) rename(tmpfilenm, bsfnm0);
		if (chngdir) {
			(void) fchdir(origdirfd);
			(void) close(origdirfd);
		}
		return err;
	}
	xdr_destroy(&xdro);
	(void) fclose(ofp);

	(void) mbbs_freebsfmem(&bsfo);
	(void) fclose(ifp);
	(void) unlink(tmpfilenm);
	if (chngdir) {
		if (fchdir(origdirfd) != 0) {
			(void) close(origdirfd);
			return BS_CHDIR;
		}
		(void) close(origdirfd);
	}

	return BS_SUCCESS;
}
