/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjustmerge.c	4/14/2014
 *    $Id$
 *
 *    Copyright (c) 2014-2014 by
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
 * Author:	D. W. Caress
 * Date:	April 14, 2014
 *
 *
 */

/* source file version string */
static char version_id[] = "$Id$";

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
#define MBNAVADJUSTMERGE_MODE_NONE 	0
#define MBNAVADJUSTMERGE_MODE_ADD 	1
#define MBNAVADJUSTMERGE_MODE_MERGE 	2
#define MBNAVADJUSTMERGE_MODE_COPY 	3
#define MBNAVADJUSTMERGE_MODE_MODIFY 	4
#define NUMBER_MODS_MAX			1000
#define MOD_MODE_NONE 			0
#define MOD_MODE_ADD_CROSSING 		1
#define MOD_MODE_SET_TIE 		2
#define MOD_MODE_SET_TIE_XYZ		3
#define MOD_MODE_SET_TIE_XY		4
#define MOD_MODE_SET_TIE_Z		5
#define MOD_MODE_SET_TIES_XYZ_FILE	6
#define MOD_MODE_SET_TIES_XY_FILE	7
#define MOD_MODE_SET_TIES_Z_FILE	8
#define MOD_MODE_SET_TIES_XYZ_SURVEY	9
#define MOD_MODE_SET_TIES_XY_SURVEY	10
#define MOD_MODE_SET_TIES_Z_SURVEY	11
#define MOD_MODE_SET_TIES_XYZ_BLOCK	12
#define MOD_MODE_SET_TIES_XY_BLOCK	13
#define MOD_MODE_SET_TIES_Z_BLOCK	14
#define MOD_MODE_SET_TIES_ZOFFSET_BLOCK	15
#define MOD_MODE_SKIP_UNSET_CROSSINGS	16

