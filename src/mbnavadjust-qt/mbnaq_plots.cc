/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_plots.cc  10/11/2026
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

#include "mbnaq_plots.h"

#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QLine>
#include <QPen>

#include <cmath>
#include <cstdio>
#include <vector>

extern "C" {
#include <X11/Intrinsic.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mb_xgraphics.h"
#include "mbnavadjust_io.h"
#include "mbnavadjust.h"
}

namespace mbnaq {

const std::array<QRgb, 256> &plotPalette() {
  static const std::array<QRgb, 256> palette = [] {
    std::array<QRgb, 256> colors;
    colors.fill(qRgb(0, 0, 0));
    colors[MB_COLOR_WHITE] = qRgb(255, 255, 255);
    colors[MB_COLOR_BLACK] = qRgb(0, 0, 0);
    colors[MB_COLOR_RED] = qRgb(255, 0, 0);
    colors[MB_COLOR_ORANGE] = qRgb(255, 165, 0);
    colors[MB_COLOR_YELLOW] = qRgb(255, 255, 0);
    colors[MB_COLOR_GREEN] = qRgb(0, 255, 0);
    colors[MB_COLOR_BLUEGREEN] = qRgb(0, 192, 160);
    colors[MB_COLOR_BLUE] = qRgb(0, 0, 255);
    colors[MB_COLOR_PURPLE] = qRgb(160, 32, 240);
    colors[MB_COLOR_CORAL] = qRgb(255, 127, 80);
    colors[MB_COLOR_LIGHTGREY] = qRgb(211, 211, 211);
    int j = MB_NDrawingColors;
    auto ramp = [&](int r0, int dr, int g0, int dg, int b0, int db) {
      for (int i = 0; i < 16; i++)
        colors[j + i] = qRgb((r0 + i * dr) >> 8, (g0 + i * dg) >> 8, (b0 + i * db) >> 8);
      j += 16;
    };
    ramp(65535, 0, 0, 2048, 0, 0);
    ramp(65535, -4096, 32767, 2048, 0, 0);
    ramp(0, 0, 65535, 0, 0, 4096);
    ramp(0, 0, 65535, -4096, 65535, 0);
    ramp(0, 4096, 0, 0, 65535, 0);
    colors[j] = qRgb(255, 0, 255);
    return colors;
  }();
  return palette;
}

static QColor colorOf(unsigned int pixel) { return QColor(plotPalette()[pixel & 0xff]); }

static QPen penOf(unsigned int pixel, int style) {
  QPen pen(colorOf(pixel));
  pen.setWidth(0);
  if (style == XG_DASHLINE)
    pen.setDashPattern({4.0, 4.0});
  return pen;
}

static QFont plotFont() {
  QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  font.setPixelSize(12);
  font.setBold(true);
  return font;
}

/*--------------------------------------------------------------------*/

ImageSurface::ImageSurface(int width, int height, bool whiteBackground) { resize(width, height, whiteBackground); }

ImageSurface::~ImageSurface() {
  if (painter_.isActive())
    painter_.end();
}

void ImageSurface::resize(int width, int height, bool whiteBackground) {
  if (painter_.isActive())
    painter_.end();
  image_ = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
  image_.fill(whiteBackground ? Qt::white : Qt::black);
  clipped_ = false;
}

void ImageSurface::begin() {
  if (!painter_.isActive()) {
    painter_.begin(&image_);
    painter_.setFont(plotFont());
    if (clipped_)
      painter_.setClipRect(clip_);
  }
}

const QImage &ImageSurface::image() {
  if (painter_.isActive())
    painter_.end();
  return image_;
}

void ImageSurface::line(int x1, int y1, int x2, int y2, unsigned int pixel, int style) {
  begin();
  painter_.setPen(penOf(pixel, style));
  painter_.drawLine(x1, y1, x2, y2);
}

void ImageSurface::rectangle(int x, int y, int width, int height, unsigned int pixel, int style) {
  begin();
  painter_.setPen(penOf(pixel, style));
  painter_.setBrush(Qt::NoBrush);
  painter_.drawRect(x, y, width, height);
}

void ImageSurface::fillRectangle(int x, int y, int width, int height, unsigned int pixel, int) {
  begin();
  painter_.fillRect(x, y, width, height, colorOf(pixel));
}

void ImageSurface::text(int x, int y, const char *string, unsigned int pixel, int) {
  begin();
  painter_.setPen(penOf(pixel, XG_SOLIDLINE));
  painter_.drawText(x, y, QString::fromLatin1(string));
}

void ImageSurface::measure(const char *string, int *width, int *ascent, int *descent) {
  const QFontMetrics metrics(plotFont());
  *width = metrics.horizontalAdvance(QString::fromLatin1(string));
  /* the plot layout expects the metrics of the 13 pixel X font of the Motif program */
  *ascent = 11;
  *descent = 2;
}

void ImageSurface::clip(int x, int y, int width, int height) {
  begin();
  clipped_ = true;
  clip_ = QRect(x, y, width, height);
  painter_.setClipRect(clip_);
}

/*--------------------------------------------------------------------*/

ContourLayers::ContourLayers(int width, int height) : width_(width), height_(height) {}

void ContourLayers::fillRectangle(int x, int y, int width, int height, unsigned int, int) {
  if (x == 0 && y == 0 && width == cont_borders[1] && height == cont_borders[3])
    stale_ = true;
}

void ContourLayers::measure(const char *string, int *width, int *ascent, int *descent) {
  *width = 8 * static_cast<int>(strlen(string));
  *ascent = 11;
  *descent = 2;
}

void ContourLayers::resize(int width, int height) {
  width_ = width;
  height_ = height;
  stale_ = true;
}

namespace {

/* Pixel positions as the shared plot code computes them. */
int plotX(double x) { return static_cast<int>(mbna_plotx_scale * (x - mbna_plot_lon_min)); }
int plotY(double y) { return static_cast<int>(cont_borders[3] - mbna_ploty_scale * (y - mbna_plot_lat_min)); }

/* Draws one contour set, keeping the order in which the lines overlap. */
void drawContours(QPainter &painter, const struct mbna_contour_vector &contours, double offsetX, double offsetY) {
  std::vector<QLine> batch;
  unsigned int pixel = mbna_color_foreground;
  int xo = 0, yo = 0;
  const auto flush = [&] {
    if (!batch.empty()) {
      painter.setPen(penOf(pixel, XG_SOLIDLINE));
      painter.drawLines(batch.data(), static_cast<int>(batch.size()));
      batch.clear();
    }
  };
  for (int i = 0; i < contours.nvector; i++) {
    const struct mbna_plot_vector &v = contours.vector[i];
    if (v.command == MBNA_PEN_COLOR) {
      if (static_cast<unsigned int>(v.color) != pixel) {
        flush();
        pixel = v.color;
      }
    } else if (v.command == MBNA_PEN_UP) {
      xo = plotX(v.x + offsetX);
      yo = plotY(v.y + offsetY);
    } else if (v.command == MBNA_PEN_DOWN) {
      const int x = plotX(v.x + offsetX);
      const int y = plotY(v.y + offsetY);
      batch.emplace_back(xo, yo, x, y);
      xo = x;
      yo = y;
    }
  }
  flush();
}

void drawTrack(QPainter &painter, const struct swathraw *swath, double offsetX, double offsetY) {
  if (swath == nullptr || swath->npings < 2)
    return;
  std::vector<QLine> lines;
  lines.reserve(swath->npings);
  int xo = plotX(swath->pingraws[0].navlon + offsetX);
  int yo = plotY(swath->pingraws[0].navlat + offsetY);
  for (int i = 1; i < swath->npings; i++) {
    const int x = plotX(swath->pingraws[i].navlon + offsetX);
    const int y = plotY(swath->pingraws[i].navlat + offsetY);
    lines.emplace_back(xo, yo, x, y);
    xo = x;
    yo = y;
  }
  painter.setPen(penOf(mbna_color_foreground, XG_SOLIDLINE));
  painter.drawLines(lines.data(), static_cast<int>(lines.size()));
}

void drawTieBox(QPainter &painter, int x, int y, int boxoff, int wid, unsigned int fill) {
  painter.fillRect(x - boxoff, y - boxoff, wid, wid, colorOf(fill));
  painter.setPen(penOf(mbna_color_foreground, XG_SOLIDLINE));
  painter.setBrush(Qt::NoBrush);
  painter.drawRect(x - boxoff, y - boxoff, wid, wid);
}

void drawTie(QPainter &painter, const struct mbna_section *section1, int snav1, const struct mbna_section *section2, int snav2,
             double offsetX, double offsetY, int boxoff, int wid, unsigned int fill) {
  const int x1 = plotX(section1->snav_lon[snav1]);
  const int y1 = plotY(section1->snav_lat[snav1]);
  drawTieBox(painter, x1, y1, boxoff, wid, fill);
  const int x2 = plotX(section2->snav_lon[snav2] + offsetX);
  const int y2 = plotY(section2->snav_lat[snav2] + offsetY);
  drawTieBox(painter, x2, y2, boxoff, wid, fill);
  painter.setPen(penOf(mbna_color_foreground, XG_SOLIDLINE));
  painter.drawLine(x1, y1, x2, y2);
}

}  // namespace

void ContourLayers::rebuild() {
  stale_ = false;
  margin_ = MAX(width_, height_) / 2;
  builtOffsetX_ = mbna_offset_x;
  builtOffsetY_ = mbna_offset_y;

  fixed_ = QImage(width_, height_, QImage::Format_ARGB32_Premultiplied);
  fixed_.fill(colorOf(mbna_color_background));
  moving_ = QImage(width_ + 2 * margin_, height_ + 2 * margin_, QImage::Format_ARGB32_Premultiplied);
  moving_.fill(Qt::transparent);

  QPainter fixedPainter(&fixed_);
  QPainter movingPainter(&moving_);
  movingPainter.translate(margin_, margin_);
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    drawContours(fixedPainter, mbna_contour1, 0.0, 0.0);
    drawTrack(fixedPainter, swathraw1, 0.0, 0.0);
  } else if (project.refgrid_status == MBNA_REFGRID_LOADED) {
    drawContours(fixedPainter, mbna_contour1, 0.0, 0.0);
  }
  drawContours(movingPainter, mbna_contour2, builtOffsetX_, builtOffsetY_);
  drawTrack(movingPainter, swathraw2, builtOffsetX_, builtOffsetY_);
}

