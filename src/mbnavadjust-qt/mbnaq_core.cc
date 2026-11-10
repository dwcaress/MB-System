/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_core.cc  9/11/2026
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

#include "mbnaq_core.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>

extern "C" {
#include <X11/Intrinsic.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbnavadjust_io.h"
#include "mbnavadjust.h"
#include "mbview.h"
}

#define MBNA_LIST_KINDS 16
#define MBNA_VIEW_MODES 7

namespace mbnaq {

static const size_t kUndoLimit = 100;

/* What an interactive edit can change. */
struct UndoStep {
  int crossingIndex = -1;
  struct mbna_crossing crossing;
  std::vector<std::pair<std::pair<int, int>, struct mbna_section>> sections;
  std::vector<int> fileStatus;
  int numTies, numGlobalTies, numCrossingsAnalyzed, numTrueCrossingsAnalyzed, numGlobalTiesAnalyzed;
  int inversionStatus, gridStatus;
  int crossingSelect, tieSelect, fileSelect, sectionSelect, surveySelect;
  int naverrMode, currentCrossing, currentTie, currentFile, currentSection;
};

static Session *the_session = nullptr;

DrawSurface::~DrawSurface() = default;

namespace {

/* Draws nothing. The plot code has side effects that the program needs even
   when no window shows the plots. */
class NullSurface : public DrawSurface {
 public:
  void line(int, int, int, int, unsigned int, int) override {}
  void rectangle(int, int, int, int, unsigned int, int) override {}
  void fillRectangle(int, int, int, int, unsigned int, int) override {}
  void text(int, int, const char *, unsigned int, int) override {}
  void measure(const char *string, int *width, int *ascent, int *descent) override {
    *width = 6 * static_cast<int>(strlen(string));
    *ascent = 10;
    *descent = 3;
  }
  void clip(int, int, int, int) override {}
};

NullSurface null_cont, null_corr, null_zoff;

std::string fmt(const char *format, ...) {
  char buffer[MB_PATH_MAXLINE * 4];
  va_list ap;
  va_start(ap, format);
  vsnprintf(buffer, sizeof(buffer), format, ap);
  va_end(ap);
  return buffer;
}

static const char *const kListTitles[MBNA_LIST_KINDS][MBNA_VIEW_MODES] = {
    /* REFERENCEGRIDS */ {
        "Reference Grids:",
        "Reference Grids:",
        "Reference Grids:",
        "Reference Grids:",
        "Reference Grids:",
        "Reference Grids:",
        "Reference Grids:",
    },
    /* SURVEYS */ {
        "Surveys:",
        "Surveys:",
        "Surveys:",
        "Surveys:",
        "Surveys:",
        "Surveys:",
        "Surveys:",
    },
    /* BLOCKS */ {
        "Survey-vs-Survey Blocks:",
        "Survey-vs-Survey Blocks:",
        "Survey-vs-Survey Blocks:",
        "Survey-vs-Survey Blocks:",
        "Survey-vs-Survey Blocks:",
        "Survey-vs-Survey Blocks:",
        "Survey-vs-Survey Blocks:",
    },
    /* FILES */ {
        "Data Files:",
        "Data Files of Survey %d:",
        "Data Files of Survey %d:",
        "Data Files of Survey-vs-Survey Block %d:",
        "Data File %d:%d:",
        "Data File %d:%d:",
        "Data File of Selected Section %d:%d:%d:",
    },
    /* FILESECTIONS */ {
        "Data File Sections:",
        "Data File Sections of Survey %d:",
        "Data File Sections of Survey %d:",
        "Data Files Sections of Survey-vs-Survey Block %d:",
        "Data File Sections of File %d:%d:",
        "Data File Sections of File %d:%d:",
        "Data File Sections of File %d:%d:",
    },
    /* CROSSINGS */ {
        "Crossings:",
        "Crossings of Survey %d:",
        "Crossings with Survey %d:",
        "Crossings of Survey-vs-Survey Block %d:",
        "Crossings of File %d:%d:",
        "Crossings with File %d:%d:",
        "Crossings with Section %d:%d:%d:",
    },
    /* MEDIOCRECROSSINGS */ {
        ">10%% Overlap Crossings:",
        ">10%% Overlap Crossings of Survey %d:",
        ">10%% Overlap Crossings with Survey %d:",
        ">10%% Overlap Crossings of Survey-vs-Survey Block %d:",
        ">10%% Overlap Crossings of File %d:%d:",
        ">10%% Overlap Crossings with File %d:%d:",
        ">10%% Overlap Crossings with Section %d:%d:%d:",
    },
    /* GOODCROSSINGS */ {
        ">25%% Overlap Crossings:",
        ">25%% Overlap Crossings of Survey %d:",
        ">25%% Overlap Crossings with Survey %d:",
        ">25%% Overlap Crossings of Survey-vs-Survey Block %d:",
        ">25%% Overlap Crossings of File %d:%d:",
        ">25%% Overlap Crossings with File %d:%d:",
        ">25%% Overlap Crossings with Section %d:%d:%d:",
    },
    /* BETTERCROSSINGS */ {
        ">50%% Crossings:",
        ">50%% Crossings of Survey %d:",
        ">50%% Overlap Crossings with Survey %d:",
        ">50%% Overlap Crossings of Survey-vs-Survey Block %d:",
        ">50%% Crossings of File %d:%d:",
        ">50%% Overlap Crossings with File %d:%d:",
        ">50%% Overlap Crossings with Section %d:%d:%d:",
    },
    /* TRUECROSSINGS */ {
        "True Crossings:",
        "True Crossings of Survey %d:",
        "True Crossings with Survey %d:",
        "True Crossings of Survey-vs-Survey Block %d:",
        "True Crossings of File %d:%d:",
        "True Crossings with File %d:%d:",
        "True Crossings with Section %d:%d:%d:",
    },
    /* TIES */ {
        "Ties:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Ties of Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Ties with Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Ties of Survey-vs-Survey Block %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Ties with File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Ties with Section %d:%d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
    },
    /* TIESSORTEDALL */ {
        "Sorted Ties:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties of Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties with Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties of Survey-vs-Survey Block %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties with File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties with Section %d:%d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
    },
    /* TIESSORTEDWORST */ {
        "Sorted Ties:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties of Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties with Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties of Survey-vs-Survey Block %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties with File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties with Section %d:%d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
    },
    /* TIESSORTEDBAD */ {
        "Sorted Ties:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties of Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties with Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties of Survey-vs-Survey Block %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties with File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Sorted Ties with Section %d:%d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
    },
    /* GLOBALTIES */ {
        "Global Ties:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties of Survey %d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties with Survey %d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties of Survey-vs-Survey Block %d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties of File %d:%d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties of File %d:%d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties of Section %d:%d:%d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
    },
    /* GLOBALTIESSORTED */ {
        "Global Ties:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties of Survey %d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties with Survey %d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties of Survey-vs-Survey Block %d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties of File %d:%d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties of File %d:%d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
        "Global Ties of Section %d:%d:%d:  Sur:File:Sec:Nv Stat RefGrid  Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr",
    },
};

const char *tieStatusText(int status) {
  switch (status) {
  case MBNA_TIE_XYZ:
    return "XYZU";
  case MBNA_TIE_XY:
    return "XY_U";
  case MBNA_TIE_Z:
    return "__ZU";
  case MBNA_TIE_XYZ_FIXED:
    return "XYZF";
  case MBNA_TIE_XY_FIXED:
    return "XY_F";
  case MBNA_TIE_Z_FIXED:
    return "__ZF";
  default:
    return "";
  }
}

const char *fileStatusText(int status) {
  switch (status) {
  case MBNA_FILE_POORNAV:
    return " poor  ";
  case MBNA_FILE_GOODNAV:
    return " good  ";
  case MBNA_FILE_FIXEDNAV:
    return " fixed ";
  case MBNA_FILE_FIXEDXYNAV:
    return "fixedxy";
  case MBNA_FILE_FIXEDZNAV:
    return "fixedz ";
  default:
    return "unknown";
  }
}

char sectionStatusChar(int status) {
  return status == MBNA_CROSSING_STATUS_NONE ? 'U' : (status == MBNA_CROSSING_STATUS_SET ? '*' : '-');
}

bool isCrossingList(int kind) {
  return kind == MBNA_VIEW_LIST_CROSSINGS || kind == MBNA_VIEW_LIST_MEDIOCRECROSSINGS ||
         kind == MBNA_VIEW_LIST_GOODCROSSINGS || kind == MBNA_VIEW_LIST_BETTERCROSSINGS ||
         kind == MBNA_VIEW_LIST_TRUECROSSINGS;
}

bool isSortedTieList(int kind) {
  return kind == MBNA_VIEW_LIST_TIESSORTEDALL || kind == MBNA_VIEW_LIST_TIESSORTEDWORST ||
         kind == MBNA_VIEW_LIST_TIESSORTEDBAD;
}

std::string tieRow(int icrossing, int itie, const struct mbna_crossing *crossing, const struct mbna_tie *tie) {
  std::string row = fmt("%6d %2d %s %2.2d:%4.4d:%3.3d:%2.2d %2.2d:%4.4d:%3.3d:%2.2d %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f",
                        icrossing, itie, tieStatusText(tie->status), project.files[crossing->file_id_1].survey,
                        crossing->file_id_1, crossing->section_1, tie->snav_1, project.files[crossing->file_id_2].survey,
                        crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                        tie->offset_z_m, tie->sigmar1, tie->sigmar2, tie->sigmar3);
  if (tie->inversion_status == MBNA_INVERSION_CURRENT || tie->inversion_status == MBNA_INVERSION_OLD) {
    row += fmt(" | %8.2f %8.2f %8.2f | %8.2f %6.3f", tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m, tie->rsigma_m);
    if (tie->inversion_status == MBNA_INVERSION_OLD)
      row += " ***";
  }
  return row;
}

/* The name of the reference grid of a global tie, or the given default. */
std::string refGridName(const struct mbna_globaltie *globaltie, const char *none) {
  int refgrid_id = project.refgrid_select;
  if (globaltie->refgrid_id >= 0 && project.refgrid_select < 0)
    refgrid_id = globaltie->refgrid_id;
  if (refgrid_id >= 0 && refgrid_id < project.num_refgrids)
    return project.refgrid_names[refgrid_id];
  return none;
}

std::string globalTieRow(const struct mbna_section *section, int survey, int ifile, int isection) {
  const struct mbna_globaltie *globaltie = &section->globaltie;
  const std::string refgrid = refGridName(globaltie, " | No reference grid");
  std::string row = fmt("%2.2d:%4.4d:%3.3d:%2.2d %s %2d %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f", survey, ifile, isection,
                        globaltie->snav, tieStatusText(globaltie->status), globaltie->refgrid_id, globaltie->offset_x_m,
                        globaltie->offset_y_m, globaltie->offset_z_m, globaltie->sigmar1, globaltie->sigmar2,
                        globaltie->sigmar3);
  if (globaltie->inversion_status == MBNA_INVERSION_CURRENT || globaltie->inversion_status == MBNA_INVERSION_OLD) {
    row += fmt(" | %8.2f %8.2f %8.2f | %8.2f %6.3f", globaltie->dx_m, globaltie->dy_m, globaltie->dz_m, globaltie->sigma_m,
               globaltie->rsigma_m);
    if (globaltie->inversion_status == MBNA_INVERSION_OLD)
      row += " ***";
  }
  row += " | " + refgrid;
  return row;
}

bool globalTieRowOk(const struct mbna_file *file, const struct mbna_section *section, int ifile, int isection) {
  return section->status == MBNA_CROSSING_STATUS_SET &&
         ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
          (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->survey) ||
          (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == ifile) ||
          (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->survey) ||
          (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == ifile) ||
          (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == ifile && mbna_section_select == isection));
}

std::string timeRange(double btime_d, double etime_d) {
  int btime_i[7], etime_i[7];
  mb_get_date(mbna_verbose, btime_d, btime_i);
  mb_get_date(mbna_verbose, etime_d, etime_i);
  return fmt("%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d", btime_i[0], btime_i[1],
             btime_i[2], btime_i[3], btime_i[4], btime_i[5], btime_i[6], etime_i[0], etime_i[1], etime_i[2], etime_i[3],
             etime_i[4], etime_i[5], etime_i[6]);
}

const char *const kShowButtons[MBNA_LIST_KINDS] = {
    "pushButton_showreferencegrids",     "pushButton_showsurveys",
    "pushButton_showblocks",             "pushButton_showdata",
    "pushButton_showsections",           "pushButton_showcrossings",
    "pushButton_showmediocrecrossings",  "pushButton_showgoodcrossings",
    "pushButton_showbettercrossings",    "pushButton_showtruecrossings",
    "pushButton_showcrossingties",       "pushButton_showcrossingtiessortedall",
    "pushButton_showcrossingtiessortedworst", "pushButton_showcrossingtiessortedbad",
    "pushButton_showglobalties",         "pushButton_showglobaltiessorted"};

const char *const kViewToggles[] = {"toggleButton_showallsurveys",        "toggleButton_showselectedsurvey",
                                    "toggleButton_showselectedblock",     "toggleButton_showselectedfile",
                                    "toggleButton_showwithselectedsurvey", "toggleButton_showwithselectedfile",
                                    "toggleButton_showselectedsection"};

const char *const kNavSteps[] = {"pushButton_naverr_previous", "pushButton_naverr_next", "pushButton_naverr_nextunset"};

const char *const kFileNavButtons[] = {"pushButton_poornav", "pushButton_goodnav", "pushButton_fixednav",
                                       "pushButton_fixedxynav", "pushButton_fixedznav"};
const int kFileNavStatus[] = {MBNA_FILE_POORNAV, MBNA_FILE_GOODNAV, MBNA_FILE_FIXEDNAV, MBNA_FILE_FIXEDXYNAV,
                              MBNA_FILE_FIXEDZNAV};

const int kCount3 = 3;
const int kCount5 = 5;
const int kCount7 = 7;

}  // namespace

