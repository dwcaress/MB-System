/*--------------------------------------------------------------------
 *    The MB-system:  mbne_canvas.h  9/11/2026
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

#ifndef MBNAVEDIT_QT_MBNE_CANVAS_H
#define MBNAVEDIT_QT_MBNE_CANVAS_H

#include <QImage>
#include <QPalette>
#include <QPoint>
#include <QWidget>

#include "mbne_core.h"

namespace mbne {

/* what the plots drew, for comparing with the Motif program */
struct DrawCounts {
  long fill_black = 0;
  long rect_red = 0;
  long rect_purple = 0;
  long rect_orange = 0;
  long line_green = 0;
  long line_blue = 0;
};

constexpr int MIN_PLOT_HEIGHT = 110;

/* sets the size and margins of the plots of a session for a drawing area, and lays them out */
void layout_plots(Session &session, const QSize &size, const QFont &font);

/* draws all of the plots of a session, laid out already */
void draw_plots(QPainter &painter, const Session &session, const QSize &size, const QPalette &palette, DrawCounts *counts = nullptr);

/* The plots: drawn from the session, and the pointer and keys turned into calls of the session. */
class Canvas : public QWidget {
  Q_OBJECT

 public:
  enum Mode { MODE_PICK = 0, MODE_SELECT = 1, MODE_DESELECT = 2, MODE_SELECTALL = 3, MODE_DESELECTALL = 4 };

  explicit Canvas(Session &session, QWidget *parent = nullptr);

  Mode mode() const { return mode_; }
  void setMode(Mode mode);
  bool intervalMode() const { return interval_mode_; }
  void setIntervalMode(bool on);

  /* after the data or the settings changed */
  void layoutChanged();
  void redraw();

 signals:
  void modeChanged(int mode);
  void selectionChanged();
  void viewChanged();

 protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void changeEvent(QEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

 private:
  void updateCursor();
  void applyAt(const QPoint &position);

  Session &session_;
  Mode mode_ = MODE_PICK;
  bool interval_mode_ = false;
  bool dragging_ = false;
  QPoint last_position_;
  QImage image_;
  bool dirty_ = true;
};

}  // namespace mbne

#endif
