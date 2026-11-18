/*--------------------------------------------------------------------
 *    The MB-system:  wcd_swath_rhi.cc  12/11/2026
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

#include "wcd_swath_rhi.h"

#include <cstring>

#include <QFile>
#include <QMatrix4x4>
#include <QOffscreenSurface>
#include <rhi/qrhi_platform.h>

#include "wcd_plot.h"

namespace wcd {

namespace {

/* uniform block of the vertex shader: transform (mat4) and amplitude range (vec4) */
constexpr quint32 UNIFORM_FLOATS = 16 + 4;

QShader load_shader(const char *name) {
  QFile file(QString::fromLatin1(name));
  return file.open(QIODevice::ReadOnly) ? QShader::fromSerialized(file.readAll()) : QShader();
}

/* a QRhi without a window; for OpenGL the surface must outlive it */
struct OffscreenRhi {
  std::unique_ptr<QOffscreenSurface> surface;
  std::unique_ptr<QRhi> rhi;
};

/* Metal on macOS, Direct3D 11 on Windows (OpenGL if that fails), OpenGL elsewhere */
void create_offscreen_rhi(OffscreenRhi &o) {
#if QT_CONFIG(metal)
  QRhiMetalInitParams metal;
  o.rhi.reset(QRhi::create(QRhi::Metal, &metal));
#elif defined(Q_OS_WIN)
  QRhiD3D11InitParams d3d11;
  o.rhi.reset(QRhi::create(QRhi::D3D11, &d3d11));
#endif
#if QT_CONFIG(opengl) && !QT_CONFIG(metal)
  if (o.rhi == nullptr) {
    o.surface.reset(QRhiGles2InitParams::newFallbackSurface());
    QRhiGles2InitParams gl;
    gl.fallbackSurface = o.surface.get();
    o.rhi.reset(QRhi::create(QRhi::OpenGLES2, &gl));
  }
#endif
}

}  // namespace

/*--------------------------------------------------------------------*/
SwathRhi::SwathRhi(QRhi *rhi) : rhi_(rhi) {}

SwathRhi::~SwathRhi() = default;

bool SwathRhi::init(QRhiRenderPassDescriptor *pass, int sample_count) {
  if (!rhi_->isFeatureSupported(QRhi::ElementIndexUint)) {
    qWarning("mbwcdview: the graphics driver has no 32 bit indices, so the water column is not drawn");
    return false;
  }
  uniforms_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, UNIFORM_FLOATS * sizeof(float)));
  lut_.reset(rhi_->newTexture(QRhiTexture::RGBA8, QSize(256, 1)));
  sampler_.reset(rhi_->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                  QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
  if (!uniforms_->create() || !lut_->create() || !sampler_->create())
    return false;
  bindings_.reset(rhi_->newShaderResourceBindings());
  bindings_->setBindings(
      {QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, uniforms_.get()),
       QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, lut_.get(),
                                                 sampler_.get())});
  if (!bindings_->create())
    return false;

  const QShader vertex = load_shader(":/mbwcdview/wcd_swath.vert.qsb");
  const QShader fragment = load_shader(":/mbwcdview/wcd_swath.frag.qsb");
  if (!vertex.isValid() || !fragment.isValid())
    return false;
  /* one vertex per sample: across, depth, amplitude */
  QRhiVertexInputLayout layout;
  layout.setBindings({{3 * sizeof(float)}});
  layout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0},
                        {0, 1, QRhiVertexInputAttribute::Float, 2 * sizeof(float)}});
  pipeline_.reset(rhi_->newGraphicsPipeline());
  pipeline_->setShaderStages({{QRhiShaderStage::Vertex, vertex}, {QRhiShaderStage::Fragment, fragment}});
  pipeline_->setVertexInputLayout(layout);
  pipeline_->setShaderResourceBindings(bindings_.get());
  pipeline_->setRenderPassDescriptor(pass);
  pipeline_->setSampleCount(sample_count);
  pipeline_->setTopology(QRhiGraphicsPipeline::Triangles);
  return pipeline_->create();
}

void SwathRhi::upload(QRhiResourceUpdateBatch *updates, const std::vector<float> &vertices,
                      const std::vector<unsigned int> &indices) {
  /* buffers the GPU still reads are released by QRhi when it is done with them */
  vertices_.reset();
  indices_.reset();
  nindices_ = 0;
  if (indices.empty())
    return;
  vertices_.reset(rhi_->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer,
                                  (quint32)(vertices.size() * sizeof(float))));
  indices_.reset(rhi_->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer,
                                 (quint32)(indices.size() * sizeof(unsigned int))));
  if (!vertices_->create() || !indices_->create()) {
    vertices_.reset();
    indices_.reset();
    return;
  }
  updates->uploadStaticBuffer(vertices_.get(), vertices.data());
  updates->uploadStaticBuffer(indices_.get(), indices.data());
  nindices_ = (quint32)indices.size();
}