#if MBNA_VIEW_LIST_GLOBALTIESSORTED != 15 || MBNA_VIEW_MODE_WITHSECTION != 6
#error "the list kinds and view modes of mbnaq_core.h no longer match mbnavadjust_io.h"
#endif

/*--------------------------------------------------------------------*/

Session *Session::create() {
  if (the_session != nullptr)
    return nullptr;
  the_session = new Session();
  return the_session;
}

Session *Session::current() { return the_session; }

Session::Session() : contourSurface_(&null_cont), misfitSurface_(&null_corr), zOffsetSurface_(&null_zoff) {
  mbnavadjust_init_globals();

  /* colors are passed through as indexes that the surfaces interpret */
  int pixels[MB_NDrawingColors + 16 * 5 + 1];
  for (int i = 0; i < MB_NDrawingColors + 16 * 5 + 1; i++)
    pixels[i] = i;
  mbnavadjust_set_colors(MB_NDrawingColors + 16 * 5 + 1, pixels);

  int cont[4] = {0, 600, 0, 600};
  int corr[4] = {0, 301, 0, 301};
  int zoff[4] = {0, 300, 0, 60};
  mbnavadjust_set_borders(cont, corr, zoff);
}

Session::~Session() {
  if (the_session == this)
    the_session = nullptr;
}

bool Session::misfitCenterIsZero() const { return mbna_misfit_center == MBNA_MISFIT_ZEROCENTER; }

void Session::setCallbacks(const Callbacks &callbacks) { callbacks_ = callbacks; }

void Session::setVerbose(int verbose) { mbna_verbose = verbose; }

void Session::setDarkBackground(bool dark) {
  mbna_color_foreground = dark ? MB_COLOR_WHITE : MB_COLOR_BLACK;
  mbna_color_background = dark ? MB_COLOR_BLACK : MB_COLOR_WHITE;
}

void Session::setResetCrossings(bool reset) { mbna_reset_crossings = reset; }

bool Session::isEnabled(const std::string &widget) const {
  auto it = enabled_.find(widget);
  return it != enabled_.end() && it->second;
}

void Session::setEnabled(const char *widget, bool enabled) { enabled_[widget] = enabled; }

void Session::setEnabled(const char *const *widgets, int count, bool enabled) {
  for (int i = 0; i < count; i++)
    enabled_[widgets[i]] = enabled;
}

Selection Session::selection() const {
  Selection s;
  s.list = mbna_view_list;
  s.mode = mbna_view_mode;
  s.survey = mbna_survey_select;
  s.survey1 = mbna_survey_select1;
  s.survey2 = mbna_survey_select2;
  s.file = mbna_file_select;
  s.section = mbna_section_select;
  s.crossing = mbna_crossing_select;
  s.tie = mbna_tie_select;
  s.naverrMode = mbna_naverr_mode;
  s.status = mbna_status;
  s.currentCrossing = mbna_current_crossing;
  s.currentTie = mbna_current_tie;
  s.currentFile = mbna_current_file;
  s.currentSection = mbna_current_section;
  s.offsetX = mbna_offset_x;
  s.offsetY = mbna_offset_y;
  s.offsetZ = mbna_offset_z;
  s.sortedThreshold = project.tiessortedthreshold;
  s.saveCount = project.save_count;
  s.projectIsOpen = project.open;
  s.numTies = project.num_ties;
  return s;
}

/*--------------------------------------------------------------------*/
/* what is displayed */

void Session::updateStatus() {
  buildStatusLines();
  buildList();
  updateEnabled();
  if (callbacks_.statusChanged)
    callbacks_.statusChanged();
}

void Session::buildStatusLines() {
  const char *use_mode = "Primary";
  if (project.use_mode == MBNA_USE_MODE_SECONDARY) {
    use_mode = "Secondary";
  } else if (project.use_mode == MBNA_USE_MODE_TERTIARY) {
    use_mode = "Tertiary";
  } else {
    project.use_mode = MBNA_USE_MODE_PRIMARY;
  }
  const char *refgrid = "<Previously Selected Reference Grid>";
  if (project.refgrid_select >= 0 && project.refgrid_select < project.num_refgrids)
    refgrid = project.refgrid_names[project.refgrid_select];

  statusLines_.clear();
  statusLines_.push_back(fmt("Project: %s", project.name));
  statusLines_.push_back(fmt("Navigation Adjustment Use Mode:       %s", use_mode));
  statusLines_.push_back(fmt("Number of Files:                               %4d      Selected Survey:  %4d",
                             project.num_files, mbna_survey_select));
  statusLines_.push_back(fmt("Number of Crossings Found:             %4d     Selected File:    %4d", project.num_crossings,
                             mbna_file_select));
  statusLines_.push_back(fmt("Number of Crossings Analyzed:        %4d     Selected Section: %4d",
                             project.num_crossings_analyzed, mbna_section_select));
  statusLines_.push_back(fmt("Number of True Crossings:               %4d     Selected Crossing:%4d",
                             project.num_truecrossings, mbna_crossing_select));
  statusLines_.push_back(fmt("Number of True Crossings Analyzed: %4d     Selected Tie:     %4d",
                             project.num_truecrossings_analyzed, mbna_tie_select));
  statusLines_.push_back(fmt("Number of Ties Set:                        %4d", project.num_ties));
  statusLines_.push_back(fmt("Number of Global Ties Set:              %4d", project.num_globalties));
  statusLines_.push_back(fmt("Reference Grid: %s", refgrid));
  if (project.inversion_status == MBNA_INVERSION_CURRENT)
    statusLines_.push_back("Inversion Performed:                     Current");
  else if (project.inversion_status == MBNA_INVERSION_OLD)
    statusLines_.push_back("Inversion Performed:                     Out of Date");
  else
    statusLines_.push_back("Inversion Performed:                         No");
  if (project.grid_status == MBNA_GRID_CURRENT)
    statusLines_.push_back("Topography Grid Status:                      Current");
  else if (project.grid_status == MBNA_GRID_OLD)
    statusLines_.push_back("Topography Grid Status:                    Out of Date");
  else
    statusLines_.push_back("Topography Grid Status:                    Not made yet");
}

std::string Session::listTitle() const {
  if (mbna_view_list < 0 || mbna_view_list >= MBNA_LIST_KINDS || mbna_view_mode < 0 || mbna_view_mode >= MBNA_VIEW_MODES)
    return std::string();
  return fmt(kListTitles[mbna_view_list][mbna_view_mode], mbna_survey_select, mbna_file_select, mbna_section_select);
}

