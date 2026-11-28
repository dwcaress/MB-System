/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_dialogs.h  11/7/2026
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
 * The dialogs of the mbgrdviz-qt main program, as in the Motif mbgrdviz: the
 * survey generation dialog and the realtime setup dialog.
 */

#ifndef MBGRDVIZ_QT_DIALOGS_H
#define MBGRDVIZ_QT_DIALOGS_H

#include <QDialog>

class QButtonGroup;
class QComboBox;
class QLabel;
class QLineEdit;
class QSlider;
class QSpinBox;

/* "Generate Survey Lines from Area": the fields set the survey state of the
   core (mbgrdviz_qt_survey_set), Generate Route makes the route */
class SurveyDialog final : public QDialog {
 public:
  explicit SurveyDialog(QWidget *parent);
  static SurveyDialog *instance(QWidget *parent);

  /* fill the fields from the survey state, without reporting changes */
  void load();
  void set_sensitivity(bool linespacing, bool platform, bool swathwidth, bool altitude, bool depth);
  void set_info(const char *info_text);
  void set_name(const char *name);

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  void changed();
  void dismiss();

  bool loading_ = false;
  QLabel *info_ = nullptr;
  QLineEdit *name_ = nullptr;
  QComboBox *color_ = nullptr;
  QComboBox *mode_ = nullptr;
  QComboBox *platform_ = nullptr;
  QComboBox *direction_ = nullptr;
  QSpinBox *crosslines_ = nullptr;
  QComboBox *crosslines_last_ = nullptr;
  QSpinBox *interleaving_ = nullptr;
  QSpinBox *linespacing_ = nullptr;
  QSpinBox *swathwidth_ = nullptr;
  QSpinBox *altitude_ = nullptr;
  QSpinBox *depth_ = nullptr;
};

/* "Realtime Setup": as in Motif, it only records the navigation source; the
   realtime display itself is not implemented in mbgrdviz */
class RealtimeDialog final : public QDialog {
 public:
  explicit RealtimeDialog(QWidget *parent);
  static RealtimeDialog *instance(QWidget *parent);

  void set_path(const QString &path);

 private:
  QLineEdit *path_ = nullptr;
};

#endif /* MBGRDVIZ_QT_DIALOGS_H */
