/*
 *	navcross -	part of the navadjust interactive multi-beam 
 *			bathymetry navigation adjustment package
 *
 *			navcross finds all crossing or overlapping
 *			swaths in a set of multibeam files.
 *
 *			David W. Caress
 *			Lamont-Doherty Geological Observatory
 *			August 1990
 */

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>

/* global array dimensions */
#define MAXSECTION 5000
#define MAXFILE 100
#define MAXLOCAL 20
#define MAXCROSS 5000

/* global structure definitions */
struct zone
	{
	int	numsec;
	int	local[MAXLOCAL];
	};
struct section
	{
	int	fileid;
	int	format;
	int	prior;
	int	post;
	int	btime_i[6];
	int	etime_i[6];
	int	output;
	int	nrec;
	double	distance;
	};
struct section *sec[MAXSECTION];
struct mbfile
	{
	char	file[128];
	int	format;
	int	prior;
	int	post;
	int	nsec;
	};
struct mbfile *files[MAXFILE];
struct cross
	{
	int	cross1;
	int	cross2;
	int	flag;
	};
struct cross *icross[MAXCROSS];
struct cross *fcross[MAXCROSS];

main (argc, argv)
int argc;
char **argv; 
{
	int	i, j, k, l, m, status;
	int	imn1, imx1, imn2, imx2;
	int	filemod;
	int	double_size;
	int	mbfile_size;
	int	section_size;
	int	zone_size;
	int	cross_size;

	/* navcross control variables */
	char	datalist[128];
	char	navlist[128];
	char	cmd1fil[128];
	char	cmd2fil[128];
	char	outlist[128];
	int	iverbose;
	int	nfil;
	int	nsectot;
	int	isec, ifil;
	int	connect, prior, post;
	int	xdim, ydim, dimtot;
	int	ix, iy, ib, ifound, indx;
	double	dx, dy;
	double	section_length;
	struct section *seccur, *seclst;
	struct zone *array;
	struct zone *zoncur;
	int	nicross, nfcross;
	int	x1, x2;
	FILE	*fp;

	/* mbio read initialization variables */
	int	pings, beams, lonflip;
	double	bounds[4];
	int	btime_i[6], etime_i[6];
	double	btime_d, etime_d;
	double	speedmin, timegap;

	/* mbio read values */
	int	rbeams, rpings;
	int	time_i[6];
	double	time_d;
	double	clon, clat;
	double	speed, heading, distance, pitch;
	double	*dep, *lon, *lat;

	/* initialize type and struct sizes */
	double_size = sizeof(double);
	mbfile_size = sizeof(struct mbfile);
	section_size = sizeof(struct section);
	zone_size = sizeof(struct zone);
	cross_size = sizeof(struct cross);

	/* initialize read control variables */
	pings = 1;
	lonflip = -1;
	bounds[0] = -360.0;
	bounds[1] = 360.0;
	bounds[2] = -90.0;
	bounds[3] = 90.0;
	btime_i[0] = 1970;
	btime_i[1] = 1;
	btime_i[2] = 1;
	btime_i[3] = 0;
	btime_i[4] = 0;
	btime_i[5] = 0;
	etime_i[0] = 1999;
	etime_i[1] = 1;
	etime_i[2] = 1;
	etime_i[3] = 0;
	etime_i[4] = 0;
	etime_i[5] = 0;
	speedmin = 0.0;
	timegap = 1.0;

	/* initialize navcross control variables */
	nfil = 0;
	nsectot = 0;
	iverbose = 0;
	connect = 0;
	section_length = 5;
	xdim = 10;
	ydim = 10;
	filemod = 493;
	nicross = 0;
	nfcross = 0;

	/* initialize filenames */
	strcpy(datalist,"data.list");
	strcpy(navlist,"nav.list");
	strcpy(cmd1fil,"section.cmd");
	strcpy(cmd2fil,"naverr.cmd");
	strcpy(outlist,"section.list");

	/* catch usage by idiot */
	if (argc < 4) 
		{
		printf("usage:  navcross -Rwest/east/south/north -Dxdim/ydim [-Ppings -Sspeed -Ttimegap -Llonflip -Xsection_length -Vverbose] -Idatalist -Nnavlist\n");
		exit(-1);
		}

	/* process argument list */
	for (i = 1; i < argc; i++) 
		{
		if (argv[i][0] == '-') 
			{
			switch (argv[i][1]) 
				{
				case 'R':
				case 'r':
					sscanf (&argv[i][2], "%lf/%lf/%lf/%lf", 
						&bounds[0],&bounds[1],
						&bounds[2],&bounds[3]);
					break;
				case 'D':
				case 'd':
					sscanf (&argv[i][2], "%d/%d", 
						&xdim,&ydim);
					break;
				case 'P':
				case 'p':
					sscanf (&argv[i][2], "%d", &pings);
					break;
				case 'S':
				case 's':
					sscanf (&argv[i][2], "%lf", &speedmin);
					break;
				case 'T':
				case 't':
					sscanf (&argv[i][2], "%lf", &timegap);
					break;
				case 'L':
				case 'l':
					sscanf (&argv[i][2], "%d", &lonflip);
					break;
				case 'V':
				case 'v':
					sscanf (&argv[i][2], "%d", &iverbose);
					if (iverbose > 2)iverbose = 1;
					if (iverbose < 0)iverbose = 0;
					break;
				case 'I':
				case 'i':
					strcpy (datalist, &argv[i][2]);
					break;
				case 'N':
				case 'n':
					strcpy (navlist, &argv[i][2]);
					break;
				case 'X':
				case 'x':
					sscanf (&argv[i][2], "%lf", &section_length);
					break;
				}
			}
		}

	/* read in list of data files */
	if ((fp = fopen(datalist,"r")) == NULL)
		{
		printf("unable to open list file:%s\n",datalist);
		exit(-1);
		}
	if ((files[nfil] = (struct mbfile *) 
		calloc(1,mbfile_size)) == NULL) exit(-1);
	while (fscanf(fp,"%s %d %d",files[nfil]->file,
		&files[nfil]->format,&files[nfil]->prior) != EOF)
		{
		if (nfil > 0) files[nfil-1]->post = files[nfil]->prior;
		if (nfil == 0) files[nfil]->prior = 0;
		files[nfil]->nsec = 0;
		if (files[nfil]->format <= 0) 
			{
			printf("illegal format %d for file:%s\n",
				files[nfil]->format,files[nfil]->file);
			exit(-1);
			}
		nfil++;
		if (nfil >= MAXFILE) 
			{
			printf("maximum number of files exceeded: %d\n",
				MAXFILE);
			exit(-1);
			}
		if ((files[nfil] = (struct mbfile *) 
			calloc(1,mbfile_size)) == NULL) exit(-1);
		}
	fclose(fp);
	if (nfil > 0) files[nfil-1]->post = 0;

	/* allocate memory for location array */
	dimtot = xdim*ydim;
	if ((array = (struct zone *) calloc(dimtot,zone_size)) == NULL)
		exit(-1);
	for (i=0;i<dimtot;i++)
		array[i].numsec = 0;
	dx = (bounds[1] - bounds[0])/xdim;
	dy = (bounds[3] - bounds[2])/ydim;

	/* loop over all of the files */
	for (ifil=0;ifil<nfil;ifil++)
		{

		/* initialize multibeam file */
		printf("processing file: %s\n",files[ifil]->file);
		if ((status = mb_read_init(files[ifil]->file,
			&files[ifil]->format,&pings,&lonflip,
			bounds,btime_i,etime_i,&btime_d,&etime_d,
			&speedmin,&timegap)) != 0)
			{
			printf("navcross:  mbio read initialization error status: %d\n",status);
			}
		prior = files[ifil]->prior;
		post = files[ifil]->post;

		/* allocate memory for reading data arrays */
		beams = mb_format(&files[ifil]->format);
		double_size = sizeof(double);
		free(dep);
		free(lon);
		free(lat);
		if ((dep = (double *) calloc(beams,double_size)) == NULL) 
			exit(-1);
		if ((lon = (double *) calloc(beams,double_size)) == NULL) 
			exit(-1);
		if ((lat = (double *) calloc(beams,double_size)) == NULL) 
			exit(-1);

		/* loop over reading */
		while (status <= 0)
			{

			/* initialize next section */
			if (nsectot >= MAXSECTION)
				{
				printf("maximum number of sections exceeded: %d\n",nsectot);
				exit(-1);
				}
			isec = files[ifil]->nsec;
			if ((sec[nsectot] = (struct section *) 
				calloc(1,section_size)) == NULL) exit(-1);
			seclst = seccur;
			seccur = sec[nsectot];
			seccur->output = 0;
			seccur->nrec = 0;
			if (isec > 0 && prior == 1)
				{
				for (i=0;i<6;i++)
					seccur->btime_i[i] = seclst->etime_i[i];
				}

			/* read first ping */
			status = mb_read(&rbeams,&rpings,time_i,&time_d,
				&clon,&clat,&speed,&heading,
				&distance,&pitch,dep,lon,lat);
			if (isec == 0 || prior == 0)
				{
				for (i=0;i<6;i++)
					seccur->btime_i[i] = time_i[i];
				}
			seccur->format = files[ifil]->format;
			seccur->prior = prior;
			seccur->post = post;
			seccur->distance = 0.0;
			seccur->fileid = ifil;
			seccur->nrec++;

			/* read and process to end of section */
			while (((status = mb_read(&rbeams,&rpings,time_i,
				&time_d,&clon,&clat,&speed,&heading,
				&distance,&pitch,dep,lon,lat)) == 0) 
				&& (seccur->distance <= section_length))
				{
				for (i=0;i<6;i++)
					seccur->etime_i[i] = time_i[i];
				seccur->distance = seccur->distance + distance;
				seccur->nrec++;

				/* loop over all beams */
				for (ib=0;ib<beams;ib++) if (dep[ib] > 0.0)
					{
					ix = (lon[ib]-bounds[0])/dx;
					iy = (lat[ib]-bounds[2])/dy;
					indx = ix + iy*xdim;
					if (ix >= 0 && ix < xdim && iy >= 0 && iy < ydim)
						{
						zoncur = &array[indx];
						ifound = 0;
						for (j=0;j<zoncur->numsec;j++)
							if (zoncur->local[j] == nsectot) ifound = 1;
						if (ifound == 0)
							{
							if (zoncur->numsec >= MAXLOCAL)
								{
								printf("maximum number of sections in zone exceeded: %d\n",zoncur->numsec);
								exit(-1);
								}
							zoncur->local[zoncur->numsec] = nsectot;
							zoncur->numsec++;
							}
						}
					}
				}

			/* deal with end of section */
			if (iverbose >= 2) printf("\nsection end status: %d\n",status);
			if (status > 0)
				{
				if ((seccur->distance < (0.5*section_length) )
					&& (seccur->prior == 1))
					{
					for (i=0;i<6;i++)
						seclst->etime_i[i] = 
							seccur->etime_i[i];
					seclst->distance = seclst->distance 
						+ seccur->distance;
					seclst->nrec = seclst->nrec + seccur->nrec;
					free(seccur);
					}
				else if (seccur->distance == 0.0)
					{
					free(seccur);
					}
				else
					{
					nsectot++;
					files[ifil]->nsec++;
					}
				}
			else if (status == 0)
				{
				nsectot++;
				files[ifil]->nsec++;
				prior = 1;
				seccur->post = 1;
				seccur->nrec++;
				}
			else if (status == -1)
				{
				prior = 0;
				seccur->post = 0;
				if ((seccur->distance < (0.5*section_length) )
					&& (seccur->prior == 1))
					{
					for (i=0;i<6;i++)
						seclst->etime_i[i] = 
							seccur->etime_i[i];
					seclst->distance = seclst->distance 
						+ seccur->distance;
					seclst->post = 0;
					seclst->nrec = seclst->nrec + seccur->nrec;
					free(seccur);
					}
				else if (seccur->distance == 0.0)
					{
					free(seccur);
					}
				else
					{
					nsectot++;
					files[ifil]->nsec++;
					}
				}
			else if (status < 0)
				{
				prior = 0;
				seccur->post = 0;
				if ((seccur->distance < (0.5*section_length) )
					&& (seccur->prior == 1))
					{
					for (i=0;i<6;i++)
						seclst->etime_i[i] = 
							seccur->etime_i[i];
					seclst->distance = seclst->distance 
						+ seccur->distance;
					seclst->nrec = seclst->nrec + seccur->nrec;
					free(seccur);
					}
                                else if (seccur->distance == 0.0)
                                        {
                                        free(seccur);
                                        }
				else
					{
					nsectot++;
					files[ifil]->nsec++;
					}
				while (status < 0)
					status = mb_read(&rbeams,&rpings,time_i,
						&time_d,&clon,&clat,&speed,
						&heading,&distance,&pitch,
						dep,lon,lat);
				}

			/* print out a mess for debugging */
			if (iverbose >= 2)
				{
				printf("global section: %4d  file: %3d  local section: %4d\n",
					(nsectot-1),ifil,(files[ifil]->nsec-1));
				printf("format: %1d  prior: %1d  post: %1d\n",
					seccur->format,seccur->prior,
					seccur->post);
				printf("btime: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d\n",
					seccur->btime_i[0],seccur->btime_i[1],
					seccur->btime_i[2],seccur->btime_i[3],
					seccur->btime_i[4],seccur->btime_i[5]);
				printf("etime: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d\n",
					seccur->etime_i[0],seccur->etime_i[1],
					seccur->etime_i[2],seccur->etime_i[3],
					seccur->etime_i[4],seccur->etime_i[5]);
				printf("records: %5d  distance: %10.5f\n",
					seccur->nrec,seccur->distance);
				}
			}

		/* close the files */
		status = mb_read_close();
		printf("%d sections found\n",files[ifil]->nsec);
		}

	/* process crossovers */
	for (i=0;i<dimtot;i++)
		{
		for (j=0;j<array[i].numsec;j++)
			for (k=j+1;k<array[i].numsec;k++)
				{
				if (nicross >= MAXCROSS)
					{
					printf("maximum number of crossovers exceeded: %d\n",nicross);
					exit(-1);
					}
				x1 = array[i].local[j];
				x2 = array[i].local[k];
				ifound = 0;
				for (m=0;m<nicross;m++)
					if ((x1 == icross[m]->cross1 && x2 == icross[m]->cross2)
						|| (x2 == icross[m]->cross1 && x1 == icross[m]->cross2))
						ifound = 1;
				if ((x1-x2) == 1 && sec[x1]->prior == 1)
					ifound = 1;
				if ((x2-x1) == 1 && sec[x2]->prior == 1)
					ifound = 1;
				if (ifound == 0)
					{
					if ((icross[nicross] = (struct cross *) 						calloc(1,cross_size)) == NULL) 
						exit(-1);
					icross[nicross]->cross1 = x1;
					icross[nicross]->cross2 = x2;
					icross[nicross]->flag = 0;
					nicross++;
					}
				}
		}
	printf("%d initial crossovers found\n",nicross);

	/* flag redundant crossovers */
	for (i=0;i<nicross;i++)
		{
		if (icross[i]->flag == 0)
			{
			/* set flag to save */
			icross[i]->flag = 1;

			/* set bounds for search of other crossovers */
			imn1 = icross[i]->cross1;
			imx1 = icross[i]->cross1;
			if (sec[icross[i]->cross1]->prior == 1) imn1--;
			if (sec[icross[i]->cross1]->post == 1) imx1++;
			imn2 = icross[i]->cross2;
			imx2 = icross[i]->cross2;
			if (sec[icross[i]->cross2]->prior == 1) imn2--;
			if (sec[icross[i]->cross2]->post == 1) imx2++;

			for (j=0;j<nicross;j++)
				{
				if (icross[j]->flag == 0 &&
					icross[j]->cross1 >= imn1 && 
					icross[j]->cross1 <= imx1 && 
					icross[j]->cross2 >= imn2 && 
					icross[j]->cross2 <= imx2) 
					{
					icross[j]->flag = -1;
					}
				}
			}
		}

	/* remove redundant crossovers */
	for (i=0;i<nicross;i++)
		if (icross[i]->flag == 1)
			{
			if ((fcross[nfcross] = (struct cross *) 		
				calloc(1,cross_size)) == NULL) 
				exit(-1);
			fcross[nfcross]->cross1 = icross[i]->cross1;
			fcross[nfcross]->cross2 = icross[i]->cross2;
			fcross[nfcross]->flag = 1;
			nfcross++;
			}
	printf("%d final crossovers found\n",nfcross);

	/* set output flags for sections to be output */
	for (i=0;i<nfcross;i++)
		{
		sec[fcross[i]->cross1]->output = 1;
		sec[fcross[i]->cross2]->output = 1;
		if (sec[fcross[i]->cross1]->prior == 1) 
			sec[fcross[i]->cross1-1]->output = 1;
		if (sec[fcross[i]->cross1]->post == 1) 
			sec[fcross[i]->cross1+1]->output = 1;
		if (sec[fcross[i]->cross2]->prior == 1) 
			sec[fcross[i]->cross2-1]->output = 1;
		if (sec[fcross[i]->cross2]->post == 1) 
			sec[fcross[i]->cross2+1]->output = 1;
		}

	/* print report on crossovers */
	if (iverbose >= 1)
		{
		printf("\nnumber of crossovers: %d\n",nfcross);
		for (i=0;i<nfcross;i++)
			printf("%4d  %4d  %4d\n",
				i,fcross[i]->cross1,fcross[i]->cross2);
		}

	/* print out a report of the sections */
	if (iverbose >= 1)
		{
		printf("\n\nnumber of files: %d   number of sections: %d\n",nfil,nsectot);
		k = 0;
		j = 0;
		for (ifil=0;ifil<nfil;ifil++)
			{
			printf("\n*** file %d: %s\n",ifil,files[ifil]->file);
			printf("*** sections: %d\n",files[ifil]->nsec);
			k = j;
			j = j + files[ifil]->nsec;
			for (isec=k;isec<j;isec++)
				{
				seccur = sec[isec];
				printf("\nglobal section: %4d  file: %3d  local section: %4d\n",
					isec,ifil,(isec-k));
				printf("format: %1d  prior: %1d  post: %1d\n",
					seccur->format,seccur->prior,
					seccur->post);
				printf("btime: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d\n",
					seccur->btime_i[0],seccur->btime_i[1],
					seccur->btime_i[2],seccur->btime_i[3],
					seccur->btime_i[4],seccur->btime_i[5]);
				printf("etime: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d\n",
					seccur->etime_i[0],seccur->etime_i[1],
					seccur->etime_i[2],seccur->etime_i[3],
					seccur->etime_i[4],seccur->etime_i[5]);
				printf("records: %5d  distance: %10.5f\n\n",
					seccur->nrec,seccur->distance);
				}
			}
		}

	/* print out cmd file to cut out needed sections */
	if ((fp = fopen(cmd1fil,"w")) == NULL)
		{
		printf("unable to open cmd file:%s\n",cmd1fil);
		exit(-1);
		}
	fprintf(fp,"# command file to cut sections from multibeam files\n\n");
	fprintf(fp,"mkdir sections\n");
	fprintf(fp,"rm -f sections/*\n");
	k = 0;
	j = 0;
	for (ifil=0;ifil<nfil;ifil++)
		{
		k = j;
		j = j + files[ifil]->nsec;
		for (isec=k;isec<j;isec++)
			{
			seccur = sec[isec];
			fprintf(fp,"# section: %d  file: %s\n",isec,files[ifil]->file);
			if (seccur->output == 0) fprintf(fp,"#");
			fprintf(fp,"mbcopy -F%1d/%1d -R%f/%f/%f/%f -P%d -S%f -L%1d -B%4.4d/%2.2d/%2.2d/%2.2d/%2.2d/%2.2d -E%4.4d/%2.2d/%2.2d/%2.2d/%2.2d/%2.2d -I%s -Osections/section%4.4d\n\n",
				seccur->format,seccur->format,bounds[0],
				bounds[1],bounds[2],bounds[3],pings,speedmin,
				lonflip,seccur->btime_i[0],seccur->btime_i[1],
				seccur->btime_i[2],seccur->btime_i[3],
				seccur->btime_i[4],seccur->btime_i[5],
				seccur->etime_i[0],seccur->etime_i[1],
				seccur->etime_i[2],seccur->etime_i[3],
				seccur->etime_i[4],seccur->etime_i[5],
				files[ifil]->file,isec);
			}
		}
	fclose(fp);
	status = chmod(cmd1fil,filemod);

	/* print out cmd file to run naverr */
	if ((fp = fopen(cmd2fil,"w")) == NULL)
		{
		printf("unable to open cmd file:%s\n",cmd2fil);
		exit(-1);
		}
	fprintf(fp,"# command file to obtain relative navigation errors\n");
	fprintf(fp,"# -  program naverr accesses the file section.list as\n");
	fprintf(fp,"#    well as the section???? files created by section.cmd\n\n");
	fprintf(fp,"naverr -I%s -N%s -$1 <<eot\n",datalist,navlist);
	for (i=0;i<nfcross;i++)
		fprintf(fp,"%4d  %4d\n",fcross[i]->cross1,fcross[i]->cross2);
	fprintf(fp,"eot\n");
	fclose(fp);
	status = chmod(cmd2fil,filemod);

	/* print out list of sections */
	if ((fp = fopen(outlist,"w")) == NULL)
		{
		printf("unable to open cmd file:%s\n",outlist);
		exit(-1);
		}
	fprintf(fp,"# list of sections\n\n");
	k = 0;
	j = 0;
	for (ifil=0;ifil<nfil;ifil++)
		{
		k = j;
		j = j + files[ifil]->nsec;
		for (isec=k;isec<j;isec++)
			{
			seccur = sec[isec];
			fprintf(fp,"global section: %4d  file: %3d  local section: %4d\n",
				isec,ifil,(isec-k));
			fprintf(fp,"file: %s\n",files[ifil]->file);
			fprintf(fp,"format: %1d  prior: %1d  post: %1d\n",
				seccur->format,seccur->prior,
				seccur->post);
			fprintf(fp,"btime: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d\n",
				seccur->btime_i[0],seccur->btime_i[1],
				seccur->btime_i[2],seccur->btime_i[3],
				seccur->btime_i[4],seccur->btime_i[5]);
			fprintf(fp,"etime: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d\n",
				seccur->etime_i[0],seccur->etime_i[1],
				seccur->etime_i[2],seccur->etime_i[3],
				seccur->etime_i[4],seccur->etime_i[5]);
			fprintf(fp,"records: %5d  distance: %10.5f\n\n",
				seccur->nrec,seccur->distance);
			}
		}
	fclose(fp); 

	/* end it all */
	exit(-1);
}
