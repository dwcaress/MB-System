/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_plots.h  10/11/2026
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
 * The plots of mbnavadjust-qt as images. ImageSurface receives the drawing of
 * the shared plot code. ContourLayers draws the contour plot from two cached
 * images, so that dragging one section over the other does not redraw every
 * contour. PlotSet owns the images of one Session. This needs QtGui only.
 */

#ifndef MBNAQ_PLOTS_H_
#define MBNAQ_PLOTS_H_

#include <QColor>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QSize>
#include <QString>

#include <array>

#include "mbnaq_core.h"

class QPaintDevice;

namespace mbnaq {

/* The colors the plot code uses: it passes indexes, and these are the colors. */
const std::array<QRgb, 256> &plotPalette();

class ImageSurface : public DrawSurface {
 public:
  ImageSurface(int width, int height, bool whiteBackground);
  ~ImageSurface() override;

  void line(int x1, int y1, int x2, int y2, unsigned int pixel, int style) override;
  void rectangle(int x, int y, int width, int height, unsigned int pixel, int style) override;
  void fillRectangle(int x, int y, int width, int height, unsigned int pixel, int style) override;
  void text(int x, int y, const char *string, unsigned int pixel, int style) override;
  void measure(const char *string, int *width, int *ascent, int *descent) override;
  void clip(int x, int y, int width, int height) override;

  /* in pixels; the contents are lost */
  void resize(int width, int height, bool whiteBackground);
  int width() const { return image_.width(); }
  int height() const { return image_.height(); }
  const QImage &image();

 private:
  void begin();

  QImage image_;
  QPainter painter_;
  bool clipped_ = false;
  QRect clip_;
};

/* The contour plot: the shared code still runs, for what it computes, but its
   drawing on this surface is dropped. A full redraw there marks the layers stale. */
class ContourLayers : public DrawSurface {
 public:
  explicit ContourLayers(int width, int height);

  void line(int, int, int, int, unsigned int, int) override {}
  void rectangle(int, int, int, int, unsigned int, int) override {}
  void fillRectangle(int x, int y, int width, int height, unsigned int pixel, int style) override;
  void text(int, int, const char *, unsigned int, int) override {}
  void measure(const char *string, int *width, int *ascent, int *descent) override;
  void clip(int, int, int, int) override {}

  void resize(int width, int height);
  int width() const { return width_; }
  int height() const { return height_; }
  void markStale() { stale_ = true; }
  /* draws the plot as it is now, with the section at the working offset */
  void paint(QPainter &painter, const Session &session);
  /* the same on a new image */
  QImage render(const Session &session);

 private:
  void rebuild();

  int width_;
  int height_;
  bool stale_ = true;
  int margin_ = 0;
  QImage fixed_;
  QImage moving_;
  double builtOffsetX_ = 0.0;
  double builtOffsetY_ = 0.0;
};

/* The images of one Session and the plot sizes. */
class PlotSet : public QObject {
  Q_OBJECT

 public:
  explicit PlotSet(Session &session, QObject *parent = nullptr);
  ~PlotSet() override;

  Session &session() { return session_; }
  ContourLayers &contour() { return contour_; }
  const QImage &misfitImage() { return frozen_ ? frozenMisfit_ : misfit_.image(); }
  const QImage &zOffsetImage() { return frozen_ ? frozenZOffset_ : zOffset_.image(); }
  const QImage &modelImage() { return frozen_ ? frozenModel_ : model_.image(); }

  /* sizes in pixels */
  void setContourSize(int width, int height);
  void setMisfitSize(int width, int height);
  void setZOffsetSize(int width, int height);
  void setModelSize(int width, int height);
  void openModelPlot();

  /* true for white lines on black */
  void setDark(bool dark);
  bool dark() const { return dark_; }

  /* the current plots as PNG files named after the prefix */
  bool exportImages(const QString &prefix);
  /* During a background job the program logic is in use, so the widgets show copies. */
  void freeze();
  void unfreeze();
  bool frozen() const { return frozen_; }
  /* whether the crossing plots have a crossing to show */
  bool crossingShown() const;
  void paintContour(QPainter &painter);

  /* the contour plot from the layers compared with the shared plot code; the number of pixels that differ */
  long compareContourWithShared();

 signals:
  void changed();

 private:
  void applySizes();

  Session &session_;
  ContourLayers contour_;
  ImageSurface misfit_;
  ImageSurface zOffset_;
  ImageSurface model_;
  bool dark_ = false;
  bool frozen_ = false;
  bool frozenCrossingShown_ = false;
  QImage frozenContour_, frozenMisfit_, frozenZOffset_, frozenModel_;
  QSize pendingContour_, pendingMisfit_, pendingZOffset_, pendingModel_;
};

}  // namespace mbnaq

#endif