void ContourLayers::paint(QPainter &painter, const Session &session) {
  if (!session.crossingLoaded()) {
    painter.fillRect(0, 0, width_, height_, colorOf(mbna_color_background));
    return;
  }

  int dx = 0, dy = 0;
  const auto shift = [&] {
    dx = static_cast<int>(std::lround(mbna_plotx_scale * (mbna_offset_x - builtOffsetX_)));
    dy = -static_cast<int>(std::lround(mbna_ploty_scale * (mbna_offset_y - builtOffsetY_)));
  };
  if (!stale_) {
    shift();
    /* moved beyond what the layer holds */
    stale_ = std::abs(dx) >= margin_ || std::abs(dy) >= margin_;
  }
  if (stale_) {
    rebuild();
    shift();
  }

  painter.drawImage(0, 0, fixed_);
  painter.drawImage(dx - margin_, dy - margin_, moving_);

  /* the tie points and boxes are drawn again for every frame */
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    const struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
    const struct mbna_section *section1 = &project.files[crossing->file_id_1].sections[crossing->section_1];
    const struct mbna_section *section2 = &project.files[crossing->file_id_2].sections[crossing->section_2];
    for (int i = 0; i < crossing->num_ties; i++) {
      const struct mbna_tie *tie = &crossing->ties[i];
      if (i == mbna_current_tie)
        drawTie(painter, section1, mbna_snav_1, section2, mbna_snav_2, mbna_offset_x, mbna_offset_y, 6, 13,
                mbna_allow_set_tie ? MB_COLOR_RED : MB_COLOR_YELLOW);
      else
        drawTie(painter, section1, tie->snav_1, section2, tie->snav_2, mbna_offset_x, mbna_offset_y, 3, 7, MB_COLOR_YELLOW);
    }
  } else {
    const struct mbna_section *section = &project.files[mbna_current_file].sections[mbna_current_section];
    if (section->globaltie.status != MBNA_TIE_NONE)
      drawTie(painter, section, mbna_snav_2, section, mbna_snav_2, mbna_offset_x, mbna_offset_y, 6, 13,
              mbna_allow_set_tie ? MB_COLOR_RED : MB_COLOR_YELLOW);
  }

  const int x1 = plotX(mbna_overlap_lon_min);
  const int y1 = plotY(mbna_overlap_lat_min);
  const int x2 = plotX(mbna_overlap_lon_max);
  const int y2 = plotY(mbna_overlap_lat_max);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(penOf(mbna_color_foreground, XG_DASHLINE));
  painter.drawRect(MIN(x1, x2), MIN(y1, y2), MAX(x1, x2) - MIN(x1, x2), MAX(y1, y2) - MIN(y1, y2));

  if (session.zooming()) {
    painter.setPen(penOf(mbna_color_foreground, XG_SOLIDLINE));
    painter.drawRect(MIN(mbna_zoom_x1, mbna_zoom_x2), MIN(mbna_zoom_y1, mbna_zoom_y2),
                     MAX(mbna_zoom_x1, mbna_zoom_x2) - MIN(mbna_zoom_x1, mbna_zoom_x2),
                     MAX(mbna_zoom_y1, mbna_zoom_y2) - MIN(mbna_zoom_y1, mbna_zoom_y2));
  }
}

