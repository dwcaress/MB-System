/*--------------------------------------------------------------------------*/
/*
 *	navadjust -	part of the navadjust interactive multi-beam 
 *			bathymetry navigation adjustment package
 *
 *			navadjust reads the navigation adjustment results
 *			from navsolve and then applies them to adjust
 *			the navigation of the multibeam data.
 *
 *			David W. Caress
 *			Lamont-Doherty Geological Observatory
 *			October 1990
 */

/* standard global include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* definitions */
#define PI 3.1415926
#define DTR PI/180.
#define ZERO 0.0

/* global structure definitions */
struct section
	{
	int	fileid;
	int	format;
	int	prior;
	int	post;
	int	btime_i[6];
	int	etime_i[6];
	double	btime_d;
	double	etime_d;
	int	output;
	int	nrec;
	double	distance;
	};
struct mbfile
	{
	char	file[128];
	int	format;
	int	prior;
	int	post;
	int	nsec;
	};
struct navpoint
	{
	int	section;
	int	prior;
	int	post;
	int	time_i[6];
	double	time_d;
	double	dlon;
	double	dlat;
	};
struct navadj
	{
	int	prior;
	int	post;
	double	time_d;
	double	dlon;
	double	dlat;
	int	set;
	};

/* data array pointers */
int 	nsec;
struct section *sec;
int 	nnav;
struct navpoint *nav;
int	nfile;
struct mbfile *files;
int	nadj;
struct navadj *adj;

