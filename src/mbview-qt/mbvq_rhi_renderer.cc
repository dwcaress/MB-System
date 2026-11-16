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
#include <functional>
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

/* a buffer of a type and a size, made; null if it cannot be made */
std::unique_ptr<QRhiBuffer> new_buffer(QRhi *rhi, QRhiBuffer::Type type, QRhiBuffer::UsageFlags usage, quint32 size) {
  std::unique_ptr<QRhiBuffer> buffer(rhi->newBuffer(type, usage, size));
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
  std::fill(outside_level_ms_, outside_level_ms_ + kLevels, -1.0);
}

MapRenderer::~MapRenderer() { release(); }

void MapRenderer::release() {
  release_fixed();
  corners_.reset();
  batch_vertices_.reset();
  batch_instances_.reset();
  closings_.reset();
  triangles_.reset();
  stipple_.reset();
  contours_.reset();
  contour_rez_ = 0;
  contour_generation_ = 0;
  contour_vertices_ = 0;
  meshes_.clear();
  item_meshes_.clear();
  batch_ready_ = false;
  drop_tiles();
  pass_.reset();
}

void MapRenderer::release_fixed() {
  terrain_[0].reset();
  terrain_[1].reset();
  for (auto &pipeline : pipelines_) {
    pipeline[0].reset();
    pipeline[1].reset();
  }
  bindings_.reset();
  uniforms_.reset();
  slot_capacity_ = 0;
}

void MapRenderer::drop_tiles() {
  /* the GPU copies and the plan that says what they hold: the next frame
     plans, prepares and uploads every tile again */
  tiles_.clear();
  drawn_.clear();
  drawn_counts_.clear();
  id_positions_.reset();
  id_colours_.reset();
  id_vertices_ = 0;
  plan_.reset();
  generation_ = -1;
}

std::unique_ptr<QRhiBuffer> MapRenderer::make_buffer(QRhiBuffer::UsageFlags usage, quint32 size,
                                                     QRhiBuffer::Type type) {
  /* RenderOptions::fail_buffers: as when the GPU memory runs out */
  if (options_.fail_buffers)
    return nullptr;
  return new_buffer(rhi_, type, usage, size);
}

int MapRenderer::fixed_resources() const {
  int n = 0;
  for (const std::unique_ptr<QRhiGraphicsPipeline> &pipeline : terrain_)
    n += pipeline != nullptr;
  for (const auto &pipeline : pipelines_)
    n += (pipeline[0] != nullptr) + (pipeline[1] != nullptr);
  n += bindings_ != nullptr;
  n += uniforms_ != nullptr;
  n += pass_ != nullptr;
  return n;
}

int MapRenderer::batch_resources() const {
  int n = 0;
  for (const std::unique_ptr<QRhiBuffer> *buffer :
       {&corners_, &batch_vertices_, &batch_instances_, &closings_, &triangles_, &stipple_, &contours_})
    n += *buffer != nullptr;
  for (const std::unique_ptr<MeshGpu> &mesh : meshes_)
    n += mesh->buffer != nullptr;
  return n;
}

int MapRenderer::resources() const {
  int n = fixed_resources() + batch_resources();
  n += id_positions_ != nullptr;
  n += id_colours_ != nullptr;
  for (const std::array<TileGpu, kLevels> &tile : tiles_)
    for (const TileGpu &level : tile)
      n += (level.positions != nullptr) + (level.colours != nullptr) + (level.indices != nullptr);
  return n;
}

bool MapRenderer::create(QRhiRenderPassDescriptor *pass, int samples) {
  release();
  /* Qt's Metal backend keeps every buffer that is not a storage buffer once
     per frame in flight, so the memory budget counts the tile levels' bytes
     that many times there (the other backends keep a static buffer once) */
  copies_ = rhi_->backend() == QRhi::Metal ? rhi_->resourceLimit(QRhi::FramesInFlight) : 1;
  plan_.set_copies(copies_);
  /* a dynamic buffer is kept once per frame in flight by Qt's Metal, Vulkan
     and Direct3D 12 backends, once by OpenGL, and once with a copy in host
     memory beside it by Direct3D 11 (one frame in flight) */
  dynamic_copies_ = rhi_->resourceLimit(QRhi::FramesInFlight) + (rhi_->backend() == QRhi::D3D11 ? 1 : 0);

  /* the wide lines, markers and point quads that later complete the map are
     instanced: without instancing nothing is drawn, rather than a map that
     lacks them */
  if (!rhi_->isFeatureSupported(QRhi::Instancing))
    return false;

  pass_.reset(pass->newCompatibleRenderPassDescriptor());
  samples_ = samples;
  if (!make_fixed()) {
    release();
    return false;
  }
  return true;
}

bool MapRenderer::make_fixed() {
  /* the ring of uniform slots: one per drawable command, grown by doubling */
  slot_stride_ = static_cast<quint32>(rhi_->ubufAligned(kSlotBytes));
  slot_capacity_ = 64;
  uniforms_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, slot_stride_ * slot_capacity_));
  if (!uniforms_->create())
    return false;
  bindings_.reset(rhi_->newShaderResourceBindings());
  bindings_->setBindings({QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(
      0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, uniforms_.get(),
      kSlotBytes)});
  if (!bindings_->create() || options_.fail_create)
    return false;

  const QShader vertex = load_shader(":/mbvq/mbvq_terrain.vert.qsb");
  const QShader fragment = load_shader(":/mbvq/mbvq_colour.frag.qsb");
  if (!vertex.isValid() || !fragment.isValid())
    return false;
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
    pipeline->setSampleCount(samples_);
    pipeline->setTopology(QRhiGraphicsPipeline::Triangles);
    pipeline->setDepthTest(depth != 0);
    pipeline->setDepthWrite(depth != 0);
    pipeline->setDepthOp(QRhiGraphicsPipeline::Less);
    if (!pipeline->create())
      return false;
    terrain_[depth] = std::move(pipeline);
  }
  return make_pipelines(pass_.get(), samples_);
}

