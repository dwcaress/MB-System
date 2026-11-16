/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_line.vert  11/15/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* mbview-qt (QRhi): lines and triangles of the map's batch straight from its
   vertices (position and colour as the core recorded them, 24 bytes each);
   the camera, or the projection alone, from the command's slot */
#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 rgb;

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
  gl_Position = mvp * vec4(position, 1.0);
}
