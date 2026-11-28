/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_widgets.cc  10/11/2026
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

#include "mbnaq_widgets.h"

#include <QAbstractTableModel>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QTableView>
#include <QVBoxLayout>

#include <cmath>
#include <functional>

#include "mbnaq_controller.h"
#include "mbnaq_core.h"
#include "mbnaq_plots.h"
#include "mbnaq_views.h"

namespace mbnaq {

namespace {

QFont fixedFont() {
  QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  font.setPointSize(font.pointSize() - 1);
  return font;
}

QStringList tokens(const QString &text) { return text.simplified().split(' ', Qt::SkipEmptyParts); }

void addPadded(QStringList &columns, QStringList values, int count) {
  while (values.size() < count)
    values.append(QString());
  for (int i = 0; i < count; i++)
    columns.append(values[i]);
}

const QStringList kDateHeaders = {"Start Date", "Start Time", "End Date", "End Time"};
const QStringList kTieErrorHeaders = {"Sigma 1", "Sigma 2", "Sigma 3", "Error X", "Error Y", "Error Z", "Error", "Rel. Error", "Note"};

QStringList headersOf(int kind) {
  switch (kind) {
  case ListReferenceGrids:
    return {"Reference Grid"};
  case ListSurveys:
    return QStringList{"Survey", "Files"} + kDateHeaders + QStringList{"Status", "Global Ties"};
  case ListBlocks:
    return {"Block", "Survey 1", "Survey 2", "True", ">50%", ">25%", "All", "Ties"};
  case ListFiles:
    return {"File", "Status", "Sections", "Heading", "Roll", "Global Ties", "Path"};
  case ListSections:
    return QStringList{"Status", "Section"} + kDateHeaders +
           QStringList{"Nav", "Tie", "Offset X", "Offset Y", "Offset Z"} + kTieErrorHeaders + QStringList{"Reference Grid"};
  case ListCrossings:
  case ListMediocreCrossings:
  case ListGoodCrossings:
  case ListBetterCrossings:
  case ListTrueCrossings:
    return {"Status", "True", "Crossing", "Section 1", "Section 2", "Overlap %", "Ties"};
  case ListTies:
  case ListTiesSortedAll:
  case ListTiesSortedWorst:
  case ListTiesSortedBad:
    return QStringList{"Crossing", "Tie", "Status", "Section 1", "Section 2", "Offset X", "Offset Y", "Offset Z"} + kTieErrorHeaders;
  case ListGlobalTies:
  case ListGlobalTiesSorted:
    return QStringList{"Section", "Status", "Ref", "Offset X", "Offset Y", "Offset Z"} + kTieErrorHeaders + QStringList{"Reference Grid"};
  default:
    return {};
  }
}

/* Splits a list row, which is one line of text in the program logic, into columns. */
QStringList columnsOf(int kind, const QString &row) {
  QStringList columns;
  switch (kind) {
  case ListReferenceGrids:
    columns.append(row);
    break;
  case ListSurveys: {
    QStringList t = tokens(row);
    if (!t.isEmpty() && t.last().startsWith("g-ties:"))
      t.last().remove(0, 7);
    addPadded(columns, t, 8);
    break;
  }
  case ListBlocks: {
    QRegularExpressionMatchIterator it = QRegularExpression("\\d+").globalMatch(row);
    QStringList numbers;
    while (it.hasNext())
      numbers.append(it.next().captured(0));
    addPadded(columns, numbers, 8);
    break;
  }
  case ListFiles: {
    const QStringList t = tokens(row);
    QStringList head = t.mid(0, 6);
    if (head.size() == 6)
      head[5].remove("g-ties:");
    addPadded(columns, head, 6);
    columns.append(t.mid(6).join(' '));
    break;
  }
  case ListSections: {
    const QStringList parts = row.split('|');
    addPadded(columns, tokens(parts.value(0)), 6);
    addPadded(columns, tokens(parts.value(1)), 5);
    addPadded(columns, tokens(parts.value(2)), 3);
    addPadded(columns, tokens(parts.value(3)), 3);
    addPadded(columns, tokens(parts.value(4)), 3);
    columns.append(parts.value(5).trimmed());
    break;
  }
  case ListCrossings:
  case ListMediocreCrossings:
  case ListGoodCrossings:
  case ListBetterCrossings:
  case ListTrueCrossings:
    columns.append(row.left(1));
    columns.append(row.mid(1, 1).trimmed());
    addPadded(columns, tokens(row.mid(2)), 5);
    break;
  case ListTies:
  case ListTiesSortedAll:
  case ListTiesSortedWorst:
  case ListTiesSortedBad: {
    const QStringList parts = row.split('|');
    addPadded(columns, tokens(parts.value(0)), 8);
    addPadded(columns, tokens(parts.value(1)), 3);
    addPadded(columns, tokens(parts.value(2)), 3);
    addPadded(columns, tokens(parts.value(3)), 3);
    break;
  }
  case ListGlobalTies:
  case ListGlobalTiesSorted: {
    const QStringList parts = row.split('|');
    addPadded(columns, tokens(parts.value(0)), 6);
    addPadded(columns, tokens(parts.value(1)), 3);
    if (parts.size() > 3) {
      addPadded(columns, tokens(parts.value(2)), 3);
      addPadded(columns, tokens(parts.value(3)), 3);
      columns.append(parts.value(4).trimmed());
    } else {
      addPadded(columns, QStringList(), 6);
      columns.append(parts.value(2).trimmed());
    }
    break;
  }
  default:
    columns.append(row);
    break;
  }
  return columns;
}

}  // namespace

/* The rows of the Session list, split into columns. */
class ListTableModel : public QAbstractTableModel {
 public:
  using QAbstractTableModel::QAbstractTableModel;

