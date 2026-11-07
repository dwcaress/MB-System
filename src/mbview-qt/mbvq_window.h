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
 * The window of an mbview instance: the map view of the backend, the View,
 * Mouse, Action and Dismiss menus, the mouse mode buttons, the pick
 * information and the status line.
 */

#ifndef MBVIEW_QT_MBVQ_WINDOW_H
#define MBVIEW_QT_MBVQ_WINDOW_H

#include <QMainWindow>
#include <QString>
#include <cstddef>
#include <vector>

#include "mbvq_bridge.h"

class QAction;
class QActionGroup;
class QButtonGroup;
class QLabel;
class QMenu;
class QRadioButton;

namespace mbvq {

class ControlDialogs;

class MapWindow final : public QMainWindow {
 public:
  explicit MapWindow(size_t instance);

  QWidget *map_view() const { return map_view_; }
  /* delete the map view now, while Qt and its OpenGL contexts are alive; the
     window itself may be deleted later, even during the exit of the program */
  void delete_map_view();
  /* show the state of the core: checked and enabled menu items, mouse modes */
  void refresh_modes();
  void set_status(const QString &text);
  /* a label the core writes (MBVQ_LABEL_*) */
  void set_label(int label, const QString &text);
  QString label(int label) const;
  void set_map_cursor(int cursor);
  /* the Controls dialogs */
  ControlDialogs *dialogs() const { return dialogs_; }
  /* Action menu */
  void add_action(int index, const QString &label);
  void add_action_separator();
  void set_action_enabled(int index, bool enabled);

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  void build_view_menu();

  size_t instance_;
  QWidget *map_view_ = nullptr;
  QLabel *status_ = nullptr;
  QLabel *pick_info_ = nullptr;
  QString labels_[MBVQ_LABEL_OTHER];
  QAction *view_action_[MBVQ_VIEW_ITEMS] = {};
  QAction *mouse_action_[MBVQ_MOUSE_MODES] = {};
  QRadioButton *mouse_button_[MBVQ_MOUSE_MODES] = {};
  QMenu *action_menu_ = nullptr;
  ControlDialogs *dialogs_ = nullptr;
  std::vector<QAction *> actions_;
  bool closing_ = false;
};

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_WINDOW_H */
