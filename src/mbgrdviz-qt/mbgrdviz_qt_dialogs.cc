/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_dialogs.cc  11/7/2026
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

#include "mbgrdviz_qt_dialogs.h"

#include <QButtonGroup>
#include <QCloseEvent>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

#include "mbgrdviz_qt_bridge.h"
#include "mbgrdviz_qt_controller.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
#include "mbgrdviz_core.h"
}

namespace {

SurveyDialog *survey_dialog = nullptr;
RealtimeDialog *realtime_dialog = nullptr;

/* the spin box steps of the Motif dialog: larger values, larger steps */
int increment(int value) {
  if (value < 25)
    return 1;
  if (value < 100)
    return 5;
  if (value < 250)
    return 10;
  if (value < 1000)
    return 25;
  if (value < 2000)
    return 50;
  return 100;
}

QComboBox *combo(const QStringList &items) {
  auto *box = new QComboBox;
  box->addItems(items);
  return box;
}

QSpinBox *spin(int minimum, int maximum, int step) {
  auto *box = new QSpinBox;
  box->setRange(minimum, maximum);
  box->setSingleStep(step);
  return box;
}

}  // namespace

/*------------------------------------------------------------------------------*/
SurveyDialog::SurveyDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("Generate Survey Lines from Area"));
  auto *layout = new QVBoxLayout(this);

  layout->addWidget(new QLabel(tr("Generate Survey Route for Selected Area...")));
  info_ = new QLabel(tr("Current Area Info:"));
  info_->setFrameShape(QFrame::StyledPanel);
  info_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  layout->addWidget(info_);

  /* the fields, labels and ranges of the Motif dialog */
  auto *form = new QFormLayout;
  name_ = new QLineEdit;
  form->addRow(tr("Route Name:"), name_);
  color_ = combo({tr("Black"), tr("Yellow"), tr("Green"), tr("Bluegreen"), tr("Blue"), tr("Purple")});
  form->addRow(tr("Route Color:"), color_);
  mode_ = combo({tr("Uniform"), tr("Variable by Swath Width")});
  form->addRow(tr("Line Spacing Mode:"), mode_);
  linespacing_ = spin(1, 10000, 10);
  form->addRow(tr("Line Spacing (m):"), linespacing_);
  platform_ = combo({tr("Surface Vessel"), tr("Submerged - constant altitude"), tr("Submerged - constant depth")});
  form->addRow(tr("Platform Type:"), platform_);
  swathwidth_ = spin(5, 175, 5);
  form->addRow(tr("Swath Width (degrees):"), swathwidth_);
  altitude_ = spin(1, 10000, 10);
  form->addRow(tr("Sonar Altitude (m):"), altitude_);
  depth_ = spin(1, 10000, 1);
  form->addRow(tr("Sonar Depth (m):"), depth_);
  direction_ = combo({tr("SW"), tr("SE"), tr("NW"), tr("NE")});
  form->addRow(tr("Start Corner:"), direction_);
  crosslines_ = spin(0, 50, 1);
  form->addRow(tr("Cross Lines:"), crosslines_);
  crosslines_last_ = combo({tr("Crosslines first"), tr("Crosslines last")});
  form->addRow(tr("Crosslines Before/After Survey:"), crosslines_last_);
  interleaving_ = spin(1, 10, 1);
  form->addRow(tr("Interleaving:"), interleaving_);
  layout->addLayout(form);

  auto *buttons = new QHBoxLayout;
  auto *generate = new QPushButton(tr("Generate Route"));
  auto *dismiss_button = new QPushButton(tr("Dismiss"));
  buttons->addWidget(generate);
  buttons->addStretch();
  buttons->addWidget(dismiss_button);
  layout->addLayout(buttons);

  for (QComboBox *box : {color_, mode_, platform_, direction_, crosslines_last_})
    connect(box, &QComboBox::currentIndexChanged, this, [this] { changed(); });
  for (QSpinBox *box : {crosslines_, interleaving_, linespacing_, swathwidth_, altitude_, depth_})
    connect(box, &QSpinBox::valueChanged, this, [this] { changed(); });
  for (QSpinBox *box : {linespacing_, altitude_, depth_})
    connect(box, &QSpinBox::valueChanged, box, [box](int value) { box->setSingleStep(increment(value)); });
  connect(name_, &QLineEdit::textEdited, this, [this] { changed(); });
  connect(generate, &QPushButton::clicked, this, [] { mbgrdviz_qt_survey_generate(); });
  connect(dismiss_button, &QPushButton::clicked, this, [this] { dismiss(); });
}

SurveyDialog *SurveyDialog::instance(QWidget *parent) {
  if (survey_dialog == nullptr)
    survey_dialog = new SurveyDialog(parent);
  return survey_dialog;
}

void SurveyDialog::load() {
  struct mbgrdviz_qt_survey survey;
  mbgrdviz_qt_survey_get(&survey);
  loading_ = true;
  name_->setText(QString::fromLatin1(survey.name));
  color_->setCurrentIndex(survey.color);
  mode_->setCurrentIndex(survey.mode);
  platform_->setCurrentIndex(survey.platform);
  direction_->setCurrentIndex(survey.direction);
  crosslines_->setValue(survey.crosslines);
  crosslines_last_->setCurrentIndex(survey.crosslines_last ? 1 : 0);
  interleaving_->setValue(survey.interleaving);
  linespacing_->setValue(survey.linespacing);
  swathwidth_->setValue(survey.swathwidth);
  altitude_->setValue(survey.altitude);
  depth_->setValue(survey.depth);
  for (QSpinBox *box : {linespacing_, altitude_, depth_})
    box->setSingleStep(increment(box->value()));
  loading_ = false;
}

