/*--------------------------------------------------------------------
 *    The MB-system:  MainWindow.h  2/11/2026
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

#ifndef MBVELOCITYTOOL_QT_MAINWINDOW_H
#define MBVELOCITYTOOL_QT_MAINWINDOW_H

#include <functional>
#include <vector>

#include <QDialog>
#include <QMainWindow>
#include <QString>

#include "mbvt_core.h"
#include "mbvt_plots.h"

class QAction;
class QActionGroup;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QProgressBar;
class QRadioButton;
class QSlider;
class QThread;

namespace mbvt {

struct PlotScale {
  double maxdepth = 3000.0;
  double velrange = 500.0;
  double velcenter = 1490.0;
  double resrange = 200.0;
};

/* modeless dialog with the sliders of the Plot Scaling button of mbvelocitytool */
class ScalingDialog : public QDialog {
  Q_OBJECT
 public:
  explicit ScalingDialog(QWidget *parent = nullptr);
  void setScale(const PlotScale &scale);
  PlotScale scale() const;

 signals:
  void changed();

 private:
  struct Row {
    QSlider *slider;
    QDoubleSpinBox *spin;
    double factor;
  };
  Row addRow(QFormLayout *form, const QString &label, double low, double high, int decimals);
  static void setRow(const Row &row, double value);

  Row maxdepth_, velrange_, velcenter_, resrange_;
};

/* how a long display profile becomes the edit profile: all of its nodes, or the fewer that keep its shape */
class ActivateDialog : public QDialog {
  Q_OBJECT
 public:
  ActivateDialog(const mbvt::Profile &profile, QWidget *parent = nullptr);
  double tolerance() const;  // m/s, 0 for all of the nodes

 private:
  QRadioButton *simplified_;
  QDoubleSpinBox *tolerance_;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  /* quiet: errors go to stderr instead of dialogs, and nothing asks anything */
  MainWindow(int verbose, int nthreads, int anglemode, const int btime_i[7], const int etime_i[7], bool quiet);
  ~MainWindow() override;

  bool openDisplayProfile(const QString &path);
  bool openEditProfile(const QString &path);
  void newEditProfile();
  void openSwath(const QString &path, int format);  // format 0 is identified from the name; runs in the background
  void reprocess();                                 // runs in the background
  void setAngleMode(int anglemode);

  /* tolerance in m/s, 0 for all of the nodes. The display profile stays; the edit profile it replaces joins the display profiles. */
  bool activateDisplayProfile(int index, double tolerance);
  void hideDisplayProfile(int index);
  void removeDisplayProfile(int index);
  void showHiddenProfiles();

  void waitUntilIdle();
  bool failed() const { return failed_; }  // an open or Reprocess failed
  bool saveImage(const QString &path, int width, int height);

  /* for the tests */
  mbvt::VelocityPlot *velocityPlot() { return velocity_plot_; }
  const mbvt::Profile &editProfile() const { return edit_; }
  const std::vector<mbvt::Profile> &displayProfiles() const { return display_; }
  const mbvt::Residuals &residuals() const { return residuals_; }
  bool residualsOutOfDate() const { return processed_version_ != edit_version_; }
  bool busy() const { return worker_ != nullptr; }
  bool editModified() const { return edit_modified_; }
  ScalingDialog *scalingDialog() { return scaling_dialog_; }

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  void createMenus();
  void runJob(const QString &message, std::function<void()> work, std::function<void()> done);
  void report(const QString &title, const QString &text);
  bool offerToSaveEdit();  // false when the user cancelled
  bool saveEditDialog();
  void openDisplayDialog();
  void openEditDialog();
  void openSwathDialog(bool ask_format);
  void saveSwathSvp();
  void saveResidualsAsOffsets();
  void showAbout();
  void applyScale();
  void editChanged();
  void updateState();
  void onProfileAction(int index, const QString &name, mbvt::ProfileAction action);
  bool saveDisplayDialog(int index);

  int verbose_, nthreads_, anglemode_;
  int btime_i_[7], etime_i_[7];
  bool quiet_;
  bool failed_ = false;

  mbvt::SwathFile swath_;
  bool have_edit_ = false;
  bool edit_modified_ = false;
  mbvt::Profile edit_;
  std::vector<mbvt::Profile> display_;
  mbvt::Residuals residuals_;
  bool have_residuals_ = false;
  double process_seconds_ = 0.0;
  long edit_version_ = 0;       // counts the changes of the edit profile and of the angle mode
  long processed_version_ = 0;  // that the residuals were calculated with
  PlotScale scale_;

  QThread *worker_ = nullptr;

  mbvt::VelocityPlot *velocity_plot_;
  mbvt::ResidualPlot *residual_plot_;
  mbvt::RaypathPlot *raypath_plot_;
  QLabel *swath_label_, *edit_label_, *display_label_, *pointer_label_, *result_label_;
  QProgressBar *progress_;
  ScalingDialog *scaling_dialog_;
  QAction *show_hidden_action_;
  QAction *open_swath_action_, *open_swath_format_action_, *save_edit_action_, *save_swath_action_, *save_residuals_action_,
      *reprocess_action_;
  QActionGroup *angle_group_;
};

}  // namespace mbvt

#endif
