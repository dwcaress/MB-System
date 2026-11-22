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
 * BatchRenderer draws a view's batch (mbv_draw_batch.h): the commands in
 * stream order (BatchPlan, mbvq_rhi_model.h), each with a uniform slot of its
 * own in one dynamic buffer per renderer, written once per frame; the contour
 * slot from the finest contours made, uploaded only when they change; lines
 * of one physical pixel with the line topology, wider ones as quads
 * extruded along their minor axis in the shaders; points of the point size,
 * or quads where the API has no point size; markers instanced per run, with
 * GLU's meshes (mbvq_rhi_glu.h). Line widths, point sizes and stipple
 * factors are scaled by the view's device pixel ratio (Style). The batch's
 * vertices and instances go up in the frame that filled them, whole: a batch
 * is drawn in the frame that filled it. They, what the plan made of them and
 * the contours are dynamic buffers of exactly the size of their data, and the
 * uniform ring one grown by doubling; each is always written whole. The
 * marker meshes and the corners of the quads of wide lines and of points are
 * static buffers, uploaded once. Qt's Metal, Vulkan and Direct3D 12 backends
 * keep a dynamic buffer once per frame slot and drop the writes still
 * pending for a slot when the whole buffer is written, so a slot that no
 * frame of the view uses for a while (a window that draws frames of its own
 * between the view's) holds one pending write, not one per frame. The
 * stipple values, which follow the camera, go up again whenever the camera
 * changed, also in a frame that draws the batch it has.
 *
 * The three views add what the slots of their batches stand for:
 * - MapRenderer, the map view: the terrain slot from the tile levels of the
 *   terrain plan, one stride per frame. The tile levels are static buffers,
 *   uploaded when they change. Qt's Metal backend keeps a static upload for
 *   each frame slot until a frame on that slot binds the buffer, and a whole
 *   upload does not drop the older ones: tile streams uploaded again in
 *   consecutive frames (shading, exaggeration and colour changes) still pile
 *   up there, one upload per frame, while something else puts every frame
 *   of the view on one frame slot, and the uploads still pending for a level
 *   no longer drawn stay until that level is drawn on that slot again or
 *   released. The plan shares the process-wide memory budget of the tile
 *   levels with the other renderers; the batch's buffers and the contours
 *   lie outside it.
 * - ProfileRenderer, the profile view: its batch alone.
 * - SoundingsRenderer, the 3D soundings view: the soundings slot as points
 *   (or point quads) of every sounding in the order of the array, or, in the
 *   frames of a camera press that the GPU-time fallback found GPU-slow
 *   (Metal, Vulkan, Direct3D 12), of every 2nd, 4th, 8th or 16th (those
 *   whose index is a multiple of it, record_soundings; the others are not
 *   drawn at all), with the depth test, the soundings the core hides
 *   (alpha 0) moved out of the clip volume; the profile lines from index
 *   lists over the soundings' positions,
 *   or, wider than one physical pixel, as quads instanced over the same
 *   positions with a colour per pair. The soundings' buffers are chunks that
 *   are always written whole (SoundingsPlan). The positions and colours are
 *   static buffers with storage use where the backend can splat (chunk_use;
 *   Qt refuses dynamic buffers with that use, and immutable ones are for data
 *   that never change): on Metal Qt keeps such a buffer once, not once per
 *   frame slot, and copies an upload into it at the buffer's next bind; a
 *   later upload does not drop one still waiting there, so the frame binds
 *   every chunk it wrote (record_soundings binds the ones it does not draw,
 *   without a draw); a frame without the soundings slot leaves them waiting
 *   until a later frame binds them, and a write into a buffer whose upload
 *   still waits makes the buffer again first (write_buffer). One copy is
 *   enough because every frame that binds the chunks is an offscreen frame,
 *   which waits for the GPU (QRhiWidget's paint and grab, the tests'
 *   renderer); the window's own frames on the same QRhi do not wait but draw
 *   only the widgets' textures (Qt keeps every other buffer once per frame
 *   slot, since writing a buffer that a frame still in flight reads is not
 *   safe). Where the backend cannot splat, and for the index lists and
 *   the colours per pair, the buffers are dynamic on Metal, so that a whole
 *   write drops the writes still pending for a frame slot, and static
 *   elsewhere, where the GPU then reads them from its own memory (Qt's
 *   Vulkan backend also keeps a host staging copy of a static buffer for
 *   each frame slot it was uploaded in). On a Mac whose GPU has memory of its
 *   own (not an Apple GPU) Qt makes a dynamic buffer shared memory, read from
 *   system memory in every frame, and a static one managed memory, copied to
 *   the GPU after each upload (not tested).
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
  Failed /* a buffer could not be made, or the 3D soundings not read: nothing is drawn (the renderers' frame()) */
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