QImage ContourLayers::render(const Session &session) {
  QImage image(width_, height_, QImage::Format_ARGB32_Premultiplied);
  QPainter painter(&image);
  paint(painter, session);
  painter.end();
  return image;
}

/*--------------------------------------------------------------------*/

PlotSet::PlotSet(Session &session, QObject *parent)
    : QObject(parent), session_(session), contour_(601, 601), misfit_(302, 302, true), zOffset_(301, 61, true), model_(1000, 600, true) {
  session_.setCrossingSurfaces(&contour_, &misfit_, &zOffset_);
  session_.setPlotsChanged([this] { emit changed(); });
}

PlotSet::~PlotSet() {
  session_.setPlotsChanged(nullptr);
  session_.setCrossingSurfaces(nullptr, nullptr, nullptr);
}

void PlotSet::applySizes() {
  session_.setCrossingPlotSizes(contour_.width() - 1, contour_.height() - 1, misfit_.width() - 1, misfit_.height() - 1,
                                zOffset_.width() - 1, zOffset_.height() - 1);
}

void PlotSet::freeze() {
  if (frozen_)
    return;
  frozenCrossingShown_ = session_.crossingLoaded();
  frozenContour_ = contour_.render(session_);
  frozenMisfit_ = misfit_.image().copy();
  frozenZOffset_ = zOffset_.image().copy();
  frozenModel_ = model_.image().copy();
  frozen_ = true;
}