bool MapRenderer::make_pipelines(QRhiRenderPassDescriptor *pass, int samples) {
  /* the shaders of the batch's draws (section 6 of the design) */
  const QShader colour = load_shader(":/mbvq/mbvq_colour.frag.qsb");
  const QShader stipple = load_shader(":/mbvq/mbvq_stipple.frag.qsb");
  const QShader line = load_shader(":/mbvq/mbvq_line.vert.qsb");
  const QShader point = load_shader(":/mbvq/mbvq_point.vert.qsb");
  const QShader wide = load_shader(":/mbvq/mbvq_wide.vert.qsb");
  const QShader contour = load_shader(":/mbvq/mbvq_contour.vert.qsb");
  const QShader wide_contour = load_shader(":/mbvq/mbvq_wide_contour.vert.qsb");
  const QShader stipple_thin = load_shader(":/mbvq/mbvq_stipple_thin.vert.qsb");
  const QShader stipple_wide = load_shader(":/mbvq/mbvq_stipple_wide.vert.qsb");
  const QShader marker = load_shader(":/mbvq/mbvq_marker.vert.qsb");
  for (const QShader *shader :
       {&colour, &stipple, &line, &point, &wide, &contour, &wide_contour, &stipple_thin, &stipple_wide, &marker})
    if (!shader->isValid())
      return false;

  using A = QRhiVertexInputAttribute;
  using B = QRhiVertexInputBinding;
  const quint32 vertex = sizeof(struct mbv_draw_vertex);
  const quint32 stipple_vertex = sizeof(StippleVertex);
  /* a batch vertex: position and colour */
  QRhiVertexInputLayout batch;
  batch.setBindings({{vertex}});
  batch.setAttributes({{0, 0, A::Float3, 0}, {0, 1, A::Float3, 12}});
  /* a wide line: the quad's corner per vertex, the ends per instance from a
     strip (two bindings one vertex apart) or from pairs */
  QRhiVertexInputLayout wide_strip;
  wide_strip.setBindings({{2 * sizeof(float)}, B(vertex, B::PerInstance), B(vertex, B::PerInstance)});
  wide_strip.setAttributes(
      {{0, 0, A::Float2, 0}, {1, 1, A::Float3, 0}, {1, 2, A::Float3, 12}, {2, 3, A::Float3, 0}, {2, 4, A::Float3, 12}});
  QRhiVertexInputLayout wide_pairs;
  wide_pairs.setBindings({{2 * sizeof(float)}, B(2 * vertex, B::PerInstance)});
  wide_pairs.setAttributes(
      {{0, 0, A::Float2, 0}, {1, 1, A::Float3, 0}, {1, 2, A::Float3, 12}, {1, 3, A::Float3, 24}, {1, 4, A::Float3, 36}});
  /* the contours: pairs of positions */
  QRhiVertexInputLayout contour_thin;
  contour_thin.setBindings({{3 * sizeof(float)}});
  contour_thin.setAttributes({{0, 0, A::Float3, 0}});
  QRhiVertexInputLayout contour_wide;
  contour_wide.setBindings({{2 * sizeof(float)}, B(6 * sizeof(float), B::PerInstance)});
  contour_wide.setAttributes({{0, 0, A::Float2, 0}, {1, 1, A::Float3, 0}, {1, 2, A::Float3, 12}});
  /* stippled lines: pairs of vertices with OpenGL's stipple counter at each
     (BatchPlan::add_stipple) */
  QRhiVertexInputLayout stipple_pairs;
  stipple_pairs.setBindings({{stipple_vertex}});
  stipple_pairs.setAttributes({{0, 0, A::Float3, 0}, {0, 1, A::Float3, 12}, {0, 2, A::Float, 24}});
  QRhiVertexInputLayout stipple_quads;
  stipple_quads.setBindings({{2 * sizeof(float)}, B(2 * stipple_vertex, B::PerInstance)});
  stipple_quads.setAttributes({{0, 0, A::Float2, 0},
                               {1, 1, A::Float3, 0},
                               {1, 2, A::Float3, 12},
                               {1, 3, A::Float, 24},
                               {1, 4, A::Float3, 28},
                               {1, 5, A::Float3, 40},
                               {1, 6, A::Float, 52}});
  /* markers: the mesh (position and ring flag) per vertex, the instance
     (place and colour) per instance */
  QRhiVertexInputLayout markers;
  markers.setBindings({{4 * sizeof(float)}, B(sizeof(struct mbv_draw_instance), B::PerInstance)});
  markers.setAttributes({{0, 0, A::Float3, 0}, {0, 1, A::Float, 12}, {1, 2, A::Float3, 0}, {1, 3, A::Float3, 12}});

  using T = QRhiGraphicsPipeline::Topology;
  struct Spec {
    Pipeline index;
    const QShader *vertex;
    const QShader *fragment;
    const QRhiVertexInputLayout *layout;
    T topology;
  };
  points_ = rhi_->isFeatureSupported(QRhi::VertexShaderPointSize);
  const Spec specs[] = {{kThinLines, &line, &colour, &batch, T::Lines},
                        {kThinStrip, &line, &colour, &batch, T::LineStrip},
                        {kPoints, &point, &colour, &batch, T::Points},
                        {kTriangles, &line, &colour, &batch, T::Triangles},
                        {kWideStrip, &wide, &colour, &wide_strip, T::Triangles},
                        {kWidePairs, &wide, &colour, &wide_pairs, T::Triangles},
                        {kContourThin, &contour, &colour, &contour_thin, T::Lines},
                        {kContourWide, &wide_contour, &colour, &contour_wide, T::Triangles},
                        {kStippleThin, &stipple_thin, &stipple, &stipple_pairs, T::Lines},
                        {kStippleWide, &stipple_wide, &stipple, &stipple_quads, T::Triangles},
                        {kMarkers, &marker, &colour, &markers, T::Triangles}};
  for (const Spec &spec : specs) {
    /* points of a size need the point size of the vertex shader (Direct3D
       lacks it; point quads come with the soundings view) */
    if (spec.index == kPoints && !points_)
      continue;
    for (int depth = 0; depth < 2; depth++) {
      std::unique_ptr<QRhiGraphicsPipeline> pipeline(rhi_->newGraphicsPipeline());
      pipeline->setShaderStages(
          {{QRhiShaderStage::Vertex, *spec.vertex}, {QRhiShaderStage::Fragment, *spec.fragment}});
      pipeline->setVertexInputLayout(*spec.layout);
      pipeline->setShaderResourceBindings(bindings_.get());
      pipeline->setRenderPassDescriptor(pass);
      pipeline->setSampleCount(samples);
      pipeline->setTopology(spec.topology);
      pipeline->setDepthTest(depth != 0);
      pipeline->setDepthWrite(depth != 0);
      pipeline->setDepthOp(QRhiGraphicsPipeline::Less);
      if (!pipeline->create())
        return false;
      pipelines_[spec.index][depth] = std::move(pipeline);
    }
  }
  return true;
}