  void setList(int kind, const std::vector<std::string> &rows) {
    if (kind == kind_ && rows == source_)
      return;
    beginResetModel();
    kind_ = kind;
    source_ = rows;
    headers_ = headersOf(kind);
    cells_.clear();
    cells_.reserve(static_cast<int>(rows.size()));
    for (const std::string &row : rows)
      cells_.append(columnsOf(kind, QString::fromStdString(row)));
    endResetModel();
  }

  int rowCount(const QModelIndex &parent = QModelIndex()) const override { return parent.isValid() ? 0 : cells_.size(); }
  int columnCount(const QModelIndex &parent = QModelIndex()) const override { return parent.isValid() ? 0 : headers_.size(); }

  QVariant data(const QModelIndex &index, int role) const override {
    if (!index.isValid() || index.row() >= cells_.size())
      return QVariant();
    const QString &cell = cells_[index.row()].value(index.column());
    if (role == Qt::DisplayRole)
      return cell;
    if (role == Qt::UserRole) {
      bool ok = false;
      const double number = cell.toDouble(&ok);
      return ok ? QVariant(number) : QVariant(cell);
    }
    if (role == Qt::UserRole + 1)
      return index.row();
    return QVariant();
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return headers_.value(section);
    return QVariant();
  }

 private:
  int kind_ = -1;
  std::vector<std::string> source_;
  QStringList headers_;
  QVector<QStringList> cells_;
};

/*--------------------------------------------------------------------*/

ProjectBrowser::ProjectBrowser(ProjectController &controller, QWidget *parent)
    : QWidget(parent), controller_(controller), title_(new QLabel), table_(new QTableView), model_(new ListTableModel(this)),
      proxy_(new QSortFilterProxyModel(this)) {
  proxy_->setSourceModel(model_);
  proxy_->setSortRole(Qt::UserRole);
  proxy_->setDynamicSortFilter(false);
  table_->setModel(proxy_);
  table_->setSortingEnabled(true);
  table_->sortByColumn(-1, Qt::AscendingOrder);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setAlternatingRowColors(true);
  table_->verticalHeader()->hide();
  table_->verticalHeader()->setDefaultSectionSize(table_->fontMetrics().height() + 4);
  title_->setWordWrap(true);
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(title_);
  layout->addWidget(table_, 1);
  connect(table_, &QTableView::clicked, this, &ProjectBrowser::rowClicked);
  table_->installEventFilter(this);
  connect(&controller_, &ProjectController::statusChanged, this, &ProjectBrowser::refresh);
  refresh();
}

void ProjectBrowser::refresh() {
  const ListView &list = controller_.session().list();
  const Selection selection = controller_.session().selection();
  title_->setText(QString::fromStdString(list.title));
  model_->setList(selection.list, list.rows);
  table_->resizeColumnsToContents();
  /* the program's row selection may have changed without a click */
  QModelIndex current;
  if (list.selected >= 0 && list.selected < model_->rowCount())
    current = proxy_->mapFromSource(model_->index(list.selected, 0));
  if (current.isValid()) {
    table_->setCurrentIndex(current);
    table_->selectRow(current.row());
    if (list.selected != shownSelection_)
      table_->scrollTo(current);
  } else {
    table_->clearSelection();
  }
  shownSelection_ = list.selected;
}

bool ProjectBrowser::eventFilter(QObject *object, QEvent *event) {
  if (object == table_ && event->type() == QEvent::KeyPress) {
    const int key = static_cast<QKeyEvent *>(event)->key();
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
      rowClicked(table_->currentIndex());
      return true;
    }
  }
  return QWidget::eventFilter(object, event);
}

