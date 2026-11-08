/*--------------------------------------------------------------------
 *    The MB-system:  mbe_canvas.h  2/11/2026
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

#ifndef MBEDIT_QT_MBE_CANVAS_H
#define MBEDIT_QT_MBE_CANVAS_H

#include <QImage>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QPolygon>
#include <QWidget>

#include "mbe_core.h"

class QTimer;

namespace mbe {

/* The soundings and the mouse and key actions on them. The soundings are drawn
   once into an image, which is rebuilt when anything changes; the highlight of
   the sounding under the mouse and the grab rectangle are drawn over it. */
class Canvas : public QWidget {
  Q_OBJECT

 public:
  enum Mode { MODE_TOGGLE = 0, MODE_PICK, MODE_ERASE, MODE_RESTORE, MODE_GRAB, MODE_INFO, MODE_LASSO };

  explicit Canvas(mbe::Session &session, QWidget *parent = nullptr);

  Mode mode() const { return mode_; }
  void setMode(Mode mode);
  bool reverseKeys = false;
  bool reverseMouse = false;
  int stepSize = 5;
  QString fileText;
  /* what would change, drawn over the soundings; not owned */
  void setPreview(const mbe::FlagPreview *preview);
  void setMarkedPing(int ping);

  /* after the session changed: follow the size of the widget and draw again */
  void layoutChanged();
  void redraw();
  /* an opaque picture of the plot, whatever the state of the widget */
  QImage render(QSize size, double ratio);

 signals:
  void edited();             // flags changed
  void moved();              // the pings in view changed
  void modeKey(int mode);    // the mode changed by a key
  void viewKey(int view);    // the view changed by a key
  void viewChanged();        // the width or exaggeration changed by zooming
  void suspectKey(int direction);
  void contextMenuAt(QPoint global, int ping, int beam);  // Shift and the right button
  void soundingInfo(const QStringList &lines);            // a click in Info mode

 protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void changeEvent(QEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;
  void leaveEvent(QEvent *event) override;
  bool event(QEvent *event) override;

 private:
  void setPlotRectFor(QSize size);
  void paintPlot(QImage &image, double ratio);
  void editAt(int x, int y, int grab_mode);
  void applyHeldKey();
  void stepBy(int direction);
  void stepRepeat();
  void updateCursor();
  void updateHover(QPoint position);

  mbe::Session &session_;
  QImage image_;
  bool image_ok_ = false;
  Mode mode_ = MODE_TOGGLE;
  bool dragging_ = false;
  QPoint last_;
  int step_direction_ = 0;
  QTimer *step_timer_;
  const mbe::FlagPreview *preview_ = nullptr;
  int marked_ping_ = -1;
  QPolygon lasso_;
  bool key_space_ = false, panning_ = false;
  bool stroke_ = false;
  bool key_g_ = false, key_z_ = false, key_s_ = false, key_a_ = false, key_d_ = false;
  int wheel_ = 0;
  bool hover_ = false;
  int hover_ping_ = 0, hover_beam_ = 0;
};

}  // namespace mbe

#endif
