/*--------------------------------------------------------------------
 *    The MB-system:  wcd_swath.vert  12/11/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* water column swath (QRhi): one vertex per sample, across track and depth in
   meters and amplitude in dB; compiled for every backend by qt6_add_shaders */
#version 440

layout(location = 0) in vec2 position;
layout(location = 1) in float value;

layout(location = 0) out float level;

layout(std140, binding = 0) uniform Frame {
  mat4 transform;  // data to clip space
  vec4 range;      // amplitudes of the first and last colour
};

void main() {
  gl_Position = transform * vec4(position, 0.0, 1.0);
  level = (value - range.x) / max(range.y - range.x, 1.0e-6);
}
