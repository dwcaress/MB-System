/*--------------------------------------------------------------------
 *    The MB-system:  wcd_plot.cc  1/11/2026
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

#include "wcd_plot.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <QOffscreenSurface>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QPainterPath>

#include "viridis.h"

namespace wcd {

namespace {

/* margins around the image area (logical pixels) */
constexpr double MARGIN_LEFT = 72.0, MARGIN_RIGHT = 104.0, MARGIN_TOP = 16.0, MARGIN_BOTTOM = 46.0;
constexpr double CAPTION_HEIGHT = 20.0;

const char *vertex_shader =
    "#version 120\n"
    "attribute vec2 position;\n"
    "attribute float value;\n"
    "uniform vec4 transform;\n"  // sx, ox, sz, oz
    "uniform vec2 range;\n"      // vmin, vmax
    "varying float level;\n"
    "void main() {\n"
    "  gl_Position = vec4(position.x * transform.x + transform.y, position.y * transform.z + transform.w, 0.0, 1.0);\n"
    "  level = (value - range.x) / max(range.y - range.x, 1.0e-6);\n"
    "}\n";

const char *fragment_shader =
    "#version 120\n"
    "uniform sampler2D lut;\n"
    "varying float level;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(lut, vec2(clamp(level, 0.0, 1.0) * 255.0 / 256.0 + 0.5 / 256.0, 0.5));\n"
    "}\n";

QImage colormap_image() {
  QImage lut(256, 1, QImage::Format_RGBA8888);
  for (int i = 0; i < 256; i++)
    lut.setPixelColor(i, 0, QColor(viridis_rgb[i][0], viridis_rgb[i][1], viridis_rgb[i][2]));
  return lut;
}

/* tick spacing 1, 2 or 5 x 10^n giving about n ticks over span */
double tick_step(double span, int n) {
  if (!(span > 0.0))
    return 1.0;
  const double raw = span / n;
  const double p = std::pow(10.0, std::floor(std::log10(raw)));
  const double m = raw / p;
  return (m < 1.5 ? 1.0 : (m < 3.5 ? 2.0 : (m < 7.5 ? 5.0 : 10.0))) * p;
}

QString tick_label(double v, double step) {
  const int decimals = step >= 1.0 ? 0 : std::min(3, (int)std::ceil(-std::log10(step)));
  return QString::number(std::fabs(v) < 0.5 * step * 1.0e-6 ? 0.0 : v, 'f', decimals);
}

}  // namespace

/*--------------------------------------------------------------------*/
SwathGL::SwathGL() = default;
SwathGL::~SwathGL() = default;

void SwathGL::init() {
  initializeOpenGLFunctions();
  program_ = std::make_unique<QOpenGLShaderProgram>();
  program_->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader);
  program_->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader);
  program_->bindAttributeLocation("position", 0);
  program_->bindAttributeLocation("value", 1);
  program_->link();
  vbo_ = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
  vbo_->create();
  ibo_ = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
  ibo_->create();
  lut_ = std::make_unique<QOpenGLTexture>(colormap_image());
  lut_->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
  lut_->setWrapMode(QOpenGLTexture::ClampToEdge);
}

void SwathGL::upload(const std::vector<float> &vertices, const std::vector<unsigned int> &indices) {
  vbo_->bind();
  vbo_->allocate(vertices.data(), (int)(vertices.size() * sizeof(float)));
  vbo_->release();
  ibo_->bind();
  ibo_->allocate(indices.data(), (int)(indices.size() * sizeof(unsigned int)));
  ibo_->release();
  nindices_ = (int)indices.size();
}