/* the drawing of a view's batch, shared by the three views (above) */
class BatchRenderer : public ViewRenderer {
 public:
  BatchRenderer(QRhi *rhi, const RenderOptions &options);
  ~BatchRenderer() override;

  bool create(QRhiRenderPassDescriptor *pass, int samples) override;
  int resources() const override;
  /* of them: pipelines, bindings, uniforms and pass; the batch's buffers and meshes */
  int fixed_resources() const;
  int batch_resources() const;
  /* the options of the next frames (fail_create counts only when the
     renderer or its pipelines are made, point_quads and chunk_storage only
     when its pipelines are made, float_terrain_colours only when the
     renderer is made) */
  void set_options(const RenderOptions &options) { options_ = options; }
  /* the view's device pixel ratio (devicePixelRatioF()) for the next frames;
     <= 0: RenderOptions::pixel_ratio */
  void set_pixel_ratio(double ratio) { view_ratio_ = ratio; }
  /* the point-size control of the 3D soundings view for the next frames
     (RenderOptions::soundings_point_size) */
  void set_point_size(double logical) { options_.soundings_point_size = logical; }

  /* draw inside the render pass */
  void record(QRhiCommandBuffer *cb, QSize pixels);
  /* the times of the frame's pass, which its caller measures (RenderStats):
     the pass begun with the frame's updates, the commands recorded and the
     pass ended, the whole frame on the CPU, the GPU time the QRhi reports
     (-1 if none), and where the frame began and ended on the clock */
  virtual void set_pass_times(double upload_ms, double record_ms, double render_ms, double gpu_ms, double begin_ms,
                              double end_ms);
  /* The GPU-time fallback's signal. A view painted the frame whose render()
     ran last (its paint began at or before that render()): the paint's
     start and end (ms since the epoch of the clock, as begin_ms); the time
     from the end of render() to the paint's end is the frame's wait, in
     which QRhiWidget ends the offscreen frame and waits for it. */
  void frame_painted(double paint_start_ms, double paint_end_ms);
  /* the view's window submitted the frame (QRhiWidget::frameSubmitted) at
     a time of the same clock: the frame painted last is measured, from the
     paint's start to here, once (a window can submit frames this view did
     not paint) */
  virtual void frame_submitted(double at_ms);
  /* an offscreen frame of the tests ended (endOffscreenFrame, which waits
     for the GPU where the API does: Metal, Vulkan and Direct3D 12): its wait
     from the end of the pass and its total from its start are measured
     there; on other APIs only while a fake time is set
     (RenderOptions::gpu_wait_ms, gpu_total_ms), whose values then replace
     the measured ones in the rule's input only */
  void offscreen_ended(double start_ms, double end_ms);
  /* the GPU-time fallback's fake wait for the next frames
     (RenderOptions::gpu_wait_ms; a view's, mbvq::set_gpu_wait) */
  void set_gpu_wait(double wait_ms) { options_.gpu_wait_ms = wait_ms; }
  /* the view's press ended (its release, or a lost one), whatever frames
     come: the next pressed frame starts a new press, also where no frame
     without the press came between (a release and a new press before the
     next paint, or a frame that failed) */
  virtual void end_press() {}

  const RenderStats &stats() const { return stats_; }
  const BatchPlan &batch_plan() const { return batch_; }

