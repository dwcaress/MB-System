/*--------------------------------------------------------------------
 *    The MB-system:  wcd_plot.h  1/11/2026
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
 * Plot of one water column swath: the amplitude cells are drawn as a triangle mesh
 * on their (ray traced or straight) positions, coloured through a colour map
 * texture, with QRhi (wcd_swath_rhi.h) or with OpenGL (SwathGL); axes, colour bar,
 * soundings and the water column bottom line are drawn with QPainter on top. Used
 * by the window (WcdView) and for image files (render(), also without a window).
 */

#ifndef MBWCDVIEW_WCD_PLOT_H
#define MBWCDVIEW_WCD_PLOT_H

#include <memory>
#include <vector>

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QRect>
#include <QRectF>
#include <QString>

#ifdef MBWCD_HAVE_GL
#include <QOpenGLFunctions>

class QOpenGLBuffer;
class QOpenGLShaderProgram;
class QOpenGLTexture;
#endif

namespace wcd {

/* the swath is drawn with QRhi where it is built, unless MB_RENDERER=legacy-gl
   asks for OpenGL where that is built too */
bool use_rhi();

/* the colour map as a 256 x 1 RGBA image */
QImage colormap_image();

struct Sounding {
  double across, depth;
  QColor color;
  bool filled;
};

/* everything drawn for one swath */
struct PlotData {
  int nbeams = 0, nsamples = 0;
  std::vector<float> across, depth, amplitude;     // nbeams x nsamples, NaN = nothing to draw
  std::vector<double> detect_across, detect_depth;  // water column bottom detections
  std::vector<Sounding> soundings;
  bool show_wc_bottom = false;
  QString ylabel = "Depth (m)";
};

#ifdef MBWCD_HAVE_GL
/* OpenGL resources of the mesh in one context */
class SwathGL : protected QOpenGLFunctions {
 public:
  SwathGL();
  ~SwathGL();
  void init();
  void upload(const std::vector<float> &vertices, const std::vector<unsigned int> &indices);
  /* draw into the current viewport; transform maps data (across, depth) to normalized coordinates */
  void draw(float sx, float ox, float sz, float oz, float vmin, float vmax);

 private:
  std::unique_ptr<QOpenGLShaderProgram> program_;
  std::unique_ptr<QOpenGLBuffer> vbo_, ibo_;
  std::unique_ptr<QOpenGLTexture> lut_;
  int nindices_ = 0;
};
#endif

class WcdPlot {
 public:
  void setData(PlotData data);
  void setColorRange(double vmin, double vmax);
  void setDepthWindow(double dmin, double dmax);
  void setAcrossMax(double xmax);  // 0: from the data
  const PlotData &data() const { return data_; }

  /* image area inside a widget/image of the given size (logical pixels) */
  QRectF plotRect(const QSize &size) const;
  bool toData(const QPointF &pixel, const QSize &size, double &across, double &depth) const;

  /* mesh for SwathGL::upload() and SwathRhi::upload(); changes with the data and the depth window */
  bool meshChanged() const { return mesh_dirty_; }
  void requestMesh() { mesh_dirty_ = true; }  // a new OpenGL context or QRhi has no mesh yet
  void buildMesh(std::vector<float> &vertices, std::vector<unsigned int> &indices);

  /* the image area in device pixels, from the bottom left corner as OpenGL and QRhi count */
  QRect deviceViewport(const QSize &size, double dpr) const;
  /* data (across, depth) to normalized device coordinates: across * sx + ox, depth * sz + oz */
  void deviceTransform(float &sx, float &ox, float &sz, float &oz) const;
  double colorMin() const { return vmin_; }
  double colorMax() const { return vmax_; }

#ifdef MBWCD_HAVE_GL
  void drawGL(SwathGL &gl, const QSize &size, double dpr);
#endif
  void drawOverlay(QPainter &painter, const QSize &size, const QString &caption = QString()) const;

  /* image of the plot (no window needed) at size x scale pixels */
  QImage render(const QSize &size, double scale, const QString &caption) const;

 private:
  void updateView();
  PlotData data_;
  double vmin_ = -60.0, vmax_ = 0.0;
  double dmin_ = 0.0, dmax_ = 1.0e5;
  double xmax_ = 0.0;
  double view_x0_ = -1.0, view_x1_ = 1.0, view_z0_ = 0.0, view_z1_ = 1.0;
  bool mesh_dirty_ = true;
};

}  // namespace wcd

#endif