void Session::buildList() {
  list_.title = listTitle();
  list_.rows.clear();
  list_.selected = -1;
  rowRefs_.clear();

  auto addRow = [this](const std::string &row, int a, int b, int c) {
    list_.rows.push_back(row);
    rowRefs_.push_back(RowRef{a, b, c});
  };

  if (mbna_view_list == MBNA_VIEW_LIST_REFERENCEGRIDS) {
    if (project.num_refgrids > 0) {
      addRow("<Previously Selected Grid>", 0, 0, 0);
      for (int i = 0; i < project.num_refgrids; i++)
        addRow(project.refgrid_names[i], 0, 0, 0);
    }
    list_.selected = project.refgrid_select + 1;
  } else if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS) {
    if (project.num_surveys > 0 && project.num_files > 0) {
      for (int isurvey = 0; isurvey < project.num_surveys; isurvey++) {
        int num_files_survey = 0;
        int num_global_ties_survey = 0;
        bool first = true;
        double btime_d = 0.0;
        double etime_d = 0.0;
        const char *filestatus = fileStatusText(-1);
        for (int ifile = 0; ifile < project.num_files; ifile++) {
          struct mbna_file *file = &(project.files[ifile]);
          if (file->survey == isurvey) {
            if (first) {
              btime_d = file->sections[0].btime_d;
              first = false;
              filestatus = fileStatusText(file->status);
            }
            num_files_survey++;
            etime_d = file->sections[file->num_sections - 1].etime_d;
            for (int isection = 0; isection < file->num_sections; isection++) {
              if (file->sections[isection].globaltie.status != MBNA_TIE_NONE)
                num_global_ties_survey++;
            }
          }
        }
        addRow(fmt("%3d %3d %s %s g-ties:%d", isurvey, num_files_survey, timeRange(btime_d, etime_d).c_str(), filestatus,
                   num_global_ties_survey),
               isurvey, 0, 0);
      }
    }
    if (mbna_survey_select != MBNA_SELECT_NONE)
      list_.selected = mbna_survey_select;
  } else if (mbna_view_list == MBNA_VIEW_LIST_BLOCKS) {
    if (project.num_files > 0) {
      const int num_blocks = project.num_surveys + (project.num_surveys * (project.num_surveys - 1) / 2);
      std::vector<int> survey1(num_blocks, 0), survey2(num_blocks, 0), n_true(num_blocks, 0), n_50(num_blocks, 0),
          n_25(num_blocks, 0), n_all(num_blocks, 0), n_tie(num_blocks, 0);
      int iblock = 0;
      for (int isurvey2 = 0; isurvey2 < project.num_surveys; isurvey2++) {
        for (int isurvey1 = 0; isurvey1 <= isurvey2; isurvey1++) {
          survey1[iblock] = isurvey1;
          survey2[iblock] = isurvey2;
          iblock++;
        }
      }
      for (int k = 0; k < project.num_crossings; k++) {
        const struct mbna_crossing *crossing = &project.crossings[k];
        const int s2 = project.files[crossing->file_id_2].survey;
        const int b = project.files[crossing->file_id_1].survey + (s2 * (s2 + 1) / 2);
        if (crossing->truecrossing)
          n_true[b]++;
        if (crossing->overlap >= 50)
          n_50[b]++;
        if (crossing->overlap >= 25)
          n_25[b]++;
        n_all[b]++;
        n_tie[b] += crossing->num_ties;
      }
      const bool file_ok = mbna_file_select >= 0 && mbna_file_select < project.num_files;
      const int file_survey = file_ok ? project.files[mbna_file_select].survey : -1;
      int block_selected = MBNA_SELECT_NONE;
      for (int b = 0; b < num_blocks; b++) {
        const bool any_with_survey = survey1[b] == mbna_survey_select || survey2[b] == mbna_survey_select;
        const bool any_with_file = file_ok && (survey1[b] == file_survey || survey2[b] == file_survey);
        if (mbna_view_mode == MBNA_VIEW_MODE_ALL ||
            (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && survey1[b] == mbna_survey_select && survey2[b] == mbna_survey_select) ||
            mbna_view_mode == MBNA_VIEW_MODE_BLOCK ||
            (mbna_view_mode == MBNA_VIEW_MODE_FILE && file_ok && survey1[b] == file_survey && survey2[b] == file_survey) ||
            (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && any_with_survey) ||
            (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && any_with_file) ||
            (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && any_with_file)) {
          if (b == mbna_survey_select)
            block_selected = static_cast<int>(list_.rows.size());
          addRow(fmt("block %4.4d: Survey %2.2d vs Survey %2.2d : Crossings: %4d %4d %4d %4d : Ties: %4d", b, survey1[b],
                     survey2[b], n_true[b], n_50[b], n_25[b], n_all[b], n_tie[b]),
                 b, survey1[b], survey2[b]);
        }
      }
      if (mbna_survey_select != MBNA_SELECT_NONE && block_selected != MBNA_SELECT_NONE)
        list_.selected = block_selected;
    }
  } else if (mbna_view_list == MBNA_VIEW_LIST_FILES) {
    if (project.num_files > 0) {
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
            (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->survey) ||
            (mbna_view_mode == MBNA_VIEW_MODE_FILE) ||
            (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->survey) ||
            (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) || (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)) {
          int num_global_ties = 0;
          for (int isection = 0; isection < file->num_sections; isection++) {
            if (file->sections[isection].globaltie.status != MBNA_TIE_NONE)
              num_global_ties++;
          }
          if (i == mbna_file_select)
            list_.selected = static_cast<int>(list_.rows.size());
          addRow(fmt("%2.2d:%4.4d %s %4d %4.1f %4.1f g-ties:%d  %s", file->survey, file->id, fileStatusText(file->status),
                     file->num_sections, file->heading_bias, file->roll_bias, num_global_ties, file->file),
                 i, file->survey, 0);
        }
      }
    }
  } else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
    if (project.num_files > 0) {
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        for (int j = 0; j < file->num_sections; j++) {
          struct mbna_section *section = &(file->sections[j]);
          if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
              (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->survey) ||
              (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
               (mbna_survey_select1 == file->survey || mbna_survey_select2 == file->survey)) ||
              (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == i) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->survey) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == i) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == i)) {
            std::string row = fmt("%c %2.2d:%4.4d:%2.2d %s", sectionStatusChar(section->status), file->survey, file->id, j,
                                  timeRange(section->btime_d, section->etime_d).c_str());
            if (section->status == MBNA_CROSSING_STATUS_SET) {
              const struct mbna_globaltie *g = &section->globaltie;
              row += fmt(" | %2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f", g->snav, tieStatusText(g->status), g->offset_x_m,
                         g->offset_y_m, g->offset_z_m, g->sigmar1, g->sigmar2, g->sigmar3);
              if (g->inversion_status == MBNA_INVERSION_CURRENT || g->inversion_status == MBNA_INVERSION_OLD) {
                row += fmt(" | %8.2f %8.2f %8.2f | %8.2f %6.3f", g->dx_m, g->dy_m, g->dz_m, g->sigma_m, g->rsigma_m);
                if (g->inversion_status == MBNA_INVERSION_OLD)
                  row += " ***";
                row += " | " + refGridName(g, " | No reference grid");
              }
            }
            if (i == mbna_file_select && j == mbna_section_select)
              list_.selected = static_cast<int>(list_.rows.size());
            addRow(row, i, j, file->survey);
          }
        }
      }
    }
  } else if (isCrossingList(mbna_view_list)) {
    if (project.num_files > 0) {
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          struct mbna_crossing *crossing = &(project.crossings[i]);
          /* the Motif code has this marker the other way round in the true crossings list */
          const bool marked = mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS && motifCompatibility_ ? !crossing->truecrossing
                                                                                                     : crossing->truecrossing;
          const char trueMarker = marked ? 'X' : ' ';
          if (i == mbna_crossing_select)
            list_.selected = static_cast<int>(list_.rows.size());
          addRow(fmt("%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", sectionStatusChar(crossing->status),
                     trueMarker, i, project.files[crossing->file_id_1].survey, crossing->file_id_1,
                     crossing->section_1, project.files[crossing->file_id_2].survey, crossing->file_id_2, crossing->section_2,
                     crossing->overlap, crossing->num_ties),
                 i, 0, 0);
        }
      }
    }
  } else if (mbna_view_list == MBNA_VIEW_LIST_TIES) {
    if (project.num_files > 0) {
      project.tiessortedthreshold = 0.0;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          struct mbna_crossing *crossing = &(project.crossings[i]);
          for (int j = 0; j < crossing->num_ties; j++) {
            if (i == mbna_crossing_select && j == mbna_tie_select)
              list_.selected = static_cast<int>(list_.rows.size());
            addRow(tieRow(i, j, crossing, &crossing->ties[j]), i, j, 0);
          }
        }
      }
    }
  } else if (isSortedTieList(mbna_view_list)) {
    if (project.num_files > 0) {
      project.tiessortedthreshold = 0.0;
      std::vector<struct mbna_tie *> ties;
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
        if (do_check_crossing_listok(icrossing)) {
          struct mbna_crossing *crossing = &(project.crossings[icrossing]);
          for (int itie = 0; itie < crossing->num_ties; itie++) {
            struct mbna_tie *tie = &crossing->ties[itie];
            tie->icrossing = icrossing;
            tie->itie = itie;
            ties.push_back(tie);
          }
        }
      }
      const int num_ties = static_cast<int>(ties.size());
      if (num_ties > 0) {
        qsort(ties.data(), ties.size(), sizeof(struct mbna_tie *), mbnavadjust_tie_compare);
        project.tiessortedthreshold = 0.0;
        if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST)
          project.tiessortedthreshold = ties[99 * num_ties / 100]->sigma_m;
        else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD)
          project.tiessortedthreshold = ties[19 * num_ties / 20]->sigma_m;
      }
      for (int ktie = num_ties - 1; ktie >= 0; ktie--) {
        struct mbna_tie *tie = ties[ktie];
        if (do_check_crossing_listok(tie->icrossing) && tie->sigma_m >= project.tiessortedthreshold) {
          struct mbna_crossing *crossing = &(project.crossings[tie->icrossing]);
          if (tie->icrossing == mbna_crossing_select && tie->itie == mbna_tie_select)
            list_.selected = static_cast<int>(list_.rows.size());
          addRow(tieRow(tie->icrossing, tie->itie, crossing, tie), tie->icrossing, tie->itie, 0);
        }
      }
    }
  } else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES) {
    if (project.num_files > 0) {
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        for (int j = 0; j < file->num_sections; j++) {
          struct mbna_section *section = &(file->sections[j]);
          if (globalTieRowOk(file, section, i, j)) {
            if (section->section_id == mbna_section_select && section->file_id == mbna_file_select)
              list_.selected = static_cast<int>(list_.rows.size());
            addRow(globalTieRow(section, file->survey, i, j), i, j, 0);
          }
        }
      }
    }
  } else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
    if (project.num_files > 0) {
      std::vector<struct mbna_section *> sections;
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        for (int j = 0; j < file->num_sections; j++) {
          if (globalTieRowOk(file, &file->sections[j], i, j))
            sections.push_back(&file->sections[j]);
        }
      }
      qsort(sections.data(), sections.size(), sizeof(struct mbna_section *), mbnavadjust_globaltie_compare);
      for (int k = static_cast<int>(sections.size()) - 1; k >= 0; k--) {
        struct mbna_section *section = sections[k];
        if (section->section_id == mbna_section_select && section->file_id == mbna_file_select)
          list_.selected = static_cast<int>(list_.rows.size());
        addRow(globalTieRow(section, project.files[section->file_id].survey, section->file_id, section->section_id),
               section->file_id, section->section_id, 0);
      }
    }
  }
}