 protected:
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
    kPointQuads,
    kPipelines
  };

  /* what a view adds: its own pipelines, made after the bindings with the
     batch's and let go of with them; the GPU resources it holds besides;
     the draws of its slots (the terrain, the soundings, the profile lines) */
  virtual bool make_view_pipelines(QRhiRenderPassDescriptor *pass, int samples);
  virtual void release_view_pipelines() {}
  virtual int view_fixed_resources() const { return 0; }
  virtual void record_view(QRhiCommandBuffer *cb, QSize pixels, const DrawItem &item, int depth);
  /* the view's GPU copies of the core's data, let go of when the renderer
     is made again or a frame fails */
  virtual void release_view() {}

  /* let go of everything of the batch part (pipelines, bindings, ring,
     buffers, meshes, the pass) */
  void release_batch_part();
  /* the pipelines, bindings and uniform ring for pass_ and samples_; false
     if one cannot be made (the caller lets go of what was made) */
  bool make_fixed();
  void release_fixed();
  void drop_batch();
  std::unique_ptr<QRhiBuffer> make_buffer(QRhiBuffer::UsageFlags usage, quint32 size,
                                          QRhiBuffer::Type type = QRhiBuffer::Static);
  /* the batch's draws, their buffers and the contours */
  Prepared prepare_batch(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels);
  /* the uniform ring of this frame; false if there is none, or if it had to
     grow and could not */
  bool write_slots(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels);
  /* a frame of the batch alone, as the frame driver's input gives it (the
     profile view, and the tests' batches); Failed drops the batch */
  Prepared frame_of_batch(const FrameInput &input, QRhiResourceUpdateBatch *updates, QSize pixels);
  /* the start of a frame: the statistics of the frame, the fixed set made
     again if a ring that could not be restored took it; false if it cannot
     be made */
  bool begin_frame(const FrameInput &input);
  /* the resources and held bytes into the statistics */
  void count_resources();
  void bind(QRhiCommandBuffer *cb, QRhiGraphicsPipeline *pipeline, int slot, QSize pixels);
  /* a buffer of exactly size bytes, made again when the size changes, none
     for 0 bytes; false if it cannot be made */
  bool fit(std::unique_ptr<QRhiBuffer> *buffer, quint32 size, QRhiBuffer::Type type = QRhiBuffer::Dynamic,
           QRhiBuffer::UsageFlags usage = QRhiBuffer::VertexBuffer);
  /* a shader of the resources (":/mbvq/...qsb") */
  static QShader shader(const char *name);
  /* the GPU-time fallback: a frame's measured wait and total into the
     statistics, then, with the fake times where set, to feed_gpu() */
  void measured(double wait_ms, double total_ms);
  /* a measured frame for the view's rule (the map, the 3D soundings) */
  virtual void feed_gpu(double /* wait_ms */, double /* total_ms */) {}
  /* the QRhi's backend as the model's plain value */
  WaitApi wait_api() const;
  /* the fallback can act in this frame (it acts only during the view's
     press): the QRhi's API waits for the GPU in endOffscreenFrame(), or the
     times are fake (gpu_wait_counts) */
  bool gpu_rule() const;
  /* uploads are copies on the GPU in the frame's command buffer (Vulkan,
     Direct3D 12), so a measured wait holds them (on Metal they are copied on
     the CPU in render()); false while a fake wait replaces the measured one */
  bool uploads_in_wait() const;
  /* the device pixel ratio the sizes are scaled with: the view's, else the
     options' */
  double pixel_ratio() const { return view_ratio_ > 0.0 ? view_ratio_ : options_.pixel_ratio; }
  /* a pipeline of the views' kind: the uniform slot's bindings, the pass,
     the depth test and write (Less) or none */
  std::unique_ptr<QRhiGraphicsPipeline> pipeline(const QShader &vertex, const QShader &fragment,
                                                 const QRhiVertexInputLayout &layout,
                                                 QRhiGraphicsPipeline::Topology topology, bool depth,
                                                 QRhiRenderPassDescriptor *pass, int samples);

  RenderOptions options_;
  int copies_ = 1;         /* the copies the QRhi keeps of each static buffer (one of a storage buffer) */
  int dynamic_copies_ = 1; /* ... and of each dynamic buffer */
  int samples_ = 1;        /* the sample count of the render targets */
  bool points_ = false;    /* points have the point size of the vertex shader (else quads) */
  bool point_quads_ = false; /* points are drawn as quads */
  std::unique_ptr<QRhiShaderResourceBindings> bindings_;
  std::unique_ptr<QRhiBuffer> corners_;      /* the corners of a wide line's quad */
  std::unique_ptr<QRhiBuffer> quad_corners_; /* the corners of a point's quad */
  Style style_;
  RenderStats stats_;

 private:
  /* the GPU-time fallback: a painted frame whose submission is still to
     come, its paint's start and its wait */
  bool paint_due_ = false;
  double paint_start_ms_ = 0.0;
  double paint_wait_ms_ = 0.0;
  struct MeshGpu {
    struct mbv_draw_shape shape;
    GluVariant variant = GluVariant::Sgi;
    std::unique_ptr<QRhiBuffer> buffer;
    quint32 vertices = 0;
    uint64_t used = 0;
  };
  bool make_pipelines(QRhiRenderPassDescriptor *pass, int samples);
  /* the bytes the batch's buffers and the contours hold, every copy counted */
  void held_stats();
  /* the mesh of a marker shape, made once and kept; null if none, with
     *failed set if its buffer could not be made (then nothing is kept) */
  MeshGpu *mesh(QRhiResourceUpdateBatch *updates, const struct mbv_draw_shape &shape, bool *failed);
  QRhiBuffer *source(DrawSource source) const;

  std::unique_ptr<QRhiBuffer> uniforms_; /* the ring of uniform slots, one per drawable command */
  quint32 slot_stride_ = 0;
  quint32 slot_capacity_ = 0;
  std::vector<unsigned char> slot_bytes_;
  std::unique_ptr<QRhiGraphicsPipeline> pipelines_[kPipelines][2];
  /* the batch's buffers: the vertices and instances, the closing segments
     of loops, the quads as triangles, the stipple pairs, the contours */
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
  /* the matrices the stipple values were made with */
  double stipple_camera_[16] = {};
  double stipple_projection_[16] = {};
  int contour_rez_ = 0; /* the contours in contours_: level, generation, vertices */
  uint64_t contour_generation_ = 0;
  size_t contour_vertices_ = 0;
  std::vector<std::unique_ptr<MeshGpu>> meshes_;
  std::vector<MeshGpu *> item_meshes_; /* the mesh of each draw of the plan (markers) */
  uint64_t mesh_clock_ = 0;
  double view_ratio_ = 0.0;
};

