/*--------------------------------------------------------------------
 *    The MB-system:  wcd_swath_rhi.h  12/11/2026
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
 * The swath mesh drawn with QRhi, Qt's interface to Metal, Direct3D, Vulkan and
 * OpenGL: the same mesh, colour map and transform as SwathGL, in the window
 * (RhiSwathView) and for image files without a window (render_swath_rhi()).
 */

#ifndef MBWCDVIEW_WCD_SWATH_RHI_H
#define MBWCDVIEW_WCD_SWATH_RHI_H

#include <memory>
#include <vector>

#include <QImage>
#include <QRect>
#include <QSize>
#include <rhi/qrhi.h>

namespace wcd {

class WcdPlot;

/* QRhi resources of the mesh in one QRhi */
class SwathRhi {
 public:
  explicit SwathRhi(QRhi *rhi);
  ~SwathRhi();
  /* pipeline for render targets compatible with pass */
  bool init(QRhiRenderPassDescriptor *pass, int sample_count);
  void upload(QRhiResourceUpdateBatch *updates, const std::vector<float> &vertices,
              const std::vector<unsigned int> &indices);
  /* transform and colour range of the plot, for the next draw() */
  void update(QRhiResourceUpdateBatch *updates, const WcdPlot &plot);
  /* draw inside a render pass into the viewport (device pixels from the bottom left) */
  void draw(QRhiCommandBuffer *cb, const QRect &viewport);

 private:
  QRhi *rhi_;
  std::unique_ptr<QRhiBuffer> vertices_, indices_, uniforms_;
  std::unique_ptr<QRhiTexture> lut_;
  std::unique_ptr<QRhiSampler> sampler_;
  std::unique_ptr<QRhiShaderResourceBindings> bindings_;
  std::unique_ptr<QRhiGraphicsPipeline> pipeline_;
  quint32 nindices_ = 0;
  bool lut_uploaded_ = false;
};

/* draw the swath of the plot into image (size x scale pixels) with a QRhi of its own */
bool render_swath_rhi(const WcdPlot &plot, const QSize &size, double scale, QImage &image);

}  // namespace wcd

#endif
