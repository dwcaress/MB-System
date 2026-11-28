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
 * The drawing of the map and its overlays, the profile view and the 3D
 * soundings decorations as data: what mbv_draw.h records instead of drawing
 * when a batch is the target. A renderer without OpenGL fills a batch by
 * calling the core's drawing function, then draws the commands in their order.
 * This file uses no toolkit and no OpenGL.
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
 * and mb3dsoundings_positions(), and the terrain and contours of the map view,
 * from the node arrays (mbview_prepare_nodes) and mbview_contour_segments() -
 * so that the order is kept.
 *
 * MARKERS. The map overlays draw sites, routes, GIS points and vector balls as
 * GLU disks or spheres compiled into display lists, placed one by one with
 * glTranslatef, glCallList and glTranslatef back. The batch keeps the shape
 * of each list as a record of its GLU parameters, where the drawing defines it
 * (a shape command), and the markers as instances - position and colour -
 * in runs of consecutive markers of one shape (a markers command). A renderer
 * tessellates the shapes itself, as GLU does, and draws each run instanced, in
 * stream order: where the depth test is off (the 2D map, unless it is shown on
 * the spheroid) the order decides overlaps.
 * The placement with a translate and the translate back leaves float rounding
 * in OpenGL's modelview, which the overlays drawn later inherit; an exact
 * replay repeats the same translates, while a renderer that places each marker
 * exactly differs from OpenGL by that rounding.
 *
 * A group command marks where each map overlay begins (MBV_DRAW_GROUP_*), so
 * that a renderer can keep each overlay apart.
 *
 * THE MAP. A renderer fills the map's batch at one resolution, with the plot
 * of that resolution (mbview_plotlow, mbview_plothigh or mbview_plotfull;
 * mbvq_c_plot in mbview-qt, which also clears the node colours before a
 * view's first drawing); only a view the renderer has marked drawable
 * (glx_init) is filled, and the batch of any other stays empty. The drawing
 * then sets no camera and clears nothing: the renderer clears to white and
 * takes its camera from mbview_camera(). The stream is the terrain slot, the
 * contour slot of that resolution (while contours are shown and made for it),
 * then the overlays, each after its group command; in 3D and on the spheroid
 * every command carries the depth test. A fill projects the data when they
 * are not projected yet (the first fill of a view, and the first after the
 * display projection changed), which raises the change marks (mbv_marks.h):
 * the renderer prepares the nodes it draws (mbview_prepare_nodes) after a
 * fill whenever the marks rose. The marker sizes are read from the nodes at
 * the corners of the view bounds, and in 3D the size of the pick marks from
 * the extent of the view bounds (mbview_picksize); the renderer gives the
 * view bounds for 3D (mbv_render.h; without them a 3D fill takes the whole
 * grid). On the spheroid the corner nodes move with the exaggeration, so there
 * the renderer fills again after it prepared them.
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
#define MBV_DRAW_SHAPE 2   /* a marker shape is defined here: primitive is its index in shapes */
#define MBV_DRAW_MARKERS 3 /* markers of one shape: primitive is the shape index, first and count
                              are in instances */
#define MBV_DRAW_GROUP 4   /* a map overlay begins here: primitive is MBV_DRAW_GROUP_* */

/* slots: what the drawing draws that is not in the batch */
#define MBV_DRAW_SLOT_SOUNDINGS 0 /* the points of the 3D soundings view */
#define MBV_DRAW_SLOT_TERRAIN 1   /* the terrain of the map view */
#define MBV_DRAW_SLOT_CONTOURS_LOW 2  /* the map's contours at low resolution */
#define MBV_DRAW_SLOT_CONTOURS_HIGH 3 /* ... at high resolution */
#define MBV_DRAW_SLOT_CONTOURS_FULL 4 /* ... at full resolution */

/* the map overlays, in the order the drawing draws them */
#define MBV_DRAW_GROUP_PICK 0
#define MBV_DRAW_GROUP_AREA 1
#define MBV_DRAW_GROUP_REGION 2
#define MBV_DRAW_GROUP_NAVPICK 3
#define MBV_DRAW_GROUP_POLYGON 4
#define MBV_DRAW_GROUP_POLYLINE 5
#define MBV_DRAW_GROUP_GISPOINT 6
#define MBV_DRAW_GROUP_SITE 7
#define MBV_DRAW_GROUP_ROUTE 8
#define MBV_DRAW_GROUP_NAV 9
#define MBV_DRAW_GROUP_VECTOR 10

/* marker shapes */
#define MBV_DRAW_SHAPE_DISKS 0  /* a filled disk, then a ring over it in its own colour (2D markers) */
#define MBV_DRAW_SHAPE_SPHERE 1 /* a sphere */

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
	int kind;      /* MBV_DRAW_PRIMITIVE, _SLOT, _SHAPE, _MARKERS or _GROUP */
	int primitive; /* MBV_DRAW_POINTS ..., MBV_DRAW_SLOT_*, a shape index or MBV_DRAW_GROUP_* */
	int first;     /* first vertex, or first instance of MBV_DRAW_MARKERS */
	int count;     /* number of vertices, or of instances */
	float line_width;
	float point_size;
	bool stipple;
	int stipple_factor;
	unsigned short stipple_pattern;
	bool depth_test;
	bool camera; /* true: the view's camera; false: modelview set to identity */
};

/* the shape of a marker display list, as the parameters of the GLU calls the
   drawing compiles into it: each GLU call with a new default quadric (filled,
   smooth, outside), the disks with one loop each */
struct mbv_draw_shape {
	int list;            /* the display list id the drawing compiles it into */
	int kind;            /* MBV_DRAW_SHAPE_* */
	double outer;        /* disks: the radius of the fill and the outer radius of the ring;
	                        sphere: the radius */
	double inner;        /* disks: the inner radius of the ring */
	int slices;          /* disks: of the fill; sphere: around its axis */
	int ring_slices;     /* disks: of the ring */
	int stacks;          /* sphere: along its axis */
	float ring_color[3]; /* disks: the ring's colour, which stays the current colour */
};

/* one marker: where it is placed (the glTranslatef arguments) and the colour
   current when it is drawn (the fill of a disk shape, all of a sphere) */
struct mbv_draw_instance {
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
};

struct mbv_draw_batch {
	struct mbv_draw_vertex *vertices;
	int num_vertices;
	int num_vertices_alloc;
	struct mbv_draw_command *commands;
	int num_commands;
	int num_commands_alloc;
	struct mbv_draw_shape *shapes;
	int num_shapes;
	int num_shapes_alloc;
	struct mbv_draw_instance *instances;
	int num_instances;
	int num_instances_alloc;
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
void mbv_draw_batch_marker_disks(struct mbv_draw_batch *batch, int list, double outer, double inner, int slices,
                                 int ring_slices, float ring_r, float ring_g, float ring_b);
void mbv_draw_batch_marker_sphere(struct mbv_draw_batch *batch, int list, double radius, int slices, int stacks);
void mbv_draw_batch_marker(struct mbv_draw_batch *batch, int list, float x, float y, float z);
void mbv_draw_batch_group(struct mbv_draw_batch *batch, int group);

#ifdef __cplusplus
}
#endif

#endif /* MBVIEW_MBV_DRAW_BATCH_H */