void SurveyDialog::changed() {
  if (loading_)
    return;
  struct mbgrdviz_qt_survey survey;
  survey.mode = mode_->currentIndex();
  survey.platform = platform_->currentIndex();
  survey.interleaving = interleaving_->value();
  survey.direction = direction_->currentIndex();
  survey.crosslines_last = crosslines_last_->currentIndex() == 1;
  survey.crosslines = crosslines_->value();
  survey.linespacing = linespacing_->value();
  survey.swathwidth = swathwidth_->value();
  survey.depth = depth_->value();
  survey.altitude = altitude_->value();
  survey.color = color_->currentIndex();
  const QByteArray name = name_->text().toLatin1();
  qstrncpy(survey.name, name.constData(), sizeof(survey.name));
  mbgrdviz_qt_survey_set(&survey);
}

void SurveyDialog::set_sensitivity(bool linespacing, bool platform, bool swathwidth, bool altitude, bool depth) {
  linespacing_->setEnabled(linespacing);
  platform_->setEnabled(platform);
  swathwidth_->setEnabled(swathwidth);
  altitude_->setEnabled(altitude);
  depth_->setEnabled(depth);
}

void SurveyDialog::set_info(const char *info_text) { info_->setText(mbvq::label_text(info_text)); }

void SurveyDialog::set_name(const char *name) {
  const bool was = loading_;
  loading_ = true;
  name_->setText(QString::fromLatin1(name));
  loading_ = was;
}

void SurveyDialog::dismiss() {
  /* the last generated route stays; a new one starts next time */
  mbgrdviz_qt_survey_dismiss();
  hide();
}

void SurveyDialog::closeEvent(QCloseEvent *event) {
  mbgrdviz_qt_survey_dismiss();
  QDialog::closeEvent(event);
}

/*------------------------------------------------------------------------------*/
RealtimeDialog::RealtimeDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("Realtime Setup"));
  auto *layout = new QVBoxLayout(this);

  auto *path_row = new QHBoxLayout;
  path_ = new QLineEdit;
  auto *browse = new QPushButton(tr("Browse"));
  auto *test = new QPushButton(tr("Test"));
  path_row->addWidget(path_, 1);
  path_row->addWidget(browse);
  path_row->addWidget(test);
  layout->addWidget(new QLabel(tr("Realtime Navigation Source:")));
  layout->addLayout(path_row);

  auto *recent = new QRadioButton(tr("Most Recent File in Directory"));
  auto *pointer = new QRadioButton(tr("Use Pointer File"));
  recent->setChecked(true);
  auto *mode = new QButtonGroup(this);
  mode->addButton(recent);
  mode->addButton(pointer);
  layout->addWidget(recent);
  layout->addWidget(pointer);

  auto *form = new QFormLayout;
  form->addRow(tr("Realtime Display Icon:"), combo({tr("Ship"), tr("ROV"), tr("AUV")}));
  auto *rate = new QSlider(Qt::Horizontal);
  rate->setRange(1, 25);
  rate->setValue(1);
  auto *rate_value = new QLabel(QStringLiteral("1"));
  connect(rate, &QSlider::valueChanged, rate_value, [rate_value](int value) { rate_value->setNum(value); });
  auto *rate_row = new QHBoxLayout;
  rate_row->addWidget(rate, 1);
  rate_row->addWidget(rate_value);
  form->addRow(tr("Display Update Rate:"), rate_row);
  layout->addLayout(form);
  layout->addWidget(new QLabel(tr("Data read before update")));

  auto *note = new QLabel(tr("The realtime display is not implemented in mbgrdviz yet; these settings have no effect."));
  note->setWordWrap(true);
  layout->addWidget(note);

  auto *dismiss = new QPushButton(tr("Dismiss"));
  auto *buttons = new QHBoxLayout;
  buttons->addStretch();
  buttons->addWidget(dismiss);
  layout->addLayout(buttons);

  /* Browse: the Motif "Set Realtime Navigation Source" file dialog */
  connect(browse, &QPushButton::clicked, this, [this] {
    if (mbgvq::Controller *controller = mbgvq::Controller::current())
      controller->fileDialog(MBGRDVIZ_REALTIME, 0, tr("Set Realtime Navigation Source"), QStringLiteral("*"));
  });
  /* as in Motif, Test does nothing yet */
  test->setEnabled(false);
  connect(dismiss, &QPushButton::clicked, this, &QDialog::hide);
}

RealtimeDialog *RealtimeDialog::instance(QWidget *parent) {
  if (realtime_dialog == nullptr)
    realtime_dialog = new RealtimeDialog(parent);
  return realtime_dialog;
}

void RealtimeDialog::set_path(const QString &path) { path_->setText(path); }

/*------------------------------------------------------------------------------*/
extern "C" {

void mbgrdviz_qt_survey_dialog(void) {
  mbgvq::Controller *controller = mbgvq::Controller::current();
  SurveyDialog *dialog = SurveyDialog::instance(controller != nullptr ? controller->dialogParent() : nullptr);
  dialog->load();
  dialog->show();
  dialog->raise();
}

void mbgrdviz_qt_survey_sensitivity(bool linespacing, bool platform, bool swathwidth, bool altitude, bool depth) {
  if (survey_dialog != nullptr)
    survey_dialog->set_sensitivity(linespacing, platform, swathwidth, altitude, depth);
}

void mbgrdviz_qt_survey_info(const char *info_text) {
  if (survey_dialog != nullptr)
    survey_dialog->set_info(info_text);
}

void mbgrdviz_qt_survey_name(const char *name) {
  if (survey_dialog != nullptr)
    survey_dialog->set_name(name);
}

}  // extern "C"
