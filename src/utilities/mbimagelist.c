/*--------------------------------------------------------------------
 *    The MB-system:	mbimagelist.c	10/10/2001
 *
 *    Copyright (c) 2001-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA, USA
 *    Dale N. Chayes (dale@ccom.unh.edu)
 *      CCOM, University of New Hampshire
 *      Palisades, NY, USA
 *    Christian dos Santos Ferreira (cferreira@marum.de)
 *      MARUM Center of Marine Environmental Science
 *      of Universitat Bremen
 *      Bremen, Germany
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBdatalist parses recursive datalist files and outputs the
 * complete list of data files and formats.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	October 10, 2001
 *
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

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"
#include "mb_process.h"


/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	char program_name[] = "mbdatalist";
	char help_message[] = "mbdatalist parses recursive datalist files and outputs the\ncomplete list of data files and formats. "
	                      "\nThe results are dumped to stdout.";
	char usage_message[] = "mbdatalist [-C -D -Fformat -Ifile -N -O -P -Q -Rw/e/s/n -S -U -Y -Z -V -H]";
	extern char *optarg;
	int option_index;
	int errflg = 0;
	int c;
	int help = 0;
	int flag = 0;
