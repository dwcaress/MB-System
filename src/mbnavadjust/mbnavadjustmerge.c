/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjustmerge.c  4/14/2014
 *
 *    Copyright (c) 2014-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Mbnavadjustmerge merges two existing mbnavadjust projects. The result
 * can be to add one project to another or to create a new, third project
 * combining the two source projects.
 *
 * Author:  D. W. Caress
 * Date:  April 14, 2014
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"
#include "mb_process.h"
#include "mb_aux.h"
#include "mbnavadjust_io.h"

/* local defines */
#define MBNAVADJUSTMERGE_MODE_NONE 0
#define MBNAVADJUSTMERGE_MODE_ADD 1
#define MBNAVADJUSTMERGE_MODE_MERGE 2
#define MBNAVADJUSTMERGE_MODE_COPY 3
#define MBNAVADJUSTMERGE_MODE_MODIFY 4
#define MBNAVADJUSTMERGE_MODE_TRIANGULATE 5
#define NUMBER_MODS_MAX 1000
#define MOD_MODE_NONE 0
#define MOD_MODE_SET_GLOBAL_TIE 1
#define MOD_MODE_SET_GLOBAL_TIE_XYZ 2
#define MOD_MODE_SET_GLOBAL_TIE_XY 3
#define MOD_MODE_SET_GLOBAL_TIE_Z 4
#define MOD_MODE_UNSET_GLOBAL_TIE 5
#define MOD_MODE_UNSET_ALL_GLOBAL_TIES 6
#define MOD_MODE_ADD_CROSSING 7
#define MOD_MODE_SET_TIE 8
#define MOD_MODE_SET_TIE_XYZ 9
#define MOD_MODE_SET_TIE_XY 10
#define MOD_MODE_SET_TIE_Z 11
#define MOD_MODE_SET_TIES_XYZ_ALL 12
#define MOD_MODE_SET_TIES_XY_ALL 13
#define MOD_MODE_SET_TIES_Z_ALL 14
#define MOD_MODE_SET_TIES_XYZ_FILE 15
#define MOD_MODE_SET_TIES_XY_FILE 16
#define MOD_MODE_SET_TIES_Z_FILE 17
#define MOD_MODE_SET_TIES_XYZ_SURVEY 18
#define MOD_MODE_SET_TIES_XY_SURVEY 19
#define MOD_MODE_SET_TIES_Z_SURVEY 20
#define MOD_MODE_SET_TIES_XYZ_BYSURVEY 21
#define MOD_MODE_SET_TIES_XY_BYSURVEY 22
#define MOD_MODE_SET_TIES_Z_BYSURVEY 23
#define MOD_MODE_SET_TIES_XYZ_BLOCK 24
#define MOD_MODE_SET_TIES_XY_BLOCK 25
#define MOD_MODE_SET_TIES_Z_BLOCK 26
#define MOD_MODE_SET_TIES_ZOFFSET_BLOCK 27
#define MOD_MODE_SET_TIES_XY_BY_TIME 28
#define MOD_MODE_UNSET_TIE 29
#define MOD_MODE_UNSET_TIES_FILE 30
#define MOD_MODE_UNSET_TIES_SURVEY 31
#define MOD_MODE_UNSET_TIES_BYSURVEY 32
#define MOD_MODE_UNSET_TIES_BLOCK 33
#define MOD_MODE_UNSET_TIES_ALL 34
#define MOD_MODE_SKIP_UNSET_CROSSINGS 35
#define MOD_MODE_UNSET_SKIPPED_CROSSINGS 36
#define MOD_MODE_UNSET_SKIPPED_CROSSINGS_BLOCK 37
#define MOD_MODE_UNSET_SKIPPED_CROSSINGS_BETWEEN_SURVEYS 38
#define MOD_MODE_INSERT_DISCONTINUITY 39
#define MOD_MODE_REIMPORT_FILE 40
#define MOD_MODE_REIMPORT_ALL_FILES 41
#define MOD_MODE_TRIANGULATE 42
#define MOD_MODE_TRIANGULATE_SECTION 43
#define IMPORT_NONE 0
#define IMPORT_TIE 1
#define IMPORT_GLOBALTIE 2
#define TRIANGULATE_NONE 0
#define TRIANGULATE_NEW 1
#define TRIANGULATE_ALL 2