void PlotSet::unfreeze() {
  if (!frozen_)
    return;
  frozen_ = false;
  contour_.markStale();
  if (pendingContour_.isValid())
    setContourSize(pendingContour_.width(), pendingContour_.height());
  if (pendingMisfit_.isValid())
    setMisfitSize(pendingMisfit_.width(), pendingMisfit_.height());
  if (pendingZOffset_.isValid())
    setZOffsetSize(pendingZOffset_.width(), pendingZOffset_.height());
  if (pendingModel_.isValid())
    setModelSize(pendingModel_.width(), pendingModel_.height());
  pendingContour_ = pendingMisfit_ = pendingZOffset_ = pendingModel_ = QSize();
  emit changed();
}

bool PlotSet::crossingShown() const { return frozen_ ? frozenCrossingShown_ : session_.crossingLoaded(); }

void PlotSet::paintContour(QPainter &painter) {
  if (frozen_)
    painter.drawImage(0, 0, frozenContour_);
  else
    contour_.paint(painter, session_);
}

void PlotSet::setContourSize(int width, int height) {
  if (frozen_) {
    pendingContour_ = QSize(width, height);
    return;
  }
  if (width == contour_.width() && height == contour_.height())
    return;
  contour_.resize(width, height);
  applySizes();
}

void PlotSet::setMisfitSize(int width, int height) {
  if (frozen_) {
    pendingMisfit_ = QSize(width, height);
    return;
  }
  if (width == misfit_.width() && height == misfit_.height())
    return;
  misfit_.resize(width, height, !dark_);
  applySizes();
}

