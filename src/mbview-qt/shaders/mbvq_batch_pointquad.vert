/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_batch_pointquad.vert  11/16/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* mbview-qt (QRhi): points of the batch (the information highlight of the 3D
   soundings view) as quads, where the API has no point size in the vertex
   shader: one instance per batch vertex, a square of the slot's point size
   in physical pixels around it, from the corner of the quad per vertex */
#version 440

layout(location = 0) in vec2 corner;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 rgb;

layout(location = 0) out vec4 v_colour;

/* the uniform slot of a drawing command (the same block in every shader) */
layout(std140, binding = 0) uniform Slot {
  mat4 mvp;      // clip-space correction x projection x modelview, built in double
  vec4 colour;   // the colour of commands drawn in one colour
  vec4 viewport; // render-target width and height, line width and point size (physical pixels)
  vec4 stipple;  // pattern, factor (physical pixels), on (0 or 1), 1 where the clip depth runs from 0 to 1
};

void main() {
  v_colour = vec4(rgb, 1.0);
  vec4 p = mvp * vec4(position, 1.0);
  vec2 offset = corner * (viewport.w / viewport.xy);
  gl_Position = vec4(p.xy + offset * p.w, p.z, p.w);
}