void ProjectBrowser::rowClicked(const QModelIndex &index) {
  if (controller_.busy() || !index.isValid())
    return;
  const int row = proxy_->mapToSource(index).row();
  if (row == controller_.session().list().selected)
    controller_.session().clickSelectedRow();
  else
    controller_.session().clickListRow(row);
}

/*--------------------------------------------------------------------*/

ProjectStatus::ProjectStatus(ProjectController &controller, QWidget *parent)
    : QWidget(parent), controller_(controller), text_(new QLabel) {
  text_->setFont(fixedFont());
  text_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(text_);
  connect(&controller_, &ProjectController::statusChanged, this, &ProjectStatus::refresh);
  refresh();
}

void ProjectStatus::refresh() {
  QStringList lines;
  for (const std::string &line : controller_.session().statusLines())
    lines.append(QString::fromStdString(line));
  text_->setText(lines.join('\n'));
}

MessageLog::MessageLog(ProjectController &controller, QWidget *parent) : QWidget(parent), text_(new QPlainTextEdit) {
  text_->setReadOnly(true);
  text_->setFont(fixedFont());
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(text_);
  connect(&controller, &ProjectController::messageLogged, this, [this](const QString &message) {
    text_->moveCursor(QTextCursor::End);
    text_->insertPlainText(message);
    text_->moveCursor(QTextCursor::End);
  });
}

/*--------------------------------------------------------------------*/

CrossingEditor::CrossingEditor(ProjectController &controller, QWidget *parent)
    : QWidget(parent), controller_(controller), contour_(new ContourView(controller.plots())), misfit_(new MisfitView(controller.plots())),
      zOffset_(new ZOffsetView(controller.plots())), statusText_(new QLabel), offsetText_(new QLabel), centerZero_(new QRadioButton("Center on Zero Offset")),
      centerCurrent_(new QRadioButton("Center on Current Offset")) {
  Session &session = controller_.session();
  statusText_->setFont(fixedFont());
  offsetText_->setFont(fixedFont());
  statusText_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  offsetText_->setTextInteractionFlags(Qt::TextSelectableByMouse);

  auto *buttons = new QGridLayout;
  const struct {
    const char *name;
    const char *label;
    std::function<void()> action;
  } items[] = {
      {"pushButton_naverr_previous", "Previous", [&session] { session.previousCrossing(); }},
      {"pushButton_naverr_next", "Next", [&session] { session.nextCrossing(); }},
      {"pushButton_naverr_nextunset", "Next Unset", [&session] { session.nextUnsetCrossing(); }},
      {"pushButton_naverr_addtie", "Add Tie", [&session] { session.addTie(); }},
      {"pushButton_naverr_deletetie", "Delete Tie", [&session] { session.deleteTie(); }},
      {"pushButton_naverr_selecttie", "Select Tie", [&session] { session.selectTie(); }},
      {"pushButton_naverr_settie", "Set Offset", [&session] { session.setTie(); }},
      {"pushButton_naverr_resettie", "Reset Offset", [&session] { session.resetTie(); }},
      {"pushButton_naverr_zerooffset", "Zero Offset", [&session] { session.zeroOffset(); }},
      {"pushButton_naverr_zerozoffset", "Zero Z Offset", [&session] { session.zeroZOffset(); }},
      {"pushButton_naverr_minimum", "Minimum Misfit", [&session] { session.moveToMinimumMisfit(); }},
      {"pushButton_naverr_minxy", "Minimum XY Misfit", [&session] { session.moveToMinimumXYMisfit(); }},
      {"pushButton_naverr_unset", "Unset Crossing", [&session] { session.unsetCrossing(); }},
      {"pushButton_naverr_setnone", "Skip Crossing", [&session] { session.skipCrossing(); }},
      {"pushButton_naverr_fullsize", "Full Size", [&session] { session.fullSize(); }},
      {"pushButton_naverr_dismiss", "Dismiss", [&session] { session.dismissCrossingWindow(); }},
  };
  int index = 0;
  for (const auto &item : items) {
    QPushButton *button = addButton(item.name, item.label);
    buttons->addWidget(button, index / 2, index % 2);
    connect(button, &QPushButton::clicked, this, item.action);
    index++;
  }
  auto *biases = new QPushButton("Biases...");
  auto *contours = new QPushButton("Contours...");
  buttons->addWidget(biases, index / 2 + 1, 0);
  buttons->addWidget(contours, index / 2 + 1, 1);
  connect(biases, &QPushButton::clicked, this, &CrossingEditor::biasesRequested);
  connect(contours, &QPushButton::clicked, this, &CrossingEditor::controlsRequested);

  auto *centers = new QButtonGroup(this);
  centers->addButton(centerZero_);
  centers->addButton(centerCurrent_);
  (session.misfitCenterIsZero() ? centerZero_ : centerCurrent_)->setChecked(true);
  connect(centerZero_, &QRadioButton::clicked, this, [&session] { session.setMisfitCenter(true); });
  connect(centerCurrent_, &QRadioButton::clicked, this, [&session] { session.setMisfitCenter(false); });

  misfit_->setFixedSize(302, 302);
  zOffset_->setFixedHeight(61);
  auto *side = new QVBoxLayout;
  side->addWidget(misfit_, 0, Qt::AlignHCenter);
  side->addWidget(zOffset_);
  side->addWidget(centerZero_);
  side->addWidget(centerCurrent_);
  side->addWidget(statusText_);
  side->addWidget(offsetText_);
  side->addLayout(buttons);
  auto *layout = new QHBoxLayout(this);
  layout->addWidget(contour_, 1);
  layout->addLayout(side);

  connect(&controller_, &ProjectController::crossingChanged, this, &CrossingEditor::refresh);
  connect(&controller_, &ProjectController::statusChanged, this, &CrossingEditor::refresh);
  connect(&controller_, &ProjectController::busyChanged, this, &CrossingEditor::refresh);
  refresh();
}

