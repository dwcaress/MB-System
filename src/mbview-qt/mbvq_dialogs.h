/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_dialogs.h  11/7/2026
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
 * The Controls dialogs of an mbview instance window, as in the Motif window:
 * colours and contours, 2D and 3D view parameters, shading, resolution and
 * projections. They show the values of the core and pass what the user
 * enters to the core functions; they hold no view logic.
 */

#ifndef MBVIEW_QT_MBVQ_DIALOGS_H
#define MBVIEW_QT_MBVQ_DIALOGS_H

#include <QObject>
#include <QString>
#include <cstddef>

#include "mbvq_bridge.h"

class QDialog;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSlider;
class QWidget;

namespace mbvq {

class ControlDialogs final : public QObject {
 public:
  ControlDialogs(size_t instance, QWidget *window);

  /* show a dialog (MBVQ_DIALOG_*) with the current values */
  void show_dialog(int dialog);
  /* view values changed in the core (MBV_VALUES_*, MBVQ_VALUES_ALL_*) */
  void values_changed(int values);
  /* the projection label text of the core */
  void set_projection_label(const QString &text);
  QDialog *dialog(int dialog) const { return dialogs_[dialog]; }
  QLineEdit *field(int field) const { return fields_[field]; }
  QRadioButton *hot_to_cold(int toggle) const { return htoc_[toggle]; }

 private:
  QDialog *make_dialog(int dialog, const QString &title);
  QLineEdit *make_field(int field);
  void make_mode_buttons(int toggle, QWidget *parent_layout_owner);
  void build_colorbounds();
  void build_shade();
  void build_view3d();
  void build_view2d();
  void build_resolution();
  void build_projection();
  /* load the fields of a dialog from the core */
  void load(int dialog);
  void load_fields(const int *fields, int count);
  void apply(int dialog);
  void resolution_changed();

  size_t instance_;
  QWidget *window_;
  QDialog *dialogs_[MBVQ_DIALOGS] = {};
  QLineEdit *fields_[MBVQ_FIELDS] = {};
  QRadioButton *htoc_[MBVQ_TOGGLES] = {};
  QRadioButton *ctoh_[MBVQ_TOGGLES] = {};
  QSlider *resolution_[4] = {};
  QLabel *resolution_value_[4] = {};
  QLabel *projection_label_ = nullptr;
  QString projection_text_;
  QRadioButton *projection_[3] = {};
  QRadioButton *lonlat_[2] = {};
};

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_DIALOGS_H */
