/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_soundings.h  11/7/2026
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
 * The 3D soundings window of mbview-qt, as the Motif "3D Soundings" window
 * (Mb3dsdg): the edit mode buttons, the status line, the bias and time lag
 * sliders, Reset View, the mouse mode buttons and label, the View, Mouse,
 * Action and Dismiss menus, and the soundings drawing of the backend. One
 * window per program, as in Motif.
 */

#ifndef MBVIEW_QT_MBVQ_SOUNDINGS_H
#define MBVIEW_QT_MBVQ_SOUNDINGS_H

#include <QMainWindow>

#include "mbvq_bridge.h"

class QAction;
class QLabel;
class QRadioButton;
class QSlider;

namespace mbvq {

class SoundingsWindow final : public QMainWindow {
 public:
  SoundingsWindow();

  /* show the modes of the core */
  void refresh();
  void set_mousemode(const QString &text);
  void set_status(const QString &text, bool sliders);
  void set_view_cursor(int cursor);
  QWidget *view() const { return view_; }
  /* free the drawing view (and its OpenGL context) now */
  void delete_view();

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  QWidget *view_ = nullptr;
  QLabel *status_ = nullptr;
  QLabel *mousemode_ = nullptr;
  QRadioButton *edit_[MBVQ_SDG_EDIT_MODES] = {};
  QRadioButton *mouse_[2] = {};
  QAction *mouse_action_[2] = {};
  QAction *view_toggle_[4] = {};
  QAction *profiles_[3] = {};
  QAction *color_[4] = {};
  QWidget *sliders_ = nullptr;
  QSlider *bias_[MBVQ_SDG_BIASES] = {};
  QLabel *bias_value_[MBVQ_SDG_BIASES] = {};
};

/* the soundings window, nullptr if it has none */
SoundingsWindow *soundings_window();

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_SOUNDINGS_H */
