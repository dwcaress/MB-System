/*******************************************************************************
NAME:                            TESTPROJ 

PURPOSE:	Makes a grid of GCTPc converted map projection
		points.  The User enters the Min and Max Lat and Long
		and the increment value.  A grid of points will be converted
		from Long/Lat to X/Y and back to Long/Lat.  The results
		will be entered into a file specified by the user.  The
		user optionally may compare the transformed data to the data
		in an existing file of the same format.


VERSION	   DATE		PROGRAMMER		REASON
-------    ----    	----------		------
1.0	   05/93	T. Mittan/S. Nelson	Initial development
1.1	   12/93	S. Nelson		Converted TEST to not only
						make a new projection table
						but also to optionally
						compare with an existing file.
c.1.1	    1/95	S. Nelson		Added OUTDATUM to the GCTP
						subroutine call.


ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Proffessional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cproj.h"
#include "proj.h"

#define CMLEN	256

main()
{
long k;
char file27[CMLEN];
char file83[CMLEN];
char *ptr;
char libgctp[CMLEN];
float i;
float j;
double incoor[2];
double incoorc[2];
double outcoor[2];
double incoorx[2];
double incoorcx[2];
double outcoorx[2];
long insys = 0;
long inzone;
double inparm[15];
long inunit;
long indatum;
long ipr = 2;
long jpr = 2;
long jprinv = 3;
long outsys;
long outzone;
double outparm[15];
long outunit;
long outdatum;
long proj;
long zonec;
float min_lon;
float max_lon;
float min_lat;
float max_lat;
long count;
long count2;
float lon_inc;
float lat_inc;
float tol;		/* tolerance value for acceptable comparisons	*/
double proj_parm[15];	/* projection parameter array			*/
long fflg;		/* error flag for forward conversion of C version */
long iflg;		/* error flag for inverse conversion of C version */

char buf2[82];		/* buffer for file2 information			*/
char temp2[82];		/* temporary buffer for header strings		*/
char fflag[2];		/* flag for saving converted values to a file	*/
char file1[CMLEN];	/* name of new data file     			*/
char file2[CMLEN];	/* name of existing file			*/
char file3[CMLEN];	/* name of Comparison file			*/
char efile[CMLEN];	/* name of error file				*/

FILE	*fptr1, *fptr2, *fptr3;

/* Assign file names
   -----------------*/
strcpy (efile,"error_file.txt");
strcpy (file27,"");
strcpy (file83,"");
strcpy (libgctp,"");

/* Get needed information
-----------------------*/
printf(" 0    GEOGRAPHIC		");
printf(" 1    UTM \n");
printf(" 2    STATE PLANE		");
printf(" 3    ALBERS CONICAL EQ AR \n");
printf(" 4    LAMBERT CONFORMAL		");
printf(" 5    MERCATOR \n");
printf(" 6    POLAR STEREOGRAPHIC	");
printf(" 7    POLYCONIC \n");
printf(" 8    EQUIDISTANT CONIC		");
printf(" 9    TRANSVERSE MERCATOR \n");
printf("10    STEREOGRAPHIC		");
printf("11    LAMBERT AZIMUTHAL \n");
printf("12    AZIMUTHAL EQUDISTANT	");
printf("13    GNOMONIC \n");
printf("14    ORHTOGRAPHIC		");
printf("15    GVNSP \n");
printf("16    SINUSIODAL		");
printf("17    EQUIRECTANGULAR \n");
printf("18    MILLER			");
printf("19    VAN DER GRINTEN \n");
printf("20    OBLIQUE MERCATOR		");
printf("21    ROBINSON \n");
printf("22    SOM			");
printf("23    ALASKA CONFORMAL \n");
printf("24    GOODE			");
printf("25    MOLLWEIDE \n");
printf("26    INTERRUPTED MOLLEIDE	");
printf("27    HAMMER \n");
printf("28    WAGNER IV			");
printf("29    WAGNER VII \n");
printf("30    OBLATED EQ AR		");

printf("\n Please enter the projection number \n");
scanf("%d",&proj);

if ((proj == 1) || (proj == 2))
   {
   printf("\n Please enter the zone number \n");
   scanf("%d",&zonec);
   }
else
   {
   zonec = 62;
   }
 