struct mbnavadjust_mod
	{
	int	mode;
	int	survey1;
	int	file1;
	int	section1;
	int	survey2;
	int	file2;
	int	section2;
	double	xoffset;
	double	yoffset;
	double	zoffset;
	};

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "mbnavadjustmerge";
	char help_message[] =  "mbnavadjustmerge merges two existing mbnavadjust projects.\n";
	char usage_message[] = "mbnavadjustmerge --verbose --help\n"
				"\t--input=project_path [--input=project_path --output=project_path\n"
				"\t--add-crossing=file1:section1/file2:section2\n"
				"\t--set-tie=file1:section1/file2:section2/xoffset/yoffset/zoffset\n"
				"\t--set-tie-xyz=file1:section1/file2:section2\n"
				"\t--set-tie-xyonly=file1:section1/file2:section2\n"
				"\t--set-tie-zonly=file1:section1/file2:section2\n"
				"\t--set-ties-xyz-with-file=file\n"
				"\t--set-ties-xyonly-with-file=file\n"
				"\t--set-ties-zonly-with-file=file\n"
				"\t--set-ties-xyz-by-survey=survey\n"
				"\t--set-ties-xyonly-by-survey=survey\n"
				"\t--set-ties-zonly-by-survey=survey\n"
				"\t--set-ties-xyz-by-block=survey1/survey2\n"
				"\t--set-ties-xyonly-by-block=survey1/survey2\n"
				"\t--set-ties-zonly-by-block=survey1/survey2\n"
				"\t--set-ties-zoffset-by-block\n"
				"\t--skip-unset-crossings\n";
	extern char *optarg;
	int	option_index;
	int	errflg = 0;
	int	c;
	int	help = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;

	/* command line option definitions */
	/* mbnavadjustmerge --verbose
	 * 		--help
	 * 		--input=project_path
	 * 		--output=project_path
	 */
	static struct option options[] =
		{
		{"verbose",			no_argument, 		NULL, 		0},
		{"help",			no_argument, 		NULL, 		0},
		{"input",			required_argument, 	NULL, 		0},
		{"output",			required_argument, 	NULL, 		0},
		{"add-crossing",		required_argument, 	NULL, 		0},
		{"set-tie",			required_argument, 	NULL, 		0},
		{"set-tie-xyz",			required_argument, 	NULL, 		0},
		{"set-tie-xyonly",		required_argument, 	NULL, 		0},
		{"set-tie-zonly",		required_argument, 	NULL, 		0},
		{"set-ties-xyz-with-file",	required_argument, 	NULL, 		0},
		{"set-ties-xyonly-with-file",	required_argument, 	NULL, 		0},
		{"set-ties-zonly-with-file",	required_argument, 	NULL, 		0},
		{"set-ties-xyz-with-survey",	required_argument, 	NULL, 		0},
		{"set-ties-xyonly-with-survey",	required_argument, 	NULL, 		0},
		{"set-ties-zonly-with-survey",	required_argument, 	NULL, 		0},
		{"set-ties-xyz-by-block",	required_argument, 	NULL, 		0},
		{"set-ties-xyonly-by-block",	required_argument, 	NULL, 		0},
		{"set-ties-zonly-by-block",	required_argument, 	NULL, 		0},
		{"set-ties-zoffset-by-block",	required_argument, 	NULL, 		0},
		{"skip-unset-crossings",	no_argument, 		NULL, 		0},
		{NULL,				0, 			NULL, 		0}
		};
		
	/* mbnavadjustmerge controls */
	int	mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_NONE;
	mb_path	project_inputbase_path;
	mb_path	project_inputadd_path;
	mb_path	project_output_path;
	int	project_inputbase_set = MB_NO;
	int	project_inputadd_set = MB_NO;
	int	project_output_set = MB_NO;
	struct mbna_project project_inputbase;
	struct mbna_project project_inputadd;
	struct mbna_project project_output;
	
	/* mbnavadjustmerge mod parameters */
	struct mbnavadjust_mod mods[NUMBER_MODS_MAX];
	int	num_mods = 0;
	
	struct mbna_file *file1;
	struct mbna_section *section1;
	struct mbna_file *file2;
	struct mbna_section *section2;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;

	int	nscan;
	int	shellstatus;
	mb_path	command;
	double	mtodeglon, mtodeglat;
	int	found, current_crossing;
	int	imod, icrossing, itie;
	int	i, j, k;
	
	memset(project_inputbase_path, 0, sizeof(mb_path));
	memset(project_inputadd_path, 0, sizeof(mb_path));
	memset(project_output_path, 0, sizeof(mb_path));

	/* process argument list */
	while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
	  switch (c)
		{
		/* long options all return c=0 */
		case 0:
			/* verbose */
			if (strcmp("verbose", options[option_index].name) == 0)
				{
				verbose++;
				}
			
			/* help */
			else if (strcmp("help", options[option_index].name) == 0)
				{
				help = MB_YES;
				}
				
			/*-------------------------------------------------------
			 * Define input and output projects */
			
			/* input */
			else if (strcmp("input", options[option_index].name) == 0)
				{
				if (project_inputbase_set == MB_NO)
					{
					strcpy(project_inputbase_path, optarg);
					project_inputbase_set = MB_YES;
					}
				else if (project_inputadd_set == MB_NO)
					{
					strcpy(project_inputadd_path, optarg);
					project_inputadd_set = MB_YES;
					}
				else
					{
					fprintf(stderr,"Input projects already set:\n\t%s\n\t%s\nProject %s ignored...\n\n",
						project_inputbase_path, project_inputadd_path, optarg);
					}
				}
						
			/* output */
			else if (strcmp("output", options[option_index].name) == 0)
				{
				if (project_output_set == MB_NO)
					{
					strcpy(project_output_path, optarg);
					project_output_set = MB_YES;
					}
				else
					{
					fprintf(stderr,"Output project already set:\n\t%s\nProject %s ignored\n\n",
						project_output_path, optarg);
					}
				}
				
			/*-------------------------------------------------------
			 * add crossing 
				--add-crossing=file1:section1/file2:section2 */
			else if (strcmp("add-crossing", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d:%d/%d:%d",
						       &mods[num_mods].file1,
						       &mods[num_mods].section1,
						       &mods[num_mods].file2,
						       &mods[num_mods].section2)) == 4)
						{
						mods[num_mods].mode = MOD_MODE_ADD_CROSSING;
						num_mods++;
						}
					else if ((nscan = sscanf(optarg, "%d/%d",
								&mods[num_mods].file1,
								&mods[num_mods].file2)) == 2)
						{
						mods[num_mods].section1 = 0;
						mods[num_mods].section2 = 0;
						mods[num_mods].mode = MOD_MODE_ADD_CROSSING;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --add-crossing=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--add-crossing=%s command ignored\n\n",
							optarg);	
					}
				}
			
			/*-------------------------------------------------------
			 * set tie offset values - add tie if needed
				--set-tie=file1:section1/file2:section2/xoffset/yoffset/zoffset
					(xoffset, yoffset, or zoffset can be left unchanged
					by putting */
			else if (strcmp("set-tie", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d:%d/%d:%d/%lf/%lf/%lf",
						       &mods[num_mods].file1,
						       &mods[num_mods].section1,
						       &mods[num_mods].file2,
						       &mods[num_mods].section2,
						       &mods[num_mods].xoffset,
						       &mods[num_mods].yoffset,
						       &mods[num_mods].zoffset)) == 7)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIE;
						num_mods++;
						}
					else if ((nscan = sscanf(optarg, "%d/%d/%lf/%lf/%lf",
								&mods[num_mods].file1,
								&mods[num_mods].file2,
								&mods[num_mods].xoffset,
								&mods[num_mods].yoffset,
								&mods[num_mods].zoffset)) == 5)
						{
						mods[num_mods].section1 = 0;
						mods[num_mods].section2 = 0;
						mods[num_mods].mode = MOD_MODE_SET_TIE;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-tie=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-tie=%s command ignored\n\n",
							optarg);	
					}
				}
			
			/*-------------------------------------------------------
			 * set tie mode
				--set-tie-xyz=file1:section1/file2:section2
				--set-tie-xyonly=file1:section1/file2:section2
				--set-tie-zonly=file1:section1/file2:section2 */
			else if (strcmp("set-tie-xyz", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d:%d/%d:%d",
						       &mods[num_mods].file1,
						       &mods[num_mods].section1,
						       &mods[num_mods].file2,
						       &mods[num_mods].section2)) == 4)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIE_XYZ;
						num_mods++;
						}
					else if ((nscan = sscanf(optarg, "%d/%d",
								&mods[num_mods].file1,
								&mods[num_mods].file2)) == 2)
						{
						mods[num_mods].section1 = 0;
						mods[num_mods].section2 = 0;
						mods[num_mods].mode = MOD_MODE_SET_TIE_XYZ;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-tie-xyz=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-tie-xyz=%s command ignored\n\n",
							optarg);	
					}
				}
			else if (strcmp("set-tie-xyonly", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d:%d/%d:%d",
						       &mods[num_mods].file1,
						       &mods[num_mods].section1,
						       &mods[num_mods].file2,
						       &mods[num_mods].section2)) == 4)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIE_XY;
						num_mods++;
						}
					else if ((nscan = sscanf(optarg, "%d/%d",
								&mods[num_mods].file1,
								&mods[num_mods].file2)) == 2)
						{
						mods[num_mods].section1 = 0;
						mods[num_mods].section2 = 0;
						mods[num_mods].mode = MOD_MODE_SET_TIE_XY;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-tie-xy=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-tie-xy=%s command ignored\n\n",
							optarg);	
					}
				}
			else if (strcmp("set-tie-zonly", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d:%d/%d:%d",
						       &mods[num_mods].file1,
						       &mods[num_mods].section1,
						       &mods[num_mods].file2,
						       &mods[num_mods].section2)) == 4)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIE_Z;
						num_mods++;
						}
					else if ((nscan = sscanf(optarg, "%d/%d",
								&mods[num_mods].file1,
								&mods[num_mods].file2)) == 2)
						{
						mods[num_mods].section1 = 0;
						mods[num_mods].section2 = 0;
						mods[num_mods].mode = MOD_MODE_SET_TIE_Z;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-tie-z=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-tie-z=%s command ignored\n\n",
							optarg);	
					}
				}
			
			/*-------------------------------------------------------
			 * set mode of all ties with a file
				--set-ties-xyz-with-file=file
				--set-ties-xyonly-with-file=file
				--set-ties-zonly-with-file=file */
			else if (strcmp("set-ties-xyz-with-file", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d",
						       &mods[num_mods].file1)) == 1)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIES_XYZ_FILE;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-ties-xyz-with-file=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-ties-xyz-with-file=%s command ignored\n\n",
							optarg);	
					}
				}
			else if (strcmp("set-ties-xyonly-with-file", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d",
						       &mods[num_mods].file1)) == 1)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIES_XY_FILE;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-ties-xyonly-with-file=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-ties-xyonly-with-file=%s command ignored\n\n",
							optarg);	
					}
				}
			else if (strcmp("set-ties-xyz-with-file", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d",
						       &mods[num_mods].file1)) == 1)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIES_Z_FILE;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-ties-xyz-with-file=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-ties-xyz-with-file=%s command ignored\n\n",
							optarg);	
					}
				}

			/*-------------------------------------------------------
			 * set mode of all ties with a survey
				--set-ties-xyz-by-survey=survey
				--set-ties-xyonly-by-survey=survey
				--set-ties-zonly-by-survey=survey */
			else if (strcmp("set-ties-xyz-with-survey", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d",
						       &mods[num_mods].survey1)) == 1)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIES_XYZ_SURVEY;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-ties-xyz-with-survey=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-ties-xyz-with-survey=%s command ignored\n\n",
							optarg);	
					}
				}
			else if (strcmp("set-ties-xyonly-with-survey", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d",
						       &mods[num_mods].survey1)) == 1)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIES_XY_SURVEY;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-ties-xyonly-with-survey=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-ties-xyonly-with-survey=%s command ignored\n\n",
							optarg);	
					}
				}
			else if (strcmp("set-ties-xyz-with-survey", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d",
						       &mods[num_mods].survey1)) == 1)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIES_Z_SURVEY;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-ties-xyz-with-survey=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-ties-xyz-with-survey=%s command ignored\n\n",
							optarg);	
					}
				}
			
			/*-------------------------------------------------------
			 * set mode of all ties between two surveys
				--set-ties-xyz-by-block=survey1/survey2
				--set-ties-xyonly-by-block=survey1/survey2
				--set-ties-zonly-by-block=survey1/survey2 */
			else if (strcmp("set-ties-xyz-by-block", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d/%d",
						       &mods[num_mods].survey1, &mods[num_mods].survey2)) == 2)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIES_XYZ_BLOCK;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-ties-xyz-with-block=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-ties-xyz-with-block=%s command ignored\n\n",
							optarg);	
					}
				}
			else if (strcmp("set-ties-xyonly-by-block", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d/%d",
						       &mods[num_mods].survey1, &mods[num_mods].survey2)) == 2)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIES_XY_BLOCK;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-ties-xyonly-with-block=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-ties-xyonly-with-block=%s command ignored\n\n",
							optarg);	
					}
				}
			else if (strcmp("set-ties-xyz-by-block", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d/%d",
						       &mods[num_mods].survey1, &mods[num_mods].survey2)) == 2)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIES_Z_BLOCK;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-ties-xyz-with-block=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-ties-xyz-with-block=%s command ignored\n\n",
							optarg);	
					}
				}
			
			/*-------------------------------------------------------
			 * set zoffset of all ties between two surveys
				--set-ties-zoffset-by-block=survey1/survey2/zoffset */
			else if (strcmp("set-ties-zoffset-by-block", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					if ((nscan = sscanf(optarg, "%d/%d/%lf",
						       &mods[num_mods].survey1, &mods[num_mods].survey2, &mods[num_mods].zoffset)) == 3)
						{
						mods[num_mods].mode = MOD_MODE_SET_TIES_ZOFFSET_BLOCK;
						num_mods++;
						}
					else
						{
						fprintf(stderr,"Failure to parse --set-ties-zoffset-with-block=%s\n\tmod command ignored\n\n",
							optarg);	
						}
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\t--set-ties-xyz-with-block=%s command ignored\n\n",
							optarg);	
					}
				}
				
			/*-------------------------------------------------------
			 * set all crossings without ties in the input project(s) to be skipped
				--skip-unset-crossings */
			else if (strcmp("skip-unset-crossings", options[option_index].name) == 0)
				{
				if (num_mods < NUMBER_MODS_MAX)
					{
					mods[num_mods].mode = MOD_MODE_SKIP_UNSET_CROSSINGS;
					num_mods++;
					}
				else
					{
					fprintf(stderr,"Maximum number of mod commands reached:\n\tskip-unset-crossings command ignored\n\n");	
					}
				}
			
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Source File Version %s\n",version_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",version_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:                    %d\n",verbose);
		fprintf(stderr,"dbg2       help:                       %d\n",help);
		fprintf(stderr,"dbg2       project_inputbase_set:      %d\n",project_inputbase_set);
		fprintf(stderr,"dbg2       project_inputbase_path:     %s\n",project_inputbase_path);
		fprintf(stderr,"dbg2       project_inputadd_set:       %d\n",project_inputadd_set);
		fprintf(stderr,"dbg2       project_inputadd_path:      %s\n",project_inputadd_path);
		fprintf(stderr,"dbg2       project_output_set:         %d\n",project_output_set);
		fprintf(stderr,"dbg2       project_output_path:        %s\n",project_output_path);
		fprintf(stderr,"dbg2       num_mods:                   %d\n",num_mods);
		for (i=0;i<num_mods;i++)
			{
			fprintf(stderr,"dbg2       mods[%d]: %d  %d %d %d   %d %d %d  %f %f %f\n",
				i, mods[i].mode,
				mods[i].survey1, mods[i].file1, mods[i].section1,
				mods[i].survey1, mods[i].file1, mods[i].section1,
				mods[i].xoffset, mods[i].yoffset, mods[i].zoffset);
			}
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}
		
	/* figure out mbnavadjust project merge mode */
	if (project_inputbase_set == MB_NO)
		{
		fprintf(stderr,"No input base project has been set.\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}
	else if (project_inputbase_set == MB_YES
		&& project_inputadd_set == MB_NO
		&& project_output_set == MB_NO)
		{
		strcpy(project_output_path, project_inputbase_path);
		project_output_set = MB_YES;
		mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_MODIFY;
		}
	else if (project_inputbase_set == MB_YES
		&& project_inputadd_set == MB_NO
		&& project_output_set == MB_YES)
		{
		project_output_set = MB_YES;
		mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_COPY;
		}
	else if (project_inputbase_set == MB_YES
		&& project_inputadd_set == MB_YES
		&& project_output_set == MB_NO)
		{
		strcpy(project_output_path, project_inputbase_path);
		project_output_set = MB_YES;
		mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_ADD;
		}
	else if (project_inputbase_set == MB_YES
		&& project_inputadd_set == MB_YES
		&& project_output_set == MB_YES
		&& strcmp(project_output_path, project_inputadd_path) == 0)
		{
		fprintf(stderr,"The output project:\n\t%s\nis identical to the input add project:\n\t%s\n",
			project_output_path, project_inputadd_path);
		fprintf(stderr,"The output project must either be the input base project or a new project.\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}
	else if (project_inputbase_set == MB_YES
		&& project_inputadd_set == MB_YES
		&& project_output_set == MB_YES
		&& strcmp(project_output_path, project_inputbase_path) == 0)
		{
		mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_ADD;
		}
	else if (project_inputbase_set == MB_YES
		&& project_inputadd_set == MB_YES
		&& project_output_set == MB_YES)
		{
		mbnavadjustmerge_mode = MBNAVADJUSTMERGE_MODE_MERGE;
		}
		
	/* initialize the project structures */
	memset(&project_inputbase, sizeof(struct mbna_project), 0);
	memset(&project_inputadd, sizeof(struct mbna_project), 0);
	memset(&project_output, sizeof(struct mbna_project), 0);
		
	/* if merging two projects then read the first, create new output project */
	if (mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_MERGE
		|| mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_COPY)
		{
		/* read the input base project */
		status = mbnavadjust_read_project(verbose, project_inputbase_path,
					&project_inputbase, &error);
		if (status == MB_SUCCESS)
			{
			fprintf(stderr,"\nInput base project loaded:\n\t%s\n", project_inputbase_path);
			fprintf(stderr,"\t%d files\n\t%d crossings\n\t%d ties\n",
				project_inputbase.num_files, project_inputbase.num_crossings, project_inputbase.num_ties);
			}
		else
			{
			fprintf(stderr,"Load failure for input base project:\n\t%s\n",
				project_inputbase_path);
			fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
			error = MB_ERROR_BAD_USAGE;
			exit(error);
			}

		status = mbnavadjust_new_project(verbose, project_output_path,
						project_inputbase.section_length,
                        			project_inputbase.section_soundings,
                        			project_inputbase.cont_int,
                        			project_inputbase.col_int,
                        			project_inputbase.tick_int,
                        			project_inputbase.label_int,
                        			project_inputbase.decimation,
                        			project_inputbase.smoothing,
                        			project_inputbase.zoffsetwidth,
						&project_output, &error);
		if (status == MB_SUCCESS)
			{
			fprintf(stderr,"\nOutput project created:\n\t%s\n", project_output_path);
			}
		else
			{
			fprintf(stderr,"Creation failure for output project:\n\t%s\n",
				project_output_path);
			fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
			error = MB_ERROR_BAD_USAGE;
			exit(error);
			}
		
		/* copy the input base project to the output project */
		//project_output.open = project_inputbase.open;
		//strcpy(project_output.name, project_inputbase.name);
		//strcpy(project_output.path, project_inputbase.path);
		//strcpy(project_output.home, project_inputbase.home);
		//strcpy(project_output.datadir, project_inputbase.datadir);
		//strcpy(project_output.logfile, project_inputbase.logfile);
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
		//project_output.inversion = project_inputbase.inversion;
		//project_output.modelplot = project_inputbase.modelplot;
		//project_output.modelplot_style = project_inputbase.modelplot_style;
		//project_output.logfp;
		
		/* allocate and copy the files */
		if (project_output.num_files > 0)
			{
			status = mb_mallocd(verbose, __FILE__, __LINE__,
						sizeof(struct mbna_file) * (project_output.num_files),
						(void **)&project_output.files,&error);
			if (status == MB_SUCCESS && project_output.files != NULL)
				{
				/* copy the file data */
				project_output.num_files_alloc = project_output.num_files;
				memcpy(project_output.files, project_inputbase.files,
				       project_output.num_files * sizeof(struct mbna_file));
		
				/* copy the sections in the files */
				for (i=0;i<project_output.num_files && status == MB_SUCCESS;i++)
					{
					/* allocate and then copy the sections in this file */
					project_output.files[i].sections = NULL;
					if (project_output.files[i].num_sections > 0)
						{
						status = mb_mallocd(verbose, __FILE__, __LINE__,
								sizeof(struct mbna_section) * (project_output.files[i].num_sections),
								(void **)&project_output.files[i].sections, &error);
						if (status == MB_SUCCESS && project_output.files[i].sections != NULL)
							{
							project_output.files[i].num_sections_alloc = project_output.files[i].num_sections;
							memcpy(project_output.files[i].sections, project_inputbase.files[i].sections,
							       project_output.files[i].num_sections * sizeof(struct mbna_section));
							}
						}
					}
				}
			else
				{
				project_output.num_files_alloc = 0;
				status = MB_FAILURE;
				error = MB_ERROR_MEMORY_FAIL;
				}
			}
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
			
		/* allocate and copy the crossings */
		if (project_output.num_crossings > 0)
			{
			status = mb_mallocd(verbose, __FILE__, __LINE__,
						sizeof(struct mbna_crossing) * (project_output.num_crossings),
						(void **)&project_output.crossings,&error);
			if (status == MB_SUCCESS && project_output.crossings != NULL)
				{
				project_output.num_crossings_alloc = project_output.num_crossings;
				memcpy(project_output.crossings, project_inputbase.crossings,
				       project_output.num_crossings * sizeof(struct mbna_crossing));
				}
			else
				{
				project_output.num_crossings_alloc = 0;
				status = MB_FAILURE;
				error = MB_ERROR_MEMORY_FAIL;
				}
			}
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
		
		/* now concatenate the log.txt from the input project with the log.txt for the new output project */
		sprintf(command,"mv %s/log.txt %s/logorg.txt", project_output.datadir, project_output.datadir);
		//fprintf(stderr, "Executing in shell: %s\n", command);
		shellstatus = system(command);
		sprintf(command,"cat %s/log.txt %s/logorg.txt > %s/log.txt",
			project_inputbase.datadir, project_output.datadir, project_output.datadir);
		//fprintf(stderr, "Executing in shell: %s\n", command);
		shellstatus = system(command);
		
		/* now fix the data file paths to be relative to the new project location */
		for (i=0;i<project_output.num_files;i++)
			{
			strcpy(project_output.files[i].file, project_output.files[i].path);
			status = mb_get_relative_path(verbose, project_output.files[i].file, project_output.path, &error);
			}		
		
		/* now copy the actual data files from the input project to the new output project */
		for (i=0;i<project_output.num_files;i++)
			{
			/* copy the file navigation */
			sprintf(command,"cp %s/nvs_%4.4d.mb166 %s", project_inputbase.datadir, i, project_output.datadir);
			//fprintf(stderr, "Executing in shell: %s\n", command);
			shellstatus = system(command);
				
			/* copy all the section files */
			for (j=0;j<project_output.files[i].num_sections;j++)
				{
				/* copy the section file */
				sprintf(command,"cp %s/nvs_%4.4d_%4.4d.mb71 %s", project_inputbase.datadir, i, j, project_output.datadir);
				//fprintf(stderr, "Executing in shell: %s\n", command);
				shellstatus = system(command);
				}
			}		
		fprintf(stderr,"\nCopied input base project to output project:\n\t%s\n", project_output_path);
		fprintf(stderr,"\t%d files\n\t%d crossings\n\t%d ties\n",
			project_output.num_files, project_output.num_crossings, project_output.num_ties);
		}
		
	/* else if adding the second project to the first, or just modifying the first,
		open the first as the output project */
	else if (mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_ADD
			|| mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_MODIFY)
		{
		/* read the input base project in as the output project */
		status = mbnavadjust_read_project(verbose, project_output_path,
					&project_output, &error);
		if (status == MB_SUCCESS)
			{
			fprintf(stderr,"\nInput base project loaded as output:\n\t%s\n", project_output_path);
			fprintf(stderr,"\t%d files\n\t%d crossings\n\t%d ties\n",
				project_output.num_files, project_output.num_crossings, project_output.num_ties);
			}
		else
			{
			fprintf(stderr,"Load failure for input base project (which is also the intended output):\n\t%s\n",
				project_output_path);
			fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
			error = MB_ERROR_BAD_USAGE;
			exit(error);
			}
		}

	/* if adding or merging projects read the input add project
		then add the input add project to the output project */
	if (mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_ADD
			|| mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_MERGE)
		{
		status = mbnavadjust_read_project(verbose, project_inputadd_path,
					&project_inputadd, &error);
		if (status == MB_SUCCESS)
			{
			fprintf(stderr,"Input add project loaded:\n\t%s\n", project_inputadd_path);
			fprintf(stderr,"\t%d files\n\t%d crossings\n\t%d ties\n",
				project_inputadd.num_files, project_inputadd.num_crossings, project_inputadd.num_ties);
			}
		else
			{
			fprintf(stderr,"Load failure for input add project:\n\t%s\n",
				project_inputadd_path);
			fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
			error = MB_ERROR_BAD_USAGE;
			exit(error);
			}

		/* allocate space for additional files */
		if (project_inputadd.num_files > 0)
			{
			/* allocate space for the files in project_inputadd */
			status = mb_reallocd(verbose, __FILE__, __LINE__,
						sizeof(struct mbna_file) * (project_output.num_files + project_inputadd.num_files),
							(void **)&project_output.files,&error);
			if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
			}
	
		/* copy the file data from project_inputadd to project_output */
		project_output.num_files_alloc = project_output.num_files + project_inputadd.num_files;
		memcpy(&project_output.files[project_output.num_files], project_inputadd.files,
		       project_inputadd.num_files * sizeof(struct mbna_file));
	
		/* copy the sections in the files */
		for (i=0;i<project_inputadd.num_files && status == MB_SUCCESS;i++)
			{
			j = project_output.num_files + i;
			project_output.files[j].id += project_output.num_files;
			project_output.files[j].block += project_output.num_blocks;
			
			/* allocate and then copy the sections in this file */
			project_output.files[j].sections = NULL;
			if (project_output.files[j].num_sections > 0)
				{
				status = mb_mallocd(verbose, __FILE__, __LINE__,
						sizeof(struct mbna_section) * (project_output.files[j].num_sections),
						(void **)&project_output.files[j].sections, &error);
				if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
				if (project_output.files[j].sections != NULL)
					{
					project_output.files[j].num_sections_alloc = project_output.files[j].num_sections;
					memcpy(project_output.files[j].sections, project_inputadd.files[i].sections,
					       project_output.files[j].num_sections * sizeof(struct mbna_section));
					}
				}
			for (k=0;k<project_output.files[j].num_sections;k++)
				{
				project_output.files[j].sections[k].global_start_ping += project_output.num_pings;
				project_output.files[j].sections[k].global_start_snav += project_output.num_snavs;
				}
			}
			
		/* allocate and copy the crossings */
		if (project_inputadd.num_crossings > 0)
			{
			status = mb_reallocd(verbose, __FILE__, __LINE__,
						sizeof(struct mbna_crossing) * (project_output.num_crossings + project_inputadd.num_crossings),
						(void **)&project_output.crossings,&error);
			if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
			if (project_output.crossings != NULL)
				{
				project_output.num_crossings_alloc = project_output.num_crossings + project_inputadd.num_crossings;
				memcpy(&project_output.crossings[project_output.num_crossings], project_inputadd.crossings,
				       project_inputadd.num_crossings * sizeof(struct mbna_crossing));
				}
			else
				{
				project_output.num_crossings_alloc = 0;
				status = MB_FAILURE;
				error = MB_ERROR_MEMORY_FAIL;
				}
			for (i=0;i<project_inputadd.num_crossings;i++)
				{
				j = project_output.num_crossings + i;
				project_output.crossings[j].file_id_1 = project_inputadd.crossings[i].file_id_1 + project_output.num_files;
				project_output.crossings[j].file_id_2 = project_inputadd.crossings[i].file_id_2 + project_output.num_files;
				for (k=0;k<project_output.crossings[j].num_ties;k++)
					{
					project_output.crossings[j].ties[k].block_1 += project_output.num_blocks;
					project_output.crossings[j].ties[k].block_2 += project_output.num_blocks;
					}
				}
			}
		if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
		
		/* now concatenate the log.txt from the inputadd project with the log.txt for the new output project */
		sprintf(command,"cat %s/log.txt %s/logorg.txt > %s/log.txt",
			project_inputadd.datadir, project_output.datadir, project_output.datadir);
		//fprintf(stderr, "Executing in shell: %s\n", command);
		shellstatus = system(command);
		
		/* now fix the data file paths to be relative to the new project location */
		for (i=0;i<project_inputadd.num_files;i++)
			{
			k = project_output.num_files + i;
			strcpy(project_output.files[k].file, project_output.files[k].path);
			status = mb_get_relative_path(verbose, project_output.files[k].file, project_output.path, &error);
			}		
		
		/* now copy the actual data files from the input project to the new output project */
		for (i=0;i<project_inputadd.num_files;i++)
			{
			k = project_output.num_files + i;

			/* copy the file navigation */
			sprintf(command,"cp %s/nvs_%4.4d.mb166 %s/nvs_%4.4d.mb166", project_inputadd.datadir, i, project_output.datadir, k);
			//fprintf(stderr, "Executing in shell: %s\n", command);
			shellstatus = system(command);
				
			/* copy all the section files */
			for (j=0;j<project_inputadd.files[i].num_sections;j++)
				{
				/* copy the section file */
				sprintf(command,"cp %s/nvs_%4.4d_%4.4d.mb71 %s/nvs_%4.4d_%4.4d.mb71", project_inputadd.datadir, i, j, project_output.datadir, k, j);
				//fprintf(stderr, "Executing in shell: %s\n", command);
				shellstatus = system(command);
				}
			}		
		fprintf(stderr,"\nCopied input add project to output project:\n\t%s\n", project_output_path);
		fprintf(stderr,"\t%d files\n\t%d crossings\n\t%d ties\n",
			project_output.num_files, project_output.num_crossings, project_output.num_ties);
	
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
	for (imod=0;imod<num_mods;imod++)
		{
		switch (mods[imod].mode)
			{
			case MOD_MODE_ADD_CROSSING:
fprintf(stderr,"\nCommand add-crossing=%4.4d:%4.4d/%4.4d:%4.4d\n",mods[imod].file1,mods[imod].section1,mods[imod].file2,mods[imod].section2);
				
				/* check to see if this crossing already exists */
				found = MB_NO;
				for (icrossing=0;icrossing<project_output.num_crossings && found == MB_NO;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2
						&& crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2)
						{
						found = MB_YES;
						}
					else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2
						&& crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2)
						{
						found = MB_YES;
						}
					}
					
				/* if the crossing does not exist, create it */
				if (found == MB_NO)
					{
					/* allocate mbna_crossing array if needed */
					if (project_output.num_crossings_alloc <= project_output.num_crossings)
					    {
					    project_output.crossings = (struct mbna_crossing *) realloc(project_output.crossings,
							    sizeof(struct mbna_crossing) * (project_output.num_crossings_alloc + ALLOC_NUM));
					    if (project_output.crossings != NULL)
						    project_output.num_crossings_alloc += ALLOC_NUM;
					    else
						{
						status = MB_FAILURE;
						error = MB_ERROR_MEMORY_FAIL;
						}
					    }
	
					/* add crossing to list */
					crossing = (struct mbna_crossing *) &project_output.crossings[project_output.num_crossings];
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
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

fprintf(stderr,"Added crossing: %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n",
current_crossing,
file1->block, crossing->file_id_1, crossing->section_1,
file2->block, crossing->file_id_2, crossing->section_2);
					}
				break;
			
			case MOD_MODE_SET_TIE:
fprintf(stderr,"\nCommand set-tie=%4.4d:%4.4d/%4.4d:%4.4d/%.3f/%.3f/%.3f\n",
mods[imod].file1,mods[imod].section1,mods[imod].file2,mods[imod].section2,
mods[imod].xoffset,mods[imod].yoffset,mods[imod].zoffset);
				
				/* check to see if this crossing already exists */
				found = MB_NO;
				for (icrossing=0;icrossing<project_output.num_crossings && found == MB_NO;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2
						&& crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2)
						{
						found = MB_YES;
						current_crossing = icrossing;
						crossing = (struct mbna_crossing *) &project_output.crossings[icrossing];
						}
					else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2
						&& crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2)
						{
						found = MB_YES;
						current_crossing = icrossing;
						crossing = (struct mbna_crossing *) &project_output.crossings[icrossing];
						}
					}
					
				/* if the crossing does not exist, create it */
				if (found == MB_NO)
					{
					/* allocate mbna_crossing array if needed */
					if (project_output.num_crossings_alloc <= project_output.num_crossings)
					    {
					    project_output.crossings = (struct mbna_crossing *) realloc(project_output.crossings,
							    sizeof(struct mbna_crossing) * (project_output.num_crossings_alloc + ALLOC_NUM));
					    if (project_output.crossings != NULL)
						    project_output.num_crossings_alloc += ALLOC_NUM;
					    else
						{
						status = MB_FAILURE;
						error = MB_ERROR_MEMORY_FAIL;
						}
					    }
	
					/* add crossing to list */
					current_crossing = project_output.num_crossings;
					crossing = (struct mbna_crossing *) &project_output.crossings[current_crossing];
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
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

fprintf(stderr,"Added crossing: %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n",
current_crossing,
file1->block, crossing->file_id_1, crossing->section_1,
file2->block, crossing->file_id_2, crossing->section_2);
					}
				
				/* if the tie does not exist, create it */
				if (crossing->num_ties == 0)
					{
					/* add tie and set number */
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					section1 = (struct mbna_section *) &file1->sections[crossing->section_1];
					section2 = (struct mbna_section *) &file2->sections[crossing->section_2];
					crossing->num_ties++;
					project_output.num_ties++;
					tie = &crossing->ties[0];
		
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						{
						project_output.num_crossings_analyzed++;
						if (crossing->truecrossing == MB_YES)
							project_output.num_truecrossings_analyzed++;
						}
					crossing->status = MBNA_CROSSING_STATUS_SET;
		
					/* use midpoint nav points */
					tie->snav_1 = section1->num_snav / 2;
					tie->snav_2 = section2->num_snav / 2;

fprintf(stderr,"Added tie: %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d\n",
current_crossing, 0,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2);
					}
					
				/* set the tie parameters */
				for (itie=0;itie<crossing->num_ties;itie++)
					{
					tie = &crossing->ties[itie];
					tie->status = MBNA_TIE_XYZ;
					tie->snav_1_time_d = section1->snav_time_d[tie->snav_1];
					tie->snav_2_time_d = section2->snav_time_d[tie->snav_2];
					mb_coor_scale(verbose,0.25 * (section1->latmin + section1->latmax + section2->latmin + section2->latmax),
							&mtodeglon,&mtodeglat);
					tie->offset_x = mods[imod].xoffset * mtodeglon;
					tie->offset_y = mods[imod].yoffset * mtodeglat;
					tie->offset_x_m = mods[imod].xoffset;
					tie->offset_y_m = mods[imod].yoffset;
					tie->offset_z_m = mods[imod].zoffset;
					tie->sigmar1 = 10.0;
					tie->sigmax1[0] = 1.0;
					tie->sigmax1[1] = 0.0;
					tie->sigmax1[2] = 0.0;
					tie->sigmar2 = 10.0;
					tie->sigmax2[0] = 0.0;
					tie->sigmax2[1] = 1.0;
					tie->sigmax2[2] = 0.0;
					tie->sigmar3 = 10.0;
					tie->sigmax3[0] = 0.0;
					tie->sigmax3[1] = 0.0;
					tie->sigmax3[2] = 1.0;
					tie->inversion_offset_x = section2->snav_lon_offset[tie->snav_2]
								- section1->snav_lon_offset[tie->snav_1];
					tie->inversion_offset_y = section2->snav_lat_offset[tie->snav_2]
								- section1->snav_lat_offset[tie->snav_1];
					tie->inversion_offset_x_m = tie->inversion_offset_x / mtodeglon;
					tie->inversion_offset_y_m = tie->inversion_offset_y / mtodeglat;
					tie->inversion_offset_z_m = section2->snav_z_offset[tie->snav_2]
								- section1->snav_z_offset[tie->snav_1];
					tie->inversion_status = MBNA_INVERSION_NONE;
					if (project_output.inversion == MBNA_INVERSION_CURRENT)
						project_output.inversion = MBNA_INVERSION_OLD;
		
					/* reset tie counts for snavs */
					section1->snav_num_ties[tie->snav_1]++;
					section2->snav_num_ties[tie->snav_2]++;

fprintf(stderr,"Set tie offsets:       %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
current_crossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
					}
				
				break;
			
			case MOD_MODE_SET_TIE_XYZ:
fprintf(stderr,"\nCommand set-tie-xyz=%4.4d:%4.4d/%4.4d:%4.4d\n",
mods[imod].file1,mods[imod].section1,mods[imod].file2,mods[imod].section2);
				
				/* check to see if this crossing already exists */
				found = MB_NO;
				for (icrossing=0;icrossing<project_output.num_crossings && found == MB_NO;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2
						&& crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2)
						{
						found = MB_YES;
						current_crossing = icrossing;
						crossing = (struct mbna_crossing *) &project_output.crossings[icrossing];
						}
					else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2
						&& crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2)
						{
						found = MB_YES;
						current_crossing = icrossing;
						crossing = (struct mbna_crossing *) &project_output.crossings[icrossing];
						}
					}
					
				/* set the tie parameters */
				if (found == MB_YES)
					{
					for (itie=0;itie<crossing->num_ties;itie++)
						{
						tie = &crossing->ties[itie];
						tie->status = MBNA_TIE_XYZ;

fprintf(stderr,"Set tie mode XYZ:      %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
current_crossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
						}
					}
					
				break;
			
			case MOD_MODE_SET_TIE_XY:
fprintf(stderr,"\nCommand set-tie-xy=%4.4d:%4.4d/%4.4d:%4.4d\n",
mods[imod].file1,mods[imod].section1,mods[imod].file2,mods[imod].section2);
				
				/* check to see if this crossing already exists */
				found = MB_NO;
				for (icrossing=0;icrossing<project_output.num_crossings && found == MB_NO;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2
						&& crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2)
						{
						found = MB_YES;
						current_crossing = icrossing;
						crossing = (struct mbna_crossing *) &project_output.crossings[icrossing];
						}
					else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2
						&& crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2)
						{
						found = MB_YES;
						current_crossing = icrossing;
						crossing = (struct mbna_crossing *) &project_output.crossings[icrossing];
						}
					}
					
				/* set the tie parameters */
				if (found == MB_YES)
					{
					for (itie=0;itie<crossing->num_ties;itie++)
						{
						tie = &crossing->ties[itie];
						tie->status = MBNA_TIE_XY;

fprintf(stderr,"Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
current_crossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
						}
					}
					
				break;
			
			case MOD_MODE_SET_TIE_Z:
fprintf(stderr,"\nCommand set-tie-z=%4.4d:%4.4d/%4.4d:%4.4d\n",
mods[imod].file1,mods[imod].section1,mods[imod].file2,mods[imod].section2);
				
				/* check to see if this crossing already exists */
				found = MB_NO;
				for (icrossing=0;icrossing<project_output.num_crossings && found == MB_NO;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if (crossing->file_id_2 == mods[imod].file1 && crossing->file_id_1 == mods[imod].file2
						&& crossing->section_2 == mods[imod].section1 && crossing->section_1 == mods[imod].section2)
						{
						found = MB_YES;
						current_crossing = icrossing;
						crossing = (struct mbna_crossing *) &project_output.crossings[icrossing];
						}
					else if (crossing->file_id_1 == mods[imod].file1 && crossing->file_id_2 == mods[imod].file2
						&& crossing->section_1 == mods[imod].section1 && crossing->section_2 == mods[imod].section2)
						{
						found = MB_YES;
						current_crossing = icrossing;
						crossing = (struct mbna_crossing *) &project_output.crossings[icrossing];
						}
					}
					
				/* set the tie parameters */
				if (found == MB_YES)
					{
					for (itie=0;itie<crossing->num_ties;itie++)
						{
						tie = &crossing->ties[itie];
						tie->status = MBNA_TIE_Z;

fprintf(stderr,"Set tie mode Z-only:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
current_crossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
						}
					}
					
				break;
			
			case MOD_MODE_SET_TIES_XYZ_FILE:
fprintf(stderr,"\nCommand set-ties-xyz-with-file=%4.4d\n", mods[imod].file1);

				/* loop over all crossings looking for ones with specified file, then set the tie modes */
				for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if (crossing->file_id_1 == mods[imod].file1 || crossing->file_id_2 == mods[imod].file2)
						{
						for (itie=0;itie<crossing->num_ties;itie++)
							{
							tie = &crossing->ties[itie];
							tie->status = MBNA_TIE_XYZ;

fprintf(stderr,"Set tie mode XYZ:      %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
icrossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
							}
						}
					}
				break;
			
			case MOD_MODE_SET_TIES_XY_FILE:
fprintf(stderr,"\nCommand set-ties-xy-with-file=%4.4d\n", mods[imod].file1);

				/* loop over all crossings looking for ones with specified file, then set the tie modes */
				for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if (crossing->file_id_1 == mods[imod].file1 || crossing->file_id_2 == mods[imod].file2)
						{
						for (itie=0;itie<crossing->num_ties;itie++)
							{
							tie = &crossing->ties[itie];
							tie->status = MBNA_TIE_XY;

fprintf(stderr,"Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
icrossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
							}
						}
					}
				break;
			
			case MOD_MODE_SET_TIES_Z_FILE:
fprintf(stderr,"\nCommand set-ties-z-with-file=%4.4d\n", mods[imod].file1);

				/* loop over all crossings looking for ones with specified file, then set the tie modes */
				for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if (crossing->file_id_1 == mods[imod].file1 || crossing->file_id_2 == mods[imod].file2)
						{
						for (itie=0;itie<crossing->num_ties;itie++)
							{
							tie = &crossing->ties[itie];
							tie->status = MBNA_TIE_Z;

fprintf(stderr,"Set tie mode Z-only:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
icrossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
							}
						}
					}
				break;
			
			case MOD_MODE_SET_TIES_XYZ_SURVEY:
fprintf(stderr,"\nCommand set-ties-xyz-with-survey=%2.2d\n", mods[imod].survey1);

				/* loop over all crossings looking for ones with specified survey, then set the tie modes */
				for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if (project_output.files[crossing->file_id_1].block == mods[imod].survey1
						|| project_output.files[crossing->file_id_2].block == mods[imod].survey1)
						{
						for (itie=0;itie<crossing->num_ties;itie++)
							{
							tie = &crossing->ties[itie];
							tie->status = MBNA_TIE_XYZ;

fprintf(stderr,"Set tie mode XYZ:      %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
icrossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
							}
						}
					}
				break;
			
			case MOD_MODE_SET_TIES_XY_SURVEY:
fprintf(stderr,"\nCommand set-ties-xy-with-survey=%2.2d\n", mods[imod].survey1);

				/* loop over all crossings looking for ones with specified survey, then set the tie modes */
				for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if (project_output.files[crossing->file_id_1].block == mods[imod].survey1
						|| project_output.files[crossing->file_id_2].block == mods[imod].survey1)
						{
						for (itie=0;itie<crossing->num_ties;itie++)
							{
							tie = &crossing->ties[itie];
							tie->status = MBNA_TIE_XY;

fprintf(stderr,"Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
icrossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
							}
						}
					}
				break;
			
			case MOD_MODE_SET_TIES_Z_SURVEY:
fprintf(stderr,"\nCommand set-ties-z-with-survey=%2.2d\n", mods[imod].survey1);

				/* loop over all crossings looking for ones with specified survey, then set the tie modes */
				for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if (project_output.files[crossing->file_id_1].block == mods[imod].survey1
						|| project_output.files[crossing->file_id_2].block == mods[imod].survey1)
						{
						for (itie=0;itie<crossing->num_ties;itie++)
							{
							tie = &crossing->ties[itie];
							tie->status = MBNA_TIE_Z;

fprintf(stderr,"Set tie mode Z-only:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
icrossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
							}
						}
					}
				break;
			
			case MOD_MODE_SET_TIES_XYZ_BLOCK:
fprintf(stderr,"\nCommand set-ties-xyz-by-block=%2.2d/%2.2d\n", mods[imod].survey1, mods[imod].survey2);

				/* loop over all crossings looking for ones with specified surveys, then set the tie modes */
				for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if ((project_output.files[crossing->file_id_1].block == mods[imod].survey1
							&& project_output.files[crossing->file_id_2].block == mods[imod].survey2)
						|| (project_output.files[crossing->file_id_2].block == mods[imod].survey1
							&& project_output.files[crossing->file_id_1].block == mods[imod].survey2))
						{
						for (itie=0;itie<crossing->num_ties;itie++)
							{
							tie = &crossing->ties[itie];
							tie->status = MBNA_TIE_XYZ;

fprintf(stderr,"Set tie mode XYZ:      %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
icrossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
							}
						}
					}
				break;
			
			case MOD_MODE_SET_TIES_XY_BLOCK:
fprintf(stderr,"\nCommand set-ties-xy-by-block=%2.2d/%2.2d\n", mods[imod].survey1, mods[imod].survey2);

				for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if ((project_output.files[crossing->file_id_1].block == mods[imod].survey1
							&& project_output.files[crossing->file_id_2].block == mods[imod].survey2)
						|| (project_output.files[crossing->file_id_2].block == mods[imod].survey1
							&& project_output.files[crossing->file_id_1].block == mods[imod].survey2))
						{
						for (itie=0;itie<crossing->num_ties;itie++)
							{
							tie = &crossing->ties[itie];
							tie->status = MBNA_TIE_XY;

fprintf(stderr,"Set tie mode XY-only:  %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
icrossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
							}
						}
					}
				break;
			
			case MOD_MODE_SET_TIES_Z_BLOCK:
fprintf(stderr,"\nCommand set-ties-z-by-block=%2.2d/%2.2d\n", mods[imod].survey1, mods[imod].survey2);

				for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if ((project_output.files[crossing->file_id_1].block == mods[imod].survey1
							&& project_output.files[crossing->file_id_2].block == mods[imod].survey2)
						|| (project_output.files[crossing->file_id_2].block == mods[imod].survey1
							&& project_output.files[crossing->file_id_1].block == mods[imod].survey2))
						{
						for (itie=0;itie<crossing->num_ties;itie++)
							{
							tie = &crossing->ties[itie];
							tie->status = MBNA_TIE_Z;

fprintf(stderr,"Set tie mode Z-only:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
icrossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
							}
						}
					}
				break;

			case MOD_MODE_SET_TIES_ZOFFSET_BLOCK:
fprintf(stderr,"\nCommand set-ties-zoffset-by-block=%2.2d/%2.2d/%f\n", mods[imod].survey1, mods[imod].survey2, mods[imod].zoffset);
				
				for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
					{
					crossing = &(project_output.crossings[icrossing]);
					file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
					file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
					if ((project_output.files[crossing->file_id_1].block == mods[imod].survey1
							&& project_output.files[crossing->file_id_2].block == mods[imod].survey2)
						|| (project_output.files[crossing->file_id_2].block == mods[imod].survey1
							&& project_output.files[crossing->file_id_1].block == mods[imod].survey2))
						{
						for (itie=0;itie<crossing->num_ties;itie++)
							{
							tie = &crossing->ties[itie];
							tie->offset_z_m = mods[imod].zoffset;

fprintf(stderr,"Set tie zoffset:   %d:%d  %2.2d:%4.4d:%4.4d:%2.2d   %2.2d:%4.4d:%4.4d:%2.2d  %.3f %.3f %.3f\n",
icrossing, itie,
file1->block, crossing->file_id_1, crossing->section_1, tie->snav_1,
file2->block, crossing->file_id_2, crossing->section_2, tie->snav_2,
tie->offset_x_m,tie->offset_y_m,tie->offset_z_m);
							}
						}
					}
				break;

			case MOD_MODE_SKIP_UNSET_CROSSINGS:
fprintf(stderr,"\nCommand skip-unset-crossings\n");

				
			for (icrossing=0;icrossing<project_output.num_crossings;icrossing++)
				{
				crossing = (struct mbna_crossing *) &project_output.crossings[icrossing];
				file1 = (struct mbna_file *) &project_output.files[crossing->file_id_1];
				file2 = (struct mbna_file *) &project_output.files[crossing->file_id_2];
				if (crossing->num_ties == 0)
					{
					crossing->status = MBNA_CROSSING_STATUS_SKIP;
fprintf(stderr,"Set crossing status to skip:   %d  %2.2d:%4.4d:%4.4d   %2.2d:%4.4d:%4.4d\n",
icrossing,
file1->block, crossing->file_id_1, crossing->section_1,
file2->block, crossing->file_id_2, crossing->section_2);
					}
				}
				
			break;

			}
		}

	/* write out the new project file */
	status = mbnavadjust_write_project(verbose, &project_output, &error);
	if (status == MB_SUCCESS)
		{
		fprintf(stderr,"Output project written:\n\t%s\n", project_output_path);
		fprintf(stderr,"\t%d files\n\t%d crossings\n\t%d ties\n",
			project_output.num_files, project_output.num_crossings, project_output.num_ties);
		}
	else
		{
		fprintf(stderr,"Write failure for output project:\n\t%s\n",
			project_output_path);
		fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
