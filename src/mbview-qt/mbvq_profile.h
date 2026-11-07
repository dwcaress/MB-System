/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_profile.h  11/7/2026
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
 * The profile window of an mbview instance, as the Motif "MBview Profile
 * Display": the profile drawing of the backend, its information label and
 * the width, slope threshold and exaggeration sliders.
 */

#ifndef MBVIEW_QT_MBVQ_PROFILE_H
#define MBVIEW_QT_MBVQ_PROFILE_H

#include <QWidget>
#include <cstddef>

class QLabel;
class QScrollArea;
class QSlider;

namespace mbvq {

class ProfileWindow final : public QWidget {
 public:
  ProfileWindow(size_t instance, QWidget *parent);

  /* View > Profile: show or dismiss */
  void toggle();
  void open();
  void dismiss();
  /* the profile information label of the core */
  void set_info(const QString &text);
  QWidget *view() const { return view_; }
  QSlider *slider(int control) const { return sliders_[control]; }

 protected:
  void closeEvent(QCloseEvent *event) override;
  bool eventFilter(QObject *watched, QEvent *event) override;

 private:
  /* the drawing is the width multiple times the visible width */
  void layout_profile();
  void control_changed(int control, int value);

  size_t instance_;
  QScrollArea *scroll_ = nullptr;
  QWidget *view_ = nullptr;
  QLabel *info_ = nullptr;
  QSlider *sliders_[3] = {};
  QLabel *values_[3] = {};
};

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_PROFILE_H */
