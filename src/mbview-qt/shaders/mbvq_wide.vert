/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_wide.vert  11/15/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira, MARUM, University of Bremen
 *    David W. Caress, Monterey Bay Aquarium Research Institute
 *    Dale N. Chayes, Center for Coastal and Ocean Mapping
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* mbview-qt (QRhi): wide lines of the batch as quads, one instance per
   segment: its two ends from the batch's vertices (two bindings one vertex
   apart for a strip, one binding of vertex pairs for separate lines), the
   corner of the quad per vertex. The colours are interpolated along the
   segment, as OpenGL's smooth shading does. */
#version 440

layout(location = 0) in vec2 corner;
layout(location = 1) in vec3 a_position;
layout(location = 2) in vec3 a_rgb;
layout(location = 3) in vec3 b_position;
layout(location = 4) in vec3 b_rgb;

layout(location = 0) out vec4 v_colour;

/* the uniform slot of a drawing command (the same block in every shader) */
layout(std140, binding = 0) uniform Slot {
  mat4 mvp;      // clip-space correction x projection x modelview, built in double
  vec4 colour;   // the colour of commands drawn in one colour
  vec4 viewport; // render-target width and height, line width and point size (physical pixels)
  vec4 stipple;  // pattern, factor (physical pixels), on (0 or 1), 1 where the clip depth runs from 0 to 1
};

/* A wide line as OpenGL draws it without antialiasing: the segment from a to
   b (clip space) widened by the line width along its minor axis in window
   coordinates (the y axis for a segment more horizontal than vertical, the
   x axis otherwise), without joins. corner.x picks the end (0 a, 1 b),
   corner.y the side (-1, +1). The segment is first clipped against the near
   plane in clip space, so that a segment crossing it keeps its visible part;
   one behind it entirely gets no area. t returns where the end lies on the
   original segment (0 to 1), for what is interpolated along it. A segment
   of zero length gets no area. */
vec4 wide_corner(vec4 a, vec4 b, vec2 corner, out float t) {
  float da = stipple.w > 0.5 ? a.z : a.z + a.w;
  float db = stipple.w > 0.5 ? b.z : b.z + b.w;
  t = corner.x;
  if (da < 0.0 && db < 0.0)
    return vec4(2.0, 2.0, 2.0, 1.0);
  float ta = 0.0;
  float tb = 1.0;
  if (da < 0.0)
    ta = da / (da - db);
  if (db < 0.0)
    tb = da / (da - db);
  vec4 ca = mix(a, b, ta);
  vec4 cb = mix(a, b, tb);
  vec2 wa = ca.xy / ca.w * viewport.xy * 0.5;
  vec2 wb = cb.xy / cb.w * viewport.xy * 0.5;
  vec2 d = wb - wa;
  vec2 minor = abs(d.x) >= abs(d.y) ? vec2(0.0, 1.0) : vec2(1.0, 0.0);
  vec2 offset = minor * (corner.y * viewport.z / viewport.xy);
  vec4 p = corner.x < 0.5 ? ca : cb;
  t = corner.x < 0.5 ? ta : tb;
  return vec4(p.xy + offset * p.w, p.z, p.w);
}

void main() {
  float t;
  gl_Position = wide_corner(mvp * vec4(a_position, 1.0), mvp * vec4(b_position, 1.0), corner, t);
  v_colour = vec4(mix(a_rgb, b_rgb, t), 1.0);
}
