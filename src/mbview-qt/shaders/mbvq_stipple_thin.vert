/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_stipple_thin.vert  11/15/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* mbview-qt (QRhi): stippled lines of one physical pixel, pairs of vertices
   with OpenGL's stipple counter at each vertex (BatchPlan::add_stipple), in
   physical pixels along the major axis */
#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 rgb;
layout(location = 2) in float along;

layout(location = 0) out vec4 v_colour;
layout(location = 1) out float v_distance;

/* the uniform slot of a drawing command (the same block in every shader) */
layout(std140, binding = 0) uniform Slot {
  mat4 mvp;      // clip-space correction x projection x modelview, built in double
  vec4 colour;   // the colour of commands drawn in one colour
  vec4 viewport; // render-target width and height, line width and point size (physical pixels)
  vec4 stipple;  // pattern, factor (physical pixels), on (0 or 1), 1 where the clip depth runs from 0 to 1
};

void main() {
  v_colour = vec4(rgb, 1.0);
  v_distance = along;
  gl_Position = mvp * vec4(position, 1.0);
}
