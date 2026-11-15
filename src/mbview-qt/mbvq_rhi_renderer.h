/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_rhi_renderer.h  11/15/2026
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
 * The QRhi backend of mbview-qt, renderer layer: the GPU side of a view, made
 * for one QRhi and dropped with it. Its buffers are only caches of the core's
 * arrays, so a new QRhi (a dock floated or docked again, a lost device)
 * builds them again from the core. The shaders and the GPU buffers of
 * mbview-qt are only here.
 *
 * A renderer is made with create() for render targets compatible with a
 * render pass. In a frame, frame() runs the frame driver's start of the frame
 * and writes the uploads and the uniforms into the frame's resource update
 * batch, and record() draws inside the pass. A view and the offscreen
 * renderer of the tests run the same frame() and record().
 *
 * MapRenderer draws the map view: the terrain from the tile levels of the
 * terrain plan (mbvq_rhi_model.h), one stride per frame, with the depth test
 * of the batch's terrain slot. Its plan shares the process-wide memory budget
 * of the tile levels with the other renderers.
 */

#ifndef MBVIEW_QT_MBVQ_RHI_RENDERER_H
#define MBVIEW_QT_MBVQ_RHI_RENDERER_H

#include <QSize>
#include <array>
#include <functional>
#include <memory>
#include <vector>

#include <rhi/qrhi.h>

#include "mbvq_rhi.h"
#include "mbvq_rhi_frame.h"
#include "mbvq_rhi_model.h"

namespace mbvq {
namespace rhi {

/* the size of the uniform slot of a drawing command (std140: mvp, colour,
   viewport, stipple; shaders/mbvq_terrain.vert) */
constexpr quint32 kSlotBytes = 112;

/* what a frame leaves to do */
enum class Prepared {
  Done,  /* everything is uploaded */
  More,  /* uploads wait for the next frames, or the fill was refused: ask for another frame */
  Failed /* a buffer could not be made: nothing is drawn (MapRenderer::frame) */
};

class ViewRenderer {
 public:
  explicit ViewRenderer(QRhi *rhi) : rhi_(rhi) {}
  virtual ~ViewRenderer() = default;
  ViewRenderer(const ViewRenderer &) = delete;
  ViewRenderer &operator=(const ViewRenderer &) = delete;

  /* the pipelines, for render targets compatible with pass; false if the
     QRhi lacks a feature the views need or a resource cannot be made, and
     then nothing is held */
  virtual bool create(QRhiRenderPassDescriptor *pass, int samples) = 0;
  /* QRhi resources held */
  virtual int resources() const = 0;
  /* the pipelines suit render targets of this pass */
  bool compatible(const QRhiRenderPassDescriptor *pass) const;

 protected:
  QRhi *rhi_;
  /* a pass compatible with the one create() was given, for the pipelines */
  std::unique_ptr<QRhiRenderPassDescriptor> pass_;
};

class MapRenderer final : public ViewRenderer {
 public:
  MapRenderer(QRhi *rhi, const RenderOptions &options);
  ~MapRenderer() override;

  bool create(QRhiRenderPassDescriptor *pass, int samples) override;
  int resources() const override;
  /* the options of the next frames (fail_create and float_terrain_colours
     count only when the renderer is made) */
  void set_options(const RenderOptions &options) { options_ = options; }
  /* the terrain colours are floats (RenderOptions::float_terrain_colours) */
  bool float_colours() const { return float_colours_; }

  /* A frame of the map before its render pass, the same for a view and the
     offscreen renderer of the tests: the frame driver's start of the frame
     (MapFrame::begin; continued: the frame was asked for only to continue
     uploads), then the plan, the uploads and the uniforms in updates.
     Failed: a tile buffer could not be made; updates was released, nothing
     is to be recorded, and the renderer dropped its tiles (its pipelines
     stay), so that a later frame plans and uploads everything again. */
  Prepared frame(MapFrame *frame, QRhiResourceUpdateBatch *updates, bool continued, QSize pixels);
  /* draw inside the render pass */
  void record(QRhiCommandBuffer *cb, QSize pixels);
  /* the times of the frame's pass, which its caller measures (RenderStats):
     the pass begun with the frame's updates, the commands recorded and the
     pass ended, the whole frame on the CPU, the GPU time the QRhi reports
     (-1 if none), and where the frame began and ended on the clock */
  void set_pass_times(double upload_ms, double record_ms, double render_ms, double gpu_ms, double begin_ms,
                      double end_ms);
  /* a view's window submitted the frame (QRhiWidget::frameSubmitted) at a
     time of the same clock (ms since its epoch): the time from the end of
     render() to there, the wait for the frame's GPU work and the window's
     composition, counts among the frame's costs outside its preparing and
     gathering, which the next drag budget leaves room for */
  void frame_submitted(double at_ms);
  /* the view is shown (true) or hidden: a shown view's latest levels are
     kept from other views' evictions */
  void set_visible(bool visible) { plan_.set_visible(visible); }
  /* called when this renderer should draw a frame for the memory budget:
     another renderer evicted tile levels of this one, whose GPU memory this
     renderer's next frame frees; or the memory budget made this renderer's
     latest frame coarser and another let go of levels it kept. The view asks
     for that frame. */
  void set_frame_request(std::function<void()> request) { plan_.set_evicted_callback(std::move(request)); }

  const RenderStats &stats() const { return stats_; }
  const TerrainPlan &plan() const { return plan_; }

 private:
  struct TileGpu {
    std::unique_ptr<QRhiBuffer> positions;
    std::unique_ptr<QRhiBuffer> colours;
    std::unique_ptr<QRhiBuffer> indices;
  };
  void release();
  void drop_tiles();
  Prepared prepare(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels);
  std::unique_ptr<QRhiBuffer> make_buffer(QRhiBuffer::UsageFlags usage, quint32 size);
  bool upload(QRhiResourceUpdateBatch *updates, TileUpload *upload);
  bool upload_triangle_ids(QRhiResourceUpdateBatch *updates, const FrameInput &input);

  RenderOptions options_;
  bool float_colours_;
  TerrainPlan plan_;
  int generation_ = -1; /* of the plan's tiles in tiles_ */
  std::vector<std::array<TileGpu, kLevels>> tiles_;
  /* the levels the last frame draws, with their triangles' index counts */
  std::vector<LevelKey> drawn_;
  std::vector<uint32_t> drawn_counts_;
  /* the previous frame's cost outside its preparing and gathering (ms), the
     level it drew, and whether its window's submission is still to come;
     during a drag the latest such cost of a frame at each level (< 0: none) */
  double outside_ms_ = 0.0;
  int outside_level_ = -1;
  bool submission_due_ = false;
  bool dragging_ = false;
  double outside_level_ms_[kLevels];
  int copies_ = 1; /* the copies the QRhi keeps of each buffer */
  std::unique_ptr<QRhiBuffer> uniforms_;
  std::unique_ptr<QRhiShaderResourceBindings> bindings_;
  std::unique_ptr<QRhiGraphicsPipeline> terrain_[2]; /* without and with the depth test */
  std::unique_ptr<QRhiBuffer> id_positions_;
  std::unique_ptr<QRhiBuffer> id_colours_;
  quint32 id_vertices_ = 0;
  bool draw_terrain_ = false;
  bool depth_test_ = false;
  RenderStats stats_;
};

}  // namespace rhi
}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_RHI_RENDERER_H */