for (k = 0; k < 15; k++)
    {
    printf("\n Please enter projection parameter %d \n",k);
    scanf("%lf",&proj_parm[k]);
    inparm[k] = 0.0;
    outparm[k] = 0.0;
    }

printf
("\nPlease enter the minimum longitude in degrees (negative for west of zero)\n");
scanf("%f",&min_lon);
printf
("\nPlease enter the maximum longitude in degrees (negative for west of zero)\n");
scanf("%f",&max_lon);
printf("\n Please enter the minimum latitude in degrees\n");
scanf("%f",&min_lat);
printf("\n Please enter the maximum latitude in degrees\n");
scanf("%f",&max_lat);

printf("\n Please enter the longitude increment in degrees\n");
scanf("%f",&lon_inc);
printf("\n Please enter the latitude increment in degrees\n");
scanf("%f",&lat_inc);

/* Get name of new data file
   -------------------------*/
printf("\n Please enter name of the new data file\n");
scanf("%s",file1);

/* If files are to be saved with converted values get file names
  -------------------------------------------------------------*/
printf("\n Are the values to be compared? (Y or N)\n");
scanf("%s",fflag);
*fflag = toupper(*fflag);

if (!strcmp(fflag,"Y"))
   {
   printf("\n Please enter name of the existing data file\n");
   scanf("%s",file2);
   }

if (!strcmp(fflag,"Y"))
   {
   printf("\n Please enter the tolerance value\n");
   scanf("%f",&tol);

   printf("\n Please enter name of the Comparison file\n");
   scanf("%s",file3);
   fptr3 = fopen(file3,"w");

   /* open existing data file
      ----------------------*/
   fptr2 = fopen(file2,"r");
   if (fptr2 == '\0')
      {
      printf("\nError opening comparison file--discontinue comparison\n");
      fprintf(fptr3,"\nError opening comp file--discontinue comparison\n");
      strcpy(fflag,"N");
      fclose(fptr3);
      }
   count2 = 0;
   
   /* Get past the heading, will stop after the line beginning with "-"
      ----------------------------------------------------------------*/
   if (!strcmp(fflag,"Y"))
    {
    for(;;)
      {
      count2++;
      if (fgets(buf2,81,fptr2) == NULL)
	 {
	 printf("\nError reading comparison file--discontinue comparison\n");
	 fprintf(fptr3,"\nError reading comp file--discontinue comparison\n");
	 strcpy(fflag,"N");
	 fclose(fptr2);
	 fclose(fptr3);
	 break;
	 }

       /* Break at "-",  test to avoid invalid data file (count2)
	  ------------------------------------------------------*/
       sscanf(buf2,"%s",temp2);
       if (temp2[1] == '-')
	 break;
       else if (count2 > 50)
	 {
	 printf("\nError reading comparison file--discontinue comparison\n");
	 fprintf(fptr3,"\nError reading comp file--discontinue comparison\n");
	 strcpy(fflag,"N");
	 fclose(fptr2);
	 fclose(fptr3);
	 break;
	 }
       }
    }
   }

/* Empty contents of new data file if it exists
---------------------------------------------*/
if((fptr1 = fopen(file1,"w")) == NULL)
fclose(fptr1);

/* Test projection 
----------------*/
/* set variables not entered with prompt 
   ------------------------------------*/
inzone = 62;
outzone = 62;
insys = 0;
outsys = 0;
count = 0;

/* for State Plane projection, get data files
   ------------------------------------------*/
if (proj == SPCS)
   {
   ptr = (char *)getenv("LIBGCTP");
   strcpy(libgctp,ptr);
   sprintf(file27,"%s/nad27sp", libgctp);
   sprintf(file83,"%s/nad83sp", libgctp);
   }

/*  Set values to change the datum and radius
    ----------------------------------------*/
indatum = 0;
if ((proj_parm[0] != 0) && (proj_parm[0] != 6370997))
   if ((proj != 1) && (proj != 2))
      indatum = -1;
outdatum = indatum;

/*  Begin convertion of grid points
   -------------------------------*/
