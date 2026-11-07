/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_window.h  11/7/2026
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
 * The window of an mbview instance: the map view of the backend, the
 * mouse mode controls, the pick information and the status line.
 */

#ifndef MBVIEW_QT_MBVQ_WINDOW_H
#define MBVIEW_QT_MBVQ_WINDOW_H

#include <QMainWindow>
#include <QString>
#include <cstddef>

class QAction;
class QActionGroup;
class QButtonGroup;
class QLabel;
class QRadioButton;

namespace mbvq {

class MapWindow final : public QMainWindow {
 public:
  explicit MapWindow(size_t instance);

  QWidget *map_view() const { return map_view_; }
  /* show the mouse mode and the view toggles of the core */
  void refresh_modes();
  void set_status(const QString &text);
  /* a label the core writes (MBVQ_LABEL_*) */
  void set_label(int label, const QString &text);
  QString label(int label) const;
  void set_map_cursor(int cursor);

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  static constexpr int kMouseModes = 9;
  size_t instance_;
  QWidget *map_view_ = nullptr;
  QLabel *status_ = nullptr;
  QLabel *pick_info_ = nullptr;
  QString labels_[3];
  QActionGroup *mouse_actions_ = nullptr;
  QButtonGroup *mouse_buttons_ = nullptr;
  QAction *mouse_action_[kMouseModes] = {};
  QRadioButton *mouse_button_[kMouseModes] = {};
};

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_WINDOW_H */
