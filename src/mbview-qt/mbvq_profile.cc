/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_profile.cc  11/7/2026
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

#include "mbvq_profile.h"

#include <QCloseEvent>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

#include "mbvq.h"
#include "mbvq_backend.h"
#include "mbvq_bridge.h"

namespace mbvq {

namespace {

/* the Motif scales: label, minimum, maximum, decimal places */
struct Control {
  const char *label;
  int min, max, decimals;
};
constexpr Control kControls[3] = {
    {"Display Width Multiple", 1, 5, 0}, {"Slope Threshold", 1, 200, 2}, {"Vertical exaggeration", 0, 100, 1}};

QString scaled(int value, int decimals) {
  double v = value;
  for (int i = 0; i < decimals; i++)
    v /= 10.0;
  return QString::number(v, 'f', decimals);
}

}  // namespace

ProfileWindow::ProfileWindow(size_t instance, QWidget *parent) : QWidget(parent, Qt::Window), instance_(instance) {
  setWindowTitle(tr("MBview Profile Display"));
  auto *layout = new QVBoxLayout(this);
  scroll_ = new QScrollArea;
  scroll_->setWidgetResizable(false);
  scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view_ = backend() == Backend::LegacyGL ? gl_backend::create_profile_view(instance, nullptr)
                                         : null_backend::create_profile_view(instance, nullptr);
  scroll_->setWidget(view_);
  scroll_->viewport()->installEventFilter(this);
  layout->addWidget(scroll_, 1);
  info_ = new QLabel;
  info_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  layout->addWidget(info_);

  auto *form = new QFormLayout;
  for (int c = 0; c < 3; c++) {
    auto *slider = new QSlider(Qt::Horizontal);
    slider->setRange(kControls[c].min, kControls[c].max);
    /* as the Motif scales: the profile follows while dragging */
    slider->setTracking(true);
    auto *value = new QLabel;
    value->setMinimumWidth(value->fontMetrics().horizontalAdvance(QStringLiteral("00.00")));
    connect(slider, &QSlider::valueChanged, this, [this, c](int v) { control_changed(c, v); });
    auto *row = new QHBoxLayout;
    row->addWidget(slider, 1);
    row->addWidget(value);
    form->addRow(tr(kControls[c].label), row);
    sliders_[c] = slider;
    values_[c] = value;
  }
  layout->addLayout(form);
  auto *buttons = new QHBoxLayout;
  buttons->addStretch(1);
  auto *dismiss = new QPushButton(tr("Dismiss"));
  connect(dismiss, &QPushButton::clicked, this, [this] { this->dismiss(); });
  buttons->addWidget(dismiss);
  layout->addLayout(buttons);
  resize(700, 360);
}

void ProfileWindow::toggle() {
  if (mbvq_c_profile_shown(instance_))
    dismiss();
  else
    open();
}

void ProfileWindow::open() {
  int values[3];
  mbvq_c_profile_controls(instance_, &values[0], &values[1], &values[2]);
  for (int c = 0; c < 3; c++) {
    const QSignalBlocker block(sliders_[c]);
    sliders_[c]->setValue(values[c]);
    values_[c]->setText(scaled(values[c], kControls[c].decimals));
  }
  show();
  raise();
  layout_profile();
  mbvq_c_profile_show(instance_, true);
}

void ProfileWindow::dismiss() {
  hide();
  mbvq_c_profile_show(instance_, false);
}

void ProfileWindow::closeEvent(QCloseEvent *event) {
  /* as the Dismiss button */
  event->accept();
  mbvq_c_profile_show(instance_, false);
}

void ProfileWindow::set_info(const QString &text) { info_->setText(text); }

void ProfileWindow::layout_profile() {
  int width = 1, slope = 0, exaggeration = 0;
  mbvq_c_profile_controls(instance_, &width, &slope, &exaggeration);
  const QSize visible = scroll_->viewport()->size();
  view_->resize(width * visible.width(), visible.height());
  if (backend() == Backend::Null) {
    const double ratio = view_->devicePixelRatioF();
    mbvq_c_profile_resize(instance_, static_cast<int>(view_->width() * ratio), static_cast<int>(view_->height() * ratio));
  }
}

bool ProfileWindow::eventFilter(QObject *watched, QEvent *event) {
  if (watched == scroll_->viewport() && event->type() == QEvent::Resize)
    layout_profile();
  return QWidget::eventFilter(watched, event);
}

void ProfileWindow::control_changed(int control, int value) {
  values_[control]->setText(scaled(value, kControls[control].decimals));
  mbvq_c_profile_control(instance_, control, value);
  /* as the Motif callbacks: size the drawing again and draw the profile */
  layout_profile();
  mbvq_c_plotprofile(instance_);
}

}  // namespace mbvq