void Session::updateEnabled() {
  const int kind = mbna_view_list;

  setEnabled(kViewToggles, kCount7, true);

  /* file navigation status buttons */
  const bool file_ok_for_nav =
      (kind == MBNA_VIEW_LIST_SURVEYS && project.num_files > 0 && mbna_survey_select != MBNA_SELECT_NONE &&
       mbna_file_select >= 0 && mbna_file_select < project.num_files) ||
      (kind == MBNA_VIEW_LIST_FILES && project.num_files > 0 && mbna_file_select != MBNA_SELECT_NONE &&
       mbna_file_select < project.num_files);
  for (int i = 0; i < kCount5; i++)
    setEnabled(kFileNavButtons[i], file_ok_for_nav && project.files[mbna_file_select].status != kFileNavStatus[i]);

  /* tie status buttons */
  int tie_status = -1;
  if ((kind == MBNA_VIEW_LIST_TIES || isSortedTieList(kind)) && project.num_files > 0 &&
      mbna_tie_select != MBNA_SELECT_NONE && mbna_crossing_select >= 0 && mbna_crossing_select < project.num_crossings &&
      mbna_tie_select >= 0 && mbna_tie_select < project.crossings[mbna_crossing_select].num_ties) {
    tie_status = project.crossings[mbna_crossing_select].ties[mbna_tie_select].status;
  } else if ((kind == MBNA_VIEW_LIST_GLOBALTIES || kind == MBNA_VIEW_LIST_GLOBALTIESSORTED) && project.num_files > 0 &&
             mbna_file_select != MBNA_SELECT_NONE && mbna_section_select != MBNA_SELECT_NONE && mbna_file_select >= 0 &&
             mbna_file_select < project.num_files && mbna_section_select >= 0 &&
             mbna_section_select < project.files[mbna_file_select].num_sections &&
             project.files[mbna_file_select].sections[mbna_section_select].status == MBNA_CROSSING_STATUS_SET) {
    tie_status = project.files[mbna_file_select].sections[mbna_section_select].globaltie.status;
  }
  if (tie_status >= 0) {
    if (tie_status == MBNA_TIE_XY || tie_status == MBNA_TIE_XY_FIXED) {
      setEnabled("pushButton_tie_xyz", true);
      setEnabled("pushButton_tie_xy", false);
      setEnabled("pushButton_tie_z", true);
    } else if (tie_status == MBNA_TIE_Z || tie_status == MBNA_TIE_Z_FIXED) {
      setEnabled("pushButton_tie_xyz", true);
      setEnabled("pushButton_tie_xy", true);
      setEnabled("pushButton_tie_z", false);
    } else if (tie_status == MBNA_TIE_XYZ || tie_status == MBNA_TIE_XYZ_FIXED) {
      setEnabled("pushButton_tie_xyz", false);
      setEnabled("pushButton_tie_xy", true);
      setEnabled("pushButton_tie_z", true);
    }
    const bool unfixed = tie_status == MBNA_TIE_XY || tie_status == MBNA_TIE_Z || tie_status == MBNA_TIE_XYZ;
    setEnabled("pushButton_tie_unfixed", !unfixed);
    setEnabled("pushButton_tie_fixed", unfixed);
  } else {
    setEnabled("pushButton_tie_xyz", false);
    setEnabled("pushButton_tie_xy", false);
    setEnabled("pushButton_tie_z", false);
    setEnabled("pushButton_tie_unfixed", false);
    setEnabled("pushButton_tie_fixed", false);
  }

  /* file menu */
  const bool idle = mbna_status == MBNA_STATUS_GUI;
  setEnabled("pushButton_new", idle && !project.open);
  setEnabled("pushButton_open", idle && !project.open);
  setEnabled("pushButton_close", idle && project.open);
  setEnabled("pushButton_importdata", idle && project.open && project.num_files >= 0);

  /* list and step buttons */
  if (project.open && project.num_files > 0) {
    setEnabled(kShowButtons, MBNA_LIST_KINDS, true);
    setEnabled(kNavSteps, kCount3, true);
    setEnabled(kViewToggles, kCount7, true);
    if (kind >= 0 && kind < MBNA_LIST_KINDS)
      setEnabled(kShowButtons[kind], false);
    if (kind == MBNA_VIEW_LIST_REFERENCEGRIDS || kind == MBNA_VIEW_LIST_SURVEYS || kind == MBNA_VIEW_LIST_BLOCKS ||
        kind == MBNA_VIEW_LIST_FILES) {
      setEnabled(kNavSteps, kCount3, false);
    } else if (kind == MBNA_VIEW_LIST_CROSSINGS || kind == MBNA_VIEW_LIST_MEDIOCRECROSSINGS ||
               kind == MBNA_VIEW_LIST_GOODCROSSINGS || kind == MBNA_VIEW_LIST_BETTERCROSSINGS) {
      if (project.num_crossings == project.num_crossings_analyzed)
        setEnabled("pushButton_naverr_nextunset", false);
    } else if (kind == MBNA_VIEW_LIST_TRUECROSSINGS) {
      if (project.num_truecrossings == project.num_truecrossings_analyzed)
        setEnabled("pushButton_naverr_nextunset", false);
    } else if (kind != MBNA_VIEW_LIST_FILESECTIONS) {
      setEnabled("pushButton_naverr_nextunset", false);
    }
  } else {
    /* the Motif code leaves the reference grid button out of this branch */
    setEnabled(kShowButtons + 1, MBNA_LIST_KINDS - 1, false);
    setEnabled(kNavSteps, kCount3, false);
    setEnabled(kViewToggles, kCount7, false);
  }

  /* actions */
  if (idle && project.open && project.num_files > 0) {
    setEnabled("pushButton_autopick", isCrossingList(kind));
    setEnabled("pushButton_autopickhorizontal", isCrossingList(kind));
    setEnabled("pushButton_autosetsvsvertical", true);
    setEnabled("pushButton_newcrossings", project.inversion_status == MBNA_INVERSION_CURRENT);
    setEnabled("pushButton_visualize", project.grid_status != MBNA_GRID_NONE && !project.visualization_status);
    setEnabled("pushButton_analyzecrossings", true);
    setEnabled("pushButton_zerozoffsets", true);
    setEnabled("pushButton_unsetskipped", true);
    setEnabled("pushButton_invertnav", project.num_truecrossings == project.num_truecrossings_analyzed ||
                                           project.num_crossings_analyzed >= 10);
    setEnabled("pushButton_updategrids", project.grid_status != MBNA_GRID_CURRENT);
    setEnabled("pushButton_applynav", project.inversion_status == MBNA_INVERSION_CURRENT);
  } else {
    static const char *const actions[] = {"pushButton_autopick",        "pushButton_autopickhorizontal",
                                          "pushButton_autosetsvsvertical", "pushButton_newcrossings",
                                          "pushButton_visualize",       "pushButton_analyzecrossings",
                                          "pushButton_zerozoffsets",    "pushButton_unsetskipped",
                                          "pushButton_invertnav",       "pushButton_updategrids",
                                          "pushButton_applynav"};
    setEnabled(actions, 11, false);
  }
  setEnabled("pushButton_showmodelplot", project.inversion_status != MBNA_INVERSION_NONE);
}

void Session::updateCrossing() {
  const bool crossing_mode = mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && mbna_current_crossing >= 0;
  const bool section_mode = mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && mbna_current_file >= 0 && mbna_current_section >= 0;

  if (crossing_mode || section_mode) {
    const double zoom_factor = (mbna_plot_lon_max - mbna_plot_lon_min) > 0.0
                                   ? 100.0 * MAX((mbna_lon_max - mbna_lon_min) / (mbna_plot_lon_max - mbna_plot_lon_min),
                                                 (mbna_lat_max - mbna_lat_min) / (mbna_plot_lat_max - mbna_plot_lat_min))
                                   : 0.0;
    const double plot_width = (mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon;
    const double misfit_width = (mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon;
    const std::string widths = fmt("Plot Widths (m): Contour: %.2f Misfit: %.2f Z: %.2f", plot_width, misfit_width, project.zoffsetwidth);
    const std::string zoom = fmt("Zoom Factor: %.2f ", zoom_factor);
    crossingLines_.clear();

    if (crossing_mode) {
      const double timediff = (project.files[mbna_file_id_2].sections[mbna_section_2].btime_d -
                               project.files[mbna_file_id_1].sections[mbna_section_1].btime_d) /
                              86400.0;
      struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
      struct mbna_tie *tie = &crossing->ties[mbna_current_tie];
      crossingLines_.push_back(fmt("Crossing: %d of %d", mbna_current_crossing, project.num_crossings));
      crossingLines_.push_back(fmt("Sections: %2.2d:%3.3d:%3.3d and %2.2d:%3.3d:%3.3d", project.files[crossing->file_id_1].survey,
                                   crossing->file_id_1, crossing->section_1, project.files[crossing->file_id_2].survey,
                                   crossing->file_id_2, crossing->section_2));
      crossingLines_.push_back(fmt("Time Difference: %f days ", timediff));
      if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
        crossingLines_.push_back("Status: Unset ");
        crossingLines_.push_back(widths);
        crossingLines_.push_back(zoom);
        crossingLines_.push_back("Relative Offsets:   None None None");
      } else if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        crossingLines_.push_back(fmt("Current Tie Point: %2d of %2d  Nav Points: %4d %4d", mbna_current_tie, crossing->num_ties,
                                     tie->snav_1, tie->snav_2));
        crossingLines_.push_back(widths);
        crossingLines_.push_back(zoom);
        crossingLines_.push_back(fmt("Relative Offsets (m):   %.3f %.3f %.3f", tie->offset_x_m, tie->offset_y_m, tie->offset_z_m));
        crossingLines_.push_back(fmt("Sigma (m):   %.3f %.3f %.3f", tie->sigmar1, tie->sigmar2, tie->sigmar3));
      } else {
        crossingLines_.push_back("Status: Skipped ");
        crossingLines_.push_back(widths);
        crossingLines_.push_back(zoom);
        crossingLines_.push_back("Relative Offsets:   Skipped Skipped Skipped");
      }
      setEnabled("pushButton_naverr_deletetie", mbna_current_tie >= 0);
      setEnabled("pushButton_naverr_selecttie", crossing->num_ties > 0);
    } else {
      struct mbna_file *file = &project.files[mbna_current_file];
      struct mbna_section *section = &file->sections[mbna_current_section];
      struct mbna_globaltie *globaltie = &section->globaltie;
      const std::string refgrid = refGridName(globaltie, "none");
      crossingLines_.push_back(fmt("Section: %2.2d:%3.3d:%3.3d", file->survey, mbna_current_file, mbna_current_section));
      if (section->status == MBNA_CROSSING_STATUS_NONE || section->status == MBNA_CROSSING_STATUS_SKIP) {
        crossingLines_.push_back(section->status == MBNA_CROSSING_STATUS_NONE ? "Global Tie Status: Unset "
                                                                              : "Global Tie Status: Unset (skipped) ");
        crossingLines_.push_back(fmt("Ref Grid: %s ", refgrid.c_str()));
        crossingLines_.push_back(widths);
        crossingLines_.push_back(zoom);
        crossingLines_.push_back("Relative Offsets:   None None None");
      } else {
        crossingLines_.push_back("Global Tie Status: Set ");
        crossingLines_.push_back(fmt("Ref Grid: %s ", refgrid.c_str()));
        crossingLines_.push_back(fmt("Nav Point: %4d", globaltie->snav));
        crossingLines_.push_back(widths);
        crossingLines_.push_back(zoom);
        crossingLines_.push_back(fmt("Relative Offsets (m):   %.3f %.3f %.3f", globaltie->offset_x_m, globaltie->offset_y_m,
                                     globaltie->offset_z_m));
        crossingLines_.push_back(fmt("Sigma (m):   %.3f %.3f %.3f", globaltie->sigmar1, globaltie->sigmar2, globaltie->sigmar3));
      }
      setEnabled("pushButton_naverr_deletetie", section->status == MBNA_CROSSING_STATUS_SET);
      setEnabled("pushButton_naverr_selecttie", false);
    }
    setEnabled("pushButton_naverr_fullsize", mbna_plot_lon_min != mbna_lon_min || mbna_plot_lon_max != mbna_lon_max ||
                                                 mbna_plot_lat_min != mbna_lat_min || mbna_plot_lat_max != mbna_lat_max);

    /* step buttons: the Motif code has no branch for some lists, which then keep their state */
    const int kind = mbna_view_list;
    if (kind == MBNA_VIEW_LIST_SURVEYS || kind == MBNA_VIEW_LIST_FILES) {
      setEnabled(kNavSteps, kCount3, false);
    } else if (section_mode && kind == MBNA_VIEW_LIST_FILESECTIONS) {
      setEnabled(kNavSteps, kCount3, true);
    } else if (kind == MBNA_VIEW_LIST_CROSSINGS || kind == MBNA_VIEW_LIST_GOODCROSSINGS || kind == MBNA_VIEW_LIST_TRUECROSSINGS ||
               (!motifCompatibility_ && (kind == MBNA_VIEW_LIST_MEDIOCRECROSSINGS || kind == MBNA_VIEW_LIST_BETTERCROSSINGS))) {
      setEnabled(kNavSteps, 2, true);
      bool all_done;
      if (section_mode && kind == MBNA_VIEW_LIST_CROSSINGS)
        all_done = project.num_globalties == project.num_globalties_analyzed;
      else if (kind == MBNA_VIEW_LIST_TRUECROSSINGS)
        all_done = project.num_truecrossings == project.num_truecrossings_analyzed;
      else
        all_done = project.num_crossings == project.num_crossings_analyzed;
      setEnabled("pushButton_naverr_nextunset", !all_done);
    } else if (kind == MBNA_VIEW_LIST_TIES || isSortedTieList(kind) || kind == MBNA_VIEW_LIST_GLOBALTIES ||
               kind == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      setEnabled(kNavSteps, 2, true);
      setEnabled("pushButton_naverr_nextunset", false);
    }
    updateCrossingOffsets();
  }

  if (callbacks_.crossingChanged)
    callbacks_.crossingChanged();
}