for(j = min_lon; j <= max_lon; j += fabs(lon_inc))
  {
  for(i = min_lat; i <= max_lat; i += fabs(lat_inc))
    {
    incoor[1] =  i;
    incoor[0] =  j;

    inunit = 4;
    outunit = 2;

    /* Call "C" version forward--lat/long to meters
      --------------------------------------------*/
    gctp(incoor,&insys,&inzone,inparm,&inunit,&indatum,&ipr,efile,&jpr,file1,
			outcoor,&proj,&zonec,proj_parm,&outunit,&outdatum,
			file27, file83,&fflg);

    /* Print Headings
       -------------*/
    if ((i == min_lat) && (j == min_lon))
	{
        if((fptr1 = fopen(file1,"a")) == NULL)
	    {
	    printf("\nError opening output data file");
	    }
	fprintf(fptr1,"\n INPUT LONG LAT        X              Y         ");
	fprintf(fptr1,"  OUTPUT LON      OUTPUT LAT");
	fprintf(fptr1,"\n--------------- --------------- --------------- ");
	fprintf(fptr1,"--------------- ---------------\n");
        fclose(fptr1);
	}

    incoorc[0] = outcoor[0];
    incoorc[1] = outcoor[1];
    inunit = 2;
    outunit = 4;

    /* Call "C" version inverse--meters to lat/long
      --------------------------------------------*/
    gctp(incoorc,&proj,&zonec,proj_parm,&inunit,&indatum,&ipr,efile,&jprinv,
        		file1,outcoor,&outsys,&outzone,outparm,
			&outunit,&outdatum,file27,file83,&iflg);

    /*  Print points to file
	-------------------*/
    if((fptr1 = fopen(file1,"a")) == NULL)
	 {
	 printf("\nError opening output data file");
	 }
    fprintf(fptr1,"%7.2lf %6.2lf %15.5lf %15.5lf %15.5lf %15.5lf\n",
	 incoor[0],incoor[1],incoorc[0],incoorc[1],outcoor[0],outcoor[1]);
    fclose(fptr1);

    count++;

    /* Compare values to data file
    -----------------------------*/
    if (!strcmp(fflag,"Y"))
      {
      if (fscanf(fptr2,"%lf %lf %lf %lf %lf %lf",&incoorx[0],&incoorx[1],
		&incoorcx[0],&incoorcx[1],&outcoorx[0],&outcoorx[1]) != 6)
	 {
	 printf("\nError reading comparison file--discontinue comparison\n");
	 fprintf(fptr3,"\nError reading comp file--discontinue comparison\n");
	 strcpy(fflag,"N");
	 fclose(fptr3);
	 fclose(fptr2);
	 }
    /* Check if original lat/long are the same--if not, error in comp file
       -------------------------------------------------------------------*/
       if (!strcmp(fflag,"Y"))
	{
        if(fabs(incoor[0]-incoorx[0])>tol || (fabs(incoor[1]-incoorx[1])>tol))
	   {
	   printf("\nInconsistant input Long/Lat--discontinue comparison\n");
	   fprintf(fptr3,
		"\nInconsistant input Long/Lat--discontinue comparison\n");
	   strcpy(fflag,"N");
	   fclose(fptr3);
	   fclose(fptr2);
	   }
	 }
       if (!strcmp(fflag,"Y"))
	 {
         if((fabs(incoorc[0] - incoorcx[0]) > tol) ||
			(fabs(incoorc[1] - incoorcx[1]) > tol))
	   {
	   fprintf(fptr3,"Differences exist at long %lf lat %lf\n",incoor[0],
							incoor[1]);
	   printf("\nDifferences exist at long %lf lat %lf",incoor[0],
							incoor[1]);
	   fprintf(fptr3,"X n %lf X o %lf Y n %lf Y o %lf\n",
			incoorc[0], incoorcx[0], incoorc[1], incoorcx[1]);
	   }
	 else if ((fabs(outcoor[0] - outcoorx[0]) > tol) ||
			(fabs(outcoor[1] - outcoorx[1]) > tol))
 	   {
	   fprintf(fptr3,"Differences exist at long %lf lat %lf\n",incoor[0],
							incoor[1]);
	   printf("\nDifferences exist at long %lf lat %lf",incoor[0],
							incoor[1]);
	   fprintf(fptr3,"lon new %lf lon old %lf lat new %lf lat old %lf\n",
			outcoor[0], outcoorx[0], outcoor[1], outcoorx[1]);
	   }
	 }
      }			/* end if comparison checks */
    }			/* end of for i */
  }			/* end of for j */
if (!strcmp(fflag,"Y"))
  {
  fclose(fptr3);
  fclose(fptr2);
  }
printf("\nNumber of points transformed %d \n",count);
}
