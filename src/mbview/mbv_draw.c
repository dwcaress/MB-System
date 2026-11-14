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
	/* mb_reallocd's status is not reliable: check the pointer */
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
	if (!batch->open)
		mbv_draw_batch_command(batch, MBV_DRAW_SLOT, slot);
}
/*------------------------------------------------------------------------------*/