/* What the status line of the model plot says, and the mode that depends on the list. */
void Session::updateModelPlotStatus() {
  if (!project.modelplot)
    return;

  setEnabled("pushButton_modelplot_clearblock", mbna_crossing_select != MBNA_SELECT_NONE);

  if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES ||
      mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED)
    mbna_modelplot_mode = MBNA_MODELPLOT_MODE_SECTIONS;
  else
    mbna_modelplot_mode = MBNA_MODELPLOT_MODE_CROSSINGS;

  const bool timeline =
      project.modelplot_style == MBNA_MODELPLOT_TIMESERIES || project.modelplot_style == MBNA_MODELPLOT_PERTURBATION;
  const bool crossing_ok = mbna_crossing_select >= 0 && mbna_crossing_select < project.num_crossings;
  std::string mouse, selected = "No Selection";
  if (mbna_modelplot_mode == MBNA_MODELPLOT_MODE_SECTIONS && timeline) {
    const bool section_ok =
        mbna_section_select != MBNA_SELECT_NONE && mbna_current_file >= 0 && mbna_current_file < project.num_files;
    mouse = section_ok ? "Mouse: <left> select global tie; <middle> select untied section <right> drag zoom extent"
                       : "Mouse: <left> select global tie; <middle> select untied section; <right> drag zoom extent";
    if (section_ok)
      selected = fmt("Selected Section: %2.2d:%3.3d:%3.3d", project.files[mbna_current_file].survey, mbna_current_file,
                     mbna_current_section);
  } else {
    mouse = timeline && mbna_modelplot_mode == MBNA_MODELPLOT_MODE_CROSSINGS
                ? "Mouse: <left> select  tie; <middle> select untied crossing; <right> drag zoom extent"
                : "Mouse: <left> select  tie; <middle> select block to view; <right> drag zoom extent";
    if (crossing_ok) {
      const struct mbna_crossing *crossing = &(project.crossings[mbna_crossing_select]);
      selected = fmt("Selected Crossing: %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d", mbna_crossing_select,
                     project.files[crossing->file_id_1].survey, crossing->file_id_1, crossing->section_1,
                     project.files[crossing->file_id_2].survey, crossing->file_id_2, crossing->section_2);
    }
  }
  modelLines_.clear();
  modelLines_.push_back(mouse);
  modelLines_.push_back(selected);
  notifyPlots();
}

void Session::updateCrossingOffsets() {
  offsetLines_.clear();
  if ((mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && mbna_current_crossing >= 0) ||
      (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && mbna_current_file >= 0 && mbna_current_section >= 0)) {
    offsetLines_.push_back(fmt("Working Offsets (m): %.3f %.3f %.3f %d:%d", mbna_offset_x / mbna_mtodeglon,
                               mbna_offset_y / mbna_mtodeglat, mbna_offset_z, mbna_snav_1, mbna_snav_2));
    offsetLines_.push_back(fmt("Sigma (m): %.3f %.3f %.3f", mbna_minmisfit_sr1, mbna_minmisfit_sr2, mbna_minmisfit_sr3));
  } else {
    offsetLines_.push_back(fmt("Working Offsets (m): %.3f %.3f %.3f", 0.0, 0.0, 0.0));
    offsetLines_.push_back(fmt("Working Tie Points: %d:%d", 0, 0));
  }
  setEnabled("pushButton_naverr_settie", mbna_allow_set_tie);
  setEnabled("pushButton_naverr_resettie", mbna_allow_set_tie);
  setEnabled("pushButton_naverr_addtie", mbna_allow_add_tie);
}

/*--------------------------------------------------------------------*/
/* from the program logic */

void Session::addInfo(const char *info, bool timeTag) {
  if (timeTag && callbacks_.info)
    callbacks_.info(info);
  if (project.logfp != NULL)
    fputs(info, project.logfp);
  if (mbna_verbose > 0)
    fputs(info, stderr);

  if (timeTag) {
    int error = MB_ERROR_NO_ERROR;
    char user[256], host[256], date[32];
    mb_user_host_date(mbna_verbose, user, host, date, &error);
    const std::string tag = fmt(" > User <%s> on cpu <%s> at <%s>\n", user, host, date);
    if (callbacks_.info)
      callbacks_.info(tag);
    if (project.logfp != NULL)
      fputs(tag.c_str(), project.logfp);
    if (mbna_verbose > 0)
      fputs(tag.c_str(), stderr);
  }
}

void Session::notifyMessageOn(const char *message) {
  if (callbacks_.messageOn)
    callbacks_.messageOn(message);
}

void Session::notifyMessageUpdate(const char *message) {
  if (callbacks_.messageUpdate)
    callbacks_.messageUpdate(message);
}

void Session::notifyMessageOff() {
  if (callbacks_.messageOff)
    callbacks_.messageOff();
}

void Session::notifyError(const char *s1, const char *s2, const char *s3) {
  if (callbacks_.error)
    callbacks_.error(s1, s2, s3);
}

/* When a step finds no other crossing the program logic clears the current
   one, and its plot function would then read project.crossings[-1]. */
void Session::plotCrossing(int plotMode) {
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && mbna_current_crossing < 0)
    return;
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && (mbna_current_file < 0 || mbna_current_section < 0))
    return;
  mbnavadjust_naverr_plot(plotMode);
  notifyPlots();
}

void Session::notifyPlots() {
  if (plotsChanged_)
    plotsChanged_();
}

void Session::initCrossingWindow(int mode) {
  mbnavadjust_set_graphics(contourSurface_, misfitSurface_, zOffsetSurface_);
  mbna_status = MBNA_STATUS_NAVERR;
  if (mode == MBNA_NAVERR_MODE_CROSSING) {
    if (mbna_crossing_select == MBNA_SELECT_NONE)
      mbnavadjust_naverr_nextunset_crossing();
    else
      mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
  } else if (mode == MBNA_NAVERR_MODE_SECTION) {
    if (mbna_section_select == MBNA_SELECT_NONE)
      mbnavadjust_naverr_nextunset_section();
    else
      mbnavadjust_naverr_specific_section(mbna_file_select, mbna_section_select);
  }
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  updateStatus();
}

/*--------------------------------------------------------------------*/
/* project */

void Session::resetSelections() {
  mbna_view_mode = MBNA_VIEW_MODE_ALL;
  mbna_file_id_1 = MBNA_SELECT_NONE;
  mbna_section_1 = MBNA_SELECT_NONE;
  mbna_file_id_2 = MBNA_SELECT_NONE;
  mbna_section_2 = MBNA_SELECT_NONE;
  mbna_current_crossing = MBNA_SELECT_NONE;
  mbna_current_tie = MBNA_SELECT_NONE;
  mbna_current_file = MBNA_SELECT_NONE;
  mbna_current_section = MBNA_SELECT_NONE;
  mbna_file_select = MBNA_SELECT_NONE;
  mbna_survey_select = MBNA_SELECT_NONE;
  mbna_survey_select1 = MBNA_SELECT_NONE;
  mbna_survey_select2 = MBNA_SELECT_NONE;
  mbna_section_select = MBNA_SELECT_NONE;
  mbna_crossing_select = MBNA_SELECT_NONE;
  mbna_tie_select = MBNA_SELECT_NONE;
  mbna_modelplot_pickfile = MBNA_SELECT_NONE;
  mbna_modelplot_picksection = MBNA_SELECT_NONE;
  mbna_modelplot_picksnav = MBNA_SELECT_NONE;
  mbna_modelplot_zoom = false;
  mbna_modelplot_zoom_x1 = 0;
  mbna_modelplot_zoom_x2 = 0;
  mbna_modelplot_tiezoom = false;
  mbna_modelplot_tiestart = 0;
  mbna_modelplot_tieend = 0;
  mbna_modelplot_tiestartzoom = 0;
  mbna_modelplot_tieendzoom = 0;
}

bool Session::newProject(const std::string &path) {
  notifyMessageOn(fmt("Creating new MBnavadjust project %s", path.c_str()).c_str());
  char name[MB_PATH_MAXLINE];
  snprintf(name, sizeof(name), "%s", path.c_str());
  const int status = mbnavadjust_file_new(name);
  if (status == MB_SUCCESS)
    resetSelections();
  notifyMessageOff();
  updateStatus();
  return status == MB_SUCCESS;
}

bool Session::openProject(const std::string &path) {
  undoSteps_.clear();
  redoSteps_.clear();
  notifyMessageOn(fmt("Opening MBnavadjust project %s", path.c_str()).c_str());
  char name[MB_PATH_MAXLINE];
  snprintf(name, sizeof(name), "%s", path.c_str());
  const int status = mbnavadjust_file_open(name);
  if (status == MB_SUCCESS)
    resetSelections();
  notifyMessageOff();
  updateStatus();
  return status == MB_SUCCESS;
}

bool Session::importData(const std::string &path, int format, bool singleSurvey) {
  int error = MB_ERROR_NO_ERROR;
  char name[MB_PATH_MAXLINE];
  snprintf(name, sizeof(name), "%s", path.c_str());
  notifyMessageOn(fmt(singleSurvey ? "Importing single survey from %s %d" : "Importing data from %s %d", path.c_str(), format).c_str());
  const int status = mbnavadjust_import_data(mbna_verbose, &project, name, format, singleSurvey, &error);
  notifyMessageOff();

  mbna_status = MBNA_STATUS_NAVSOLVE;
  notifyMessageOn("Updating bathymetry grids...");
  mbnavadjust_updategrid(mbna_verbose, &project);
  notifyMessageOff();
  mbna_status = MBNA_STATUS_GUI;
  refreshAfterEdit(false);
  return status == MB_SUCCESS;
}

bool Session::importReference(const std::string &path) {
  int error = MB_ERROR_NO_ERROR;
  char name[MB_PATH_MAXLINE];
  snprintf(name, sizeof(name), "%s", path.c_str());
  const int status = mbnavadjust_import_reference(mbna_verbose, &project, name, &error);
  if (status == MB_SUCCESS) {
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
    project.save_count = 0;
  }
  updateStatus();
  return status == MB_SUCCESS;
}

void Session::closeProject(bool writeOutstanding) {
  undoSteps_.clear();
  redoSteps_.clear();
  int error = MB_ERROR_NO_ERROR;
  if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED)
    dismissCrossingWindow();
  if (writeOutstanding && project.open && project.save_count != 0) {
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
    project.save_count = 0;
  }
  mbnavadjust_close_project(mbna_verbose, &project, &error);
  resetSelections();
  refreshAfterEdit(false);
}

std::string Session::versionText() const { return fmt("%s %s", MB_VERSION, MB_VERSION_DATE); }

std::string Session::projectName() const { return project.open ? std::string(project.name) : std::string(); }

bool Session::writeProject() {
  int error = MB_ERROR_NO_ERROR;
  const int status = mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
  project.save_count = 0;
  return status == MB_SUCCESS;
}

/* What the Motif callbacks do after a change: redraw the status and list, then the open views. */
void Session::refreshAfterEdit(bool modelOutOfDate) {
  updateStatus();
  if (project.modelplot) {
    if (modelOutOfDate)
      project.modelplot_uptodate = false;
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
    notifyPlots();
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/
/* main window list */

void Session::showList(ListKind kind) {
  if (mbna_view_list != kind)
    project.modelplot_uptodate = false;
  mbna_view_list = kind;
  refreshAfterEdit(false);
}

bool Session::setViewMode(ViewMode mode) {
  if ((mode == ViewFile || mode == ViewWithFile) && mbna_file_select == MBNA_SELECT_NONE)
    return false;
  if (mode == ViewWithSection && (mbna_file_select == MBNA_SELECT_NONE || mbna_section_select == MBNA_SELECT_NONE))
    return false;
  if (mbna_view_mode != mode)
    project.modelplot_uptodate = false;
  mbna_view_mode = mode;
  refreshAfterEdit(false);
  return true;
}

void Session::loadCrossingRow(int crossing, int tie) {
  mbna_crossing_select = crossing;
  mbna_tie_select = tie;
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
    initCrossingWindow(MBNA_NAVERR_MODE_CROSSING);
  } else {
    mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
    plotCrossing(MBNA_PLOT_MODE_FIRST);
    updateCrossing();
  }
}

void Session::loadSectionRow(int file, int section) {
  mbna_file_select = file;
  mbna_section_select = section;
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
    initCrossingWindow(MBNA_NAVERR_MODE_SECTION);
  } else {
    mbnavadjust_naverr_specific_section(mbna_file_select, mbna_section_select);
    plotCrossing(MBNA_PLOT_MODE_FIRST);
    updateCrossing();
  }
}

