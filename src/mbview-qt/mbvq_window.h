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
 * One whole mbview map as a widget (MapWidget): the map view of the backend,
 * the View, Controls, Mouse, Action and Dismiss menus, the mouse mode buttons,
 * the pick information and the status line. MapWindow is the window of the
 * standalone programs: a MapWidget with a menu bar and a status bar.
 */

#ifndef MBVIEW_QT_MBVQ_WINDOW_H
#define MBVIEW_QT_MBVQ_WINDOW_H

#include <QList>
#include <QMainWindow>
#include <QString>
#include <QWidget>
#include <cstddef>
#include <vector>

#include "mbvq_bridge.h"

class QAction;
class QActionGroup;
class QButtonGroup;
class QDialog;
class QLabel;
class QMenu;
class QMenuBar;
class QRadioButton;
class QVBoxLayout;

namespace mbvq {

class ControlDialogs;
class ProfileWindow;

class MapWidget final : public QWidget {
  Q_OBJECT

 public:
  explicit MapWidget(size_t instance, QWidget *parent = nullptr);

  QSize sizeHint() const override;

  /* for the program around the map */
  size_t instance() const { return instance_; }
  QString title() const { return title_; }
  void setTitle(const QString &title);
  /* View, Controls, Mouse, Action and Dismiss, to show in the bar of the host */
  QList<QMenu *> menus() const { return menus_; }
  /* the menus in a bar inside this widget, for several maps in one window */
  void setMenuBarShown(bool shown);
  /* the left panel, if a host wants it somewhere else */
  QWidget *controlPanel() const { return control_panel_; }
  /* as the Dismiss menu; the library removes this widget afterwards */
  void dismiss();

  /* for the library */
  QLabel *takeStatusLabel();
  QWidget *map_view() const { return map_view_; }
  /* delete the map view now, while Qt and its OpenGL contexts are alive; the
     widget itself may be deleted later, even during the exit of the program */
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
  /* the profile window */
  ProfileWindow *profile() const { return profile_; }
  /* a message while the program works (do_mbview_message_on/off) */
  void show_message(const QString &text, bool on);
  /* the About dialog */
  void show_about();
  /* Action menu */
  void add_action(int index, const QString &label);
  void add_action_separator();
  void set_action_enabled(int index, bool enabled);

 signals:
  void titleChanged(const QString &title);
  void statusMessage(const QString &text);
  void closed(size_t instance);

 private:
  QMenu *new_menu(const QString &title);
  void build_view_menu();

  size_t instance_;
  QString title_;
  QList<QMenu *> menus_;
  QVBoxLayout *outer_ = nullptr;
  QMenuBar *menu_bar_ = nullptr;
  QWidget *control_panel_ = nullptr;
  QWidget *map_view_ = nullptr;
  QLabel *status_ = nullptr;
  QLabel *pick_info_ = nullptr;
  QString labels_[MBVQ_LABEL_OTHER];
  QAction *view_action_[MBVQ_VIEW_ITEMS] = {};
  QAction *mouse_action_[MBVQ_MOUSE_MODES] = {};
  QRadioButton *mouse_button_[MBVQ_MOUSE_MODES] = {};
  QMenu *action_menu_ = nullptr;
  ControlDialogs *dialogs_ = nullptr;
  ProfileWindow *profile_ = nullptr;
  QDialog *about_ = nullptr;
  QDialog *message_ = nullptr;
  QLabel *message_label_ = nullptr;
  std::vector<QAction *> actions_;
};

class MapWindow final : public QMainWindow {
 public:
  explicit MapWindow(MapWidget *widget);

  MapWidget *widget() const { return widget_; }

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  MapWidget *widget_;
  bool closing_ = false;
};

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_WINDOW_H */
