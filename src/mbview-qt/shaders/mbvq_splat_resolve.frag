/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_splat_resolve.frag  11/22/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* mbview-qt (QRhi): compute splatting of the 3D soundings, the resolve: each pixel that a sounding won takes its
   colour, opaque, with its depth, so that what the soundings view draws before (the box, the profile lines) and
   after them (the information highlight, the grab rectangle) is tested against the soundings as with points; a
   pixel without one is left (discard). The pixel is gl_FragCoord's, in the framebuffer's own row order, which the
   compute passes use too. */
#version 440

layout(location = 0) out vec4 frag_colour;

/* the block of a splat dispatch or draw (the same in the splat shaders; SplatBlock, mbvq_rhi_model.h) */
layout(std140, binding = 4) uniform Splat {
  mat4 mvp;      // the soundings slot's matrix: clip-space correction x projection x modelview
  vec4 viewport; // render-target width and height, the splat's size (physical pixels), 1 to flip the rows
  uvec4 counts;  // soundings of the dispatch (at the stride), pixels, the chunk's first sounding, 1: clip depth 0..1
  uvec4 grid;    // groups dispatched (no gl_NumWorkGroups), the stride, the phase, the pass
};

layout(std430, binding = 2) readonly buffer Depth { uint depth[]; };
layout(std430, binding = 3) readonly buffer Winner { uint winner[]; };

void main() {
  uint i = uint(gl_FragCoord.y) * uint(viewport.x) + uint(gl_FragCoord.x);
  uint d = depth[i];
  uint w = winner[i];
  /* no sounding here; or a depth without a colour (never expected) */
  if (d == 0xFFFFFFFFu || w < 0x80000000u)
    discard;
  frag_colour = vec4(unpackUnorm4x8(w).rgb, 1.0);
  gl_FragDepth = uintBitsToFloat(d);
}