class MapRenderer final : public BatchRenderer {
 public:
  MapRenderer(QRhi *rhi, const RenderOptions &options);
  ~MapRenderer() override;

  int resources() const override;
  /* the terrain colours are floats (RenderOptions::float_terrain_colours) */
  bool float_colours() const { return float_colours_; }

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
  void set_pass_times(double upload_ms, double record_ms, double render_ms, double gpu_ms, double begin_ms,
                      double end_ms) override;
  /* a view's window submitted the frame (QRhiWidget::frameSubmitted) at a
     time of the same clock (ms since its epoch): the time from the end of
     render() to there, the wait for the frame's GPU work and the window's
     composition, counts among the frame's costs outside its preparing and
     gathering, which the next drag budget leaves room for; then the GPU-time
     fallback's measurement (BatchRenderer) */
  void frame_submitted(double at_ms) override;
  /* the drag ended: the next frame with a button down starts a new one
     (the drag budget's costs per level, the GPU-time fallback) */
  void end_press() override { dragging_ = false; }
  /* the view is shown (true) or hidden: a shown view's latest levels are
     kept from other views' evictions */
  void set_visible(bool visible) { plan_.set_visible(visible); }
  /* called when this renderer should draw a frame for the memory budget:
     another renderer evicted tile levels of this one, whose GPU memory this
     renderer's next frame frees; or the memory budget made this renderer's
     latest frame coarser and another let go of levels it kept. The view asks
     for that frame. */
  void set_frame_request(std::function<void()> request) { plan_.set_evicted_callback(std::move(request)); }

  const TerrainPlan &plan() const { return plan_; }

 protected:
  bool make_view_pipelines(QRhiRenderPassDescriptor *pass, int samples) override;
  void release_view_pipelines() override;
  int view_fixed_resources() const override;
  void record_view(QRhiCommandBuffer *cb, QSize pixels, const DrawItem &item, int depth) override;
  void release_view() override { drop_tiles(); }
  void feed_gpu(double wait_ms, double total_ms) override;

