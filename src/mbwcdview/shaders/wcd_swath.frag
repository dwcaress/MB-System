/*--------------------------------------------------------------------
 *    The MB-system:  wcd_swath.frag  12/11/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* water column swath (QRhi): the amplitude level through the colour map */
#version 440

layout(location = 0) in float level;

layout(location = 0) out vec4 color;

layout(binding = 1) uniform sampler2D lut;

void main() {
  color = texture(lut, vec2(clamp(level, 0.0, 1.0) * 255.0 / 256.0 + 0.5 / 256.0, 0.5));
}
