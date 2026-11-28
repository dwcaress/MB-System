/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_pointquad.vert  11/16/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* mbview-qt (QRhi): the soundings of the 3D soundings view as quads, where
   the API has no point size in the vertex shader (Direct3D): one instance per
   sounding (its display position and colour bytes), a square of the slot's
   point size in physical pixels around the place of the point, from the
   corner of the quad per vertex (-1 or 1 in x and y). A sounding that is not
   drawn (alpha 0) is placed outside the clip volume. */
#version 440

layout(location = 0) in vec2 corner;
layout(location = 1) in vec3 position;
layout(location = 2) in vec4 rgba;

layout(location = 0) out vec4 v_colour;

/* the uniform slot of a drawing command (the same block in every shader) */
layout(std140, binding = 0) uniform Slot {
  mat4 mvp;      // clip-space correction x projection x modelview, built in double
  vec4 colour;   // the colour of commands drawn in one colour
  vec4 viewport; // render-target width and height, line width and point size (physical pixels)
  vec4 stipple;  // pattern, factor (physical pixels), on (0 or 1), 1 where the clip depth runs from 0 to 1
};

void main() {
  v_colour = vec4(rgba.rgb, 1.0);
  vec4 p = mvp * vec4(position, 1.0);
  /* half the point size in normalised device coordinates: size / 2 pixels
     of the 2 / width (height) per pixel */
  vec2 offset = corner * (viewport.w / viewport.xy);
  gl_Position = rgba.a < 0.5 ? vec4(2.0, 2.0, 2.0, 1.0) : vec4(p.xy + offset * p.w, p.z, p.w);
}
