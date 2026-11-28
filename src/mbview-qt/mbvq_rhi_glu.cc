/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_rhi_glu.cc  11/15/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * The QRhi backend of mbview-qt: GLU's disks and spheres (mbvq_rhi_glu.h).
 * The arithmetic follows the SGI sample implementation's gluPartialDisk and
 * gluSphere (as in Mesa's GLU, src/libutil/quad.c) for a default quadric:
 * angles computed in double and kept as GLfloat, their sine and cosine taken
 * in double and kept as GLfloat, every vertex coordinate a float product.
 * Compiled without floating-point contraction (CMakeLists.txt).
 */

#include "mbvq_rhi_glu.h"

#include <cmath>

namespace mbvq {
namespace rhi {

namespace {

/* GLU's cache size: more slices or stacks than this are clamped */
constexpr int kCacheSize = 240;
constexpr double kPi = 3.14159265358979323846;

/* SIN and COS of GLU: the double functions of a GLfloat angle, kept as GLfloat */
float glu_sin(float angle) { return static_cast<float>(std::sin(static_cast<double>(angle))); }
float glu_cos(float angle) { return static_cast<float>(std::cos(static_cast<double>(angle))); }

void vertex(GluPrimitive *primitive, float x, float y, float z) { primitive->xyz.insert(primitive->xyz.end(), {x, y, z}); }

/* the sine and cosine caches of a sphere (gluSphere): around the axis (a) for
   slices + 1 entries, the last one the first again; along it (b), as the
   radius times the sine and cosine of the stack's angle: in double (SGI), or
   as the float radius times the float sine (Apple) */
void sphere_caches(double radius, int slices, int stacks, GluVariant variant, float *sin_a, float *cos_a, float *sin_b,
                   float *cos_b) {
  for (int i = 0; i < slices; i++) {
    const float angle = static_cast<float>(2 * kPi * i / slices);
    sin_a[i] = glu_sin(angle);
    cos_a[i] = glu_cos(angle);
  }
  for (int j = 0; j <= stacks; j++) {
    const float angle = static_cast<float>(kPi * j / stacks);
    if (variant == GluVariant::Apple) {
      const float r = static_cast<float>(radius);
      sin_b[j] = r * glu_sin(angle);
      cos_b[j] = r * glu_cos(angle);
    }
    else {
      sin_b[j] = static_cast<float>(radius * std::sin(static_cast<double>(angle)));
      cos_b[j] = static_cast<float>(radius * std::cos(static_cast<double>(angle)));
    }
  }
  /* make sure it comes to a point */
  sin_b[0] = 0;
  sin_b[stacks] = 0;
  sin_a[slices] = sin_a[0];
  cos_a[slices] = cos_a[0];
}

}  // namespace

std::vector<GluPrimitive> glu_disk(double inner, double outer, int slices, int loops, GluVariant variant) {
  std::vector<GluPrimitive> primitives;
  if (slices >= kCacheSize)
    slices = kCacheSize - 1;
  if (slices < 2 || loops < 1 || outer <= 0.0 || inner < 0.0 || inner > outer)
    return primitives;

  /* gluDisk is gluPartialDisk from 0 over 360 degrees */
  const double start_angle = 0.0;
  const double sweep_angle = 360.0;
  float sin_cache[kCacheSize];
  float cos_cache[kCacheSize];
  const float delta_radius =
      variant == GluVariant::Apple ? static_cast<float>(outer) - static_cast<float>(inner) : static_cast<float>(outer - inner);
  /* the radius of a fraction of the way in: the double outer radius less a
     float (SGI), or all in float (Apple) */
  const auto radius_at = [&](float fraction) {
    if (variant == GluVariant::Apple)
      return static_cast<float>(outer) - delta_radius * fraction;
    return static_cast<float>(outer - delta_radius * fraction);
  };
  const float angle_offset = static_cast<float>(start_angle / 180.0 * kPi);
  for (int i = 0; i <= slices; i++) {
    float angle;
    if (variant == GluVariant::Apple) {
      /* Apple's GLU: the sweep in radians as a float, times i, over slices, in float, and the radii in float */
      const float sweep = static_cast<float>((kPi * sweep_angle) / 180.0);
      angle = sweep * static_cast<float>(i) / static_cast<float>(slices);
    }
    else {
      angle = static_cast<float>(angle_offset + ((kPi * sweep_angle) / 180.0) * i / slices);
    }
    sin_cache[i] = glu_sin(angle);
    cos_cache[i] = glu_cos(angle);
  }
  sin_cache[slices] = sin_cache[0];
  cos_cache[slices] = cos_cache[0];

  int finish;
  if (inner == 0.0) {
    finish = loops - 1;
    /* the innermost loop as a fan */
    GluPrimitive fan;
    fan.mode = kGluTriangleFan;
    vertex(&fan, 0.0f, 0.0f, 0.0f);
    const float radius_low = radius_at(static_cast<float>(loops - 1) / static_cast<float>(loops));
    for (int i = slices; i >= 0; i--)
      vertex(&fan, radius_low * sin_cache[i], radius_low * cos_cache[i], 0.0f);
    primitives.push_back(std::move(fan));
  }
  else {
    finish = loops;
  }
  for (int j = 0; j < finish; j++) {
    const float radius_low = radius_at(static_cast<float>(j) / static_cast<float>(loops));
    const float radius_high = radius_at(static_cast<float>(j + 1) / static_cast<float>(loops));
    GluPrimitive strip;
    strip.mode = kGluQuadStrip;
    for (int i = 0; i <= slices; i++) {
      vertex(&strip, radius_low * sin_cache[i], radius_low * cos_cache[i], 0.0f);
      vertex(&strip, radius_high * sin_cache[i], radius_high * cos_cache[i], 0.0f);
    }
    primitives.push_back(std::move(strip));
  }
  return primitives;
}

std::vector<GluPrimitive> glu_sphere(double radius, int slices, int stacks, GluVariant variant) {
  std::vector<GluPrimitive> primitives;
  if (slices >= kCacheSize)
    slices = kCacheSize - 1;
  if (stacks >= kCacheSize)
    stacks = kCacheSize - 1;
  if (slices < 2 || stacks < 1 || radius < 0.0)
    return primitives;
  float sin_a[kCacheSize];
  float cos_a[kCacheSize];
  float sin_b[kCacheSize];
  float cos_b[kCacheSize];
  sphere_caches(radius, slices, stacks, variant, sin_a, cos_a, sin_b, cos_b);
  /* the vertex of ring j at slice i */
  const auto ring = [&](GluPrimitive *primitive, int j, int i) {
    vertex(primitive, sin_b[j] * sin_a[i], sin_b[j] * cos_a[i], cos_b[j]);
  };

  if (variant == GluVariant::Apple) {
    /* Apple's GLU: one quad strip from pole to pole, each band from the last
       slice to the first, joined by its last vertex repeated */
    GluPrimitive strip;
    strip.mode = kGluQuadStrip;
    for (int j = 0; j < stacks; j++) {
      for (int i = slices; i >= 0; i--) {
        ring(&strip, j, i);
        ring(&strip, j + 1, i);
      }
      ring(&strip, j + 1, 0);
      ring(&strip, j + 1, 0);
    }
    primitives.push_back(std::move(strip));
    return primitives;
  }

  /* the SGI sample implementation: the two ends as fans, then the bands
     between them as quad strips */
  if (stacks < 2)
    return primitives;
  GluPrimitive low;
  low.mode = kGluTriangleFan;
  vertex(&low, 0.0f, 0.0f, static_cast<float>(radius));
  for (int i = slices; i >= 0; i--)
    ring(&low, 1, i);
  primitives.push_back(std::move(low));
  GluPrimitive high;
  high.mode = kGluTriangleFan;
  vertex(&high, 0.0f, 0.0f, static_cast<float>(-radius));
  for (int i = 0; i <= slices; i++)
    ring(&high, stacks - 1, i);
  primitives.push_back(std::move(high));
  for (int j = 1; j < stacks - 1; j++) {
    GluPrimitive strip;
    strip.mode = kGluQuadStrip;
    for (int i = 0; i <= slices; i++) {
      ring(&strip, j + 1, i);
      ring(&strip, j, i);
    }
    primitives.push_back(std::move(strip));
  }
  return primitives;
}

void glu_triangles(const std::vector<GluPrimitive> &primitives, float flag, std::vector<float> *vertices) {
  const auto add = [&](const GluPrimitive &primitive, size_t k) {
    vertices->insert(vertices->end(),
                     {primitive.xyz[3 * k], primitive.xyz[3 * k + 1], primitive.xyz[3 * k + 2], flag});
  };
  for (const GluPrimitive &primitive : primitives) {
    const size_t n = primitive.xyz.size() / 3;
    if (primitive.mode == kGluTriangleFan) {
      for (size_t k = 1; k + 1 < n; k++) {
        add(primitive, 0);
        add(primitive, k);
        add(primitive, k + 1);
      }
    }
    else if (primitive.mode == kGluQuadStrip) {
      /* a quad strip covers what the triangle strip of its vertices covers */
      for (size_t k = 0; k + 2 < n; k++) {
        if (k % 2 == 0) {
          add(primitive, k);
          add(primitive, k + 1);
          add(primitive, k + 2);
        }
        else {
          add(primitive, k + 1);
          add(primitive, k);
          add(primitive, k + 2);
        }
      }
    }
  }
}

std::vector<float> marker_mesh(const struct mbv_draw_shape &shape, GluVariant variant) {
  /* each GLU call with a new default quadric, the disks with one loop each
     (mbv_draw_batch.h) */
  std::vector<float> vertices;
  if (shape.kind == MBV_DRAW_SHAPE_DISKS) {
    glu_triangles(glu_disk(0.0, shape.outer, shape.slices, 1, variant), 0.0f, &vertices);
    glu_triangles(glu_disk(shape.inner, shape.outer, shape.ring_slices, 1, variant), 1.0f, &vertices);
  }
  else if (shape.kind == MBV_DRAW_SHAPE_SPHERE) {
    glu_triangles(glu_sphere(shape.outer, shape.slices, shape.stacks, variant), 0.0f, &vertices);
  }
  return vertices;
}

}  // namespace rhi
}  // namespace mbvq