void Session::clickListRow(int row) {
  if (row < 0 || row >= static_cast<int>(rowRefs_.size()))
    return;
  const RowRef ref = rowRefs_[row];
  const int kind = mbna_view_list;

  if (kind == MBNA_VIEW_LIST_REFERENCEGRIDS) {
    project.refgrid_select = row - 1;
  } else if (kind == MBNA_VIEW_LIST_SURVEYS) {
    mbna_section_select = 0;
    mbna_file_select = MBNA_SELECT_NONE;
    mbna_survey_select = row;
    for (int i = 0; i < project.num_files; i++) {
      if (mbna_file_select == MBNA_SELECT_NONE && mbna_survey_select == project.files[i].survey) {
        mbna_file_select = i;
        mbna_section_select = 0;
      }
    }
    project.modelplot_uptodate = false;
  } else if (kind == MBNA_VIEW_LIST_BLOCKS) {
    mbna_survey_select = ref.a;
    mbna_survey_select1 = ref.b;
    mbna_survey_select2 = ref.c;
    project.modelplot_uptodate = false;
  } else if (kind == MBNA_VIEW_LIST_FILES) {
    mbna_section_select = 0;
    mbna_file_select = ref.a;
    mbna_survey_select = ref.b;
    project.modelplot_uptodate = false;
  } else if (kind == MBNA_VIEW_LIST_FILESECTIONS) {
    mbna_survey_select = ref.c;
    project.modelplot_uptodate = false;
    loadSectionRow(ref.a, ref.b);
  } else if (isCrossingList(kind)) {
    project.modelplot_uptodate = false;
    loadCrossingRow(ref.a, 0);
  } else if (kind == MBNA_VIEW_LIST_TIES || isSortedTieList(kind)) {
    loadCrossingRow(ref.a, ref.b);
    project.modelplot_uptodate = false;
  } else if (kind == MBNA_VIEW_LIST_GLOBALTIES || kind == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
    loadSectionRow(ref.a, ref.b);
    project.modelplot_uptodate = false;
  }
  refreshAfterEdit(false);
}

void Session::clickSelectedRow() {
  const int kind = mbna_view_list;
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
    if (kind == MBNA_VIEW_LIST_FILESECTIONS || kind == MBNA_VIEW_LIST_GLOBALTIES || kind == MBNA_VIEW_LIST_GLOBALTIESSORTED)
      initCrossingWindow(MBNA_NAVERR_MODE_SECTION);
    else if (kind == MBNA_VIEW_LIST_CROSSINGS || kind == MBNA_VIEW_LIST_GOODCROSSINGS ||
             kind == MBNA_VIEW_LIST_BETTERCROSSINGS || kind == MBNA_VIEW_LIST_TRUECROSSINGS ||
             kind == MBNA_VIEW_LIST_TIES || isSortedTieList(kind))
      initCrossingWindow(MBNA_NAVERR_MODE_CROSSING);
  }
  refreshAfterEdit(false);
}

/*--------------------------------------------------------------------*/
/* crossing window */

void Session::openCrossingWindow(CrossingWindowMode mode) {
  initCrossingWindow(mode == CrossingWindowMode::Crossing ? MBNA_NAVERR_MODE_CROSSING : MBNA_NAVERR_MODE_SECTION);
}

void Session::nextCrossing() {
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING)
    mbnavadjust_naverr_next_crossing();
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION)
    mbnavadjust_naverr_next_section();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(false);
}

void Session::previousCrossing() {
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING)
    mbnavadjust_naverr_previous_crossing();
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION)
    mbnavadjust_naverr_previous_section();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(false);
}

void Session::nextUnsetCrossing() {
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING)
    mbnavadjust_naverr_nextunset_crossing();
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION)
    mbnavadjust_naverr_nextunset_section();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(false);
}

void Session::addTie() {
  pushUndo();
  mbnavadjust_naverr_addtie();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(true);
}

void Session::deleteTie() {
  pushUndo();
  mbnavadjust_naverr_deletetie();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(true);
}

void Session::selectTie() {
  mbnavadjust_naverr_selecttie();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(true);
}

void Session::setTie() {
  pushUndo();
  mbnavadjust_naverr_save();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(true);
}

void Session::resetTie() {
  mbnavadjust_naverr_resettie();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(false);
}

void Session::unsetCrossing() {
  pushUndo();
  mbnavadjust_naverr_unset();
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING)
    mbnavadjust_naverr_next_crossing();
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION)
    mbnavadjust_naverr_next_section();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(true);
}

void Session::skipCrossing() {
  pushUndo();
  mbnavadjust_naverr_skip();
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING)
    mbnavadjust_naverr_nextunset_crossing();
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION)
    mbnavadjust_naverr_nextunset_section();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(true);
}

void Session::dismissCrossingWindow() {
  if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    mbnavadjust_crossing_unload();
    mbnavadjust_referencegrid_unload();
  }
  mbna_status = MBNA_STATUS_GUI;
  mbna_current_crossing = MBV_SELECT_NONE;
  mbna_current_tie = MBV_SELECT_NONE;
  mbna_current_file = MBV_SELECT_NONE;
  mbna_current_section = MBV_SELECT_NONE;
  mbnavadjust_naverr_checkoksettie();
  updateCrossing();
  refreshAfterEdit(false);
  notifyPlots();
}

void Session::fullSize() {
  mbna_plot_lon_min = mbna_lon_min;
  mbna_plot_lon_max = mbna_lon_max;
  mbna_plot_lat_min = mbna_lat_min;
  mbna_plot_lat_max = mbna_lat_max;
  mbnavadjust_get_misfit();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
}

void Session::zeroOffset() {
  mbna_offset_x = 0.0;
  mbna_offset_y = 0.0;
  mbna_offset_z = 0.0;
  mbnavadjust_get_misfitxy();
  mbnavadjust_naverr_replot();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossingOffsets();
}

void Session::zeroZOffset() {
  mbna_offset_z = 0.0;
  mbnavadjust_get_misfitxy();
  mbnavadjust_naverr_replot();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossingOffsets();
}

void Session::applyZOffset() {
  mbnavadjust_get_misfitxy();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossingOffsets();
}

void Session::moveToMinimumMisfit() {
  mbna_offset_x = mbna_minmisfit_x;
  mbna_offset_y = mbna_minmisfit_y;
  mbna_offset_z = mbna_minmisfit_z;
  mbnavadjust_get_misfitxy();
  mbnavadjust_naverr_replot();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
}

void Session::moveToMinimumXYMisfit() {
  mbna_offset_x = mbna_minmisfit_xh;
  mbna_offset_y = mbna_minmisfit_yh;
  mbna_offset_z = mbna_minmisfit_zh;
  mbnavadjust_naverr_replot();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
}

void Session::setMisfitCenter(bool zeroCenter) {
  mbna_misfit_center = zeroCenter ? MBNA_MISFIT_ZEROCENTER : MBNA_MISFIT_AUTOCENTER;
  mbnavadjust_get_misfit();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
}

/*--------------------------------------------------------------------*/
/* forms and undo */

void Session::reloadCrossingWindow() {
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && mbna_current_crossing >= 0)
    mbnavadjust_naverr_specific_crossing(mbna_current_crossing, mbna_current_tie);
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && mbna_current_file >= 0 && mbna_current_section >= 0)
    mbnavadjust_naverr_specific_section(mbna_current_file, mbna_current_section);
  else
    return;
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  updateStatus();
}

ControlValues Session::controlValues() const {
  ControlValues v;
  v.decimation = project.decimation;
  v.sectionLength = project.section_length;
  v.sectionSoundings = project.section_soundings;
  v.contourInterval = project.cont_int;
  v.colorInterval = project.col_int;
  v.tickInterval = project.tick_int;
  v.smoothing = project.smoothing;
  v.zOffsetWidth = project.zoffsetwidth;
  v.useMode = project.use_mode == MBNA_USE_MODE_SECONDARY ? 1 : (project.use_mode == MBNA_USE_MODE_TERTIARY ? 2 : 0);
  return v;
}

void Session::applyControls(const ControlValues &values) {
  int error = MB_ERROR_NO_ERROR;
  project.decimation = values.decimation;
  project.section_length = values.sectionLength;
  project.section_soundings = values.sectionSoundings;
  project.cont_int = values.contourInterval;
  project.col_int = values.colorInterval;
  project.tick_int = values.tickInterval;
  project.smoothing = values.smoothing;
  project.zoffsetwidth = values.zOffsetWidth;
  project.use_mode = values.useMode == 1 ? MBNA_USE_MODE_SECONDARY
                                         : (values.useMode == 2 ? MBNA_USE_MODE_TERTIARY : MBNA_USE_MODE_PRIMARY);
  if (mbna_file_id_1 >= 0 && mbna_section_1 >= 0)
    project.files[mbna_file_id_1].sections[mbna_section_1].contoursuptodate = false;
  if (mbna_file_id_2 >= 0 && mbna_section_2 >= 0)
    project.files[mbna_file_id_2].sections[mbna_section_2].contoursuptodate = false;
  if (crossingLoaded())
    mbnavadjust_naverr_replot();
  mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
  if (crossingLoaded()) {
    mbnavadjust_get_misfit();
    plotCrossing(MBNA_PLOT_MODE_FIRST);
  }
  updateStatus();
  updateCrossing();
}

BiasValues Session::biasValues() const {
  BiasValues v = {false, -1, -1, -1, -1, 0.0, 0.0, 0.0, 0.0};
  if (mbna_file_id_1 < 0 || mbna_file_id_1 >= project.num_files || mbna_file_id_2 < 0 || mbna_file_id_2 >= project.num_files)
    return v;
  v.available = true;
  v.file1 = mbna_file_id_1;
  v.section1 = mbna_section_1;
  v.file2 = mbna_file_id_2;
  v.section2 = mbna_section_2;
  v.heading1 = project.files[mbna_file_id_1].heading_bias;
  v.roll1 = project.files[mbna_file_id_1].roll_bias;
  v.heading2 = project.files[mbna_file_id_2].heading_bias;
  v.roll2 = project.files[mbna_file_id_2].roll_bias;
  return v;
}

void Session::applyBiases(double heading1, double roll1, double heading2, double roll2) {
  if (mbna_file_id_1 < 0 || mbna_file_id_1 >= project.num_files || mbna_file_id_2 < 0 || mbna_file_id_2 >= project.num_files)
    return;
  struct mbna_file *file1 = &(project.files[mbna_file_id_1]);
  struct mbna_file *file2 = &(project.files[mbna_file_id_2]);
  file1->heading_bias = heading1;
  file1->roll_bias = roll1;
  file2->heading_bias = heading2;
  file2->roll_bias = roll2;
  for (int i = 0; i < file1->num_sections; i++)
    file1->sections[i].contoursuptodate = false;
  for (int i = 0; i < file2->num_sections; i++)
    file2->sections[i].contoursuptodate = false;
  if (crossingLoaded()) {
    mbnavadjust_naverr_replot();
    mbnavadjust_get_misfit();
    plotCrossing(MBNA_PLOT_MODE_FIRST);
  }
  updateStatus();
}

