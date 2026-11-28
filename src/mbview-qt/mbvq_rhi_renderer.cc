/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_rhi_renderer.cc  11/15/2026
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
 * The QRhi backend of mbview-qt, renderer layer: the map renderer
 * (mbvq_rhi_renderer.h) and the offscreen renderer of the tests (mbvq_rhi.h).
 */

#include "mbvq_rhi_renderer.h"

#include <QColor>
#include <QFile>
#include <QImage>
#include <QMatrix4x4>
#include <algorithm>
#include <cstdint>
#include <cstring>

#include <rhi/qrhi_platform.h>

#include "mbvq_bridge.h"

namespace mbvq {
namespace rhi {

namespace {

QShader load_shader(const char *name) {
  QFile file(QString::fromLatin1(name));
  return file.open(QIODevice::ReadOnly) ? QShader::fromSerialized(file.readAll()) : QShader();
}

/* a static buffer of a size, made; null if it cannot be made */
std::unique_ptr<QRhiBuffer> static_buffer(QRhi *rhi, QRhiBuffer::UsageFlags usage, quint32 size) {
  std::unique_ptr<QRhiBuffer> buffer(rhi->newBuffer(QRhiBuffer::Static, usage, size));
  if (!buffer->create())
    buffer.reset();
  return buffer;
}

}  // namespace

/*--------------------------------------------------------------------*/
bool ViewRenderer::compatible(const QRhiRenderPassDescriptor *pass) const {
  return pass_ != nullptr && pass != nullptr && pass_->isCompatible(pass);
}

/*--------------------------------------------------------------------*/
MapRenderer::MapRenderer(QRhi *rhi, const RenderOptions &options)
    : ViewRenderer(rhi), options_(options), float_colours_(options.float_terrain_colours) {
  plan_.set_float_colours(float_colours_);
}

MapRenderer::~MapRenderer() { release(); }

void MapRenderer::release() {
  terrain_[0].reset();
  terrain_[1].reset();
  bindings_.reset();
  uniforms_.reset();
  drop_tiles();
  pass_.reset();
}

void MapRenderer::drop_tiles() {
  /* the GPU copies and the plan that says what they hold: the next frame
     plans, prepares and uploads every tile again */
  tiles_.clear();
  id_positions_.reset();
  id_colours_.reset();
  id_vertices_ = 0;
  plan_ = TerrainPlan();
  plan_.set_float_colours(float_colours_);
  generation_ = -1;
}

std::unique_ptr<QRhiBuffer> MapRenderer::make_buffer(QRhiBuffer::UsageFlags usage, quint32 size) {
  /* RenderOptions::fail_buffers: as when the GPU memory runs out */
  if (options_.fail_buffers)
    return nullptr;
  return static_buffer(rhi_, usage, size);
}

int MapRenderer::resources() const {
  int n = 0;
  for (const std::unique_ptr<QRhiGraphicsPipeline> &pipeline : terrain_)
    n += pipeline != nullptr;
  n += bindings_ != nullptr;
  n += uniforms_ != nullptr;
  n += pass_ != nullptr;
  n += id_positions_ != nullptr;
  n += id_colours_ != nullptr;
  for (const TileGpu &tile : tiles_)
    n += (tile.positions != nullptr) + (tile.colours != nullptr) + (tile.indices != nullptr);
  return n;
}

bool MapRenderer::create(QRhiRenderPassDescriptor *pass, int samples) {
  release();

  /* the wide lines, markers and point quads that later complete the map are
     instanced: without instancing nothing is drawn, rather than a map that
     lacks them */
  if (!rhi_->isFeatureSupported(QRhi::Instancing))
    return false;

  pass_.reset(pass->newCompatibleRenderPassDescriptor());
  uniforms_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer,
                                  static_cast<quint32>(rhi_->ubufAligned(kSlotBytes))));
  if (!uniforms_->create()) {
    release();
    return false;
  }
  bindings_.reset(rhi_->newShaderResourceBindings());
  bindings_->setBindings({QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(
      0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, uniforms_.get(),
      kSlotBytes)});
  if (!bindings_->create() || options_.fail_create) {
    release();
    return false;
  }

