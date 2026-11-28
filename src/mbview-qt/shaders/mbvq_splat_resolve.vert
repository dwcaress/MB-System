/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_splat_resolve.vert  11/22/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* mbview-qt (QRhi): compute splatting of the 3D soundings, the resolve: a triangle over the whole render target,
   vertices (-1, -1), (3, -1), (-1, 3), from gl_VertexIndex, without vertex input */
#version 440

void main() {
  vec2 corner = vec2(float((gl_VertexIndex << 1) & 2), float(gl_VertexIndex & 2));
  gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);
}