void Session::applyBiasesToAllFiles(double heading, double roll) {
  for (int ifile = 0; ifile < project.num_files; ifile++) {
    struct mbna_file *file = &(project.files[ifile]);
    file->heading_bias = heading;
    file->roll_bias = roll;
    for (int i = 0; i < file->num_sections; i++)
      file->sections[i].contoursuptodate = false;
  }
  if (crossingLoaded()) {
    mbnavadjust_naverr_replot();
    mbnavadjust_get_misfit();
    plotCrossing(MBNA_PLOT_MODE_FIRST);
  }
  refreshAfterEdit(false);
}

void Session::takeStep(UndoStep &step) const {
  const int crossing = mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && mbna_current_crossing >= 0 ? mbna_current_crossing
                                                                                                   : mbna_crossing_select;
  const auto addSection = [&](int file, int section) {
    if (file < 0 || file >= project.num_files || section < 0 || section >= project.files[file].num_sections)
      return;
    for (const auto &saved : step.sections) {
      if (saved.first.first == file && saved.first.second == section)
        return;
    }
    step.sections.push_back(std::make_pair(std::make_pair(file, section), project.files[file].sections[section]));
  };
  if (crossing >= 0 && crossing < project.num_crossings) {
    step.crossingIndex = crossing;
    step.crossing = project.crossings[crossing];
    addSection(step.crossing.file_id_1, step.crossing.section_1);
    addSection(step.crossing.file_id_2, step.crossing.section_2);
  }
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION)
    addSection(mbna_current_file, mbna_current_section);
  addSection(mbna_file_select, mbna_section_select);
  step.fileStatus.resize(project.num_files);
  for (int i = 0; i < project.num_files; i++)
    step.fileStatus[i] = project.files[i].status;
  step.numTies = project.num_ties;
  step.numGlobalTies = project.num_globalties;
  step.numCrossingsAnalyzed = project.num_crossings_analyzed;
  step.numTrueCrossingsAnalyzed = project.num_truecrossings_analyzed;
  step.numGlobalTiesAnalyzed = project.num_globalties_analyzed;
  step.inversionStatus = project.inversion_status;
  step.gridStatus = project.grid_status;
  step.crossingSelect = mbna_crossing_select;
  step.tieSelect = mbna_tie_select;
  step.fileSelect = mbna_file_select;
  step.sectionSelect = mbna_section_select;
  step.surveySelect = mbna_survey_select;
  step.naverrMode = mbna_naverr_mode;
  step.currentCrossing = mbna_current_crossing;
  step.currentTie = mbna_current_tie;
  step.currentFile = mbna_current_file;
  step.currentSection = mbna_current_section;
}

void Session::applyStep(UndoStep &step) {
  if (step.crossingIndex >= 0 && step.crossingIndex < project.num_crossings)
    project.crossings[step.crossingIndex] = step.crossing;
  for (const auto &saved : step.sections)
    project.files[saved.first.first].sections[saved.first.second] = saved.second;
  for (int i = 0; i < project.num_files && i < static_cast<int>(step.fileStatus.size()); i++)
    project.files[i].status = step.fileStatus[i];
  project.num_ties = step.numTies;
  project.num_globalties = step.numGlobalTies;
  project.num_crossings_analyzed = step.numCrossingsAnalyzed;
  project.num_truecrossings_analyzed = step.numTrueCrossingsAnalyzed;
  project.num_globalties_analyzed = step.numGlobalTiesAnalyzed;
  project.inversion_status = step.inversionStatus;
  project.grid_status = step.gridStatus;
  project.modelplot_uptodate = false;
  project.save_count++;
  if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
    int error = MB_ERROR_NO_ERROR;
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
    project.save_count = 0;
  }
  mbna_crossing_select = step.crossingSelect;
  mbna_tie_select = step.tieSelect;
  mbna_file_select = step.fileSelect;
  mbna_section_select = step.sectionSelect;
  mbna_survey_select = step.surveySelect;
  if (step.naverrMode == MBNA_NAVERR_MODE_CROSSING && step.currentCrossing >= 0) {
    if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
      mbna_crossing_select = step.currentCrossing;
      mbna_tie_select = step.currentTie;
      initCrossingWindow(MBNA_NAVERR_MODE_CROSSING);
    } else {
      mbnavadjust_naverr_specific_crossing(step.currentCrossing, step.currentTie);
    }
  } else if (step.naverrMode == MBNA_NAVERR_MODE_SECTION && step.currentFile >= 0 && step.currentSection >= 0) {
    if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED)
      initCrossingWindow(MBNA_NAVERR_MODE_SECTION);
    else
      mbnavadjust_naverr_specific_section(step.currentFile, step.currentSection);
  }
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  refreshAfterEdit(true);
}

void Session::pushUndo() {
  UndoStep step;
  takeStep(step);
  undoSteps_.push_back(std::move(step));
  if (undoSteps_.size() > kUndoLimit)
    undoSteps_.erase(undoSteps_.begin());
  redoSteps_.clear();
}

void Session::undo() {
  if (undoSteps_.empty())
    return;
  UndoStep current;
  takeStep(current);
  UndoStep step = std::move(undoSteps_.back());
  undoSteps_.pop_back();
  redoSteps_.push_back(std::move(current));
  applyStep(step);
}

void Session::redo() {
  if (redoSteps_.empty())
    return;
  UndoStep current;
  takeStep(current);
  UndoStep step = std::move(redoSteps_.back());
  redoSteps_.pop_back();
  undoSteps_.push_back(std::move(current));
  applyStep(step);
}

/*--------------------------------------------------------------------*/
/* plots */

void Session::setCrossingSurfaces(DrawSurface *contour, DrawSurface *misfit, DrawSurface *zOffset) {
  contourSurface_ = contour != nullptr ? contour : &null_cont;
  misfitSurface_ = misfit != nullptr ? misfit : &null_corr;
  zOffsetSurface_ = zOffset != nullptr ? zOffset : &null_zoff;
  mbnavadjust_set_graphics(contourSurface_, misfitSurface_, zOffsetSurface_);
}

bool Session::crossingLoaded() const {
  return (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && mbna_current_crossing >= 0) ||
         (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && mbna_current_file >= 0 && mbna_current_section >= 0);
}

void Session::replotCrossing() {
  if (crossingLoaded()) {
    mbnavadjust_get_misfit();
    plotCrossing(MBNA_PLOT_MODE_FIRST);
  }
}

void Session::setCrossingPlotSizes(int contourWidth, int contourHeight, int misfitWidth, int misfitHeight, int zOffsetWidth,
                                   int zOffsetHeight) {
  /* The scaling widens the bounds to fit the shape of the plot, so a resize
     starts again from the bounds that were asked for unless they have changed. */
  if (crossingLoaded()) {
    const bool unchanged = haveRequestedBounds_ && mbna_plot_lon_min == scaledBounds_[0] &&
                           mbna_plot_lon_max == scaledBounds_[1] && mbna_plot_lat_min == scaledBounds_[2] &&
                           mbna_plot_lat_max == scaledBounds_[3];
    if (unchanged) {
      mbna_plot_lon_min = requestedBounds_[0];
      mbna_plot_lon_max = requestedBounds_[1];
      mbna_plot_lat_min = requestedBounds_[2];
      mbna_plot_lat_max = requestedBounds_[3];
    } else {
      requestedBounds_[0] = mbna_plot_lon_min;
      requestedBounds_[1] = mbna_plot_lon_max;
      requestedBounds_[2] = mbna_plot_lat_min;
      requestedBounds_[3] = mbna_plot_lat_max;
      haveRequestedBounds_ = true;
    }
  }

  int cont[4] = {0, contourWidth, 0, contourHeight};
  int corr[4] = {0, misfitWidth, 0, misfitHeight};
  int zoff[4] = {0, zOffsetWidth, 0, zOffsetHeight};
  mbnavadjust_set_borders(cont, corr, zoff);
  if (crossingLoaded()) {
    plotCrossing(MBNA_PLOT_MODE_FIRST);
    scaledBounds_[0] = mbna_plot_lon_min;
    scaledBounds_[1] = mbna_plot_lon_max;
    scaledBounds_[2] = mbna_plot_lat_min;
    scaledBounds_[3] = mbna_plot_lat_max;
  }
}

void Session::contourPress(int button, int x, int y) {
  if (!crossingLoaded())
    return;
  if (button == 1) {
    button1Down_ = true;
    locX_ = x;
    locY_ = y;
    mbna_offset_x_old = mbna_offset_x;
    mbna_offset_y_old = mbna_offset_y;
    mbna_offset_z_old = mbna_offset_z;
    mbnavadjust_naverr_checkoksettie();
    updateCrossingOffsets();
  } else if (button == 2) {
    button2Down_ = true;
    mbna_zoom_x1 = x;
    mbna_zoom_y1 = y;
    mbna_zoom_x2 = x;
    mbna_zoom_y2 = y;
    plotCrossing(MBNA_PLOT_MODE_ZOOMFIRST);
  } else if (button == 3) {
    button3Down_ = true;
    mbnavadjust_naverr_snavpoints(x, y);
    plotCrossing(MBNA_PLOT_MODE_FIRST);
    updateCrossing();
  }
}

void Session::contourDrag(int x, int y) {
  if (!crossingLoaded())
    return;
  if (button1Down_) {
    mbna_offset_x = mbna_offset_x_old + (x - locX_) / mbna_plotx_scale;
    mbna_offset_y = mbna_offset_y_old - (y - locY_) / mbna_ploty_scale;
    plotCrossing(MBNA_PLOT_MODE_MOVE);
    updateCrossingOffsets();
    locX_ = x;
    locY_ = y;
    mbna_offset_x_old = mbna_offset_x;
    mbna_offset_y_old = mbna_offset_y;
  } else if (button2Down_) {
    mbna_zoom_x2 = x;
    mbna_zoom_y2 = y;
    plotCrossing(MBNA_PLOT_MODE_ZOOM);
  }
}

void Session::contourRelease(int button, int x, int y) {
  if (button == 1) {
    button1Down_ = false;
    notifyPlots();
  } else if (button == 2) {
    button2Down_ = false;
    if (!crossingLoaded())
      return;
    mbna_zoom_x2 = x;
    mbna_zoom_y2 = y;
    /* a click without a drag would make an empty plot */
    if (mbna_zoom_x1 != mbna_zoom_x2 && mbna_zoom_y1 != mbna_zoom_y2) {
      const double x1 = mbna_zoom_x1 / mbna_plotx_scale + mbna_plot_lon_min;
      const double y1 = (cont_borders[3] - mbna_zoom_y1) / mbna_ploty_scale + mbna_plot_lat_min;
      const double x2 = mbna_zoom_x2 / mbna_plotx_scale + mbna_plot_lon_min;
      const double y2 = (cont_borders[3] - mbna_zoom_y2) / mbna_ploty_scale + mbna_plot_lat_min;
      mbna_plot_lon_min = MIN(x1, x2);
      mbna_plot_lon_max = MAX(x1, x2);
      mbna_plot_lat_min = MIN(y1, y2);
      mbna_plot_lat_max = MAX(y1, y2);
      mbnavadjust_get_misfit();
    }
    plotCrossing(MBNA_PLOT_MODE_FIRST);
    updateCrossing();
  } else if (button == 3) {
    button3Down_ = false;
  }
}

