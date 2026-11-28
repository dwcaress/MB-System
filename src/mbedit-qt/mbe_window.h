/*--------------------------------------------------------------------
 *    The MB-system:  mbe_window.h  8/11/2026
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

#ifndef MBEDIT_QT_MBE_WINDOW_H
#define MBEDIT_QT_MBE_WINDOW_H

#include <QMainWindow>

#include "mbe_editor.h"

namespace mbe {

/* The window of the program mbedit-qt: the editor with a menu bar, status bar and title. */
class EditorWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit EditorWindow(QWidget *parent = nullptr);

  EditorWidget *editor() { return editor_; }
  Session &session() { return editor_->session(); }
  Canvas *canvas() { return editor_->canvas(); }
  bool busy() const { return editor_->busy(); }

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  EditorWidget *editor_;
};

}  // namespace mbe

#endif
