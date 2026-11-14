/*--------------------------------------------------------------------
 *    The MB-system:  mbv_marks.c  11/14/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Change marks of a view's terrain (see mbv_marks.h). This file uses no
 * toolkit and no OpenGL.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

#include "mbv_marks.h"

/*------------------------------------------------------------------------------*/
/* the blocks a range of nodes falls in, both ends included; false if the range
   or the block grid is empty */
static bool mbv_marks_range(const struct mbv_marks *marks, int i0, int i1, int j0, int j1, int block[4]) {
	if (marks->block == NULL || marks->nblock_i <= 0 || marks->nblock_j <= 0)
		return (false);
	if (i0 > i1 || j0 > j1)
		return (false);
	if (i0 < 0)
		i0 = 0;
	if (j0 < 0)
		j0 = 0;
	if (i1 < 0 || j1 < 0)
		return (false);
	block[0] = i0 / MBV_MARKS_BLOCK;
	block[1] = i1 / MBV_MARKS_BLOCK;
	block[2] = j0 / MBV_MARKS_BLOCK;
	block[3] = j1 / MBV_MARKS_BLOCK;
	if (block[0] >= marks->nblock_i || block[2] >= marks->nblock_j)
		return (false);
	if (block[1] >= marks->nblock_i)
		block[1] = marks->nblock_i - 1;
	if (block[3] >= marks->nblock_j)
		block[3] = marks->nblock_j - 1;
	return (true);
}
/*------------------------------------------------------------------------------*/
void mbv_marks_init(struct mbv_marks *marks) {
	memset(marks, 0, sizeof(*marks));
}
/*------------------------------------------------------------------------------*/
int mbv_marks_setsize(int verbose, struct mbv_marks *marks, int n_columns, int n_rows, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       n_columns:        %d\n", n_columns);
		fprintf(stderr, "dbg2       n_rows:           %d\n", n_rows);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* the versions of the old blocks mean nothing for a new grid, and the block
	   grid itself changes size, so the array is replaced; every user rebuilds
	   from the marks raised below */
	if (marks->block != NULL)
		mb_freed(verbose, __FILE__, __LINE__, (void **)&marks->block, error);
	marks->block = NULL;
	marks->nblock_i = 0;
	marks->nblock_j = 0;

	const int nblock_i = (n_columns > 0) ? (n_columns + MBV_MARKS_BLOCK - 1) / MBV_MARKS_BLOCK : 0;
	const int nblock_j = (n_rows > 0) ? (n_rows + MBV_MARKS_BLOCK - 1) / MBV_MARKS_BLOCK : 0;
	const size_t nblock = (size_t)nblock_i * (size_t)nblock_j;
	if (nblock > 0) {
		status = mb_mallocd(verbose, __FILE__, __LINE__, nblock * sizeof(uint64_t), (void **)&marks->block, error);
		/* mb_mallocd leaves the pointer NULL when it cannot allocate */
		if (status != MB_SUCCESS || marks->block == NULL) {
			marks->block = NULL;
			*error = MB_ERROR_MEMORY_FAIL;
			status = MB_FAILURE;
		}
		else {
			/* fresh memory is not zero, and a stale version read as unchanged
			   would be a wrong answer */
			memset(marks->block, 0, nblock * sizeof(uint64_t));
			marks->nblock_i = nblock_i;
			marks->nblock_j = nblock_j;
		}
	}
	mbv_marks_everything(marks);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nblock_i:         %d\n", marks->nblock_i);
		fprintf(stderr, "dbg2       nblock_j:         %d\n", marks->nblock_j);
		fprintf(stderr, "dbg2       error:            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:           %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
void mbv_marks_free(int verbose, struct mbv_marks *marks) {
	int error = MB_ERROR_NO_ERROR;
	if (marks->block != NULL)
		mb_freed(verbose, __FILE__, __LINE__, (void **)&marks->block, &error);
	marks->block = NULL;
	marks->nblock_i = 0;
	marks->nblock_j = 0;
	mbv_marks_everything(marks);
}
/*------------------------------------------------------------------------------*/
void mbv_marks_position(struct mbv_marks *marks) {
	marks->position++;
}
/*------------------------------------------------------------------------------*/
void mbv_marks_color(struct mbv_marks *marks) {
	marks->color++;
}
/*------------------------------------------------------------------------------*/
void mbv_marks_mask(struct mbv_marks *marks) {
	marks->mask++;
}
/*------------------------------------------------------------------------------*/
void mbv_marks_drawn(struct mbv_marks *marks) {
	marks->drawn++;
}
/*------------------------------------------------------------------------------*/
void mbv_marks_everything(struct mbv_marks *marks) {
	marks->position++;
	marks->color++;
	marks->mask++;
	marks->drawn++;
}
/*------------------------------------------------------------------------------*/
void mbv_marks_node(struct mbv_marks *marks, int i, int j) {
	mbv_marks_nodes(marks, i, i, j, j);
}
/*------------------------------------------------------------------------------*/
void mbv_marks_nodes(struct mbv_marks *marks, int i0, int i1, int j0, int j1) {
	int block[4];
	if (!mbv_marks_range(marks, i0, i1, j0, j1, block)) {
		/* without blocks the change can only be told as a whole-view one; never
		   lose it */
		if (marks->block == NULL) {
			marks->position++;
			marks->color++;
		}
		return;
	}

	/* one version for the whole change, so that a user sees it as one event */
	marks->counter++;
	for (int bi = block[0]; bi <= block[1]; bi++)
		for (int bj = block[2]; bj <= block[3]; bj++)
			marks->block[(size_t)bi * marks->nblock_j + bj] = marks->counter;
}
/*------------------------------------------------------------------------------*/
uint64_t mbv_marks_getposition(const struct mbv_marks *marks) {
	return (marks->position);
}
/*------------------------------------------------------------------------------*/
uint64_t mbv_marks_getcolor(const struct mbv_marks *marks) {
	return (marks->color);
}
/*------------------------------------------------------------------------------*/
uint64_t mbv_marks_getmask(const struct mbv_marks *marks) {
	return (marks->mask);
}
/*------------------------------------------------------------------------------*/
uint64_t mbv_marks_getdrawn(const struct mbv_marks *marks) {
	return (marks->drawn);
}
/*------------------------------------------------------------------------------*/
uint64_t mbv_marks_nodesversion(const struct mbv_marks *marks, int i0, int i1, int j0, int j1) {
	int block[4];
	if (!mbv_marks_range(marks, i0, i1, j0, j1, block)) {
		/* with no blocks there is no per-node answer: a value that no stored
		   version can equal, so a user treats the range as changed */
		if (marks->block == NULL)
			return (UINT64_MAX);
		return (0);
	}

	uint64_t version = 0;
	for (int bi = block[0]; bi <= block[1]; bi++)
		for (int bj = block[2]; bj <= block[3]; bj++) {
			const uint64_t stamp = marks->block[(size_t)bi * marks->nblock_j + bj];
			if (stamp > version)
				version = stamp;
		}
	return (version);
}
/*------------------------------------------------------------------------------*/
void mbv_marks_blocks(const struct mbv_marks *marks, int *block_nodes, int *nblock_i, int *nblock_j) {
	*block_nodes = MBV_MARKS_BLOCK;
	*nblock_i = marks->nblock_i;
	*nblock_j = marks->nblock_j;
}
/*------------------------------------------------------------------------------*/