main(argc, argv)
int argc;
char **argv; 
{
	int	i, j, k, l, m, n, status, iadj;
	double	a, factor, deglontokm, deglattokm;
	double	get_time();
	void	km_scale(), sort();
	char	suffix[5];

	/* filenames and file pointers */
	char	datalist[128], sectionlist[128], navsolvelist[128];
	char	chartmp[128], navfile[128];
	FILE	*fp;

	/* mbio read and write initialization variables */
	char	ofile[128];
	int	read_pings, lonflip;
	double	bounds[4];
	int	btime_i[6], etime_i[6];
	double	btime_d, etime_d;
	double	speedmin, timegap;

	/* mbio read and write values */
	int	beams;
	int	pings;
	int	time_i[6];
	double	time_d;
	double	clon, clat;
	double	speed, heading, distance, pitch;
	int	irec, orec;
	int	*dep, *dis;

	/* initialize filenames and variables */
	strcpy(datalist,"data.list");
	strcpy(sectionlist,"section.list");
	strcpy(navsolvelist,"navsolve.list");

	/* inititalize mbio variables */
	read_pings = 1;
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

	/* process argument list */
	for (i = 1; i < argc; i++) 
		{
		if (argv[i][0] == '-') 
			{
			switch (argv[i][1]) 
				{
				case 'I':
				case 'i':
					strcpy (datalist, &argv[i][2]);
					break;
				}
			}
		}

	/* find the number of files */
	if ((fp = fopen(datalist,"r")) == NULL)
		{
		printf("unable to open list file:%s\n",datalist);
		exit(-1);
		}
	nfile = 0;
	while (fgets(chartmp,128,fp) != NULL)
		nfile++;
	fclose(fp);
	if ((files = (struct mbfile *) calloc(nfile,sizeof(struct mbfile))) == NULL) exit(-1);

	/* read in list of data files */
	if ((fp = fopen(datalist,"r")) == NULL)
		{
		printf("unable to open list file:%s\n",datalist);
		exit(-1);
		}
	nfile = 0;
	while (fscanf(fp,"%s %d %d",files[nfile].file,
		&files[nfile].format,&files[nfile].prior) != EOF)
		{
		if (nfile > 0) files[nfile-1].post = files[nfile].prior;
		if (nfile == 0) files[nfile].prior = 0;
		files[nfile].nsec = 0;
		if (files[nfile].format <= 0) 
			{
			printf("illegal format %d for file:%s\n",
				files[nfile].format,files[nfile].file);
			exit(-1);
			}
		nfile++;
		}
	fclose(fp);
	if (nfile > 0) files[nfile-1].post = 0;
	printf("%d data files\n",nfile);

	/* find the number of sections */
	if ((fp = fopen(sectionlist,"r")) == NULL)
		{
		printf("unable to open list file:%s\n",sectionlist);
		exit(-1);
		}
	if (fgets(chartmp,128,fp) == NULL)
		{
		printf("error reading file:%s\n",sectionlist);
		exit(-1);
		}
	if (fgets(chartmp,128,fp) == NULL)
		{
		printf("error reading file:%s\n",sectionlist);
		exit(-1);
		}
	nsec = 0;
	while (fgets(chartmp,128,fp) != NULL)
		{
		sscanf(chartmp,"global section: %d file: %d local section: %d",
		&i,&j,&k);
		fgets(chartmp,128,fp);
		fgets(chartmp,128,fp);
		sscanf(chartmp,"format: %d prior: %d post: %d",&i,&j,&k);
		fgets(chartmp,128,fp);
		sscanf(chartmp,"btime: %d %d %d %d:%d:%d",&i,&j,&k,&l,&m,&n);
		fgets(chartmp,128,fp);
		sscanf(chartmp,"etime: %d %d %d %d:%d:%d",&i,&j,&k,&l,&m,&n);
		fgets(chartmp,128,fp);
		sscanf(chartmp,"records: %d distance: %lf",&i,&a);
		fgets(chartmp,128,fp);
		nsec++;
		}
	fclose(fp);
	if ((sec = (struct section *) calloc(nsec,sizeof(struct section))) == NULL) exit(-1);

	/* read in the sections */
	if ((fp = fopen(sectionlist,"r")) == NULL)
		{
		printf("unable to open list file:%s\n",sectionlist);
		exit(-1);
		}
	if (fgets(chartmp,128,fp) == NULL)
		{
		printf("error reading file:%s\n",sectionlist);
		exit(-1);
		}
	if (fgets(chartmp,128,fp) == NULL)
		{
		printf("error reading file:%s\n",sectionlist);
		exit(-1);
		}
	for (i=0;i<nsec;i++)
		{
		fgets(chartmp,128,fp);
		sscanf(chartmp,"global section: %d file: %d local section: %d",
		&j,&(sec[i].fileid),&k);
		fgets(chartmp,128,fp);
		fgets(chartmp,128,fp);
		sscanf(chartmp,"format: %d prior: %d post: %d",&(sec[i].format),&(sec[i].prior),&(sec[i].post));
		fgets(chartmp,128,fp);
		sscanf(chartmp,"btime: %d %d %d %d:%d:%d",&(sec[i].btime_i[0]),&(sec[i].btime_i[1]),&(sec[i].btime_i[2]),&(sec[i].btime_i[3]),&(sec[i].btime_i[4]),&(sec[i].btime_i[5]));
		sec[i].btime_d = get_time(sec[i].btime_i);
		fgets(chartmp,128,fp);
		sscanf(chartmp,"etime: %d %d %d %d:%d:%d",&(sec[i].etime_i[0]),&(sec[i].etime_i[1]),&(sec[i].etime_i[2]),&(sec[i].etime_i[3]),&(sec[i].etime_i[4]),&(sec[i].etime_i[5]));
		sec[i].etime_d = get_time(sec[i].etime_i);
		fgets(chartmp,128,fp);
		sscanf(chartmp,"records: %d distance: %lf",&(sec[i].nrec),&(sec[i].distance));
		fgets(chartmp,128,fp);
		files[sec[i].fileid].nsec++;
		}
	fclose(fp);
	printf("%d sections read from %s\n",nsec,sectionlist);

	/* find the number of navpoint points */
	if ((fp = fopen(navsolvelist,"r")) == NULL)
		{
		printf("unable to open navadj list file:%s\n",navsolvelist);
		exit(-1);
		}
	nnav = 0;
	while (fgets(chartmp,128,fp) != NULL)
		nnav++;
	fclose(fp);
	if ((nav = (struct navpoint *) calloc(nnav,sizeof(struct navpoint))) 
		== NULL) exit(-1);

	/* read in nav points */
	if ((fp = fopen(navsolvelist,"r")) == NULL)
		{
		printf("unable to open navadj list file:%s\n",navsolvelist);
		exit(-1);
		}
	nnav = 0;
	while (fscanf(fp,"%d %d %d %d %d %d %d %lf %lf",
		&(nav[nnav].section),
		&(nav[nnav].time_i[0]),&(nav[nnav].time_i[1]),
		&(nav[nnav].time_i[2]),&(nav[nnav].time_i[3]),
		&(nav[nnav].time_i[4]),&(nav[nnav].time_i[5]),
		&(nav[nnav].dlon),&(nav[nnav].dlat))
		!= EOF)
		{
		nav[nnav].time_d = get_time(nav[nnav].time_i);
		nnav++;
		}
	fclose(fp);
	printf("%d nav adjustment points read from %s\n",nnav,navsolvelist);

	/* find total number for navigation adjustment list */
	nadj = nnav;
	for (i=0;i<nsec;i++)
		{
		nadj++;
		if (sec[i].post == 0)
			nadj++;
		}
	adj = (struct navadj *) calloc(nadj,sizeof(struct navadj));

	/* construct navigation adjustment list */
	nadj = 0;
	for (i=0;i<nsec;i++)
		{
		if (sec[i].prior == 0)
			{
			adj[nadj].prior = 0;
			adj[nadj].post = 1;
			adj[nadj].time_d = sec[i].btime_d;
			adj[nadj].set = 0;
			nadj++;
			}
		for (j=0;j<nnav;j++)
			if (nav[j].time_d >= sec[i].btime_d 
			&& nav[j].time_d < sec[i].etime_d)
				{
				adj[nadj].prior = 1;
				adj[nadj].post = 1;
				adj[nadj].time_d = nav[j].time_d;
				adj[nadj].set = 1;
				adj[nadj].dlon = nav[j].dlon;
				adj[nadj].dlat = nav[j].dlat;
				nadj++;
				}

		adj[nadj].prior = 1;
		adj[nadj].post = sec[i].post;
		adj[nadj].time_d = sec[i].etime_d;
		adj[nadj].set = 0;
		nadj++;
		}
	printf("%d total adjustment points\n\n",nadj);

	/* interpolate adjustments to section boundaries in list */
	for (i=0;i<nadj;i++)
		{
		if (adj[i].set == 0)
			{
			j = i - 1;
			while (j >= 0 && adj[j].set == 0 
				&& adj[j].post == 1)
				j--;
			k = i + 1;
			while (k < nadj && adj[k].set == 0 
				&& adj[k].prior == 1)
				k++;
			if ((adj[j].post == 0 || j < 0) 
				&& (adj[k].prior == 0 || k >= nadj))
				{
				adj[i].dlon = ZERO;
				adj[i].dlat = ZERO;
				}
			else if (adj[j].post == 0 || j < 0)
				{
				adj[i].dlon = adj[k].dlon;
				adj[i].dlat = adj[k].dlat;
				}
			else if (adj[k].prior == 0 || k >= nadj)
				{
				adj[i].dlon = adj[j].dlon;
				adj[i].dlat = adj[j].dlat;
				}
			else
				{
				factor = (adj[i].time_d - adj[j].time_d)
					/(adj[k].time_d - adj[j].time_d);
				adj[i].dlon = adj[j].dlon 
					+ (adj[k].dlon - adj[j].dlon)*factor;
				adj[i].dlat = adj[j].dlat 
					+ (adj[k].dlat - adj[j].dlat)*factor;
				}
			}
		}

	/* sort adjustments to get a continuous sequence in time */
	sort();

	/* loop over all of the files */
	for (i=0;i<nfile;i++)
		{

		/* allocate memory for reading data arrays */
		beams = mb_format(&files[i].format);
		if (i > 0)
			{
			free(dep);
			free(dis);
			}
		if ((dep = (int *) calloc(beams,sizeof(int))) == NULL) 
			exit(-1);
		if ((dis = (int *) calloc(beams,sizeof(int))) == NULL) 
			exit(-1);

		/* initialize multibeam files */
		strcpy(suffix,".adj");
		strcpy(ofile,files[i].file);
		strcat(ofile,suffix);
		printf("adjusting:   %s\n",files[i].file);
		printf("output file: %s\n",ofile);
		if ((status = mb_read_init(files[i].file,&(files[i].format),
			&read_pings,&lonflip,bounds,btime_i,etime_i,&btime_d,
			&etime_d,&speedmin,&timegap)) != 0)
			{
			printf("navadjust:  mbio read initialization error status: %d\n",status);
			exit(-1);
			}
		unlink(ofile);
		if ((status = mb_write_init(ofile,&(files[i].format))) != 0)
			{
			printf("navadjust:  mbio write initialization error status: %d\n",status);
			exit(-1);
			}

		/* read and write */
		irec = 0;
		orec = 0;
		iadj = 0;
		while (status <= 0)
			{
			/* read some data */
			if ((status = mb_get(&beams,&pings,time_i,&time_d,
				&clon,&clat,&speed,&heading,&distance,&pitch,
				dep,dis)) != 0 && status != -1 && status != 1)
				printf("navadjust:  mbio read error status: %d\n",status);
			if (status == 0 || status < -1) irec++;

			/* adjust the navigation */
			if (status == 0)
				{
				while (iadj < nadj-1 &&
					time_d > adj[iadj+1].time_d)
					iadj++;
				if (iadj < nadj-1
					&& time_d >= adj[iadj].time_d
					&& time_d <= adj[iadj+1].time_d
					&& (adj[iadj].prior == 1
					| adj[iadj].post == 1))
					{
					km_scale(clat,&deglontokm,&deglattokm);
					factor = (time_d - adj[iadj].time_d)
						/(adj[iadj+1].time_d 
						- adj[iadj].time_d);
					clon = clon + (adj[iadj].dlon + 
						(adj[iadj+1].dlon - 
						adj[iadj].dlon)*factor)
						/deglontokm;
					clat = clat + (adj[iadj].dlat + 
						(adj[iadj+1].dlat - 
						adj[iadj].dlat)*factor)
						/deglattokm;
					}
				else
					status = -1;
				}

			/* write some data */
			if (status == 0)
				{
				if (time_d > adj[iadj+1].time_d)
					iadj++;
				if ((status = mb_put(&beams,time_i,&time_d,
					&clon,&clat,&speed,
					&heading,&pitch,dep,dis)) != 0)
					printf("mbcopy:  mbio write error status: %d\n",status);
				else
					orec++;
				}
			}

		/* close the files */
		status = mb_read_close();
		status = mb_write_close();

		/* give the statistics */
		printf("%d input records\n%d output records\n\n",irec,orec);

		}

	/* end it all */
	exit(-1);
}
/*--------------------------------------------------------------------------*/
/* 	function km_scale returns scaling factors to turn lat and lon 
 *	differences in distance in km. 
 *	- formula based on world geodetic system ellipsoid of 1972.
 *	-  see bowditch (h.o. 9 -- american practical navigator) */
