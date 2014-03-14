/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_tm.c	3/3/2014
 *	$Id$
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
/* This source code is part of the mbbsio library used to read and write
 * swath sonar data in the bsio format devised and used by the
 * Hawaii Mapping Research Group of the University of Hawaii.
 * This source code was made available by Roger Davis of the
 * University of Hawaii under the GPL. Minor modifications have
 * been made to the version distributed here as part of MB-System.
 *
 * Author:	Roger Davis (primary author)
 * Author:	D. W. Caress (MB-System revisions)
 * Date:	March 3, 2014 (MB-System revisions)
 *
 *--------------------------------------------------------------------*/
/*
 *	Copyright (c) 1993 by University of Hawaii.
 */

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "mbbs_defines.h"

static int tm_monthdays[]= { 31, 28, 31, 30, 31, 30,
			     31, 31, 30, 31, 30, 31,
			     31 };
static int tm_callertz= TM_TZ_UNKNOWN;

static char *tm_newtzval, *tm_oldtzval;
static int tm_rsttzval, tm_clrtzval;

int
mbbs_tmparsegmttz(char *str, int tmmode, double *dtm)
/*
   It turns out that making large numbers of calls to mbbs_tmparse()
   is very expensive memory-wise due to the repeated calloc() calls
   which are performed whenever mbbs_setgmttz() and mbbs_rsttz() are
   called by mbbs_tmparse(). It is much cheaper to have the caller
   set the time zone to GMT once, if possible, and then call this
   routine instead. This routine sets and restores a special flag
   that turns both mbbs_setgmttz() and mbbs_rsttz() into no-ops.
*/
{
	int err;
	int mbbs_tmparse(char *, int, double *);

	tm_callertz= TM_TZ_GMT;
	err= mbbs_tmparse(str, tmmode, dtm);
	tm_callertz= TM_TZ_UNKNOWN;

	return err;
}