  const QShader vertex = load_shader(":/mbvq/mbvq_terrain.vert.qsb");
  const QShader fragment = load_shader(":/mbvq/mbvq_colour.frag.qsb");
  if (!vertex.isValid() || !fragment.isValid()) {
    release();
    return false;
  }
  /* the terrain: a position (3 floats) and a colour (4 bytes, or 4 floats for
     comparisons) per node, each in a buffer of its own */
  QRhiVertexInputLayout layout;
  const quint32 colour_stride = float_colours_ ? static_cast<quint32>(4 * sizeof(float)) : 4;
  layout.setBindings({{3 * sizeof(float)}, {colour_stride}});
  layout.setAttributes(
      {{0, 0, QRhiVertexInputAttribute::Float3, 0},
       {1, 1, float_colours_ ? QRhiVertexInputAttribute::Float4 : QRhiVertexInputAttribute::UNormByte4, 0}});
  /* without the depth test (2D), and with it (3D and the spheroid, mbview_drawdata) */
  for (int depth = 0; depth < 2; depth++) {
    std::unique_ptr<QRhiGraphicsPipeline> pipeline(rhi_->newGraphicsPipeline());
    pipeline->setShaderStages({{QRhiShaderStage::Vertex, vertex}, {QRhiShaderStage::Fragment, fragment}});
    pipeline->setVertexInputLayout(layout);
    pipeline->setShaderResourceBindings(bindings_.get());
    pipeline->setRenderPassDescriptor(pass_.get());
    pipeline->setSampleCount(samples);
    pipeline->setTopology(QRhiGraphicsPipeline::Triangles);
    pipeline->setDepthTest(depth != 0);
    pipeline->setDepthWrite(depth != 0);
    pipeline->setDepthOp(QRhiGraphicsPipeline::Less);
    if (!pipeline->create()) {
      release();
      return false;
    }
    terrain_[depth] = std::move(pipeline);
  }
  return true;
}

bool MapRenderer::upload(QRhiResourceUpdateBatch *updates, TileUpload *upload) {
  TileGpu &gpu = tiles_[upload->tile];
  const TerrainPlan::Tile &tile = plan_.tiles()[upload->tile];

  /* a tile without triangles holds nothing */
  if (upload->indices && upload->index.empty()) {
    gpu = TileGpu();
    return true;
  }
  const quint32 nodes = static_cast<quint32>(tile.ni) * static_cast<quint32>(tile.nj);
  if (upload->positions) {
    if (gpu.positions == nullptr && (gpu.positions = make_buffer(QRhiBuffer::VertexBuffer, 12 * nodes)) == nullptr)
      return false;
    updates->uploadStaticBuffer(gpu.positions.get(), upload->xyz.data());
  }
  if (upload->colours) {
    const quint32 size = (float_colours_ ? static_cast<quint32>(4 * sizeof(float)) : 4) * nodes;
    if (gpu.colours == nullptr && (gpu.colours = make_buffer(QRhiBuffer::VertexBuffer, size)) == nullptr)
      return false;
    if (float_colours_)
      updates->uploadStaticBuffer(gpu.colours.get(), upload->rgba_float.data());
    else
      updates->uploadStaticBuffer(gpu.colours.get(), upload->rgba.data());
  }
  if (upload->indices) {
    const quint32 size = static_cast<quint32>(2 * upload->index.size());
    if (gpu.indices == nullptr || gpu.indices->size() != size) {
      gpu.indices.reset();
      if ((gpu.indices = make_buffer(QRhiBuffer::IndexBuffer, size)) == nullptr)
        return false;
    }
    updates->uploadStaticBuffer(gpu.indices.get(), upload->index.data());
  }
  return true;
}

