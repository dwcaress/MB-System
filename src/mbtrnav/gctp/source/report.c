/*******************************************************************************
NAME                    Projection support routines listed below

PURPOSE:	The following functions are included in REPORT.C	

		INIT:
			Initializes the output device for error messages and
		  	report headings.

		P_ERROR:
			Reports errors to the terminal, a specified file, or
			both.

		PTITLE, RADIUS, RADIUS2, CENLON, CENLONMER, CENLAT, ORIGIN,
		STANPARL, STPARL1, OFFSET, GENRPT, GENRPT_LONG, PBLANK:
			Reports projection parameters to the terminal,
			specified file, or both. 


PROGRAMMER              DATE		REASON
----------              ----		------
D. Steinwand, EROS      July, 1991	Initial development.
T. Mittan		Mar,  1993	Adapted code to new "C" version of
					GCTP library.
S. Nelson		Jun, 1993	Added inline code. 
					Added error messages if no filename
					was specified.
S. Nelson		Jan, 1998	Returned OK instead of 0.

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "cproj.h"

#define TRUE 1
#define FALSE 0

static long terminal_p;		/* flag for printing parameters to terminal */
static long terminal_e;		/* flag for printing errors to terminal */
static long file_p;		/* flag for printing parameters to file */
static long file_e;		/* flag for printing errors to terminal */
static FILE  *fptr_p;
static FILE  *fptr_e;
static char parm_file[256];
static char err_file[256];

/* initialize output device
-------------------------*/
long init(ipr,jpr,efile,pfile)

long ipr;		/* flag for printing errors (0,1,or 2)		*/
long jpr;		/* flag for printing parameters (0,1,or 2)	*/
char *efile;		/* name of error file				*/
char *pfile;		/* name of parameter file			*/

{
if (ipr == 0)
   {
   terminal_e = TRUE;
   file_e = FALSE;
   }
else
if (ipr == 1)
   {
   terminal_e = FALSE;
   if (strlen(efile) == 0)
      {
      return(6);
      }
   file_e = TRUE;
   strcpy(err_file,efile);
   }
else
if (ipr == 2)
   {
   terminal_e = TRUE;
   if (strlen(efile) == 0)
      {
      file_e = FALSE;
      p_error("Output file name not specified","report-file");
      return(6);
      }
   file_e = TRUE;
   strcpy(err_file,efile);
   }
else
   {
   terminal_e = FALSE;
   file_e = FALSE;
   }
if (jpr == 0)
   {
   terminal_p = TRUE;
   file_p = FALSE;
   }
else
if (jpr == 1)
   {
   terminal_p = FALSE;
   if (strlen(pfile) == 0)
      {
      return(6);
      }
   file_p = TRUE;
   strcpy(parm_file,pfile);
   }
else
if (jpr == 2)
   {
   terminal_p = TRUE;
   if (strlen(pfile) == 0)
      {
      file_p = FALSE;
      p_error("Output file name not specified","report-file");
      return(6);
      }
   file_p = TRUE;
   strcpy(parm_file,pfile);
   }
else
   {
   terminal_p = FALSE;
   file_p = FALSE;
   }
return(OK);
}

void close_file()
{
if (fptr_e != NULL)
   fclose(fptr_e);
if (fptr_p != NULL)
   fclose(fptr_p);
}

/* Functions to report projection parameters
  -----------------------------------------*/
void ptitle(A)
  char *A; 
      {  
      if (terminal_p)
           printf("\n%s PROJECTION PARAMETERS:\n\n",A); 
      if (file_p)
	   {
           fptr_p = (FILE *)fopen(parm_file,"a");
           fprintf(fptr_p,"\n%s PROJECTION PARAMETERS:\n\n",A); 
	   fclose(fptr_p);
	   }
      }

void radius(A)
  double A;
      {
      if (terminal_p)
         printf("   Radius of Sphere:     %lf meters\n",A); 
      if (file_p)
	 {
         fptr_p = (FILE *)fopen(parm_file,"a");
         fprintf(fptr_p,"   Radius of Sphere:     %lf meters\n",A); 
	 fclose(fptr_p);
	 }
      }