void km_scale(lat,deglontokm,deglattokm)
double lat,*deglontokm,*deglattokm;
#define C1 111412.84
#define C2 -93.5
#define C3 0.118
#define C4 111132.92
#define C5 -559.82
#define C6 1.175
#define C7 0.0023
{
	double avlat;
	avlat = DTR*lat;
	*deglontokm = 0.001*fabs(C1*cos(avlat) + C2*cos(3*avlat) 
			+ C3*cos(5*avlat));
	*deglattokm = 0.001*fabs(C4 + C5*cos(2*avlat) 
			+ C6*cos(4*avlat) + C7*cos(6*avlat));
	return;
}
/*--------------------------------------------------------------------------*/
/* 	function sort sorts the adjustment points into order of increasing
 *	time using the heapsort algorithm from numerical recipies */
void sort()
{
	int	l, j, it, i;
	int	prior;
	int	post;
	double	time_d;
	double	dlon;
	double	dlat;
	int	set;

	l = (nadj >> 1) + 1;
	it = nadj;
	for (;;)
		{
		if (l > 1)
			{
			--l;
			time_d = adj[l-1].time_d;
			prior = adj[l-1].prior;
			post = adj[l-1].post;
			dlon = adj[l-1].dlon;
			dlat = adj[l-1].dlat;
			set = adj[l-1].set;
			}
		else
			{
			time_d = adj[it-1].time_d;
			prior = adj[it-1].prior;
			post = adj[it-1].post;
			dlon = adj[it-1].dlon;
			dlat = adj[it-1].dlat;
			set = adj[it-1].set;
			adj[it-1].time_d = adj[0].time_d;
			adj[it-1].prior = adj[0].prior;
			adj[it-1].post = adj[0].post;
			adj[it-1].dlon = adj[0].dlon;
			adj[it-1].dlat = adj[0].dlat;
			adj[it-1].set = adj[0].set;
			if (--it == 1)
				{
				adj[0].time_d = time_d;
				adj[0].prior = prior;
				adj[0].post = post;
				adj[0].dlon = dlon;
				adj[0].dlat = dlat;
				adj[0].set = set;
				return;
				}
			}
		i = l;
		j = l << 1;
		while (j <= it)
			{
			if (j < it && adj[j-1].time_d < adj[j].time_d) ++j;
			if (time_d < adj[j-1].time_d)
				{
				adj[i-1].time_d = adj[j-1].time_d;
				adj[i-1].prior = adj[j-1].prior;
				adj[i-1].post = adj[j-1].post;
				adj[i-1].dlon = adj[j-1].dlon;
				adj[i-1].dlat = adj[j-1].dlat;
				adj[i-1].set = adj[j-1].set;
				j += (i=j);
				}
			else
				j = it + 1;
			}
		adj[i-1].time_d = time_d;
		adj[i-1].prior = prior;
		adj[i-1].post = post;
		adj[i-1].dlon = dlon;
		adj[i-1].dlat = dlat;
		adj[i-1].set = set;
		}
}
