/*--------------------------------------------------------------------
 *    The MB-system:  mbne_window.h  9/11/2026
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

#ifndef MBNAVEDIT_QT_MBNE_WINDOW_H
#define MBNAVEDIT_QT_MBNE_WINDOW_H

#include <QMainWindow>

#include "mbne_widget.h"

namespace mbne {

/* The window of the program mbnavedit-qt: the editor with a menu bar, status bar and title. */
class NavEditWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit NavEditWindow(QWidget *parent = nullptr);

  NavEditWidget *editor() { return editor_; }
  Session &session() { return editor_->session(); }
  Canvas *canvas() { return editor_->canvas(); }
  bool busy() const { return editor_->busy(); }

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  NavEditWidget *editor_;
};

}  // namespace mbne

#endif
