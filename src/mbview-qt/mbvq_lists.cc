/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_lists.cc  11/7/2026
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

#include "mbvq_lists.h"

#include <QCloseEvent>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

#include "mbvq_bridge.h"

namespace mbvq {

namespace {

ListWindow *windows[MBVQ_LISTS] = {};

bool is_gis(int list) { return list == MBVQ_LIST_POLYLINE || list == MBVQ_LIST_POLYGON || list == MBVQ_LIST_GISPOINT; }

}  // namespace

ListWindow::ListWindow(int list) : QWidget(nullptr), list_(list) {
  setWindowTitle(QString::fromLatin1(mbvq_c_list_title(list)));
  auto *layout = new QVBoxLayout(this);
  auto *header = new QLabel(QString::fromLatin1(mbvq_c_list_header(list)));
  const QFont fixed = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  header->setFont(fixed);
  layout->addWidget(header);
  view_ = new QListWidget;
  view_->setFont(fixed);
  /* as the Motif lists: extended selection */
  view_->setSelectionMode(QAbstractItemView::ExtendedSelection);
  layout->addWidget(view_, 1);
  /* Motif calls the selection callback for every click, also on a selected
     item (a second click on a site changes its colour) */
  connect(view_, &QListWidget::itemSelectionChanged, this, [this] { schedule_selection(); });
  connect(view_, &QListWidget::itemClicked, this, [this] { schedule_selection(); });

  /* the buttons of the Motif list windows */
  auto *buttons = new QHBoxLayout;
  auto add = [this, buttons](const QString &text, int action) {
    auto *button = new QPushButton(text);
    connect(button, &QPushButton::clicked, this, [this, action] { this->action(action); });
    buttons->addWidget(button);
    return button;
  };
  if (list == MBVQ_LIST_SITE) {
    add(tr("Delete Selected Sites"), MBVQ_LIST_DELETE);
  }
  else if (list == MBVQ_LIST_ROUTE) {
    add(tr("Delete Selected Routes"), MBVQ_LIST_DELETE);
    topolylines_ = add(tr("Convert Selected to Polylines"), MBVQ_LIST_TOPOLYLINES);
  }
  else if (list == MBVQ_LIST_NAV) {
    add(tr("Delete Selected Navigation"), MBVQ_LIST_DELETE);
  }
  else {
    add(tr("Show/Hide Selected"), MBVQ_LIST_SHOWHIDE);
    add(tr("Delete Selected"), MBVQ_LIST_DELETE);
    if (list == MBVQ_LIST_POLYLINE)
      add(tr("Convert Selected to Routes"), MBVQ_LIST_TOROUTES);
  }
  buttons->addStretch(1);
  auto *dismiss = new QPushButton(tr("Dismiss"));
  connect(dismiss, &QPushButton::clicked, this, [this] { this->dismiss(); });
  buttons->addWidget(dismiss);
  layout->addLayout(buttons);
  resize(list == MBVQ_LIST_ROUTE ? 760 : 600, 400);
}

void ListWindow::popup() {
  show();
  raise();
  activateWindow();
  mbvq_c_list_shown(list_, true);
  refill();
}

void ListWindow::refill() {
  if (!isVisible())
    return;
  if (topolylines_ != nullptr)
    topolylines_->setVisible(mbvq_c_routes_to_polylines());
  /* as XmListSelectPos without notification: no selection callback */
  const QSignalBlocker block(view_);
  view_->clear();
  void *rows = mbvq_c_rows(list_);
  const int n = mbvq_c_rows_count(rows);
  for (int i = 0; i < n; i++)
    view_->addItem(QString::fromLocal8Bit(mbvq_c_rows_text(rows, i)));
  const int selected = mbvq_c_rows_selected(rows);
  if (selected > 0 && selected <= n) {
    view_->item(selected - 1)->setSelected(true);
    /* as XmListSetPos(MAX(selected - 5, 1)): the item near the top */
    view_->scrollToItem(view_->item(std::max(selected - 5, 1) - 1), QAbstractItemView::PositionAtTop);
  }
  mbvq_c_rows_free(rows);
}

std::vector<int> ListWindow::selected_positions() const {
  std::vector<int> positions;
  for (const QModelIndex &index : view_->selectionModel()->selectedIndexes())
    positions.push_back(index.row() + 1);
  std::sort(positions.begin(), positions.end());
  return positions;
}

void ListWindow::select_positions(const std::vector<int> &positions) {
  const QSignalBlocker block(view_);
  view_->clearSelection();
  for (int p : positions)
    if (p >= 1 && p <= view_->count())
      view_->item(p - 1)->setSelected(true);
}

void ListWindow::schedule_selection() {
  /* one call per user action, after Qt has updated the selection */
  if (selection_pending_)
    return;
  selection_pending_ = true;
  QTimer::singleShot(0, this, [this] {
    selection_pending_ = false;
    selection_made();
  });
}

void ListWindow::selection_made() {
  const std::vector<int> positions = selected_positions();
  const int reselect = mbvq_c_list_action(list_, MBVQ_LIST_SELECT, static_cast<int>(positions.size()), positions.data());
  if (is_gis(list_)) {
    if (reselect > 0)
      select_positions({reselect});
    mbvq_c_list_redraw();
  }
}

void ListWindow::action(int action) {
  const std::vector<int> positions = selected_positions();
  if (action == MBVQ_LIST_SHOWHIDE && positions.empty())
    return;
  mbvq_c_list_action(list_, action, static_cast<int>(positions.size()), positions.data());
  if (action == MBVQ_LIST_SHOWHIDE) {
    /* keep the selection, and update the displays */
    select_positions(positions);
    mbvq_c_list_redraw();
  }
}

void ListWindow::dismiss() {
  mbvq_c_list_shown(list_, false);
  const QSignalBlocker block(view_);
  view_->clear();
  hide();
}

void ListWindow::close_quietly() {
  const QSignalBlocker block(view_);
  view_->clear();
  hide();
}

void ListWindow::closeEvent(QCloseEvent *event) {
  event->accept();
  mbvq_c_list_shown(list_, false);
  const QSignalBlocker block(view_);
  view_->clear();
}

ListWindow *list_window(int list) {
  if (list < 0 || list >= MBVQ_LISTS)
    return nullptr;
  if (windows[list] == nullptr)
    windows[list] = new ListWindow(list);
  return windows[list];
}

void show_list(int list) {
  if (ListWindow *window = list_window(list))
    window->popup();
}

void list_changed(int list) {
  if (list >= 0 && list < MBVQ_LISTS && windows[list] != nullptr)
    windows[list]->refill();
}

void close_lists() {
  for (ListWindow *window : windows)
    if (window != nullptr)
      window->close_quietly();
}

}  // namespace mbvq