void SwathGL::draw(float sx, float ox, float sz, float oz, float vmin, float vmax) {
  if (nindices_ == 0 || !program_->isLinked())
    return;
  program_->bind();
  program_->setUniformValue("transform", sx, ox, sz, oz);
  program_->setUniformValue("range", vmin, vmax);
  program_->setUniformValue("lut", 0);
  lut_->bind(0);
  vbo_->bind();
  ibo_->bind();
  program_->enableAttributeArray(0);
  program_->enableAttributeArray(1);
  program_->setAttributeBuffer(0, GL_FLOAT, 0, 2, 3 * sizeof(float));
  program_->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 1, 3 * sizeof(float));
  glDrawElements(GL_TRIANGLES, nindices_, GL_UNSIGNED_INT, nullptr);
  program_->disableAttributeArray(0);
  program_->disableAttributeArray(1);
  ibo_->release();
  vbo_->release();
  lut_->release();
  program_->release();
}

/*--------------------------------------------------------------------*/
void WcdPlot::setData(PlotData data) {
  data_ = std::move(data);
  mesh_dirty_ = true;
  updateView();
}

void WcdPlot::setColorRange(double vmin, double vmax) {
  vmin_ = vmin;
  vmax_ = vmax;
}

void WcdPlot::setDepthWindow(double dmin, double dmax) {
  if (dmin != dmin_ || dmax != dmax_)
    mesh_dirty_ = true;
  dmin_ = dmin;
  dmax_ = dmax;
  updateView();
}

void WcdPlot::setAcrossMax(double xmax) {
  xmax_ = xmax;
  updateView();
}

/* visible range: across from the data (or +-xmax), depth from the window down to the data */
void WcdPlot::updateView() {
  double x0 = std::numeric_limits<double>::max(), x1 = -x0, z1 = -x0;
  const size_t n = data_.across.size();
  for (size_t i = 0; i < n; i++) {
    const float x = data_.across[i], z = data_.depth[i];
    if (std::isfinite(x) && std::isfinite(z) && std::isfinite(data_.amplitude[i]) && z >= dmin_ && z <= dmax_) {
      x0 = std::min(x0, (double)x);
      x1 = std::max(x1, (double)x);
      z1 = std::max(z1, (double)z);
    }
  }
  if (x0 > x1) {
    x0 = -1.0;
    x1 = 1.0;
    z1 = dmax_;
  }
  if (xmax_ > 0.0) {
    x0 = -xmax_;
    x1 = xmax_;
  }
  view_x0_ = x0;
  view_x1_ = x1 > x0 ? x1 : x0 + 1.0;
  view_z0_ = dmin_;
  view_z1_ = std::min(dmax_, z1);
  if (view_z1_ - view_z0_ < 1.0e-3)
    view_z1_ = view_z0_ + 1.0e-3;
}

QRectF WcdPlot::plotRect(const QSize &size) const {
  return QRectF(MARGIN_LEFT, MARGIN_TOP, std::max(10.0, size.width() - MARGIN_LEFT - MARGIN_RIGHT),
                std::max(10.0, size.height() - MARGIN_TOP - MARGIN_BOTTOM - CAPTION_HEIGHT));
}

bool WcdPlot::toData(const QPointF &pixel, const QSize &size, double &across, double &depth) const {
  const QRectF r = plotRect(size);
  if (!r.contains(pixel))
    return false;
  across = view_x0_ + (pixel.x() - r.left()) / r.width() * (view_x1_ - view_x0_);
  depth = view_z0_ + (pixel.y() - r.top()) / r.height() * (view_z1_ - view_z0_);
  return true;
}