bool MapRenderer::upload_triangle_ids(QRhiResourceUpdateBatch *updates, const FrameInput &input) {
  /* every triangle of the full-resolution terrain, in the colour that names
     it (RenderOptions::triangle_ids); the nodes were prepared by the plan */
  const struct mbvq_grid &grid = input.terrain.grid;
  id_positions_.reset();
  id_colours_.reset();
  id_vertices_ = 0;
  if (grid.data == nullptr || grid.n_columns < 2 || grid.n_rows < 2)
    return true;
  const int nc = grid.n_columns;
  const int nr = grid.n_rows;
  std::vector<unsigned char> valid(static_cast<size_t>(nc) * static_cast<size_t>(nr));
  mbvq_c_node_validity(input.terrain.instance, 0, nc - 1, 0, nr - 1, 1, valid.data());
  std::vector<float> xyz;
  std::vector<unsigned char> rgba;
  std::vector<float> rgba_float;
  auto add = [&](int k, uint32_t value) {
    xyz.insert(xyz.end(), {grid.x[k], grid.y[k], grid.z[k]});
    const unsigned char bytes[4] = {static_cast<unsigned char>(value & 255),
                                    static_cast<unsigned char>((value >> 8) & 255),
                                    static_cast<unsigned char>((value >> 16) & 255), 255};
    /* in the layout of the terrain pipeline's colours */
    for (unsigned char byte : bytes) {
      if (float_colours_)
        rgba_float.push_back(static_cast<float>(byte) / 255.0f);
      else
        rgba.push_back(byte);
    }
  };
  for (int i = 0; i + 1 < nc; i++) {
    for (int j = 0; j + 1 < nr; j++) {
      const int k = i * nr + j;
      const int l = (i + 1) * nr + j;
      const int m = k + 1;
      const int n = l + 1;
      const uint32_t id = 2 * static_cast<uint32_t>(i * (nr - 1) + j) + 1;
      if (valid[k] && valid[l] && valid[m]) {
        add(k, id);
        add(l, id);
        add(m, id);
      }
      if (valid[l] && valid[n] && valid[m]) {
        add(l, id + 1);
        add(n, id + 1);
        add(m, id + 1);
      }
    }
  }
  if (xyz.empty())
    return true;
  id_vertices_ = static_cast<quint32>(xyz.size() / 3);
  id_positions_ = make_buffer(QRhiBuffer::VertexBuffer, 12 * id_vertices_);
  const quint32 colour_size = float_colours_ ? static_cast<quint32>(4 * sizeof(float)) : 4;
  id_colours_ = make_buffer(QRhiBuffer::VertexBuffer, colour_size * id_vertices_);
  if (id_positions_ == nullptr || id_colours_ == nullptr)
    return false;
  updates->uploadStaticBuffer(id_positions_.get(), xyz.data());
  if (float_colours_)
    updates->uploadStaticBuffer(id_colours_.get(), rgba_float.data());
  else
    updates->uploadStaticBuffer(id_colours_.get(), rgba.data());
  return true;
}

Prepared MapRenderer::frame(MapFrame *frame, QRhiResourceUpdateBatch *updates, bool continued, QSize pixels) {
  FrameInput input;
  const bool refused = !frame->begin(continued, pixels.width(), pixels.height(), &input);
  const Clock::time_point start = Clock::now();
  const Prepared prepared = prepare(updates, input, pixels);
  /* the times of this frame so far; the pass's come from the caller */
  stats_.fill_ms = frame->fill_ms();
  stats_.marks_ms = frame->marks_ms();
  stats_.gather_ms = milliseconds(start, Clock::now()) - stats_.prepare_ms;
  set_pass_times(0.0, 0.0, 0.0, -1.0, 0.0, 0.0);
  if (prepared == Prepared::Failed) {
    /* this frame's uploads go with the batch; the tiles are planned and
       uploaded again from the core's arrays in a later frame */
    updates->release();
    drop_tiles();
    stats_.failed = true;
    stats_.tiles_drawn = 0;
    stats_.triangles = 0;
    stats_.resources = resources();
    return prepared;
  }
  /* a refused fill: the batch this view has, and another frame for the fill */
  return refused ? Prepared::More : prepared;
}