bool MapRenderer::fit(std::unique_ptr<QRhiBuffer> *buffer, quint32 size) {
  /* exactly the size, so that every write is one of the whole buffer: a
     batch of the same size again needs no new buffer, and one of another
     size holds no more than it needs */
  if (*buffer != nullptr && (*buffer)->size() == size)
    return true;
  buffer->reset();
  if (size == 0)
    return true;
  *buffer = make_buffer(QRhiBuffer::VertexBuffer, size, QRhiBuffer::Dynamic);
  return *buffer != nullptr;
}

void MapRenderer::held_stats() {
  size_t batch = 0;
  for (const std::unique_ptr<QRhiBuffer> *buffer :
       {&batch_vertices_, &batch_instances_, &closings_, &triangles_, &stipple_})
    batch += *buffer != nullptr ? (*buffer)->size() : 0;
  stats_.batch_held_bytes = batch * static_cast<size_t>(dynamic_copies_);
  stats_.contour_held_bytes =
      (contours_ != nullptr ? static_cast<size_t>(contours_->size()) : 0) * static_cast<size_t>(dynamic_copies_);
  stats_.dynamic_copies = dynamic_copies_;
}

bool MapRenderer::upload(QRhiResourceUpdateBatch *updates, TileUpload *upload) {
  TileGpu &gpu = tiles_[upload->tile][upload->level];
  const TerrainPlan::Level &level = plan_.tiles()[upload->tile].levels[upload->level];

  /* a tile level without triangles holds nothing */
  if (upload->indices && upload->index.empty()) {
    gpu = TileGpu();
    return true;
  }
  const quint32 nodes = static_cast<quint32>(level.ni) * static_cast<quint32>(level.nj);
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
  /* the previous frame's submission came, or will not count any more */
  submission_due_ = false;
  FrameInput input;
  const bool refused = !frame->begin(continued, pixels.width(), pixels.height(), &input);
  const Clock::time_point start = Clock::now();
  const Prepared prepared = prepare(updates, input, pixels);
  /* the times of this frame so far; the pass's come from the caller */
  stats_.fill_ms = frame->fill_ms();
  stats_.marks_ms = frame->marks_ms();
  stats_.refills = frame->refills();
  stats_.fallback_fills = frame->fallback_fills();
  stats_.pick_sizings = frame->pick_sizings();
  stats_.gather_ms = milliseconds(start, Clock::now()) - stats_.prepare_ms;
  set_pass_times(0.0, 0.0, 0.0, -1.0, 0.0, 0.0);
  if (prepared == Prepared::Failed) {
    /* this frame's uploads go with the batch; the tiles are planned and
       uploaded again from the core's arrays in a later frame */
    updates->release();
    drop_tiles();
    drop_batch();
    stats_.failed = true;
    stats_.tiles_drawn = 0;
    stats_.triangles = 0;
    stats_.resources = resources();
    stats_.fixed_resources = fixed_resources();
    stats_.batch_resources = batch_resources();
    held_stats();
    return prepared;
  }
  /* a refused fill: the batch this view has, and another frame for the fill */
  return refused ? Prepared::More : prepared;
}

Prepared MapRenderer::frame(const FrameInput &input, QRhiResourceUpdateBatch *updates, QSize pixels) {
  submission_due_ = false;
  const Prepared prepared = prepare(updates, input, pixels);
  if (prepared == Prepared::Failed) {
    updates->release();
    drop_batch();
    stats_.failed = true;
    stats_.resources = resources();
    stats_.fixed_resources = fixed_resources();
    stats_.batch_resources = batch_resources();
    held_stats();
  }
  return prepared;
}