int
mbbs_tmparse(char *str, int tmmode, double *dtm)
{
	char tmstrcp[TM_MAXSTRLEN+1];
	struct tm ts;
	char *token, *cp;
	double seconds, fraction;
	int err;
	time_t tmt;
	int mbbs_setgmttz();
	int mbbs_rsttz(int);
	void mbbs_jul2cal(struct tm *), mbbs_cal2jul(struct tm *);

	switch (tmmode) {
	case TM_JULIAN:
	case TM_CALENDAR:
		break;
	default:
		return BS_BADARG;
	}

	/* mktime() gives unreliable results unless
	   the time zone is explicitly set to GMT */
	if ((err= mbbs_setgmttz()) != BS_SUCCESS)
		return err;

	if ((str == (char *) 0) || (strlen(str) == 0))
		return mbbs_rsttz(BS_BADARG);
	else if (strlen(str) <= TM_MAXSTRLEN)
		(void) strcpy(tmstrcp, str);
	else
		return mbbs_rsttz(BS_BADARG);

	ts.tm_sec= 0;
	ts.tm_min= 0;
	ts.tm_hour= 0;
	ts.tm_mday= 1;
	ts.tm_mon= 0;
	ts.tm_wday= 0;
	ts.tm_yday= 0;
	ts.tm_isdst= 0;

	if ((token= strtok(tmstrcp, ":/- ")) == (char *) 0)
		 return mbbs_rsttz(BS_BADARG);
	ts.tm_year= (int) strtol(token, &cp, 10);
	if ((cp == token) || (*cp != '\0'))
		 return mbbs_rsttz(BS_BADARG);
	if (ts.tm_year < 0)
		 return mbbs_rsttz(BS_BADARG);
	if (ts.tm_year < 100) {

		/* this breaks in 2050 ;-> */
		if (ts.tm_year < 50)
			ts.tm_year+= 2000;
		else
			ts.tm_year+= 1900;
	}
	ts.tm_year-= 1900;

	if ((token= strtok((char *) 0, ":/- ")) == (char *) 0) {
		if ((tmt= mktime(&ts)) == (time_t) -1)
			return mbbs_rsttz(BS_FAILURE);
		*dtm= (double) tmt;
		return mbbs_rsttz(BS_SUCCESS);
	}
	switch (tmmode) {
	case TM_JULIAN:
		ts.tm_yday= (int) strtol(token, &cp, 10);
		if ((cp == token) || (*cp != '\0'))
			 return mbbs_rsttz(BS_BADARG);
		if ((ts.tm_yday < 1) || (ts.tm_yday > 366))
			 return mbbs_rsttz(BS_BADARG);
		ts.tm_yday-= 1;
		mbbs_jul2cal(&ts);
		break;

	case TM_CALENDAR:
		ts.tm_mon= (int) strtol(token, &cp, 10);
		if ((cp == token) || (*cp != '\0'))
			 return mbbs_rsttz(BS_BADARG);
		if ((ts.tm_mon < 1) || (ts.tm_mon > 12))
			 return mbbs_rsttz(BS_BADARG);
		ts.tm_mon-= 1;

		if ((token= strtok((char *) 0, ":/- ")) == (char *) 0) {
			mbbs_cal2jul(&ts);
			if ((tmt= mktime(&ts)) == (time_t) -1)
				return mbbs_rsttz(BS_FAILURE);
			*dtm= (double) tmt;
			return mbbs_rsttz(BS_SUCCESS);
		}
		ts.tm_mday= (int) strtol(token, &cp, 10);
		if ((cp == token) || (*cp != '\0'))
			 return mbbs_rsttz(BS_BADARG);
		if ((ts.tm_mday < 1) || (ts.tm_mday > 31))
			 return mbbs_rsttz(BS_BADARG);
		mbbs_cal2jul(&ts);
		break;
	}

	if ((token= strtok((char *) 0, ":/- ")) == (char *) 0) {
		if ((tmt= mktime(&ts)) == (time_t) -1)
			return mbbs_rsttz(BS_FAILURE);
		*dtm= (double) tmt;
		return mbbs_rsttz(BS_SUCCESS);
	}
	ts.tm_hour= (int) strtol(token, &cp, 10);
	if ((cp == token) || (*cp != '\0'))
		 return mbbs_rsttz(BS_BADARG);
	if ((ts.tm_hour < 0) || (ts.tm_hour > 23))
		 return mbbs_rsttz(BS_BADARG);

	if ((token= strtok((char *) 0, ":/- ")) == (char *) 0) {
		if ((tmt= mktime(&ts)) == (time_t) -1)
			return mbbs_rsttz(BS_FAILURE);
		*dtm= (double) tmt;
		return mbbs_rsttz(BS_SUCCESS);
	}
	ts.tm_min= (int) strtol(token, &cp, 10);
	if ((cp == token) || (*cp != '\0'))
		 return mbbs_rsttz(BS_BADARG);
	if ((ts.tm_min < 0) || (ts.tm_min > 59))
		 return mbbs_rsttz(BS_BADARG);

	if ((token= strtok((char *) 0, ":/- ")) == (char *) 0) {
		if ((tmt= mktime(&ts)) == (time_t) -1)
			return mbbs_rsttz(BS_FAILURE);
		*dtm= (double) tmt;
		return mbbs_rsttz(BS_SUCCESS);
	}
	seconds= strtod(token, &cp);
	if ((cp == token) || (*cp != '\0'))
		 return mbbs_rsttz(BS_BADARG);
	if ((seconds < 0.) || (seconds >= 60.))
		 return mbbs_rsttz(BS_BADARG);
	ts.tm_sec= (int) seconds;
	fraction= seconds-((double) ts.tm_sec);

	if ((tmt= mktime(&ts)) == (time_t) -1)
		return mbbs_rsttz(BS_FAILURE);
	*dtm= ((double) tmt)+fraction;

	return mbbs_rsttz(BS_SUCCESS);
}