void Session::misfitPress(int x, int y) {
  if (!crossingLoaded())
    return;
  button1Down_ = true;
  mbna_offset_x_old = mbna_offset_x;
  mbna_offset_y_old = mbna_offset_y;
  mbna_offset_z_old = mbna_offset_z;
  mbna_offset_x = mbna_misfit_offset_x + (x - (corr_borders[0] + corr_borders[1]) / 2) / mbna_misfit_xscale;
  mbna_offset_y = mbna_misfit_offset_y - (y - (corr_borders[3] + corr_borders[2]) / 2) / mbna_misfit_yscale;
  plotCrossing(MBNA_PLOT_MODE_MOVE);
  updateCrossing();
  updateCrossingOffsets();
}

void Session::misfitDrag(int x, int y) {
  if (!crossingLoaded() || !button1Down_)
    return;
  mbna_offset_x = mbna_misfit_offset_x + (x - (corr_borders[0] + corr_borders[1]) / 2) / mbna_misfit_xscale;
  mbna_offset_y = mbna_misfit_offset_y - (y - (corr_borders[3] + corr_borders[2]) / 2) / mbna_misfit_yscale;
  plotCrossing(MBNA_PLOT_MODE_MOVE);
  updateCrossing();
  updateCrossingOffsets();
  mbna_offset_x_old = mbna_offset_x;
  mbna_offset_y_old = mbna_offset_y;
  mbna_offset_z_old = mbna_offset_z;
}

void Session::misfitRelease() {
  button1Down_ = false;
  notifyPlots();
}

void Session::zOffsetPress(int x, int y) {
  (void)y;
  if (!crossingLoaded())
    return;
  button1Down_ = true;
  mbna_offset_x_old = mbna_offset_x;
  mbna_offset_y_old = mbna_offset_y;
  mbna_offset_z_old = mbna_offset_z;
  mbna_offset_z = (x / mbna_zoff_scale_x) + mbna_misfit_offset_z - 0.5 * project.zoffsetwidth;
  mbnavadjust_get_misfitxy();
  plotCrossing(MBNA_PLOT_MODE_MOVE);
  updateCrossing();
  updateCrossingOffsets();
}

void Session::zOffsetDrag(int x, int y) {
  (void)y;
  if (!crossingLoaded() || !button1Down_)
    return;
  mbna_offset_z = (x / mbna_zoff_scale_x) + mbna_misfit_offset_z - 0.5 * project.zoffsetwidth;
  mbnavadjust_get_misfitxy();
  plotCrossing(MBNA_PLOT_MODE_MOVE);
  updateCrossing();
  updateCrossingOffsets();
  mbna_offset_x_old = mbna_offset_x;
  mbna_offset_y_old = mbna_offset_y;
  mbna_offset_z_old = mbna_offset_z;
}

void Session::zOffsetRelease() {
  button1Down_ = false;
  if (!crossingLoaded())
    return;
  mbnavadjust_naverr_replot();
  plotCrossing(MBNA_PLOT_MODE_FIRST);
  updateCrossing();
  updateCrossingOffsets();
}

/*--------------------------------------------------------------------*/
/* model plot */

bool Session::modelPlotOpen() const { return project.modelplot; }

void Session::setModelPlotBorders(int width, int height) {
  mbna_modelplot_width = width;
  mbna_modelplot_height = height;
  modelBorders_[0] = 0;
  modelBorders_[1] = width - 1;
  modelBorders_[2] = 0;
  modelBorders_[3] = height - 1;
  mbnavadjust_set_modelplot_graphics(modelSurface_, modelBorders_);
}

void Session::openModelPlot(DrawSurface *surface, int width, int height) {
  modelSurface_ = surface;
  setModelPlotBorders(width, height);
  project.modelplot = true;
  project.modelplot_uptodate = false;
  mbna_modelplot_zoom = false;
  mbna_modelplot_zoom_x1 = 0;
  mbna_modelplot_zoom_x2 = 0;
  mbna_modelplot_start = 0;
  mbna_modelplot_end = 0;
  updateStatus();
  updateModelPlotStatus();
  mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  notifyPlots();
}

void Session::closeModelPlot() {
  project.modelplot = false;
  notifyPlots();
}

void Session::resizeModelPlot(int width, int height) {
  if (!project.modelplot || (width == mbna_modelplot_width && height == mbna_modelplot_height))
    return;
  setModelPlotBorders(width, height);
  project.modelplot_uptodate = false;
  mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  notifyPlots();
}

void Session::replotModelPlot() {
  if (!project.modelplot)
    return;
  project.modelplot_uptodate = false;
  mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  notifyPlots();
}

void Session::setModelPlotStyle(ModelPlotStyle style) {
  project.modelplot_style = style == ModelPlotStyle::TimeSeries
                                ? MBNA_MODELPLOT_TIMESERIES
                                : (style == ModelPlotStyle::Perturbation ? MBNA_MODELPLOT_PERTURBATION : MBNA_MODELPLOT_TIEOFFSETS);
  project.modelplot_uptodate = false;
  updateModelPlotStatus();
  mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  notifyPlots();
}

void Session::modelPlotFullSize() {
  mbna_modelplot_zoom_x1 = 0;
  mbna_modelplot_zoom_x2 = 0;
  if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES || project.modelplot_style == MBNA_MODELPLOT_PERTURBATION) {
    mbna_modelplot_zoom = false;
    mbna_modelplot_start = 0;
    mbna_modelplot_end = 0;
  } else {
    mbna_modelplot_tiezoom = false;
    mbna_modelplot_tiestartzoom = 0;
    mbna_modelplot_tieendzoom = 0;
    mbna_survey_select = MBNA_SELECT_NONE;
    mbna_survey_select1 = MBNA_SELECT_NONE;
    mbna_survey_select2 = MBNA_SELECT_NONE;
  }
  project.modelplot_uptodate = false;
  mbnavadjust_modelplot_setzoom();
  mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  updateModelPlotStatus();
  notifyPlots();
}

void Session::modelPlotClearBlock() {
  mbnavadjust_modelplot_clearblock();
  if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    plotCrossing(MBNA_PLOT_MODE_FIRST);
    updateCrossing();
  }
  updateStatus();
  updateModelPlotStatus();
  mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  notifyPlots();
}

void Session::modelPlotPress(int button, int x, int y) {
  (void)y;
  if (!project.modelplot)
    return;
  if (button == 1) {
    button1Down_ = true;
  } else if (button == 2) {
    button2Down_ = true;
  } else if (button == 3) {
    button3Down_ = true;
    mbna_modelplot_zoom_x1 = x;
    mbna_modelplot_zoom_x2 = x;
    project.modelplot_uptodate = false;
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
    notifyPlots();
  }
}

void Session::modelPlotDrag(int x, int y) {
  (void)y;
  if (!project.modelplot || !button3Down_)
    return;
  mbna_modelplot_zoom_x2 = x;
  project.modelplot_uptodate = false;
  mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  notifyPlots();
}

void Session::modelPlotRelease(int button, int x, int y) {
  if (!project.modelplot)
    return;
  if (button == 1 || button == 2) {
    if (button == 1) {
      button1Down_ = false;
      mbnavadjust_modelplot_pick(x, y);
    } else {
      button2Down_ = false;
      mbnavadjust_modelplot_middlepick(x, y);
    }
    if (!project.modelplot_uptodate) {
      updateModelPlotStatus();
      updateStatus();
      updateModelPlotStatus();
      mbnavadjust_modelplot_plot(__FILE__, __LINE__);
      if (project.visualization_status)
        do_update_visualization_status();
    }
  } else if (button == 3) {
    button3Down_ = false;
    mbna_modelplot_zoom_x2 = x;
    updateModelPlotStatus();
    mbnavadjust_modelplot_setzoom();
    project.modelplot_uptodate = false;
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
    mbna_modelplot_zoom_x1 = 0;
    mbna_modelplot_zoom_x2 = 0;
  }
  notifyPlots();
}

std::string Session::drawCounts() const {
  const DrawSurface *surfaces[4] = {contourSurface_, misfitSurface_, zOffsetSurface_, modelSurface_};
  std::string line;
  for (int i = 0; i < 4; i++) {
    const DrawSurface *s = surfaces[i];
    line += fmt("%s%ld %ld %ld %ld", i == 0 ? "" : " | ", s != nullptr ? s->lines : 0, s != nullptr ? s->rectangles : 0,
                s != nullptr ? s->fills : 0, s != nullptr ? s->texts : 0);
  }
  return line;
}

/*--------------------------------------------------------------------*/
/* main window actions */

void Session::setFileNav(FileNav value) {
  pushUndo();
  switch (value) {
  case FileNav::Poor:
    mbnavadjust_poornav_file();
    break;
  case FileNav::Good:
    mbnavadjust_goodnav_file();
    break;
  case FileNav::Fixed:
    mbnavadjust_fixednav_file();
    break;
  case FileNav::FixedXY:
    mbnavadjust_fixedxynav_file();
    break;
  case FileNav::FixedZ:
    mbnavadjust_fixedznav_file();
    break;
  }
  updateStatus();
}

void Session::setTieSetting(TieSetting value) {
  pushUndo();
  switch (value) {
  case TieSetting::XY:
    mbnavadjust_set_tie_xy();
    break;
  case TieSetting::Z:
    mbnavadjust_set_tie_z();
    break;
  case TieSetting::XYZ:
    mbnavadjust_set_tie_xyz();
    break;
  case TieSetting::Unfixed:
    mbnavadjust_set_tie_unfixed();
    break;
  case TieSetting::Fixed:
    mbnavadjust_set_tie_fixed();
    break;
  }
  updateStatus();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    do_update_visualization_status();
}

static void autopick_progress(void) {
  if (Session::current() != nullptr)
    Session::current()->updateStatus();
}

void Session::autoPick(bool withVertical) {
  int error = MB_ERROR_NO_ERROR;
  mbna_status = MBNA_STATUS_AUTOPICK;
  mbnavadjust_autopick(mbna_verbose, &project, mbna_view_list, mbna_view_mode, mbna_survey_select, mbna_survey_select1,
                       mbna_survey_select2, mbna_file_select, mbna_section_select, MBNA_MEDIOCREOVERLAP_THRESHOLD,
                       withVertical, autopick_progress, &error);
  mbna_status = MBNA_STATUS_GUI;
  refreshAfterEdit(false);
}

void Session::autoSetVertical() {
  mbna_status = MBNA_STATUS_AUTOPICK;
  mbnavadjust_autosetsvsvertical();
  mbna_status = MBNA_STATUS_GUI;
  refreshAfterEdit(false);
}

void Session::findCrossings() {
  int error = MB_ERROR_NO_ERROR;
  notifyMessageOn("Checking for crossings...");
  mbnavadjust_findcrossings(mbna_verbose, &project, &error);
  notifyMessageOff();
  updateStatus();
}

void Session::zeroZOffsets() {
  mbnavadjust_zerozoffsets();
  updateStatus();
}

void Session::unsetSkipped() {
  mbnavadjust_unsetskipped();
  updateStatus();
}

void Session::invert() {
  mbna_status = MBNA_STATUS_NAVSOLVE;
  notifyMessageOn("Inverting for navigation adjustment model...");
  mbnavadjust_invertnav(mbna_verbose, &project);
  notifyMessageOff();
  mbna_status = MBNA_STATUS_GUI;
  refreshAfterEdit(false);
}

void Session::updateGrids() {
  mbna_status = MBNA_STATUS_NAVSOLVE;
  notifyMessageOn("Updating bathymetry grids...");
  mbnavadjust_updategrid(mbna_verbose, &project);
  notifyMessageOff();
  mbna_status = MBNA_STATUS_GUI;
  updateStatus();
}

void Session::applyNavigation() {
  notifyMessageOn("Applying adjusted navigation...");
  mbnavadjust_applynav(mbna_verbose, &project);
  notifyMessageOff();
  updateStatus();
}

}  // namespace mbnaq