/*--------------------------------------------------------------------*/
void WcdPlot::buildMesh(std::vector<float> &vertices, std::vector<unsigned int> &indices) {
  const int nb = data_.nbeams, ns = data_.nsamples;
  vertices.resize((size_t)nb * ns * 3);
  std::vector<char> valid((size_t)nb * ns);
  for (size_t i = 0; i < (size_t)nb * ns; i++) {
    const float x = data_.across[i], z = data_.depth[i], a = data_.amplitude[i];
    valid[i] = std::isfinite(x) && std::isfinite(z) && std::isfinite(a) && z >= dmin_ && z <= dmax_;
    vertices[3 * i] = valid[i] ? x : 0.0f;
    vertices[3 * i + 1] = valid[i] ? z : 0.0f;
    vertices[3 * i + 2] = valid[i] ? a : 0.0f;
  }
  indices.clear();
  indices.reserve((size_t)nb * ns * 6);
  for (int b = 0; b + 1 < nb; b++) {
    for (int k = 0; k + 1 < ns; k++) {
      const unsigned int i00 = b * ns + k, i01 = i00 + 1, i10 = i00 + ns, i11 = i10 + 1;
      if (valid[i00] && valid[i10] && valid[i01]) {
        indices.push_back(i00);
        indices.push_back(i10);
        indices.push_back(i01);
      }
      if (valid[i10] && valid[i11] && valid[i01]) {
        indices.push_back(i10);
        indices.push_back(i11);
        indices.push_back(i01);
      }
    }
  }
  mesh_dirty_ = false;
}

void WcdPlot::drawGL(SwathGL &gl, const QSize &size, double dpr) {
  const QRectF r = plotRect(size);
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
  f->glViewport((int)std::lround(r.left() * dpr), (int)std::lround((size.height() - r.bottom()) * dpr),
                (int)std::lround(r.width() * dpr), (int)std::lround(r.height() * dpr));
  /* data to normalized device coordinates: depth increases downward */
  const float sx = (float)(2.0 / (view_x1_ - view_x0_));
  const float ox = (float)(-1.0 - view_x0_ * sx);
  const float sz = (float)(-2.0 / (view_z1_ - view_z0_));
  const float oz = (float)(1.0 - view_z0_ * sz);
  gl.draw(sx, ox, sz, oz, (float)vmin_, (float)vmax_);
}

