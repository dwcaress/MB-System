/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_colour.frag  11/15/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* mbview-qt (QRhi): the colour from the vertex shader, interpolated across a
   triangle or along a line as OpenGL's default smooth shading does; opaque */
#version 440

layout(location = 0) in vec4 v_colour;

layout(location = 0) out vec4 frag_colour;

void main() {
  frag_colour = vec4(v_colour.rgb, 1.0);
}