Prepared MapRenderer::prepare(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels) {
  stats_.frames++;
  stats_.failed = false;
  stats_.fills += input.filled ? 1 : 0;
  stats_.fill_rez = input.rez;
  stats_.terrain_slot = input.terrain_slot;
  stats_.depth_test = input.depth_test;
  stats_.groups = 0;
  if (input.batch != nullptr)
    for (int c = 0; c < input.batch->num_commands; c++)
      stats_.groups += input.batch->commands[c].kind == MBV_DRAW_GROUP;
  draw_terrain_ = input.terrain_slot;
  depth_test_ = input.depth_test;

  /* the terrain: what the marks made stale, within the upload budget (all of
     it at once for the triangle colours of a test); only after a fill that
     drew the terrain, which also projected the data */
  std::vector<TileUpload> uploads;
  PlanStats plan_stats;
  const bool more = draw_terrain_ && plan_.plan(input.terrain, options_.triangle_ids ? SIZE_MAX : options_.upload_budget,
                                                &uploads, &plan_stats);
  stats_.prepare_ms = plan_stats.prepare_ms;
  if (generation_ != plan_.generation()) {
    tiles_.clear();
    tiles_.resize(plan_.tiles().size());
    generation_ = plan_.generation();
  }
  for (TileUpload &upload : uploads)
    if (!this->upload(updates, &upload))
      return Prepared::Failed;
  stats_.tiles = static_cast<int>(plan_.tiles().size());
  stats_.tiles_waiting = plan_stats.tiles_waiting;
  stats_.tiles_prepared += plan_stats.tiles_prepared;
  stats_.uploads_positions += plan_stats.uploads_positions;
  stats_.uploads_colours += plan_stats.uploads_colours;
  stats_.uploads_indices += plan_stats.uploads_indices;
  stats_.bytes_positions += plan_stats.bytes_positions;
  stats_.bytes_colours += plan_stats.bytes_colours;
  stats_.bytes_indices += plan_stats.bytes_indices;
  stats_.bytes_largest_frame = std::max(
      stats_.bytes_largest_frame, plan_stats.bytes_positions + plan_stats.bytes_colours + plan_stats.bytes_indices);
  if (options_.triangle_ids && !upload_triangle_ids(updates, input))
    return Prepared::Failed;

  /* the uniform slot: the camera of the core corrected for this API's clip
     space, multiplied in double and stored as floats */
  const QMatrix4x4 correction = rhi_->clipSpaceCorrMatrix();
  double clip[16];
  for (int i = 0; i < 16; i++)
    clip[i] = correction.constData()[i];
  double mvp[16];
  multiply(clip, input.camera, mvp);
  float slot[kSlotBytes / sizeof(float)] = {};
  for (int i = 0; i < 16; i++)
    slot[i] = static_cast<float>(mvp[i]);
  slot[19] = 1.0f; /* colour: opaque black */
  slot[20] = static_cast<float>(pixels.width());
  slot[21] = static_cast<float>(pixels.height());
  slot[22] = 1.0f; /* line width and point size: 1 physical pixel */
  slot[23] = 1.0f;
  updates->updateDynamicBuffer(uniforms_.get(), 0, kSlotBytes, slot);
  stats_.resources = resources();
  return more ? Prepared::More : Prepared::Done;
}

void MapRenderer::record(QRhiCommandBuffer *cb, QSize pixels) {
  stats_.tiles_drawn = 0;
  stats_.triangles = 0;
  if (!draw_terrain_ || terrain_[0] == nullptr)
    return;
  cb->setGraphicsPipeline(terrain_[depth_test_ ? 1 : 0].get());
  /* after every pipeline: Vulkan takes the scissor from the viewport */
  cb->setViewport(QRhiViewport(0, 0, static_cast<float>(pixels.width()), static_cast<float>(pixels.height())));
  const QRhiCommandBuffer::DynamicOffset slot(0, 0);
  cb->setShaderResources(bindings_.get(), 1, &slot);
  if (options_.triangle_ids) {
    if (id_vertices_ > 0) {
      const QRhiCommandBuffer::VertexInput inputs[] = {{id_positions_.get(), 0}, {id_colours_.get(), 0}};
      cb->setVertexInput(0, 2, inputs);
      cb->draw(id_vertices_);
      stats_.triangles = id_vertices_ / 3;
    }
    return;
  }
  /* the tiles whose every stream is as new as the marks; the others wait for
     their upload (a hole for a frame or two) */
  for (int t = 0; t < static_cast<int>(tiles_.size()); t++) {
    const TileGpu &gpu = tiles_[t];
    if (!plan_.drawable(t) || gpu.positions == nullptr || gpu.colours == nullptr || gpu.indices == nullptr)
      continue;
    const QRhiCommandBuffer::VertexInput inputs[] = {{gpu.positions.get(), 0}, {gpu.colours.get(), 0}};
    cb->setVertexInput(0, 2, inputs, gpu.indices.get(), 0, QRhiCommandBuffer::IndexUInt16);
    cb->drawIndexed(plan_.tiles()[t].index_count);
    stats_.tiles_drawn++;
    stats_.triangles += plan_.tiles()[t].index_count / 3;
  }
}