 private:
  struct TileGpu {
    std::unique_ptr<QRhiBuffer> positions;
    std::unique_ptr<QRhiBuffer> colours;
    std::unique_ptr<QRhiBuffer> indices;
  };
  void drop_tiles();
  Prepared prepare(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels);
  bool upload(QRhiResourceUpdateBatch *updates, TileUpload *upload);
  bool upload_triangle_ids(QRhiResourceUpdateBatch *updates, const FrameInput &input);
  void record_terrain(QRhiCommandBuffer *cb, QSize pixels, int slot, int depth);

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
  /* the GPU-time fallback over the terrain's levels, a rule for each
     zoom's level (started at a drag's first frame, with outside_level_ms_),
     the zoom's level of the last frame (whose rule its waits are fed to),
     and whether the last frame's wait holds uploads, which it then does not
     count */
  GpuFallback gpu_{kLevels - 1};
  int gpu_zoom_ = 0;
  bool gpu_skip_ = false;
  std::unique_ptr<QRhiGraphicsPipeline> terrain_[2]; /* without and with the depth test */
  std::unique_ptr<QRhiBuffer> id_positions_;
  std::unique_ptr<QRhiBuffer> id_colours_;
  quint32 id_vertices_ = 0;
  bool draw_terrain_ = false;
  bool depth_test_ = false;
};

/* the profile view: its batch alone */
class ProfileRenderer final : public BatchRenderer {
 public:
  ProfileRenderer(QRhi *rhi, const RenderOptions &options);
  ~ProfileRenderer() override;

  /* A frame of the profile before its render pass: the frame driver's fill
     (ProfileFrame::begin), then the batch's uploads and uniforms. Failed as
     for the map: updates was released and the batch dropped. More: the
     fill was refused (another fill runs); the frame draws the batch it has. */
  Prepared frame(ProfileFrame *frame, QRhiResourceUpdateBatch *updates, QSize pixels);
};

/* the 3D soundings view (above) */
class SoundingsRenderer final : public BatchRenderer {
 public:
  SoundingsRenderer(QRhi *rhi, const RenderOptions &options);
  ~SoundingsRenderer() override;

  int resources() const override;
  /* A frame of the soundings before its render pass: the frame driver's
     start of the frame (SoundingsFrame::begin; continued: the frame was asked
     for only to continue uploads), the soundings read where their change
     marks rose and written within the frame's budget (SoundingsPlan), then
     the batch's uploads and uniforms. More: writes wait for the next frames,
     a mark rose while its stream's upload ran (its data are read in a later
     frame, which nothing else asks for), or the fill was refused. Failed: a
     buffer could not be made, or a read of the soundings' data failed;
     updates was released, the soundings and the batch dropped, and a later
     frame reads and writes everything again. */
  Prepared frame(SoundingsFrame *frame, QRhiResourceUpdateBatch *updates, bool continued, QSize pixels);
  /* The program released its soundings while the view is hidden (Dismiss),
     so no frame comes to drop what this renderer holds of them: the GPU
     buffers and the data read go now (the next frame with soundings reads
     and writes everything again). */
  void release_soundings();
  /* the camera press ended: the next frame with one starts a new one (the
     GPU-time fallback) */
  void end_press() override { pressed_ = false; }

  const SoundingsPlan &plan() const { return plan_; }
  /* the profile lines are drawn wide (instanced quads with a colour per
     pair), not from the index lists */
  bool wide_lines() const { return wide_; }
  /* the option chunk_storage the pipelines were made with */
  int chunk_storage() const { return chunk_storage_; }

 protected:
  bool make_view_pipelines(QRhiRenderPassDescriptor *pass, int samples) override;
  void release_view_pipelines() override;
  int view_fixed_resources() const override;
  void record_view(QRhiCommandBuffer *cb, QSize pixels, const DrawItem &item, int depth) override;
  void release_view() override { drop_soundings(); }
  void feed_gpu(double wait_ms, double total_ms) override;

 private:
  /* the buffers of a chunk: positions and colours in two sets, the index
     lists of the black and the red pairs, the colours per pair */
  struct ChunkGpu {
    std::unique_ptr<QRhiBuffer> positions[2];
    std::unique_ptr<QRhiBuffer> colours[2];
    std::unique_ptr<QRhiBuffer> black;
    std::unique_ptr<QRhiBuffer> red;
    std::unique_ptr<QRhiBuffer> pair_colours;
    /* the indices the lists hold, and of them those of the pairs that stay
       inside the chunk's own soundings (all but a pair that reaches into
       the next chunk) */
    quint32 black_count = 0;
    quint32 red_count = 0;
    quint32 black_inner = 0;
    quint32 red_inner = 0;
    /* positions and colours (storage use, on Metal) with an upload that Qt
       still holds for the buffer's next bind; cleared wherever the renderer
       binds the buffer, since every bind runs its waiting writes */
    bool waiting_positions[2] = {false, false};
    bool waiting_colours[2] = {false, false};
  };
  /* the soundings as points or quads, at level 0 and at the levels of
     detail 1 to 4 (every 2nd, 4th, 8th or 16th sounding, kSoundingLevel1 +
     level - 1, of the kind in use), the thin and the wide profile lines */
  enum ViewPipeline {
    kSoundingPoints,
    kSoundingQuads,
    kProfileThin,
    kProfileWide,
    kSoundingLevel1,
    kSoundingLevel2,
    kSoundingLevel3,
    kSoundingLevel4,
    kViewPipelines
  };
  static constexpr int kSoundingLevels = 4;