void MapRenderer::drop_batch() {
  /* the batch's buffers whose uploads went with a released update batch:
     the next frame makes and uploads them again */
  corners_.reset();
  batch_vertices_.reset();
  batch_instances_.reset();
  closings_.reset();
  triangles_.reset();
  stipple_.reset();
  contours_.reset();
  contour_rez_ = 0;
  contour_generation_ = 0;
  contour_vertices_ = 0;
  meshes_.clear();
  item_meshes_.clear();
  batch_ready_ = false;
  batch_ = BatchPlan();
}

Prepared MapRenderer::prepare(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels) {
  stats_.frames++;
  stats_.failed = false;
  stats_.frame_slot = rhi_->currentFrameSlot();
  stats_.ring_bytes = 0;
  /* the pipelines, bindings and uniform ring, if a ring that could not be
     made again let go of them (write_slots): made again as create() made
     them, for the same render pass and sample count; the frame fails if they
     cannot be, and the next one tries again */
  if (terrain_[0] == nullptr && !make_fixed()) {
    release_fixed();
    return Prepared::Failed;
  }
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

  /* the terrain: what the marks made stale at the frame's stride, within the
     budgets (every tile at stride 1, all at once, for the triangle colours
     of a test); only after a fill that drew the terrain, which also
     projected the data */
  PlanOptions options;
  options.upload_budget = options_.triangle_ids ? SIZE_MAX : options_.upload_budget;
  options.force_stride1 = options_.force_stride1 || options_.triangle_ids;
  options.culling = options_.culling && !options_.triangle_ids;
  options.drag = input.drag;
  /* The drag budget leaves room for the costs of the previous frame outside
     its preparing and gathering. Its uploads, its recording and the wait for
     its GPU work grow with the bytes of its stride, so during a drag a
     stride is weighed with the latest such costs of a frame drawn at it, if
     larger: a finer stride is not taken on the costs of a coarser frame
     alone, only to be dropped again in the next frame for its own. */
  if (input.drag && !dragging_)
    std::fill(outside_level_ms_, outside_level_ms_ + kLevels, -1.0);
  dragging_ = input.drag;
  options.budget_ms = options_.prep_budget_ms >= 0.0 ? options_.prep_budget_ms : drag_budget_ms(outside_ms_);
  if (options_.prep_budget_ms < 0.0)
    for (int l = 0; l < kLevels; l++)
      if (outside_level_ms_[l] >= 0.0)
        options.level_budget_ms[l] = drag_budget_ms(std::max(outside_ms_, outside_level_ms_[l]));
  options.node_ns_colour = options_.node_ns_colour;
  options.node_ns_z = options_.node_ns_z;
  PlanResult result;
  const bool more = draw_terrain_ && plan_.plan(input.terrain, options, &result);
  const PlanStats &plan_stats = result.stats;
  stats_.prepare_ms = plan_stats.prepare_ms;
  if (generation_ != plan_.generation()) {
    tiles_.clear();
    tiles_.resize(plan_.tiles().size());
    generation_ = plan_.generation();
  }
  else {
    /* levels evicted for the memory budget, by this frame or another
       renderer's, go before the uploads */
    for (const LevelKey &key : result.drops)
      tiles_[key.tile][key.level] = TileGpu();
  }
  for (TileUpload &upload : result.uploads)
    if (!this->upload(updates, &upload))
      return Prepared::Failed;
  drawn_ = result.drawn;
  drawn_counts_.clear();
  for (const LevelKey &key : drawn_)
    drawn_counts_.push_back(plan_.tiles()[key.tile].levels[key.level].index_count);
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
  stats_.stride = plan_stats.stride;
  stats_.stride_zoom = plan_stats.stride_zoom;
  stats_.stride_target = plan_stats.stride_target;
  stats_.tiles_visible = plan_stats.tiles_visible;
  stats_.tiles_culled = plan_stats.tiles_culled;
  stats_.drag_budget = plan_stats.drag_coarser;
  stats_.memory_budget = plan_stats.memory_coarser;
  stats_.over_memory_budget = plan_stats.over_memory;
  stats_.budget_ms = plan_stats.budget_ms;
  stats_.outside_ms = outside_ms_;
  for (int l = 0; l < kLevels; l++)
    if (level_stride(l) == plan_stats.stride_target && outside_level_ms_[l] >= 0.0)
      stats_.outside_ms = std::max(outside_ms_, outside_level_ms_[l]);
  stats_.estimated_ms = plan_stats.estimated_ms;
  stats_.work_ms = plan_stats.work_ms;
  stats_.bounds_ms = plan_stats.bounds_ms;
  stats_.resident_bytes = plan_.held_bytes();
  stats_.cache_bytes = cache_bytes();
  stats_.buffer_copies = copies_;
  stats_.evictions = plan_.evictions();
  if (options_.triangle_ids && !upload_triangle_ids(updates, input))
    return Prepared::Failed;

  /* the batch's draws, their buffers and their uniform slots */
  const Prepared batch = prepare_batch(updates, input, pixels);
  if (batch == Prepared::Failed)
    return batch;
  if (!write_slots(updates, input, pixels))
    return Prepared::Failed;
  stats_.resources = resources();
  stats_.fixed_resources = fixed_resources();
  stats_.batch_resources = batch_resources();
  held_stats();
  return more ? Prepared::More : Prepared::Done;
}

