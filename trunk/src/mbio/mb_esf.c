/*--------------------------------------------------------------------
 *    The MB-system:	mb_esf.c	4/10/2003
 *    $Id: mb_esf.c,v 5.13 2008/07/10 06:43:40 caress Exp $
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
 * $Log: mb_esf.c,v $
 * Revision 5.13  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.12  2007/07/03 17:33:07  caress
 * A couple of debug statements added.
 *
 * Revision 5.11  2007/06/18 01:19:48  caress
 * Changes as of 17 June 2007.
 *
 * Revision 5.10  2006/01/24 19:11:17  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.9  2006/01/06 18:27:19  caress
 * Working towards 5.0.8
 *
 * Revision 5.8  2005/04/07 04:24:33  caress
 * 5.0.7 Release.
 *
 * Revision 5.7  2005/03/26 22:05:18  caress
 * Release 5.0.7.
 *
 * Revision 5.6  2004/12/02 06:33:30  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.5  2004/04/27 01:46:12  caress
 * Various updates of April 26, 2004.
 *
 * Revision 5.4  2004/02/24 22:29:02  caress
 * Fixed errors in handling Simrad datagrams and edit save files on byteswapped machines (e.g. Intel or AMD processors).
 *
 * Revision 5.3  2003/07/30 16:19:20  caress
 * Changes during iSSP meeting July 2003.
 *
 * Revision 5.2  2003/07/27 21:58:57  caress
 * Added mb_mergesort function for 5.0.0
 *
 * Revision 5.1  2003/07/26 17:59:32  caress
 * Changed beamflag handling code.
 *
 * Revision 5.0  2003/04/16 16:45:50  caress
 * Initial Version.
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_process.h"
#include "../../include/mb_swap.h"

static char rcs_id[]="$Id: mb_esf.c,v 5.13 2008/07/10 06:43:40 caress Exp $";

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
		
	/* initialize the esf structure */
	esf->nedit = 0;
	esf->esffile[0] = '\0';
	esf->esstream[0] = '\0';
	esf->edit = NULL;
	esf->esffp = NULL;
	esf->essfp = NULL;
	esf->byteswapped = mb_swap_check();

	/* get name of existing or new esffile, then load old edits
		and/or open new esf file */
	status = mb_esf_check(verbose, swathfile, esffile, &found, error);
	if ((load == MB_YES && found == MB_YES) || output != MBP_ESF_NOWRITE)
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
				i,esf->edit[i].time_d,esf->edit[i].beam,
				esf->edit[i].action,esf->edit[i].use);
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
		The output flag indicates whether to open an output
			edit save file and edit save stream. If 
			the output flag is MBP_ESF_WRITE a new 
			esf file is created. If the output flag is
			MBP_ESF_APPEND then edit events are appended
			to any existing esf file. Any
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
	double	stime_d;
	int	sbeam;
	int	saction;
	char	fmode[16];
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
		fprintf(stderr,"dbg2       esf:         %d\n",esf);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		}
		
	/* initialize the esf structure */
	esf->nedit = 0;
	strcpy(esf->esffile, esffile);
	sprintf(esf->esstream, "%s.stream", esffile);
	esf->edit = NULL;
	esf->esffp = NULL;
	esf->essfp = NULL;
	esf->byteswapped = mb_swap_check();
	
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
			status = mb_mallocd(verbose, __FILE__, __LINE__, esf->nedit * sizeof(struct mb_edit_struct), (void **)&(esf->edit), error);
			if (status == MB_SUCCESS)
			memset(esf->edit, 0, esf->nedit * sizeof(struct mb_edit_struct));

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
			&& (esffp = fopen(esffile,"rw")) == NULL)
			{
			fprintf(stderr, "\nnedit:%d\n",
			    esf->nedit);
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
				fprintf(stderr, "Reading %d old edits...\n", esf->nedit);

			*error = MB_ERROR_NO_ERROR;
			for (i=0;i<esf->nedit && *error == MB_ERROR_NO_ERROR;i++)
			    {
			    if (fread(&(esf->edit[i].time_d), sizeof(double), 1, esffp) != 1
				|| fread(&(esf->edit[i].beam), sizeof(int), 1, esffp) != 1
				|| fread(&(esf->edit[i].action), sizeof(int), 1, esffp) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			    else if (esf->byteswapped == MB_YES)
				{
				mb_swap_double(&(esf->edit[i].time_d));
				esf->edit[i].beam = mb_swap_int(esf->edit[i].beam);
				esf->edit[i].action = mb_swap_int(esf->edit[i].action);
				}
/*fprintf(stderr,"EDITS READ: i:%d edit: %f %d %d  use:%d\n",
i,esf->edit[i].time_d,esf->edit[i].beam,
esf->edit[i].action,esf->edit[i].use);*/
			    }
			    
			/* close the file */
			fclose(esffp);

			/* reset message */
			if (verbose > 0)
				fprintf(stderr, "Sorting %d old edits...\n", esf->nedit);
			
			/* now sort the edits */
			mb_mergesort((char *)esf->edit, esf->nedit, 
					sizeof(struct mb_edit_struct), mb_edit_compare);
/*for (i=0;i<esf->nedit;i++)
fprintf(stderr,"EDITS SORTED: i:%d edit: %f %d %d  use:%d\n",
i,esf->edit[i].time_d,esf->edit[i].beam,
esf->edit[i].action,esf->edit[i].use);*/
			}
		    }
	    	}
		
	if (status == MB_SUCCESS
		&& output != MBP_ESF_NOWRITE)
	    	{
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
		if (output == MBP_ESF_WRITE)
			strcpy(fmode,"wb");
		else if (output == MBP_ESF_APPEND)
			strcpy(fmode,"ab");
		if ((esf->esffp = fopen(esf->esffile,fmode)) == NULL)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_OPEN_FAIL;
		    }
