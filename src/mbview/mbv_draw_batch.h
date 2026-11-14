/*--------------------------------------------------------------------
 *    The MB-system:  mbv_draw_batch.h  11/13/2026
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
 * The drawing of the overlays, the profile view and the 3D soundings
 * decorations as data: what mbv_draw.h records instead of drawing when a
 * batch is the target. A renderer without OpenGL fills a batch by calling the
 * core's drawing function, then draws the commands in their order. This file
 * uses no toolkit and no OpenGL.
 *
 * The batch is an exact record of the calls. Each primitive is kept as it was
 * drawn, with OpenGL's names and numbering - points, lines, line strips, line
 * loops, triangles, quads - because a stippled strip or loop carries its
 * pattern from segment to segment while separate lines start it again: a
 * renderer converts the primitives itself. Each command carries the state the
 * drawing set for it: line width and point size in pixels, the stipple of
 * glLineStipple, the depth test (always the less-than test where it is on),
 * and whether the vertices are seen through the view's camera or with the
 * modelview reset to identity (the grab rectangle of the soundings view). A
 * slot command marks where the drawing put something that is not in the batch
 * - the soundings points, which a renderer draws from mb3dsoundings_colors()
 * and mb3dsoundings_positions() - so that the order is kept.
 */

#ifndef MBVIEW_MBV_DRAW_BATCH_H
#define MBVIEW_MBV_DRAW_BATCH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* primitives, numbered as in OpenGL */
#define MBV_DRAW_POINTS 0x0000
#define MBV_DRAW_LINES 0x0001
#define MBV_DRAW_LINE_LOOP 0x0002
#define MBV_DRAW_LINE_STRIP 0x0003
#define MBV_DRAW_TRIANGLES 0x0004
#define MBV_DRAW_QUADS 0x0007

/* states switched on and off */
#define MBV_DRAW_DEPTH_TEST 1
#define MBV_DRAW_LINE_STIPPLE 2

/* kinds of command */
#define MBV_DRAW_PRIMITIVE 0
#define MBV_DRAW_SLOT 1

/* slots: what the drawing draws that is not in the batch */
#define MBV_DRAW_SLOT_SOUNDINGS 0 /* the points of the 3D soundings view */

struct mbv_draw_vertex {
	float x;
	float y;
	float z;
	/* the colour as the drawing gave it to OpenGL: a renderer converts it the
	   way its framebuffer needs (bytes made from floats round differently
	   depending on how they are computed, so the batch keeps the floats) */
	float r;
	float g;
	float b;
};

struct mbv_draw_command {
	int kind;      /* MBV_DRAW_PRIMITIVE or MBV_DRAW_SLOT */
	int primitive; /* MBV_DRAW_POINTS ... or MBV_DRAW_SLOT_* */
	int first;     /* first vertex */
	int count;     /* number of vertices */
	float line_width;
	float point_size;
	bool stipple;
	int stipple_factor;
	unsigned short stipple_pattern;
	bool depth_test;
	bool camera; /* true: the view's camera; false: modelview set to identity */
};

struct mbv_draw_batch {
	struct mbv_draw_vertex *vertices;
	int num_vertices;
	int num_vertices_alloc;
	struct mbv_draw_command *commands;
	int num_commands;
	int num_commands_alloc;
	bool failed; /* memory ran out: the batch is incomplete */

	/* the drawing state, kept as OpenGL keeps it */
	float color[3];
	float line_width;
	float point_size;
	bool stipple;
	int stipple_factor;
	unsigned short stipple_pattern;
	bool depth_test;
	bool camera;
	bool open; /* between begin and end */
	int open_primitive;
	int open_first;
};

/* an empty batch with OpenGL's initial state */
void mbv_draw_batch_init(struct mbv_draw_batch *batch);
/* empty it again, keeping its memory */
void mbv_draw_batch_clear(struct mbv_draw_batch *batch);
/* release its memory */
void mbv_draw_batch_free(struct mbv_draw_batch *batch);

/* where mbv_draw.h draws: into a batch, or with OpenGL when batch is NULL
   (the default). A renderer sets its batch, calls the drawing function and
   sets NULL again. */
void mbv_draw_set_target(struct mbv_draw_batch *batch);
struct mbv_draw_batch *mbv_draw_get_target(void);

/* the recording side of mbv_draw.h */
void mbv_draw_batch_begin(struct mbv_draw_batch *batch, int primitive);
void mbv_draw_batch_end(struct mbv_draw_batch *batch);
void mbv_draw_batch_color3f(struct mbv_draw_batch *batch, float r, float g, float b);
void mbv_draw_batch_vertex3f(struct mbv_draw_batch *batch, float x, float y, float z);
void mbv_draw_batch_line_width(struct mbv_draw_batch *batch, float width);
void mbv_draw_batch_point_size(struct mbv_draw_batch *batch, float size);
void mbv_draw_batch_line_stipple(struct mbv_draw_batch *batch, int factor, unsigned short pattern);
void mbv_draw_batch_enable(struct mbv_draw_batch *batch, int state, bool on);
void mbv_draw_batch_identity_modelview(struct mbv_draw_batch *batch);
void mbv_draw_batch_slot(struct mbv_draw_batch *batch, int slot);

#ifdef __cplusplus
}
#endif

#endif /* MBVIEW_MBV_DRAW_BATCH_H */