int
mbbs_setgmttz()
/*
   Set the timezone to GMT if necessary so that
   SYSV mktime() will work properly. A call to this
   function should always be followed by a call to
   mbbs_rsttz() to undo its effect, if any.
*/
{
	char *tz;

	if (tm_callertz == TM_TZ_GMT)
		return BS_SUCCESS;

	tm_rsttzval= tm_clrtzval= 0;
	if (((tz= getenv("TZ")) != (char *) 0) && (strlen(tz) > 0)) {
		if (strcmp(tz, "GMT")) {
			if ((tm_oldtzval= (char *) calloc((size_t) (strlen("TZ=")+strlen(tz)+1), sizeof(char))) == (char *) 0)
				return BS_MEMALLOC;
			(void) strcpy(tm_oldtzval, "TZ=");
			(void) strcat(tm_oldtzval, tz);
			if ((tm_newtzval= (char *) calloc((size_t) (strlen("TZ=GMT")+1), sizeof(char))) == (char *) 0) {
				free((void *) tm_oldtzval);
				return BS_MEMALLOC;
			}
			(void) strcpy(tm_newtzval, "TZ=GMT");
			if (putenv(tm_newtzval) != 0) {
				free((void *) tm_oldtzval);
				free((void *) tm_newtzval);
				return BS_FAILURE;
			}
			tm_rsttzval= 1;
		}
	}
	else {
		if ((tm_newtzval= (char *) calloc((size_t) (strlen("TZ=GMT")+1), sizeof(char))) == (char *) 0)
			return BS_MEMALLOC;
		(void) strcpy(tm_newtzval, "TZ=GMT");
		if (putenv(tm_newtzval) != 0) {
			free((void *) tm_newtzval);
			return BS_FAILURE;
		}
		tm_clrtzval= 1;
	}

	return BS_SUCCESS;
}

int
mbbs_rsttz(int code)
/*
   Attempt to restore the original timezone that was replaced
   by a preceding mbbs_setgmttz() call. If there was no original
   timezone, just zero the value. This function is always called
   from within a return statement, so always return the code
   passed in as an argument regardless of whether the environment
   is successfully restored to its original state.
*/
{
	if (tm_callertz == TM_TZ_GMT)
		return code;
	if (tm_rsttzval)
		(void) putenv(tm_oldtzval);
	else if (tm_clrtzval) {
		if ((tm_newtzval= (char *) calloc((size_t) (strlen("TZ=")+1), sizeof(char))) == (char *) 0)
			return code;
		(void) strcpy(tm_newtzval, "TZ=");
		(void) putenv(tm_newtzval);
	}

	return code;
}

void
mbbs_jul2cal(struct tm *ts)
/*
   Derive Unix calendar month (0-11) and day (1-31)
   from Unix year (real year minus 1900) and julian day (0-365).
*/
{
	int mdays, leap;
	int mbbs_leapyr(struct tm *);

	leap= mbbs_leapyr(ts);
	ts->tm_mday= ts->tm_yday+1;
	for (ts->tm_mon= 0; ts->tm_mon < 12; ts->tm_mon++) {
		mdays= tm_monthdays[ts->tm_mon];
		if (leap && (ts->tm_mon == 1))
			mdays++;
		if (ts->tm_mday <= mdays)
			return;
		ts->tm_mday-= mdays;
	}

	return;
}

void
mbbs_cal2jul(struct tm *ts)
/*
   Derive Unix julian day (0-365) from Unix year (real year
   minus 1900), calendar month (0-11) and day (1-31).
*/
{
	int leap, month, mdays;
	int mbbs_leapyr(struct tm *);

	leap= mbbs_leapyr(ts);
	ts->tm_yday= 0;
	for (month= 0; month < ts->tm_mon; month++){
		mdays= tm_monthdays[month];
		if (leap && (month == 1))
			mdays++;
		ts->tm_yday+= mdays;
	}
	ts->tm_yday+= ts->tm_mday-1;

	return;
}

int
mbbs_leapyr(struct tm *ts)
/*
   Returns 1 if leap year, 0 otherwise.
*/
{
	int year;

	year= ts->tm_year+1900;

	return ((year%4 == 0) && ((year%100 != 0) || (year%400 == 0)));
}
