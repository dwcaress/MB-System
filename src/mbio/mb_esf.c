/*--------------------------------------------------------------------
 *    The MB-system:	mb_esf.c	4/10/2003
 *
 *    Copyright (c) 2003-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_esf.c includes the functions used to read, write, and use
 * edit save files.
 *
 * Author:	D. W. Caress
 * Date:	April 10, 2003
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"

void mb_mergesort_setup(mb_u_char *list1, mb_u_char *list2, size_t n, size_t size, int (*cmp)(const void *, const void *));
void mb_mergesort_insertionsort(mb_u_char *a, size_t n, size_t size, int (*cmp)(const void *, const void *));

/*--------------------------------------------------------------------*/
/* 	function mb_esf_check checks for an existing esf file. */
int mb_esf_check(int verbose, char *swathfile, char *esffile, int *found, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
		fprintf(stderr, "dbg2       swathfile:   %s\n", swathfile);
	}

	/* check if edit save file is set in mbprocess parameter file
	    or just lying around */
	int mbp_edit_mode;
	char mbp_editfile[MB_PATH_MAXLINE];
	int status = mb_pr_get_edit(verbose, swathfile, &mbp_edit_mode, mbp_editfile, error);
	if (mbp_edit_mode == MBP_EDIT_ON) {
		*found = true;
		strcpy(esffile, mbp_editfile);
	}
	else {
		*found = false;
		sprintf(esffile, "%s.esf", swathfile);
	}

	/* assume success */
	status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       esfile:      %s\n", esffile);
		fprintf(stderr, "dbg2       found:       %d\n", *found);
		fprintf(stderr, "dbg2       error:       %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	/* return success */
	return (status);
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
        If both load and output are false, nothing will be
        done. */
int mb_esf_load(int verbose, const char *program_name, char *swathfile, bool load, int output, char *esffile, struct mb_esf_struct *esf,
                int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
		fprintf(stderr, "dbg2       program_name:  %s\n", program_name);
		fprintf(stderr, "dbg2       swathfile:     %s\n", swathfile);
		fprintf(stderr, "dbg2       load:          %d\n", load);
		fprintf(stderr, "dbg2       output:        %d\n", output);
	}

	/* initialize the esf structure */
	esf->esffile[0] = '\0';
	esf->esstream[0] = '\0';
	esf->byteswapped = mb_swap_check();
	esf->version = 3;
	esf->mode = MB_ESF_MODE_EXPLICIT;
	esf->nedit = 0;
	esf->edit = NULL;
	esf->esffp = NULL;
	esf->essfp = NULL;
	esf->startnextsearch = 0;

	/* get name of existing or new esffile, then load old edits
	    and/or open new esf file */
	int found;  // TODO(schwehr): Make mb_esf_check take a bool.
	int status = mb_esf_check(verbose, swathfile, esffile, &found, error);
	if ((load && found) || output != MBP_ESF_NOWRITE) {
		status = mb_esf_open(verbose, program_name, esffile, load, output, esf, error);
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_NO_DATA_LOADED;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       esfile:      %s\n", esffile);
		fprintf(stderr, "dbg2       nedit:       %d\n", esf->nedit);
		for (int i = 0; i < esf->nedit; i++)
			fprintf(stderr, "dbg2       edit event:  %d %.6f %5d %3d %3d\n", i, esf->edit[i].time_d, esf->edit[i].beam,
			        esf->edit[i].action, esf->edit[i].use);
		fprintf(stderr, "dbg2       esf->esffp:  %p\n", (void *)esf->esffp);
		fprintf(stderr, "dbg2       error:       %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	/* return success */
	return (status);
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
int mb_esf_open(int verbose, const char *program_name, char *esffile, bool load, int output, struct mb_esf_struct *esf, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
		fprintf(stderr, "dbg2       program_name:  %s\n", program_name);
		fprintf(stderr, "dbg2       esffile:       %s\n", esffile);
		fprintf(stderr, "dbg2       load:          %d\n", load);
		fprintf(stderr, "dbg2       output:        %d\n", output);
		fprintf(stderr, "dbg2       esf:           %p\n", (void *)esf);
		fprintf(stderr, "dbg2       error:         %p\n", (void *)error);
	}

	int status = MB_SUCCESS;
	char command[MB_PATH_MAXLINE];
	FILE *esffp;
	struct stat file_status;
	int fstat;
	char fmode[16];
	mb_path esf_header;

	/* initialize the esf structure */
	strcpy(esf->esffile, esffile);
	sprintf(esf->esstream, "%s.stream", esffile);
	esf->byteswapped = mb_swap_check();
	esf->version = 3;
	esf->mode = MB_ESF_MODE_EXPLICIT;
	esf->nedit = 0;
	esf->edit = NULL;
	esf->esffp = NULL;
	esf->essfp = NULL;
	esf->startnextsearch = 0;

	/* load edits from existing esf file if requested */
	if (load) {
		/* check that esf file exists */
		fstat = stat(esffile, &file_status);
		if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
			/* save filename in structure */
			strcpy(esf->esffile, esffile);

			/* get number of old edits */
			esf->nedit = file_status.st_size / (sizeof(double) + 2 * sizeof(int));

			/* allocate arrays for old edits */
			if (esf->nedit > 0) {
				status = mb_mallocd(verbose, __FILE__, __LINE__, esf->nedit * sizeof(struct mb_edit_struct),
				                    (void **)&(esf->edit), error);
				if (status == MB_SUCCESS)
					memset(esf->edit, 0, esf->nedit * sizeof(struct mb_edit_struct));

				/* if error initializing memory then quit */
				if (status != MB_SUCCESS) {
					*error = MB_ERROR_MEMORY_FAIL;
					fprintf(stderr, "\nUnable to allocate memory for %d edit events\n", esf->nedit);
					esf->nedit = 0;
					return (status);
				}
			}

			/* open and read the old edit file */
			strcpy(fmode, "rb");
			if (status == MB_SUCCESS && esf->nedit > 0 && (esffp = fopen(esffile, fmode)) == NULL) {
				fprintf(stderr, "\nnedit:%d\n", esf->nedit);
				esf->nedit = 0;
				*error = MB_ERROR_OPEN_FAIL;
				fprintf(stderr, "\nUnable to open edit save file %s\n", esffile);
			}
			else if (status == MB_SUCCESS && esf->nedit > 0) {
				/* reset message */
				if (verbose > 0)
					fprintf(stderr, "Reading %d old edits...\n", esf->nedit);

				/* read file header to discern the format */
				if (fread(esf_header, MB_PATH_MAXLINE, 1, esffp) == 1) {
					if (strncmp(esf_header, "ESFVERSION03", 12) == 0) {
						esf->version = 3;
						esf->nedit -= MB_PATH_MAXLINE / (sizeof(double) + 2 * sizeof(int));
						sscanf(&esf_header[13], "ESF Mode: %d", &esf->mode);
					}
					else if (strncmp(esf_header, "ESFVERSION02", 12) == 0) {
						esf->version = 2;
						esf->nedit -= MB_PATH_MAXLINE / (sizeof(double) + 2 * sizeof(int));
						esf->mode = MB_ESF_MODE_EXPLICIT;
					}
					else {
						rewind(esffp);
						esf->version = 1;
						esf->mode = MB_ESF_MODE_EXPLICIT;
					}
				}
				else {
					rewind(esffp);
					esf->version = 1;
					esf->mode = MB_ESF_MODE_EXPLICIT;
				}

				*error = MB_ERROR_NO_ERROR;
				int nedit = 0;
				while (nedit < esf->nedit && *error == MB_ERROR_NO_ERROR) {
					if (fread(&(esf->edit[nedit].time_d), sizeof(double), 1, esffp) != 1 ||
					    fread(&(esf->edit[nedit].beam), sizeof(int), 1, esffp) != 1 ||
					    fread(&(esf->edit[nedit].action), sizeof(int), 1, esffp) != 1) {
						status = MB_FAILURE;
						*error = MB_ERROR_EOF;
					}
					else if (esf->byteswapped) {
						mb_swap_double(&(esf->edit[nedit].time_d));
						esf->edit[nedit].beam = mb_swap_int(esf->edit[nedit].beam);
						esf->edit[nedit].action = mb_swap_int(esf->edit[nedit].action);
					}
					if (*error == MB_ERROR_NO_ERROR && esf->edit[nedit].time_d < 4.29497e9) {
						nedit++;
					}
					else {
						if (fread(esf_header, MB_PATH_MAXLINE - (sizeof(double) + 2 * sizeof(int)), 1, esffp) != 1) {
              status = MB_FAILURE;
              *error = MB_ERROR_EOF;
            }
					}
				}
				esf->nedit = nedit;
				if (*error == MB_ERROR_EOF) {
					status = MB_SUCCESS;
					*error = MB_ERROR_NO_ERROR;
				}

				/* close the file */
				fclose(esffp);

				/* reset message */
				if (verbose > 0)
					fprintf(stderr, "Sorting %d old edits...\n", esf->nedit);

				/* first round all timestamps to the nearest 0.1 millisecond to avoid
				    comparison errors during sorting */
				/* for (i=0;i<esf->nedit;i++)
				    {
				    esf->edit[i].time_d = 0.0001 * floor(10000.0 * esf->edit[i].time_d + 0.5);
				    } */

				/* now sort the edits */
				if (esf->nedit > 1) {
					if (esf->version > 1)
						mb_mergesort((char *)esf->edit, esf->nedit, sizeof(struct mb_edit_struct), mb_edit_compare);
					else
						mb_mergesort((char *)esf->edit, esf->nedit, sizeof(struct mb_edit_struct), mb_edit_compare_coarse);
				}
				/* for (i=0;i<esf->nedit;i++)
				fprintf(stderr,"EDITS SORTED: i:%d edit: %f %d %d  use:%d\n",
				i,esf->edit[i].time_d,esf->edit[i].beam,
				esf->edit[i].action,esf->edit[i].use); */
			}
		}
	}

	if (status == MB_SUCCESS && output != MBP_ESF_NOWRITE) {
		/* check if esf file exists */
		bool header = true;
		fstat = stat(esffile, &file_status);
		if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
			/* copy old edit save file to tmp file */
			if (load) {
				sprintf(command, "cp %s %s.tmp\n", esffile, esffile);
				if (system(command) != 0) {
    			status = MB_FAILURE;
    			*error = MB_ERROR_OPEN_FAIL;
    			fprintf(stderr,"Failed to copy existing esffile %s\n",esf->esffile);
        }
			}
			if (output == MBP_ESF_APPEND)
				header = false;
		}

		/* open the edit save file */
		if (output == MBP_ESF_WRITE) {
			strcpy(fmode, "wb");
		}
		else if (output == MBP_ESF_APPEND) {
			strcpy(fmode, "ab");
		}
		if ((esf->esffp = fopen(esf->esffile, fmode)) == NULL) {
			status = MB_FAILURE;
			*error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"Failed to open esffile %s with file mode %s\n", esf->esffile, fmode);
		}
		//else
		//	fprintf(stderr,"esffile %s opened with file mode %s\n", esf->esffile, fmode);

		/* open the edit save stream file */
		if (status == MB_SUCCESS) {
			if ((esf->essfp = fopen(esf->esstream, fmode)) == NULL) {
				status = MB_FAILURE;
				*error = MB_ERROR_OPEN_FAIL;
				fprintf(stderr,"Failed to open esstream %s with file mode %s\n", esf->esstream, fmode);
			}
			//else
			//	fprintf(stderr,"esstream %s opened with file mode %s\n", esf->esstream, fmode);
		}

		/* if writing a new esf file then put version header at beginning */
		if (status == MB_SUCCESS && header) {
      char user[256], host[256], date[32];
      status = mb_user_host_date(verbose, user, host, date, error);
			memset(esf_header, 0, MB_PATH_MAXLINE);
			sprintf(esf_header,
			        "ESFVERSION03\nESF Mode: %d\nMB-System Version %s\nProgram: %s\nUser: %s\nCPU: %s\nDate: %s\n",
			        esf->mode, MB_VERSION, program_name, user, host, date);
			if (fwrite(esf_header, MB_PATH_MAXLINE, 1, esf->esffp) != 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
			}
			else if (fwrite(esf_header, MB_PATH_MAXLINE, 1, esf->essfp) != 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
			}
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       nedit:       %d\n", esf->nedit);
		fprintf(stderr, "dbg2       mode:        %d\n", esf->mode);
		for (int i = 0; i < esf->nedit; i++)
			fprintf(stderr, "dbg2       edit event:  %d %.6f %5d %3d %3d\n", i, esf->edit[i].time_d, esf->edit[i].beam,
			        esf->edit[i].action, esf->edit[i].use);
		fprintf(stderr, "dbg2       esf->esffile:          %s\n", esf->esffile);
		fprintf(stderr, "dbg2       esf->esstream:         %s\n", esf->esstream);
		fprintf(stderr, "dbg2       esf->esffp:            %p\n", (void *)esf->esffp);
		fprintf(stderr, "dbg2       esf->essfp:            %p\n", (void *)esf->essfp);
		fprintf(stderr, "dbg2       esf->byteswapped:      %d\n", esf->byteswapped);
		fprintf(stderr, "dbg2       esf->version:          %d\n", esf->version);
		fprintf(stderr, "dbg2       esf->startnextsearch:  %d\n", esf->startnextsearch);
		fprintf(stderr, "dbg2       error:                 %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_esf_fixtimestamps fixes timestamps of all edits
        in esf that are within tolerance of time_d - those timestamps
        are set to time_d so that the edits correspond to this ping.
        This function is used to rectify edit timestamps when edits are
        being extracted from one version of a dataset and applied to
        another. */
int mb_esf_fixtimestamps(int verbose, struct mb_esf_struct *esf, double time_d, double tolerance, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       nedit:            %d\n", esf->nedit);
		for (int i = 0; i < esf->nedit; i++)
			fprintf(stderr, "dbg2       edit event: %d %.6f %5d %3d %3d\n", i, esf->edit[i].time_d, esf->edit[i].beam,
			        esf->edit[i].action, esf->edit[i].use);
		fprintf(stderr, "dbg2       time_d:           %f\n", time_d);
		fprintf(stderr, "dbg2       tolerance:        %f\n", tolerance);
	}

	/* all edits that have timestamps within tolerance of time_d will have
	their timestamps set to time_d */
	for (int j = 0; j < esf->nedit; j++) {
		if (fabs(esf->edit[j].time_d - time_d) < tolerance) {
			esf->edit[j].time_d = time_d;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		for (int i = 0; i < esf->nedit; i++)
			fprintf(stderr, "dbg2       edit event: %d %.6f %5d %3d %3d\n", i, esf->edit[i].time_d, esf->edit[i].beam,
			        esf->edit[i].action, esf->edit[i].use);
		fprintf(stderr, "dbg2       error:  %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_esf_apply applies saved edits to the beamflags
    in a ping. If an output esf file is open the applied edits
    are saved to that file.  */
int mb_esf_apply(int verbose, struct mb_esf_struct *esf, double time_d, int pingmultiplicity, int nbath, char *beamflag,
                 int *error) {
	int firstedit, lastedit;
	int action;
	int beamoffset, beamoffsetmax;
	char beamflagorg;
	double maxtimediff;
	int ibeam;
	int j;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       esf:              %p\n", esf);
		fprintf(stderr, "dbg2       nedit:            %d\n", esf->nedit);
		fprintf(stderr, "dbg2       mode:             %d\n", esf->mode);
		for (int i = 0; i < esf->nedit; i++)
			fprintf(stderr, "dbg2       edit event: %d %.6f %5d %3d %3d\n", i, esf->edit[i].time_d, esf->edit[i].beam,
			        esf->edit[i].action, esf->edit[i].use);
		fprintf(stderr, "dbg2       time_d:           %f\n", time_d);
		fprintf(stderr, "dbg2       pingmultiplicity: %d\n", pingmultiplicity);
		fprintf(stderr, "dbg2       nbath:            %d\n", nbath);
		for (int i = 0; i < nbath; i++)
			fprintf(stderr, "dbg2       beamflag:    %d %d\n", i, beamflag[i]);
	}

	/* if ping has the same time stamp as previous pings, pingmultiplicity will be
	    > 0 and the edit beam values will be augmented by
	    MB_ESF_MULTIPLICITY_FACTOR * pingmultiplicity */
	beamoffset = MB_ESF_MULTIPLICITY_FACTOR * pingmultiplicity;
	beamoffsetmax = beamoffset + MB_ESF_MULTIPLICITY_FACTOR;

	/* if the esf file version is old then use a larger
	 * criteria to match timestamps because some esf timestamps were truncated
	 * to a 1 msec granularity */
	if (esf->version == 1)
		maxtimediff = MB_ESF_MAXTIMEDIFF_X10;
	else
		maxtimediff = MB_ESF_MAXTIMEDIFF;

	/* find first and last edits for this ping - take ping multiplicity into account */
	if (esf->nedit > 0 && esf->startnextsearch > 0
		&& time_d < (esf->edit[esf->startnextsearch].time_d - maxtimediff)
			&& time_d < (esf->edit[esf->startnextsearch - 1].time_d - maxtimediff))
		firstedit = 0;
	else if (esf->nedit > 0 && esf->startnextsearch > 0
		&& fabs(time_d - esf->edit[esf->startnextsearch - 1].time_d) <= maxtimediff
		&& (esf->edit[esf->startnextsearch - 1].beam < beamoffset
				|| esf->edit[esf->startnextsearch-1].beam > beamoffsetmax))
		firstedit = 0;
	else
		firstedit = esf->startnextsearch;
	lastedit = firstedit - 1;
	for (j = firstedit; j < esf->nedit && time_d >= (esf->edit[j].time_d - maxtimediff); j++) {
		if (fabs(esf->edit[j].time_d - time_d) < maxtimediff && esf->edit[j].beam >= beamoffset &&
		    esf->edit[j].beam < beamoffsetmax) {
			if (lastedit < firstedit)
				firstedit = j;
			lastedit = j;
		}
	}

	/* apply edits */
	if (lastedit >= firstedit) {
		/* check for edits with bad beam numbers */
		for (j = firstedit; j <= lastedit; j++) {
			if ((esf->edit[j].beam % MB_ESF_MULTIPLICITY_FACTOR) >= nbath)
				esf->edit[j].use += 10000;
		}

		bool apply;

		/* loop over all beams */
		for (int i = 0; i < nbath; i++) {
			/* apply beam offset for cases of multiple pings */
			ibeam = i + beamoffset;

			/* loop over all edits for this ping */
			apply = false;
			beamflagorg = beamflag[i];
			for (j = firstedit; j <= lastedit; j++) {
				/* apply the edits for this beam in the
				   order they were created so that the last
				   edit event is applied last - only the
				   last event will be output to a new
				   esf file - the overridden edit events
				   may already be indicated by a use value
				   of 100 or more. */
				if (esf->edit[j].beam == ibeam && esf->edit[j].use < 100) {
					/* some actions only work on non-null beams */
					if (!mb_beam_check_flag_unusable(beamflag[i])) {
						if (esf->edit[j].action == MBP_EDIT_FLAG) {
							//	fprintf(stderr,"beam:%4.4d edit:%d time_d:%.6f MBP_EDIT_FLAG  flag:%d
							//",i,j,esf->edit[j].time_d,beamflag[i]);
							beamflag[i] = mb_beam_set_flag_manual(beamflag[i]);
							esf->edit[j].use++;
							apply = true;
							action = esf->edit[j].action;
							//	fprintf(stderr," %d\n",beamflag[i]);
						}
						else if (esf->edit[j].action == MBP_EDIT_FILTER) {
							//	fprintf(stderr,"beam:%4.4d edit:%d time_d:%.6f MBP_EDIT_FILTER  flag:%d
							//",i,j,esf->edit[j].time_d,beamflag[i]);
							beamflag[i] = mb_beam_set_flag_filter(beamflag[i]);
							esf->edit[j].use++;
							apply = true;
							action = esf->edit[j].action;
							//	fprintf(stderr," %d\n",beamflag[i]);
						}
						else if (esf->edit[j].action == MBP_EDIT_SONAR) {
							//	fprintf(stderr,"beam:%4.4d edit:%d time_d:%.6f MBP_EDIT_SONAR  flag:%d
							//",i,j,esf->edit[j].time_d,beamflag[i]);
							beamflag[i] = mb_beam_set_flag_sonar(beamflag[i]);
							esf->edit[j].use++;
							apply = true;
							action = esf->edit[j].action;
							//	fprintf(stderr," %d\n",beamflag[i]);
						}
						else if (esf->edit[j].action == MBP_EDIT_UNFLAG) {
							//	fprintf(stderr,"beam:%4.4d edit:%d time_d:%.6f MBP_EDIT_UNFLAG  flag:%d
							//",i,j,esf->edit[j].time_d,beamflag[i]);
							beamflag[i] = mb_beam_set_flag_none(beamflag[i]);
							esf->edit[j].use++;
							apply = true;
							action = esf->edit[j].action;
							//	fprintf(stderr," %d\n",beamflag[i]);
						}
						else if (esf->edit[j].action == MBP_EDIT_ZERO) {
							//	fprintf(stderr,"beam:%4.4d edit:%d time_d:%.6f MBP_EDIT_ZERO  flag:%d
							//",i,j,esf->edit[j].time_d,beamflag[i]);
							beamflag[i] = mb_beam_set_flag_null(beamflag[i]);
							esf->edit[j].use++;
							apply = true;
							action = esf->edit[j].action;
							//	fprintf(stderr," %d\n",beamflag[i]);
						}
					}
					else {
						//	fprintf(stderr,"beam:%4.4d edit:%d time_d:%.6f NOT USED  flag:%d
						//\n",i,j,esf->edit[j].time_d,beamflag[i]);
						esf->edit[j].use += 1000;
						//	fprintf(stderr,"Dup Edit[%d]?: ping:%f beam:%d flag:%d action:%d\n",
						//	j, time_d, i, beamflag[i], esf->edit[j].action);
					}
				}
			}

			/* handle implicit default modes:
			 * if the esf file mode is
			 *      MB_ESF_MODE_IMPLICIT_NULL == 1
			 * or
			 *      MB_ESF_MODE_IMPLICIT_GOOD == 2
			 * then the esf file will include events for all beams different from the
			 * implicit value. Such files will only be created by mbgetesf
			 * using the -M4 or -M5 commands. If a beam is not set by an edit event,
			 * set it to the implicit value
			 */
			if (!apply) {
				if (esf->mode == MB_ESF_MODE_IMPLICIT_NULL) {
					beamflag[i] = MB_FLAG_NULL;
				}
				else if (esf->mode == MB_ESF_MODE_IMPLICIT_GOOD) {
					beamflag[i] = MB_FLAG_NONE;
				}
				if (beamflag[i] != beamflagorg)
					apply = true;
			}

			/* output change to stream file */
			if (apply && esf->essfp != NULL && beamflag[i] != beamflagorg)
				mb_ess_save(verbose, esf, time_d, ibeam, action, error);
		}

		/* reset startnextsearch */
		esf->startnextsearch = lastedit + 1;
		if (esf->startnextsearch >= esf->nedit)
			esf->startnextsearch = esf->nedit - 1;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       time_d:           %f\n", time_d);
		fprintf(stderr, "dbg2       pingmultiplicity: %d\n", pingmultiplicity);
		fprintf(stderr, "dbg2       nbath:            %d\n", nbath);
		for (int i = 0; i < nbath; i++)
			fprintf(stderr, "dbg2       beamflag:    %d %d %d\n", i, ibeam, beamflag[i]);
		fprintf(stderr, "dbg2       error:  %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_esf_save saves one edit event to an esf file. */
int mb_esf_save(int verbose, struct mb_esf_struct *esf, double time_d, int beam, int action, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       esf->nedit:       %d\n", esf->nedit);
		fprintf(stderr, "dbg2       esf->edit:        %p\n", (void *)esf->edit);
		fprintf(stderr, "dbg2       esf->esffp:       %p\n", (void *)esf->esffp);
		fprintf(stderr, "dbg2       time_d:           %f\n", time_d);
		fprintf(stderr, "dbg2       beam:             %d\n", beam);
		fprintf(stderr, "dbg2       action:           %d\n", action);
	}

	int status = MB_SUCCESS;

	/* write out the edit */
	if (esf->esffp != NULL) {
		if (esf->byteswapped) {
			mb_swap_double(&time_d);
			beam = mb_swap_int(beam);
			action = mb_swap_int(action);
		}
		if (fwrite(&time_d, sizeof(double), 1, esf->esffp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		if (status == MB_SUCCESS && fwrite(&beam, sizeof(int), 1, esf->esffp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		if (status == MB_SUCCESS && fwrite(&action, sizeof(int), 1, esf->esffp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       esf->nedit:       %d\n", esf->nedit);
		fprintf(stderr, "dbg2       esf->edit:        %p\n", (void *)esf->edit);
		fprintf(stderr, "dbg2       esf->esffp:       %p\n", (void *)esf->esffp);
		fprintf(stderr, "dbg2       error:            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:           %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_ess_save saves one edit event to an edit save stream file. */
int mb_ess_save(int verbose, struct mb_esf_struct *esf, double time_d, int beam, int action, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       esf->nedit:       %d\n", esf->nedit);
		fprintf(stderr, "dbg2       esf->edit:        %p\n", (void *)esf->edit);
		fprintf(stderr, "dbg2       esf->essfp:       %p\n", (void *)esf->essfp);
		fprintf(stderr, "dbg2       time_d:           %f\n", time_d);
		fprintf(stderr, "dbg2       beam:             %d\n", beam);
		fprintf(stderr, "dbg2       action:           %d\n", action);
	}

	int status = MB_SUCCESS;

	/* write out the edit */
	if (esf->essfp != NULL) {
		if (esf->byteswapped) {
			mb_swap_double(&time_d);
			beam = mb_swap_int(beam);
			action = mb_swap_int(action);
		}
		if (fwrite(&time_d, sizeof(double), 1, esf->essfp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		if (status == MB_SUCCESS && fwrite(&beam, sizeof(int), 1, esf->essfp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		if (status == MB_SUCCESS && fwrite(&action, sizeof(int), 1, esf->essfp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       esf->nedit:       %d\n", esf->nedit);
		fprintf(stderr, "dbg2       esf->edit:        %p\n", (void *)esf->edit);
		fprintf(stderr, "dbg2       esf->essfp:       %p\n", (void *)esf->essfp);
		fprintf(stderr, "dbg2       error:            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:           %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_esf_close deallocates memory in the esf structure. */
int mb_esf_close(int verbose, struct mb_esf_struct *esf, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       esf->nedit:       %d\n", esf->nedit);
		fprintf(stderr, "dbg2       esf->edit:        %p\n", (void *)esf->edit);
		fprintf(stderr, "dbg2       esf->esffp:       %p\n", (void *)esf->esffp);
	}

	int status = MB_SUCCESS;

    /* check for edits read in that were never used */
	/* if (verbose > 0) {
      if (esf->edit != NULL && esf->nedit > 0) {
        bool unused_output = false;
        for (int i=0; i<esf->nedit; i++) {
          if (!esf->edit[i].use) {
            if (!unused_output) {
              unused_output = true;
              fprintf(stderr, "\nUnused beam flags in %s:\n", __FUNCTION__);
            }
            fprintf(stderr, "%3d  %14.6f %4d %d %d\n", i, esf->edit[i].time_d, esf->edit[i].beam, esf->edit[i].action, esf->edit[i].use);
          }
        }
      }
	}*/

	/* deallocate the arrays */
	if (esf->edit != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(esf->edit), error);
	esf->nedit = 0;

	/* close the esf file */
	if (esf->esffp != NULL) {
		fclose(esf->esffp);
		esf->esffp = NULL;
	}

	/* close the esf stream file */
	if (esf->essfp != NULL) {
		fclose(esf->essfp);
		esf->essfp = NULL;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       esf->nedit:       %d\n", esf->nedit);
		fprintf(stderr, "dbg2       esf->edit:        %p\n", (void *)esf->edit);
		fprintf(stderr, "dbg2       esf->esffp:       %p\n", (void *)esf->esffp);
		fprintf(stderr, "dbg2       esf->essfp:       %p\n", (void *)esf->essfp);
		fprintf(stderr, "dbg2       error:            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:           %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/
/* The following code has been modified from code obtained from
	http://www.gnu-darwin.org/sources/4Darwin-x86/src/lib/libc/stdlib/merge.c
   on July 27, 2003 by David W. Caress.
   On January 28, 2023 I found that the above url no longer exists, but the
   pre-Darwin version can be found in FreeBSD at:
   https://github.com/freebsd/freebsd-src/blob/master/lib/libc/stdlib/merge.c
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
const int THRESHOLD = 16; /* Best choice for natural merge cut-off. */

/* #define NATURAL to get hybrid natural merge.
 * (The default is pairwise merging.)
 */

#define ISIZE sizeof(int)
#define PSIZE sizeof(mb_u_char *)

#define ICOPY_LIST(src, dst, last) \
	do \
		*(int *)dst = *(int *)src, src += ISIZE, dst += ISIZE; \
	while (src < last)

#define ICOPY_ELT(src, dst, i) \
	do \
		*(int *)dst = *(int *)src, src += ISIZE, dst += ISIZE; \
	while (i -= ISIZE)

#define CCOPY_LIST(src, dst, last) \
	do \
		*dst++ = *src++; \
	while (src < last)
#define CCOPY_ELT(src, dst, i) \
	do \
		*dst++ = *src++; \
	while (i -= 1)

/*
 * Find the next possible pointer head.  (Trickery for forcing an array
 * to do double duty as a linked list when objects do not align with word
 * boundaries.
 */
/* Assumption: PSIZE is a power of 2. */
#define EVAL(p) (mb_u_char **)((mb_u_char *)0 + (((mb_u_char *)p + PSIZE - 1 - (mb_u_char *)0) & ~(PSIZE - 1)))

/*
 * Arguments are as for qsort.
 */
int mb_mergesort(void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *)) {

	if (size < PSIZE / 2) { /* Pointers must fit into 2 * size. */
		/*errno = EINVAL;*/
		return (-1);
	}

	/*
	 * XXX
	 * Stupid subtraction for the Cray.
	 */
	int iflag = 0;
	if (!(size % ISIZE) && !(((char *)base - (char *)0) % ISIZE))
		iflag = 1;

	mb_u_char *list2 = (mb_u_char *)malloc(nmemb * size + PSIZE);
	if (list2 == NULL)
		return (-1);

	int sense;
	mb_u_char *f1, *f2, *t, *b, *tp2, *q;
	mb_u_char *l1;
	mb_u_char *p2, *p, *last;
	mb_u_char *list1 = base;
	mb_mergesort_setup(list1, list2, nmemb, size, cmp);
	last = list2 + nmemb * size;
	unsigned int i = 0;
	int big = 0;
	while (*EVAL(list2) != last) {
		mb_u_char *l2 = list1;
		mb_u_char **p1 = EVAL(list1);
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
				}
				else {
					q = f1;
					b = f2, t = l2;
					sense = 0;
				}
				if (!big) { /* here i = 0 */
					/*LINEAR:*/ while ((b += size) < t && cmp(q, b) > sense)
						if (++i == 6) {
							big = 1;
							goto EXPONENTIAL;
						}
				}
				else {
				EXPONENTIAL:
					for (i = size;; i <<= 1)
						if ((p = (b + i)) >= t) {
							if ((p = t - size) > b && (*cmp)(q, p) <= sense)
								t = p;
							else
								b = p;
							break;
						}
						else if ((*cmp)(q, p) <= sense) {
							t = p;
							if (i == size)
								big = 0;
							goto FASTCASE;
						}
						else
							b = p;
					/*SLOWCASE:*/ while (t > b + size) {
						i = (((t - b) / size) >> 1) * size;
						if ((*cmp)(q, p = b + i) <= sense)
							t = p;
						else
							b = p;
					}
					goto COPY;
				FASTCASE:
					while (i > size)
						if ((*cmp)(q, p = b + (i >>= 1)) <= sense)
							t = p;
						else
							b = p;
				COPY:
					b = t;
				}
				i = size;
				if (q == f1) {
					if (iflag) {
						ICOPY_LIST(f2, tp2, b);
						ICOPY_ELT(f1, tp2, i);
					}
					else {
						CCOPY_LIST(f2, tp2, b);
						CCOPY_ELT(f1, tp2, i);
					}
				}
				else {
					if (iflag) {
						ICOPY_LIST(f1, tp2, b);
						ICOPY_ELT(f2, tp2, i);
					}
					else {
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
			}
			else if (f1 < l1) {
				if (iflag)
					ICOPY_LIST(f1, tp2, l1);
				else
					CCOPY_LIST(f1, tp2, l1);
			}
			*p1 = l2;
		}
		tp2 = list1; /* swap list1, list2 */
		list1 = list2;
		list2 = tp2;
		last = list2 + nmemb * size;
	}
	if (base == list2) {
		memmove(list2, list1, nmemb * size);
		list2 = list1;
	}
	free(list2);
	return (0);
}

#define swap(a, b) \
	{ \
		s = b; \
		i = size; \
		do { \
			tmp = *a; \
			*a++ = *s; \
			*s++ = tmp; \
		} while (--i); \
		a -= size; \
	}
#define reverse(bot, top) \
	{ \
		s = top; \
		do { \
			i = size; \
			do { \
				tmp = *bot; \
				*bot++ = *s; \
				*s++ = tmp; \
			} while (--i); \
			s -= size2; \
		} while (bot < s); \
	}

/*
 * Optional hybrid natural/pairwise first pass.  Eats up list1 in runs of
 * increasing order, list2 in a corresponding linked list.  Checks for runs
 * when THRESHOLD/2 pairs compare with same sense.  (Only used when NATURAL
 * is defined.  Otherwise simple pairwise merging is used.)
 */
void mb_mergesort_setup(mb_u_char *list1, mb_u_char *list2, size_t n, size_t size, int (*cmp)(const void *, const void *)) {
	if (n <= 5) {
		mb_mergesort_insertionsort(list1, n, size, cmp);
		*EVAL(list2) = (mb_u_char *)list2 + n * size;
		return;
	}

	const int size2 = size * 2;

	/*
	 * Avoid running pointers out of bounds; limit n to evens
	 * for simplicity.
	 */
	int i = 4 + (n & 1);
	mb_mergesort_insertionsort(list1 + (n - i) * size, i, size, cmp);
	mb_u_char *last = list1 + size * (n - i);
	*EVAL(list2 + (last - list1)) = list2 + n * size;

	int length, tmp, sense;
	mb_u_char *f1, *f2, *s, *l2, *p2;

#ifdef NATURAL
	p2 = list2;
	f1 = list1;
	sense = (cmp(f1, f1 + size) > 0);
	for (; f1 < last; sense = !sense) {
		length = 2;
		/* Find pairs with same sense. */
		for (f2 = f1 + size2; f2 < last; f2 += size2) {
			if ((cmp(f2, f2 + size) > 0) != sense)
				break;
			length += 2;
		}
		if (length < THRESHOLD) { /* Pairwise merge */
			do {
				p2 = *EVAL(p2) = f1 + size2 - list1 + list2;
				if (sense > 0)
					swap(f1, f1 + size);
			} while ((f1 += size2) < f2);
		}
		else { /* Natural merge */
			l2 = f2;
			for (f2 = f1 + size2; f2 < l2; f2 += size2) {
				if ((cmp(f2 - size, f2) > 0) != sense) {
					p2 = *EVAL(p2) = f2 - list1 + list2;
					if (sense > 0)
						reverse(f1, f2 - size);
					f1 = f2;
				}
			}
			if (sense > 0)
				reverse(f1, f2 - size);
			f1 = f2;
			if (f2 < last || cmp(f2 - size, f2) > 0)
				p2 = *EVAL(p2) = f2 - list1 + list2;
			else
				p2 = *EVAL(p2) = list2 + n * size;
		}
	}
#else  /* pairwise merge only. */
	for (f1 = list1, p2 = list2; f1 < last; f1 += size2) {
		p2 = *EVAL(p2) = p2 + size2;
		if (cmp(f1, f1 + size) > 0)
			swap(f1, f1 + size);
	}
#endif /* NATURAL */
}

/*
 * This is to avoid out-of-bounds addresses in sorting the
 * last 4 elements.
 */
void mb_mergesort_insertionsort(mb_u_char *a, size_t n, size_t size, int (*cmp)(const void *, const void *)) {
	mb_u_char *s, tmp;
	int i;

	for (mb_u_char *ai = a + size; --n >= 1; ai += size)
		for (mb_u_char *t = ai; t > a; t -= size) {
			mb_u_char *u = t - size;
			if (cmp(u, t) <= 0)
				break;
			swap(u, t);
		}
}

/*--------------------------------------------------------------------*/