Prepared MapRenderer::prepare_batch(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels) {
  /* the sizes the core asks for scaled by the view's pixel ratio (the
     offscreen renderer's from its options) */
  style_.ratio = view_ratio_ > 0.0 ? view_ratio_ : options_.pixel_ratio;
  stats_.pixel_ratio = style_.ratio;
  if (input.batch == nullptr) {
    batch_ = BatchPlan();
    return Prepared::Done;
  }
  const struct mbv_draw_batch &b = *input.batch;
  batch_.build(b, style_, input.camera, input.projection, pixels.width(), pixels.height());

  /* the corners of a wide line's quad: (end, side), two triangles */
  if (corners_ == nullptr) {
    static const float corners[12] = {0.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    if ((corners_ = make_buffer(QRhiBuffer::VertexBuffer, sizeof(corners))) == nullptr)
      return Prepared::Failed;
    updates->uploadStaticBuffer(corners_.get(), corners);
  }

  /* The batch's vertices and instances and what the plan made of them, in
     the frame that filled them (or after a new renderer, a new size or a new
     ratio, which change the stipple), whole: the batch is drawn in this
     frame. Each buffer has exactly the size of its data, so that each write
     covers the whole buffer (mbvq_rhi_renderer.h). The camera changes the
     stipple values too: a frame that draws the previous batch with a new
     camera, after a refused fill, uses the old values until the next fill. */
  if (input.filled || !batch_ready_ || pixels != batch_pixels_ || style_.ratio != batch_ratio_) {
    struct Upload {
      std::unique_ptr<QRhiBuffer> *buffer;
      const void *data;
      size_t bytes;
    };
    const Upload uploads[] = {
        {&batch_vertices_, b.vertices, static_cast<size_t>(b.num_vertices) * sizeof(struct mbv_draw_vertex)},
        {&batch_instances_, b.instances, static_cast<size_t>(b.num_instances) * sizeof(struct mbv_draw_instance)},
        {&closings_, batch_.closings().data(), batch_.closings().size() * sizeof(struct mbv_draw_vertex)},
        {&triangles_, batch_.triangles().data(), batch_.triangles().size() * sizeof(struct mbv_draw_vertex)},
        {&stipple_, batch_.stipple().data(), batch_.stipple().size() * sizeof(StippleVertex)}};
    size_t bytes = 0;
    for (const Upload &upload : uploads) {
      if (upload.bytes > 0xffffffffu || !fit(upload.buffer, static_cast<quint32>(upload.bytes)))
        return Prepared::Failed;
      if (upload.bytes == 0)
        continue;
      updates->updateDynamicBuffer(upload.buffer->get(), 0, static_cast<quint32>(upload.bytes), upload.data);
      bytes += upload.bytes;
    }
    stats_.batch_uploads++;
    stats_.batch_bytes += bytes;
    stats_.batch_largest = std::max(stats_.batch_largest, bytes);
    if (bytes > kBatchCap)
      stats_.batch_over_cap++;
    batch_ready_ = true;
    batch_pixels_ = pixels;
    batch_ratio_ = style_.ratio;
  }

  /* the finest contours made, uploaded when they change: their pointer is
     valid only in this frame */
  stats_.contour_rez = 0;
  stats_.contour_vertices = 0;
  if (input.contours != nullptr && input.contour_vertices >= 2) {
    if (input.contour_rez != contour_rez_ || input.contour_generation != contour_generation_ ||
        input.contour_vertices != contour_vertices_ || contours_ == nullptr) {
      const size_t bytes = input.contour_vertices * 3 * sizeof(float);
      if (bytes > 0xffffffffu || !fit(&contours_, static_cast<quint32>(bytes)))
        return Prepared::Failed;
      updates->updateDynamicBuffer(contours_.get(), 0, static_cast<quint32>(bytes), input.contours);
      contour_rez_ = input.contour_rez;
      contour_generation_ = input.contour_generation;
      contour_vertices_ = input.contour_vertices;
      stats_.contour_uploads++;
      stats_.contour_bytes += bytes;
    }
    stats_.contour_rez = contour_rez_;
    stats_.contour_vertices = contour_vertices_;
  }
  else {
    /* no contours in this frame: record() draws none, and the buffer goes
       (made and filled again when contours come back) */
    contours_.reset();
    contour_rez_ = 0;
    contour_vertices_ = 0;
  }

  /* the meshes of the marker runs; beyond kMeshes those not used in the
     last frame go, the least recently used first */
  mesh_clock_++;
  while (meshes_.size() > kMeshes) {
    auto oldest = std::min_element(
        meshes_.begin(), meshes_.end(),
        [](const std::unique_ptr<MeshGpu> &a, const std::unique_ptr<MeshGpu> &b) { return a->used < b->used; });
    if ((*oldest)->used + 1 >= mesh_clock_)
      break;
    meshes_.erase(oldest);
  }
  item_meshes_.assign(batch_.items().size(), nullptr);
  for (size_t d = 0; d < batch_.items().size(); d++) {
    const DrawItem &item = batch_.items()[d];
    if (item.kind == DrawKind::Markers && item.shape >= 0 && item.shape < b.num_shapes) {
      bool failed = false;
      item_meshes_[d] = mesh(updates, b.shapes[item.shape], &failed);
      if (failed)
        return Prepared::Failed;
    }
  }
  stats_.markers = batch_.markers();
  stats_.uniform_slots = static_cast<int>(batch_.slot_specs().size());
  return Prepared::Done;
}

MapRenderer::MeshGpu *MapRenderer::mesh(QRhiResourceUpdateBatch *updates, const struct mbv_draw_shape &shape,
                                         bool *failed) {
  /* by the whole record, never by the display list's id (a list is defined
     again with other radii when the view changes) */
  const GluVariant variant = options_.glu_variant;
  for (const std::unique_ptr<MeshGpu> &m : meshes_) {
    if (m->variant == variant && m->shape.kind == shape.kind && m->shape.outer == shape.outer &&
        m->shape.inner == shape.inner && m->shape.slices == shape.slices && m->shape.ring_slices == shape.ring_slices &&
        m->shape.stacks == shape.stacks) {
      m->used = mesh_clock_;
      return m->buffer != nullptr ? m.get() : nullptr;
    }
  }
  auto m = std::make_unique<MeshGpu>();
  m->shape = shape;
  m->variant = variant;
  m->used = mesh_clock_;
  const std::vector<float> vertices = marker_mesh(shape, variant);
  m->vertices = static_cast<quint32>(vertices.size() / 4);
  if (m->vertices > 0) {
    /* a buffer that could not be made fails the frame and is not kept: a
       later frame makes it again (a shape without triangles is kept) */
    m->buffer = make_buffer(QRhiBuffer::VertexBuffer, static_cast<quint32>(vertices.size() * sizeof(float)));
    if (m->buffer == nullptr) {
      *failed = true;
      return nullptr;
    }
    updates->uploadStaticBuffer(m->buffer.get(), vertices.data());
  }
  stats_.meshes++;
  meshes_.push_back(std::move(m));
  return meshes_.back()->buffer != nullptr ? meshes_.back().get() : nullptr;
}

bool MapRenderer::write_slots(QRhiResourceUpdateBatch *updates, const FrameInput &input, QSize pixels) {
  /* One slot per drawable command, at offsets aligned for dynamic uniform
     buffers, written once per frame: the camera of the core (or its
     projection alone) corrected for this API's clip space, multiplied in
     double and stored as floats, the command's colour, the drawing's size,
     its line width or point size, and its stipple. The whole ring is
     written, the slots beyond the commands as zeros (mbvq_rhi_renderer.h). */
  /* no ring: prepare() makes the fixed set again before this, and without
     a ring the frame fails rather than doubling a capacity of 0 forever */
  if (uniforms_ == nullptr || slot_capacity_ == 0)
    return false;
  const std::vector<SlotSpec> &specs = batch_.slot_specs();
  const quint32 count = std::max<quint32>(1, static_cast<quint32>(specs.size()));
  if (count > slot_capacity_) {
    /* the ring grows by doubling; if the larger buffer cannot be made, the
       ring goes back to its old size and the frame fails as for the other
       buffers (RenderOptions::fail_slot_growth: as when memory runs out).
       If not even that can be made (fail_slot_restore), a ring or bindings
       without their native buffers must not be used: the renderer lets go of
       them and of the pipelines made with those bindings, and prepare()
       makes them again in a later frame. */
    const quint32 old_capacity = slot_capacity_;
    quint32 capacity = slot_capacity_;
    while (capacity < count)
      capacity *= 2;
    /* a ring whose bytes do not fit in 32 bits cannot be made either (with
       slots of 256 bytes, more than 2^23 drawable commands): the frame
       fails, and the ring keeps its size */
    if (static_cast<quint64>(slot_stride_) * capacity > 0xffffffffu)
      return false;
    uniforms_->setSize(slot_stride_ * capacity);
    if (options_.fail_slot_growth || !uniforms_->create() || !bindings_->create()) {
      uniforms_->setSize(slot_stride_ * old_capacity);
      if (options_.fail_slot_restore || !uniforms_->create() || !bindings_->create())
        release_fixed();
      return false;
    }
    slot_capacity_ = capacity;
  }
  const QMatrix4x4 correction = rhi_->clipSpaceCorrMatrix();
  double clip[16];
  for (int i = 0; i < 16; i++)
    clip[i] = correction.constData()[i];
  double with_camera[16];
  double projection_only[16];
  multiply(clip, input.camera, with_camera);
  multiply(clip, input.projection, projection_only);
  const float zero_to_one = rhi_->isClipDepthZeroToOne() ? 1.0f : 0.0f;
  const size_t ring_bytes = static_cast<size_t>(slot_stride_) * slot_capacity_;
  slot_bytes_.assign(ring_bytes, 0);
  for (quint32 k = 0; k < count; k++) {
    float slot[kSlotBytes / sizeof(float)] = {};
    SlotSpec spec;
    if (k < specs.size())
      spec = specs[k];
    const double *mvp = spec.camera ? with_camera : projection_only;
    for (int i = 0; i < 16; i++)
      slot[i] = static_cast<float>(mvp[i]);
    slot[16] = spec.colour[0];
    slot[17] = spec.colour[1];
    slot[18] = spec.colour[2];
    slot[19] = 1.0f;
    slot[20] = static_cast<float>(pixels.width());
    slot[21] = static_cast<float>(pixels.height());
    slot[22] = static_cast<float>(spec.width);
    slot[23] = static_cast<float>(spec.width);
    slot[24] = static_cast<float>(spec.pattern);
    slot[25] = static_cast<float>(spec.factor);
    slot[26] = spec.stipple ? 1.0f : 0.0f;
    slot[27] = zero_to_one;
    std::memcpy(&slot_bytes_[static_cast<size_t>(slot_stride_) * k], slot, kSlotBytes);
  }
  updates->updateDynamicBuffer(uniforms_.get(), 0, static_cast<quint32>(ring_bytes), slot_bytes_.data());
  stats_.ring_bytes = ring_bytes;
  return true;
}

QRhiBuffer *MapRenderer::source(DrawSource source) const {
  switch (source) {
  case DrawSource::Batch:
    return batch_vertices_.get();
  case DrawSource::Closings:
    return closings_.get();
  case DrawSource::Triangles:
    return triangles_.get();
  case DrawSource::Stipple:
    return stipple_.get();
  case DrawSource::Instances:
    return batch_instances_.get();
  case DrawSource::None:
    break;
  }
  return nullptr;
}

void MapRenderer::bind(QRhiCommandBuffer *cb, QRhiGraphicsPipeline *pipeline, int slot, QSize pixels) {
  cb->setGraphicsPipeline(pipeline);
  /* after every pipeline: Vulkan takes the scissor from the viewport */
  cb->setViewport(QRhiViewport(0, 0, static_cast<float>(pixels.width()), static_cast<float>(pixels.height())));
  const QRhiCommandBuffer::DynamicOffset offset(0, static_cast<quint32>(slot) * slot_stride_);
  cb->setShaderResources(bindings_.get(), 1, &offset);
}

void MapRenderer::record(QRhiCommandBuffer *cb, QSize pixels) {
  stats_.tiles_drawn = 0;
  stats_.triangles = 0;
  stats_.draws = 0;
  stats_.thin_draws = 0;
  stats_.wide_draws = 0;
  stats_.marker_draws = 0;
  if (terrain_[0] == nullptr)
    return;
  const std::vector<SlotSpec> &specs = batch_.slot_specs();
  const std::vector<DrawItem> &items = batch_.items();
  for (size_t d = 0; d < items.size(); d++) {
    const DrawItem &item = items[d];
    const SlotSpec &spec = specs[static_cast<size_t>(item.slot)];
    const int depth = spec.depth_test ? 1 : 0;
    if (item.kind == DrawKind::Terrain) {
      if (draw_terrain_)
        record_terrain(cb, pixels, item.slot, depth);
      continue;
    }
    /* the comparisons of the triangle colours draw the terrain alone */
    if (options_.triangle_ids)
      continue;
    const QRhiCommandBuffer::VertexInput corners(corners_.get(), 0);
    switch (item.kind) {
    case DrawKind::Contours: {
      if (contours_ == nullptr || contour_vertices_ < 2)
        break;
      const quint32 pairs = static_cast<quint32>(contour_vertices_ / 2);
      if (Style::thin(spec.width)) {
        bind(cb, pipelines_[kContourThin][depth].get(), item.slot, pixels);
        const QRhiCommandBuffer::VertexInput input(contours_.get(), 0);
        cb->setVertexInput(0, 1, &input);
        cb->draw(2 * pairs);
        stats_.thin_draws++;
      }
      else {
        bind(cb, pipelines_[kContourWide][depth].get(), item.slot, pixels);
        const QRhiCommandBuffer::VertexInput inputs[] = {corners, {contours_.get(), 0}};
        cb->setVertexInput(0, 2, inputs);
        cb->draw(6, pairs);
        stats_.wide_draws++;
      }
      stats_.draws++;
      break;
    }
    case DrawKind::Lines:
    case DrawKind::LineStrip:
    case DrawKind::Points:
    case DrawKind::Triangles:
    case DrawKind::StippleLines: {
      QRhiBuffer *buffer = source(item.source);
      const Pipeline index = item.kind == DrawKind::Lines       ? kThinLines
                             : item.kind == DrawKind::LineStrip ? kThinStrip
                             : item.kind == DrawKind::Points    ? kPoints
                             : item.kind == DrawKind::Triangles ? kTriangles
                                                                : kStippleThin;
      QRhiGraphicsPipeline *pipeline = pipelines_[index][depth].get();
      if (buffer == nullptr || pipeline == nullptr || item.count == 0)
        break;
      bind(cb, pipeline, item.slot, pixels);
      const QRhiCommandBuffer::VertexInput input(buffer, 0);
      cb->setVertexInput(0, 1, &input);
      cb->draw(item.count, 1, item.first, 0);
      stats_.draws++;
      stats_.thin_draws += item.kind != DrawKind::Triangles && item.kind != DrawKind::Points;
      break;
    }
    case DrawKind::WideStrip:
    case DrawKind::WidePairs:
    case DrawKind::StippleWide: {
      QRhiBuffer *buffer = source(item.source);
      if (buffer == nullptr || item.count == 0)
        break;
      if (item.kind == DrawKind::WideStrip) {
        /* the segments' two ends: the strip's vertices from first, and from
           the next one */
        bind(cb, pipelines_[kWideStrip][depth].get(), item.slot, pixels);
        const QRhiCommandBuffer::VertexInput inputs[] = {
            corners, {buffer, item.offset}, {buffer, item.offset + static_cast<quint32>(sizeof(struct mbv_draw_vertex))}};
        cb->setVertexInput(0, 3, inputs);
      }
      else {
        bind(cb, pipelines_[item.kind == DrawKind::WidePairs ? kWidePairs : kStippleWide][depth].get(), item.slot,
             pixels);
        const QRhiCommandBuffer::VertexInput inputs[] = {corners, {buffer, item.offset}};
        cb->setVertexInput(0, 2, inputs);
      }
      cb->draw(6, item.count);
      stats_.draws++;
      stats_.wide_draws++;
      break;
    }
    case DrawKind::Markers: {
      /* each marker draws its fill, then its ring, before the next one, as
         glCallList does: instances are drawn in order */
      const MeshGpu *m = d < item_meshes_.size() ? item_meshes_[d] : nullptr;
      if (m == nullptr || batch_instances_ == nullptr || item.count == 0)
        break;
      bind(cb, pipelines_[kMarkers][depth].get(), item.slot, pixels);
      const QRhiCommandBuffer::VertexInput inputs[] = {{m->buffer.get(), 0}, {batch_instances_.get(), item.offset}};
      cb->setVertexInput(0, 2, inputs);
      cb->draw(m->vertices, item.count);
      stats_.draws++;
      stats_.marker_draws++;
      break;
    }
    case DrawKind::Terrain:
      break;
    }
  }
}

void MapRenderer::record_terrain(QRhiCommandBuffer *cb, QSize pixels, int slot, int depth) {
  bind(cb, terrain_[depth].get(), slot, pixels);
  stats_.draws++;
  if (options_.triangle_ids) {
    if (id_vertices_ > 0) {
      const QRhiCommandBuffer::VertexInput inputs[] = {{id_positions_.get(), 0}, {id_colours_.get(), 0}};
      cb->setVertexInput(0, 2, inputs);
      cb->draw(id_vertices_);
      stats_.triangles = id_vertices_ / 3;
    }
    return;
  }
  /* the tile levels of the frame's stride whose every stream is as new as
     the marks (the plan's drawn set) */
  for (size_t d = 0; d < drawn_.size(); d++) {
    const TileGpu &gpu = tiles_[drawn_[d].tile][drawn_[d].level];
    if (gpu.positions == nullptr || gpu.colours == nullptr || gpu.indices == nullptr)
      continue;
    const QRhiCommandBuffer::VertexInput inputs[] = {{gpu.positions.get(), 0}, {gpu.colours.get(), 0}};
    cb->setVertexInput(0, 2, inputs, gpu.indices.get(), 0, QRhiCommandBuffer::IndexUInt16);
    cb->drawIndexed(drawn_counts_[d]);
    stats_.tiles_drawn++;
    stats_.triangles += drawn_counts_[d] / 3;
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
  /* the frame's cost outside its preparing and gathering (the fill, the
     marks, the uploads, the uniforms and the pass), for the next frame's
     drag budget, also as the latest of its stride during a drag; a view adds
     the time to its window's submission */
  if (render_ms > 0.0) {
    outside_ms_ = std::max(0.0, render_ms - stats_.work_ms);
    outside_level_ = drawn_.empty() ? -1 : drawn_.front().level;
    if (dragging_ && outside_level_ >= 0)
      outside_level_ms_[outside_level_] = outside_ms_;
    submission_due_ = true;
  }
}

void MapRenderer::frame_submitted(double at_ms) {
  /* once per frame: a window can submit frames this view did not render */
  if (!submission_due_)
    return;
  outside_ms_ += std::max(0.0, at_ms - stats_.render_end_ms);
  if (dragging_ && outside_level_ >= 0)
    outside_level_ms_[outside_level_] = outside_ms_;
  submission_due_ = false;
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
  /* the target and the renderer for frames of a size */
  bool begin(QSize pixels, const RenderOptions &options);
  /* frames until everything is uploaded or max_frames, the last one read
     back into image (may be null); prepare runs the steps before the pass */
  bool frames(QSize pixels, int max_frames, QImage *image,
              const std::function<rhi::Prepared(QRhiResourceUpdateBatch *, int)> &prepare);
  void drop_renderer() {
    if (renderer != nullptr) {
      stats = renderer->stats();
      renderer.reset();
    }
    stats.resources = 0;
    stats.batch_held_bytes = 0;
    stats.contour_held_bytes = 0;
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

bool OffscreenRenderer::Impl::begin(QSize pixels, const RenderOptions &options) {
  Impl &d = *this;
  d.last_frames = 0;
  if (d.rhi == nullptr || pixels.width() < 1 || pixels.height() < 1)
    return false;
  if (pixels != d.size && !d.make_target(pixels))
    return false;
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
  return true;
}

bool OffscreenRenderer::Impl::frames(QSize pixels, int max_frames, QImage *image,
                                     const std::function<rhi::Prepared(QRhiResourceUpdateBatch *, int)> &prepare) {
  Impl &d = *this;
  const int width = pixels.width();
  const int height = pixels.height();
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
    const rhi::Prepared prepared = prepare(updates, f);
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

bool OffscreenRenderer::render_map(size_t instance, int width, int height, const RenderOptions &options, QImage *image,
                                   int max_frames) {
  Impl &d = *impl_;
  const QSize pixels(width, height);
  if (!d.begin(pixels, options))
    return false;
  if (d.frame == nullptr || d.frame->instance() != instance)
    d.frame = std::make_unique<rhi::MapFrame>(instance);
  /* the frames as while a map mouse button is down, or not */
  d.frame->set_drag(options.drag);
  d.frame->set_fill_fallback_ms(options.fill_fallback_ms);
  /* the steps of a view's frame; the frames after the first only continue
     uploads, and reuse the batch while nothing changed */
  return d.frames(pixels, max_frames, image, [&](QRhiResourceUpdateBatch *updates, int f) {
    return d.renderer->frame(d.frame.get(), updates, f > 0, pixels);
  });
}

bool OffscreenRenderer::render_batch(const struct mbv_draw_batch *batch, const double camera[16],
                                     const double projection[16], int width, int height, const RenderOptions &options,
                                     QImage *image) {
  Impl &d = *impl_;
  const QSize pixels(width, height);
  if (batch == nullptr || !d.begin(pixels, options))
    return false;
  /* one frame of the batch as the map's frame driver would give it, without
     terrain or contours */
  rhi::FrameInput input;
  input.batch = batch;
  input.filled = true;
  std::copy(camera, camera + 16, input.camera);
  std::copy(projection, projection + 16, input.projection);
  return d.frames(pixels, 1, image, [&](QRhiResourceUpdateBatch *updates, int) {
    return d.renderer->frame(input, updates, pixels);
  });
}

bool OffscreenRenderer::empty_frame() {
  Impl &d = *impl_;
  if (d.rhi == nullptr || d.target == nullptr)
    return false;
  QRhiCommandBuffer *cb = nullptr;
  if (d.rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess)
    return false;
  cb->beginPass(d.target.get(), Qt::white, {1.0f, 0});
  cb->endPass();
  d.rhi->endOffscreenFrame();
  return true;
}

}  // namespace mbvq
