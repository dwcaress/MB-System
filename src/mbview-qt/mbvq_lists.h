/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_lists.h  11/7/2026
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
 * The site, route, navigation, polyline, polygon and point list windows,
 * shared by all instances as in Motif. They show the rows of the core and
 * pass the selected list positions to the core actions.
 */

#ifndef MBVIEW_QT_MBVQ_LISTS_H
#define MBVIEW_QT_MBVQ_LISTS_H

#include <QWidget>
#include <vector>

class QListWidget;
class QPushButton;

namespace mbvq {

class ListWindow final : public QWidget {
 public:
  explicit ListWindow(int list);

  /* show the window with the rows of the core (the Motif popup) */
  void popup();
  /* show the rows of the core again, if the window is shown */
  void refill();
  /* close without the core (the last instance closed) */
  void close_quietly();
  QListWidget *view() const { return view_; }

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  std::vector<int> selected_positions() const;
  void select_positions(const std::vector<int> &positions);
  void schedule_selection();
  void selection_made();
  void action(int action);
  void dismiss();

  int list_;
  QListWidget *view_ = nullptr;
  QPushButton *topolylines_ = nullptr;
  bool selection_pending_ = false;
};

/* the window of a list (MBVQ_LIST_*), created when first used */
ListWindow *list_window(int list);
void show_list(int list);
/* the rows of a list changed in the core */
void list_changed(int list);
/* close all list windows */
void close_lists();

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_LISTS_H */
