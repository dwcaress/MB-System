/*
 *	MBPLOTFILTER is a filter to take the output of mb_realtime and put
 *	it into Lamont graphics metafile format.  Essentially, it
 *	reads the plot calls made by mb_contfilter and in turn
 *	makes the appropriate calls to the Lamont graphics library.
 *
 *	David W. Caress
 *	L-DGO
 *	May 1991
 */
/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <graphics.h>

/* MBIO include files */
#include "../../include/mb_status.h"

#define IUP 3
#define IDN 2

char	line[200];
float	width, xmin,xmax,ymin,ymax;
float	homex = 25.;
float	homey = 25;
float	x,y;
int	verbose;
int	ipen;
int	fd;
int	status;
int	istart,iplotting;
float	cmerc, er, pr, scl;
float     mtx[3][2];

main(argc, argv)
int argc;
char **argv; 
{
	int	error = MB_ERROR_NO_ERROR;
	int	i;

	/* set defaults */
	width = 24.0;	/* in inches */
	xmin = 0.0;
	xmax = 0.0;
	ymin = 0.0;
	ymax = 0.0;
	verbose = 0;

	/* process argument list */
	for (i = 1; i < argc; i++) 
		{
		if (argv[i][0] == '-') 
			{
			switch (argv[i][1]) 
				{
				case 'R':
				case 'r':
					sscanf (&argv[i][2], "%f/%f/%f/%f", 
						&xmin,&xmax,&ymin,&ymax);
					break;
				case 'W':
				case 'w':
					sscanf (&argv[i][2], "%f",
						&width);
					break;
				case 'V':
				case 'v':
					verbose = 1;
					break;
				}
			}
		}

	/* check for bounds */
	if (xmin == xmax || ymin == ymax)
		{
		fprintf(stderr,"mb_plotfilter error: bounds not set or set incorrectly\n");
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* read until init call is made */
	istart = 1;
	while (istart)
		{
		fgets(line,sizeof(line),stdin);
		if (strncmp(line,"init",4) == 0)
			{
			istart = 0;
			initg();
			fd = 1;
			mfon(fd,0);
			cmerc = (xmin +xmax)/2.;
			scl = -width/(xmax-xmin);
			er = 0.;
			pr = 0.;
			fprintf(stderr,"calling vmerc: %f %f %f %f\n",
				cmerc,scl,er,pr);
			vmerc_(&cmerc,&scl,&er,&pr);
			fprintf(stderr,"done vmerc: %f %f %f %f\n",
				cmerc,scl,er,pr);
			x = xmin;
			y = ymin;
			fprintf(stderr,"calling xymerc: xmin:%f ymin:%f\n",x,y);
			xymerc_(&x,&y);
			fprintf(stderr,"done xymerc: xmin:%f ymin:%f\n:",x,y);
			fprintf(stderr,"calling initm2: [0][0]:%d [0][1]:%d %d [1][0]:%d [1][1]:%d %d [2][0]:%d [2][1]:%d %d\n",
				mtx[0][0],mtx[0][1],
				mtx[1][0],mtx[1][1],
				mtx[2][0],mtx[2][1]);
			initm2(mtx);
			fprintf(stderr,"done initm2: [0][0]:%d [0][1]:%d %d [1][0]:%d [1][1]:%d %d [2][0]:%d [2][1]:%d %d\n",
				mtx[0][0],mtx[0][1],
				mtx[1][0],mtx[1][1],
				mtx[2][0],mtx[2][1]);
			fprintf(stderr,"calling tlate2: %f %f\n",-x,-y);
			tlate2(mtx,-x,-y);
			fprintf(stderr,"done tlate2: %f %f\n",-x,-y);
			makcur();

			/* init message */
			if (verbose)
				{
				fprintf(stderr,"\nMBPLOTFILTER:\n");
				fprintf(stderr,"bounds: %f %f %f %f\n",xmin,xmax,ymin,ymax);
				fprintf(stderr,"width:  %f\n\n",width);
				}
			}
		}


	/* now read until stop call is made */
	iplotting = 1;
	while (iplotting)
		{
		fgets(line,sizeof(line),stdin);
		if (strncmp(line,"plot",4) == 0)
			{
			sscanf(line, "plot %f %f %d",&x,&y,&ipen);
			if (x > -360.1 && x < 360.1 
				&& y > -90.1 && y< 90.1)
				{
				xymerc_(&x,&y);
				domtr2(mtx,&x,&y);
				if (ipen == IUP)
					move2(x,y);
				else if (ipen == IDN)
					line2(x,y);
				}
			}
		else if (strncmp(line,"newp",4) == 0)
			{
			sscanf(line, "newp %d",&ipen);
			setpen(ipen+1);
			}
		else if (strncmp(line,"flus",4) == 0)
			{
			for (i=0;i<210;i++)
				move2(x,y);
			makcur();
			fflush(stdout);
			fflush(stderr);
			}
		else if (strncmp(line,"stop",4) == 0 || status == EOF)
			{
			iplotting = 0;
			move2(homex,homey);
			makcur();
			mfoff(fd,0);
			termg();
			fflush(stdout);
			fflush(stderr);
			close(fd);
			}

		}

	/* all done */
	exit(error);
}