void PlotSet::setZOffsetSize(int width, int height) {
  if (frozen_) {
    pendingZOffset_ = QSize(width, height);
    return;
  }
  if (width == zOffset_.width() && height == zOffset_.height())
    return;
  zOffset_.resize(width, height, !dark_);
  applySizes();
}

void PlotSet::setModelSize(int width, int height) {
  if (frozen_) {
    pendingModel_ = QSize(width, height);
    return;
  }
  if (width == model_.width() && height == model_.height())
    return;
  model_.resize(width, height, !dark_);
  session_.resizeModelPlot(width, height);
}

void PlotSet::openModelPlot() { session_.openModelPlot(&model_, model_.width(), model_.height()); }

void PlotSet::setDark(bool dark) {
  if (dark == dark_)
    return;
  dark_ = dark;
  session_.setDarkBackground(dark);
  contour_.markStale();
  misfit_.resize(misfit_.width(), misfit_.height(), !dark_);
  zOffset_.resize(zOffset_.width(), zOffset_.height(), !dark_);
  model_.resize(model_.width(), model_.height(), !dark_);
  session_.replotCrossing();
  session_.replotModelPlot();
  emit changed();
}

bool PlotSet::exportImages(const QString &prefix) {
  bool ok = true;
  if (session_.crossingLoaded()) {
    ok = contour_.render(session_).save(prefix + "_contour.png") && ok;
    ok = misfit_.image().save(prefix + "_misfit.png") && ok;
    ok = zOffset_.image().save(prefix + "_zoffset.png") && ok;
  }
  if (session_.modelPlotOpen())
    ok = model_.image().save(prefix + "_model.png") && ok;
  return ok;
}

long PlotSet::compareContourWithShared() {
  if (!session_.crossingLoaded())
    return 0;
  ImageSurface shared(contour_.width(), contour_.height(), !dark_);
  session_.setCrossingSurfaces(&shared, &misfit_, &zOffset_);
  session_.replotCrossing();
  const QImage reference = shared.image().copy();
  session_.setCrossingSurfaces(&contour_, &misfit_, &zOffset_);
  session_.replotCrossing();
  const QImage layers = contour_.render(session_);
  long different = 0;
  for (int y = 0; y < reference.height(); y++) {
    const QRgb *a = reinterpret_cast<const QRgb *>(reference.constScanLine(y));
    const QRgb *b = reinterpret_cast<const QRgb *>(layers.constScanLine(y));
    for (int x = 0; x < reference.width(); x++) {
      if (a[x] != b[x])
        different++;
    }
  }
  return different;
}

}  // namespace mbnaq
