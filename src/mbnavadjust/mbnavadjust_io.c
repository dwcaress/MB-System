/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust_io.c	3/23/00
 *    $Id$
 *
 *    Copyright (c) 2014-2015 by
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
 * mbnavadjust is an interactive navigation adjustment package
 * for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This file contains the code that does not directly depend on the
 * MOTIF interface.
 *
 * Author:	D. W. Caress
 * Date:	April 14, 2014
 *
 * $Log: mbnavadjust_io.c,v $
 *
 */

/*--------------------------------------------------------------------*/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"

/* define mbnavadjust io structures */
#include "mbnavadjust_io.h"

static char version_id[] = "$Id$";
static char program_name[] = "mbnavadjust i/o functions";

/*--------------------------------------------------------------------*/
int mbnavadjust_new_project(int verbose, char *projectpath,
                            double section_length,
                            int	section_soundings,
                            double cont_int,
                            double col_int,
                            double tick_int,
                            double label_int,
                            int	decimation,
                            double smoothing,
                            double zoffsetwidth,
                            struct mbna_project *project, int *error)
{
	/* local variables */
	char	*function_name = "mbnavadjust_new_project";
	int	status = MB_SUCCESS;
	char	*slashptr, *nameptr;
	struct stat statbuf;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:            %d\n",verbose);
		fprintf(stderr,"dbg2       projectpath:        %s\n",projectpath);
		fprintf(stderr,"dbg2       section_length:     %f\n",section_length);
		fprintf(stderr,"dbg2       section_soundings:  %d\n",section_soundings);
		fprintf(stderr,"dbg2       cont_int:           %f\n",cont_int);
		fprintf(stderr,"dbg2       col_int:            %f\n",col_int);
		fprintf(stderr,"dbg2       tick_int:           %f\n",tick_int);
		fprintf(stderr,"dbg2       label_int:          %f\n",label_int);
		fprintf(stderr,"dbg2       decimation:         %d\n",decimation);
		fprintf(stderr,"dbg2       smoothing:          %f\n",smoothing);
		fprintf(stderr,"dbg2       zoffsetwidth:       %f\n",zoffsetwidth);
		fprintf(stderr,"dbg2       project:            %p\n",project);
		}
                
        /* if project structure holds an open project close it first */
        if (project->open == MB_YES)
                status = mbnavadjust_close_project(verbose, project, error);

	/* check path to see if new project can be created */
        nameptr = (char *) NULL;
        slashptr = strrchr(projectpath,'/');
        if (slashptr != (char *) NULL)
                nameptr = slashptr + 1;
        else
                nameptr = projectpath;
        if (strlen(nameptr) > 4
                && strcmp(&nameptr[strlen(nameptr)-4],".nvh") == 0)
                nameptr[strlen(nameptr)-4] = '\0';
        if (strlen(nameptr) == 0)
                {
                fprintf(stderr,"Unable to create new project!\nInvalid project path: %s\n", projectpath);
                *error = MB_ERROR_INIT_FAIL;
                status = MB_FAILURE;
                }
        
        /* try to create new project */
        if (status == MB_SUCCESS)
                {
                strcpy(project->name,nameptr);
		if (strlen(projectpath) == strlen(nameptr))
			{
			getcwd(project->path, MB_PATH_MAXLINE);
			strcat(project->path, "/");
			}
                else
			{
			strncpy(project->path, projectpath, strlen(projectpath)-strlen(nameptr));
			}
                strcpy(project->home,project->path);
                strcat(project->home,project->name);
                strcat(project->home,".nvh");
                strcpy(project->datadir,project->path);
                strcat(project->datadir,project->name);
                strcat(project->datadir,".dir");
                strcpy(project->logfile,project->datadir);
                strcat(project->logfile,"/log.txt");

                /* no new project if file or directory already exist */
                if (stat(project->home,&statbuf) == 0)
                        {
                        fprintf(stderr,"Unable to create new project!\nHome file %s already exists\n", project->home);
                        *error = MB_ERROR_INIT_FAIL;
                        status = MB_FAILURE;
                        }
                if (stat(project->datadir,&statbuf) == 0)
                        {
                        fprintf(stderr,"Unable to create new project!\nData directory %s already exists\n", project->datadir);
                        *error = MB_ERROR_INIT_FAIL;
                        status = MB_FAILURE;
                        }

                /* initialize new project */
                if (status == MB_SUCCESS)
                        {
                        /* set values */
                        project->open = MB_YES;
                        project->num_files = 0;
                        project->num_files_alloc = 0;
                        project->files = NULL;
                        project->num_snavs = 0;
                        project->num_pings = 0;
                        project->num_beams = 0;
                        project->num_crossings = 0;
                        project->num_crossings_alloc = 0;
                        project->num_crossings_analyzed = 0;
                        project->num_goodcrossings = 0;
                        project->num_truecrossings = 0;
                        project->num_truecrossings_analyzed = 0;
                        project->crossings = NULL;
                        project->num_ties = 0;
                        project->section_length = section_length;
                        project->section_soundings = section_soundings;
                        project->cont_int = cont_int;
                        project->col_int = col_int;
                        project->tick_int = tick_int;
                        project->label_int = label_int;
                        project->decimation = decimation;
                        project->precision = SIGMA_MINIMUM;
                        project->smoothing = smoothing;
                        project->zoffsetwidth = zoffsetwidth;
                        project->inversion = MBNA_INVERSION_NONE;
                        project->modelplot = MB_NO;
                        project->modelplot_style = MBNA_MODELPLOT_SEQUENTIAL;
                        project->logfp = NULL;
                        
                        project->inversion = MBNA_INVERSION_NONE;
                        project->precision = SIGMA_MINIMUM;
                        project->smoothing = MBNA_SMOOTHING_DEFAULT;
                        project->zoffsetwidth = 5.0;

                        /* create data directory */
                        if (mkdir(project->datadir,00775) != 0)
                                {
                                fprintf(stderr,"Error creating data directory %s\n", project->datadir);
                                *error = MB_ERROR_INIT_FAIL;
                                status = MB_FAILURE;
                                }

                        /* write home file and other files */
                        else if ((status = mbnavadjust_write_project(verbose, project, error)) == MB_FAILURE)
                                {
                                fprintf(stderr,"Failure to write project file %s\n", project->home);
                                *error = MB_ERROR_INIT_FAIL;
                                status = MB_FAILURE;
                                }

                        /* initialize log file */
                        else if ((project->logfp = fopen(project->logfile, "w")) == NULL)
                                {
                                fprintf(stderr,"Failure to create log file %s\n", project->logfile);
                                *error = MB_ERROR_INIT_FAIL;
                                status = MB_FAILURE;
                                }
                        
                        /* first message in log file */
                        else
                                {
                                fprintf(project->logfp,"New project initialized: %s\n > Project home: %s\n",
                                                        project->name, project->home);
                               }
                        }
                }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_read_project(int verbose, char *projectpath,
                                struct mbna_project *project, int *error)
{
	/* local variables */
	char	*function_name = "mbnavadjust_read_project";
	int	status = MB_SUCCESS;
	char	*slashptr, *nameptr;
	struct stat statbuf;
	FILE	*hfp;
	struct mbna_file *file;
	struct mbna_section *section, *section1, *section2;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	char	label[STRING_MAX];
	char	buffer[BUFFER_MAX];
	char	obuffer[BUFFER_MAX];
	char	command[MB_PATH_MAXLINE];
	char	*result;
	int	versionmajor, versionminor;
	double	dummy;
	int	nscan, idummy, jdummy;
	int	s1id, s2id;
	int	shellstatus;
	int	i, j, k, l;
	double	mtodeglon;
	double	mtodeglat;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:            %d\n",verbose);
		fprintf(stderr,"dbg2       projectname:        %s\n",projectpath);
		fprintf(stderr,"dbg2       project:            %p\n",project);
		}
                
        /* if project structure holds an open project close it first */
        if (project->open == MB_YES)
                status = mbnavadjust_close_project(verbose, project, error);

	/* check path to see if project exists */
        nameptr = (char *) NULL;
        slashptr = strrchr(projectpath,'/');
        if (slashptr != (char *) NULL)
                nameptr = slashptr + 1;
        else
                nameptr = projectpath;
        if (strlen(nameptr) > 4
                && strcmp(&nameptr[strlen(nameptr)-4],".nvh") == 0)
                nameptr[strlen(nameptr)-4] = '\0';
        if (strlen(nameptr) == 0)
                {
                fprintf(stderr,"Unable to read project!\nInvalid project path: %s\n", projectpath);
                *error = MB_ERROR_INIT_FAIL;
                status = MB_FAILURE;
                }
        
        /* try to read project */
        if (status == MB_SUCCESS)
                {
                strcpy(project->name,nameptr);
		if (strlen(projectpath) == strlen(nameptr))
			{
			getcwd(project->path, MB_PATH_MAXLINE);
			strcat(project->path, "/");
			}
                else
			{
			strcpy(project->path, projectpath);
			project->path[strlen(projectpath)-strlen(nameptr)] = '\0';
			}
                strcpy(project->home,project->path);
                strcat(project->home,project->name);
                strcat(project->home,".nvh");
                strcpy(project->datadir,project->path);
                strcat(project->datadir,project->name);
                strcat(project->datadir,".dir");
                strcpy(project->logfile,project->datadir);
                strcat(project->logfile,"/log.txt");

                /* check if project exists */
                if (stat(project->home,&statbuf) != 0)
                        {
                        fprintf(stderr,"Project home file %s does not exist\n", project->home);
                        *error = MB_ERROR_INIT_FAIL;
                        status = MB_FAILURE;
                        }
                if (stat(project->datadir,&statbuf) != 0)
                        {
                        fprintf(stderr,"Data directory %s does not exist\n", project->datadir);
                        *error = MB_ERROR_INIT_FAIL;
                        status = MB_FAILURE;
                        }

                /* read the project */
                if (status == MB_SUCCESS)
                        {
                        /* first save copy of the project file */
                        sprintf(command,"cp %s %s.save", project->home, project->home);
                        shellstatus = system(command);
                
                        /* open and read home file */
                        status = MB_SUCCESS;
                        if ((hfp = fopen(project->home,"r")) != NULL)
                                {
                                /* check for proper header */
                                if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                        || strncmp(buffer,"##MBNAVADJUST PROJECT",21) != 0)
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                /* read basic names and stats */
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
                                                || strcmp(label,"MB-SYSTEM_VERSION") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
                                                || strcmp(label,"PROGRAM_VERSION") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %d.%d",label,&versionmajor,&versionminor)) != 3
                                                || strcmp(label,"FILE_VERSION") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (versionmajor > 3 || (versionmajor == 3 && versionminor > 2))
                                        {
                                        if (status == MB_SUCCESS
                                                && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                        || (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
                                                        || strcmp(label,"ORIGIN") != 0))
                                                status = MB_FAILURE;
                                        }
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
                                                || strcmp(label,"NAME") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
                                                || strcmp(label,"PATH") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
                                                || strcmp(label,"HOME") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
                                                || strcmp(label,"DATADIR") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %d",label,&project->num_files)) != 2
                                                || strcmp(label,"NUMFILES") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %d",label,&project->num_crossings)) != 2
                                                || strcmp(label,"NUMCROSSINGS") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %lf",label,&project->section_length)) != 2
                                                || strcmp(label,"SECTIONLENGTH") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((100*versionmajor + versionminor) > 100)
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %d",label,&project->section_soundings)) != 2
                                                || strcmp(label,"SECTIONSOUNDINGS") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %d",label,&project->decimation)) != 2
                                                || strcmp(label,"DECIMATION") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %lf",label,&project->cont_int)) != 2
                                                || strcmp(label,"CONTOURINTERVAL") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %lf",label,&project->col_int)) != 2
                                                || strcmp(label,"COLORINTERVAL") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %lf",label,&project->tick_int)) != 2
                                                || strcmp(label,"TICKINTERVAL") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS
                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                || (nscan = sscanf(buffer,"%s %d",label,&project->inversion)) != 2
                                                || strcmp(label,"INVERSION") != 0))
                                        status = MB_FAILURE;
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
                
                                if (status == MB_SUCCESS)
                                        {
                                        if (versionmajor > 3 || (versionmajor == 3 && versionminor > 0))
                                                {
                                                if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                        || (nscan = sscanf(buffer,"%s %lf",label,&project->smoothing)) != 2
                                                        || strcmp(label,"SMOOTHING") != 0)
                                                        status = MB_FAILURE;
                                                project->precision = SIGMA_MINIMUM;
                                                }
                                        else if (versionmajor > 1 || (versionmajor == 1 && versionminor > 2))
                                                {
                                                if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                        || (nscan = sscanf(buffer,"%s %lf",label,&project->precision)) != 2
                                                        || strcmp(label,"PRECISION") != 0)
                                                        status = MB_FAILURE;
                                                project->smoothing = MBNA_SMOOTHING_DEFAULT;
                                                }
                                        else
                                                {
                                                project->precision = SIGMA_MINIMUM;
                                                project->smoothing = MBNA_SMOOTHING_DEFAULT;
                                                }
                                        }
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
                
                                if (status == MB_SUCCESS)
                                        {
                                        if ((versionmajor > 1 || (versionmajor == 1 && versionminor > 4))
                                                && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                        || (nscan = sscanf(buffer,"%s %lf",label,&project->zoffsetwidth)) != 2
                                                        || strcmp(label,"ZOFFSETWIDTH") != 0))
                                                status = MB_FAILURE;
                                        else
                                                project->zoffsetwidth = 5.0;
                                        }
                
                                /* allocate memory for files array */
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
                
                                if (project->num_files > 0)
                                        {
                                        project->files = (struct mbna_file *)
                                                malloc(sizeof(struct mbna_file) * (project->num_files));
                                        if (project->files != NULL)
                                                {
                                                project->num_files_alloc = project->num_files;
                                                memset(project->files,0,project->num_files_alloc * sizeof(struct mbna_file));
                                                }
                                        else
                                                {
                                                project->num_files_alloc = 0;
                                                status = MB_FAILURE;
                                                *error = MB_ERROR_MEMORY_FAIL;
                                                }
                                        }
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
                
                                if (project->num_crossings > 0)
                                        {
                                        project->crossings = (struct mbna_crossing *)
                                                malloc(sizeof(struct mbna_crossing) * (project->num_crossings));
                                        if (project->crossings != NULL)
                                                {
                                                project->num_crossings_alloc = project->num_crossings;
                                                memset(project->crossings,0,sizeof(struct mbna_crossing) * project->num_crossings_alloc);
                                                }
                                        else
                                                {
                                                project->num_crossings_alloc = 0;
                                                status = MB_FAILURE;
                                                *error = MB_ERROR_MEMORY_FAIL;
                                                }
                                        }
                if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
                
                                for (i=0;i<project->num_files;i++)
                                        {
                                        file = &project->files[i];
                                        file->num_sections_alloc = 0;
                                        file->sections = NULL;
                                        file->num_snavs = 0;
                                        file->num_pings = 0;
                                        file->num_beams = 0;
                                        if (status == MB_SUCCESS
                                                && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                        || (nscan = sscanf(buffer,"FILE %d %d %d %d %lf %lf %lf %lf %d %d %s",
                                                        &idummy,
                                                        &(file->status),
                                                        &(file->id),
                                                        &(file->format),
                                                        &(file->heading_bias_import),
                                                        &(file->roll_bias_import),
                                                        &(file->heading_bias),
                                                        &(file->roll_bias),
                                                        &(file->num_sections),
                                                        &(file->output_id),
                                                        file->file)) != 11))
                                                status = MB_FAILURE;
                
                                        /* set file->path as absolute path
                                            - file->file may be a relative path */
                                        if (status == MB_SUCCESS)
                                                {
                                                if (file->file[0] == '/')
                                                    strcpy(file->path, file->file);
                                                else
                                                    {
                                                    strcpy(file->path, project->path);
                                                    strcat(file->path, file->file);
                                                    }
                                                }
                
                                        /* read section info */
                                        if (file->num_sections > 0)
                                                {
                                                file->sections = (struct mbna_section *)
                                                        malloc(sizeof(struct mbna_section) * (file->num_sections));
                                                if (file->sections != NULL)
                                                        {
                                                        file->num_sections_alloc = file->num_sections;
                                                        memset(file->sections,0,sizeof(struct mbna_section) * file->num_sections_alloc);
                                                        }
                                                else
                                                        {
                                                        file->num_sections_alloc = 0;
                                                        status = MB_FAILURE;
                                                        *error = MB_ERROR_MEMORY_FAIL;
                                                        }
                                                }
                                        for (j=0;j<file->num_sections;j++)
                                                {
                                                section = &file->sections[j];
                                                if (status == MB_SUCCESS)
                                                        result = fgets(buffer,BUFFER_MAX,hfp);
                                                if (status == MB_SUCCESS && result == buffer)
                                                        nscan = sscanf(buffer,"SECTION %d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %d",
                                                                &idummy,
                                                                &section->num_pings,
                                                                &section->num_beams,
                                                                &section->num_snav,
                                                                &section->continuity,
                                                                &section->distance,
                                                                &section->btime_d,
                                                                &section->etime_d,
                                                                &section->lonmin,
                                                                &section->lonmax,
                                                                &section->latmin,
                                                                &section->latmax,
                                                                &section->depthmin,
                                                                &section->depthmax,
                                                                &section->contoursuptodate);
                                                if (result != buffer || nscan < 14)
                                                        {
                                                        status = MB_FAILURE;
fprintf(stderr, "read failed on section: %s\n", buffer);
                                                        }
                                                if (nscan < 15)
                                                        section->contoursuptodate = MB_NO;
                                                for (k=MBNA_MASK_DIM-1;k>=0;k--)
                                                    {
                                                    if (status == MB_SUCCESS)
                                                        result = fgets(buffer,BUFFER_MAX,hfp);
                                                    for (l=0;l<MBNA_MASK_DIM;l++)
                                                        {
                                                        sscanf(&buffer[l], "%1d", &section->coverage[l+k*MBNA_MASK_DIM]);
                                                        }
                                                    }
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
/*fprintf(stderr,"%s/nvs_%4.4d_%4.4d.mb71\n",
project->datadir,file->id,j);
for (k=MBNA_MASK_DIM-1;k>=0;k--)
{
for (l=0;l<MBNA_MASK_DIM;l++)
{
fprintf(stderr, "%1d", section->coverage[l + k * MBNA_MASK_DIM]);
}
fprintf(stderr, "\n");
}*/
                                                for (k=0;k<section->num_snav;k++)
                                                    {
                                                    if (status == MB_SUCCESS)
                                                        result = fgets(buffer,BUFFER_MAX,hfp);
                                                    if (status == MB_SUCCESS && result == buffer)
                                                        nscan = sscanf(buffer,"SNAV %d %d %lf %lf %lf %lf %lf %lf %lf",
                                                                &idummy,
                                                                &section->snav_id[k],
                                                                &section->snav_distance[k],
                                                                &section->snav_time_d[k],
                                                                &section->snav_lon[k],
                                                                &section->snav_lat[k],
                                                                &section->snav_lon_offset[k],
                                                                &section->snav_lat_offset[k],
                                                                &section->snav_z_offset[k]);
                                                    section->snav_num_ties[k] = 0;
                                                    section->snav_lon_offset_int[k] = 0.0;
                                                    section->snav_lat_offset_int[k] = 0.0;
                                                    section->snav_z_offset_int[k] = 0.0;
                                                    if (result == buffer && nscan == 6)
                                                        {
                                                        section->snav_lon_offset[k] = 0.0;
                                                        section->snav_lat_offset[k] = 0.0;
                                                        section->snav_z_offset[k] = 0.0;
                                                        }
                                                    else if (result == buffer && nscan == 8)
                                                        {
                                                        section->snav_z_offset[k] = 0.0;
                                                        }
                                                    else if (result != buffer || nscan != 9)
                                                        {
                                                        status = MB_FAILURE;
fprintf(stderr, "read failed on snav: %s\n", buffer);
                                                        }
                
                                                    /* reverse offset values if older values */
                                                    if (versionmajor < 3)
                                                        {
                                                        section->snav_lon_offset[k] *= -1.0;
                                                        section->snav_lat_offset[k] *= -1.0;
                                                        section->snav_z_offset[k] *= -1.0;
                                                        }
                                                    }
                                                section->global_start_ping = project->num_pings;
                                                section->global_start_snav = project->num_snavs - section->continuity;
                                                file->num_snavs += section->num_pings;
                                                file->num_pings += section->num_pings;
                                                file->num_beams += section->num_beams;
                                                project->num_snavs += (section->num_snav - section->continuity);
                                                project->num_pings += section->num_pings;
                                                project->num_beams += section->num_beams;
                                                }
                                        }
                
                                /* count the number of blocks */
                                project->num_blocks = 0;
                                for (i=0;i<project->num_files;i++)
                                    {
                                    file = &project->files[i];
                                    if (i==0 || file->sections[0].continuity == MB_NO)
                                        {
                                        project->num_blocks++;
                                        }
                                    file->block = project->num_blocks - 1;
                                    file->block_offset_x = 0.0;
                                    file->block_offset_y = 0.0;
                                    file->block_offset_z = 0.0;
                                    }
                
                                /* read crossings */
                                project->num_crossings_analyzed = 0;
                                project->num_goodcrossings = 0;
                                project->num_truecrossings = 0;
                                project->num_truecrossings_analyzed = 0;
                                project->num_ties = 0;
                                for (i=0;i<project->num_crossings;i++)
                                        {
                                        /* read each crossing */
                                        crossing = &project->crossings[i];
                                        if (status == MB_SUCCESS
                                                && ((100*versionmajor + versionminor) >= 106))
                                                {
                                                if (status == MB_SUCCESS
                                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                                || (nscan = sscanf(buffer,"CROSSING %d %d %d %d %d %d %d %d %d",
                                                                &idummy,
                                                                &crossing->status,
                                                                &crossing->truecrossing,
                                                                &crossing->overlap,
                                                                &crossing->file_id_1,
                                                                &crossing->section_1,
                                                                &crossing->file_id_2,
                                                                &crossing->section_2,
                                                                &crossing->num_ties)) != 9))
                                                                {
                                                                status = MB_FAILURE;
fprintf(stderr, "read failed on crossing: %s\n", buffer);
                                                                }
                                                }
                                        else if (status == MB_SUCCESS
                                                && ((100*versionmajor + versionminor) >= 102))
                                                {
                                                crossing->overlap = 0;
                                                if (status == MB_SUCCESS
                                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                                || (nscan = sscanf(buffer,"CROSSING %d %d %d %d %d %d %d %d",
                                                                &idummy,
                                                                &crossing->status,
                                                                &crossing->truecrossing,
                                                                &crossing->file_id_1,
                                                                &crossing->section_1,
                                                                &crossing->file_id_2,
                                                                &crossing->section_2,
                                                                &crossing->num_ties)) != 8))
                                                                {
                                                                status = MB_FAILURE;
fprintf(stderr, "read failed on crossing: %s\n", buffer);
                                                                }
                                                }
                                        else if (status == MB_SUCCESS)
                                                {
                                                crossing->truecrossing = MB_NO;
                                                crossing->overlap = 0;
                                                if (status == MB_SUCCESS
                                                        && ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                                || (nscan = sscanf(buffer,"CROSSING %d %d %d %d %d %d %d",
                                                                &idummy,
                                                                &crossing->status,
                                                                &crossing->file_id_1,
                                                                &crossing->section_1,
                                                                &crossing->file_id_2,
                                                                &crossing->section_2,
                                                                &crossing->num_ties)) != 7))
                                                                {
                                                                status = MB_FAILURE;
fprintf(stderr, "read failed on old format crossing: %s\n", buffer);
                                                                }
                                                }
                                        if (status == MB_SUCCESS
                                            && crossing->status != MBNA_CROSSING_STATUS_NONE)
                                                project->num_crossings_analyzed++;
                                        if (status == MB_SUCCESS
                                            && crossing->truecrossing == MB_YES)
                                                {
                                                project->num_truecrossings++;
                                                if (crossing->status != MBNA_CROSSING_STATUS_NONE)
                                                project->num_truecrossings_analyzed++;
                                                }
                
                                        /* reorder crossing to be early file first older file second if
                                                file version prior to 3.00 */
                                        if (versionmajor < 3)
                                                {
                                                idummy = crossing->file_id_1;
                                                jdummy = crossing->section_1;
                                                crossing->file_id_1 = crossing->file_id_2;
                                                crossing->section_1 = crossing->section_2;
                                                crossing->file_id_2 = idummy;
                                                crossing->section_2 = jdummy;
                                                }
                
                                        /* read ties */
                                        if (status == MB_SUCCESS)
                                        for (j=0;j<crossing->num_ties;j++)
                                                {
                                                /* read each tie */
                                                tie = &crossing->ties[j];
                                                if (status == MB_SUCCESS && ((100*versionmajor + versionminor) > 301))
                                                        {
                                                        if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                                || (nscan = sscanf(buffer,"TIE %d %d %d %lf %d %lf %lf %lf %lf %d %lf %lf %lf",
                                                                        &idummy,
                                                                        &tie->status,
                                                                        &tie->snav_1,
                                                                        &tie->snav_1_time_d,
                                                                        &tie->snav_2,
                                                                        &tie->snav_2_time_d,
                                                                        &tie->offset_x,
                                                                        &tie->offset_y,
                                                                        &tie->offset_z_m,
                                                                        &tie->inversion_status,
                                                                        &tie->inversion_offset_x,
                                                                        &tie->inversion_offset_y,
                                                                        &tie->inversion_offset_z_m)) != 13)
                                                                {
                                                                status = MB_FAILURE;
fprintf(stderr, "read failed on tie: %s\n", buffer);
                                                                }
                                                        }
                                                else if (status == MB_SUCCESS && ((100*versionmajor + versionminor) > 103))
                                                        {
                                                        if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                                || (nscan = sscanf(buffer,"TIE %d %d %lf %d %lf %lf %lf %lf %d %lf %lf %lf",
                                                                        &idummy,
                                                                        &tie->snav_1,
                                                                        &tie->snav_1_time_d,
                                                                        &tie->snav_2,
                                                                        &tie->snav_2_time_d,
                                                                        &tie->offset_x,
                                                                        &tie->offset_y,
                                                                        &tie->offset_z_m,
                                                                        &tie->inversion_status,
                                                                        &tie->inversion_offset_x,
                                                                        &tie->inversion_offset_y,
                                                                        &tie->inversion_offset_z_m)) != 12)
                                                                {
                                                                status = MB_FAILURE;
fprintf(stderr, "read failed on tie: %s\n", buffer);
                                                                }
                                                        tie->status = MBNA_TIE_XYZ;
                                                        }
                                                else if (status == MB_SUCCESS)
                                                        {
                                                        if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                                || (nscan = sscanf(buffer,"TIE %d %d %lf %d %lf %lf %lf %d %lf %lf",
                                                                        &idummy,
                                                                        &tie->snav_1,
                                                                        &tie->snav_1_time_d,
                                                                        &tie->snav_2,
                                                                        &tie->snav_2_time_d,
                                                                        &tie->offset_x,
                                                                        &tie->offset_y,
                                                                        &tie->inversion_status,
                                                                        &tie->inversion_offset_x,
                                                                        &tie->inversion_offset_y)) != 10)
                                                                {
                                                                status = MB_FAILURE;
fprintf(stderr, "read failed on tie: %s\n", buffer);
                                                                }
                                                        tie->status = MBNA_TIE_XYZ;
                                                        tie->offset_z_m = 0.0;
                                                        tie->inversion_offset_z_m = 0.0;
                                                        }
                
                                                /* reorder crossing to be early file first older file second if
                                                        file version prior to 3.00 */
                                                if (versionmajor < 3)
                                                        {
                                                        idummy = tie->snav_1;
                                                        dummy = tie->snav_1_time_d;
                                                        tie->snav_1 = tie->snav_2;
                                                        tie->snav_1_time_d = tie->snav_2_time_d;
                                                        tie->snav_2 = idummy;
                                                        tie->snav_2_time_d = dummy;
/*					                tie->offset_x *= -1.0;
                                                        tie->offset_y *= -1.0;
                                                        tie->offset_z_m *= -1.0;
                                                        tie->inversion_offset_x *= -1.0;
                                                        tie->inversion_offset_y *= -1.0;
                                                        tie->inversion_offset_z_m *= -1.0;*/
                                                        }
                
                                                /* for version 2.0 or later read covariance */
                                                if (status == MB_SUCCESS && ((100*versionmajor + versionminor) >= 200))
                                                        {
                                                        if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
                                                                || (nscan = sscanf(buffer,"COV %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                                                                        &tie->sigmar1,
                                                                        &(tie->sigmax1[0]),
                                                                        &(tie->sigmax1[1]),
                                                                        &(tie->sigmax1[2]),
                                                                        &tie->sigmar2,
                                                                        &(tie->sigmax2[0]),
                                                                        &(tie->sigmax2[1]),
                                                                        &(tie->sigmax2[2]),
                                                                        &tie->sigmar3,
                                                                        &(tie->sigmax3[0]),
                                                                        &(tie->sigmax3[1]),
                                                                        &(tie->sigmax3[2]))) != 12)
                                                                {
                                                                status = MB_FAILURE;
fprintf(stderr, "read failed on tie covariance: %s\n", buffer);
                                                                }
                                                        if (tie->sigmar1 <= 0.0)
                                                                {
                                                                tie->sigmax1[0] = 1.0;
                                                                tie->sigmax1[1] = 0.0;
                                                                tie->sigmax1[2] = 0.0;
                                                                }
                                                        if (tie->sigmar2 <= 0.0)
                                                                {
                                                                tie->sigmax2[0] = 0.0;
                                                                tie->sigmax2[1] = 1.0;
                                                                tie->sigmax2[2] = 0.0;
                                                                }
                                                        if (tie->sigmar3 <= 0.0)
                                                                {
                                                                tie->sigmax3[0] = 0.0;
                                                                tie->sigmax3[1] = 0.0;
                                                                tie->sigmax3[2] = 1.0;
                                                                }
                                                        }
                                                else if (status == MB_SUCCESS)
                                                        {
                                                        tie->sigmar1 = 100.0;
                                                        tie->sigmax1[0] = 1.0;
                                                        tie->sigmax1[1] = 0.0;
                                                        tie->sigmax1[2] = 0.0;
                                                        tie->sigmar2 = 100.0;
                                                        tie->sigmax2[0] = 0.0;
                                                        tie->sigmax2[1] = 1.0;
                                                        tie->sigmax2[2] = 0.0;
                                                        tie->sigmar3 = 100.0;
                                                        tie->sigmax3[0] = 0.0;
                                                        tie->sigmax3[1] = 0.0;
                                                        tie->sigmax3[2] = 1.0;
                                                        }
                
                                                /* update number of ties */
                                                if (status == MB_SUCCESS)
                                                    {
                                                    project->num_ties++;
                                                    }
                
                                                /* check for reasonable snav id's */
                                                if (status == MB_SUCCESS)
                                                    {
                                                    file = &project->files[crossing->file_id_1];
                                                    section = &file->sections[crossing->section_1];
                                                    if (tie->snav_1 >= section->num_snav)
                                                        {
                                                        tie->snav_1 = ((double)tie->snav_1
                                                                                / (double)section->num_pings)
                                                                                * (MBNA_SNAV_NUM - 1);
                                                        tie->snav_1_time_d = section->snav_time_d[tie->snav_1];
fprintf(stderr,"Reset tie snav_1 on read:%d\n",tie->snav_1);
                                                        }
                                                    file = &project->files[crossing->file_id_2];
                                                    section = &file->sections[crossing->section_2];
                                                    if (tie->snav_2 >= section->num_snav)
                                                        {
                                                        tie->snav_2 = ((double)tie->snav_2
                                                                                / (double)section->num_pings)
                                                                                * (MBNA_SNAV_NUM - 1);
                                                        tie->snav_2_time_d = section->snav_time_d[tie->snav_2];
fprintf(stderr,"Reset tie snav_2 on read:%d\n",tie->snav_2);
                                                        }
                                                    }
                
                                                /* update number of ties for snavs */
                                                if (status == MB_SUCCESS)
                                                    {
                                                    file = &project->files[crossing->file_id_1];
                                                    section = &file->sections[crossing->section_1];
                                                    section->snav_num_ties[tie->snav_1]++;
                                                    file = &project->files[crossing->file_id_2];
                                                    section = &file->sections[crossing->section_2];
                                                    section->snav_num_ties[tie->snav_2]++;
                                                    }
                
                                                /* calculate offsets in local meters */
                                                if (status == MB_SUCCESS)
                                                    {
                                                    section1 = &(project->files[crossing->file_id_1].sections[crossing->section_1]);
                                                    section2 = &(project->files[crossing->file_id_2].sections[crossing->section_2]);
                                                    mb_coor_scale(verbose,0.5 * (MIN(section1->latmin,section2->latmin)
										 + MAX(section1->latmax,section2->latmax)),
                                                                    &mtodeglon,&mtodeglat);
                                                    tie->offset_x_m = tie->offset_x / mtodeglon;
                                                    tie->offset_y_m = tie->offset_y / mtodeglat;
                                                    tie->inversion_offset_x_m = tie->inversion_offset_x / mtodeglon;
                                                    tie->inversion_offset_y_m = tie->inversion_offset_y / mtodeglat;
                                                    }
                                                }
                
                                        /* finally make sure crossing has later section second, switch if needed */
                                        s1id = crossing->file_id_1 * 1000 + crossing->section_1;
                                        s2id = crossing->file_id_2 * 1000 + crossing->section_2;
                                        if (s2id < s1id)
                                                {
                                                idummy = crossing->file_id_1;
                                                jdummy = crossing->section_1;
                                                crossing->file_id_1 = crossing->file_id_2;
                                                crossing->section_1 = crossing->section_2;
                                                crossing->file_id_2 = idummy;
                                                crossing->section_2 = jdummy;
                                                for (j=0;j<crossing->num_ties;j++)
                                                        {
                                                        tie = &crossing->ties[j];
                                                        idummy = tie->snav_1;
                                                        dummy = tie->snav_1_time_d;
                                                        tie->snav_1 = tie->snav_2;
                                                        tie->snav_1_time_d = tie->snav_2_time_d;
                                                        tie->snav_2 = idummy;
                                                        tie->snav_2_time_d = dummy;
                                                        tie->offset_x *= -1.0;
                                                        tie->offset_y *= -1.0;
                                                        tie->offset_x_m *= -1.0;
                                                        tie->offset_y_m *= -1.0;
                                                        tie->offset_z_m *= -1.0;
                                                        tie->inversion_offset_x *= -1.0;
                                                        tie->inversion_offset_y *= -1.0;
                                                        tie->inversion_offset_x_m *= -1.0;
                                                        tie->inversion_offset_y_m *= -1.0;
                                                        tie->inversion_offset_z_m *= -1.0;
                                                        }
                                                }
                                        }
                
                                /* close home file */
                                fclose(hfp);
                
                                /* set project status flag */
                                if (status == MB_SUCCESS)
                                        project->open = MB_YES;
                                else
                                        {
                                        for (i=0;i<project->num_files;i++)
                                                {
                                                file = &project->files[i];
                                                if (file->sections != NULL)
                                                         free( file->sections);
                                                }
                                        if (project->files != NULL)
                                                free(project->files);
                                        if (project->crossings != NULL)
                                                free(project->crossings);
                                        project->open = MB_NO;
                                        memset(project->name,0,STRING_MAX);
                                        strcpy(project->name,"None");
                                        memset(project->path,0,STRING_MAX);
                                        memset(project->datadir,0,STRING_MAX);
                                        project->num_files = 0;
                                        project->num_files_alloc = 0;
                                        project->num_snavs = 0;
                                        project->num_pings = 0;
                                        project->num_beams = 0;
                                        project->num_crossings = 0;
                                        project->num_crossings_alloc = 0;
                                        project->num_crossings_analyzed = 0;
                                        project->num_goodcrossings = 0;
                                        project->num_truecrossings = 0;
                                        project->num_truecrossings_analyzed = 0;
                                        project->num_ties = 0;
                                        }
                                }
                
                        /* else set error */
                        else
                                {
                                status = MB_FAILURE;
                                }
                        }

                /* open log file */
                if ((project->logfp = fopen(project->logfile, "a")) != NULL)
                        {
                        fprintf(project->logfp,"Project opened: %s\n > Project home: %s\n > Number of Files: %d\n > Number of Crossings Found: %d\n > Number of Crossings Analyzed: %d\n > Number of Navigation Ties: %d\n",
                                                project->name, project->home, project->num_files, project->num_crossings,
                                                project->num_crossings_analyzed, project->num_ties);
                        }
                else
                        {
                        fprintf(stderr,"Failure to open log file %s\n", project->logfile);
                        *error = MB_ERROR_INIT_FAIL;
                        status = MB_FAILURE;
                        }
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_close_project(int verbose, struct mbna_project *project,
                                int *error)
{
	/* local variables */
	char	*function_name = "mbnavadjust_close_project";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       verbose:            %d\n",verbose);
		fprintf(stderr,"dbg2       project:            %p\n",project);
		}

        /* add info text */
	fprintf(project->logfp,"Project closed: %s\n", project->name);
	fprintf(project->logfp,"Log file %s/log.txt closed\n", project->datadir);

	/* deallocate memory and reset values */
	for (i=0;i<project->num_files;i++)
		{
		file = &project->files[i];
		if (file->sections != NULL)
			mb_freed(verbose,__FILE__,__LINE__,(void **)&file->sections,error);
		}
	if (project->files != NULL)
		{
		free(project->files);
		project->files = NULL;
		project->num_files_alloc = 0;
		}
	if (project->crossings != NULL)
		{
		free(project->crossings);
		project->crossings = NULL;
		project->num_crossings_alloc = 0;
		}
	if (project->logfp != NULL)
		{
		fclose(project->logfp);
		project->logfp = NULL;
		}

	/* reset values */
	project->open = MB_NO;
	memset(project->name,0,STRING_MAX);
	strcpy(project->name,"None");
 	memset(project->path,0,STRING_MAX);
	memset(project->datadir,0,STRING_MAX);
	memset(project->logfile,0,STRING_MAX);
	project->num_files = 0;
	project->num_snavs = 0;
	project->num_pings = 0;
	project->num_beams = 0;
	project->num_crossings = 0;
 	project->num_crossings_analyzed = 0;
	project->num_goodcrossings = 0;
	project->num_truecrossings = 0;
 	project->num_truecrossings_analyzed = 0;
	project->num_ties = 0;
 	project->inversion = MBNA_INVERSION_NONE;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_write_project(int verbose, struct mbna_project *project,
                                int *error)
{
	/* local variables */
	char	*function_name = "mbnavadjust_write_project";
	int	status = MB_SUCCESS;
	FILE	*hfp, *xfp, *yfp;
	struct mbna_file *file, *file_1, *file_2;
	struct mbna_section *section, *section_1, *section_2;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	char	datalist[STRING_MAX];
	char	routefile[STRING_MAX];
	char	routename[STRING_MAX];
	char	xoffsetfile[STRING_MAX];
	char	yoffsetfile[STRING_MAX];
	double	navlon1, navlon2, navlat1, navlat2;
	int	nroute;
	int	snav_1, snav_2;
	int	ncrossings_true = 0;
	int	ncrossings_gt50 = 0;
	int	ncrossings_gt25 = 0;
	int	ncrossings_lt25 = 0;
	int	ncrossings_fixed = 0;
	int	nties_unfixed = 0;
	int	nties_fixed = 0;
	char	status_char, truecrossing_char;
	int	routecolor = 1;
	char	*unknown = "Unknown";
	double	mtodeglon, mtodeglat;

	/* time, user, host variables */
	time_t	right_now;
	char	date[32], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];

	int	i, j, k, l;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       verbose:            %d\n",verbose);
		fprintf(stderr,"dbg2       project:            %p\n",project);
		}

	/* open and write home file */
	if ((hfp = fopen(project->home,"w")) != NULL)
		{
fprintf(stderr,"Writing project %s\n", project->name);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user,user_ptr);
		else
			strcpy(user, "unknown");
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp,"##MBNAVADJUST PROJECT\n");
		fprintf(hfp,"MB-SYSTEM_VERSION\t%s\n",MB_VERSION);
		fprintf(hfp,"PROGRAM_VERSION\t%s\n",version_id);
		fprintf(hfp,"FILE_VERSION\t3.03\n");
		fprintf(hfp,"ORIGIN\tGenerated by user <%s> on cpu <%s> at <%s>\n", user,host,date);
		fprintf(hfp,"NAME\t%s\n",project->name);
		fprintf(hfp,"PATH\t%s\n",project->path);
		fprintf(hfp,"HOME\t%s\n",project->home);
		fprintf(hfp,"DATADIR\t%s\n",project->datadir);
		fprintf(hfp,"NUMFILES\t%d\n",project->num_files);
		fprintf(hfp,"NUMCROSSINGS\t%d\n",project->num_crossings);
		fprintf(hfp,"SECTIONLENGTH\t%f\n",project->section_length);
		fprintf(hfp,"SECTIONSOUNDINGS\t%d\n",project->section_soundings);
		fprintf(hfp,"DECIMATION\t%d\n",project->decimation);
		fprintf(hfp,"CONTOURINTERVAL\t%f\n",project->cont_int);
		fprintf(hfp,"COLORINTERVAL\t%f\n",project->col_int);
		fprintf(hfp,"TICKINTERVAL\t%f\n",project->tick_int);
		fprintf(hfp,"INVERSION\t%d\n",project->inversion);
		fprintf(hfp,"SMOOTHING\t%f\n",project->smoothing);
		fprintf(hfp,"ZOFFSETWIDTH\t%f\n",project->zoffsetwidth);
		for (i=0;i<project->num_files;i++)
			{
			/* write out basic file info */
			file = &project->files[i];
			fprintf(hfp,"FILE %4d %4d %4d %4d %4.1f %4.1f %4.1f %4.1f %4d %4d %s\n",
				i,
				file->status,
				file->id,
				file->format,
				file->heading_bias_import,
				file->roll_bias_import,
				file->heading_bias,
				file->roll_bias,
				file->num_sections,
				file->output_id,
				file->file);

			/* write out section info */
			for (j=0;j<file->num_sections;j++)
				{
				section = &file->sections[j];
				fprintf(hfp,"SECTION %4d %5d %5d %d %d %10.6f %16.6f %16.6f %13.8f %13.8f %13.8f %13.8f %9.3f %9.3f %d\n",
					j,
					section->num_pings,
					section->num_beams,
					section->num_snav,
					section->continuity,
					section->distance,
					section->btime_d,
					section->etime_d,
					section->lonmin,
					section->lonmax,
					section->latmin,
					section->latmax,
					section->depthmin,
					section->depthmax,
					section->contoursuptodate);
				for (k=MBNA_MASK_DIM-1;k>=0;k--)
				    {
				    for (l=0;l<MBNA_MASK_DIM;l++)
					{
					fprintf(hfp, "%1d", section->coverage[l+k*MBNA_MASK_DIM]);
					}
				    fprintf(hfp, "\n");
				    }
				for (k=0;k<section->num_snav;k++)
				    {
				    fprintf(hfp,"SNAV %4d %5d %10.6f %16.6f %13.8f %13.8f %13.8f %13.8f %13.8f\n",
					    k,
					    section->snav_id[k],
					    section->snav_distance[k],
					    section->snav_time_d[k],
					    section->snav_lon[k],
					    section->snav_lat[k],
					    section->snav_lon_offset[k],
					    section->snav_lat_offset[k],
					    section->snav_z_offset[k]);
				    }
				}
			}

		/* write out crossing info */
		for (i=0;i<project->num_crossings;i++)
			{
			/* write out basic crossing info */
			crossing = &project->crossings[i];
			fprintf(hfp,"CROSSING %5d %d %d %3d %5d %3d %5d %3d %2d\n",
				i,
				crossing->status,
				crossing->truecrossing,
				crossing->overlap,
				crossing->file_id_1,
				crossing->section_1,
				crossing->file_id_2,
				crossing->section_2,
				crossing->num_ties);

			/* write out tie info */
			for (j=0;j<crossing->num_ties;j++)
				{
				/* write out basic tie info */
				tie = &crossing->ties[j];
				fprintf(hfp,"TIE %5d %1d %5d %16.6f %5d %16.6f %13.8f %13.8f %13.8f %1.1d %13.8f %13.8f %13.8f\n",
					j,
					tie->status,
					tie->snav_1,
					tie->snav_1_time_d,
					tie->snav_2,
					tie->snav_2_time_d,
					tie->offset_x,
					tie->offset_y,
					tie->offset_z_m,
					tie->inversion_status,
					tie->inversion_offset_x,
					tie->inversion_offset_y,
					tie->inversion_offset_z_m);
				fprintf(hfp,"COV %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f\n",
					tie->sigmar1,
					tie->sigmax1[0],
					tie->sigmax1[1],
					tie->sigmax1[2],
					tie->sigmar2,
					tie->sigmax2[0],
					tie->sigmax2[1],
					tie->sigmax2[2],
					tie->sigmar3,
					tie->sigmax3[0],
					tie->sigmax3[1],
					tie->sigmax3[2]);
				}
			}

		/* close home file */
		fclose(hfp);
		status = MB_SUCCESS;
		}

	/* else set error */
	else
		{
		status = MB_FAILURE;
                *error = MB_ERROR_WRITE_FAIL;
		fprintf(stderr,"Unable to update project %s\n > Home file: %s\n",
			project->name, project->home);
		}

	/* open and write datalist file */
	sprintf(datalist,"%s%s.mb-1",project->path,project->name);
	if ((hfp = fopen(datalist,"w")) != NULL)
		{
		for (i=0;i<project->num_files;i++)
			{
			/* write file entry */
			file = &project->files[i];
			fprintf(hfp,"%s %d\n", file->file, file->format);
			}
		fclose(hfp);
		}

	/* else set error */
	else
		{
		status = MB_FAILURE;
                *error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"Unable to update project %s\n > Datalist file: %s\n",
			project->name, datalist);
		}

	/* write mbgrdviz route files in which each tie point or crossing is a two point route
		consisting of the connected snav points
		- output several different route files
		- route files of ties (fixed and unfixed separate) represent each tie as a
			two point route consisting of the connected snav points
		- route files of crossings (<25%, >= 25% && < 50%, >= 50%, true crossings)
			represent each crossing as a two point route consisting of the central
			snav points for each of the two sections.
		- first count different types of crossings and ties to output as routes
		- then output each time of route file */
	ncrossings_true = 0;
	ncrossings_gt50 = 0;
	ncrossings_gt25 = 0;
	ncrossings_lt25 = 0;
	ncrossings_fixed = 0;
	nties_unfixed = 0;
	nties_fixed = 0;
	for (i=0;i<project->num_crossings;i++)
		{
		crossing = &project->crossings[i];
		
		/* check all crossings */
		if (project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
			    || project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)
			ncrossings_fixed++;
		else
			{
			if (crossing->truecrossing == MB_YES)
				ncrossings_true++;
			else if (crossing->overlap >= 50)
				ncrossings_gt50++;
			else if (crossing->overlap >= 25)
				ncrossings_gt25++;
			else
				ncrossings_lt25++;
			}
    
		/* check ties */
		if (crossing->status == MBNA_CROSSING_STATUS_SET)
			{
			if (project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				|| project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)
				nties_fixed += crossing->num_ties;
			else
				nties_unfixed += crossing->num_ties;
			}
		}		

	/* write mbgrdviz route file for all unfixed true crossings */
	sprintf(routefile,"%s%s_truecrossing.rte",project->path,project->name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, " > Unable to open output tie route file %s\n", routefile);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",version_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",ncrossings_true);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project->num_crossings;i++)
			{
			crossing = &project->crossings[i];
    
			/* output only unfixed true crossings */
			if (crossing->truecrossing == MB_YES
			    && !(project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				    || project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				file_1 = (struct mbna_file *) &project->files[crossing->file_id_1];
				file_2 = (struct mbna_file *) &project->files[crossing->file_id_2];
				section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
				section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
				snav_1 = section_1->num_snav/2;
				snav_2 = section_2->num_snav/2;
				navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
				navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
				navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
				navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					{
					status_char = 'U';
					routecolor = ROUTE_COLOR_YELLOW;
					}
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					{
					status_char = '*';
					routecolor = ROUTE_COLOR_GREEN;
					}
				else
					{
					status_char = '-';
					routecolor = ROUTE_COLOR_RED;
					}
				if (crossing->truecrossing == MB_NO)
					truecrossing_char = ' ';
				else
					truecrossing_char = 'X';
				sprintf(routename,"%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d",
					status_char, truecrossing_char, i,
					file_1->block,
					crossing->file_id_1,
					crossing->section_1,
					file_2->block,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
				fprintf(hfp,"## ROUTENAME %s\n", routename);
				fprintf(hfp,"## ROUTESIZE %d\n", 1);
				fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
				fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
				fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
				fprintf(hfp,"> ## STARTROUTE\n");
				fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
					navlon1,navlat1,navlon2,navlat2);
				nroute++;
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) true crossing locations to %s\n", nroute, ncrossings_true, routefile);
		}

	/* write mbgrdviz route file for all unfixed >=50% crossings */
	sprintf(routefile,"%s%s_gt50crossing.rte",project->path,project->name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, " > Unable to open output tie route file %s\n", routefile);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",version_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",ncrossings_gt50);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project->num_crossings;i++)
			{
			crossing = &project->crossings[i];
   
			/* output only unfixed >=50% crossings */
			if (crossing->overlap >= 50
			    && !(project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				    || project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				file_1 = (struct mbna_file *) &project->files[crossing->file_id_1];
				file_2 = (struct mbna_file *) &project->files[crossing->file_id_2];
				section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
				section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
				snav_1 = section_1->num_snav/2;
				snav_2 = section_2->num_snav/2;
				navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
				navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
				navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
				navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					{
					status_char = 'U';
					routecolor = ROUTE_COLOR_YELLOW;
					}
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					{
					status_char = '*';
					routecolor = ROUTE_COLOR_GREEN;
					}
				else
					{
					status_char = '-';
					routecolor = ROUTE_COLOR_RED;
					}
				if (crossing->truecrossing == MB_NO)
					truecrossing_char = ' ';
				else
					truecrossing_char = 'X';
				sprintf(routename,"%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d",
					status_char, truecrossing_char, i,
					file_1->block,
					crossing->file_id_1,
					crossing->section_1,
					file_2->block,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
				fprintf(hfp,"## ROUTENAME %s\n", routename);
				fprintf(hfp,"## ROUTESIZE %d\n", 1);
				fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
				fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
				fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
				fprintf(hfp,"> ## STARTROUTE\n");
				fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
					navlon1,navlat1,navlon2,navlat2);
				nroute++;
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) >=50%% overlap crossing locations to %s\n", nroute, ncrossings_gt50, routefile);
		}

	/* write mbgrdviz route file for all unfixed >=25% but less than 50% crossings */
	sprintf(routefile,"%s%s_gt25crossing.rte",project->path,project->name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, " > Unable to open output tie route file %s\n", routefile);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",version_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",ncrossings_gt25);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project->num_crossings;i++)
			{
			crossing = &project->crossings[i];
    
			/* output only unfixed >=25% but less than 50% crossings crossings */
			if (crossing->overlap >= 25
			    && !(project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				    || project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				file_1 = (struct mbna_file *) &project->files[crossing->file_id_1];
				file_2 = (struct mbna_file *) &project->files[crossing->file_id_2];
				section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
				section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
				snav_1 = section_1->num_snav/2;
				snav_2 = section_2->num_snav/2;
				navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
				navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
				navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
				navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					{
					status_char = 'U';
					routecolor = ROUTE_COLOR_YELLOW;
					}
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					{
					status_char = '*';
					routecolor = ROUTE_COLOR_GREEN;
					}
				else
					{
					status_char = '-';
					routecolor = ROUTE_COLOR_RED;
					}
				if (crossing->truecrossing == MB_NO)
					truecrossing_char = ' ';
				else
					truecrossing_char = 'X';
				sprintf(routename,"%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d",
					status_char, truecrossing_char, i,
					file_1->block,
					crossing->file_id_1,
					crossing->section_1,
					file_2->block,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
				fprintf(hfp,"## ROUTENAME %s\n", routename);
				fprintf(hfp,"## ROUTESIZE %d\n", 1);
				fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
				fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
				fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
				fprintf(hfp,"> ## STARTROUTE\n");
				fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
					navlon1,navlat1,navlon2,navlat2);
				nroute++;
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) >=25%% && < 50%% overlap crossing locations to %s\n", nroute, ncrossings_gt25, routefile);
		}

	/* write mbgrdviz route file for all unfixed <25% crossings */
	sprintf(routefile,"%s%s_lt25crossing.rte",project->path,project->name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, " > Unable to open output tie route file %s\n", routefile);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",version_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",ncrossings_lt25);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project->num_crossings;i++)
			{
			crossing = &project->crossings[i];
    
			/* output only unfixed <25% crossings crossings */
			if (crossing->overlap < 25
			    && !(project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				    || project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				file_1 = (struct mbna_file *) &project->files[crossing->file_id_1];
				file_2 = (struct mbna_file *) &project->files[crossing->file_id_2];
				section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
				section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
				snav_1 = section_1->num_snav/2;
				snav_2 = section_2->num_snav/2;
				navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
				navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
				navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
				navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					{
					status_char = 'U';
					routecolor = ROUTE_COLOR_YELLOW;
					}
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					{
					status_char = '*';
					routecolor = ROUTE_COLOR_GREEN;
					}
				else
					{
					status_char = '-';
					routecolor = ROUTE_COLOR_RED;
					}
				if (crossing->truecrossing == MB_NO)
					truecrossing_char = ' ';
				else
					truecrossing_char = 'X';
				sprintf(routename,"%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d",
					status_char, truecrossing_char, i,
					file_1->block,
					crossing->file_id_1,
					crossing->section_1,
					file_2->block,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
				fprintf(hfp,"## ROUTENAME %s\n", routename);
				fprintf(hfp,"## ROUTESIZE %d\n", 1);
				fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
				fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
				fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
				fprintf(hfp,"> ## STARTROUTE\n");
				fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
					navlon1,navlat1,navlon2,navlat2);
				nroute++;
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) <25%% overlap crossing locations to %s\n", nroute, ncrossings_lt25, routefile);
		}

	/* write mbgrdviz route file for all fixed crossings */
	sprintf(routefile,"%s%s_fixedcrossing.rte",project->path,project->name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, " > Unable to open output fixed crossings route file %s\n", routefile);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",version_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",ncrossings_fixed);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project->num_crossings;i++)
			{
			crossing = &project->crossings[i];
  
			/* output only fixed crossings */
			if (project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				    || project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)
				{
				file_1 = (struct mbna_file *) &project->files[crossing->file_id_1];
				file_2 = (struct mbna_file *) &project->files[crossing->file_id_2];
				section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
				section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
				snav_1 = section_1->num_snav/2;
				snav_2 = section_2->num_snav/2;
				navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
				navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
				navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
				navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					{
					status_char = 'U';
					routecolor = ROUTE_COLOR_YELLOW;
					}
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					{
					status_char = '*';
					routecolor = ROUTE_COLOR_GREEN;
					}
				else
					{
					status_char = '-';
					routecolor = ROUTE_COLOR_RED;
					}
				if (crossing->truecrossing == MB_NO)
					truecrossing_char = ' ';
				else
					truecrossing_char = 'X';
				sprintf(routename,"%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d",
					status_char, truecrossing_char, i,
					file_1->block,
					crossing->file_id_1,
					crossing->section_1,
					file_2->block,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
				fprintf(hfp,"## ROUTENAME %s\n", routename);
				fprintf(hfp,"## ROUTESIZE %d\n", 1);
				fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
				fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
				fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
				fprintf(hfp,"> ## STARTROUTE\n");
				fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
					navlon1,navlat1,navlon2,navlat2);
				nroute++;
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) fixed crossing locations to %s\n", nroute, ncrossings_fixed, routefile);
		}

	/* write mbgrdviz route file for all unfixed ties */
	sprintf(routefile,"%s%s_unfixedties.rte",project->path,project->name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, " > Unable to open output unfixed ties route file %s\n", routefile);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",version_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",nties_unfixed);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");
		routecolor = ROUTE_COLOR_BLUEGREEN;

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project->num_crossings;i++)
			{
			crossing = &project->crossings[i];
   
			/* output only unfixed ties */
			if (crossing->status == MBNA_CROSSING_STATUS_SET
				&& !(project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
					|| project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				for (j=0;j<crossing->num_ties;j++)
					{
					file_1 = (struct mbna_file *) &project->files[crossing->file_id_1];
					file_2 = (struct mbna_file *) &project->files[crossing->file_id_2];
					section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
					section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
					tie = (struct mbna_tie *) &crossing->ties[j];
					snav_1 = tie->snav_1;
					snav_2 = tie->snav_2;
					navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
					navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
					navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
					navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					if (crossing->truecrossing == MB_NO)
						truecrossing_char = ' ';
					else
						truecrossing_char = 'X';
					sprintf(routename,"Tie: %c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d of %2d",
						status_char, truecrossing_char, i,
						file_1->block,
						crossing->file_id_1,
						crossing->section_1,
						file_2->block,
						crossing->file_id_2,
						crossing->section_2,
						crossing->overlap,
						j, crossing->num_ties);
					fprintf(hfp,"## ROUTENAME %s\n", routename);
					fprintf(hfp,"## ROUTESIZE %d\n", 1);
					fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
					fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
					fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
					fprintf(hfp,"> ## STARTROUTE\n");
					fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
						navlon1,navlat1,navlon2,navlat2);
					nroute++;
					}
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) unfixed tie locations to %s\n", nroute, nties_unfixed, routefile);
		}

	/* write mbgrdviz route file for all fixed ties */
	sprintf(routefile,"%s%s_fixedties.rte",project->path,project->name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, " > Unable to open output fixed ties route file %s\n", routefile);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",version_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",nties_fixed);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");
		routecolor = ROUTE_COLOR_RED;

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project->num_crossings;i++)
			{
			crossing = &project->crossings[i];
   
			/* output only fixed ties */
			if (crossing->status == MBNA_CROSSING_STATUS_SET
				&& (project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
					|| project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				for (j=0;j<crossing->num_ties;j++)
					{
					file_1 = (struct mbna_file *) &project->files[crossing->file_id_1];
					file_2 = (struct mbna_file *) &project->files[crossing->file_id_2];
					section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
					section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
					tie = (struct mbna_tie *) &crossing->ties[j];
					snav_1 = tie->snav_1;
					snav_2 = tie->snav_2;
					navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
					navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
					navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
					navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					if (crossing->truecrossing == MB_NO)
						truecrossing_char = ' ';
					else
						truecrossing_char = 'X';
					sprintf(routename,"Tie: %c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d of %2d",
						status_char, truecrossing_char, i,
						file_1->block,
						crossing->file_id_1,
						crossing->section_1,
						file_2->block,
						crossing->file_id_2,
						crossing->section_2,
						crossing->overlap,
						j, crossing->num_ties);
					fprintf(hfp,"## ROUTENAME %s\n", routename);
					fprintf(hfp,"## ROUTESIZE %d\n", 1);
					fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
					fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
					fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
					fprintf(hfp,"> ## STARTROUTE\n");
					fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
						navlon1,navlat1,navlon2,navlat2);
					nroute++;
					}
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) fixed tie locations to %s\n", nroute, nties_fixed, routefile);
		}

	/* output offset vectors */
	if (project->inversion == MBNA_INVERSION_CURRENT)
		{
		sprintf(xoffsetfile,"%s%s_dx.txt",project->path,project->name);
		sprintf(yoffsetfile,"%s%s_dy.txt",project->path,project->name);
		if ((xfp = fopen(xoffsetfile,"w")) != NULL
		    && (yfp = fopen(yoffsetfile,"w")) != NULL)
			{
			for (i=0;i<project->num_files;i++)
			    {
			    file = &project->files[i];
			    for (j=0;j<file->num_sections;j++)
				{
				section = &file->sections[j];
                                mb_coor_scale(verbose,0.5 * (section->latmin + section->latmax),
                                                                    &mtodeglon,&mtodeglat);
				for (k=0;k<section->num_snav;k++)
				    {
				    fprintf(xfp, "%.10f %.10f %.10f\n", section->snav_lon[k], section->snav_lat[k],
								section->snav_lon_offset[k]/mtodeglon);
				    fprintf(yfp, "%.10f %.10f %.10f\n", section->snav_lon[k], section->snav_lat[k],
								section->snav_lat_offset[k]/mtodeglat);
				    }
				}
			    }
			fclose(xfp);
			fclose(yfp);
			}

		/* else set error */
		else
			{
			status = MB_FAILURE;
                        *error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"Unable to update project %s\n > Offset vector files: %s %s\n",
				project->name, xoffsetfile, yoffsetfile);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
