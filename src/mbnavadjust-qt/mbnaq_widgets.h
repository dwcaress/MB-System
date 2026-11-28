/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_widgets.h  10/11/2026
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
 * The widgets of mbnavadjust-qt, each a view of one ProjectController, so a
 * host program can place them separately.
 */

#ifndef MBNAQ_WIDGETS_H_
#define MBNAQ_WIDGETS_H_

#include <QMap>
#include <QString>
#include <QStringList>
#include <QModelIndex>
#include <QWidget>

#include <string>
#include <vector>

class QAbstractButton;
class QButtonGroup;
class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QSortFilterProxyModel;
class QTableView;

namespace mbnaq {

class ProjectController;
class ListTableModel;
class ContourView;
class MisfitView;
class ZOffsetView;
class ModelPlotView;

/* The list of the project as a table with sortable columns. */
class ProjectBrowser : public QWidget {
  Q_OBJECT

 public:
  explicit ProjectBrowser(ProjectController &controller, QWidget *parent = nullptr);
  QTableView *table() { return table_; }

 protected:
  /* Enter on the table opens the current row, like a click */
  bool eventFilter(QObject *object, QEvent *event) override;

 private:
  void refresh();
  void rowClicked(const QModelIndex &index);

  ProjectController &controller_;
  QLabel *title_;
  QTableView *table_;
  ListTableModel *model_;
  QSortFilterProxyModel *proxy_;
  int shownSelection_ = -1;
};

/* The project summary. */
class ProjectStatus : public QWidget {
  Q_OBJECT

 public:
  explicit ProjectStatus(ProjectController &controller, QWidget *parent = nullptr);

 private:
  void refresh();

  ProjectController &controller_;
  QLabel *text_;
};

/* What the project steps report. */
class MessageLog : public QWidget {
  Q_OBJECT

 public:
  explicit MessageLog(ProjectController &controller, QWidget *parent = nullptr);

 private:
  QPlainTextEdit *text_;
};

/* The crossing being analyzed: the three plots and the tie controls. */
class CrossingEditor : public QWidget {
  Q_OBJECT

 public:
  explicit CrossingEditor(ProjectController &controller, QWidget *parent = nullptr);
  QPushButton *button(const char *name) const { return buttons_.value(name, nullptr); }
  ContourView *contourView() { return contour_; }
  MisfitView *misfitView() { return misfit_; }
  ZOffsetView *zOffsetView() { return zOffset_; }

 signals:
  void biasesRequested();
  void controlsRequested();

 private:
  void refresh();
  QPushButton *addButton(const char *name, const QString &label);

  ProjectController &controller_;
  ContourView *contour_;
  MisfitView *misfit_;
  ZOffsetView *zOffset_;
  QLabel *statusText_;
  QLabel *offsetText_;
  QRadioButton *centerZero_;
  QRadioButton *centerCurrent_;
  QMap<QByteArray, QPushButton *> buttons_;
};

/* The inversion results. */
class ModelPlot : public QWidget {
  Q_OBJECT

 public:
  explicit ModelPlot(ProjectController &controller, QWidget *parent = nullptr);
  ModelPlotView *view() { return view_; }

 signals:
  void dismissed();

 private:
  void refresh();

  ProjectController &controller_;
  ModelPlotView *view_;
  QLabel *statusText_;
  QRadioButton *timeSeries_;
  QRadioButton *perturbation_;
  QRadioButton *tieOffsets_;
  QPushButton *clearBlock_;
};

/* Section, contour and inversion settings. */
class ControlsPanel : public QWidget {
  Q_OBJECT

 public:
  explicit ControlsPanel(ProjectController &controller, QWidget *parent = nullptr);
  void load();

 private:
  void apply();

  ProjectController &controller_;
  QSpinBox *decimation_;
  QDoubleSpinBox *sectionLength_;
  QSpinBox *sectionSoundings_;
  QDoubleSpinBox *contourInterval_;
  QDoubleSpinBox *colorInterval_;
  QDoubleSpinBox *tickInterval_;
  QDoubleSpinBox *smoothing_;
  QDoubleSpinBox *zOffsetWidth_;
  QRadioButton *usePrimary_;
  QRadioButton *useSecondary_;
  QRadioButton *useTertiary_;
};

/* The heading and roll biases of the two loaded sections. */
class BiasesPanel : public QWidget {
  Q_OBJECT

 public:
  explicit BiasesPanel(ProjectController &controller, QWidget *parent = nullptr);
  void load();

 private:
  void togglesChanged();

  ProjectController &controller_;
  QLabel *sections_;
  QRadioButton *together_;
  QRadioButton *separate_;
  QDoubleSpinBox *heading1_;
  QDoubleSpinBox *roll1_;
  QDoubleSpinBox *heading2_;
  QDoubleSpinBox *roll2_;
  QPushButton *apply_;
  QPushButton *applyAll_;
};

}  // namespace mbnaq

#endif
