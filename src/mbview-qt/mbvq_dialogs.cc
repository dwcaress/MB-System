/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_dialogs.cc  11/7/2026
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

#include "mbvq_dialogs.h"

#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QVBoxLayout>
#include <iterator>
#include <vector>

#include "mbv_ui.h"

namespace mbvq {

namespace {

/* the fields each dialog shows */
constexpr int kColorboundsFields[] = {MBVQ_F_DATAMIN,   MBVQ_F_DATAMAX,    MBVQ_F_CONTOURS,  MBVQ_F_SLOPEMIN,
                                      MBVQ_F_SLOPEMAX,  MBVQ_F_OVERLAYMIN, MBVQ_F_OVERLAYMAX};
constexpr int kShadeFields[] = {MBVQ_F_ILLUM_AMP, MBVQ_F_ILLUM_AZI,   MBVQ_F_ILLUM_ELEV,
                                MBVQ_F_SLOPE_AMP, MBVQ_F_OVERLAY_AMP, MBVQ_F_OVERLAY_CENTER};
constexpr int kView3dFields[] = {MBVQ_F_MODEL_AZI,   MBVQ_F_MODEL_ELEV, MBVQ_F_VIEW_AZI,
                                 MBVQ_F_VIEW_ELEV,   MBVQ_F_EXAGGERATION, MBVQ_F_OFFSET3DX,
                                 MBVQ_F_OFFSET3DY,   MBVQ_F_MODEL_3DZOOM, MBVQ_F_VIEW_3DZOOM};
constexpr int kView2dFields[] = {MBVQ_F_OFFSET2DX, MBVQ_F_OFFSET2DY, MBVQ_F_ZOOM2D};

/* fields shown again when values change while dragging (MBV_VALUES_*) */
struct ValuesFields {
  int values;
  int fields[2];
  int count;
};
constexpr ValuesFields kValuesFields[] = {
    {MBV_VALUES_VIEW_2DOFFSET, {MBVQ_F_OFFSET2DX, MBVQ_F_OFFSET2DY}, 2},
    {MBV_VALUES_VIEW_3DOFFSET, {MBVQ_F_OFFSET3DX, MBVQ_F_OFFSET3DY}, 2},
    {MBV_VALUES_MODEL_ORIENTATION, {MBVQ_F_MODEL_AZI, MBVQ_F_MODEL_ELEV}, 2},
    {MBV_VALUES_ILLUMINATION, {MBVQ_F_ILLUM_AZI, MBVQ_F_ILLUM_ELEV}, 2},
    {MBV_VALUES_VIEW_ORIENTATION, {MBVQ_F_VIEW_AZI, MBVQ_F_VIEW_ELEV}, 2},
    {MBV_VALUES_VIEW_2DZOOM, {MBVQ_F_ZOOM2D, 0}, 1},
    {MBV_VALUES_MODEL_3DZOOM, {MBVQ_F_MODEL_3DZOOM, 0}, 1},
    {MBV_VALUES_EXAGGERATION, {MBVQ_F_EXAGGERATION, 0}, 1},
    {MBV_VALUES_ILLUMINATION_AMP, {MBVQ_F_ILLUM_AMP, 0}, 1},
    {MBV_VALUES_SLOPE_AMP, {MBVQ_F_SLOPE_AMP, 0}, 1},
    {MBV_VALUES_OVERLAY_AMP, {MBVQ_F_OVERLAY_AMP, 0}, 1},
    {MBV_VALUES_VIEW_3DZOOM, {MBVQ_F_VIEW_3DZOOM, 0}, 1},
};

/* the dialog a field belongs to */
int dialog_of(int field) {
  for (int f : kColorboundsFields)
    if (f == field)
      return MBVQ_DIALOG_COLORBOUNDS;
  for (int f : kShadeFields)
    if (f == field)
      return MBVQ_DIALOG_SHADE;
  for (int f : kView3dFields)
    if (f == field)
      return MBVQ_DIALOG_VIEW3D;
  return MBVQ_DIALOG_VIEW2D;
}

QString format(double value) { return QString::asprintf("%g", value); }

}  // namespace

ControlDialogs::ControlDialogs(size_t instance, QWidget *window) : QObject(window), instance_(instance), window_(window) {
  build_colorbounds();
  build_view2d();
  build_view3d();
  build_shade();
  build_resolution();
  build_projection();
}

QDialog *ControlDialogs::make_dialog(int dialog, const QString &title) {
  auto *d = new QDialog(window_);
  d->setWindowTitle(title);
  d->setModal(false);
  dialogs_[dialog] = d;
  return d;
}

QLineEdit *ControlDialogs::make_field(int field) {
  fields_[field] = new QLineEdit;
  return fields_[field];
}

void ControlDialogs::make_mode_buttons(int toggle, QWidget *owner) {
  ctoh_[toggle] = new QRadioButton(tr("Cold-to-Hot"));
  htoc_[toggle] = new QRadioButton(tr("Hot-to-Cold"));
  auto *group = new QButtonGroup(owner);
  group->addButton(ctoh_[toggle]);
  group->addButton(htoc_[toggle]);
}

/* the Apply and Dismiss buttons of the Motif dialogs */
static QDialogButtonBox *apply_dismiss(QDialog *d, bool with_apply) {
  auto *box = new QDialogButtonBox;
  if (with_apply)
    box->addButton(QObject::tr("Apply"), QDialogButtonBox::ApplyRole);
  QPushButton *dismiss = box->addButton(QObject::tr("Dismiss"), QDialogButtonBox::RejectRole);
  QObject::connect(dismiss, &QPushButton::clicked, d, &QDialog::hide);
  return box;
}

static QGroupBox *group(const QString &title, QFormLayout **form) {
  auto *box = new QGroupBox(title);
  *form = new QFormLayout(box);
  return box;
}

void ControlDialogs::build_colorbounds() {
  QDialog *d = make_dialog(MBVQ_DIALOG_COLORBOUNDS, tr("Colors and Contours"));
  auto *layout = new QVBoxLayout(d);
  QFormLayout *form = nullptr;
  struct Bounds {
    const char *title;
    int min, max, toggle;
  };
  const Bounds bounds[] = {{"Data Color Bounds:", MBVQ_F_DATAMIN, MBVQ_F_DATAMAX, MBVQ_T_DATA_HTOC},
                           {"Data Slope Color Bounds:", MBVQ_F_SLOPEMIN, MBVQ_F_SLOPEMAX, MBVQ_T_SLOPE_HTOC},
                           {"Overlay Color Bounds:", MBVQ_F_OVERLAYMIN, MBVQ_F_OVERLAYMAX, MBVQ_T_OVERLAY_HTOC}};
  for (const Bounds &b : bounds) {
    layout->addWidget(group(tr(b.title), &form));
    form->addRow(tr("Minimum:"), make_field(b.min));
    form->addRow(tr("Maximum:"), make_field(b.max));
    make_mode_buttons(b.toggle, d);
    auto *modes = new QHBoxLayout;
    modes->addWidget(ctoh_[b.toggle]);
    modes->addWidget(htoc_[b.toggle]);
    form->addRow(modes);
    if (b.min == MBVQ_F_DATAMIN)
      form->addRow(tr("Data Contour Interval (meters):"), make_field(MBVQ_F_CONTOURS));
  }
  QDialogButtonBox *box = apply_dismiss(d, true);
  connect(box, &QDialogButtonBox::clicked, this, [this, box](QAbstractButton *b) {
    if (box->buttonRole(b) == QDialogButtonBox::ApplyRole)
      apply(MBVQ_DIALOG_COLORBOUNDS);
  });
  layout->addWidget(box);
}

void ControlDialogs::build_shade() {
  QDialog *d = make_dialog(MBVQ_DIALOG_SHADE, tr("Shading"));
  auto *layout = new QVBoxLayout(d);
  QFormLayout *form = nullptr;
  layout->addWidget(group(tr("Shading by Illumination:"), &form));
  form->addRow(tr("Amplitude:"), make_field(MBVQ_F_ILLUM_AMP));
  form->addRow(tr("Azimuth (degrees):"), make_field(MBVQ_F_ILLUM_AZI));
  form->addRow(tr("Elevation (degrees):"), make_field(MBVQ_F_ILLUM_ELEV));
  layout->addWidget(group(tr("Shading by Slope:"), &form));
  form->addRow(tr("Amplitude:"), make_field(MBVQ_F_SLOPE_AMP));
  layout->addWidget(group(tr("Shading by Overlay:"), &form));
  form->addRow(tr("Amplitude:"), make_field(MBVQ_F_OVERLAY_AMP));
  form->addRow(tr("Center:"), make_field(MBVQ_F_OVERLAY_CENTER));
  make_mode_buttons(MBVQ_T_OVERLAY_SHADE_HTOC, d);
  auto *modes = new QHBoxLayout;
  modes->addWidget(ctoh_[MBVQ_T_OVERLAY_SHADE_HTOC]);
  modes->addWidget(htoc_[MBVQ_T_OVERLAY_SHADE_HTOC]);
  form->addRow(modes);
  QDialogButtonBox *box = apply_dismiss(d, true);
  connect(box, &QDialogButtonBox::clicked, this, [this, box](QAbstractButton *b) {
    if (box->buttonRole(b) == QDialogButtonBox::ApplyRole)
      apply(MBVQ_DIALOG_SHADE);
  });
  layout->addWidget(box);
}

void ControlDialogs::build_view3d() {
  QDialog *d = make_dialog(MBVQ_DIALOG_VIEW3D, tr("3D Controls"));
  auto *layout = new QVBoxLayout(d);
  QFormLayout *form = nullptr;
  layout->addWidget(group(tr("Model Orientation && Zoom:"), &form));
  form->addRow(tr("Azimuth (degrees):"), make_field(MBVQ_F_MODEL_AZI));
  form->addRow(tr("Elevation (degrees):"), make_field(MBVQ_F_MODEL_ELEV));
  form->addRow(tr("Zoom:"), make_field(MBVQ_F_MODEL_3DZOOM));
  layout->addWidget(group(tr("View Orientation && Zoom:"), &form));
  form->addRow(tr("Azimuth (degrees):"), make_field(MBVQ_F_VIEW_AZI));
  form->addRow(tr("Elevation (degrees):"), make_field(MBVQ_F_VIEW_ELEV));
  form->addRow(tr("Zoom:"), make_field(MBVQ_F_VIEW_3DZOOM));
  layout->addWidget(group(tr("Pan:"), &form));
  form->addRow(tr("X:"), make_field(MBVQ_F_OFFSET3DX));
  form->addRow(tr("Y:"), make_field(MBVQ_F_OFFSET3DY));
  auto *exaggeration = new QFormLayout;
  exaggeration->addRow(tr("Vertical exaggeration:"), make_field(MBVQ_F_EXAGGERATION));
  layout->addLayout(exaggeration);
  QDialogButtonBox *box = apply_dismiss(d, true);
  connect(box, &QDialogButtonBox::clicked, this, [this, box](QAbstractButton *b) {
    if (box->buttonRole(b) == QDialogButtonBox::ApplyRole)
      apply(MBVQ_DIALOG_VIEW3D);
  });
  layout->addWidget(box);
}

void ControlDialogs::build_view2d() {
  QDialog *d = make_dialog(MBVQ_DIALOG_VIEW2D, tr("2D Controls"));
  auto *layout = new QVBoxLayout(d);
  QFormLayout *form = nullptr;
  layout->addWidget(group(tr("View Offset:"), &form));
  form->addRow(tr("X:"), make_field(MBVQ_F_OFFSET2DX));
  form->addRow(tr("Y:"), make_field(MBVQ_F_OFFSET2DY));
  auto *zoom = new QFormLayout;
  zoom->addRow(tr("Zoom:"), make_field(MBVQ_F_ZOOM2D));
  layout->addLayout(zoom);
  QDialogButtonBox *box = apply_dismiss(d, true);
  connect(box, &QDialogButtonBox::clicked, this, [this, box](QAbstractButton *b) {
    if (box->buttonRole(b) == QDialogButtonBox::ApplyRole)
      apply(MBVQ_DIALOG_VIEW2D);
  });
  layout->addWidget(box);
}

void ControlDialogs::build_resolution() {
  QDialog *d = make_dialog(MBVQ_DIALOG_RESOLUTION, tr("Resolution"));
  auto *layout = new QVBoxLayout(d);
  QFormLayout *form = nullptr;
  /* the ranges of the Motif scales */
  struct Scale {
    const char *label;
    int min, max;
  };
  const Scale scales[4] = {{"Low Resolution Dimension", 10, 500},
                           {"Medium Resolution Dimension", 100, 1000},
                           {"Low Resolution Decimation", 1, 25},
                           {"Medium Resolution Decimation", 1, 10}};
  for (int i = 0; i < 4; i++) {
    if (i == 0)
      layout->addWidget(group(tr("Grid Rendering Resolution:"), &form));
    if (i == 2)
      layout->addWidget(group(tr("Nav Rendering Decimation"), &form));
    auto *slider = new QSlider(Qt::Horizontal);
    slider->setRange(scales[i].min, scales[i].max);
    /* as the Motif scales: the value counts when the slider is released */
    slider->setTracking(false);
    auto *value = new QLabel;
    value->setMinimumWidth(value->fontMetrics().horizontalAdvance(QStringLiteral("00000")));
    connect(slider, &QSlider::valueChanged, value, [value](int v) { value->setNum(v); });
    connect(slider, &QSlider::sliderMoved, value, [value](int v) { value->setNum(v); });
    connect(slider, &QSlider::valueChanged, this, [this] { resolution_changed(); });
    auto *row = new QHBoxLayout;
    row->addWidget(slider, 1);
    row->addWidget(value);
    form->addRow(tr(scales[i].label), row);
    resolution_[i] = slider;
    resolution_value_[i] = value;
  }
  layout->addWidget(apply_dismiss(d, false));
}

void ControlDialogs::build_projection() {
  QDialog *d = make_dialog(MBVQ_DIALOG_PROJECTION, tr("Projections"));
  auto *layout = new QVBoxLayout(d);
  projection_label_ = new QLabel;
  projection_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  layout->addWidget(projection_label_);
  auto *display = new QGroupBox(tr("Display Projection:"));
  auto *display_layout = new QHBoxLayout(display);
  const char *names[3] = {"Geographic", "UTM", "Spheroid"};
  auto *display_group = new QButtonGroup(d);
  for (int i = 0; i < 3; i++) {
    projection_[i] = new QRadioButton(tr(names[i]));
    display_group->addButton(projection_[i], i);
    display_layout->addWidget(projection_[i]);
    connect(projection_[i], &QRadioButton::clicked, this, [this, i] { mbvq_c_setprojection(instance_, i); });
  }
  layout->addWidget(display);
  auto *annotation = new QGroupBox(tr("Annotation Style:"));
  auto *annotation_layout = new QHBoxLayout(annotation);
  auto *annotation_group = new QButtonGroup(d);
  lonlat_[1] = new QRadioButton(tr("Degrees + Minutes"));
  lonlat_[0] = new QRadioButton(tr("Decimal Degrees"));
  for (int style : {1, 0}) {
    annotation_group->addButton(lonlat_[style], style);
    annotation_layout->addWidget(lonlat_[style]);
    connect(lonlat_[style], &QRadioButton::clicked, this, [this, style] { mbvq_c_setlonlatstyle(instance_, style); });
  }
  layout->addWidget(annotation);
  layout->addWidget(apply_dismiss(d, false));
}

void ControlDialogs::load_fields(const int *fields, int count) {
  struct mbvq_controls c;
  mbvq_c_controls(instance_, &c);
  for (int i = 0; i < count; i++)
    fields_[fields[i]]->setText(format(c.field[fields[i]]));
}

void ControlDialogs::load(int dialog) {
  struct mbvq_controls c;
  mbvq_c_controls(instance_, &c);
  auto set_mode = [this, &c](int toggle) {
    htoc_[toggle]->setChecked(c.toggle[toggle] != 0);
    ctoh_[toggle]->setChecked(c.toggle[toggle] == 0);
  };
  switch (dialog) {
  case MBVQ_DIALOG_COLORBOUNDS:
    load_fields(kColorboundsFields, static_cast<int>(std::size(kColorboundsFields)));
    set_mode(MBVQ_T_DATA_HTOC);
    set_mode(MBVQ_T_SLOPE_HTOC);
    set_mode(MBVQ_T_OVERLAY_HTOC);
    /* as mbview_update_sensitivity: the grids that are present */
    for (int f : {MBVQ_F_DATAMIN, MBVQ_F_DATAMAX, MBVQ_F_CONTOURS, MBVQ_F_SLOPEMIN, MBVQ_F_SLOPEMAX})
      fields_[f]->setEnabled(c.primary);
    for (int t : {MBVQ_T_DATA_HTOC, MBVQ_T_SLOPE_HTOC}) {
      htoc_[t]->setEnabled(c.primary);
      ctoh_[t]->setEnabled(c.primary);
    }
    for (int f : {MBVQ_F_OVERLAYMIN, MBVQ_F_OVERLAYMAX})
      fields_[f]->setEnabled(c.secondary);
    htoc_[MBVQ_T_OVERLAY_HTOC]->setEnabled(c.secondary);
    ctoh_[MBVQ_T_OVERLAY_HTOC]->setEnabled(c.secondary);
    break;
  case MBVQ_DIALOG_SHADE:
    load_fields(kShadeFields, static_cast<int>(std::size(kShadeFields)));
    set_mode(MBVQ_T_OVERLAY_SHADE_HTOC);
    for (int f : {MBVQ_F_ILLUM_AMP, MBVQ_F_ILLUM_AZI, MBVQ_F_ILLUM_ELEV, MBVQ_F_SLOPE_AMP})
      fields_[f]->setEnabled(c.primary);
    for (int f : {MBVQ_F_OVERLAY_AMP, MBVQ_F_OVERLAY_CENTER})
      fields_[f]->setEnabled(c.secondary);
    htoc_[MBVQ_T_OVERLAY_SHADE_HTOC]->setEnabled(c.secondary);
    ctoh_[MBVQ_T_OVERLAY_SHADE_HTOC]->setEnabled(c.secondary);
    break;
  case MBVQ_DIALOG_VIEW3D:
    load_fields(kView3dFields, static_cast<int>(std::size(kView3dFields)));
    for (int f : kView3dFields)
      fields_[f]->setEnabled(c.primary);
    break;
  case MBVQ_DIALOG_VIEW2D:
    load_fields(kView2dFields, static_cast<int>(std::size(kView2dFields)));
    for (int f : kView2dFields)
      fields_[f]->setEnabled(c.primary);
    break;
  case MBVQ_DIALOG_RESOLUTION: {
    const int values[4] = {c.lorez_dimension, c.hirez_dimension, c.lorez_navdecimate, c.hirez_navdecimate};
    for (int i = 0; i < 4; i++) {
      const QSignalBlocker block(resolution_[i]);
      resolution_[i]->setValue(values[i]);
      resolution_value_[i]->setNum(values[i]);
    }
    break;
  }
  case MBVQ_DIALOG_PROJECTION: {
    for (int i = 0; i < 3; i++)
      projection_[i]->setChecked(c.display_projection == i);
    lonlat_[c.lonlatstyle]->setChecked(true);
    /* the core builds the label text and sends it (set_projection_label) */
    mbvq_c_projection_label(instance_);
    break;
  }
  default:
    break;
  }
}

void ControlDialogs::show_dialog(int dialog) {
  if (dialog < 0 || dialog >= MBVQ_DIALOGS)
    return;
  load(dialog);
  dialogs_[dialog]->show();
  dialogs_[dialog]->raise();
  dialogs_[dialog]->activateWindow();
}

void ControlDialogs::apply(int dialog) {
  std::vector<QByteArray> texts(MBVQ_FIELDS);
  const char *text[MBVQ_FIELDS];
  for (int f = 0; f < MBVQ_FIELDS; f++) {
    texts[f] = fields_[f]->text().toLocal8Bit();
    text[f] = texts[f].constData();
  }
  int toggle[MBVQ_TOGGLES];
  for (int t = 0; t < MBVQ_TOGGLES; t++)
    toggle[t] = htoc_[t]->isChecked() ? 1 : 0;
  mbvq_c_apply(instance_, dialog, text, toggle);
}

void ControlDialogs::resolution_changed() {
  mbvq_c_setresolution(instance_, resolution_[0]->value(), resolution_[1]->value(), resolution_[2]->value(),
                       resolution_[3]->value());
}

void ControlDialogs::values_changed(int values) {
  if (values == MBVQ_VALUES_ALL_2D) {
    if (dialogs_[MBVQ_DIALOG_VIEW2D]->isVisible())
      load(MBVQ_DIALOG_VIEW2D);
    return;
  }
  if (values == MBVQ_VALUES_ALL_3D) {
    if (dialogs_[MBVQ_DIALOG_VIEW3D]->isVisible())
      load(MBVQ_DIALOG_VIEW3D);
    return;
  }
  if (values == MBV_VALUES_RESOLUTION) {
    if (dialogs_[MBVQ_DIALOG_RESOLUTION]->isVisible())
      load(MBVQ_DIALOG_RESOLUTION);
    return;
  }
  /* as the Motif dialogs: only the fields of the dragged values, if shown */
  for (const ValuesFields &v : kValuesFields) {
    if (v.values != values)
      continue;
    if (dialogs_[dialog_of(v.fields[0])]->isVisible())
      load_fields(v.fields, v.count);
  }
}

void ControlDialogs::set_projection_label(const QString &text) {
  projection_text_ = text;
  if (projection_label_ != nullptr)
    projection_label_->setText(text);
}

}  // namespace mbvq
