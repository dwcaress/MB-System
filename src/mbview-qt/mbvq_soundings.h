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
 * Action and Dismiss menus, and the soundings drawing of the backend. The
 * content is SoundingsWidget; SoundingsWindow is the window of the standalone
 * programs. One per program, as in Motif.
 */

#ifndef MBVIEW_QT_MBVQ_SOUNDINGS_H
#define MBVIEW_QT_MBVQ_SOUNDINGS_H

#include <QList>
#include <QMainWindow>
#include <functional>

#include "mbvq_bridge.h"

class QAction;
class QLabel;
class QMenu;
class QMenuBar;
class QRadioButton;
class QSlider;
class QVBoxLayout;

namespace mbvq {

class SoundingsWidget final : public QWidget {
  Q_OBJECT

 public:
  explicit SoundingsWidget(QWidget *parent = nullptr);

  QSize sizeHint() const override { return QSize(1040, 600); }

  /* for the program around the editor, the same calls as mbe::EditorWidget */
  QString title() const { return title_; }
  void setTitle(const QString &title);
  /* View, Mouse, Action and Dismiss, to show in the bar of the host */
  QList<QMenu *> menus() const { return menus_; }
  /* the menus in a bar inside this widget */
  void setMenuBarShown(bool shown);
  /* as the Dismiss menu; the program is told through its dismiss function */
  void dismiss();
  /* True: the edits go to the program as they are made, so nothing has to wait */
  bool prepareToClose(std::function<void()> closed_later = {});

  /* for the library: show the modes of the core */
  void refresh();
  void set_mousemode(const QString &text);
  void set_status(const QString &text, bool sliders);
  void set_view_cursor(int cursor);
  QWidget *view() const { return view_; }
  /* free the drawing view (and its OpenGL context) now */
  void delete_view();

 signals:
  void titleChanged(const QString &title);
  void statusMessage(const QString &text);
  /* the core shows the editor, or hides or ends it */
  void opened();
  void closed();

 private:
  QMenu *new_menu(const QString &title);

  QString title_;
  QList<QMenu *> menus_;
  QVBoxLayout *layout_ = nullptr;
  QMenuBar *menu_bar_ = nullptr;
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

class SoundingsWindow final : public QMainWindow {
 public:
  explicit SoundingsWindow(SoundingsWidget *widget);

  SoundingsWidget *widget() const { return widget_; }

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  SoundingsWidget *widget_;
};

/* the content of the soundings window, nullptr if there is none */
SoundingsWidget *soundings_widget();

/* the standalone soundings window, nullptr if it has none or a host placed the widget */
SoundingsWindow *soundings_window();

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_SOUNDINGS_H */