void MapRenderer::set_pass_times(double upload_ms, double record_ms, double render_ms, double gpu_ms, double begin_ms,
                                 double end_ms) {
  stats_.upload_ms = upload_ms;
  stats_.record_ms = record_ms;
  stats_.render_ms = render_ms;
  stats_.gpu_ms = gpu_ms;
  stats_.render_begin_ms = begin_ms;
  stats_.render_end_ms = end_ms;
}

}  // namespace rhi

/*--------------------------------------------------------------------*/
/* the offscreen renderer of the tests */
struct OffscreenRenderer::Impl {
  std::unique_ptr<QRhi> rhi;
  std::unique_ptr<QRhiTexture> texture;
  std::unique_ptr<QRhiRenderBuffer> depth;
  std::unique_ptr<QRhiRenderPassDescriptor> pass;
  std::unique_ptr<QRhiTextureRenderTarget> target;
  QSize size;
  std::unique_ptr<rhi::MapFrame> frame;
  std::unique_ptr<rhi::MapRenderer> renderer;
  RenderStats stats;
  int last_frames = 0;

  void drop_target() {
    target.reset();
    pass.reset();
    depth.reset();
    texture.reset();
    size = QSize();
  }
  bool make_target(QSize pixels) {
    drop_target();
    texture.reset(rhi->newTexture(QRhiTexture::RGBA8, pixels, 1,
                                  QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    depth.reset(rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, pixels, 1));
    if (!texture->create() || !depth->create()) {
      drop_target();
      return false;
    }
    QRhiTextureRenderTargetDescription description(QRhiColorAttachment(texture.get()));
    description.setDepthStencilBuffer(depth.get());
    target.reset(rhi->newTextureRenderTarget(description));
    pass.reset(target->newCompatibleRenderPassDescriptor());
    target->setRenderPassDescriptor(pass.get());
    if (!target->create()) {
      drop_target();
      return false;
    }
    size = pixels;
    return true;
  }
  void drop_renderer() {
    if (renderer != nullptr) {
      stats = renderer->stats();
      renderer.reset();
    }
    stats.resources = 0;
  }
};

OffscreenRenderer::OffscreenRenderer(Api api) : impl_(new Impl) {
  if (api == Api::Null) {
    QRhiNullInitParams params;
    impl_->rhi.reset(QRhi::create(QRhi::Null, &params));
  }
#if QT_CONFIG(metal)
  else {
    QRhiMetalInitParams params;
    impl_->rhi.reset(QRhi::create(QRhi::Metal, &params));
  }
#endif
  if (impl_->rhi != nullptr) {
    /* the renderer's resources go before the QRhi itself (destroy_rhi()) */
    Impl *d = impl_.get();
    impl_->rhi->addCleanupCallback(d, [d](QRhi *) {
      d->drop_renderer();
      d->drop_target();
    });
  }
}

OffscreenRenderer::~OffscreenRenderer() {
  impl_->drop_renderer();
  impl_->drop_target();
  impl_->frame.reset();
  if (impl_->rhi != nullptr)
    impl_->rhi->removeCleanupCallback(impl_.get());
}

bool OffscreenRenderer::valid() const { return impl_->rhi != nullptr; }

const char *OffscreenRenderer::backend_name() const { return impl_->rhi != nullptr ? impl_->rhi->backendName() : "none"; }

int OffscreenRenderer::last_frames() const { return impl_->last_frames; }

const RenderStats &OffscreenRenderer::stats() const {
  if (impl_->renderer != nullptr)
    impl_->stats = impl_->renderer->stats();
  return impl_->stats;
}

void OffscreenRenderer::release() { impl_->drop_renderer(); }

void OffscreenRenderer::destroy_rhi() {
  impl_->rhi.reset();
  impl_->frame.reset();
}

bool OffscreenRenderer::render_map(size_t instance, int width, int height, const RenderOptions &options, QImage *image,
                                   int max_frames) {
  Impl &d = *impl_;
  d.last_frames = 0;
  if (d.rhi == nullptr || width < 1 || height < 1)
    return false;
  const QSize pixels(width, height);
  if (pixels != d.size && !d.make_target(pixels))
    return false;
  if (d.frame == nullptr || d.frame->instance() != instance)
    d.frame = std::make_unique<rhi::MapFrame>(instance);
  if (d.renderer != nullptr &&
      (!d.renderer->compatible(d.pass.get()) || d.renderer->float_colours() != options.float_terrain_colours))
    d.drop_renderer();
  if (d.renderer == nullptr) {
    d.renderer = std::make_unique<rhi::MapRenderer>(d.rhi.get(), options);
    if (!d.renderer->create(d.pass.get(), 1)) {
      /* what the failed create() left behind: nothing, by its contract */
      const int held = d.renderer->resources();
      d.drop_renderer();
      d.stats.failed = true;
      d.stats.held_after_failure = held;
      return false;
    }
  }
  d.renderer->set_options(options);

  QRhiReadbackResult result;
  bool more = true;
  for (int f = 0; f < max_frames && more; f++) {
    QRhiCommandBuffer *cb = nullptr;
    if (d.rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess)
      return false;
    /* the steps of a view's frame; the frames after the first only continue
       uploads, and reuse the batch while nothing changed */
    const rhi::Clock::time_point start = rhi::Clock::now();
    QRhiResourceUpdateBatch *updates = d.rhi->nextResourceUpdateBatch();
    const rhi::Prepared prepared = d.renderer->frame(d.frame.get(), updates, f > 0, pixels);
    if (prepared == rhi::Prepared::Failed) {
      /* nothing drawn; the renderer keeps its pipelines, and the next
         render_map() uploads everything again */
      cb->beginPass(d.target.get(), Qt::white, {1.0f, 0});
      cb->endPass();
      d.rhi->endOffscreenFrame();
      d.last_frames++;
      d.stats = d.renderer->stats();
      return false;
    }
    more = prepared == rhi::Prepared::More;
    const rhi::Clock::time_point pass = rhi::Clock::now();
    cb->beginPass(d.target.get(), Qt::white, {1.0f, 0}, updates);
    const rhi::Clock::time_point commands = rhi::Clock::now();
    d.renderer->record(cb, pixels);
    QRhiResourceUpdateBatch *readback = nullptr;
    if (image != nullptr && (!more || f + 1 == max_frames)) {
      readback = d.rhi->nextResourceUpdateBatch();
      readback->readBackTexture(QRhiReadbackDescription(d.texture.get()), &result);
    }
    cb->endPass(readback);
    const rhi::Clock::time_point end = rhi::Clock::now();
    /* as in a view, the GPU time of the frame the QRhi completed last */
    const double gpu = cb->lastCompletedGpuTime();
    d.renderer->set_pass_times(rhi::milliseconds(pass, commands), rhi::milliseconds(commands, end),
                               rhi::milliseconds(start, end), gpu > 0.0 ? 1000.0 * gpu : -1.0,
                               rhi::milliseconds(start), rhi::milliseconds(end));
    /* waits for the GPU, so the read-back is complete */
    d.rhi->endOffscreenFrame();
    d.last_frames++;
  }
  d.stats = d.renderer->stats();

  if (image != nullptr) {
    const int w = result.pixelSize.width();
    const int h = result.pixelSize.height();
    if (w != width || h != height || result.data.size() < static_cast<qsizetype>(w) * h * 4)
      return false;
    /* image rows top down: a framebuffer with y up holds them bottom up */
    const bool flip = d.rhi->isYUpInFramebuffer();
    QImage frame(w, h, result.format == QRhiTexture::BGRA8 ? QImage::Format_ARGB32 : QImage::Format_RGBA8888);
    for (int y = 0; y < h; y++)
      std::memcpy(frame.scanLine(y), result.data.constData() + static_cast<qsizetype>(flip ? h - 1 - y : y) * w * 4,
                  static_cast<size_t>(w) * 4);
    *image = frame;
  }
  return true;
}

}  // namespace mbvq