/*else
fprintf(stderr,"esffile %s opened with mode %s\n",esf->esffile,fmode);*/
		
		/* open the edit save stream file */
		if ((esf->essfp = fopen(esf->esstream,fmode)) == NULL)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_OPEN_FAIL;
		    }
/*else
fprintf(stderr,"esstream %s opened with mode %s\n",esf->esstream,fmode);*/
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
				i,esf->edit[i].time_d,esf->edit[i].beam,
				esf->edit[i].action,esf->edit[i].use);
		fprintf(stderr,"dbg2       esf->esffile:  %s\n",esf->esffile);
		fprintf(stderr,"dbg2       esf->esstream: %s\n",esf->esstream);
		fprintf(stderr,"dbg2       esf->esffp:    %d\n",esf->esffp);
		fprintf(stderr,"dbg2       esf->essfp:    %d\n",esf->essfp);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
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
				i,esf->edit[i].time_d,esf->edit[i].beam,
				esf->edit[i].action,esf->edit[i].use);
		fprintf(stderr,"dbg2       time_d:      %f\n",time_d);
		fprintf(stderr,"dbg2       nbath:       %d\n",nbath);
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2       beamflag:    %d %d\n",i,beamflag[i]);
		}

	/* find first and last edits for this ping */
	firstedit = 0;
	lastedit = firstedit - 1;
	for (j = firstedit; j < esf->nedit && time_d >= (esf->edit[j].time_d - MB_ESF_MAXTIMEDIFF); j++)
		{
		if (fabs(esf->edit[j].time_d - time_d) < MB_ESF_MAXTIMEDIFF)
		    {
		    if (lastedit < firstedit)
			firstedit = j;
		    lastedit = j;
		    }
		}