  void drop_soundings();
  /* the soundings' data read again where their marks rose, and the uploads
     of this frame; false if a read failed or a buffer could not be made */
  bool prepare_soundings(QRhiResourceUpdateBatch *updates, SoundingsFrame *frame, const SoundingsInput &input);
  /* the data on the GPU are older than the core's marks of this frame: a
     mark rose while its stream's upload ran, so prepare_soundings() left it
     for a frame after that upload */
  bool behind(const SoundingsInput &input) const;
  /* the pairs of the lists, made local to their chunks: indices from each
     chunk's first sounding, the black and the red pairs of each chunk */
  void split_lists();
  bool write(QRhiResourceUpdateBatch *updates, const SoundingsPlan::Upload &upload);
  /* waiting: the chunk's flag where the buffer's uploads wait for its next
     bind (storage use on Metal), else null */
  bool write_buffer(QRhiResourceUpdateBatch *updates, std::unique_ptr<QRhiBuffer> *buffer, const void *data,
                    size_t bytes, QRhiBuffer::Type type, QRhiBuffer::UsageFlags usage, bool *waiting);
  void record_soundings(QRhiCommandBuffer *cb, QSize pixels, const DrawItem &item, int depth);
  void record_lines(QRhiCommandBuffer *cb, QSize pixels, const DrawItem &item, int depth);

  std::unique_ptr<QRhiGraphicsPipeline> view_pipelines_[kViewPipelines][2];
  bool uint_indices_ = false; /* 32-bit indices: the thin profile lines from index lists */
  /* how the positions and colours are made (chunk_use, or the other
     buffers' types with the option chunk_storage 0), the option it was made
     with, their type and use; the type of the index lists and the colours
     per pair (dynamic on Metal, static elsewhere) */
  ChunkUse chunk_use_;
  int chunk_storage_ = -1;
  QRhiBuffer::Type chunk_type_ = QRhiBuffer::Static;
  QRhiBuffer::UsageFlags chunk_usage_;
  QRhiBuffer::Type list_type_ = QRhiBuffer::Static;
  SoundingsPlan plan_;
  std::vector<ChunkGpu> chunks_;
  /* the core's data as last read, which the uploads take: positions,
     colours, the pairs of the profile lines (made local to their chunks,
     with each chunk's range in the two lists) and the colours per pair */
  std::vector<float> positions_;
  std::vector<unsigned char> colours_;
  std::vector<uint32_t> black_;
  std::vector<uint32_t> red_;
  std::vector<size_t> black_range_;
  std::vector<size_t> red_range_;
  std::vector<unsigned char> pair_colours_;
  /* what was read, and of which marks: the count of the allocation, the
     marks the positions and colours were read at, the marks, count and mode
     the lists were made for; versions of the lists and of what the GPU holds
     of them */
  bool allocated_ = false;
  uint64_t mask_ = 0;
  bool positions_read_ = false;
  uint64_t position_mark_ = 0;
  bool colours_read_ = false;
  uint64_t colour_mark_ = 0;
  bool lists_made_ = false;
  uint64_t lists_colour_ = 0;
  int lists_mode_ = -1;
  uint64_t lists_version_ = 0;
  uint64_t lists_uploaded_ = 0;
  uint64_t pair_colours_uploaded_ = 0;
  bool wide_ = false;
  /* the GPU-time fallback over the levels of detail (started at a camera
     press's first frame), the press of the last frame, the level of detail
     it draws, and whether its wait holds uploads */
  GpuFallback gpu_{kSoundingLevels};
  bool pressed_ = false;
  int level_ = 0;
  bool gpu_skip_ = false;
};

}  // namespace rhi
}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_RHI_RENDERER_H */