void radius2(A,B)
  double A,B;
      {
      if (terminal_p)
         {
         printf("   Semi-Major Axis of Ellipsoid:     %lf meters\n",A);
         printf("   Semi-Minor Axis of Ellipsoid:     %lf meters\n",B);
         }
      if (file_p)
         {
         fptr_p = (FILE *)fopen(parm_file,"a");
         fprintf(fptr_p,"   Semi-Major Axis of Ellipsoid:     %lf meters\n",A);
         fprintf(fptr_p,"   Semi-Minor Axis of Ellipsoid:     %lf meters\n",B); 
	 fclose(fptr_p);
         }
      }

void cenlon(A)
  double A;
   { 
   if (terminal_p)
       printf("   Longitude of Center:     %lf degrees\n",A*R2D);
   if (file_p)
       {
       fptr_p = (FILE *)fopen(parm_file,"a");
       fprintf(fptr_p,"   Longitude of Center:     %lf degrees\n",A*R2D);
       fclose(fptr_p);
       }
   }
 
void cenlonmer(A)
  double A;
   { 
   if (terminal_p)
     printf("   Longitude of Central Meridian:     %lf degrees\n",A*R2D);
   if (file_p)
     {
     fptr_p = (FILE *)fopen(parm_file,"a");
    fprintf(fptr_p,"   Longitude of Central Meridian:     %lf degrees\n",A*R2D);
     fclose(fptr_p);
     }
   }

void cenlat(A)
  double A;
   {
   if (terminal_p)
      printf("   Latitude  of Center:     %lf degrees\n",A*R2D);
   if (file_p)
      {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   Latitude of Center:     %lf degrees\n",A*R2D);
      fclose(fptr_p);
      }
   }

void origin(A)
  double A;
   {
   if (terminal_p)
      printf("   Latitude of Origin:     %lf degrees\n",A*R2D);
   if (file_p)
      {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   Latitude  of Origin:     %lf degrees\n",A*R2D);
      fclose(fptr_p);
      }
   }
void stanparl(A,B)
  double A,B;
   {
   if (terminal_p)
      {
      printf("   1st Standard Parallel:     %lf degrees\n",A*R2D);
      printf("   2nd Standard Parallel:     %lf degrees\n",B*R2D);
      }
   if (file_p)
      {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   1st Standard Parallel:     %lf degrees\n",A*R2D);
      fprintf(fptr_p,"   2nd Standard Parallel:     %lf degrees\n",B*R2D);
      fclose(fptr_p);
      }
   }

void stparl1(A)
  double A;
   {
   if (terminal_p)
      {
      printf("   Standard Parallel:     %lf degrees\n",A*R2D);
      }
   if (file_p)
      {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   Standard Parallel:     %lf degrees\n",A*R2D);
      fclose(fptr_p);
      }
   }

void offsetp(A,B)
  double A,B;
   {
   if (terminal_p)
      {
      printf("   False Easting:      %lf meters \n",A);
      printf("   False Northing:     %lf meters \n",B);
      }
   if (file_p)
      {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   False Easting:      %lf meters \n",A);
      fprintf(fptr_p,"   False Northing:     %lf meters \n",B);
      fclose(fptr_p);
      }      
   }

void genrpt(A,S)
  double A; char *S;
   {
   if (terminal_p)
      printf("   %s %lf\n", S, A);
   if (file_p)
      {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   %s %lf\n", S, A);
      fclose(fptr_p);
      }
   }
void genrpt_long(A,S)
  long A; char *S;
   {
   if (terminal_p)
      printf("   %s %d\n", S, A);
   if (file_p)
      {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   %s %d\n", S, A);
      fclose(fptr_p);
      }
   }
void pblank() 
   {
   if (terminal_p)
      printf("\n");
   if (file_p)
      {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"\n");
      fclose(fptr_p);
      }
   }

/* Function to report errors 
  -------------------------*/
void p_error(what, where) 
   char *what;
   char *where; 
   {
   if (terminal_e)
      printf("[%s] %s\n",where,what);
   if (file_e)
      {
      fptr_e = (FILE *)fopen(err_file,"a");
      fprintf(fptr_e,"[%s] %s\n",where,what);
      fclose(fptr_e);
      }
   }