/*fprintf(stderr,"firstedit:%d lastedit:%d\n",firstedit,lastedit);*/
			
	/* apply edits */
	if (lastedit >= firstedit)
		{
		/* check for edits with bad beam numbers */
		for (j=firstedit;j<=lastedit;j++)
		    {
		    if (esf->edit[j].beam < 0 
		    	|| esf->edit[j].beam >= nbath)
		    	esf->edit[j].use += 10000;
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
			   esf file - the overridden edit events
			   may already be indicated by a use value
			   of 100 or more. */
			if (esf->edit[j].beam == i
			    && esf->edit[j].use < 100)
			    {
			    /* apply edit */
			    if (esf->edit[j].action == MBP_EDIT_FLAG
				&& beamflag[i] != MB_FLAG_NULL)
				{
/*fprintf(stderr,"edit:%d beam:%d MBP_EDIT_FLAG  flag:%d ",j,i,beamflag[i]);*/
				beamflag[i] 
				    = MB_FLAG_FLAG + MB_FLAG_MANUAL;
				esf->edit[j].use++;
				apply = MB_YES;
				action = esf->edit[j].action;
/*fprintf(stderr," %d\n",beamflag[i]);*/
				}
			    else if (esf->edit[j].action == MBP_EDIT_FILTER
				&& beamflag[i] != MB_FLAG_NULL)
				{
/*fprintf(stderr,"edit:%d beam:%d MBP_EDIT_FILTER\n",j,i);*/
				beamflag[i] 
				    = MB_FLAG_FLAG + MB_FLAG_FILTER;
				esf->edit[j].use++;
				apply = MB_YES;
				action = esf->edit[j].action;
				}
			    else if (esf->edit[j].action == MBP_EDIT_UNFLAG
				&& beamflag[i] != MB_FLAG_NULL)
				{
/*fprintf(stderr,"edit:%d beam:%d MBP_EDIT_UNFLAG\n",j,i);*/
				beamflag[i] = MB_FLAG_NONE;
				esf->edit[j].use++;
				apply = MB_YES;
				action = esf->edit[j].action;
				}
			    else if (esf->edit[j].action == MBP_EDIT_ZERO)
				{
/*fprintf(stderr,"edit:%d beam:%d MBP_EDIT_ZERO\n",j,i);*/
				beamflag[i] = MB_FLAG_NULL;
				esf->edit[j].use++;
				apply = MB_YES;
				action = esf->edit[j].action;
				}
			    else
				{
/*fprintf(stderr,"edit:%d beam:%d NOT USED\n",j,i);*/
				esf->edit[j].use += 1000;
/*fprintf(stderr,"Dup Edit[%d]?: ping:%f beam:%d flag:%d action:%d\n",
j, time_d, i, beamflag[i], esf->edit[j].action);*/
				}
			    }
			}
		    if (apply == MB_YES 
		    	&& esf->essfp != NULL
			&& beamflag[i] != beamflagorg)
		    	mb_ess_save(verbose, esf, time_d, i, action, error);
		    }
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       time_d:      %f\n",time_d);
		fprintf(stderr,"dbg2       nbath:       %d\n",nbath);
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2       beamflag:    %d %d\n",i,beamflag[i]);
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
		fprintf(stderr,"dbg2       esf->edit:        %d\n",esf->edit);
		fprintf(stderr,"dbg2       esf->esffp:       %d\n",esf->esffp);
		fprintf(stderr,"dbg2       time_d:           %f\n",time_d);
		fprintf(stderr,"dbg2       beam:             %d\n",beam);
		fprintf(stderr,"dbg2       action:           %d\n",action);
		}

	/* write out the edit */
	if (esf->esffp != NULL)
	    {		
/*fprintf(stderr,"OUTPUT EDIT: %f %d %d\n",time_d,beam,action);*/
	    if (esf->byteswapped == MB_YES)
	    	{
	   	mb_swap_double(&time_d);
	    	beam = mb_swap_int(beam);
	    	action = mb_swap_int(action);
		}
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
		fprintf(stderr,"dbg2       esf->edit:        %d\n",esf->edit);
		fprintf(stderr,"dbg2       esf->esffp:       %d\n",esf->esffp);
		fprintf(stderr,"dbg2       error:            %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:           %d\n",status);
		}

	/* return success */
	return(status);
}