QPushButton *CrossingEditor::addButton(const char *name, const QString &label) {
  auto *button = new QPushButton(label);
  buttons_.insert(name, button);
  return button;
}

void CrossingEditor::refresh() {
  Session &session = controller_.session();
  const bool loaded = session.crossingLoaded() && !controller_.busy();

  QStringList lines;
  for (const std::string &line : session.crossingStatusLines())
    lines.append(QString::fromStdString(line));
  statusText_->setText(loaded ? lines.join('\n') : QString());
  lines.clear();
  for (const std::string &line : session.crossingOffsetLines())
    lines.append(QString::fromStdString(line));
  offsetText_->setText(loaded ? lines.join('\n') : QString());

  static const char *const fromState[] = {"pushButton_naverr_previous", "pushButton_naverr_next", "pushButton_naverr_nextunset",
                                          "pushButton_naverr_addtie", "pushButton_naverr_deletetie", "pushButton_naverr_selecttie",
                                          "pushButton_naverr_settie", "pushButton_naverr_resettie", "pushButton_naverr_fullsize"};
  for (auto it = buttons_.begin(); it != buttons_.end(); ++it)
    it.value()->setEnabled(loaded);
  for (const char *name : fromState)
    buttons_.value(name)->setEnabled(loaded && session.isEnabled(name));
  misfit_->setEnabled(loaded);
  zOffset_->setEnabled(loaded);
  contour_->setEnabled(loaded);
  centerZero_->setEnabled(loaded);
  centerCurrent_->setEnabled(loaded);
}

/*--------------------------------------------------------------------*/

