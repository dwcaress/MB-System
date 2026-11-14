/*--------------------------------------------------------------------
 *    The MB-system:  mbv_draw.c  11/13/2026
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
 * The recording side of the drawing interface (mbv_draw.h, mbv_draw_batch.h).
 * This file uses no toolkit and no OpenGL.
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

#include "mbv_draw_batch.h"

/* the target of mbv_draw.h, NULL for OpenGL */
struct mbv_draw_batch *mbv_draw_target = NULL;

/*------------------------------------------------------------------------------*/
void mbv_draw_set_target(struct mbv_draw_batch *batch) { mbv_draw_target = batch; }
/*------------------------------------------------------------------------------*/
struct mbv_draw_batch *mbv_draw_get_target(void) { return mbv_draw_target; }
/*------------------------------------------------------------------------------*/
/* OpenGL's initial state */
static void mbv_draw_batch_reset_state(struct mbv_draw_batch *batch) {
	batch->color[0] = 1.0f;
	batch->color[1] = 1.0f;
	batch->color[2] = 1.0f;
	batch->line_width = 1.0f;
	batch->point_size = 1.0f;
	batch->stipple = false;
	batch->stipple_factor = 1;
	batch->stipple_pattern = 0xFFFF;
	batch->depth_test = false;
	batch->camera = true;
	batch->open = false;
	batch->open_primitive = MBV_DRAW_POINTS;
	batch->open_first = 0;
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_init(struct mbv_draw_batch *batch) {
	memset(batch, 0, sizeof(*batch));
	mbv_draw_batch_reset_state(batch);
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_clear(struct mbv_draw_batch *batch) {
	batch->num_vertices = 0;
	batch->num_commands = 0;
	batch->num_shapes = 0;
	batch->num_instances = 0;
	batch->failed = false;
	mbv_draw_batch_reset_state(batch);
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_free(struct mbv_draw_batch *batch) {
	int error = MB_ERROR_NO_ERROR;
	if (batch->vertices != NULL)
		mb_freed(0, __FILE__, __LINE__, (void **)&batch->vertices, &error);
	if (batch->commands != NULL)
		mb_freed(0, __FILE__, __LINE__, (void **)&batch->commands, &error);
	if (batch->shapes != NULL)
		mb_freed(0, __FILE__, __LINE__, (void **)&batch->shapes, &error);
	if (batch->instances != NULL)
		mb_freed(0, __FILE__, __LINE__, (void **)&batch->instances, &error);
	mbv_draw_batch_init(batch);
}
/*------------------------------------------------------------------------------*/
/* room for one more element, doubling the array; false when memory ran out,
   which leaves the array as it was and marks the batch incomplete */
static bool mbv_draw_batch_grow(struct mbv_draw_batch *batch, void **array, int count, int *alloc, size_t size) {
	if (count < *alloc)
		return true;
	const int alloc_new = *alloc > 0 ? 2 * *alloc : 256;
	void *grown = *array;
	int error = MB_ERROR_NO_ERROR;
	/* check the pointer: without #1687 mb_reallocd reports success when it fails */
	mb_reallocd(0, __FILE__, __LINE__, (size_t)alloc_new * size, &grown, &error);
	if (grown == NULL) {
		batch->failed = true;
		return false;
	}
	*array = grown;
	*alloc = alloc_new;
	return true;
}
/*------------------------------------------------------------------------------*/
/* a command with the current state */
static struct mbv_draw_command *mbv_draw_batch_command(struct mbv_draw_batch *batch, int kind, int primitive) {
	if (!mbv_draw_batch_grow(batch, (void **)&batch->commands, batch->num_commands, &batch->num_commands_alloc,
	                         sizeof(struct mbv_draw_command)))
		return NULL;
	struct mbv_draw_command *command = &batch->commands[batch->num_commands++];
	command->kind = kind;
	command->primitive = primitive;
	command->first = batch->num_vertices;
	command->count = 0;
	command->line_width = batch->line_width;
	command->point_size = batch->point_size;
	command->stipple = batch->stipple;
	command->stipple_factor = batch->stipple_factor;
	command->stipple_pattern = batch->stipple_pattern;
	command->depth_test = batch->depth_test;
	command->camera = batch->camera;
	return command;
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_begin(struct mbv_draw_batch *batch, int primitive) {
	batch->open = true;
	batch->open_primitive = primitive;
	batch->open_first = batch->num_vertices;
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_end(struct mbv_draw_batch *batch) {
	if (!batch->open)
		return;
	batch->open = false;
	const int count = batch->num_vertices - batch->open_first;
	if (count <= 0)
		return;
	struct mbv_draw_command *command = mbv_draw_batch_command(batch, MBV_DRAW_PRIMITIVE, batch->open_primitive);
	if (command != NULL) {
		command->first = batch->open_first;
		command->count = count;
	}
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_color3f(struct mbv_draw_batch *batch, float r, float g, float b) {
	batch->color[0] = r;
	batch->color[1] = g;
	batch->color[2] = b;
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_vertex3f(struct mbv_draw_batch *batch, float x, float y, float z) {
	/* OpenGL draws nothing for a vertex outside begin and end */
	if (!batch->open)
		return;
	if (!mbv_draw_batch_grow(batch, (void **)&batch->vertices, batch->num_vertices, &batch->num_vertices_alloc,
	                         sizeof(struct mbv_draw_vertex)))
		return;
	struct mbv_draw_vertex *vertex = &batch->vertices[batch->num_vertices++];
	vertex->x = x;
	vertex->y = y;
	vertex->z = z;
	vertex->r = batch->color[0];
	vertex->g = batch->color[1];
	vertex->b = batch->color[2];
}
/*------------------------------------------------------------------------------*/
/* the state calls change nothing while a primitive is open, as in OpenGL,
   where they are errors between glBegin and glEnd; a line width or point size
   of 0 or less is an error too, and OpenGL keeps the one it has */
void mbv_draw_batch_line_width(struct mbv_draw_batch *batch, float width) {
	if (!batch->open && width > 0.0f)
		batch->line_width = width;
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_point_size(struct mbv_draw_batch *batch, float size) {
	if (!batch->open && size > 0.0f)
		batch->point_size = size;
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_line_stipple(struct mbv_draw_batch *batch, int factor, unsigned short pattern) {
	if (!batch->open) {
		batch->stipple_factor = factor;
		batch->stipple_pattern = pattern;
	}
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_enable(struct mbv_draw_batch *batch, int state, bool on) {
	if (batch->open)
		return;
	if (state == MBV_DRAW_DEPTH_TEST)
		batch->depth_test = on;
	else if (state == MBV_DRAW_LINE_STIPPLE)
		batch->stipple = on;
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_identity_modelview(struct mbv_draw_batch *batch) {
	if (!batch->open)
		batch->camera = false;
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_slot(struct mbv_draw_batch *batch, int slot) {
	if (batch->open)
		return;
	mbv_draw_batch_command(batch, MBV_DRAW_SLOT, slot);

	/* the contour display list sets black and a line width of 1, and leaves
	   them current (mbview_contour) */
	if (slot == MBV_DRAW_SLOT_CONTOURS_LOW || slot == MBV_DRAW_SLOT_CONTOURS_HIGH ||
	    slot == MBV_DRAW_SLOT_CONTOURS_FULL) {
		batch->color[0] = 0.0f;
		batch->color[1] = 0.0f;
		batch->color[2] = 0.0f;
		batch->line_width = 1.0f;
	}
}
/*------------------------------------------------------------------------------*/
/* a marker shape where the drawing defines its display list; defining a list
   draws nothing and changes no state (GL_COMPILE) */
static void mbv_draw_batch_shape(struct mbv_draw_batch *batch, const struct mbv_draw_shape *shape) {
	/* a display list cannot be defined between glBegin and glEnd */
	if (batch->open)
		return;
	if (!mbv_draw_batch_grow(batch, (void **)&batch->shapes, batch->num_shapes, &batch->num_shapes_alloc,
	                         sizeof(struct mbv_draw_shape)))
		return;
	const int index = batch->num_shapes;
	struct mbv_draw_command *command = mbv_draw_batch_command(batch, MBV_DRAW_SHAPE, index);
	if (command == NULL)
		return;
	batch->shapes[batch->num_shapes++] = *shape;
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_marker_disks(struct mbv_draw_batch *batch, int list, double outer, double inner, int slices,
                                 int ring_slices, float ring_r, float ring_g, float ring_b) {
	struct mbv_draw_shape shape;
	memset(&shape, 0, sizeof(shape));
	shape.list = list;
	shape.kind = MBV_DRAW_SHAPE_DISKS;
	shape.outer = outer;
	shape.inner = inner;
	shape.slices = slices;
	shape.ring_slices = ring_slices;
	shape.ring_color[0] = ring_r;
	shape.ring_color[1] = ring_g;
	shape.ring_color[2] = ring_b;
	mbv_draw_batch_shape(batch, &shape);
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_marker_sphere(struct mbv_draw_batch *batch, int list, double radius, int slices, int stacks) {
	struct mbv_draw_shape shape;
	memset(&shape, 0, sizeof(shape));
	shape.list = list;
	shape.kind = MBV_DRAW_SHAPE_SPHERE;
	shape.outer = radius;
	shape.slices = slices;
	shape.stacks = stacks;
	mbv_draw_batch_shape(batch, &shape);
}
/*------------------------------------------------------------------------------*/
/* a marker of the shape last defined for its list: with OpenGL, glCallList
   draws what the list holds now. A list with no shape in the batch draws
   nothing. Consecutive markers of one shape with the same depth test and
   camera make one command, which keeps the line width, point size and stipple
   of its first marker: they do not change GLU's filled shapes. */
void mbv_draw_batch_marker(struct mbv_draw_batch *batch, int list, float x, float y, float z) {
	if (batch->open)
		return;
	int index = batch->num_shapes - 1;
	while (index >= 0 && batch->shapes[index].list != list)
		index--;
	if (index < 0)
		return;
	if (!mbv_draw_batch_grow(batch, (void **)&batch->instances, batch->num_instances, &batch->num_instances_alloc,
	                         sizeof(struct mbv_draw_instance)))
		return;

	/* extend the run of the last command, or start one */
	struct mbv_draw_command *last = batch->num_commands > 0 ? &batch->commands[batch->num_commands - 1] : NULL;
	if (last != NULL && last->kind == MBV_DRAW_MARKERS && last->primitive == index &&
	    last->first + last->count == batch->num_instances && last->depth_test == batch->depth_test &&
	    last->camera == batch->camera) {
		last->count++;
	} else {
		struct mbv_draw_command *command = mbv_draw_batch_command(batch, MBV_DRAW_MARKERS, index);
		if (command == NULL)
			return;
		command->first = batch->num_instances;
		command->count = 1;
	}
	struct mbv_draw_instance *instance = &batch->instances[batch->num_instances++];
	instance->x = x;
	instance->y = y;
	instance->z = z;
	instance->r = batch->color[0];
	instance->g = batch->color[1];
	instance->b = batch->color[2];

	/* a disk shape's list sets the ring colour and leaves it current */
	if (batch->shapes[index].kind == MBV_DRAW_SHAPE_DISKS) {
		batch->color[0] = batch->shapes[index].ring_color[0];
		batch->color[1] = batch->shapes[index].ring_color[1];
		batch->color[2] = batch->shapes[index].ring_color[2];
	}
}
/*------------------------------------------------------------------------------*/
void mbv_draw_batch_group(struct mbv_draw_batch *batch, int group) {
	if (!batch->open)
		mbv_draw_batch_command(batch, MBV_DRAW_GROUP, group);
}
/*------------------------------------------------------------------------------*/
