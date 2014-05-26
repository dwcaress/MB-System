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

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "mbnavadjustmerge";
	char help_message[] =  "mbnavadjustmerge merges two existing mbnavadjust projects.\n";
	char usage_message[] = "mbnavadjustmerge --verbose --help --input=project_path --input=project_path --output=project_path";
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
	
	int	shellstatus;
	mb_path	command;
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
			
			/* input-base */
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
	if (verbose >= 0)
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
	else if (project_inputadd_set == MB_NO)
		{
		fprintf(stderr,"No input add project has been set.\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
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
	if (mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_MERGE)
		{
		/* read the input base project */
		status = mbnavadjust_read_project(verbose, project_inputbase_path,
					&project_inputbase, &error);
		if (status == MB_FAILURE)
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
		fprintf(stderr, "Executing in shell: %s\n", command);
		shellstatus = system(command);
		sprintf(command,"cat %s/log.txt %s/logorg.txt > %s/log.txt",
			project_inputbase.datadir, project_output.datadir, project_output.datadir);
		fprintf(stderr, "Executing in shell: %s\n", command);
		shellstatus = system(command);
		
		/* now copy the actual data files from the input project to the new output project */
		for (i=0;i<project_output.num_files;i++)
			{
			/* copy the file navigation */
			sprintf(command,"cp %s/nvs_%4.4d.mb166 %s", project_inputbase.datadir, i, project_output.datadir);
			fprintf(stderr, "Executing in shell: %s\n", command);
			shellstatus = system(command);
				
			/* copy all the section files */
			for (j=0;j<project_output.files[i].num_sections;j++)
				{
				/* copy the section file */
				sprintf(command,"cp %s/nvs_%4.4d_%4.4d.mb71 %s", project_inputbase.datadir, i, j, project_output.datadir);
				fprintf(stderr, "Executing in shell: %s\n", command);
				shellstatus = system(command);
				}
			}		
		}
		
	/* else if adding the second project to the first, just open the first as the output project */
	else if (mbnavadjustmerge_mode == MBNAVADJUSTMERGE_MODE_ADD)
		{
		/* read the input base project in as the output project */
		status = mbnavadjust_read_project(verbose, project_output_path,
					&project_output, &error);
		if (status == MB_FAILURE)
			{
			fprintf(stderr,"Load failure for input base project (which is also the intended output):\n\t%s\n",
				project_output_path);
			fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
			error = MB_ERROR_BAD_USAGE;
			exit(error);
			}
		}

	/* read the input add project */
	if (status == MB_SUCCESS)
		status = mbnavadjust_read_project(verbose, project_inputadd_path,
					&project_inputadd, &error);
	if (status == MB_FAILURE)
		{
		fprintf(stderr,"Load failure for input add project:\n\t%s\n",
			project_inputadd_path);
		fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* Add the input add project to the output project */
	if (project_inputadd.num_files > 0)
		{
		/* allocate space for the files in project_inputadd */
		status = mb_reallocd(verbose, __FILE__, __LINE__,
					sizeof(struct mbna_file) * (project_output.num_files + project_inputadd.num_files),
						(void **)&project_output.files,&error);
		if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
	
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
		fprintf(stderr, "Executing in shell: %s\n", command);
		shellstatus = system(command);
		
		/* now copy the actual data files from the input project to the new output project */
		for (i=0;i<project_inputadd.num_files;i++)
			{
			k = project_output.num_files + i;

			/* copy the file navigation */
			sprintf(command,"cp %s/nvs_%4.4d.mb166 %s/nvs_%4.4d.mb166", project_inputadd.datadir, i, project_output.datadir, k);
			fprintf(stderr, "Executing in shell: %s\n", command);
			shellstatus = system(command);
				
			/* copy all the section files */
			for (j=0;j<project_inputadd.files[i].num_sections;j++)
				{
				/* copy the section file */
				sprintf(command,"cp %s/nvs_%4.4d_%4.4d.mb71 %s/nvs_%4.4d_%4.4d.mb71", project_inputadd.datadir, i, j, project_output.datadir, k, j);
				fprintf(stderr, "Executing in shell: %s\n", command);
				shellstatus = system(command);
				}
			}		
		}
	
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
	
	/* write out the new project file */
	status = mbnavadjust_write_project(verbose, &project_output, &error);

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
