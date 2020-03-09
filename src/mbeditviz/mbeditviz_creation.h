#ifndef mbeditviz_creation_H
#define mbeditviz_creation_H

#ifdef DECLARE_BX_GLOBALS
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

EXTERNAL Widget bulletinBoard_error;
EXTERNAL Widget label_error_two;
EXTERNAL Widget label_error_one;
EXTERNAL Widget label_error_three;
EXTERNAL Widget scale_cellsize;
EXTERNAL Widget label_current;
EXTERNAL Widget label_implied;
EXTERNAL Widget label_gridalgorithm;
EXTERNAL Widget radioBox_gridalgorithm;
EXTERNAL Widget toggleButton_gridalgorithm_simplemean;
EXTERNAL Widget toggleButton_gridalgorithm_footprint;
EXTERNAL Widget toggleButton_gridalgorithm_shoalbias;
EXTERNAL Widget text_interpolation;
EXTERNAL Widget label_interpolation;
EXTERNAL Widget radioBox_openmode;
EXTERNAL Widget toggleButton_openmodeedit;
EXTERNAL Widget toggleButton_openmodebrowse;
EXTERNAL Widget dialogShell_open;
EXTERNAL Widget form_open;
EXTERNAL Widget text_format;
EXTERNAL Widget fileSelectionBox;
EXTERNAL Widget dialogShell_mbeditviz_message;
EXTERNAL Widget bulletinBoard_mbeditviz_message;
EXTERNAL Widget label_mbeditviz_message;
EXTERNAL Widget label_about_version;
EXTERNAL Widget pushButton_openswath;
EXTERNAL Widget pushButton_updategrid;
EXTERNAL Widget label_mbeditviz_status;
EXTERNAL Widget toggleButton_mode_edit;
EXTERNAL Widget toggleButton_mode_browse;
EXTERNAL Widget pushButton_deleteselected;
EXTERNAL Widget pushButton_viewselected;
EXTERNAL Widget pushButton_viewall;
EXTERNAL Widget list_filelist;

#endif
