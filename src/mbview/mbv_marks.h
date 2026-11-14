/*--------------------------------------------------------------------
 *    The MB-system:  mbv_marks.h  11/13/2026
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
 * Change marks of a view's terrain, for users outside the drawing code: a
 * renderer that keeps its own copy of the node arrays, and the terrain pick
 * on the CPU. The drawing code neither reads nor needs them.
 *
 * WHAT A MARK MEANS. A mark is raised where terrain state becomes INVALID,
 * not where it is computed again. The drawing code scales z and colours a
 * node the first time it needs it (mbview_drawdata), and that lazy work
 * raises nothing: whatever invalidated those values raised a mark already.
 * So a user that sees a mark must PREPARE the nodes it wants and only then
 * read them - mbview_prepare_nodes() does what the drawing does, without
 * OpenGL. A mark never means "the values are ready".
 * Keeping the lazy work unmarked is also what keeps the Motif programs
 * exactly as fast as before: nothing is added to their per-node loops.
 *
 * THE MARKS, all counters that only rise:
 *   position  every node's display position is invalid (the grid, the
 *             projection, the vertical exaggeration or the z scale changed).
 *   color     every node's colour is invalid (colour table, shading,
 *             histogram, grid mode, or the data behind the colours).
 *   mask      which nodes hold data may have changed, so triangles can have
 *             appeared or disappeared.
 *   drawn     the node range or the stride a renderer should draw changed
 *             (resolution settings, window size, 2D/3D). The data did not.
 *   block     one counter per block of MBV_MARKS_BLOCK x MBV_MARKS_BLOCK
 *             nodes, raised where single cells changed, as while editing in
 *             mbeditviz. Ask for it over a RANGE of nodes
 *             (mbv_marks_nodesversion): the node ranges of a renderer's
 *             tiles and of the pick's search blocks do not line up with
 *             these blocks, and both include their far edge, so a range
 *             query is the only sound comparison.
 *
 * Nothing is reset by reading: each user keeps the values it last saw and
 * compares, so several users can follow one view.
 *
 * This file uses no toolkit and no OpenGL.
 */

#ifndef MBVIEW_MBV_MARKS_H
#define MBVIEW_MBV_MARKS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* nodes per side of a block. Node (i, j) lies in block (i / MBV_MARKS_BLOCK,
   j / MBV_MARKS_BLOCK), and a range of nodes covers every block its ends fall
   in, both included. */
#define MBV_MARKS_BLOCK 16

struct mbv_marks {
	uint64_t position; /* node positions invalid */
	uint64_t color;    /* node colours invalid */
	uint64_t mask;     /* which nodes hold data may have changed */
	uint64_t drawn;    /* the node range or stride to draw changed */
	uint64_t counter;  /* last version given to a block */
	int nblock_i;      /* blocks across (columns) */
	int nblock_j;      /* blocks down (rows) */
	uint64_t *block;   /* nblock_i * nblock_j versions, 0 if never changed */
};

/* no marks and no blocks; call before first use */
void mbv_marks_init(struct mbv_marks *marks);

/* size the block versions for a grid and raise every mark; returns MB_SUCCESS,
   or MB_FAILURE with *error set, in which case there are no blocks and node
   changes are reported as whole-view marks instead */
int mbv_marks_setsize(int verbose, struct mbv_marks *marks, int n_columns, int n_rows, int *error);

/* free the block versions and raise every mark */
void mbv_marks_free(int verbose, struct mbv_marks *marks);

/* raise one whole-view mark */
void mbv_marks_position(struct mbv_marks *marks);
void mbv_marks_color(struct mbv_marks *marks);
void mbv_marks_mask(struct mbv_marks *marks);
void mbv_marks_drawn(struct mbv_marks *marks);

/* raise all of them: a new grid, a new projection, a view being destroyed */
void mbv_marks_everything(struct mbv_marks *marks);

/* the nodes of one grid cell, or of a range of columns and rows (ends
   included), changed. Without blocks these raise the position and colour
   marks instead, so a user always learns of the change. */
void mbv_marks_node(struct mbv_marks *marks, int i, int j);
void mbv_marks_nodes(struct mbv_marks *marks, int i0, int i1, int j0, int j1);

/* what a user compares with the values it last saw */
uint64_t mbv_marks_getposition(const struct mbv_marks *marks);
uint64_t mbv_marks_getcolor(const struct mbv_marks *marks);
uint64_t mbv_marks_getmask(const struct mbv_marks *marks);
uint64_t mbv_marks_getdrawn(const struct mbv_marks *marks);

/* the largest block version over a range of nodes (ends included): 0 if no
   block of that range ever changed, and UINT64_MAX while there are no blocks,
   which no stored value can equal */
uint64_t mbv_marks_nodesversion(const struct mbv_marks *marks, int i0, int i1, int j0, int j1);

/* the block grid, for a user that wants to size its own cache; the version of
   a node range is still asked for with mbv_marks_nodesversion() */
void mbv_marks_blocks(const struct mbv_marks *marks, int *block_nodes, int *nblock_i, int *nblock_j);

#ifdef __cplusplus
}
#endif

#endif /* MBVIEW_MBV_MARKS_H */
