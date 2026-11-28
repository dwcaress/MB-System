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
 * MapRenderer draws the map view: the commands of the map's batch in stream
 * order (BatchPlan, mbvq_rhi_model.h), each with a uniform slot of its own in
 * one dynamic buffer per renderer, written once per frame; the terrain slot
 * from the tile levels of the terrain plan, one stride per frame; the contour
 * slot from the finest contours made, uploaded only when they change; lines
 * of one physical pixel with the line topology, wider ones as quads
 * extruded along their minor axis in the shaders; markers instanced per run,
 * with GLU's meshes (mbvq_rhi_glu.h). Line widths, point sizes and stipple
 * factors are scaled by the view's device pixel ratio (Style). The batch's
 * vertices and instances go up in the frame that filled them, whole: a batch
 * is drawn in the frame that filled it. They, what the plan made of them and
 * the contours are dynamic buffers of exactly the size of their data, and the
 * uniform ring one grown by doubling; each is always written whole. Qt's
 * Metal, Vulkan and Direct3D 12 backends keep a dynamic buffer once per frame
 * slot and drop the writes still pending for a slot when the whole buffer is
 * written, so a slot that no frame of the view uses for a while (a window
 * that draws frames of its own between the view's) holds one pending write,
 * not one per frame. The tile levels, the marker meshes and the corners of
 * the wide lines are static buffers, uploaded once or when they change. Qt's
 * Metal backend keeps a static upload for each frame slot until a frame on
 * that slot binds the buffer, and a whole upload does not drop the older
 * ones: tile streams uploaded again in consecutive frames (shading,
 * exaggeration and colour changes) still pile up there, one upload per
 * frame, while something else puts every frame of the view on one frame
 * slot, and the uploads still pending for a level no longer drawn stay until
 * that level is drawn on that slot again or released. The plan shares the
 * process-wide memory budget of the tile levels with the other renderers;
 * the batch's buffers and the contours lie outside it.
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
#include "mbvq_rhi_glu.h"
#include "mbvq_rhi_model.h"

namespace mbvq {
namespace rhi {

/* the size of the uniform slot of a drawing command (std140: mvp, colour,
   viewport, stipple; shaders/mbvq_terrain.vert) */
constexpr quint32 kSlotBytes = 112;
/* a batch upload above this is reported (RenderStats::batch_over_cap); it is
   never split, since a batch is drawn in the frame that filled it */
constexpr size_t kBatchCap = static_cast<size_t>(64) << 20;
/* marker meshes kept for later frames, the least recently used dropped */
constexpr size_t kMeshes = 32;

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
  /* of them: pipelines, bindings, uniforms and pass; the batch's buffers and meshes */
  int fixed_resources() const;
  int batch_resources() const;
  /* the options of the next frames (fail_create counts only when the
     renderer or its pipelines are made, float_terrain_colours only when the
     renderer is made) */
  void set_options(const RenderOptions &options) { options_ = options; }
  /* the terrain colours are floats (RenderOptions::float_terrain_colours) */
  bool float_colours() const { return float_colours_; }
  /* the view's device pixel ratio (devicePixelRatioF()) for the next frames;
     <= 0: RenderOptions::pixel_ratio */
  void set_pixel_ratio(double ratio) { view_ratio_ = ratio; }

  /* A frame of the map before its render pass, the same for a view and the
     offscreen renderer of the tests: the frame driver's start of the frame
     (MapFrame::begin; continued: the frame was asked for only to continue
     uploads), then the plan, the uploads and the uniforms in updates.
     Failed: a buffer could not be made; updates was released, nothing is to
     be recorded, and the renderer dropped its tiles and the batch's buffers
     (its pipelines stay), so that a later frame plans and uploads everything
     again. A uniform ring that could not even be made again at its old size
     takes the pipelines and bindings with it: the renderer makes them again
     at the start of its next frame. */
  Prepared frame(MapFrame *frame, QRhiResourceUpdateBatch *updates, bool continued, QSize pixels);
  /* a frame of a batch given as the frame driver's input, without a map
     (OffscreenRenderer::render_batch of the tests) */
  Prepared frame(const FrameInput &input, QRhiResourceUpdateBatch *updates, QSize pixels);
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
  const BatchPlan &batch_plan() const { return batch_; }

 private:
  struct TileGpu {
    std::unique_ptr<QRhiBuffer> positions;
    std::unique_ptr<QRhiBuffer> colours;
    std::unique_ptr<QRhiBuffer> indices;
  };
  void release();
  /* the pipelines, bindings and uniform ring for pass_ and samples_; false
     if one cannot be made (the caller lets go of what was made) */
  bool make_fixed();
  void release_fixed();
  void drop_tiles();
  void drop_batch();
  Prepared prepare(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels);
  std::unique_ptr<QRhiBuffer> make_buffer(QRhiBuffer::UsageFlags usage, quint32 size,
                                          QRhiBuffer::Type type = QRhiBuffer::Static);
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
  int copies_ = 1;         /* the copies the QRhi keeps of each static buffer */
  int dynamic_copies_ = 1; /* ... and of each dynamic buffer */
  int samples_ = 1;        /* the sample count of the render targets */
  /* the pipelines of the batch's draws, each without and with the depth test */
  enum Pipeline {
    kThinLines,
    kThinStrip,
    kPoints,
    kTriangles,
    kWideStrip,
    kWidePairs,
    kContourThin,
    kContourWide,
    kStippleThin,
    kStippleWide,
    kMarkers,
    kPipelines
  };
  struct MeshGpu {
    struct mbv_draw_shape shape;
    GluVariant variant = GluVariant::Sgi;
    std::unique_ptr<QRhiBuffer> buffer;
    quint32 vertices = 0;
    uint64_t used = 0;
  };
  bool make_pipelines(QRhiRenderPassDescriptor *pass, int samples);
  /* a dynamic vertex buffer of exactly size bytes, made again when the size
     changes, none for 0 bytes; false if it cannot be made */
  bool fit(std::unique_ptr<QRhiBuffer> *buffer, quint32 size);
  /* the bytes the batch's buffers and the contours hold, every copy counted */
  void held_stats();
  Prepared prepare_batch(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels);
  /* the mesh of a marker shape, made once and kept; null if none, with
     *failed set if its buffer could not be made (then nothing is kept) */
  MeshGpu *mesh(QRhiResourceUpdateBatch *updates, const struct mbv_draw_shape &shape, bool *failed);
  /* the uniform ring of this frame; false if there is none, or if it had to
     grow and could not */
  bool write_slots(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels);
  QRhiBuffer *source(DrawSource source) const;
  void bind(QRhiCommandBuffer *cb, QRhiGraphicsPipeline *pipeline, int slot, QSize pixels);
  void record_terrain(QRhiCommandBuffer *cb, QSize pixels, int slot, int depth);

  std::unique_ptr<QRhiBuffer> uniforms_; /* the ring of uniform slots, one per drawable command */
  quint32 slot_stride_ = 0;
  quint32 slot_capacity_ = 0;
  std::vector<unsigned char> slot_bytes_;
  std::unique_ptr<QRhiShaderResourceBindings> bindings_;
  std::unique_ptr<QRhiGraphicsPipeline> terrain_[2]; /* without and with the depth test */
  std::unique_ptr<QRhiGraphicsPipeline> pipelines_[kPipelines][2];
  bool points_ = false; /* the QRhi sets the point size in the vertex shader */
  /* the batch's buffers: the corners of a wide line's quad, the vertices
     and instances, the closing segments of loops, the quads as triangles,
     the stipple pairs, the contours */
  std::unique_ptr<QRhiBuffer> corners_;
  std::unique_ptr<QRhiBuffer> batch_vertices_;
  std::unique_ptr<QRhiBuffer> batch_instances_;
  std::unique_ptr<QRhiBuffer> closings_;
  std::unique_ptr<QRhiBuffer> triangles_;
  std::unique_ptr<QRhiBuffer> stipple_;
  std::unique_ptr<QRhiBuffer> contours_;
  BatchPlan batch_;
  bool batch_ready_ = false; /* the batch's buffers hold the frame driver's batch */
  QSize batch_pixels_;
  double batch_ratio_ = 0.0;
  int contour_rez_ = 0; /* the contours in contours_: level, generation, vertices */
  uint64_t contour_generation_ = 0;
  size_t contour_vertices_ = 0;
  std::vector<std::unique_ptr<MeshGpu>> meshes_;
  std::vector<MeshGpu *> item_meshes_; /* the mesh of each draw of the plan (markers) */
  uint64_t mesh_clock_ = 0;
  double view_ratio_ = 0.0;
  Style style_;
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