void SwathRhi::update(QRhiResourceUpdateBatch *updates, const WcdPlot &plot) {
  if (!lut_uploaded_) {
    updates->uploadTexture(lut_.get(), colormap_image());
    lut_uploaded_ = true;
  }
  float sx, ox, sz, oz;
  plot.deviceTransform(sx, ox, sz, oz);
  /* the transform of SwathGL, then to the clip space of the backend */
  const QMatrix4x4 transform = rhi_->clipSpaceCorrMatrix() * QMatrix4x4(sx, 0.0f, 0.0f, ox,
                                                                       0.0f, sz, 0.0f, oz,
                                                                       0.0f, 0.0f, 1.0f, 0.0f,
                                                                       0.0f, 0.0f, 0.0f, 1.0f);
  float block[UNIFORM_FLOATS];
  std::memcpy(block, transform.constData(), 16 * sizeof(float));
  block[16] = (float)plot.colorMin();
  block[17] = (float)plot.colorMax();
  block[18] = 0.0f;
  block[19] = 0.0f;
  updates->updateDynamicBuffer(uniforms_.get(), 0, sizeof(block), block);
}

void SwathRhi::draw(QRhiCommandBuffer *cb, const QRect &viewport) {
  if (nindices_ == 0)
    return;
  cb->setGraphicsPipeline(pipeline_.get());
  cb->setViewport(QRhiViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height()));
  cb->setShaderResources();
  const QRhiCommandBuffer::VertexInput input(vertices_.get(), 0);
  cb->setVertexInput(0, 1, &input, indices_.get(), 0, QRhiCommandBuffer::IndexUInt32);
  cb->drawIndexed(nindices_);
}

/*--------------------------------------------------------------------*/
bool render_swath_rhi(const WcdPlot &plot, const QSize &size, double scale, QImage &image) {
  OffscreenRhi o;
  create_offscreen_rhi(o);
  if (o.rhi == nullptr)
    return false;
  QRhi *rhi = o.rhi.get();
  const QSize pixels = image.size();
  std::unique_ptr<QRhiTexture> texture(
      rhi->newTexture(QRhiTexture::RGBA8, pixels, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
  if (!texture->create())
    return false;
  std::unique_ptr<QRhiTextureRenderTarget> target(rhi->newTextureRenderTarget({texture.get()}));
  std::unique_ptr<QRhiRenderPassDescriptor> pass(target->newCompatibleRenderPassDescriptor());
  target->setRenderPassDescriptor(pass.get());
  if (!target->create())
    return false;

  WcdPlot copy(plot);
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  copy.buildMesh(vertices, indices);
  QRhiReadbackResult result;
  {
    SwathRhi swath(rhi);
    QRhiCommandBuffer *cb = nullptr;
    if (!swath.init(pass.get(), 1) || rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess)
      return false;
    QRhiResourceUpdateBatch *updates = rhi->nextResourceUpdateBatch();
    swath.upload(updates, vertices, indices);
    swath.update(updates, copy);
    cb->beginPass(target.get(), Qt::white, {1.0f, 0}, updates);
    swath.draw(cb, copy.deviceViewport(size, scale));
    QRhiResourceUpdateBatch *readback = rhi->nextResourceUpdateBatch();
    readback->readBackTexture({texture.get()}, &result);
    cb->endPass(readback);
    rhi->endOffscreenFrame();  // waits for the GPU, so the result is complete
  }
  const int w = result.pixelSize.width(), h = result.pixelSize.height();
  if (w != pixels.width() || h != pixels.height() || result.data.size() < (qsizetype)w * h * 4)
    return false;

  /* image rows top down: OpenGL reads them bottom up */
  const bool flip = rhi->isYUpInFramebuffer();
  QImage frame(w, h, result.format == QRhiTexture::BGRA8 ? QImage::Format_ARGB32 : QImage::Format_RGBA8888);
  for (int y = 0; y < h; y++)
    std::memcpy(frame.scanLine(y), result.data.constData() + (qsizetype)(flip ? h - 1 - y : y) * w * 4,
                (size_t)w * 4);
  image = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
  return true;
}

}  // namespace wcd