struct mbnavadjust_mod {
  int mode;
  int survey1;
  int file1;
  int section1;
  int snav1;
  int survey2;
  int file2;
  int section2;
  int snav2;
  double xoffset;
  double yoffset;
  double zoffset;
  double xsigma;
  double ysigma;
  double zsigma;
  double dt;
};

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  char program_name[] = "mbnavadjustmerge";
  char help_message[] = "mbnavadjustmerge merges two existing mbnavadjust projects.\n";
  char usage_message[] = "mbnavadjustmerge --input=project_path \n"
                          "\t[--input=project_path\n"
                          "\t--output=project_path\n"
                          "\t--set-global-tie=file:section[:snav]/xoffset/yoffset/zoffset[/xsigma/ysigma/zsigma]\n"
                          "\t--set-global-tie-xyz=file:section[:snav]\n"
                          "\t--set-global-tie-xyonly=file:section[:snav]\n"
                          "\t--set-global-tie-zonly=file:section[:snav]\n"
                          "\t--unset-global-tie=file:section\n"
                          "\t--unset-all-global-ties\n"
                          "\t--add-crossing=file1:section1/file2:section2\n"
                          "\t--set-tie=file1/file2/xoffset/yoffset/zoffset\n"
                          "\t--set-tie=file1:section1/file2:section2/xoffset/yoffset/zoffset\n"
                          "\t--set-tie=file1:section1/file2:section2/xoffset/yoffset/zoffset/xsigma/ysigma/zsigma\n"
                          "\t--set-tie-xyz=file1:section1/file2:section2\n"
                          "\t--set-tie-xyonly=file1:section1/file2:section2\n"
                          "\t--set-tie-zonly=file1:section1/file2:section2\n"
                          "\t--set-ties-xyz-all\n"
                          "\t--set-ties-xyonly-all\n"
                          "\t--set-ties-zonly-all\n"
                          "\t--set-ties-xyz-with-file=file\n"
                          "\t--set-ties-xyonly-with-file=file\n"
                          "\t--set-ties-zonly-with-file=file\n"
                          "\t--set-ties-xyz-with-survey=survey\n"
                          "\t--set-ties-xyonly-with-survey=survey\n"
                          "\t--set-ties-zonly-with-survey=survey\n"
                          "\t--set-ties-xyz-by-survey=survey\n"
                          "\t--set-ties-xyonly-by-survey=survey\n"
                          "\t--set-ties-zonly-by-survey=survey\n"
                          "\t--set-ties-xyz-by-block=survey1/survey2\n"
                          "\t--set-ties-xyonly-by-block=survey1/survey2\n"
                          "\t--set-ties-zonly-by-block=survey1/survey2\n"
                          "\t--set-ties-zoffset-by-block=survey1/survey2/zoffset\n"
                          "\t--set-ties-xyonly-by-time=timethreshold\n"
                          "\t--unset-tie=file1:section1/file2:section2\n"
                          "\t--unset-ties-with-file=file\n"
                          "\t--unset-ties-with-survey=survey\n"
                          "\t--unset-ties-by-survey=survey\n"
                          "\t--unset-ties-by-block=survey1/survey2\n"
                          "\t--unset-all-ties\n"
                          "\t--skip-unset-crossings\n"
                          "\t--unset-skipped-crossings\n"
                          "\t--unset-skipped-crossings-by-block=survey1/survey2\n"
                          "\t--unset-skipped-crossings-between-surveys\n"
                          "\t--insert-discontinuity=file:section\n"
                          "\t--reimport-file=file\n"
                          "\t--reimport-all-files\n"
                          "\t--import-tie-list=file\n"
                          "\t--export-tie-list=file\n"
                          "\t--triangulate\n"
                          "\t--triangulate-all\n"
                          "\t--triangulate-scale=scale\n"
                          "\t--triangulate-section=file:section\n"
                          "\t--verbose --help]\n";
  extern char *optarg;
  int option_index;
  int errflg = 0;
  int c;
  int help = 0;

  /* MBIO status variables */
  int status = MB_SUCCESS;
  int verbose = 0;
  int error = MB_ERROR_NO_ERROR;

  /* command line option definitions */
  /* mbnavadjustmerge --verbose
   *     --help
   *     --input=project_path
   *     --output=project_path
   */
  static struct option options[] = {{"verbose", no_argument, NULL, 0},
                                    {"help", no_argument, NULL, 0},
                                    {"input", required_argument, NULL, 0},
                                    {"output", required_argument, NULL, 0},
                                    {"set-global-tie", required_argument, NULL, 0},
                                    {"set-global-tie-xyz", required_argument, NULL, 0},
                                    {"set-global-tie-xyonly", required_argument, NULL, 0},
                                    {"set-global-tie-zonly", required_argument, NULL, 0},
                                    {"unset-global-tie", required_argument, NULL, 0},
                                    {"unset-all-global-ties", no_argument, NULL, 0},
                                    {"add-crossing", required_argument, NULL, 0},
                                    {"set-tie", required_argument, NULL, 0},
                                    {"set-tie-xyz", required_argument, NULL, 0},
                                    {"set-tie-xyonly", required_argument, NULL, 0},
                                    {"set-tie-zonly", required_argument, NULL, 0},
                                    {"set-ties-xyz-all", no_argument, NULL, 0},
                                    {"set-ties-xyonly-all", no_argument, NULL, 0},
                                    {"set-ties-zonly-all", no_argument, NULL, 0},
                                    {"set-ties-xyz-with-file", required_argument, NULL, 0},
                                    {"set-ties-xyonly-with-file", required_argument, NULL, 0},
                                    {"set-ties-zonly-with-file", required_argument, NULL, 0},
                                    {"set-ties-xyz-with-survey", required_argument, NULL, 0},
                                    {"set-ties-xyonly-with-survey", required_argument, NULL, 0},
                                    {"set-ties-zonly-with-survey", required_argument, NULL, 0},
                                    {"set-ties-xyz-by-survey", required_argument, NULL, 0},
                                    {"set-ties-xyonly-by-survey", required_argument, NULL, 0},
                                    {"set-ties-zonly-by-survey", required_argument, NULL, 0},
                                    {"set-ties-xyz-by-block", required_argument, NULL, 0},
                                    {"set-ties-xyonly-by-block", required_argument, NULL, 0},
                                    {"set-ties-zonly-by-block", required_argument, NULL, 0},
                                    {"set-ties-zoffset-by-block", required_argument, NULL, 0},
                                    {"set-ties-xyonly-by-time", required_argument, NULL, 0},
                                    {"unset-tie", required_argument, NULL, 0},
                                    {"unset-ties-with-file", required_argument, NULL, 0},
                                    {"unset-ties-with-survey", required_argument, NULL, 0},
                                    {"unset-ties-by-survey", required_argument, NULL, 0},
                                    {"unset-ties-by-block", required_argument, NULL, 0},
                                    {"unset-ties-all", required_argument, NULL, 0},
                                    {"unset-all-ties", required_argument, NULL, 0},
                                    {"skip-unset-crossings", no_argument, NULL, 0},
                                    {"unset-skipped-crossings", no_argument, NULL, 0},
                                    {"unset-skipped-crossings-by-block", required_argument, NULL, 0},
                                    {"unset-skipped-crossings-between-surveys", no_argument, NULL, 0},
                                    {"insert-discontinuity", required_argument, NULL, 0},
                                    {"reimport-file", required_argument, NULL, 0},
                                    {"reimport-all-files", no_argument, NULL, 0},
                                    {"import-tie-list", required_argument, NULL, 0},
                                    {"export-tie-list", required_argument, NULL, 0},
                                    {"triangulate", no_argument, NULL, 0},
                                    {"triangulate-all", no_argument, NULL, 0},
                                    {"triangulate-section", required_argument, NULL, 0},
                                    {"triangulate-scale", required_argument, NULL, 0},
                                    {NULL, 0, NULL, 0}};

  /* mbnavadjustmerge controls */
  int mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_NONE;
  mb_path project_inputbase_path;
  mb_path project_inputadd_path;
  mb_path project_output_path;
  mb_path import_tie_list_path;
  mb_path export_tie_list_path;
  int project_inputbase_set = MB_NO;
  int project_inputadd_set = MB_NO;
  int project_output_set = MB_NO;
  int import_tie_list_set = MB_NO;
  int export_tie_list_set = MB_NO;
  int update_datalist = MB_NO;
  struct mbna_project project_inputbase;
  struct mbna_project project_inputadd;
  struct mbna_project project_output;

  /* mbnavadjustmerge mod parameters */
  struct mbnavadjust_mod mods[NUMBER_MODS_MAX];
  int num_mods = 0;

  struct mbna_file *file1;
  struct mbna_section *section1;
  struct mbna_file *file2;
  struct mbna_section *section2;
  struct mbna_file *file;
  struct mbna_section *section;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;

  int triangulate = TRIANGULATE_NONE;
  double triangle_scale = 0.0;

  int num_import_tie;
  int num_import_globaltie;
  int import_status = IMPORT_NONE;
  int import_tie_status;
  mb_path import_tie_file_1_path;
  mb_path import_tie_file_2_path;
  mb_path import_tie_file_1_name;
  mb_path import_tie_file_2_name;
  double import_tie_snav_1_time_d;
  double import_tie_snav_2_time_d;
  double import_tie_offset_x_m;
  double import_tie_offset_y_m;
  double import_tie_offset_z_m;
  double import_tie_sigmar1;
  double import_tie_sigmax1[3];
  double import_tie_sigmar2;
  double import_tie_sigmax2[3];
  double import_tie_sigmar3;
  double import_tie_sigmax3[3];
  int import_tie_file_1;
  int import_tie_file_2;
  int import_tie_section_1_id;
  int import_tie_section_2_id;
  int import_tie_snav_1;
  int import_tie_snav_2;
  int num_old_ties, num_new_ties;
  mb_path import_globaltie_file_path;
  mb_path import_globaltie_file_name;
  int import_globaltie_status;
  int import_globaltie_file;
  int import_globaltie_section_id;
  int import_globaltie_snav;
  double import_globaltie_snav_time_d;
  double import_globaltie_offset_x_m;
  double import_globaltie_offset_y_m;
  double import_globaltie_offset_z_m;
  double import_globaltie_offset_xsigma;
  double import_globaltie_offset_ysigma;
  double import_globaltie_offset_zsigma;
  FILE *tfp;

  char buffer[BUFFER_MAX];
  char *result = NULL;
  int nscan;
  int shellstatus;
  mb_path command = "";
  mb_path filename = "";
  double mtodeglon, mtodeglat;
  int found, current_crossing;
  int imod, ifile, icrossing, itie, itie_set, isection, isnav;
  double timediff, timediffmin;
  mb_path tmp_mb_path = "";
  int tmp_int;
  double tmp_double;
  int done;
  int i, j, k;

  memset(project_inputbase_path, 0, sizeof(mb_path));
  memset(project_inputadd_path, 0, sizeof(mb_path));
  memset(project_output_path, 0, sizeof(mb_path));
  memset(mods, 0, NUMBER_MODS_MAX * sizeof(struct mbnavadjust_mod));

  /* process argument list */
  while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
    switch (c) {
    /* long options all return c=0 */
    case 0:
      /* verbose */
      if (strcmp("verbose", options[option_index].name) == 0) {
        verbose++;
      }

      /* help */
      else if (strcmp("help", options[option_index].name) == 0) {
        help = MB_YES;
      }

      /*-------------------------------------------------------
       * Define input and output projects */

      /* input */
      else if (strcmp("input", options[option_index].name) == 0) {
        if (project_inputbase_set == MB_NO) {
          strcpy(project_inputbase_path, optarg);
          project_inputbase_set = MB_YES;
        }
        else if (project_inputadd_set == MB_NO) {
          strcpy(project_inputadd_path, optarg);
          project_inputadd_set = MB_YES;
         }
        else {
          fprintf(stderr, "Input projects already set:\n\t%s\n\t%s\nProject %s ignored...\n\n", project_inputbase_path,
                  project_inputadd_path, optarg);
        }
      }

      /* output */
      else if (strcmp("output", options[option_index].name) == 0) {
        if (project_output_set == MB_NO) {
          strcpy(project_output_path, optarg);
          project_output_set = MB_YES;
        }
        else {
          fprintf(stderr, "Output project already set:\n\t%s\nProject %s ignored\n\n", project_output_path, optarg);
        }
      }

      /*-------------------------------------------------------
       * set global tie
          --set-global-tie=file:section:snav/xoffset/yoffset/zoffset/xsigma/ysigma/zsigma
          --set-global-tie=file:section/xoffset/yoffset/zoffset/xsigma/ysigma/zsigma
          --set-global-tie=file:section:snav/xoffset/yoffset/zoffset
          --set-global-tie=file:section/xoffset/yoffset/zoffset */
      else if (strcmp("set-global-tie", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d:%d/%lf/%lf/%lf/%lf/%lf/%lf", &mods[num_mods].file1,
                              &mods[num_mods].section1, &mods[num_mods].snav1, &mods[num_mods].xoffset,
                              &mods[num_mods].yoffset, &mods[num_mods].zoffset, &mods[num_mods].xsigma,
                              &mods[num_mods].ysigma, &mods[num_mods].zsigma)) == 9) {
            mods[num_mods].mode = MOD_MODE_SET_GLOBAL_TIE;
            num_mods++;
          }
          else if ((nscan =
                        sscanf(optarg, "%d:%d/%lf/%lf/%lf/%lf/%lf/%lf", &mods[num_mods].file1, &mods[num_mods].section1,
                               &mods[num_mods].xoffset, &mods[num_mods].yoffset, &mods[num_mods].zoffset,
                               &mods[num_mods].xsigma, &mods[num_mods].ysigma, &mods[num_mods].zsigma)) == 8) {
            mods[num_mods].mode = MOD_MODE_SET_GLOBAL_TIE;
            mods[num_mods].snav1 = 0;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d:%d:%d/%lf/%lf/%lf", &mods[num_mods].file1, &mods[num_mods].section1,
                                   &mods[num_mods].snav1, &mods[num_mods].xoffset, &mods[num_mods].yoffset,
                                   &mods[num_mods].zoffset)) == 6) {
            mods[num_mods].mode = MOD_MODE_SET_GLOBAL_TIE;
            mods[num_mods].xsigma = 10.0;
            mods[num_mods].ysigma = 10.0;
            mods[num_mods].zsigma = 0.5;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d:%d/%lf/%lf/%lf", &mods[num_mods].file1, &mods[num_mods].section1,
                                   &mods[num_mods].xoffset, &mods[num_mods].yoffset, &mods[num_mods].zoffset)) == 5) {
            mods[num_mods].mode = MOD_MODE_SET_GLOBAL_TIE;
            mods[num_mods].snav1 = 0;
            mods[num_mods].xsigma = 10.0;
            mods[num_mods].ysigma = 10.0;
            mods[num_mods].zsigma = 0.5;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-global-tie=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--add-crossing=%s command ignored\n\n", optarg);
        }
      }

      /*-------------------------------------------------------
       * set global tie mode
          --set-global-tie-xyz=file:section:snav
          --set-global-tie-xyonly=file:section:snav
          --set-global-tie-zonly=file:section:snav */
      else if (strcmp("set-global-tie-xyz", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d:%d", &mods[num_mods].file1, &mods[num_mods].section1,
                              &mods[num_mods].snav1)) == 3) {
            mods[num_mods].mode = MOD_MODE_SET_GLOBAL_TIE_XYZ;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d:%d", &mods[num_mods].file1, &mods[num_mods].section1)) == 2) {
            mods[num_mods].snav1 = 0;
            mods[num_mods].mode = MOD_MODE_SET_GLOBAL_TIE_XYZ;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-global-tie-xyz=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--add-crossing=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("set-global-tie-xyonly", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d:%d", &mods[num_mods].file1, &mods[num_mods].section1,
                              &mods[num_mods].snav1)) == 3) {
            mods[num_mods].mode = MOD_MODE_SET_GLOBAL_TIE_XY;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d:%d", &mods[num_mods].file1, &mods[num_mods].section1)) == 2) {
            mods[num_mods].snav1 = 0;
            mods[num_mods].mode = MOD_MODE_SET_GLOBAL_TIE_XY;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-global-tie-xy=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--add-crossing=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("set-global-tie-zonly", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d:%d", &mods[num_mods].file1, &mods[num_mods].section1,
                              &mods[num_mods].snav1)) == 3) {
            mods[num_mods].mode = MOD_MODE_SET_GLOBAL_TIE_Z;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d:%d", &mods[num_mods].file1, &mods[num_mods].section1)) == 2) {
            mods[num_mods].snav1 = 0;
            mods[num_mods].mode = MOD_MODE_SET_GLOBAL_TIE_Z;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-global-tie-z=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--add-crossing=%s command ignored\n\n", optarg);
        }
      }

      /*-------------------------------------------------------
       * unset global ties
          --unset-global-tie=file:section
          --unset-all-global-ties  */
      else if (strcmp("unset-global-tie", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d", &mods[num_mods].file1, &mods[num_mods].section1)) == 2) {
            mods[num_mods].mode = MOD_MODE_UNSET_GLOBAL_TIE;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-global-tie-z=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--add-crossing=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("unset-all-global-ties", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_UNSET_ALL_GLOBAL_TIES;
          num_mods++;
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--add-crossing=%s command ignored\n\n", optarg);
        }
      }

      /*-------------------------------------------------------
       * add crossing
          --add-crossing=file1:section1/file2:section2 */
      else if (strcmp("add-crossing", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d/%d:%d", &mods[num_mods].file1, &mods[num_mods].section1,
                              &mods[num_mods].file2, &mods[num_mods].section2)) == 4) {
            mods[num_mods].mode = MOD_MODE_ADD_CROSSING;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d/%d", &mods[num_mods].file1, &mods[num_mods].file2)) == 2) {
            mods[num_mods].section1 = 0;
            mods[num_mods].section2 = 0;
            mods[num_mods].mode = MOD_MODE_ADD_CROSSING;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --add-crossing=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--add-crossing=%s command ignored\n\n", optarg);
        }
      }

      /*-------------------------------------------------------
       * set tie offset values - add tie if needed
          --set-tie=file1:section1/file2:section2/xoffset/yoffset/zoffset */
      else if (strcmp("set-tie", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d/%d:%d/%lf/%lf/%lf/%lf/%lf/%lf", &mods[num_mods].file1,
                              &mods[num_mods].section1, &mods[num_mods].file2, &mods[num_mods].section2,
                              &mods[num_mods].xoffset, &mods[num_mods].yoffset, &mods[num_mods].zoffset,
                              &mods[num_mods].xsigma, &mods[num_mods].ysigma, &mods[num_mods].zsigma)) == 10) {
            mods[num_mods].mode = MOD_MODE_SET_TIE;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d:%d/%d:%d/%lf/%lf/%lf", &mods[num_mods].file1, &mods[num_mods].section1,
                                   &mods[num_mods].file2, &mods[num_mods].section2, &mods[num_mods].xoffset,
                                   &mods[num_mods].yoffset, &mods[num_mods].zoffset)) == 7) {
            mods[num_mods].xsigma = 10.0;
            mods[num_mods].ysigma = 10.0;
            mods[num_mods].zsigma = 1.0;
            mods[num_mods].mode = MOD_MODE_SET_TIE;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d/%d/%lf/%lf/%lf", &mods[num_mods].file1, &mods[num_mods].file2,
                                   &mods[num_mods].xoffset, &mods[num_mods].yoffset, &mods[num_mods].zoffset)) == 5) {
            mods[num_mods].section1 = 0;
            mods[num_mods].section2 = 0;
            mods[num_mods].xsigma = 10.0;
            mods[num_mods].ysigma = 10.0;
            mods[num_mods].zsigma = 1.0;
            mods[num_mods].mode = MOD_MODE_SET_TIE;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-tie=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-tie=%s command ignored\n\n", optarg);
        }
      }

      /*-------------------------------------------------------
       * set tie mode
          --set-tie-xyz=file1:section1/file2:section2
          --set-tie-xyonly=file1:section1/file2:section2
          --set-tie-zonly=file1:section1/file2:section2 */
      else if (strcmp("set-tie-xyz", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d/%d:%d", &mods[num_mods].file1, &mods[num_mods].section1,
                              &mods[num_mods].file2, &mods[num_mods].section2)) == 4) {
            mods[num_mods].mode = MOD_MODE_SET_TIE_XYZ;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d/%d", &mods[num_mods].file1, &mods[num_mods].file2)) == 2) {
            mods[num_mods].section1 = 0;
            mods[num_mods].section2 = 0;
            mods[num_mods].mode = MOD_MODE_SET_TIE_XYZ;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-tie-xyz=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-tie-xyz=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("set-tie-xyonly", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d/%d:%d", &mods[num_mods].file1, &mods[num_mods].section1,
                              &mods[num_mods].file2, &mods[num_mods].section2)) == 4) {
            mods[num_mods].mode = MOD_MODE_SET_TIE_XY;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d/%d", &mods[num_mods].file1, &mods[num_mods].file2)) == 2) {
            mods[num_mods].section1 = 0;
            mods[num_mods].section2 = 0;
            mods[num_mods].mode = MOD_MODE_SET_TIE_XY;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-tie-xy=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-tie-xy=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("set-tie-zonly", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d/%d:%d", &mods[num_mods].file1, &mods[num_mods].section1,
                              &mods[num_mods].file2, &mods[num_mods].section2)) == 4) {
            mods[num_mods].mode = MOD_MODE_SET_TIE_Z;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%d/%d", &mods[num_mods].file1, &mods[num_mods].file2)) == 2) {
            mods[num_mods].section1 = 0;
            mods[num_mods].section2 = 0;
            mods[num_mods].mode = MOD_MODE_SET_TIE_Z;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-tie-z=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-tie-z=%s command ignored\n\n", optarg);
        }
      }

      /*-------------------------------------------------------
       * set mode of all ties
          --set-ties-xyz-all
          --set-ties-xyonly-all
          --set-ties-zonly-all */
      else if (strcmp("set-ties-xyz-all", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_SET_TIES_XYZ_ALL;
          num_mods++;
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-ties-xyz-all=%s command ignored\n\n",
                  optarg);
        }
      }
      else if (strcmp("set-ties-xyonly-all", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_SET_TIES_XY_ALL;
          num_mods++;
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-ties-xyonly-all=%s command ignored\n\n",
                  optarg);
        }
      }
      else if (strcmp("set-ties-xyz-all", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_SET_TIES_Z_ALL;
          num_mods++;
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-ties-xyz-all=%s command ignored\n\n",
                  optarg);
        }
      }

      /*-------------------------------------------------------
       * set mode of all ties with a file
          --set-ties-xyz-with-file=file
          --set-ties-xyonly-with-file=file
          --set-ties-zonly-with-file=file */
      else if (strcmp("set-ties-xyz-with-file", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].file1)) == 1) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_XYZ_FILE;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyz-with-file=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-ties-xyz-with-file=%s command ignored\n\n",
                  optarg);
        }
      }
      else if (strcmp("set-ties-xyonly-with-file", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].file1)) == 1) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_XY_FILE;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyonly-with-file=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\t--set-ties-xyonly-with-file=%s command ignored\n\n",
                  optarg);
        }
      }
      else if (strcmp("set-ties-xyz-with-file", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].file1)) == 1) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_Z_FILE;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyz-with-file=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-ties-xyz-with-file=%s command ignored\n\n",
                  optarg);
        }
      }

      /*-------------------------------------------------------
       * set mode of all ties with a survey
          --set-ties-xyz-with-survey=survey
          --set-ties-xyonly-with-survey=survey
          --set-ties-zonly-with-survey=survey */
      else if (strcmp("set-ties-xyz-with-survey", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].survey1)) == 1) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_XYZ_SURVEY;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyz-with-survey=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\t--set-ties-xyz-with-survey=%s command ignored\n\n",
                  optarg);
        }
      }
      else if (strcmp("set-ties-xyonly-with-survey", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].survey1)) == 1) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_XY_SURVEY;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyonly-with-survey=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\t--set-ties-xyonly-with-survey=%s command ignored\n\n",
                  optarg);
        }
      }
      else if (strcmp("set-ties-zonly-with-survey", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].survey1)) == 1) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_Z_SURVEY;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-zonly-with-survey=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\t--set-ties-zonly-with-survey=%s command ignored\n\n",
                  optarg);
        }
      }

      /*-------------------------------------------------------
       * set mode of all ties by survey
          --set-ties-xyz-by-survey=survey
          --set-ties-xyonly-by-survey=survey
          --set-ties-zonly-by-survey=survey */
      else if (strcmp("set-ties-xyz-by-survey", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].survey1)) == 1) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_XYZ_BYSURVEY;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyz-by-survey=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\t--set-ties-xyz-by-survey=%s command ignored\n\n",
                  optarg);
        }
      }
      else if (strcmp("set-ties-xyonly-by-survey", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].survey1)) == 1) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_XY_BYSURVEY;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyonly-by-survey=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\t--set-ties-xyonly-by-survey=%s command ignored\n\n",
                  optarg);
        }
      }
      else if (strcmp("set-ties-zonly-by-survey", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].survey1)) == 1) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_Z_BYSURVEY;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-zonly-by-survey=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\t--set-ties-zonly-by-survey=%s command ignored\n\n",
                  optarg);
        }
      }

      /*-------------------------------------------------------
       * set mode of all ties between two surveys
          --set-ties-xyz-by-block=survey1/survey2
          --set-ties-xyonly-by-block=survey1/survey2
          --set-ties-zonly-by-block=survey1/survey2 */
      else if (strcmp("set-ties-xyz-by-block", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d/%d", &mods[num_mods].survey1, &mods[num_mods].survey2)) == 2) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_XYZ_BLOCK;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyz-with-block=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-ties-xyz-with-block=%s command ignored\n\n",
                  optarg);
        }
      }
      else if (strcmp("set-ties-xyonly-by-block", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d/%d", &mods[num_mods].survey1, &mods[num_mods].survey2)) == 2) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_XY_BLOCK;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyonly-with-block=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\t--set-ties-xyonly-with-block=%s command ignored\n\n",
                  optarg);
        }
      }
      else if (strcmp("set-ties-xyz-by-block", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d/%d", &mods[num_mods].survey1, &mods[num_mods].survey2)) == 2) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_Z_BLOCK;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyz-with-block=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-ties-xyz-with-block=%s command ignored\n\n",
                  optarg);
        }
      }

      /*-------------------------------------------------------
       * set zoffset of all ties between two surveys
          --set-ties-zoffset-by-block=survey1/survey2/zoffset */
      else if (strcmp("set-ties-zoffset-by-block", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d/%d/%lf", &mods[num_mods].survey1, &mods[num_mods].survey2,
                              &mods[num_mods].zoffset)) == 3) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_ZOFFSET_BLOCK;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-zoffset-with-block=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-ties-xyz-with-block=%s command ignored\n\n",
                  optarg);
        }
      }

      /*-------------------------------------------------------
       * set all ties between nav points separated by more time than timethreshold to xyonly
          --set-ties-xyonly-by-time=timethreshold[y | d | h | m] */
      else if (strcmp("set-ties-xyonly-by-time", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%lfy", &mods[num_mods].dt)) == 1) {
            mods[num_mods].dt *= MB_SECINYEAR;
            mods[num_mods].mode = MOD_MODE_SET_TIES_XY_BY_TIME;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%lfd", &mods[num_mods].dt)) == 1) {
            mods[num_mods].dt *= MB_SECINDAY;
            mods[num_mods].mode = MOD_MODE_SET_TIES_XY_BY_TIME;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%lfh", &mods[num_mods].dt)) == 1) {
            mods[num_mods].dt *= MB_SECINHOUR;
            mods[num_mods].mode = MOD_MODE_SET_TIES_XY_BY_TIME;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%lfm", &mods[num_mods].dt)) == 1) {
            mods[num_mods].dt *= MB_SECINMINUTE;
            mods[num_mods].mode = MOD_MODE_SET_TIES_XY_BY_TIME;
            num_mods++;
          }
          else if ((nscan = sscanf(optarg, "%lf", &mods[num_mods].dt)) == 1) {
            mods[num_mods].mode = MOD_MODE_SET_TIES_XY_BY_TIME;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --set-ties-xyonly-by-time=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--set-ties-xyonly-by-time=%s command ignored\n\n",
                  optarg);
        }
      }

      /*-------------------------------------------------------
       * unset ties
          --unset-tie=file1:section1/file2:section2
          --unset-ties-with-file=file
          --unset-ties-with-survey=survey
          --unset-ties-by-survey=survey
          --unset-ties-by-block=survey1/survey2
          --unset-ties-all */
      else if (strcmp("unset-tie", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d/%d:%d", &mods[num_mods].file1, &mods[num_mods].section1,
                              &mods[num_mods].file2, &mods[num_mods].section2)) == 4) {
            mods[num_mods].mode = MOD_MODE_UNSET_TIE;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --unset-tie=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--unset-tie=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("unset-ties-with-file", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].file1)) == 1) {
            mods[num_mods].mode = MOD_MODE_UNSET_TIES_FILE;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --unset-ties-with-file=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--unset-ties-with-file=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("unset-ties-with-survey", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].survey1)) == 1) {
            mods[num_mods].mode = MOD_MODE_UNSET_TIES_SURVEY;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --unset-ties-with-survey=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--unset-ties-with-survey=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("unset-ties-by-survey", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].survey1)) == 1) {
            mods[num_mods].mode = MOD_MODE_UNSET_TIES_BYSURVEY;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --unset-ties-by-survey=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--unset-ties-by-survey=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("unset-ties-by-block", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d/%d",
            &mods[num_mods].survey1, &mods[num_mods].survey2)) == 2) {
            mods[num_mods].mode = MOD_MODE_UNSET_TIES_BLOCK;
            num_mods++;
          }
          else {
            fprintf(stderr, "Failure to parse --unset-ties-by-block=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--unset-ties-by-block=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("unset-ties-all", options[option_index].name) == 0
        || strcmp("unset-all-ties", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_UNSET_TIES_ALL;
          num_mods++;
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--unset-ties-all=%s command ignored\n\n", optarg);
        }
      }

      /*-------------------------------------------------------
       * set all crossings without ties in the input project(s) to be skipped
          --skip-unset-crossings */
      else if (strcmp("skip-unset-crossings", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_SKIP_UNSET_CROSSINGS;
          num_mods++;
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\tskip-unset-crossings command ignored\n\n");
        }
      }

      /*-------------------------------------------------------
       * unset all skipped crossings in a specified survey by survey block
          --unset-skipped-crossings
          --unset-skipped-crossings-by-block
          --unset-skipped-crossings-betweeen-surveys */
      else if (strcmp("unset-skipped-crossings", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_UNSET_SKIPPED_CROSSINGS;
          num_mods++;
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\tunset-skipped-crossings command ignored\n\n");
        }
      }
      else if (strcmp("unset-skipped-crossings-by-block", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d", &mods[num_mods].survey1, &mods[num_mods].survey2)) == 2) {
            mods[num_mods].mode = MOD_MODE_UNSET_SKIPPED_CROSSINGS_BLOCK;
            num_mods++;
          }
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\tunset-skipped-crossings-by-block command ignored\n\n");
        }
      }
      else if (strcmp("unset-skipped-crossings-between-surveys", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_UNSET_SKIPPED_CROSSINGS_BETWEEN_SURVEYS;
          num_mods++;
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\tunset-skipped-crossings-by-block command ignored\n\n");
        }
      }

      /*-------------------------------------------------------
       * Insert discontinuity immediately before the file and section specified
          --insert-discontinuity */
      else if (strcmp("insert-discontinuity", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d", &mods[num_mods].file1, &mods[num_mods].section1)) == 2) {
            mods[num_mods].mode = MOD_MODE_INSERT_DISCONTINUITY;
            num_mods++;
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\tskip-unset-crossings command ignored\n\n");
        }
      }

      /*-------------------------------------------------------
       * Reimport file (or files)
          --reimport-file
          --reimport-all-files */
      else if (strcmp("reimport-file", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d", &mods[num_mods].file1)) == 2) {
            mods[num_mods].mode = MOD_MODE_REIMPORT_FILE;
            num_mods++;
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\tskip-unset-crossings command ignored\n\n");
        }
      }
      else if (strcmp("reimport-all-files", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_REIMPORT_ALL_FILES;
          num_mods++;
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\tskip-unset-crossings command ignored\n\n");
        }
      }

      /*-------------------------------------------------------
       * Import or export list of ties
          --import-tie-list=file
          --export-tie-list=file */
      else if (strcmp("import-tie-list", options[option_index].name) == 0) {
        strcpy(import_tie_list_path, optarg);
        import_tie_list_set = MB_YES;
      }
      else if (strcmp("export-tie-list", options[option_index].name) == 0) {
        strcpy(export_tie_list_path, optarg);
        export_tie_list_set = MB_YES;
      }

      /*-------------------------------------------------------
       * Triangulate sections in preparation for contouring
          --triangulate
          --triangulate-section=file:section
          --triangulate-scale=scale */
      else if (strcmp("triangulate", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_TRIANGULATE;
          num_mods++;
          triangulate = TRIANGULATE_NEW;
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\ttriangulate command ignored\n\n");
        }
      }
      else if (strcmp("triangulate-all", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          mods[num_mods].mode = MOD_MODE_TRIANGULATE;
          num_mods++;
          triangulate = TRIANGULATE_ALL;
        }
        else {
          fprintf(stderr,
                  "Maximum number of mod commands reached:\n\ttriangulate-all command ignored\n\n");
        }
      }
      else if (strcmp("triangulate-section", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%d:%d", &mods[num_mods].file1, &mods[num_mods].section1)) == 2) {
            mods[num_mods].mode = MOD_MODE_TRIANGULATE_SECTION;
            num_mods++;
            triangulate = TRIANGULATE_ALL;
          }
          else {
            fprintf(stderr, "Failure to parse --triangulate-section=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--triangulate-section=%s command ignored\n\n", optarg);
        }
      }
      else if (strcmp("triangulate-scale", options[option_index].name) == 0) {
        if (num_mods < NUMBER_MODS_MAX) {
          if ((nscan = sscanf(optarg, "%lf", &triangle_scale)) != 1) {
            fprintf(stderr, "Failure to parse --triangulate-scale=%s\n\tmod command ignored\n\n", optarg);
          }
        }
        else {
          fprintf(stderr, "Maximum number of mod commands reached:\n\t--triangulate-scale=%s command ignored\n\n", optarg);
        }
      }

      /*-------------------------------------------------------*/

      break;
    case '?':
      errflg++;
    }

  /* if error flagged then print it and exit */
  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    error = MB_ERROR_BAD_USAGE;
    exit(error);
  }

  /* print starting message */
  if (verbose == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }

  /* print starting debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
    fprintf(stderr, "dbg2       help:                       %d\n", help);
    fprintf(stderr, "dbg2       project_inputbase_set:      %d\n", project_inputbase_set);
    fprintf(stderr, "dbg2       project_inputbase_path:     %s\n", project_inputbase_path);
    fprintf(stderr, "dbg2       project_inputadd_set:       %d\n", project_inputadd_set);
    fprintf(stderr, "dbg2       project_inputadd_path:      %s\n", project_inputadd_path);
    fprintf(stderr, "dbg2       project_output_set:         %d\n", project_output_set);
    fprintf(stderr, "dbg2       project_output_path:        %s\n", project_output_path);
    fprintf(stderr, "dbg2       import_tie_list_set:        %d\n", import_tie_list_set);
    fprintf(stderr, "dbg2       import_tie_list_path:       %s\n", import_tie_list_path);
    fprintf(stderr, "dbg2       export_tie_list_set:        %d\n", export_tie_list_set);
    fprintf(stderr, "dbg2       export_tie_list_path:       %s\n", export_tie_list_path);
    fprintf(stderr, "dbg2       num_mods:                   %d\n", num_mods);
    fprintf(stderr, "dbg2       mod# mode survey1 file1 section1 survey2 file2 section2 "
                    "xoffset yoffset zoffset xsigma ysigma zsigma dt\n");
    for (i = 0; i < num_mods; i++) {
      fprintf(stderr, "dbg2       mods[%d]: %d  %d %d %d   %d %d %d  %f %f %f  %f %f %f  %f\n", i, mods[i].mode,
              mods[i].survey1, mods[i].file1, mods[i].section1, mods[i].survey2, mods[i].file2, mods[i].section2,
              mods[i].xoffset, mods[i].yoffset, mods[i].zoffset, mods[i].xsigma, mods[i].ysigma, mods[i].zsigma,
              mods[i].dt);
    }
  }

  /* if help desired then print it and exit */
  if (help) {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    exit(error);
  }

  /* figure out mbnavadjust project merge mode */
  if (project_inputbase_set == MB_NO) {
    fprintf(stderr, "No input base project has been set.\n");
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    error = MB_ERROR_BAD_USAGE;
    exit(error);
  }
  else if (project_inputbase_set == MB_YES && project_inputadd_set == MB_NO && project_output_set == MB_NO) {
    strcpy(project_output_path, project_inputbase_path);
    int triangulate_only = MB_NO;
    if (triangulate != TRIANGULATE_NONE && import_tie_list_set == MB_NO) {
      triangulate_only = MB_YES;
      for (int imod = 0; imod < num_mods; imod++) {
        if (mods[imod].mode != MOD_MODE_TRIANGULATE
            && mods[imod].mode != MOD_MODE_TRIANGULATE_SECTION) {
              triangulate_only = MB_NO;
        }
      }
    }
    if (triangulate_only == MB_NO) {
      project_output_set = MB_YES;
      mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_MODIFY;
    }
    else {
      mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_TRIANGULATE;
    }

  }
  else if (project_inputbase_set == MB_YES && project_inputadd_set == MB_NO && project_output_set == MB_YES) {
    project_output_set = MB_YES;
    mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_COPY;
    update_datalist = MB_YES;
  }
  else if (project_inputbase_set == MB_YES && project_inputadd_set == MB_YES && project_output_set == MB_NO) {
    strcpy(project_output_path, project_inputbase_path);
    project_output_set = MB_YES;
    mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_ADD;
    update_datalist = MB_YES;
  }
  else if (project_inputbase_set == MB_YES && project_inputadd_set == MB_YES && project_output_set == MB_YES &&
    strcmp(project_output_path, project_inputadd_path) == 0) {
    fprintf(stderr, "The output project:\n\t%s\nis identical to the input add project:\n\t%s\n", project_output_path,
            project_inputadd_path);
    fprintf(stderr, "The output project must either be the input base project or a new project.\n");
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    error = MB_ERROR_BAD_USAGE;
    exit(error);
  }
  else if (project_inputbase_set == MB_YES && project_inputadd_set == MB_YES && project_output_set == MB_YES &&
    strcmp(project_output_path, project_inputbase_path) == 0) {
    mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_ADD;
  }
  else if (project_inputbase_set == MB_YES && project_inputadd_set == MB_YES && project_output_set == MB_YES) {
    mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_MERGE;
  }

  /* initialize the project structures */
  memset(&project_inputbase, 0, sizeof(struct mbna_project));
  memset(&project_inputadd, 0, sizeof(struct mbna_project));
  memset(&project_output, 0, sizeof(struct mbna_project));

  /* if merging two projects then read the first, create new output project */
  if (mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_MERGE || mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_COPY) {
    /* read the input base project */
    status = mbnavadjust_read_project(verbose, project_inputbase_path, &project_inputbase, &error);
    if (status == MB_SUCCESS) {
      fprintf(stderr, "\nInput base project loaded:\n\t%s\n", project_inputbase_path);
      fprintf(stderr, "\t%d files\n\t%d crossings\n\t%d ties\n", project_inputbase.num_files,
              project_inputbase.num_crossings, project_inputbase.num_ties);
    }
    else {
      fprintf(stderr, "Load failure for input base project:\n\t%s\n", project_inputbase_path);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      error = MB_ERROR_BAD_USAGE;
      exit(error);
    }

    status = mbnavadjust_new_project(
        verbose, project_output_path, project_inputbase.section_length, project_inputbase.section_soundings,
        project_inputbase.cont_int, project_inputbase.col_int, project_inputbase.tick_int, project_inputbase.label_int,
        project_inputbase.decimation, project_inputbase.smoothing, project_inputbase.zoffsetwidth, &project_output, &error);
    if (status == MB_SUCCESS) {
      fprintf(stderr, "\nOutput project created:\n\t%s\n", project_output_path);
    }
    else {
      fprintf(stderr, "Creation failure for output project:\n\t%s\n", project_output_path);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      error = MB_ERROR_BAD_USAGE;
      exit(error);
    }

    /* copy the input base project to the output project */
    // project_output.open = project_inputbase.open;
    // strcpy(project_output.name, project_inputbase.name);
    // strcpy(project_output.path, project_inputbase.path);
    // strcpy(project_output.home, project_inputbase.home);
    // strcpy(project_output.datadir, project_inputbase.datadir);
    // strcpy(project_output.logfile, project_inputbase.logfile);
    project_output.num_files = project_inputbase.num_files;
    project_output.num_files_alloc = 0;
    project_output.files = NULL;
    project_output.num_blocks = project_inputbase.num_blocks;
    project_output.num_snavs = project_inputbase.num_snavs;
    project_output.num_pings = project_inputbase.num_pings;
    project_output.num_beams = project_inputbase.num_beams;
    project_output.num_crossings = project_inputbase.num_crossings;
    project_output.num_crossings_alloc = 0;
    project_output.num_crossings_analyzed = project_inputbase.num_crossings_analyzed;
    project_output.num_goodcrossings = project_inputbase.num_goodcrossings;
    project_output.num_truecrossings = project_inputbase.num_truecrossings;
    project_output.num_truecrossings_analyzed = project_inputbase.num_truecrossings_analyzed;
    project_output.crossings = NULL;
    project_output.num_ties = project_inputbase.num_ties;
    project_output.section_length = project_inputbase.section_length;
    project_output.section_soundings = project_inputbase.section_soundings;
    project_output.cont_int = project_inputbase.cont_int;
    project_output.col_int = project_inputbase.col_int;
    project_output.tick_int = project_inputbase.tick_int;
    project_output.label_int = project_inputbase.label_int;
    project_output.decimation = project_inputbase.decimation;
    project_output.precision = project_inputbase.precision;
    project_output.smoothing = project_inputbase.smoothing;
    project_output.zoffsetwidth = project_inputbase.zoffsetwidth;
    // project_output.inversion_status = project_inputbase.inversion_status;
    // project_output.grid_status = project_inputbase.grid_status;
    // project_output.modelplot = project_inputbase.modelplot;
    // project_output.modelplot_style = project_inputbase.modelplot_style;
        // project_output.modelplot_uptodate = MB_NO;
    // project_output.logfp;

    /* allocate and copy the files */
    if (project_output.num_files > 0) {
      status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbna_file) * (project_output.num_files),
                          (void **)&project_output.files, &error);
      if (status == MB_SUCCESS && project_output.files != NULL) {
        /* copy the file data */
        project_output.num_files_alloc = project_output.num_files;
        memcpy(project_output.files, project_inputbase.files, project_output.num_files * sizeof(struct mbna_file));

        /* copy the sections in the files */
        for (i = 0; i < project_output.num_files && status == MB_SUCCESS; i++) {
          /* allocate and then copy the sections in this file */
          project_output.files[i].sections = NULL;
          if (project_output.files[i].num_sections > 0) {
            status = mb_mallocd(verbose, __FILE__, __LINE__,
                                sizeof(struct mbna_section) * (project_output.files[i].num_sections),
                                (void **)&project_output.files[i].sections, &error);
            if (status == MB_SUCCESS && project_output.files[i].sections != NULL) {
              project_output.files[i].num_sections_alloc = project_output.files[i].num_sections;
              memcpy(project_output.files[i].sections, project_inputbase.files[i].sections,
                     project_output.files[i].num_sections * sizeof(struct mbna_section));
            }
          }
        }
      }
      else {
        project_output.num_files_alloc = 0;
        status = MB_FAILURE;
        error = MB_ERROR_MEMORY_FAIL;
      }
    }
    if (status == MB_FAILURE) {
      fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
      exit(0);
    }

    /* allocate and copy the crossings */
    if (project_output.num_crossings > 0) {
      status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbna_crossing) * (project_output.num_crossings),
                          (void **)&project_output.crossings, &error);
      if (status == MB_SUCCESS && project_output.crossings != NULL) {
        project_output.num_crossings_alloc = project_output.num_crossings;
        memcpy(project_output.crossings, project_inputbase.crossings,
               project_output.num_crossings * sizeof(struct mbna_crossing));
      }
      else {
        project_output.num_crossings_alloc = 0;
        status = MB_FAILURE;
        error = MB_ERROR_MEMORY_FAIL;
      }
    }
    if (status == MB_FAILURE) {
      fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
      exit(0);
    }

    /* now concatenate the log.txt from the input project with the log.txt for the new output project */
    sprintf(command, "mv %s/log.txt %s/logorg.txt", project_output.datadir, project_output.datadir);
    // fprintf(stderr, "Executing in shell: %s\n", command);
    shellstatus = system(command);
    sprintf(command, "cat %s/log.txt %s/logorg.txt > %s/log.txt", project_inputbase.datadir, project_output.datadir,
            project_output.datadir);
    // fprintf(stderr, "Executing in shell: %s\n", command);
    shellstatus = system(command);

    /* now fix the data file paths to be relative to the new project location */
    for (i = 0; i < project_output.num_files; i++) {
      strcpy(project_output.files[i].file, project_output.files[i].path);
      status = mb_get_relative_path(verbose, project_output.files[i].file, project_output.path, &error);
    }

    /* now copy the actual data files from the input project to the new output project */
    for (i = 0; i < project_output.num_files; i++) {
      /* copy the file navigation */
      sprintf(command, "cp %s/nvs_%4.4d.mb166 %s", project_inputbase.datadir, i, project_output.datadir);
      // fprintf(stderr, "Executing in shell: %s\n", command);
      shellstatus = system(command);

      /* copy all the section files */
      for (j = 0; j < project_output.files[i].num_sections; j++) {
        /* copy the section file */
        sprintf(command, "cp %s/nvs_%4.4d_%4.4d.mb71* %s", project_inputbase.datadir, i, j, project_output.datadir);
        // fprintf(stderr, "Executing in shell: %s\n", command);
        shellstatus = system(command);
      }
    }
    fprintf(stderr, "\nCopied input base project to output project:\n\t%s\n", project_output_path);
    fprintf(stderr, "\t%d files\n\t%d crossings\n\t%d ties\n", project_output.num_files, project_output.num_crossings,
            project_output.num_ties);
  }

  /* else if adding the second project to the first, or just modifying the first,
      or just making triangle files, open the first as the output project */
  else if (mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_ADD
            || mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_MODIFY
            || mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_TRIANGULATE ) {

    /* read the input base project in as the output project */
    status = mbnavadjust_read_project(verbose, project_output_path, &project_output, &error);
    if (status == MB_SUCCESS) {
      fprintf(stderr, "\nInput base project loaded as output:\n\t%s\n", project_output_path);
      fprintf(stderr, "\t%d files\n\t%d crossings\n\t%d ties\n", project_output.num_files, project_output.num_crossings,
              project_output.num_ties);
    }
    else {
      fprintf(stderr, "Load failure for input base project (which is also the intended output):\n\t%s\n",
              project_output_path);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      error = MB_ERROR_BAD_USAGE;
      exit(error);
    }
  }

  /* if adding or merging projects read the input add project
      then add the input add project to the output project */
  if (mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_ADD || mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_MERGE) {
    status = mbnavadjust_read_project(verbose, project_inputadd_path, &project_inputadd, &error);
    if (status == MB_SUCCESS) {
      fprintf(stderr, "Input add project loaded:\n\t%s\n", project_inputadd_path);
      fprintf(stderr, "\t%d files\n\t%d crossings\n\t%d ties\n", project_inputadd.num_files, project_inputadd.num_crossings,
              project_inputadd.num_ties);
    }
    else {
      fprintf(stderr, "Load failure for input add project:\n\t%s\n", project_inputadd_path);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      error = MB_ERROR_BAD_USAGE;
      exit(error);
    }

    /* allocate space for additional files */
    if (project_inputadd.num_files > 0) {
      /* allocate space for the files in project_inputadd */
      status = mb_reallocd(verbose, __FILE__, __LINE__,
                           sizeof(struct mbna_file) * (project_output.num_files + project_inputadd.num_files),
                           (void **)&project_output.files, &error);
      if (status == MB_FAILURE) {
        fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
        exit(0);
      }
    }

    /* copy the file data from project_inputadd to project_output */
    project_output.num_files_alloc = project_output.num_files + project_inputadd.num_files;
    memcpy(&project_output.files[project_output.num_files], project_inputadd.files,
           project_inputadd.num_files * sizeof(struct mbna_file));

    /* copy the sections in the files */
    for (i = 0; i < project_inputadd.num_files && status == MB_SUCCESS; i++) {
      j = project_output.num_files + i;
      project_output.files[j].id += project_output.num_files;
      project_output.files[j].block += project_output.num_blocks;

      /* allocate and then copy the sections in this file */
      project_output.files[j].sections = NULL;
      if (project_output.files[j].num_sections > 0) {
        status =
            mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbna_section) * (project_output.files[j].num_sections),
                       (void **)&project_output.files[j].sections, &error);
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
          exit(0);
        }
        if (project_output.files[j].sections != NULL) {
          project_output.files[j].num_sections_alloc = project_output.files[j].num_sections;
          memcpy(project_output.files[j].sections, project_inputadd.files[i].sections,
                 project_output.files[j].num_sections * sizeof(struct mbna_section));
        }
      }
      for (k = 0; k < project_output.files[j].num_sections; k++) {
        project_output.files[j].sections[k].global_start_ping += project_output.num_pings;
        project_output.files[j].sections[k].global_start_snav += project_output.num_snavs;
      }
    }

    /* allocate and copy the crossings */
    if (project_inputadd.num_crossings > 0) {
      status = mb_reallocd(verbose, __FILE__, __LINE__,
                           sizeof(struct mbna_crossing) * (project_output.num_crossings + project_inputadd.num_crossings),
                           (void **)&project_output.crossings, &error);
      if (status == MB_FAILURE) {
        fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
        exit(0);
      }
      if (project_output.crossings != NULL) {
        project_output.num_crossings_alloc = project_output.num_crossings + project_inputadd.num_crossings;
        memcpy(&project_output.crossings[project_output.num_crossings], project_inputadd.crossings,
               project_inputadd.num_crossings * sizeof(struct mbna_crossing));
      }
      else {
        project_output.num_crossings_alloc = 0;
        status = MB_FAILURE;
        error = MB_ERROR_MEMORY_FAIL;
      }
      for (i = 0; i < project_inputadd.num_crossings; i++) {
        j = project_output.num_crossings + i;
        project_output.crossings[j].file_id_1 = project_inputadd.crossings[i].file_id_1 + project_output.num_files;
        project_output.crossings[j].file_id_2 = project_inputadd.crossings[i].file_id_2 + project_output.num_files;
        for (k = 0; k < project_output.crossings[j].num_ties; k++) {
          project_output.crossings[j].ties[k].block_1 += project_output.num_blocks;
          project_output.crossings[j].ties[k].block_2 += project_output.num_blocks;
        }
      }
    }
    if (status == MB_FAILURE) {
      fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
      exit(0);
    }

    /* now concatenate the log.txt from the inputadd project with the log.txt for the new output project */
    sprintf(command, "cat %s/log.txt %s/logorg.txt > %s/log.txt", project_inputadd.datadir, project_output.datadir,
            project_output.datadir);
    // fprintf(stderr, "Executing in shell: %s\n", command);
    shellstatus = system(command);

    /* now fix the data file paths to be relative to the new project location */
    for (i = 0; i < project_inputadd.num_files; i++) {
      k = project_output.num_files + i;
      strcpy(project_output.files[k].file, project_output.files[k].path);
      status = mb_get_relative_path(verbose, project_output.files[k].file, project_output.path, &error);
    }

    /* now copy the actual data files from the input project to the new output project */
    for (i = 0; i < project_inputadd.num_files; i++) {
      k = project_output.num_files + i;

      /* copy the file navigation */
      sprintf(command, "cp %s/nvs_%4.4d.mb166 %s/nvs_%4.4d.mb166", project_inputadd.datadir, i, project_output.datadir, k);
      // fprintf(stderr, "Executing in shell: %s\n", command);
      shellstatus = system(command);

      /* copy all the section files */
      for (j = 0; j < project_inputadd.files[i].num_sections; j++) {
        /* copy the section file */
        sprintf(command, "cp %s/nvs_%4.4d_%4.4d.mb71 %s/nvs_%4.4d_%4.4d.mb71", project_inputadd.datadir, i, j,
                project_output.datadir, k, j);
        // fprintf(stderr, "Executing in shell: %s\n", command);
        shellstatus = system(command);
      }
    }
    fprintf(stderr, "\nCopied input add project to output project:\n\t%s\n", project_output_path);
    fprintf(stderr, "\t%d files\n\t%d crossings\n\t%d ties\n", project_output.num_files, project_output.num_crossings,
            project_output.num_ties);

    /* finally update all of the global counters */
    project_output.num_files += project_inputadd.num_files;
    project_output.num_blocks += project_inputadd.num_blocks;
    project_output.num_snavs += project_inputadd.num_snavs;
    project_output.num_pings += project_inputadd.num_pings;
    project_output.num_beams += project_inputadd.num_beams;
    project_output.num_crossings += project_inputadd.num_crossings;
    project_output.num_crossings_analyzed += project_inputadd.num_crossings_analyzed;
    project_output.num_goodcrossings += project_inputadd.num_goodcrossings;
    project_output.num_truecrossings += project_inputadd.num_truecrossings;
    project_output.num_truecrossings_analyzed += project_inputadd.num_truecrossings_analyzed;
    project_output.num_ties += project_inputadd.num_ties;
  }

  /* apply any specified changes to the output project */
  for (imod = 0; imod < num_mods; imod++) {
    switch (mods[imod].mode) {
    case MOD_MODE_SET_GLOBAL_TIE:
      fprintf(stderr, "\nCommand set-global-tie=%4.4d:%4.4d:%4.4d/%.3f/%.3f/%.3f/%.3f/%.3f/%.3f\n", mods[imod].file1,
              mods[imod].section1, mods[imod].snav2, mods[imod].xoffset, mods[imod].yoffset, mods[imod].zoffset,
              mods[imod].xsigma, mods[imod].ysigma, mods[imod].zsigma);

      /* if this file, section, and snav exists apply the global tie as XYZ
          if the file and section exist but the snav does not then
          unset the global tie */
      if (mods[imod].file1 >= 0 && mods[imod].file1 < project_output.num_files) {
        file1 = (struct mbna_file *)&project_output.files[mods[imod].file1];
        if (mods[imod].section1 >= 0 && mods[imod].section1 < file1->num_sections) {
          section1 = (struct mbna_section *)&file1->sections[mods[imod].section1];
          mb_coor_scale(verbose, 0.5 * (section1->latmin + section1->latmax), &mtodeglon, &mtodeglat);
          if (mods[imod].snav1 >= 0 && mods[imod].snav1 < section1->num_snav) {
            section1->global_tie_status = MBNA_TIE_XYZ;
            section1->global_tie_snav = mods[imod].snav1;
                        section1->global_tie_inversion_status = MBNA_INVERSION_NONE;
            section1->offset_x = mods[imod].xoffset * mtodeglon;
            section1->offset_y = mods[imod].yoffset * mtodeglat;
            section1->offset_x_m = mods[imod].xoffset;
            section1->offset_y_m = mods[imod].yoffset;
            section1->offset_z_m = mods[imod].zoffset;
            section1->xsigma = mods[imod].xsigma;
            section1->ysigma = mods[imod].ysigma;
            section1->zsigma = mods[imod].zsigma;
          }
          else {
            section1->global_tie_status = MBNA_TIE_NONE;
            section1->global_tie_snav = MBNA_SELECT_NONE;
                        section1->global_tie_inversion_status = MBNA_INVERSION_NONE;
            section1->offset_x = 0.0;
            section1->offset_y = 0.0;
            section1->offset_x_m = 0.0;
            section1->offset_y_m = 0.0;
            section1->offset_z_m = 0.0;
            section1->xsigma = 0.0;
            section1->ysigma = 0.0;
            section1->zsigma = 0.0;
          }
                section1->inversion_offset_x = 0.0;
                section1->inversion_offset_y = 0.0;
                section1->inversion_offset_x_m = 0.0;
                section1->inversion_offset_y_m = 0.0;
                section1->inversion_offset_z_m = 0.0;
                section1->dx_m = 0.0;
                section1->dy_m = 0.0;
                section1->dz_m = 0.0;
                section1->sigma_m = 0.0;
                section1->dr1_m = 0.0;
                section1->dr2_m = 0.0;
                section1->dr3_m = 0.0;
                section1->rsigma_m = 0.0;
        }
      }
      break;

    case MOD_MODE_SET_GLOBAL_TIE_XYZ:
      fprintf(stderr, "\nCommand set-global-tie-xyz=%4.4d:%4.4d:%4.4d\n", mods[imod].file1, mods[imod].section1,
              mods[imod].snav2);

      /* if this file, section, and snav exists apply the global tie as XYZ
          if the file and section exist but the snav does not then
          unset the global tie */
      if (mods[imod].file1 >= 0 && mods[imod].file1 < project_output.num_files) {
        file1 = (struct mbna_file *)&project_output.files[mods[imod].file1];
        if (mods[imod].section1 >= 0 && mods[imod].section1 < file1->num_sections) {
          section1 = (struct mbna_section *)&file1->sections[mods[imod].section1];
          if (mods[imod].snav1 >= 0 && mods[imod].snav1 < section1->num_snav) {
            section1->global_tie_status = MBNA_TIE_XYZ;
            section1->global_tie_snav = mods[imod].snav1;
          }
          else {
            section1->global_tie_status = MBNA_TIE_NONE;
            section1->global_tie_snav = MBNA_SELECT_NONE;
          }
        }
      }
      break;

    case MOD_MODE_SET_GLOBAL_TIE_XY:
      fprintf(stderr, "\nCommand set-global-tie-xyonly=%4.4d:%4.4d:%4.4d\n", mods[imod].file1, mods[imod].section1,
              mods[imod].snav2);

      /* if this file, section, and snav exists apply the global tie as XY-only
          if the file and section exist but the snav does not then
          unset the global tie */
      if (mods[imod].file1 >= 0 && mods[imod].file1 < project_output.num_files) {
        file1 = (struct mbna_file *)&project_output.files[mods[imod].file1];
        if (mods[imod].section1 >= 0 && mods[imod].section1 < file1->num_sections) {
          section1 = (struct mbna_section *)&file1->sections[mods[imod].section1];
          if (mods[imod].snav1 >= 0 && mods[imod].snav1 < section1->num_snav) {
            section1->global_tie_status = MBNA_TIE_XY;
            section1->global_tie_snav = mods[imod].snav1;
          }
          else {
            section1->global_tie_status = MBNA_TIE_NONE;
            section1->global_tie_snav = MBNA_SELECT_NONE;
          }
        }
      }
      break;

    case MOD_MODE_SET_GLOBAL_TIE_Z:
      fprintf(stderr, "\nCommand set-global-tie-zonly=%4.4d:%4.4d:%4.4d\n", mods[imod].file1, mods[imod].section1,
              mods[imod].snav2);

      /* if this file, section, and snav exists apply the global tie as Z-only
          if the file and section exist but the snav does not then
          unset the global tie */
      if (mods[imod].file1 >= 0 && mods[imod].file1 < project_output.num_files) {
        file1 = (struct mbna_file *)&project_output.files[mods[imod].file1];
        if (mods[imod].section1 >= 0 && mods[imod].section1 < file1->num_sections) {
          section1 = (struct mbna_section *)&file1->sections[mods[imod].section1];
          if (mods[imod].snav1 >= 0 && mods[imod].snav1 < section1->num_snav) {
            section1->global_tie_status = MBNA_TIE_Z;
            section1->global_tie_snav = mods[imod].snav1;
          }
          else {
            section1->global_tie_status = MBNA_TIE_NONE;
            section1->global_tie_snav = MBNA_SELECT_NONE;
          }
        }
      }
      break;

    case MOD_MODE_UNSET_GLOBAL_TIE:
      fprintf(stderr, "\nCommand unset-global-tie=%4.4d:%4.4d\n", mods[imod].file1, mods[imod].section1);

      /* if this file and section exists unset the global tie */
      if (mods[imod].file1 >= 0 && mods[imod].file1 < project_output.num_files) {
        file1 = (struct mbna_file *)&project_output.files[mods[imod].file1];
        if (mods[imod].section1 >= 0 && mods[imod].section1 < file1->num_sections) {
          section1 = (struct mbna_section *)&file1->sections[mods[imod].section1];
          section1->global_tie_status = MBNA_TIE_NONE;
                    section1->global_tie_snav = MBNA_SELECT_NONE;
                    section1->global_tie_inversion_status = MBNA_INVERSION_NONE;
                    section1->offset_x = 0.0;
                    section1->offset_y = 0.0;
                    section1->offset_x_m = 0.0;
                    section1->offset_y_m = 0.0;
                    section1->offset_z_m = 0.0;
                    section1->xsigma = 0.0;
                    section1->ysigma = 0.0;
                    section1->zsigma = 0.0;
                    section1->inversion_offset_x = 0.0;
                    section1->inversion_offset_y = 0.0;
                    section1->inversion_offset_x_m = 0.0;
                    section1->inversion_offset_y_m = 0.0;
                    section1->inversion_offset_z_m = 0.0;
                    section1->dx_m = 0.0;
                    section1->dy_m = 0.0;
                    section1->dz_m = 0.0;
                    section1->sigma_m = 0.0;
                    section1->dr1_m = 0.0;
                    section1->dr2_m = 0.0;
                    section1->dr3_m = 0.0;
                    section1->rsigma_m = 0.0;
        }
      }
      break;

        case MOD_MODE_UNSET_ALL_GLOBAL_TIES:
            for (ifile = 0; ifile < project_output.num_files; ifile++) {
                file = &project_output.files[ifile];
                for (isection = 0; isection < file->num_sections; isection++) {
                    section1 = &file->sections[isection];
                    if (section1->global_tie_status != MBNA_TIE_NONE) {
                        section1->global_tie_status = MBNA_TIE_NONE;
                        section1->global_tie_snav = 0;
                        section1->global_tie_status = MBNA_TIE_NONE;
                        section1->global_tie_snav = MBNA_SELECT_NONE;
                        section1->global_tie_inversion_status = MBNA_INVERSION_NONE;
                        section1->offset_x = 0.0;
                        section1->offset_y = 0.0;
                        section1->offset_x_m = 0.0;
                        section1->offset_y_m = 0.0;
                        section1->offset_z_m = 0.0;
                        section1->xsigma = 0.0;
                        section1->ysigma = 0.0;
                        section1->zsigma = 0.0;
                        section1->inversion_offset_x = 0.0;
                        section1->inversion_offset_y = 0.0;
                        section1->inversion_offset_x_m = 0.0;
                        section1->inversion_offset_y_m = 0.0;
                        section1->inversion_offset_z_m = 0.0;
                        section1->dx_m = 0.0;
                        section1->dy_m = 0.0;
                        section1->dz_m = 0.0;
                        section1->sigma_m = 0.0;
                        section1->dr1_m = 0.0;
                        section1->dr2_m = 0.0;
                        section1->dr3_m = 0.0;
                        section1->rsigma_m = 0.0;
                    }
                }
            }
            break;

    case MOD_MODE_ADD_CROSSING:
      fprintf(stderr, "\nCommand add-crossing=%4.4d:%4.4d/%4.4d:%4.4d\n", mods[imod].file1, mods[imod].section1,
              mods[imod].file2, mods[imod].section2);

      /* check to see if this crossing already exists */
      found = MB_NO;
      for (icrossing = 0; icrossing < project_output.num_crossings && found == MB_NO; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2 &&
            crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2) {
          found = MB_YES;
        }
        else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2 &&
                 crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2) {
          found = MB_YES;
        }
      }

      /* if the crossing does not exist, create it */
      if (found == MB_NO) {
        /* allocate mbna_crossing array if needed */
        if (project_output.num_crossings_alloc <= project_output.num_crossings) {
          project_output.crossings = (struct mbna_crossing *)realloc(
              project_output.crossings,
              sizeof(struct mbna_crossing) * (project_output.num_crossings_alloc + ALLOC_NUM));
          if (project_output.crossings != NULL)
            project_output.num_crossings_alloc += ALLOC_NUM;
          else {
            status = MB_FAILURE;
            error = MB_ERROR_MEMORY_FAIL;
          }
        }

        /* add crossing to list */
        crossing = (struct mbna_crossing *)&project_output.crossings[project_output.num_crossings];
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        crossing->status = MBNA_CROSSING_STATUS_NONE;
        crossing->truecrossing = MB_NO;
        crossing->overlap = 0;
        crossing->file_id_1 = mods[imod].file1;
        crossing->section_1 = mods[imod].section1;
        crossing->file_id_2 = mods[imod].file2;
        crossing->section_2 = mods[imod].section2;
        crossing->num_ties = 0;
        current_crossing = project_output.num_crossings;
        project_output.num_crossings++;

        fprintf(stderr, "Added crossing: %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", current_crossing, file1->block,
                crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2, crossing->section_2);
      }
      break;

    case MOD_MODE_SET_TIE:
      fprintf(stderr, "\nCommand set-tie=%4.4d:%4.4d/%4.4d:%4.4d/%.3f/%.3f/%.3f/%.3f/%.3f/%.3f\n", mods[imod].file1,
              mods[imod].section1, mods[imod].file2, mods[imod].section2, mods[imod].xoffset, mods[imod].yoffset,
              mods[imod].zoffset, mods[imod].xsigma, mods[imod].ysigma, mods[imod].zsigma);

      /* check to see if this crossing already exists */
      found = MB_NO;
      for (icrossing = 0; icrossing < project_output.num_crossings && found == MB_NO; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2 &&
            crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2) {
          found = MB_YES;
          current_crossing = icrossing;
          crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        }
        else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2 &&
                 crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2) {
          found = MB_YES;
          current_crossing = icrossing;
          crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        }
      }

      /* if the crossing does not exist, create it */
      if (found == MB_NO) {
        /* allocate mbna_crossing array if needed */
        if (project_output.num_crossings_alloc <= project_output.num_crossings) {
          project_output.crossings = (struct mbna_crossing *)realloc(
              project_output.crossings,
              sizeof(struct mbna_crossing) * (project_output.num_crossings_alloc + ALLOC_NUM));
          if (project_output.crossings != NULL)
            project_output.num_crossings_alloc += ALLOC_NUM;
          else {
            status = MB_FAILURE;
            error = MB_ERROR_MEMORY_FAIL;
          }
        }

        /* add crossing to list */
        current_crossing = project_output.num_crossings;
        crossing = (struct mbna_crossing *)&project_output.crossings[current_crossing];
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        crossing->status = MBNA_CROSSING_STATUS_NONE;
        crossing->truecrossing = MB_NO;
        crossing->overlap = 0;
        crossing->file_id_1 = mods[imod].file1;
        crossing->section_1 = mods[imod].section1;
        crossing->file_id_2 = mods[imod].file2;
        crossing->section_2 = mods[imod].section2;
        crossing->num_ties = 0;
        current_crossing = project_output.num_crossings;
        project_output.num_crossings++;

        fprintf(stderr, "Added crossing: %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", current_crossing, file1->block,
                crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2, crossing->section_2);
      }

      /* if the tie does not exist, create it */
      if (crossing->num_ties == 0) {
        /* add tie and set number */
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        section1 = (struct mbna_section *)&file1->sections[crossing->section_1];
        section2 = (struct mbna_section *)&file2->sections[crossing->section_2];
        crossing->num_ties++;
        project_output.num_ties++;
        tie = &crossing->ties[0];

        if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
          project_output.num_crossings_analyzed++;
          if (crossing->truecrossing == MB_YES)
            project_output.num_truecrossings_analyzed++;
        }
        crossing->status = MBNA_CROSSING_STATUS_SET;

        /* use midpoint nav points */
        tie->snav_1 = section1->num_snav / 2;
        tie->snav_2 = section2->num_snav / 2;

        fprintf(stderr, "Added tie: %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d\n", current_crossing, 0,
                file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block, crossing->file_id_2,
                crossing->section_2, tie->snav_2);
      }

      /* set the tie parameters */
      for (itie = 0; itie < crossing->num_ties; itie++) {
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        section1 = (struct mbna_section *)&file1->sections[crossing->section_1];
        section2 = (struct mbna_section *)&file2->sections[crossing->section_2];
        tie = &crossing->ties[itie];
        tie->status = MBNA_TIE_XYZ;
        tie->snav_1_time_d = section1->snav_time_d[tie->snav_1];
        tie->snav_2_time_d = section2->snav_time_d[tie->snav_2];
        mb_coor_scale(verbose, 0.25 * (section1->latmin + section1->latmax + section2->latmin + section2->latmax),
                      &mtodeglon, &mtodeglat);
        tie->offset_x = mods[imod].xoffset * mtodeglon;
        tie->offset_y = mods[imod].yoffset * mtodeglat;
        tie->offset_x_m = mods[imod].xoffset;
        tie->offset_y_m = mods[imod].yoffset;
        tie->offset_z_m = mods[imod].zoffset;
        tie->sigmar1 = mods[imod].xsigma;
        tie->sigmax1[0] = 1.0;
        tie->sigmax1[1] = 0.0;
        tie->sigmax1[2] = 0.0;
        tie->sigmar2 = mods[imod].ysigma;
        tie->sigmax2[0] = 0.0;
        tie->sigmax2[1] = 1.0;
        tie->sigmax2[2] = 0.0;
        tie->sigmar3 = mods[imod].zsigma;
        tie->sigmax3[0] = 0.0;
        tie->sigmax3[1] = 0.0;
        tie->sigmax3[2] = 1.0;
        tie->inversion_offset_x = section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1];
        tie->inversion_offset_y = section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1];
        tie->inversion_offset_x_m = tie->inversion_offset_x / mtodeglon;
        tie->inversion_offset_y_m = tie->inversion_offset_y / mtodeglat;
        tie->inversion_offset_z_m = section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1];
        tie->inversion_status = MBNA_INVERSION_NONE;
        if (project_output.inversion_status == MBNA_INVERSION_CURRENT)
          project_output.inversion_status = MBNA_INVERSION_OLD;
        project_output.grid_status = MBNA_GRID_OLD;

        /* reset tie counts for snavs */
        section1->snav_num_ties[tie->snav_1]++;
        section2->snav_num_ties[tie->snav_2]++;

        fprintf(stderr,
                "Set tie offsets:       %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                current_crossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
      }

      break;

    case MOD_MODE_SET_TIE_XYZ:
      fprintf(stderr, "\nCommand set-tie-xyz=%4.4d:%4.4d/%4.4d:%4.4d\n", mods[imod].file1, mods[imod].section1,
              mods[imod].file2, mods[imod].section2);

      /* check to see if this crossing already exists */
      found = MB_NO;
      for (icrossing = 0; icrossing < project_output.num_crossings && found == MB_NO; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2 &&
            crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2) {
          found = MB_YES;
          current_crossing = icrossing;
          crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        }
        else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2 &&
                 crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2) {
          found = MB_YES;
          current_crossing = icrossing;
          crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        }
      }

      /* set the tie parameters */
      if (found == MB_YES) {
        for (itie = 0; itie < crossing->num_ties; itie++) {
          tie = &crossing->ties[itie];
          tie->status = MBNA_TIE_XYZ;

          fprintf(stderr,
                  "Set tie mode XYZ:      %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                  current_crossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
                  file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                  tie->offset_z_m);
        }
      }

      break;

    case MOD_MODE_SET_TIE_XY:
      fprintf(stderr, "\nCommand set-tie-xy=%4.4d:%4.4d/%4.4d:%4.4d\n", mods[imod].file1, mods[imod].section1,
              mods[imod].file2, mods[imod].section2);

      /* check to see if this crossing already exists */
      found = MB_NO;
      for (icrossing = 0; icrossing < project_output.num_crossings && found == MB_NO; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2 &&
            crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2) {
          found = MB_YES;
          current_crossing = icrossing;
          crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        }
        else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2 &&
                 crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2) {
          found = MB_YES;
          current_crossing = icrossing;
          crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        }
      }

      /* set the tie parameters */
      if (found == MB_YES) {
        for (itie = 0; itie < crossing->num_ties; itie++) {
          tie = &crossing->ties[itie];
          tie->status = MBNA_TIE_XY;

          fprintf(stderr,
                  "Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                  current_crossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
                  file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                  tie->offset_z_m);
        }
      }

      break;

    case MOD_MODE_SET_TIE_Z:
      fprintf(stderr, "\nCommand set-tie-z=%4.4d:%4.4d/%4.4d:%4.4d\n", mods[imod].file1, mods[imod].section1,
              mods[imod].file2, mods[imod].section2);

      /* check to see if this crossing already exists */
      found = MB_NO;
      for (icrossing = 0; icrossing < project_output.num_crossings && found == MB_NO; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2 &&
            crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2) {
          found = MB_YES;
          current_crossing = icrossing;
          crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        }
        else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2 &&
                 crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2) {
          found = MB_YES;
          current_crossing = icrossing;
          crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        }
      }

      /* set the tie parameters */
      if (found == MB_YES) {
        for (itie = 0; itie < crossing->num_ties; itie++) {
          tie = &crossing->ties[itie];
          tie->status = MBNA_TIE_Z;

          fprintf(stderr,
                  "Set tie mode Z-only:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                  current_crossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
                  file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                  tie->offset_z_m);
        }
      }

      break;

    case MOD_MODE_SET_TIES_XYZ_ALL:
      fprintf(stderr, "\nCommand set-ties-xyz-all=%4.4d\n", mods[imod].file1);

      /* loop over all crossings looking for ties, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        for (itie = 0; itie < crossing->num_ties; itie++) {
          tie = &crossing->ties[itie];
          if (tie->status != MBNA_TIE_XYZ) {
            tie->status = MBNA_TIE_XYZ;

            fprintf(
                stderr,
                "Set tie mode XYZ:      %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_XY_ALL:
      fprintf(stderr, "\nCommand set-ties-xy-all=%4.4d\n", mods[imod].file1);

      /* loop over all crossings looking for ties, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        for (itie = 0; itie < crossing->num_ties; itie++) {
          tie = &crossing->ties[itie];
          if (tie->status != MBNA_TIE_XY) {
            tie->status = MBNA_TIE_XY;

            fprintf(
                stderr,
                "Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_Z_ALL:
      fprintf(stderr, "\nCommand set-ties-z-all=%4.4d\n", mods[imod].file1);

      /* loop over all crossings looking for ties, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        for (itie = 0; itie < crossing->num_ties; itie++) {
          tie = &crossing->ties[itie];
          if (tie->status != MBNA_TIE_Z) {
            tie->status = MBNA_TIE_Z;

            fprintf(
                stderr,
                "Set tie mode Z-only:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_XYZ_FILE:
      fprintf(stderr, "\nCommand set-ties-xyz-with-file=%4.4d\n", mods[imod].file1);

      /* loop over all crossings looking for ones with specified file, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (crossing->file_id_1 == mods[imod].file1 || crossing->file_id_2 == mods[imod].file2) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_XYZ;

            fprintf(
                stderr,
                "Set tie mode XYZ:      %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_XY_FILE:
      fprintf(stderr, "\nCommand set-ties-xy-with-file=%4.4d\n", mods[imod].file1);

      /* loop over all crossings looking for ones with specified file, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (crossing->file_id_1 == mods[imod].file1 || crossing->file_id_2 == mods[imod].file2) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_XY;

            fprintf(
                stderr,
                "Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_Z_FILE:
      fprintf(stderr, "\nCommand set-ties-z-with-file=%4.4d\n", mods[imod].file1);

      /* loop over all crossings looking for ones with specified file, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (crossing->file_id_1 == mods[imod].file1 || crossing->file_id_2 == mods[imod].file2) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_Z;

            fprintf(
                stderr,
                "Set tie mode Z-only:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_XYZ_SURVEY:
      fprintf(stderr, "\nCommand set-ties-xyz-with-survey=%2.2d\n", mods[imod].survey1);

      /* loop over all crossings looking for ones with specified survey, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (project_output.files[crossing->file_id_1].block == mods[imod].survey1 ||
            project_output.files[crossing->file_id_2].block == mods[imod].survey1) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_XYZ;

            fprintf(
                stderr,
                "Set tie mode XYZ:      %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_XY_SURVEY:
      fprintf(stderr, "\nCommand set-ties-xy-with-survey=%2.2d\n", mods[imod].survey1);

      /* loop over all crossings looking for ones with specified survey, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (project_output.files[crossing->file_id_1].block == mods[imod].survey1 ||
            project_output.files[crossing->file_id_2].block == mods[imod].survey1) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_XY;

            fprintf(
                stderr,
                "Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_Z_SURVEY:
      fprintf(stderr, "\nCommand set-ties-z-with-survey=%2.2d\n", mods[imod].survey1);

      /* loop over all crossings looking for ones with specified survey, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (project_output.files[crossing->file_id_1].block == mods[imod].survey1 ||
            project_output.files[crossing->file_id_2].block == mods[imod].survey1) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_Z;

            fprintf(
                stderr,
                "Set tie mode Z-only:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_XYZ_BYSURVEY:
      fprintf(stderr, "\nCommand set-ties-xyz-by-survey=%2.2d\n", mods[imod].survey1);

      /* loop over all crossings looking for ones with specified survey, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (project_output.files[crossing->file_id_1].block == mods[imod].survey1 &&
            project_output.files[crossing->file_id_2].block == mods[imod].survey1) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_XYZ;

            fprintf(
                stderr,
                "Set tie mode XYZ:      %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_XY_BYSURVEY:
      fprintf(stderr, "\nCommand set-ties-xy-by-survey=%2.2d\n", mods[imod].survey1);

      /* loop over all crossings looking for ones with specified survey, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (project_output.files[crossing->file_id_1].block == mods[imod].survey1 &&
            project_output.files[crossing->file_id_2].block == mods[imod].survey1) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_XY;

            fprintf(
                stderr,
                "Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_Z_BYSURVEY:
      fprintf(stderr, "\nCommand set-ties-z-by-survey=%2.2d\n", mods[imod].survey1);

      /* loop over all crossings looking for ones with specified survey, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (project_output.files[crossing->file_id_1].block == mods[imod].survey1 &&
            project_output.files[crossing->file_id_2].block == mods[imod].survey1) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_Z;

            fprintf(
                stderr,
                "Set tie mode Z-only:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_XYZ_BLOCK:
      fprintf(stderr, "\nCommand set-ties-xyz-by-block=%2.2d/%2.2d\n", mods[imod].survey1, mods[imod].survey2);

      /* loop over all crossings looking for ones with specified surveys, then set the tie modes */
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if ((project_output.files[crossing->file_id_1].block == mods[imod].survey1 &&
             project_output.files[crossing->file_id_2].block == mods[imod].survey2) ||
            (project_output.files[crossing->file_id_2].block == mods[imod].survey1 &&
             project_output.files[crossing->file_id_1].block == mods[imod].survey2)) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_XYZ;

            fprintf(
                stderr,
                "Set tie mode XYZ:      %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_XY_BLOCK:
      fprintf(stderr, "\nCommand set-ties-xy-by-block=%2.2d/%2.2d\n", mods[imod].survey1, mods[imod].survey2);

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if ((project_output.files[crossing->file_id_1].block == mods[imod].survey1 &&
             project_output.files[crossing->file_id_2].block == mods[imod].survey2) ||
            (project_output.files[crossing->file_id_2].block == mods[imod].survey1 &&
             project_output.files[crossing->file_id_1].block == mods[imod].survey2)) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_XY;

            fprintf(
                stderr,
                "Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_Z_BLOCK:
      fprintf(stderr, "\nCommand set-ties-z-by-block=%2.2d/%2.2d\n", mods[imod].survey1, mods[imod].survey2);

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if ((project_output.files[crossing->file_id_1].block == mods[imod].survey1 &&
             project_output.files[crossing->file_id_2].block == mods[imod].survey2) ||
            (project_output.files[crossing->file_id_2].block == mods[imod].survey1 &&
             project_output.files[crossing->file_id_1].block == mods[imod].survey2)) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_Z;

            fprintf(
                stderr,
                "Set tie mode Z-only:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_ZOFFSET_BLOCK:
      fprintf(stderr, "\nCommand set-ties-zoffset-by-block=%2.2d/%2.2d/%f\n", mods[imod].survey1, mods[imod].survey2,
              mods[imod].zoffset);

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if ((project_output.files[crossing->file_id_1].block == mods[imod].survey1 &&
             project_output.files[crossing->file_id_2].block == mods[imod].survey2) ||
            (project_output.files[crossing->file_id_2].block == mods[imod].survey1 &&
             project_output.files[crossing->file_id_1].block == mods[imod].survey2)) {
          for (itie = 0; itie < crossing->num_ties; itie++) {
            tie = &crossing->ties[itie];
            tie->offset_z_m = mods[imod].zoffset;

            fprintf(stderr,
                    "Set tie zoffset:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                    icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
                    file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m,
                    tie->offset_y_m, tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_SET_TIES_XY_BY_TIME:
      fprintf(stderr, "\nCommand set-ties-xyonly-by-time=%f\n", mods[imod].dt);

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        for (itie = 0; itie < crossing->num_ties; itie++) {
          tie = &crossing->ties[itie];
          fprintf(stderr,
                  "Testing by time:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                  icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                  crossing->file_id_2, crossing->section_2, tie->snav_2, tie->snav_2_time_d, tie->snav_1_time_d,
                  fabs(tie->snav_2_time_d - tie->snav_1_time_d));
          if (fabs(tie->snav_2_time_d - tie->snav_1_time_d) >= mods[imod].dt) {
            tie = &crossing->ties[itie];
            tie->status = MBNA_TIE_XY;

            fprintf(
                stderr,
                "Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                tie->offset_z_m);
          }
        }
      }
      break;

    case MOD_MODE_UNSET_TIE:
      fprintf(stderr, "\nCommand unset-tie=%4.4d:%4.4d/%4.4d:%4.4d\n", mods[imod].file1, mods[imod].section1,
              mods[imod].file2, mods[imod].section2);

      /* check to see if this crossing already exists */
      found = MB_NO;
      for (icrossing = 0; icrossing < project_output.num_crossings && found == MB_NO; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2 &&
            crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2) {
          found = MB_YES;
          current_crossing = icrossing;
        }
        else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2 &&
                 crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2) {
          found = MB_YES;
          current_crossing = icrossing;
        }
      }

      /* unset the ties associated with this crossing */
      if (found == MB_YES && crossing->num_ties > 0) {
        crossing->num_ties = 0;

        fprintf(stderr, "Unset tie:   %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", current_crossing, file1->block,
                crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2, crossing->section_2);
      }
      break;

    case MOD_MODE_UNSET_TIES_FILE:
      fprintf(stderr, "\nCommand unset-ties-with-file=%d\n", mods[imod].file1);

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        if (crossing->num_ties > 0
            && (crossing->file_id_1 == mods[imod].file1
                || crossing->file_id_2 == mods[imod].file1)) {
          file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
          file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
          fprintf(stderr, "Unset tie(s) of crossing:   %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", current_crossing, file1->block,
                  crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2, crossing->section_2);
          crossing->num_ties = 0;
        }
      }
      break;

    case MOD_MODE_UNSET_TIES_SURVEY:
      fprintf(stderr, "\nCommand unset-ties-with-survey=%d\n", mods[imod].survey1);

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        if (crossing->num_ties > 0
            && (project_output.files[crossing->file_id_1].block == mods[imod].survey1 ||
                project_output.files[crossing->file_id_2].block == mods[imod].survey1)) {
          file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
          file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
          fprintf(stderr, "Unset tie(s) of crossing:   %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", current_crossing, file1->block,
                  crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2, crossing->section_2);
          crossing->num_ties = 0;
        }
      }
      break;

    case MOD_MODE_UNSET_TIES_BYSURVEY:
      fprintf(stderr, "\nCommand unset-ties-by-survey=%d\n", mods[imod].survey1);

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        if (crossing->num_ties > 0
            && (project_output.files[crossing->file_id_1].block == mods[imod].survey1 &&
                project_output.files[crossing->file_id_2].block == mods[imod].survey1)) {
          file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
          file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
          fprintf(stderr, "Unset tie(s) of crossing:   %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", current_crossing, file1->block,
                  crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2, crossing->section_2);
          crossing->num_ties = 0;
        }
      }
      break;

    case MOD_MODE_UNSET_TIES_BLOCK:
      fprintf(stderr, "\nCommand unset-ties-by-block=%d/%d\n", mods[imod].survey1, mods[imod].survey2);

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        if (crossing->num_ties > 0
            && ((project_output.files[crossing->file_id_1].block == mods[imod].survey1 ||
                  project_output.files[crossing->file_id_2].block == mods[imod].survey2)
              || (project_output.files[crossing->file_id_1].block == mods[imod].survey1 ||
                  project_output.files[crossing->file_id_2].block == mods[imod].survey2))) {
          file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
          file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
          fprintf(stderr, "Unset tie(s) of crossing:   %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", current_crossing, file1->block,
                  crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2, crossing->section_2);
          crossing->num_ties = 0;
        }
      }
      break;

    case MOD_MODE_UNSET_TIES_ALL:
      fprintf(stderr, "\nCommand unset-ties-all\n");

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        if (crossing->num_ties > 0) {
          file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
          file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
          fprintf(stderr, "Unset tie(s) of crossing:   %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", current_crossing, file1->block,
                  crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2, crossing->section_2);
          crossing->num_ties = 0;
        }
      }
      break;

    case MOD_MODE_SKIP_UNSET_CROSSINGS:
      fprintf(stderr, "\nCommand skip-unset-crossings\n");

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (crossing->num_ties == 0) {
          crossing->status = MBNA_CROSSING_STATUS_SKIP;
          fprintf(stderr, "Set crossing status to skip:   %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", icrossing,
                  file1->block, crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2,
                  crossing->section_2);
        }
      }
      break;

    case MOD_MODE_UNSET_SKIPPED_CROSSINGS:
      fprintf(stderr, "\nCommand unset-skipped-crossings\n");

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (crossing->status == MBNA_CROSSING_STATUS_SKIP)
            crossing->status = MBNA_CROSSING_STATUS_NONE;
        fprintf(stderr, "Unset skipped crossing:   %d:%d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", icrossing, itie,
                  file1->block, crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2,
                  crossing->section_2);
      }
      break;

    case MOD_MODE_UNSET_SKIPPED_CROSSINGS_BLOCK:
      fprintf(stderr, "\nCommand unset-skipped-crossings-by-block=%2.2d/%2.2d\n", mods[imod].survey1, mods[imod].survey2);

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if ((project_output.files[crossing->file_id_1].block == mods[imod].survey1 &&
             project_output.files[crossing->file_id_2].block == mods[imod].survey2) ||
            (project_output.files[crossing->file_id_2].block == mods[imod].survey1 &&
             project_output.files[crossing->file_id_1].block == mods[imod].survey2)) {
          if (crossing->status == MBNA_CROSSING_STATUS_SKIP)
            crossing->status = MBNA_CROSSING_STATUS_NONE;
          fprintf(stderr, "Unset skipped crossing:   %d:%d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", icrossing, itie,
                  file1->block, crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2,
                  crossing->section_2);
        }
      }
      break;

    case MOD_MODE_UNSET_SKIPPED_CROSSINGS_BETWEEN_SURVEYS:
      fprintf(stderr, "\nCommand unset-skipped-crossings-between-surveys\n");

      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
        file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
        if (project_output.files[crossing->file_id_1].block != project_output.files[crossing->file_id_2].block) {
          if (crossing->status == MBNA_CROSSING_STATUS_SKIP)
            crossing->status = MBNA_CROSSING_STATUS_NONE;
          fprintf(stderr, "Unset skipped crossing:   %d:%d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", icrossing, itie,
                  file1->block, crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2,
                  crossing->section_2);
        }
      }
      break;

    case MOD_MODE_INSERT_DISCONTINUITY:
      fprintf(stderr, "\nCommand insert-discontinuity=%2.2d:%2.2d\n", mods[imod].file1, mods[imod].section1);

      if (mods[imod].file1 >= 0 && mods[imod].file1 < project_output.num_files) {
        file1 = &(project_output.files[mods[imod].file1]);
        section1 = &(file1->sections[mods[imod].section1]);
        if (mods[imod].section1 >= 0 && mods[imod].section1 < file1->num_sections && section1->continuity == MB_YES) {
          section1->continuity = MB_NO;
          fprintf(stderr, "Set discontinuity before survey:file:section :   %2.2d:%4.4d:%4.4d\n", file1->block,
                  mods[imod].file1, mods[imod].section1);
        }
      }
      break;

    case MOD_MODE_REIMPORT_FILE:
    case MOD_MODE_REIMPORT_ALL_FILES:
      if (mods[imod].mode == MOD_MODE_REIMPORT_FILE)
        fprintf(stderr, "\nCommand reimport-file=%2.2d\n", mods[imod].file1);
      else
        fprintf(stderr, "\nCommand reimport-all-files\n");

      /* identify the file or files to be reimported */
      for (ifile = 0; ifile < project_output.num_files; ifile++) {
        /* either reimport a specific file or all the files */
        if (mods[imod].mode == MOD_MODE_REIMPORT_ALL_FILES || ifile == mods[imod].file1) {
          file = &(project_output.files[ifile]);

          /* load and copy the pre-adjusted navigation */

          /* open the processed data and read to the end
           * using existing section breaks unless and until the
           * input data extends later in time */
        }
      }
      break;

    case MOD_MODE_TRIANGULATE:
      // loop over all files and sections making the triangles for contouring
      project_output.triangle_scale = triangle_scale;
      for (ifile = 0; ifile < project_output.num_files; ifile++) {
        mb_path trianglefile;
        struct stat file_status;
        file = &(project_output.files[ifile]);
        for (isection = 0; isection < file->num_sections; isection++) {
          section = &(file->sections[isection]);
          struct mbna_swathraw *swathraw = NULL;
          struct swath *swath = NULL;
          sprintf(trianglefile, "%s/nvs_%4.4d_%4.4d.mb71.tri", project_output.datadir, ifile, isection);
          const int fstat = stat(trianglefile, &file_status);

          // if triangle file needs to be made (either specified or doesn't exist yet)
          // then load the section, which makes a triangle file if it is missing
          if (triangulate == TRIANGULATE_ALL
                || !(fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR
                      && file_status.st_size > 0)) {
            if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
              unlink(trianglefile);
            }
            if (verbose)
              fprintf(stderr,"Triangulating section %2.2d:%4.4d ", ifile, isection);
            status = mbnavadjust_section_load(verbose, &project_output, ifile, isection,
                                              (void **)&swathraw, (void **)&swath, section->num_pings, &error);
            status = mbnavadjust_section_unload(verbose, (void **)&swathraw, (void **)&swath, &error);
          }
          else {
            fprintf(stderr,"Skip triangulating section %2.2d:%4.4d - already exists\n", ifile, isection);
          }
        }
      }
      break;

    case MOD_MODE_TRIANGULATE_SECTION:

      // load the section - triangles will be created if they don't already exist
      project_output.triangle_scale = triangle_scale;
      file1 = &(project_output.files[mods[imod].file1]);
      section1 = &(file1->sections[mods[imod].section1]);
      struct mbna_swathraw *swathraw = NULL;
      struct swath *swath = NULL;
      // load the section - the triangle file will be created as part of loading
      if (verbose)
        fprintf(stderr,"Triangulating section %2.2d:%4.4d ", ifile, isection);
      status = mbnavadjust_section_load(verbose, &project_output, mods[imod].file1, mods[imod].section1,
                                          (void **)&swathraw, (void **)&swath, section1->num_pings, &error);
      status = mbnavadjust_section_unload(verbose, (void **)&swathraw, (void **)&swath, &error);
      break;

    }
  }

  /* if specified import ties from a tie list file */
  if (import_tie_list_set == MB_YES) {
    if ((tfp = fopen(import_tie_list_path, "r")) == NULL) {
      fprintf(stderr, "Unable to open tie list file %s for reading\n", import_tie_list_path);
      status = MB_FAILURE;
      error = MB_ERROR_OPEN_FAIL;
      exit(error);
    }

    /* read and process the ties */
    done = MB_NO;
    num_import_tie = 0;
    num_import_globaltie = 0;
    while (done == MB_NO) {
      import_status = IMPORT_NONE;

      /* read the next line  */
      if ((result = fgets(buffer, BUFFER_MAX, tfp)) != buffer) {
        done = MB_YES;
      }
      else if (strncmp(buffer, "TIE", 3) == 0) {
        /* read the first line of the next tie */
        if ((nscan = sscanf(buffer, "TIE %s %s %d %lf %lf %lf %lf %lf", import_tie_file_1_path, import_tie_file_2_path,
                            &import_tie_status, &import_tie_snav_1_time_d, &import_tie_snav_2_time_d,
                            &import_tie_offset_x_m, &import_tie_offset_y_m, &import_tie_offset_z_m)) == 8) {
          /* read the second line of the next tie */
          if ((result = fgets(buffer, BUFFER_MAX, tfp)) != buffer) {
            done = MB_YES;
          }
          else if ((nscan = sscanf(buffer, "COV %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &import_tie_sigmar1,
                                   &(import_tie_sigmax1[0]), &(import_tie_sigmax1[1]), &(import_tie_sigmax1[2]),
                                   &import_tie_sigmar2, &(import_tie_sigmax2[0]), &(import_tie_sigmax2[1]),
                                   &(import_tie_sigmax2[2]), &import_tie_sigmar3, &(import_tie_sigmax3[0]),
                                   &(import_tie_sigmax3[1]), &(import_tie_sigmax3[2]))) == 12) {
            import_status = IMPORT_TIE;
          }
        }
      }
      else if (strncmp(buffer, "GLOBALTIE", 9) == 0) {
        if ((nscan = sscanf(buffer, "GLOBALTIE %s %d %lf %lf %lf %lf %lf %lf %lf", import_globaltie_file_path,
                            &import_globaltie_status, &import_globaltie_snav_time_d, &import_globaltie_offset_x_m,
                            &import_globaltie_offset_y_m, &import_globaltie_offset_z_m, &import_globaltie_offset_xsigma,
                            &import_globaltie_offset_ysigma, &import_globaltie_offset_zsigma)) == 9) {
          import_status = IMPORT_GLOBALTIE;
        }
      }

      /* apply the new global tie if it has been read */
      if (done == MB_NO && import_status == IMPORT_GLOBALTIE) {
        fprintf(stderr, "\nAttempting to import global tie from list: \n\t%s\n%1d %16.6f %13.8f %13.8f %13.8f\n",
                import_globaltie_file_path, import_globaltie_status, import_globaltie_snav_time_d,
                import_globaltie_offset_x_m, import_globaltie_offset_y_m, import_globaltie_offset_z_m);

        /* figure out the file and section */
        found = MB_NO;
        strcpy(import_globaltie_file_name, (strrchr(import_globaltie_file_path, '/') + 1));
        for (ifile = 0; ifile < project_output.num_files && found == MB_NO; ifile++) {
          /* compare the file name rather than the path */
          file = &(project_output.files[ifile]);
          strcpy(filename, (strrchr(file->path, '/') + 1));
          if (strcmp(import_globaltie_file_name, filename) == 0) {
            import_globaltie_file = ifile;

            /* found the file, now find the section and snav */
            for (isection = 0; isection < file->num_sections; isection++) {
              section = &(file->sections[isection]);
              if (import_globaltie_snav_time_d >= section->btime_d &&
                  import_globaltie_snav_time_d <= section->etime_d) {
                /* now pick the closest snav */
                found = MB_YES;
                import_globaltie_section_id = isection;
                timediffmin = fabs(section->etime_d - section->btime_d);
                import_globaltie_snav = 0;
                for (isnav = 0; isnav < section->num_snav; isnav++) {
                  timediff = fabs(import_globaltie_snav_time_d - section->snav_time_d[isnav]);
                  if (timediff < timediffmin) {
                    import_globaltie_snav = isnav;
                    timediffmin = timediff;
                  }
                }
              }
            }
          }
        }

        /* if not found ignore the global tie */
        if (found == MB_NO) {
          fprintf(stderr, "Failure!!\n");
        }

        /* apply the global tie */
        else if (found == MB_YES) {
          fprintf(stderr, "Success!!\nImport global tie from list: %4.4d:%4.4d:%2.2d %.3f/%.3f/%.3f  %.3f/%.3f/%.3f\n",
                  import_globaltie_file, import_globaltie_section_id, import_globaltie_snav,
                  import_globaltie_offset_x_m, import_globaltie_offset_y_m, import_globaltie_offset_z_m,
                  import_globaltie_offset_xsigma, import_globaltie_offset_ysigma, import_globaltie_offset_zsigma);

          /* count the imported global ties */
          num_import_globaltie++;

          /* apply the global tie */
          file = &(project_output.files[import_globaltie_file]);
          section = &(file->sections[import_globaltie_section_id]);
          section->global_tie_status = import_globaltie_status;
          section->global_tie_snav = import_globaltie_snav;
                    section->global_tie_inversion_status = MBNA_INVERSION_NONE;
          section->offset_x_m = import_globaltie_offset_x_m;
          section->offset_y_m = import_globaltie_offset_y_m;
          section->offset_z_m = import_globaltie_offset_z_m;
          section->xsigma = import_globaltie_offset_xsigma;
          section->ysigma = import_globaltie_offset_ysigma;
          section->zsigma = import_globaltie_offset_zsigma;
          mb_coor_scale(verbose, 0.5 * (section->latmin + section->latmax), &mtodeglon, &mtodeglat);
          section->offset_x = section->offset_x_m * mtodeglon;
          section->offset_y = section->offset_y_m * mtodeglat;
                    section->inversion_offset_x = 0.0;
                    section->inversion_offset_y = 0.0;
                    section->inversion_offset_x_m = 0.0;
                    section->inversion_offset_y_m = 0.0;
                    section->inversion_offset_z_m = 0.0;
                    section->dx_m = 0.0;
                    section->dy_m = 0.0;
                    section->dz_m = 0.0;
                    section->sigma_m = 0.0;
                    section->dr1_m = 0.0;
                    section->dr2_m = 0.0;
                    section->dr3_m = 0.0;
                    section->rsigma_m = 0.0;
        }
      }

      /* apply the new tie if it has been read */
      if (done == MB_NO && import_status == IMPORT_TIE) {
        fprintf(stderr, "\nAttempting to import tie from list: \n\t%s\n\t%s\n%1d %16.6f %16.6f %13.8f %13.8f %13.8f\n",
                import_tie_file_1_path, import_tie_file_2_path, import_tie_status, import_tie_snav_1_time_d,
                import_tie_snav_2_time_d, import_tie_offset_x_m, import_tie_offset_y_m, import_tie_offset_z_m);

        /* figure out the file and block ids for the first file */
        found = MB_NO;
        strcpy(import_tie_file_1_name, (strrchr(import_tie_file_1_path, '/') + 1));
        for (ifile = 0; ifile < project_output.num_files; ifile++) {
          /* compare the file name rather than the path */
          file1 = &(project_output.files[ifile]);
          strcpy(filename, (strrchr(file1->path, '/') + 1));
          if (strcmp(import_tie_file_1_name, filename) == 0) {
            import_tie_file_1 = ifile;

            /* found the file, now find the section and snav */
            for (isection = 0; isection < file1->num_sections; isection++) {
              section1 = &(file1->sections[isection]);
              if (import_tie_snav_1_time_d >= section1->btime_d && import_tie_snav_1_time_d <= section1->etime_d) {
                /* now pick the closest snav */
                found = MB_YES;
                import_tie_section_1_id = isection;
                timediffmin = fabs(section1->etime_d - section1->btime_d);
                import_tie_snav_1 = 0;
                for (isnav = 0; isnav < section1->num_snav; isnav++) {
                  timediff = fabs(import_tie_snav_1_time_d - section1->snav_time_d[isnav]);
                  if (timediff < timediffmin) {
                    import_tie_snav_1 = isnav;
                    timediffmin = timediff;
                  }
                }
              }
            }
          }
        }

        /* figure out the file and block ids for the second file */
        if (found == MB_YES) {
          found = MB_NO;
          strcpy(import_tie_file_2_name, (strrchr(import_tie_file_2_path, '/') + 1));
          for (ifile = 0; ifile < project_output.num_files; ifile++) {
            /* compare the file name rather than the path */
            file2 = &(project_output.files[ifile]);
            strcpy(filename, (strrchr(file2->path, '/') + 1));
            if (strcmp(import_tie_file_2_name, filename) == 0) {
              import_tie_file_2 = ifile;

              /* found the file, now find the section and snav */
              for (isection = 0; isection < file2->num_sections; isection++) {
                section2 = &(file2->sections[isection]);
                if (import_tie_snav_2_time_d >= section2->btime_d &&
                    import_tie_snav_2_time_d <= section2->etime_d) {
                  /* now pick the closest snav */
                  found = MB_YES;
                  import_tie_section_2_id = isection;
                  timediffmin = fabs(section2->etime_d - section2->btime_d);
                  import_tie_snav_2 = 0;
                  for (isnav = 0; isnav < section2->num_snav; isnav++) {
                    timediff = fabs(import_tie_snav_2_time_d - section2->snav_time_d[isnav]);
                    if (timediff < timediffmin) {
                      import_tie_snav_2 = isnav;
                      timediffmin = timediff;
                    }
                  }
                }
              }
            }
          }
        }

        if (found == MB_NO) {
          fprintf(stderr, "Failure!!\n");
        }

        else if (found == MB_YES) {
          /* swap the order of the nav points if necessary */
          if (import_tie_file_1 > import_tie_file_2 ||
              (import_tie_file_1 == import_tie_file_2 && import_tie_section_1_id > import_tie_section_2_id)) {
            strcpy(tmp_mb_path, import_tie_file_1_path);
            strcpy(import_tie_file_1_path, import_tie_file_2_path);
            strcpy(import_tie_file_2_path, tmp_mb_path);
            tmp_double = import_tie_snav_1_time_d;
            import_tie_snav_1_time_d = import_tie_snav_2_time_d;
            import_tie_snav_2_time_d = tmp_double;
            tmp_int = import_tie_file_1;
            import_tie_file_1 = import_tie_file_2;
            import_tie_file_2 = tmp_int;
            tmp_int = import_tie_section_1_id;
            import_tie_section_1_id = import_tie_section_2_id;
            import_tie_section_2_id = tmp_int;
            tmp_int = import_tie_snav_1;
            import_tie_snav_1 = import_tie_snav_2;
            import_tie_snav_2 = tmp_int;
            import_tie_offset_x_m *= -1.0;
            import_tie_offset_y_m *= -1.0;
            import_tie_offset_z_m *= -1.0;
            import_tie_sigmax1[0] *= -1.0;
            import_tie_sigmax1[1] *= -1.0;
            import_tie_sigmax1[2] *= -1.0;
            import_tie_sigmax2[0] *= -1.0;
            import_tie_sigmax2[1] *= -1.0;
            import_tie_sigmax2[2] *= -1.0;
            import_tie_sigmax3[0] *= -1.0;
            import_tie_sigmax3[1] *= -1.0;
            import_tie_sigmax3[2] *= -1.0;
          }

          fprintf(
              stderr,
              "Success!!\nImport tie from list: %4.4d:%4.4d:%2.2d %4.4d:%4.4d:%2.2d  %.3f/%.3f/%.3f  %.3f/%.3f/%.3f\n",
              import_tie_file_1, import_tie_section_1_id, import_tie_snav_1, import_tie_file_2, import_tie_section_2_id,
              import_tie_snav_2, import_tie_offset_x_m, import_tie_offset_y_m, import_tie_offset_z_m,
              import_tie_sigmar1, import_tie_sigmar2, import_tie_sigmar3);
          /* count the imported ties */
          num_import_tie++;

          /* check to see if this crossing already exists */
          found = MB_NO;
          for (icrossing = 0; icrossing < project_output.num_crossings && found == MB_NO; icrossing++) {
            crossing = &(project_output.crossings[icrossing]);
            if (crossing->file_id_2 == import_tie_file_1 && crossing->file_id_1 == import_tie_file_2 &&
                crossing->section_2 == import_tie_section_1_id && crossing->section_1 == import_tie_section_2_id) {
              found = MB_YES;
              current_crossing = icrossing;
              crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
            }
            else if (crossing->file_id_1 == import_tie_file_1 && crossing->file_id_2 == import_tie_file_2 &&
                     crossing->section_1 == import_tie_section_1_id &&
                     crossing->section_2 == import_tie_section_2_id) {
              found = MB_YES;
              current_crossing = icrossing;
              crossing = (struct mbna_crossing *)&project_output.crossings[icrossing];
            }
          }

          /* if the crossing does not exist, create it */
          if (found == MB_YES) {
            fprintf(stderr, "Found existing crossing: %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", current_crossing,
                    file1->block, crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2,
                    crossing->section_2);
          }
          else if (found == MB_NO) {
            /* allocate mbna_crossing array if needed */
            if (project_output.num_crossings_alloc <= project_output.num_crossings) {
              project_output.crossings = (struct mbna_crossing *)realloc(
                  project_output.crossings,
                  sizeof(struct mbna_crossing) * (project_output.num_crossings_alloc + ALLOC_NUM));
              if (project_output.crossings != NULL)
                project_output.num_crossings_alloc += ALLOC_NUM;
              else {
                status = MB_FAILURE;
                error = MB_ERROR_MEMORY_FAIL;
              }
            }

            /* add crossing to list */
            current_crossing = project_output.num_crossings;
            crossing = (struct mbna_crossing *)&project_output.crossings[current_crossing];
            file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
            file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
            crossing->status = MBNA_CROSSING_STATUS_NONE;
            crossing->truecrossing = MB_NO;
            crossing->overlap = 0;
            crossing->file_id_1 = import_tie_file_1;
            crossing->section_1 = import_tie_section_1_id;
            crossing->file_id_2 = import_tie_file_2;
            crossing->section_2 = import_tie_section_2_id;
            crossing->num_ties = 0;
            current_crossing = project_output.num_crossings;
            project_output.num_crossings++;

            fprintf(stderr, "Added crossing: %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n", current_crossing,
                    file1->block, crossing->file_id_1, crossing->section_1, file2->block, crossing->file_id_2,
                    crossing->section_2);
          }

          /* check if this tie already exists */
          found = MB_NO;
          if (crossing->num_ties > 0) {
            for (itie = 0; itie < crossing->num_ties; itie++) {
              tie = &crossing->ties[itie];
              if (tie->snav_1 == import_tie_snav_1 && tie->snav_2 == import_tie_snav_2) {
                found = MB_YES;
                itie_set = itie;
              }
            }
          }

          /* if the tie exists change it */
          if (found == MB_YES) {
            /* set nav points */
            tie = &crossing->ties[itie_set];
            tie->snav_1 = import_tie_snav_1;
            tie->snav_2 = import_tie_snav_2;
            fprintf(stderr, "Reset existing tie: %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d\n",
                    current_crossing, 0, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
                    file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2);
          }

          /* if the tie does not exist, create it */
          else {
            /* add tie and set number */
            file1 = (struct mbna_file *)&project_output.files[crossing->file_id_1];
            file2 = (struct mbna_file *)&project_output.files[crossing->file_id_2];
            section1 = (struct mbna_section *)&file1->sections[crossing->section_1];
            section2 = (struct mbna_section *)&file2->sections[crossing->section_2];
            itie_set = crossing->num_ties;
            crossing->num_ties++;
            project_output.num_ties++;
            tie = &crossing->ties[itie_set];

            if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
              project_output.num_crossings_analyzed++;
              if (crossing->truecrossing == MB_YES)
                project_output.num_truecrossings_analyzed++;
            }
            crossing->status = MBNA_CROSSING_STATUS_SET;

            /* set nav points */
            tie->snav_1 = import_tie_snav_1;
            tie->snav_2 = import_tie_snav_2;

            /* augment tie counts for snavs */
            (section1->snav_num_ties[tie->snav_1])++;
            (section2->snav_num_ties[tie->snav_2])++;

            fprintf(stderr, "Added tie: %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d\n", current_crossing,
                    0, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1, file2->block,
                    crossing->file_id_2, crossing->section_2, tie->snav_2);
          }

          /* set the tie parameters */
          tie = &crossing->ties[itie_set];

          tie->status = -import_tie_status;
          tie->snav_1_time_d = section1->snav_time_d[tie->snav_1];
          tie->snav_2_time_d = section2->snav_time_d[tie->snav_2];
          mb_coor_scale(verbose, 0.25 * (section1->latmin + section1->latmax + section2->latmin + section2->latmax),
                        &mtodeglon, &mtodeglat);
          tie->offset_x = import_tie_offset_x_m * mtodeglon;
          tie->offset_y = import_tie_offset_y_m * mtodeglat;
          tie->offset_x_m = import_tie_offset_x_m;
          tie->offset_y_m = import_tie_offset_y_m;
          tie->offset_z_m = import_tie_offset_z_m;
          tie->sigmar1 = import_tie_sigmar1;
          tie->sigmax1[0] = import_tie_sigmax1[0];
          tie->sigmax1[1] = import_tie_sigmax1[1];
          tie->sigmax1[2] = import_tie_sigmax1[2];
          tie->sigmar2 = import_tie_sigmar2;
          tie->sigmax2[0] = import_tie_sigmax2[0];
          tie->sigmax2[1] = import_tie_sigmax2[1];
          tie->sigmax2[2] = import_tie_sigmax2[2];
          tie->sigmar3 = import_tie_sigmar3;
          tie->sigmax3[0] = import_tie_sigmax3[0];
          tie->sigmax3[1] = import_tie_sigmax3[1];
          tie->sigmax3[2] = import_tie_sigmax3[2];
          tie->inversion_offset_x = section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1];
          tie->inversion_offset_y = section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1];
          tie->inversion_offset_x_m = tie->inversion_offset_x / mtodeglon;
          tie->inversion_offset_y_m = tie->inversion_offset_y / mtodeglat;
          tie->inversion_offset_z_m = section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1];
          tie->inversion_status = MBNA_INVERSION_NONE;
          if (project_output.inversion_status == MBNA_INVERSION_CURRENT)
            project_output.inversion_status = MBNA_INVERSION_OLD;
          project_output.grid_status = MBNA_GRID_OLD;

          fprintf(stderr,
                  "Set tie offsets:       %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
                  current_crossing, itie, file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
                  file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m,
                  tie->offset_z_m);
        }
      }
    }

    /* now loop over all crossings. Imported ties have a negative status.
     * In cases where a crossing only has imported ties, set the status positive.
     * In cases where a crossing has both pre-existing and imported ties, then
     * delete the pre-existing ties, and set the status of the remaining
     * imported ties positive.
     */
    if (num_import_tie > 0) {
      for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
        crossing = &(project_output.crossings[icrossing]);
        num_old_ties = 0;
        num_new_ties = 0;
        for (itie = 0; itie < crossing->num_ties; itie++) {
          tie = &crossing->ties[itie];
          if (tie->status > 0)
            num_old_ties++;
          if (tie->status < 0)
            num_new_ties++;
        }
        if (num_old_ties > 0 && num_new_ties > 0) {
          for (itie = crossing->num_ties - 1; itie >= 0; itie--) {
            tie = &crossing->ties[itie];
            if (tie->status > 0) {
              fprintf(stderr,
                      "Removed duplicate pre-existing tie: crossing %d ( tie %d of %d): %2.2d:%4.4d:%4.4d:%2.2d   "
                      "%2.2d:%4.4d:%4.4d:%2.2d\n",
                      icrossing, itie, crossing->num_ties, file1->block, crossing->file_id_1, crossing->section_1,
                      tie->snav_1, file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2);

              /* reset tie counts for snavs */
              file1 = &project_output.files[crossing->file_id_1];
              section1 = &file1->sections[crossing->section_1];
              section1->snav_num_ties[tie->snav_1]--;
              file2 = &project_output.files[crossing->file_id_2];
              section2 = &file2->sections[crossing->section_2];
              section2->snav_num_ties[tie->snav_2]--;

              /* delete tie and set number */
              for (i = itie; i < crossing->num_ties - 1; i++) {
                crossing->ties[i].status = crossing->ties[i + 1].status;
                crossing->ties[i].snav_1 = crossing->ties[i + 1].snav_1;
                crossing->ties[i].snav_1_time_d = crossing->ties[i + 1].snav_1_time_d;
                crossing->ties[i].snav_2 = crossing->ties[i + 1].snav_2;
                crossing->ties[i].snav_2_time_d = crossing->ties[i + 1].snav_2_time_d;
                crossing->ties[i].offset_x = crossing->ties[i + 1].offset_x;
                crossing->ties[i].offset_y = crossing->ties[i + 1].offset_y;
                crossing->ties[i].offset_x_m = crossing->ties[i + 1].offset_x_m;
                crossing->ties[i].offset_y_m = crossing->ties[i + 1].offset_y_m;
                crossing->ties[i].offset_z_m = crossing->ties[i + 1].offset_z_m;
              }
              crossing->num_ties--;
              project_output.num_ties--;
            }
          }
        }
        for (itie = 0; itie < crossing->num_ties; itie++) {
          tie = &crossing->ties[itie];
          if (tie->status < 0) {
            tie->status = -tie->status;
          }
        }
      }
    }
  }

  /* if specified output ties to a tie list file */
  if (export_tie_list_set == MB_YES) {
    if ((tfp = fopen(export_tie_list_path, "w")) == NULL) {
      fprintf(stderr, "Unable to open tie list file %s for writing\n", export_tie_list_path);
      status = MB_FAILURE;
      error = MB_ERROR_OPEN_FAIL;
      exit(error);
    }

    /* output navigation crossing ties */
    for (icrossing = 0; icrossing < project_output.num_crossings; icrossing++) {
      crossing = &(project_output.crossings[icrossing]);
      file1 = &(project_output.files[crossing->file_id_1]);
      file2 = &(project_output.files[crossing->file_id_2]);
      section1 = &(file1->sections[crossing->section_1]);
      section2 = &(file2->sections[crossing->section_2]);
      for (itie = 0; itie < crossing->num_ties; itie++) {
        tie = &(crossing->ties[itie]);
        fprintf(tfp, "TIE %s %s %1d %16.6f %16.6f %13.8f %13.8f %13.8f\n", project_output.files[crossing->file_id_1].path,
                project_output.files[crossing->file_id_2].path, tie->status, section1->snav_time_d[tie->snav_1],
                section2->snav_time_d[tie->snav_2], tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
        fprintf(tfp, "COV %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f\n",
                tie->sigmar1, tie->sigmax1[0], tie->sigmax1[1], tie->sigmax1[2], tie->sigmar2, tie->sigmax2[0],
                tie->sigmax2[1], tie->sigmax2[2], tie->sigmar3, tie->sigmax3[0], tie->sigmax3[1], tie->sigmax3[2]);
      }
    }

    /* output global ties */
    for (ifile = 0; ifile < project_output.num_files; ifile++) {
      file = &project_output.files[ifile];
      for (isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        if (section->global_tie_status != MBNA_TIE_NONE) {
          fprintf(tfp, "GLOBALTIE %s %1d %16.6f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f\n", file->path,
                  section->global_tie_status, section->snav_time_d[section->global_tie_snav],
                  section->offset_x_m, section->offset_y_m, section->offset_z_m,
                  section->xsigma, section->ysigma, section->zsigma);
        }
      }
    }

    fclose(tfp);
  }

  /* write out the new project file */
  if (project_output_set == MB_YES) {
    status = mbnavadjust_write_project(verbose, &project_output, &error);
    if (status == MB_SUCCESS) {
      fprintf(stderr, "Output project written:\n\t%s\n", project_output_path);
      fprintf(stderr, "\t%d files\n\t%d crossings\n\t%d ties\n", project_output.num_files, project_output.num_crossings,
              project_output.num_ties);
    }
    else {
      fprintf(stderr, "Write failure for output project:\n\t%s\n", project_output_path);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      error = MB_ERROR_BAD_USAGE;
      exit(error);
    }

        if (update_datalist == MB_YES) {
            /* update datalist and ancillary files */
            sprintf(filename, "%s/%s.dir/datalist.mb-1", project_output.path, project_output.name);
            if ((tfp = fopen(filename, "w")) != NULL) {
                for (i = 0; i < project_output.num_files; i++) {
                    file1 = &project_output.files[i];
                    for (j = 0; j < file1->num_sections; j++) {
                        fprintf(tfp, "%s/nvs_%4.4d_%4.4d.mb71 71\n", project_output.datadir, file1->id, j);
                    }
                }
                fclose(tfp);
            }
            sprintf(filename, "cd %s/%s.dir ; mbdatalist -Idatalist.mb-1 -O -Z -V", project_output.path, project_output.name);
            shellstatus = system(filename);
            sprintf(filename, "%s/%s.dir/mbgrid.cmd", project_output.path, project_output.name);
            if ((tfp = fopen(filename, "w")) != NULL) {
                fprintf(tfp, "mbgrid -I datalistp.mb-1 \\\n\t-A2 -F5 -N -C2 \\\n\t-O ProjectTopo\n\n");
                fclose(tfp);
            }
        }
    }

  /* check memory */
  if (verbose >= 4)
    status = mb_memory_list(verbose, &error);

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
    fprintf(stderr, "dbg2  Ending status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* end it all */
  exit(error);
}
/*--------------------------------------------------------------------*/