/*--------------------------------------------------------------------*/
/* 	function mb_ess_save saves one edit event to an edit save stream file. */
int mb_ess_save(int verbose, struct mb_esf_struct *esf, 
		double time_d, int beam, int action, int *error)
{
  	char	*function_name = "mb_ess_save";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       esf->nedit:       %d\n",esf->nedit);
		fprintf(stderr,"dbg2       esf->edit:        %d\n",esf->edit);
		fprintf(stderr,"dbg2       esf->essfp:       %d\n",esf->essfp);
		fprintf(stderr,"dbg2       time_d:           %f\n",time_d);
		fprintf(stderr,"dbg2       beam:             %d\n",beam);
		fprintf(stderr,"dbg2       action:           %d\n",action);
		}

	/* write out the edit */
	if (esf->essfp != NULL)
	    {		
/*fprintf(stderr,"OUTPUT EDIT: %f %d %d\n",time_d,beam,action);*/
	    if (esf->byteswapped == MB_YES)
	    	{
	        mb_swap_double(&time_d);
	        beam = mb_swap_int(beam);
	        action = mb_swap_int(action);
		}
	    if (fwrite(&time_d, sizeof(double), 1, esf->essfp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    if (status == MB_SUCCESS
		&& fwrite(&beam, sizeof(int), 1, esf->essfp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    if (status == MB_SUCCESS
		&& fwrite(&action, sizeof(int), 1, esf->essfp) != 1)
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
		fprintf(stderr,"dbg2       esf->edit:        %d\n",esf->edit);
		fprintf(stderr,"dbg2       esf->essfp:       %d\n",esf->essfp);
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
		fprintf(stderr,"dbg2       esf->edit:        %d\n",esf->edit);
		fprintf(stderr,"dbg2       esf->esffp:       %d\n",esf->esffp);
		}

	/* deallocate the arrays */
	if (esf->nedit != 0)
		{
		if (esf->edit != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__,(void **)&(esf->edit), error);
		}
	esf->nedit = 0;

	/* close the esf file */
	if (esf->esffp != NULL)
		{
		fclose(esf->esffp);
		esf->esffp = NULL;
		}

	/* close the esf stream file */
	if (esf->essfp != NULL)
		{
		fclose(esf->essfp);
		esf->essfp = NULL;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       esf->nedit:       %d\n",esf->nedit);
		fprintf(stderr,"dbg2       esf->edit:        %d\n",esf->edit);
		fprintf(stderr,"dbg2       esf->esffp:       %d\n",esf->esffp);
		fprintf(stderr,"dbg2       esf->essfp:       %d\n",esf->essfp);
		fprintf(stderr,"dbg2       error:            %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:           %d\n",status);
		}

	/* return success */
	return(status);
}

/*--------------------------------------------------------------------*/
/* The following code has been modified from code obtained from
	http://www.gnu-darwin.org/sources/4Darwin-x86/src/lib/libc/stdlib/merge.c
   on July 27, 2003 by David W. Caress.
 */

/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Peter McIlroy.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


/*
 * Hybrid exponential search/linear search merge sort with hybrid
 * natural/pairwise first pass.  Requires about .3% more comparisons
 * for random data than LSMS with pairwise first pass alone.
 * It works for objects as small as two bytes.
 */

#define NATURAL
#define THRESHOLD 16	/* Best choice for natural merge cut-off. */

/* #define NATURAL to get hybrid natural merge.
 * (The default is pairwise merging.)
 */

#define ISIZE sizeof(int)
#define PSIZE sizeof(u_char *)
#define ICOPY_LIST(src, dst, last)				\
	do							\
	*(int*)dst = *(int*)src, src += ISIZE, dst += ISIZE;	\
	while(src < last)
#define ICOPY_ELT(src, dst, i)					\
	do							\
	*(int*) dst = *(int*) src, src += ISIZE, dst += ISIZE;	\
	while (i -= ISIZE)

#define CCOPY_LIST(src, dst, last)		\
	do					\
		*dst++ = *src++;		\
	while (src < last)
#define CCOPY_ELT(src, dst, i)			\
	do					\
		*dst++ = *src++;		\
	while (i -= 1)
		
/*
 * Find the next possible pointer head.  (Trickery for forcing an array
 * to do double duty as a linked list when objects do not align with word
 * boundaries.
 */
/* Assumption: PSIZE is a power of 2. */
#define EVAL(p) (u_char **)						\
	((u_char *)0 +							\
	    (((u_char *)p + PSIZE - 1 - (u_char *) 0) & ~(PSIZE - 1)))

/*
 * Arguments are as for qsort.
 */
int mb_mergesort(void *base, size_t nmemb,register size_t size, 
	int (*cmp) (const void *, const void *))
{
	register int i, sense;
	int big, iflag;
	register u_char *f1, *f2, *t, *b, *tp2, *q, *l1, *l2;
	u_char *list2, *list1, *p2, *p, *last, **p1;

	if (size < PSIZE / 2) {		/* Pointers must fit into 2 * size. */
		/*errno = EINVAL;*/
		return (-1);
	}

	/*
	 * XXX
	 * Stupid subtraction for the Cray.
	 */
	iflag = 0;
	if (!(size % ISIZE) && !(((char *)base - (char *)0) % ISIZE))
		iflag = 1;

	if ((list2 = (u_char *) malloc(nmemb * size + PSIZE)) == NULL)
		return (-1);

	list1 = base;
	mb_mergesort_setup(list1, list2, nmemb, size, cmp);
	last = list2 + nmemb * size;
	i = big = 0;
	while (*EVAL(list2) != last) {
	    l2 = list1;
	    p1 = EVAL(list1);
	    for (tp2 = p2 = list2; p2 != last; p1 = EVAL(l2)) {
	    	p2 = *EVAL(p2);
	    	f1 = l2;
	    	f2 = l1 = list1 + (p2 - list2);
	    	if (p2 != last)
	    		p2 = *EVAL(p2);
	    	l2 = list1 + (p2 - list2);
	    	while (f1 < l1 && f2 < l2) {
	    		if ((*cmp)(f1, f2) <= 0) {
	    			q = f2;
	    			b = f1, t = l1;
	    			sense = -1;
	    		} else {
	    			q = f1;
	    			b = f2, t = l2;
	    			sense = 0;
	    		}
	    		if (!big) {	/* here i = 0 */
LINEAR:	    			while ((b += size) < t && cmp(q, b) >sense)
	    				if (++i == 6) {
	    					big = 1;
	    					goto EXPONENTIAL;
	    				}
	    		} else {
EXPONENTIAL:	    		for (i = size; ; i <<= 1)
	    				if ((p = (b + i)) >= t) {
	    					if ((p = t - size) > b &&
						    (*cmp)(q, p) <= sense)
	    						t = p;
	    					else
	    						b = p;
	    					break;
	    				} else if ((*cmp)(q, p) <= sense) {
	    					t = p;
	    					if (i == size)
	    						big = 0; 
	    					goto FASTCASE;
	    				} else
	    					b = p;
SLOWCASE:	    		while (t > b+size) {
	    				i = (((t - b) / size) >> 1) * size;
	    				if ((*cmp)(q, p = b + i) <= sense)
	    					t = p;
	    				else
	    					b = p;
	    			}
	    			goto COPY;
FASTCASE:	    		while (i > size)
	    				if ((*cmp)(q,
	    					p = b + (i >>= 1)) <= sense)
	    					t = p;
	    				else
	    					b = p;
COPY:	    			b = t;
	    		}
	    		i = size;
	    		if (q == f1) {
	    			if (iflag) {
	    				ICOPY_LIST(f2, tp2, b);
	    				ICOPY_ELT(f1, tp2, i);
	    			} else {
	    				CCOPY_LIST(f2, tp2, b);
	    				CCOPY_ELT(f1, tp2, i);
	    			}
	    		} else {
	    			if (iflag) {
	    				ICOPY_LIST(f1, tp2, b);
	    				ICOPY_ELT(f2, tp2, i);
	    			} else {
	    				CCOPY_LIST(f1, tp2, b);
	    				CCOPY_ELT(f2, tp2, i);
	    			}
	    		}
	    	}
	    	if (f2 < l2) {
	    		if (iflag)
	    			ICOPY_LIST(f2, tp2, l2);
	    		else
	    			CCOPY_LIST(f2, tp2, l2);
	    	} else if (f1 < l1) {
	    		if (iflag)
	    			ICOPY_LIST(f1, tp2, l1);
	    		else
	    			CCOPY_LIST(f1, tp2, l1);
	    	}
	    	*p1 = l2;
	    }
	    tp2 = list1;	/* swap list1, list2 */
	    list1 = list2;
	    list2 = tp2;
	    last = list2 + nmemb*size;
	}
	if (base == list2) {
		memmove(list2, list1, nmemb*size);
		list2 = list1;
	}
	free(list2);
	return (0);
}

#define	swap(a, b) {					\
		s = b;					\
		i = size;				\
		do {					\
			tmp = *a; *a++ = *s; *s++ = tmp; \
		} while (--i);				\
		a -= size;				\
	}
#define reverse(bot, top) {				\
	s = top;					\
	do {						\
		i = size;				\
		do {					\
			tmp = *bot; *bot++ = *s; *s++ = tmp; \
		} while (--i);				\
		s -= size2;				\
	} while(bot < s);				\
}

/*
 * Optional hybrid natural/pairwise first pass.  Eats up list1 in runs of
 * increasing order, list2 in a corresponding linked list.  Checks for runs
 * when THRESHOLD/2 pairs compare with same sense.  (Only used when NATURAL
 * is defined.  Otherwise simple pairwise merging is used.)
 */
mb_mergesort_setup(u_char *list1, u_char *list2, size_t n, size_t size, 
	int (*cmp) (const void *, const void *))
{
	int i, length, size2, tmp, sense;
	u_char *f1, *f2, *s, *l2, *last, *p2;

	size2 = size*2;
	if (n <= 5) {
		mb_mergesort_insertionsort(list1, n, size, cmp);
		*EVAL(list2) = (u_char*) list2 + n*size;
		return;
	}
	/*
	 * Avoid running pointers out of bounds; limit n to evens
	 * for simplicity.
	 */
	i = 4 + (n & 1);
	mb_mergesort_insertionsort(list1 + (n - i) * size, i, size, cmp);
	last = list1 + size * (n - i);
	*EVAL(list2 + (last - list1)) = list2 + n * size;

#ifdef NATURAL
	p2 = list2;
	f1 = list1;
	sense = (cmp(f1, f1 + size) > 0);
	for (; f1 < last; sense = !sense) {
		length = 2;
					/* Find pairs with same sense. */
		for (f2 = f1 + size2; f2 < last; f2 += size2) {
			if ((cmp(f2, f2+ size) > 0) != sense)
				break;
			length += 2;
		}
		if (length < THRESHOLD) {		/* Pairwise merge */
			do {
				p2 = *EVAL(p2) = f1 + size2 - list1 + list2;
				if (sense > 0)
					swap (f1, f1 + size);
			} while ((f1 += size2) < f2);
		} else {				/* Natural merge */
			l2 = f2;
			for (f2 = f1 + size2; f2 < l2; f2 += size2) {
				if ((cmp(f2-size, f2) > 0) != sense) {
					p2 = *EVAL(p2) = f2 - list1 + list2;
					if (sense > 0)
						reverse(f1, f2-size);
					f1 = f2;
				}
			}
			if (sense > 0)
				reverse (f1, f2-size);
			f1 = f2;
			if (f2 < last || cmp(f2 - size, f2) > 0)
				p2 = *EVAL(p2) = f2 - list1 + list2;
			else
				p2 = *EVAL(p2) = list2 + n*size;
		}
	}
#else		/* pairwise merge only. */
	for (f1 = list1, p2 = list2; f1 < last; f1 += size2) {
		p2 = *EVAL(p2) = p2 + size2;
		if (cmp (f1, f1 + size) > 0)
			swap(f1, f1 + size);
	}
#endif /* NATURAL */
}

/*
 * This is to avoid out-of-bounds addresses in sorting the
 * last 4 elements.
 */
mb_mergesort_insertionsort(u_char *a, size_t n, size_t size, 
	int (*cmp)(const void *, const void *))
{
	u_char *ai, *s, *t, *u, tmp;
	int i;

	for (ai = a+size; --n >= 1; ai += size)
		for (t = ai; t > a; t -= size) {
			u = t - size;
			if (cmp(u, t) <= 0)
				break;
			swap(u, t);
		}
}

