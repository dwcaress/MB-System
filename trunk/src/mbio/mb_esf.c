/*--------------------------------------------------------------------
 *    The MB-system:	mb_esf.c	4/10/2003
 *    $Id: mb_esf.c,v 5.0 2003-04-16 16:45:50 caress Exp $
 *
 *    Copyright (c) 2003 by
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
 * mb_esf.c includes the functions used to read, write, and use
 * edit save files.
 *
 * Author:	D. W. Caress
 * Date:	April 10, 2003
 *
 * $Log: not supported by cvs2svn $
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_process.h"
#include "../../include/mb_swap.h"

static char rcs_id[]="$Id: mb_esf.c,v 5.0 2003-04-16 16:45:50 caress Exp $";

/*--------------------------------------------------------------------*/
/* 	function mb_esf_check checks for an existing esf file. */
int mb_esf_check(int verbose, char *swathfile, char *esffile, 
		int *found, int *error)
{
  	char	*function_name = "mb_esf_check";
	int	status = MB_SUCCESS;
	int	mbp_edit_mode;
	char	mbp_editfile[MB_PATH_MAXLINE];
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:     %d\n",verbose);
		fprintf(stderr,"dbg2       swathfile:   %s\n",swathfile);
		}

	/* check if edit save file is set in mbprocess parameter file
		or just lying around */
	status = mb_pr_get_edit(verbose, swathfile, 
			&mbp_edit_mode, mbp_editfile, error);
	if (mbp_edit_mode == MBP_EDIT_ON)
		{
		*found = MB_YES;
		strcpy(esffile, mbp_editfile);
		}
	else
		{
		*found = MB_NO;
		sprintf(esffile, "%s.esf", swathfile);
		}

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       esfile:      %s\n",esffile);
		fprintf(stderr,"dbg2       found:       %d\n",*found);
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return success */
	return(status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_esf_load starts handling an edit save file for
		the specified swath file.
		The load flag indicates whether an existing esf file 
			should be loaded. 
		The output flag indicates whether an output
			esf file should be opened, 
			overwriting any existing esf file. Any
			existing esf file will be backed up first. 
		If both load and output are MB_NO, nothing will be
		done. */
int mb_esf_load(int verbose, char *swathfile,
		int load, int output,
		char *esffile, 
		struct mb_esf_struct *esf,
		int *error)
{
  	char	*function_name = "mb_esf_load";
	int	status = MB_SUCCESS;
	int	found;
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:     %d\n",verbose);
		fprintf(stderr,"dbg2       swathfile:   %s\n",swathfile);
		fprintf(stderr,"dbg2       load:        %d\n",load);
		fprintf(stderr,"dbg2       output:      %d\n",output);
		}

	/* get name of existing or new esffile, then load old edits
		and/or open new esf file */
	status = mb_esf_check(verbose, swathfile, esffile, &found, error);
	if (load == MB_YES || output == MB_YES)
		{
		status = mb_esf_open(verbose, esffile, 
				load, output, esf, error);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       esfile:      %s\n",esffile);
		fprintf(stderr,"dbg2       nedit:       %d\n",esf->nedit);
		for (i=0;i<esf->nedit;i++)
			fprintf(stderr,"dbg2       edit event:  %d %.6f %5d %3d %3d\n",
				i,esf->edit_time_d[i],esf->edit_beam[i],
				esf->edit_action[i],esf->edit_use[i]);
		fprintf(stderr,"dbg2       esf->esffp:  %d\n",esf->esffp);
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return success */
	return(status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_esf_open starts handling of an edit save file.
		The load flag indicates whether an existing esf file 
			should be loaded. 
		The output flag indicates whether an output
			esf file should be opened, 
			overwriting any existing esf file. Any
			existing esf file will be backed up first. */
int mb_esf_open(int verbose, char *esffile, 
		int load, int output,
		struct mb_esf_struct *esf,
		int *error)
{
  	char	*function_name = "mb_esf_open";
	int	status = MB_SUCCESS;
	char	command[MB_PATH_MAXLINE];
	FILE	*esffp;
	struct stat file_status;
	int	fstat;
	int	insert;
	double	stime_d;
	int	sbeam;
	int	saction;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:     %d\n",verbose);
		fprintf(stderr,"dbg2       esffile:     %s\n",esffile);
		fprintf(stderr,"dbg2       load:        %d\n",load);
		fprintf(stderr,"dbg2       output:      %d\n",output);
		}
		
	/* initialize the esf structure */
	esf->nedit = 0;
	strcpy(esf->esffile, esffile);
	esf->edit_time_d = NULL;
	esf->edit_beam = NULL;
	esf->edit_action = NULL;
	esf->edit_use = NULL;
	esf->esffp = NULL;
	
	/* load edits from existing esf file if requested */
	if (load == MB_YES)
		{

		/* check that esf file exists */
		fstat = stat(esffile, &file_status);
		if (fstat == 0
		    && (file_status.st_mode & S_IFMT) != S_IFDIR)
		    {
		    /* save filename in structure */
		    strcpy(esf->esffile, esffile);

		    /* get number of old edits */
		    esf->nedit = file_status.st_size
				/ (sizeof(double) + 2 * sizeof(int));

		    /* allocate arrays for old edits */
		    if (esf->nedit > 0)
			{
			status = mb_malloc(verbose, esf->nedit *sizeof(double), &(esf->edit_time_d), error);
			if (status == MB_SUCCESS)
			status = mb_malloc(verbose, esf->nedit *sizeof(int), &(esf->edit_beam), error);
			if (status == MB_SUCCESS)
			status = mb_malloc(verbose, esf->nedit *sizeof(int), &(esf->edit_action), error);
			if (status == MB_SUCCESS)
			status = mb_malloc(verbose, esf->nedit *sizeof(int), &(esf->edit_use), error);
			memset(esf->edit_time_d, 0, esf->nedit *sizeof(double));
			memset(esf->edit_beam, 0, esf->nedit *sizeof(int));
			memset(esf->edit_action, 0, esf->nedit *sizeof(int));
			memset(esf->edit_use, 0, esf->nedit *sizeof(int));

			/* if error initializing memory then quit */
			if (status != MB_SUCCESS)
			    {
			    *error = MB_ERROR_MEMORY_FAIL;
			    fprintf(stderr, "\nUnable to allocate memory for %d edit events\n",
				esf->nedit);
			    esf->nedit = 0;
			    return(status);
			    }	
			}	

		    /* open and read the old edit file */
		    if (status == MB_SUCCESS
	    		&& esf->nedit > 0
			&& (esffp = fopen(esffile,"r")) == NULL)
			{
			esf->nedit = 0;
			*error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open edit save file %s\n",
			    esffile);
			}
		    else if (status == MB_SUCCESS
	    		&& esf->nedit > 0)
			{
			/* reset message */
			if (verbose > 0)
				fprintf(stderr, "Sorting %d old edits...\n", esf->nedit);

			error = MB_ERROR_NO_ERROR;
			insert = 0;
			for (i=0;i<esf->nedit && error == MB_ERROR_NO_ERROR;i++)
			    {
			    /* reset message */
			    if ((i+1)%10000 == 0)
				{
				if (verbose > 0)
				fprintf(stderr, "%d of %d old edits sorted...\n", i+1, esf->nedit);
				}

			    if (fread(&stime_d, sizeof(double), 1, esffp) != 1
				|| fread(&sbeam, sizeof(int), 1, esffp) != 1
				|| fread(&saction, sizeof(int), 1, esffp) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
#ifdef BYTESWAPPED
			    else
				{
				mb_swap_double(&stime_d);
				sbeam = mb_swap_int(sbeam);
				saction = mb_swap_int(saction);
				}
#endif

			    /* insert into sorted array */
			    if (i > 0)
				{
		    		while (insert > 0 && stime_d < esf->edit_time_d[insert-1])
		    		    insert--;
		    		while (insert < i && stime_d >= esf->edit_time_d[insert])
		    		    insert++;
				if (insert < i)
				    {
				    memmove(&esf->edit_time_d[insert+1], 
					    &esf->edit_time_d[insert], 
					    sizeof(double) * (i - insert));
				    memmove(&esf->edit_beam[insert+1], 
					    &esf->edit_beam[insert], 
					    sizeof(int) * (i - insert));
				    memmove(&esf->edit_action[insert+1], 
					    &esf->edit_action[insert], 
					    sizeof(int) * (i - insert));
				    }
				}
			    esf->edit_time_d[insert] = stime_d;
			    esf->edit_beam[insert] = sbeam;
			    esf->edit_action[insert] = saction;
/*fprintf(stderr,"EDITS READ: i:%d edit: %f %d %d  use:%d\n",
insert,esf->edit_time_d[insert],esf->edit_beam[insert],
esf->edit_action[insert],esf->edit_use[insert]);*/
			    }
			fclose(esffp);
			}
		    }
	    	}
		
	if (status == MB_SUCCESS
		&& output == MB_YES)
	    	{
		/* save filename in structure */
		strcpy(esf->esffile, esffile);

		/* check if esf file exists */
		fstat = stat(esffile, &file_status);
		if (fstat == 0
		    && (file_status.st_mode & S_IFMT) != S_IFDIR)
		    {
		    /* copy old edit save file to tmp file */
		    if (load == MB_YES)
	    		{
			sprintf(command, "cp %s %s.tmp\n", 
				esffile, esffile);
	    		system(command);
			}
		    }
		
		/* open the edit save file */
		if ((esf->esffp = fopen(esffile,"w")) == NULL)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_OPEN_FAIL;
		    }
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       nedit:       %d\n",esf->nedit);
		for (i=0;i<esf->nedit;i++)
			fprintf(stderr,"dbg2       edit event:  %d %.6f %5d %3d %3d\n",
				i,esf->edit_time_d[i],esf->edit_beam[i],
				esf->edit_action[i],esf->edit_use[i]);
		fprintf(stderr,"dbg2       esf->esffp:  %d\n",esf->esffp);
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return success */
	return(status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_esf_apply applies saved edits to the beamflags
	in a ping. If an output esf file is open the applied edits
	are saved to that file. */
int mb_esf_apply(int verbose, struct mb_esf_struct *esf,
		double time_d, int nbath, char *beamflag, 
		int *error)
{
  	char	*function_name = "mb_esf_apply";
	int	status = MB_SUCCESS;
	int	firstedit, lastedit;
	int	apply, action;
	char	beamflagorg;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:     %d\n",verbose);
		fprintf(stderr,"dbg2       nedit:  %d\n",esf->nedit);
		for (i=0;i<esf->nedit;i++)
			fprintf(stderr,"dbg2       edit event: %d %.6f %5d %3d %3d\n",
				i,esf->edit_time_d[i],esf->edit_beam[i],
				esf->edit_action[i],esf->edit_use[i]);
		fprintf(stderr,"dbg2       time_d:      %f\n",time_d);
		fprintf(stderr,"dbg2       nbath:       %d\n",nbath);
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2       beamflag:    %d %d\n",i,beamflag[i]);
		}

	/* find first and last edits for this ping */
	firstedit = 0;
	lastedit = firstedit - 1;
	for (j = firstedit; j < esf->nedit && time_d >= esf->edit_time_d[j]; j++)
		{
		if (fabs(esf->edit_time_d[j] - time_d) < MB_ESF_MAXTIMEDIFF)
		    {
		    if (lastedit < firstedit)
			firstedit = j;
		    lastedit = j;
		    }
		}
			
	/* apply edits */
	if (lastedit >= firstedit)
		{
		/* check for edits with bad beam numbers */
		for (j=firstedit;j<=lastedit;j++)
		    {
		    if (esf->edit_beam[j] < 0 
		    	|| esf->edit_beam[j] >= nbath)
		    	esf->edit_use[j] += 1000;
		    }
		    
		/* loop over all beams */
		for (i=0;i<nbath;i++)
		    {
		    /* loop over all edits for this ping */
		    apply = MB_NO;
		    beamflagorg = beamflag[i];
		    for (j=firstedit;j<=lastedit;j++)
			{
			/* apply the edits for this beam in the
			   order they were created so that the last
			   edit event is applied last - only the 
			   last event will be output to a new
			   esf file */
			if (esf->edit_beam[j] == i)
			    {
			    /* apply edit */
			    if (esf->edit_action[j] == MBP_EDIT_FLAG
				&& beamflag[i] != MB_FLAG_NULL)
				{
				beamflag[i] 
				    = MB_FLAG_FLAG + MB_FLAG_MANUAL;
				esf->edit_use[j]++;
				apply = MB_YES;
				action = esf->edit_action[j];
				}
			    else if (esf->edit_action[j] == MBP_EDIT_FILTER
				&& beamflag[i] != MB_FLAG_NULL)
				{
				beamflag[i] 
				    = MB_FLAG_FLAG + MB_FLAG_FILTER;
				esf->edit_use[j]++;
				apply = MB_YES;
				action = esf->edit_action[j];
				}
			    else if (esf->edit_action[j] == MBP_EDIT_UNFLAG
				&& beamflag[i] != MB_FLAG_NULL)
				{
				beamflag[i] = MB_FLAG_NONE;
				esf->edit_use[j]++;
				apply = MB_YES;
				action = esf->edit_action[j];
				}
			    else if (esf->edit_action[j] == MBP_EDIT_ZERO)
				{
				beamflag[i] = MB_FLAG_NULL;
				esf->edit_use[j]++;
				apply = MB_YES;
				action = esf->edit_action[j];
				}
			    else
				{
				esf->edit_use[j] += 100;
/*fprintf(stderr,"Dup Edit[%d]?: ping:%f beam:%d flag:%d action:%d\n",
j, time_d, i, beamflag[i], esf->edit_action[j]);*/
				}
			    }
			}
		    if (apply == MB_YES 
		    	&& esf->esffp != NULL
			&& beamflag[i] != beamflagorg)
		    	mb_esf_save(verbose, esf, time_d, i, action, error);
		    }
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       nedit:  %d\n",esf->nedit);
		for (i=0;i<esf->nedit;i++)
			fprintf(stderr,"dbg2       edit event: %d %.6f %5d %3d %3d\n",
				i,esf->edit_time_d[i],esf->edit_beam[i],
				esf->edit_action[i],esf->edit_use[i]);
		fprintf(stderr,"dbg2       error:  %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return success */
	return(status);
}


/*--------------------------------------------------------------------*/
/* 	function mb_esf_save saves one edit event to an esf file. */
int mb_esf_save(int verbose, struct mb_esf_struct *esf, 
		double time_d, int beam, int action, int *error)
{
  	char	*function_name = "mb_esf_save";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       esf->nedit:       %d\n",esf->nedit);
		fprintf(stderr,"dbg2       esf->edit_time_d: %d\n",esf->edit_time_d);
		fprintf(stderr,"dbg2       esf->edit_beam:   %d\n",esf->edit_beam);
		fprintf(stderr,"dbg2       esf->edit_action: %d\n",esf->edit_action);
		fprintf(stderr,"dbg2       esf->edit_use:    %d\n",esf->edit_use);
		fprintf(stderr,"dbg2       esf->esffp:       %d\n",esf->esffp);
		fprintf(stderr,"dbg2       time_d:           %f\n",time_d);
		fprintf(stderr,"dbg2       beam:             %d\n",beam);
		fprintf(stderr,"dbg2       action:           %d\n",action);
		}

	/* write out the edit */
/* fprintf(stderr,"OUTPUT EDIT: %f %d %d\n",time_d,beam,action);*/
	if (esf->esffp != NULL)
	    {		
#ifdef BYTESWAPPED
	    mb_swap_double(&time_d);
	    beam = mb_swap_int(beam);
	    action = mb_swap_int(action);
#endif
	    if (fwrite(&time_d, sizeof(double), 1, esf->esffp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    if (status == MB_SUCCESS
		&& fwrite(&beam, sizeof(int), 1, esf->esffp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    if (status == MB_SUCCESS
		&& fwrite(&action, sizeof(int), 1, esf->esffp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       esf->nedit:       %d\n",esf->nedit);
		fprintf(stderr,"dbg2       esf->edit_time_d: %d\n",esf->edit_time_d);
		fprintf(stderr,"dbg2       esf->edit_beam:   %d\n",esf->edit_beam);
		fprintf(stderr,"dbg2       esf->edit_action: %d\n",esf->edit_action);
		fprintf(stderr,"dbg2       esf->edit_use:    %d\n",esf->edit_use);
		fprintf(stderr,"dbg2       esf->esffp:       %d\n",esf->esffp);
		fprintf(stderr,"dbg2       error:            %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:           %d\n",status);
		}

	/* return success */
	return(status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_esf_close deallocates memory in the esf structure. */
int mb_esf_close(int verbose, struct mb_esf_struct *esf, int *error)
{
  	char	*function_name = "mb_esf_close";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       esf->nedit:       %d\n",esf->nedit);
		fprintf(stderr,"dbg2       esf->edit_time_d: %d\n",esf->edit_time_d);
		fprintf(stderr,"dbg2       esf->edit_beam:   %d\n",esf->edit_beam);
		fprintf(stderr,"dbg2       esf->edit_action: %d\n",esf->edit_action);
		fprintf(stderr,"dbg2       esf->edit_use:    %d\n",esf->edit_use);
		fprintf(stderr,"dbg2       esf->esffp:       %d\n",esf->esffp);
		}

	/* deallocate the arrays */
	esf->nedit = 0;
	if (esf->nedit != 0)
		{
		if (esf->edit_time_d != NULL)
			status = mb_free(verbose, &(esf->edit_time_d), error);
		if (esf->edit_beam != NULL)
			status = mb_free(verbose, &(esf->edit_beam), error);
		if (esf->edit_action != NULL)
			status = mb_free(verbose, &(esf->edit_action), error);
		if (esf->edit_use != NULL)
			status = mb_free(verbose, &(esf->edit_use), error);
		}

	/* close the esf file */
	if (esf->esffp != NULL)
		{
		fclose(esf->esffp);
		esf->esffp = NULL;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       esf->nedit:       %d\n",esf->nedit);
		fprintf(stderr,"dbg2       esf->edit_time_d: %d\n",esf->edit_time_d);
		fprintf(stderr,"dbg2       esf->edit_beam:   %d\n",esf->edit_beam);
		fprintf(stderr,"dbg2       esf->edit_action: %d\n",esf->edit_action);
		fprintf(stderr,"dbg2       esf->edit_use:    %d\n",esf->edit_use);
		fprintf(stderr,"dbg2       esf->esffp:       %d\n",esf->esffp);
		fprintf(stderr,"dbg2       error:            %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:           %d\n",status);
		}

	/* return success */
	return(status);
}

/*--------------------------------------------------------------------*/