ModelPlot::ModelPlot(ProjectController &controller, QWidget *parent)
    : QWidget(parent), controller_(controller), view_(new ModelPlotView(controller.plots())), statusText_(new QLabel),
      timeSeries_(new QRadioButton("Time Series")), perturbation_(new QRadioButton("Perturbation")), tieOffsets_(new QRadioButton("Tie Offsets")),
      clearBlock_(new QPushButton("Clear Block")) {
  Session &session = controller_.session();
  auto *fullSize = new QPushButton("Full Size");
  auto *dismiss = new QPushButton("Dismiss");
  auto *group = new QButtonGroup(this);
  group->addButton(timeSeries_);
  group->addButton(perturbation_);
  group->addButton(tieOffsets_);
  timeSeries_->setChecked(true);
  connect(timeSeries_, &QRadioButton::clicked, this, [&session] { session.setModelPlotStyle(ModelPlotStyle::TimeSeries); });
  connect(perturbation_, &QRadioButton::clicked, this, [&session] { session.setModelPlotStyle(ModelPlotStyle::Perturbation); });
  connect(tieOffsets_, &QRadioButton::clicked, this, [&session] { session.setModelPlotStyle(ModelPlotStyle::TieOffsets); });
  connect(fullSize, &QPushButton::clicked, this, [&session] { session.modelPlotFullSize(); });
  connect(clearBlock_, &QPushButton::clicked, this, [&session] { session.modelPlotClearBlock(); });
  connect(dismiss, &QPushButton::clicked, this, [this, &session] {
    session.closeModelPlot();
    emit dismissed();
  });

  auto *top = new QHBoxLayout;
  top->addWidget(timeSeries_);
  top->addWidget(perturbation_);
  top->addWidget(tieOffsets_);
  top->addStretch(1);
  top->addWidget(fullSize);
  top->addWidget(clearBlock_);
  top->addWidget(dismiss);
  auto *layout = new QVBoxLayout(this);
  layout->addLayout(top);
  layout->addWidget(statusText_);
  layout->addWidget(view_, 1);
  connect(&controller_.plots(), &PlotSet::changed, this, &ModelPlot::refresh);
  connect(&controller_, &ProjectController::statusChanged, this, &ModelPlot::refresh);
  refresh();
}

void ModelPlot::refresh() {
  QStringList lines;
  for (const std::string &line : controller_.session().modelPlotLines())
    lines.append(QString::fromStdString(line));
  statusText_->setText(lines.join('\n'));
  clearBlock_->setEnabled(controller_.session().isEnabled("pushButton_modelplot_clearblock"));
}

/*--------------------------------------------------------------------*/

static QDoubleSpinBox *numberBox(double minimum, double maximum, int decimals, const QString &suffix) {
  auto *box = new QDoubleSpinBox;
  box->setRange(minimum, maximum);
  box->setDecimals(decimals);
  box->setSingleStep(std::pow(10.0, -decimals));
  box->setSuffix(suffix);
  return box;
}

ControlsPanel::ControlsPanel(ProjectController &controller, QWidget *parent)
    : QWidget(parent), controller_(controller), decimation_(new QSpinBox), sectionLength_(numberBox(0.01, 50.0, 2, " km")), sectionSoundings_(new QSpinBox),
      contourInterval_(numberBox(0.01, 400.0, 2, " m")), colorInterval_(numberBox(0.01, 400.0, 2, " m")), tickInterval_(numberBox(0.01, 400.0, 2, " m")),
      smoothing_(numberBox(0.01, 50.0, 2, "")), zOffsetWidth_(numberBox(0.1, 5.0, 1, " m")), usePrimary_(new QRadioButton("Primary")),
      useSecondary_(new QRadioButton("Secondary")), useTertiary_(new QRadioButton("Tertiary")) {
  decimation_->setRange(1, 10);
  sectionSoundings_->setRange(1, 10000000);

  auto *form = new QFormLayout;
  form->addRow("Max Section Length:", sectionLength_);
  form->addRow("Max # Soundings in Section:", sectionSoundings_);
  form->addRow("Decimation:", decimation_);
  form->addRow("Contour Interval:", contourInterval_);
  form->addRow("Color Interval:", colorInterval_);
  form->addRow("Tick Interval:", tickInterval_);
  form->addRow("Inversion Smoothing:", smoothing_);
  form->addRow("Z Offset Width:", zOffsetWidth_);
  auto *use = new QHBoxLayout;
  use->addWidget(usePrimary_);
  use->addWidget(useSecondary_);
  use->addWidget(useTertiary_);
  form->addRow("Use Mode:", use);
  auto *apply = new QPushButton("Apply");
  auto *layout = new QVBoxLayout(this);
  layout->addLayout(form);
  layout->addWidget(apply);
  layout->addStretch(1);
  connect(apply, &QPushButton::clicked, this, &ControlsPanel::apply);
  load();
}