/*--------------------------------------------------------------------*/
void WcdPlot::drawOverlay(QPainter &p, const QSize &size, const QString &caption) const {
  const QRectF r = plotRect(size);
  p.setRenderHint(QPainter::Antialiasing, true);
  auto px = [&](double x) { return r.left() + (x - view_x0_) / (view_x1_ - view_x0_) * r.width(); };
  auto pz = [&](double z) { return r.top() + (z - view_z0_) / (view_z1_ - view_z0_) * r.height(); };

  /* water column bottom detections and soundings, clipped to the image area */
  p.save();
  p.setClipRect(r);
  if (data_.show_wc_bottom) {
    QPainterPath path;
    bool open = false;
    for (size_t i = 0; i < data_.detect_across.size(); i++) {
      const double x = data_.detect_across[i], z = data_.detect_depth[i];
      if (!std::isfinite(x) || !std::isfinite(z)) {
        open = false;
        continue;
      }
      if (open)
        path.lineTo(px(x), pz(z));
      else
        path.moveTo(px(x), pz(z));
      open = true;
    }
    p.setPen(QPen(QColor(255, 165, 0), 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
  }
  for (const Sounding &s : data_.soundings) {
    p.setPen(QPen(s.color, 1.0));
    p.setBrush(s.filled ? QBrush(s.color) : Qt::NoBrush);
    p.drawEllipse(QPointF(px(s.across), pz(s.depth)), 2.5, 2.5);
  }
  p.restore();

  /* frame, ticks and labels */
  p.setRenderHint(QPainter::Antialiasing, false);
  p.setPen(QPen(Qt::black, 1.0));
  p.setBrush(Qt::NoBrush);
  p.drawRect(r);
  const QFontMetricsF fm(p.font());
  const double xs = tick_step(view_x1_ - view_x0_, 8);
  for (double x = std::ceil(view_x0_ / xs) * xs; x <= view_x1_ + 1.0e-9; x += xs) {
    p.drawLine(QPointF(px(x), r.bottom()), QPointF(px(x), r.bottom() + 4));
    const QString t = tick_label(x, xs);
    p.drawText(QPointF(px(x) - fm.horizontalAdvance(t) / 2, r.bottom() + 6 + fm.ascent()), t);
  }
  const double zs = tick_step(view_z1_ - view_z0_, 8);
  for (double z = std::ceil(view_z0_ / zs) * zs; z <= view_z1_ + 1.0e-9; z += zs) {
    p.drawLine(QPointF(r.left() - 4, pz(z)), QPointF(r.left(), pz(z)));
    const QString t = tick_label(z, zs);
    p.drawText(QPointF(r.left() - 6 - fm.horizontalAdvance(t), pz(z) + fm.ascent() / 2 - 1), t);
  }
  const QString xlabel = "Across-track (m, starboard +)";
  p.drawText(QPointF(r.center().x() - fm.horizontalAdvance(xlabel) / 2, r.bottom() + 8 + 2 * fm.height()), xlabel);
  p.save();
  p.translate(14, r.center().y() + fm.horizontalAdvance(data_.ylabel) / 2);
  p.rotate(-90);
  p.drawText(QPointF(0, 0), data_.ylabel);
  p.restore();

  /* colour bar */
  const QRectF cb(r.right() + 16, r.top() + 0.1 * r.height(), 16, 0.8 * r.height());
  QImage bar(1, 256, QImage::Format_RGB32);
  for (int i = 0; i < 256; i++)
    bar.setPixelColor(0, 255 - i, QColor(viridis_rgb[i][0], viridis_rgb[i][1], viridis_rgb[i][2]));
  p.drawImage(cb, bar);
  p.drawRect(cb);
  const double vs = tick_step(vmax_ - vmin_, 6);
  for (double v = std::ceil(vmin_ / vs) * vs; v <= vmax_ + 1.0e-9; v += vs) {
    const double y = cb.bottom() - (v - vmin_) / std::max(vmax_ - vmin_, 1.0e-6) * cb.height();
    p.drawLine(QPointF(cb.right(), y), QPointF(cb.right() + 4, y));
    p.drawText(QPointF(cb.right() + 6, y + fm.ascent() / 2 - 1), tick_label(v, vs));
  }
  p.save();
  p.translate(cb.right() + 50, cb.center().y() + fm.horizontalAdvance("Amplitude (dB)") / 2);
  p.rotate(-90);
  p.drawText(QPointF(0, 0), "Amplitude (dB)");
  p.restore();

  /* caption (file name and metadata) for saved images */
  if (!caption.isEmpty()) {
    const QRectF box(4, size.height() - CAPTION_HEIGHT + 2, fm.horizontalAdvance(caption) + 8, CAPTION_HEIGHT - 4);
    p.fillRect(box, QColor(0, 0, 0, 128));
    p.setPen(Qt::white);
    p.drawText(QPointF(box.left() + 4, box.top() + fm.ascent() + 1), caption);
  }
}

/*--------------------------------------------------------------------*/
QImage WcdPlot::render(const QSize &size, double scale, const QString &caption) const {
  const QSize pixels(std::lround(size.width() * scale), std::lround(size.height() * scale));
  QImage image(pixels, QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::white);

  QOffscreenSurface surface;
  surface.create();
  QOpenGLContext context;
  if (context.create() && context.makeCurrent(&surface)) {
    {
      QOpenGLFramebufferObjectFormat format;
      format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
      QOpenGLFramebufferObject fbo(pixels, format);
      fbo.bind();
      QOpenGLFunctions *f = context.functions();
      f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      f->glClear(GL_COLOR_BUFFER_BIT);
      WcdPlot copy(*this);
      std::vector<float> vertices;
      std::vector<unsigned int> indices;
      copy.buildMesh(vertices, indices);
      SwathGL gl;
      gl.init();
      gl.upload(vertices, indices);
      copy.drawGL(gl, size, scale);
      f->glFinish();
      fbo.release();
      image = fbo.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }
    context.doneCurrent();
  }
  image.setDevicePixelRatio(scale);
  QPainter painter(&image);
  drawOverlay(painter, size, caption);
  painter.end();
  return image;
}

}  // namespace wcd
