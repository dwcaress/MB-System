/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_rhi_glu.h  11/15/2026
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
 * The QRhi backend of mbview-qt, model layer: the marker shapes of the map
 * overlays as GLU tessellates them, without OpenGL. The map overlays draw
 * sites, routes, GIS points and vector balls as gluDisk and gluSphere calls
 * compiled into display lists (mbv_draw_batch.h keeps their parameters); the
 * QRhi renderer draws the same vertices, so that its markers cover the
 * pixels OpenGL's cover.
 *
 * GLU's vertices are float products of float sine and cosine caches, so the
 * arithmetic is written as GLU writes it, and this file is compiled without
 * floating-point contraction (-ffp-contract=off): a fused multiply-add would
 * change the last bits. Two algorithms:
 * - GluVariant::Sgi, the SGI sample implementation that Mesa's GLU carries
 *   (Linux and the Motif programs), for the views;
 * - GluVariant::Apple, Apple's GLU (the macOS OpenGL views), for comparisons
 *   with OpenGL on macOS only: its disks take their angles and radii in
 *   float, its sphere multiplies the float radius by the float sine and
 *   cosine and is one quad strip.
 * Both are checked bit for bit against vertices traced from the two GLU
 * libraries (test/mbview-qt).
 */

#ifndef MBVIEW_QT_MBVQ_RHI_GLU_H
#define MBVIEW_QT_MBVQ_RHI_GLU_H

#include <vector>

#include "mbv_draw_batch.h"
#include "mbvq_rhi.h"

namespace mbvq {
namespace rhi {

/* the primitives GLU draws its quadrics with, numbered as in OpenGL */
constexpr int kGluTriangleFan = 6;
constexpr int kGluQuadStrip = 8;

/* one primitive of a GLU call: its mode and its vertices, x y z each */
struct GluPrimitive {
  int mode = kGluTriangleFan;
  std::vector<float> xyz;
};

/* gluDisk(quadric, inner, outer, slices, loops) with a default quadric
   (filled, outside) */
std::vector<GluPrimitive> glu_disk(double inner, double outer, int slices, int loops, GluVariant variant);
/* gluSphere(quadric, radius, slices, stacks) with a default quadric */
std::vector<GluPrimitive> glu_sphere(double radius, int slices, int stacks, GluVariant variant);

/* the primitives as separate triangles, appended to vertices as x y z flag
   per vertex: a fan as (centre, k, k + 1), a quad strip as the triangle
   strip of its vertices */
void glu_triangles(const std::vector<GluPrimitive> &primitives, float flag, std::vector<float> *vertices);

/* The mesh of a marker shape as the overlays compile it (mbv_draw_batch.h):
   a disk shape is the filled disk (flag 0, drawn in the marker's colour),
   then the ring over it (flag 1, drawn in the shape's ring colour); a sphere
   is the sphere (flag 0). x y z flag per vertex, separate triangles, in
   GLU's order. */
std::vector<float> marker_mesh(const struct mbv_draw_shape &shape, GluVariant variant);

}  // namespace rhi
}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_RHI_GLU_H */