void ControlsPanel::load() {
  const ControlValues v = controller_.session().controlValues();
  decimation_->setValue(v.decimation);
  sectionLength_->setValue(v.sectionLength);
  sectionSoundings_->setValue(v.sectionSoundings);
  contourInterval_->setValue(v.contourInterval);
  colorInterval_->setValue(v.colorInterval);
  tickInterval_->setValue(v.tickInterval);
  smoothing_->setValue(v.smoothing);
  zOffsetWidth_->setValue(v.zOffsetWidth);
  (v.useMode == 1 ? useSecondary_ : (v.useMode == 2 ? useTertiary_ : usePrimary_))->setChecked(true);
}

void ControlsPanel::apply() {
  ControlValues v;
  v.decimation = decimation_->value();
  v.sectionLength = sectionLength_->value();
  v.sectionSoundings = sectionSoundings_->value();
  v.contourInterval = contourInterval_->value();
  v.colorInterval = colorInterval_->value();
  v.tickInterval = tickInterval_->value();
  v.smoothing = smoothing_->value();
  v.zOffsetWidth = zOffsetWidth_->value();
  v.useMode = useSecondary_->isChecked() ? 1 : (useTertiary_->isChecked() ? 2 : 0);
  controller_.session().applyControls(v);
}

BiasesPanel::BiasesPanel(ProjectController &controller, QWidget *parent)
    : QWidget(parent), controller_(controller), sections_(new QLabel), together_(new QRadioButton("Apply Biases Together")), separate_(new QRadioButton("Apply Biases Separately")),
      heading1_(numberBox(-9.9, 9.9, 1, " deg")), roll1_(numberBox(-9.9, 9.9, 1, " deg")), heading2_(numberBox(-9.9, 9.9, 1, " deg")), roll2_(numberBox(-9.9, 9.9, 1, " deg")),
      apply_(new QPushButton("Apply")), applyAll_(new QPushButton("Apply to All Files")) {
  auto *group = new QButtonGroup(this);
  group->addButton(together_);
  group->addButton(separate_);
  auto *form = new QFormLayout;
  form->addRow("Section 1 Heading:", heading1_);
  form->addRow("Section 1 Roll:", roll1_);
  form->addRow("Section 2 Heading:", heading2_);
  form->addRow("Section 2 Roll:", roll2_);
  auto *layout = new QVBoxLayout(this);
  layout->addWidget(sections_);
  layout->addWidget(together_);
  layout->addWidget(separate_);
  layout->addLayout(form);
  layout->addWidget(apply_);
  layout->addWidget(applyAll_);
  layout->addStretch(1);
  connect(together_, &QRadioButton::toggled, this, &BiasesPanel::togglesChanged);
  connect(separate_, &QRadioButton::toggled, this, &BiasesPanel::togglesChanged);
  connect(heading1_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
    if (together_->isChecked())
      heading2_->setValue(value);
  });
  connect(roll1_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
    if (together_->isChecked())
      roll2_->setValue(value);
  });
  connect(apply_, &QPushButton::clicked, this, [this] {
    controller_.session().applyBiases(heading1_->value(), roll1_->value(), heading2_->value(), roll2_->value());
  });
  connect(applyAll_, &QPushButton::clicked, this, [this] {
    controller_.session().applyBiasesToAllFiles(heading1_->value(), roll1_->value());
  });
  load();
}

void BiasesPanel::load() {
  const BiasValues v = controller_.session().biasValues();
  sections_->setText(v.available ? QString("Sections (file:section): %1:%2 and %3:%4").arg(v.file1).arg(v.section1).arg(v.file2).arg(v.section2)
                                 : QString("No sections are loaded."));
  (v.heading1 == v.heading2 && v.roll1 == v.roll2 ? together_ : separate_)->setChecked(true);
  heading1_->setValue(v.heading1);
  roll1_->setValue(v.roll1);
  heading2_->setValue(v.heading2);
  roll2_->setValue(v.roll2);
  togglesChanged();
  apply_->setEnabled(v.available);
  applyAll_->setEnabled(v.available && together_->isChecked());
}

void BiasesPanel::togglesChanged() {
  const bool separate = separate_->isChecked();
  heading2_->setEnabled(separate);
  roll2_->setEnabled(separate);
  applyAll_->setEnabled(!separate);
  if (!separate) {
    heading2_->setValue(heading1_->value());
    roll2_->setValue(roll1_->value());
  }
}

}  // namespace mbnaq
