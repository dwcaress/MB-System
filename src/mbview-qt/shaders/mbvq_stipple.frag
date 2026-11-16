/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_stipple.frag  11/15/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* mbview-qt (QRhi): the stipple of glLineStipple: a fragment is drawn where
   bit floor(counter / factor) mod 16 of the 16-bit pattern is set, with
   OpenGL's stipple counter (BatchPlan::add_stipple), in physical pixels
   along the major axis */
#version 440

layout(location = 0) in vec4 v_colour;
layout(location = 1) in float v_distance;

layout(location = 0) out vec4 frag_colour;

/* the uniform slot of a drawing command (the same block in every shader) */
layout(std140, binding = 0) uniform Slot {
  mat4 mvp;      // clip-space correction x projection x modelview, built in double
  vec4 colour;   // the colour of commands drawn in one colour
  vec4 viewport; // render-target width and height, line width and point size (physical pixels)
  vec4 stipple;  // pattern, factor (physical pixels), on (0 or 1), 1 where the clip depth runs from 0 to 1
};

void main() {
  float bit = mod(floor(v_distance / stipple.y), 16.0);
  if (stipple.z > 0.5 && mod(floor(stipple.x / exp2(bit)), 2.0) < 0.5)
    discard;
  frag_colour = vec4(v_colour.rgb, 1.0);
}
