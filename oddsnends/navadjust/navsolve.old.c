/*--------------------------------------------------------------------------*/
/*
 *	navsolve -	part of the navadjust interactive multi-beam 
 *			bathymetry navigation adjustment package
 *
 *			navsolve reads the output from navcross and naverr
 *			and then sets up and solves the inverse problem for 
 *			navigation adjustment, outputting the results to
 *			be applied by navadjust.
 *
 *			David W. Caress
 *			Lamont-Doherty Geological Observatory
 *			October 1990
 */

/* standard global include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* define needed values */
#define PI 3.1415926
#define DTR PI/180.0
#define FILEMOD 493

/* global structure definitions */
struct section
	{
	double	btime_d;
	double	etime_d;
	double	distance;
	int	fileid;
	int	format;
	int	prior;
	int	post;
	int	btime_i[6];
	int	etime_i[6];
	int	output;
	int	nrec;
	};
struct navfix
	{
	double	clon;
	double	clat;
	double	time_d;
	double	azi;
	double	major;
	double	minor;
	char	type[4];
	int	section;
	int	time_i[6];
	int	id;	/* index in *tie */
	};
struct cross
	{
	double	time1_d;
	double	time2_d;
	double	xoff;
	double	yoff;
	double	azi;
	double	major;
	double	minor;
	int	section1;
	int	time1_i[6];
	int	section2;
	int	time2_i[6];
	int	id1;	/* index in *tie */
	int	id2;	/* index in *tie */
	};
struct tiepoint
	{
	double	time_d;
	int	type;	/* -1 = nav, 0 = 1st cross, 1 = 2nd cross */
	int	id;	/* index in *nav or *naverr */
	int	section;
	int	prior;
	int	post;
	};
struct array
	{
	double	a[2];
	};
struct index
	{
	int	a[2];
	};

/* global data array pointers */
int 	nsec;
struct section *sec;
int 	nnav;
struct navfix *nav;
int	nerr;
struct cross *naverr;
int	ntie;
struct tiepoint *tie;
int	nconnect, nconstraint;
struct array *arr;
struct index *iarr;
double	*data;
double	*lonsol, *latsol;
double	*lonwgt, *latwgt;

/* global filenames and file pointers */
char	datalist[128], navlist[128], sectionlist[128], naverrlist[128];
char	navsolvelist[128], navsolveout[128], cmdfile[128];
FILE	*fp1, *fp2;

/* global inversion parameters */
double	smx, err, sup, supt, slo, band, errlsq;
int	nnz, ncyceig, nrepeig, ncycle, ncyc, nsig;
double	*sigma, *swork, *work;
int	nfix, ifix;
double	fix;

main(argc, argv)
int argc;
char **argv; 
{
	int	i, j, k, l, m, n, status;
	int	ncon;
	double	a, angle, weight, dtime;
	double	xoff, yoff, xxoff, yyoff;
	double	tievar, delta;
	double	get_time();
	char	chartmp[128], navfile[128];
	void	sort_ties(), lspeig_(), chebyu_(), lsqup_();
	double	errlim_();

	/* initialize filenames and variables */
	strcpy(datalist,"data.list");
	strcpy(navlist,"nav.list");
	strcpy(sectionlist,"section.list");
	strcpy(naverrlist,"naverr.list");
	strcpy(navsolvelist,"navsolve.list");
	strcpy(navsolveout,"navsolve.out");
	strcpy(cmdfile,"navadjust.cmd");
	tievar = 4.0;
	delta = 60.0;
	nnz = 2;
	band = 10000.0;
	ncyceig = 16;
	nrepeig = 4;
	ncycle = 256;
	nfix = 0;

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
				case 'N':
				case 'n':
					strcpy (navlist, &argv[i][2]);
					break;
				case 'C':
				case 'c':
					sscanf (&argv[i][2], "%lf/%lf", 
						&tievar,&delta);
					break;
				case 'E':
				case 'e':
					sscanf (&argv[i][2], "%d/%d", 
						&nrepeig,&ncyceig);
					break;
				case 'B':
				case 'b':
					sscanf (&argv[i][2], "%d/%lf", 
						&ncycle,&band);
					break;
				}
			}
		}

	/* find the number of sections */
	if ((fp1 = fopen(sectionlist,"r")) == NULL)
		{
		printf("unable to open list file:%s\n",sectionlist);
		exit(-1);
		}
	if (fgets(chartmp,128,fp1) == NULL)
		{
		printf("error reading file:%s\n",sectionlist);
		exit(-1);
		}
	if (fgets(chartmp,128,fp1) == NULL)
		{
		printf("error reading file:%s\n",sectionlist);
		exit(-1);
		}
	nsec = 0;
	while (fgets(chartmp,128,fp1) != NULL)
		{
		sscanf(chartmp,"global section: %d file: %d local section: %d",
		&i,&j,&k);
		fgets(chartmp,128,fp1);
		fgets(chartmp,128,fp1);
		sscanf(chartmp,"format: %d prior: %d post: %d",&i,&j,&k);
		fgets(chartmp,128,fp1);
		sscanf(chartmp,"btime: %d %d %d %d:%d:%d",&i,&j,&k,&l,&m,&n);
		fgets(chartmp,128,fp1);
		sscanf(chartmp,"etime: %d %d %d %d:%d:%d",&i,&j,&k,&l,&m,&n);
		fgets(chartmp,128,fp1);
		sscanf(chartmp,"records: %d distance: %lf",&i,&a);
		fgets(chartmp,128,fp1);
		nsec++;
		}
	fclose(fp1);
	if ((sec = (struct section *) calloc(nsec,sizeof(struct section))) == NULL) exit(-1);

	/* read in the sections */
	if ((fp1 = fopen(sectionlist,"r")) == NULL)
		{
		printf("unable to open list file:%s\n",sectionlist);
		exit(-1);
		}
	if (fgets(chartmp,128,fp1) == NULL)
		{
		printf("error reading file:%s\n",sectionlist);
		exit(-1);
		}
	if (fgets(chartmp,128,fp1) == NULL)
		{
		printf("error reading file:%s\n",sectionlist);
		exit(-1);
		}
	for (i=0;i<nsec;i++)
		{
		fgets(chartmp,128,fp1);
		sscanf(chartmp,"global section: %d file: %d local section: %d",
		&j,&(sec[i].fileid),&k);
		fgets(chartmp,128,fp1);
		fgets(chartmp,128,fp1);
		sscanf(chartmp,"format: %d prior: %d post: %d",&(sec[i].format),&(sec[i].prior),&(sec[i].post));
		fgets(chartmp,128,fp1);
		sscanf(chartmp,"btime: %d %d %d %d:%d:%d",&(sec[i].btime_i[0]),&(sec[i].btime_i[1]),&(sec[i].btime_i[2]),&(sec[i].btime_i[3]),&(sec[i].btime_i[4]),&(sec[i].btime_i[5]));
		sec[i].btime_d = get_time(sec[i].btime_i);
		fgets(chartmp,128,fp1);
		sscanf(chartmp,"etime: %d %d %d %d:%d:%d",&(sec[i].etime_i[0]),&(sec[i].etime_i[1]),&(sec[i].etime_i[2]),&(sec[i].etime_i[3]),&(sec[i].etime_i[4]),&(sec[i].etime_i[5]));
		sec[i].etime_d = get_time(sec[i].etime_i);
		fgets(chartmp,128,fp1);
		sscanf(chartmp,"records: %d distance: %lf",&(sec[i].nrec),&(sec[i].distance));
		fgets(chartmp,128,fp1);
		}
	fclose(fp1);
	printf("%d sections read from %s\n",nsec,sectionlist);

	/* find the number of nav points */
	if ((fp1 = fopen(navlist,"r")) == NULL)
		{
		printf("unable to open nav list file:%s\n",navlist);
		exit(-1);
		}
	nnav = 0;
	while (fscanf(fp1,"%s",navfile) != EOF)
		{
		if ((fp2 = fopen(navfile,"r")) == NULL)
			{
			printf("unable to open nav file:%s\n",navfile);
			exit(-1);
			}
		while (fgets(chartmp,128,fp2) != NULL)
			nnav++;
		fclose(fp2);
		}
	fclose(fp1);
	if ((nav = (struct navfix *) calloc(nnav,sizeof(struct navfix))) 
		== NULL) exit(-1);

	/* read in nav points */
	if ((fp1 = fopen(navlist,"r")) == NULL)
		{
		printf("unable to open list file:%s\n",navlist);
		exit(-1);
		}
	nnav = 0;
	while (fscanf(fp1,"%s",navfile) != EOF)
		{
		if ((fp2 = fopen(navfile,"r")) == NULL)
			{
			printf("unable to open nav file:%s\n",navfile);
			exit(-1);
			}
		j = 0;
		while (fscanf(fp2,"%d %d %d %d %d %d %lf %lf %lf %lf %lf %s",
			&(nav[nnav].time_i[0]),&(nav[nnav].time_i[1]),
			&(nav[nnav].time_i[2]),&(nav[nnav].time_i[3]),
			&(nav[nnav].time_i[4]),&(nav[nnav].time_i[5]),
			&(nav[nnav].clon),&(nav[nnav].clat),&(nav[nnav].azi),
			&(nav[nnav].major),&(nav[nnav].minor),nav[nnav].type)
			!= EOF)
			{
			nav[nnav].time_d = get_time(nav[nnav].time_i);
			for (i=0;i<nsec;i++)
				{
				if (nav[nnav].time_d >= sec[i].btime_d 
					&& nav[nnav].time_d <= sec[i].etime_d)
					{
					nnav++;
					j++;
					nav[nnav].section = i;
					break;
					}
				}
			}
		printf("%d nav points read from %s\n",j,navfile);
		fclose(fp2);
		}
	fclose(fp1);
	printf("%d nav points in total\n",nnav);

	/* read in naverr.list */
	nerr = 0;
	if ((fp1 = fopen(naverrlist,"r")) == NULL)
		{
		printf("unable to open naverrlist file:%s\n",naverrlist);
		exit(-1);
		}
	while (fgets(chartmp,128,fp1) != NULL)
		nerr++;
	fclose(fp1);
	if ((naverr = (struct cross *) calloc(nerr,sizeof(struct cross))) 
		== NULL) exit(-1);

	/* read in crossing points */
	if ((fp1 = fopen(naverrlist,"r")) == NULL)
		{
		printf("unable to open list file:%s\n",naverrlist);
		exit(-1);
		}
	nerr = 0;
	while (fscanf(fp1,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %lf %lf %lf %lf %lf",
		&(naverr[nerr].section1),&(naverr[nerr].section2),
		&(naverr[nerr].time1_i[0]),&(naverr[nerr].time1_i[1]),
		&(naverr[nerr].time1_i[2]),&(naverr[nerr].time1_i[3]),
		&(naverr[nerr].time1_i[4]),&(naverr[nerr].time1_i[5]),
		&(naverr[nerr].time2_i[0]),&(naverr[nerr].time2_i[1]),
		&(naverr[nerr].time2_i[2]),&(naverr[nerr].time2_i[3]),
		&(naverr[nerr].time2_i[4]),&(naverr[nerr].time2_i[5]),
		&(naverr[nerr].xoff),&(naverr[nerr].yoff),
		&(naverr[nerr].azi),&(naverr[nerr].major),
		&(naverr[nerr].minor))
		!= EOF)
		{
		naverr[nerr].time1_d = get_time(naverr[nerr].time1_i);
		naverr[nerr].time2_d = get_time(naverr[nerr].time2_i);
		nerr++;
		}
	printf("%d crossing points read from %s\n",nerr,naverrlist);
	fclose(fp1);

	/* set up list of tie points */
	ntie = nnav + 2*nerr;
	if ((tie = (struct tiepoint *) calloc(ntie,sizeof(struct tiepoint))) 
		== NULL) exit(-1);
	ntie = 0;
	for (j=0;j<nnav;j++)
		{
		tie[ntie].type = -1;
		tie[ntie].id = j;
		tie[ntie].time_d = nav[j].time_d;
		ntie++;
		}
	for (j=0;j<nerr;j++)
		{
		tie[ntie].type = 0;
		tie[ntie].id = j;
		tie[ntie].time_d = naverr[j].time1_d;
		ntie++;
		tie[ntie].type = 1;
		tie[ntie].id = j;
		tie[ntie].time_d = naverr[j].time2_d;
		ntie++;
		}

	/* sort the tie points */
	sort_ties();

	/* find the sections of each tie point */
	for (i=0;i<ntie;i++)
		{
		for (j=0;j<nsec;j++)
			{
			if (tie[i].time_d >= sec[j].btime_d 
				&& tie[i].time_d <= sec[j].etime_d)
				{
				tie[i].section = j;
				break;
				}
			}
		}

	/* set the id's in *nav and *naverr from tie points */
	for (i=0;i<ntie;i++)
		{
		if (tie[i].type == -1)
			{
			nav[tie[i].id].id = i;
			nav[tie[i].id].section = tie[i].section;
			}
		else if (tie[i].type == 0)
			{
			naverr[tie[i].id].id1 = i;
			naverr[tie[i].id].section1 = tie[i].section;
			}
		else if (tie[i].type == 1)
			{
			naverr[tie[i].id].id2 = i;
			naverr[tie[i].id].section2 = tie[i].section;
			}
		}

	/* set the prior and post for each tie point */
	nconnect = 0;
	for (i=0;i<ntie;i++)
		{
		if (i == 0)
			{
			tie[i].prior = 0;
			if ((tie[i].section == tie[i+1].section) ||
				(sec[tie[i].section].post == 1))
				{
				tie[i].post = 1;
				nconnect++;
				}
			else
				tie[i].post = 0;
			}
		else if (i == (ntie - 1))
			{
			tie[i].prior = tie[i-1].post;
			tie[i].post = 0;
			}
		else
			{
			tie[i].prior = tie[i-1].post;
			if ((tie[i].section == tie[i+1].section) ||
				(sec[tie[i].section].post == 1))
				{
				tie[i].post = 1;
				nconnect++;
				}
			else
				tie[i].post = 0;
			}
		}

	/* open a file for inversion output */
	if ((fp1 = fopen(navsolveout,"w")) == NULL)
		{
		printf("unable to open output file:%s\n",navsolveout);
		exit(-1);
		}

	/* print out size of matrix problem */
	nconstraint = nnav + nerr + nconnect;
	fprintf(fp1,"NAVSOLVE results\n\n");
	fprintf(fp1,"number of nav fixes:           %5d\n",nnav);
	fprintf(fp1,"number of crossing points:     %5d\n",nerr);
	fprintf(fp1,"number of adjacent tie points: %5d\n",nconnect);
	fprintf(fp1,"total number of constraints:   %5d\n",nconstraint);
	fprintf(fp1,"total number of unknowns:      %5d\n\n",ntie);
	fprintf(fp1,"number of eigenvalue cycles:      %5d\n",ncyceig);
	fprintf(fp1,"number of eigenvalue repititions: %5d\n",nrepeig);
	fprintf(fp1,"number of inversion iterations:   %5d\n",ncycle);
	fprintf(fp1,"inversion bandwidth:              %f\n",band);

	/* allocate memory for arrays */
	if ((arr = (struct array *) calloc(nconstraint,sizeof(struct array))) 
		== NULL) exit(-1);
	if ((iarr = (struct index *) calloc(nconstraint,sizeof(struct index))) 
		== NULL) exit(-1);
	if ((data = (double *) calloc(nconstraint,sizeof(double))) 
		== NULL) exit(-1);
	if ((lonsol = (double *) calloc(ntie,sizeof(double))) 
		== NULL) exit(-1);
	if ((latsol = (double *) calloc(ntie,sizeof(double))) 
		== NULL) exit(-1);
	if ((lonwgt = (double *) calloc(nconstraint,sizeof(double))) 
		== NULL) exit(-1);
	if ((latwgt = (double *) calloc(nconstraint,sizeof(double))) 
		== NULL) exit(-1);

	/* allocate vectors for inversion */
	if ((sigma = (double *) calloc(ncycle,sizeof(double))) 
		== NULL) exit(-1);
	if ((swork = (double *) calloc(ncycle,sizeof(double))) 
		== NULL) exit(-1);
	if ((work = (double *) calloc(nconstraint,sizeof(double))) 
		== NULL) exit(-1);

	/* make the longitude matrix problem (use fortran style 1-n indexing) */
	printf("setting up the longitude inverse problem\n");
	ncon = 0;
	for (i=0;i<nnav;i++)
		{
		angle = nav[i].azi;
		if (angle > 180.0) angle = angle - 180.0;
		angle = DTR*angle;
		weight = nav[i].major*fabs(cos(angle)) 
			+ nav[i].minor*fabs(sin(angle));
		arr[ncon].a[0] = 1.0/weight;
		arr[ncon].a[1] = 0.0;
		iarr[ncon].a[0] = nav[i].id + 1;
		iarr[ncon].a[1] = 1;
		data[ncon] = 0.0;
		lonwgt[ncon] = weight;
		ncon++;
		}
	for (i=0;i<nerr;i++)
		{
		angle = naverr[i].azi;
		if (angle > 180.0) angle = angle - 180.0;
		angle = DTR*angle;
		weight = naverr[i].major*fabs(cos(angle)) 
			+ naverr[i].minor*fabs(sin(angle));
		arr[ncon].a[0] = -1.0/weight;
		arr[ncon].a[1] = 1.0/weight;
		iarr[ncon].a[0] = naverr[i].id1 + 1;
		iarr[ncon].a[1] = naverr[i].id2 + 1;
		data[ncon] = naverr[i].xoff/weight;
		lonwgt[ncon] = weight;
		ncon++;
		}
	for (i=0;i<ntie;i++)
		{
		if (tie[i].post == 1)
			{
			dtime = tie[i+1].time_d - tie[i].time_d;
			weight = tievar*exp(-dtime*dtime/(2*delta*delta));
			arr[ncon].a[0] = 1.0*weight;
			arr[ncon].a[1] = -1.0*weight;
			iarr[ncon].a[0] = i + 1;
			iarr[ncon].a[1] = i + 2;
			data[ncon] = 0.0;
			lonwgt[ncon] = 1./weight;
			ncon++;
			}
		}

	/* find upper bound on maximum eigenvalue */
	printf("finding upper bound on maximum eigenvalue\n");
	ncyc = 0;
	nsig = 0;
	lspeig_(arr,iarr,&nnz,&nnz,&ntie,&nconstraint,&ncyc,&nsig,
		lonsol,work,sigma,swork,&smx,&err,&sup);
	supt = ((smx + err) > sup) ? (smx + err) : sup;
	fprintf(fp1,"\nsmx:%f  err:%f  sup:%f  supt:%f\n",smx,err,sup,supt);
	for (i=0;i<nrepeig;i++)
		{
		ncyc = ncyceig;
		lspeig_(arr,iarr,&nnz,&nnz,&ntie,&nconstraint,&ncyc,&nsig,
			lonsol,work,sigma,swork,&smx,&err,&sup);
		supt = ((smx + err) > sup) ? (smx + err) : sup;
		fprintf(fp1,"smx:%f  err:%f  sup:%f  supt:%f\n",
			smx,err,sup,supt);
		}

	/* calculate chebyshev factors (errlsq is the theoretical error) */
	printf("finding chebyshev factors\n");
	slo = supt/band;
	fprintf(fp1,"\nsupt:%f  slo:%f\n",supt,slo);
	chebyu_(sigma,&ncycle,&supt,&slo,swork);
	errlsq = errlim_(sigma,&ncycle,&supt,&slo);
	fprintf(fp1,"theoretical error:%f\n",errlsq);

	/* solve the overdetermined least squares problem */
	printf("solving the inverse problem\n");
	for (i=0;i<ntie;i++)
		lonsol[i] = 0.0;
	lsqup_(arr,iarr,&nnz,&nnz,&ntie,&nconstraint,lonsol,
		work,data,&nfix,&ifix,&fix,&ncycle,sigma);

	/* make the latitude matrix problem (use fortran style 1-n indexing) */
	printf("setting up the latitude inverse problem\n");
	ncon = 0;
	for (i=0;i<nnav;i++)
		{
		angle = nav[i].azi;
		if (angle > 180.0) angle = angle - 180.0;
		angle = DTR*angle;
		weight = nav[i].major*fabs(sin(angle)) 
			+ nav[i].minor*fabs(cos(angle));
		arr[ncon].a[0] = 1.0/weight;
		arr[ncon].a[1] = 0.0;
		iarr[ncon].a[0] = nav[i].id + 1;
		iarr[ncon].a[1] = 1;
		data[ncon] = 0.0;
		latwgt[ncon] = weight;
		ncon++;
		}
	for (i=0;i<nerr;i++)
		{
		angle = naverr[i].azi;
		if (angle > 180.0) angle = angle - 180.0;
		angle = DTR*angle;
		weight = naverr[i].major*fabs(sin(angle)) 
			+ naverr[i].minor*fabs(cos(angle));
		arr[ncon].a[0] = -1.0/weight;
		arr[ncon].a[1] = 1.0/weight;
		iarr[ncon].a[0] = naverr[i].id1 + 1;
		iarr[ncon].a[1] = naverr[i].id2 + 1;
		data[ncon] = naverr[i].yoff/weight;
		latwgt[ncon] = weight;
		ncon++;
		}
	for (i=0;i<ntie;i++)
		{
		if (tie[i].post == 1)
			{
			dtime = tie[i+1].time_d - tie[i].time_d;
			weight = tievar*exp(-dtime*dtime/(2*delta*delta));
			arr[ncon].a[0] = 1.0*weight;
			arr[ncon].a[1] = -1.0*weight;
			iarr[ncon].a[0] = i + 1;
			iarr[ncon].a[1] = i + 2;
			data[ncon] = 0.0;
			latwgt[ncon] = 1./weight;
			ncon++;
			}
		}

	/* find upper bound on maximum eigenvalue */
	printf("finding upper bound on maximum eigenvalue\n");
	ncyc = 0;
	nsig = 0;
	lspeig_(arr,iarr,&nnz,&nnz,&ntie,&nconstraint,&ncyc,&nsig,
		latsol,work,sigma,swork,&smx,&err,&sup);
	supt = ((smx + err) > sup) ? (smx + err) : sup;
	fprintf(fp1,"\nsmx:%f  err:%f  sup:%f  supt:%f\n",smx,err,sup,supt);
	for (i=0;i<nrepeig;i++)
		{
		ncyc = ncyceig;
		lspeig_(arr,iarr,&nnz,&nnz,&ntie,&nconstraint,&ncyc,&nsig,
			latsol,work,sigma,swork,&smx,&err,&sup);
		supt = ((smx + err) > sup) ? (smx + err) : sup;
		fprintf(fp1,"smx:%f  err:%f  sup:%f  supt:%f\n",
			smx,err,sup,supt);
		}

	/* calculate chebyshev factors (errlsq is the theoretical error) */
	printf("finding chebyshev factors\n");
	slo = supt/band;
	fprintf(fp1,"\nsupt:%f  slo:%f\n",supt,slo);
	chebyu_(sigma,&ncycle,&supt,&slo,swork);
	errlsq = errlim_(sigma,&ncycle,&supt,&slo);
	fprintf(fp1,"theoretical error:%f\n",errlsq);

	/* solve the overdetermined least squares problem */
	printf("solving the inverse problem\n");
	for (i=0;i<ntie;i++)
		latsol[i] = 0.0;
	lsqup_(arr,iarr,&nnz,&nnz,&ntie,&nconstraint,latsol,
		work,data,&nfix,&ifix,&fix,&ncycle,sigma);

	/* output results to navsolveout file */
	fprintf(fp1,"\nadjustment solution:\n");
	for (i=0;i<ntie;i++)
		{
		j = tie[i].id;
		if (tie[i].type == -1)
			{
			fprintf(fp1,"%5d  %4d %2d %2d %2d %2d %2d %10.5f %10.5f\n",
				i,nav[j].time_i[0],nav[j].time_i[1],
				nav[j].time_i[2],nav[j].time_i[3],
				nav[j].time_i[4],nav[j].time_i[5],
				lonsol[i],latsol[i]);
			}
		else if (tie[i].type == 0)
			{
			fprintf(fp1,"%5d  %4d %2d %2d %2d %2d %2d %10.5f %10.5f\n",
				i,naverr[j].time1_i[0],naverr[j].time1_i[1],
				naverr[j].time1_i[2],naverr[j].time1_i[3],
				naverr[j].time1_i[4],naverr[j].time1_i[5],
				lonsol[i],latsol[i]);
			}
		else if (tie[i].type == 1)
			{
			fprintf(fp1,"%5d  %4d %2d %2d %2d %2d %2d %10.5f %10.5f\n",
				i,naverr[j].time2_i[0],naverr[j].time2_i[1],
				naverr[j].time2_i[2],naverr[j].time2_i[3],
				naverr[j].time2_i[4],naverr[j].time2_i[5],
				lonsol[i],latsol[i]);
			}
		}
	ncon = 0;
	fprintf(fp1,"\nnavigation fixes:\n");
	for (i=0;i<nnav;i++)
		{
		j = nav[i].id;
		fprintf(fp1,"%5d %5d   lon:%9.5f %9.5f   lat:%9.5f %9.5f\n",
			i,j,lonsol[j],lonwgt[ncon],
			lonsol[j],latwgt[ncon]);
		ncon++;
		}
	fprintf(fp1,"\ncrossing points:\n");
	for (i=0;i<nerr;i++)
		{
		j = naverr[i].id1;
		k = naverr[i].id2;
		xoff = lonsol[k] - lonsol[j];
		yoff = latsol[k] - latsol[j];
		xxoff = xoff - naverr[i].xoff;
		yyoff = yoff - naverr[i].yoff;
		fprintf(fp1,"%5d %5d %5d  lon:%9.5f %9.5f %9.5f %9.5f   lat:%9.5f %9.5f %9.5f %9.5f",
			i,j,k,naverr[i].xoff,xoff,xxoff,lonwgt[ncon],
			naverr[i].yoff,yoff,yyoff,latwgt[ncon]);
			if (fabs(xxoff) > lonwgt[ncon] || fabs(yyoff) > latwgt[ncon])
				fprintf(fp1,"  *****");
			fprintf(fp1,"\n");
		ncon++;
		}
	fprintf(fp1,"\nadjacent points:\n");
	for (i=0;i<ntie;i++)
		{
		if (tie[i].post == 1)
			{
			xoff = lonsol[i+1] - lonsol[i];
			yoff = latsol[i+1] - latsol[i];
			fprintf(fp1,"%5d %5d  lon:%9.5f %9.5f  lat:%9.5f %9.5f",
				i,i+1,xoff,lonwgt[ncon],yoff,latwgt[ncon]);
			if (fabs(xoff) > lonwgt[ncon] || fabs(yoff) > latwgt[ncon])
				fprintf(fp1,"  *****");
			fprintf(fp1,"\n");
			ncon++;
			}
		}
	fclose(fp1);

	/* output the solution navigation adjustments to file */
	if ((fp1 = fopen(navsolvelist,"w")) == NULL)
		{
		printf("unable to open matrix list file:%s\n",navsolvelist);
		exit(-1);
		}
	for (i=0;i<ntie;i++)
		{
		j = tie[i].id;
		if (tie[i].type == -1)
			{
			fprintf(fp1,"%4d  %4d %2d %2d %2d %2d %2d %10.5f %10.5f\n",
				nav[j].section,
				nav[j].time_i[0],nav[j].time_i[1],
				nav[j].time_i[2],nav[j].time_i[3],
				nav[j].time_i[4],nav[j].time_i[5],
				lonsol[i],latsol[i]);
			}
		else if (tie[i].type == 0)
			{
			fprintf(fp1,"%4d  %4d %2d %2d %2d %2d %2d %10.5f %10.5f\n",
				naverr[j].section1,
				naverr[j].time1_i[0],naverr[j].time1_i[1],
				naverr[j].time1_i[2],naverr[j].time1_i[3],
				naverr[j].time1_i[4],naverr[j].time1_i[5],
				lonsol[i],latsol[i]);
			}
		else if (tie[i].type == 1)
			{
			fprintf(fp1,"%4d  %4d %2d %2d %2d %2d %2d %10.5f %10.5f\n",
				naverr[j].section2,
				naverr[j].time2_i[0],naverr[j].time2_i[1],
				naverr[j].time2_i[2],naverr[j].time2_i[3],
				naverr[j].time2_i[4],naverr[j].time2_i[5],
				lonsol[i],latsol[i]);
			}
		}
	close(fp1);

	/* output the navadjust command file */
	if ((fp1 = fopen(cmdfile,"w")) == NULL)
		{
		printf("unable to open command file:%s\n",cmdfile);
		exit(-1);
		}
	fprintf(fp1,"# command file to adjust navigation\n");
	fprintf(fp1,"navadjust -I%s -A%s\n",datalist,navsolvelist);
	fclose(fp1);
	status = chmod(cmdfile,FILEMOD);

	/* end it all */
	exit(-1);
}

/*--------------------------------------------------------------------------*/
/* 	function sort_ties sorts the tie points into order of increasing
 *	time using the heapsort algorithm from numerical recipies */
void sort_ties()
{
	int	l, j, it, i;
	double	time_d;
	int	type, id;

	l = (ntie >> 1) + 1;
	it = ntie;
	for (;;)
		{
		if (l > 1)
			{
			--l;
			time_d = tie[l-1].time_d;
			type = tie[l-1].type;
			id = tie[l-1].id;
			}
		else
			{
			time_d = tie[it-1].time_d;
			type = tie[it-1].type;
			id = tie[it-1].id;
			tie[it-1].time_d = tie[0].time_d;
			tie[it-1].type = tie[0].type;
			tie[it-1].id = tie[0].id;
			if (--it == 1)
				{
				tie[0].time_d = time_d;
				tie[0].type = type;
				tie[0].id = id;
				return;
				}
			}
		i = l;
		j = l << 1;
		while (j <= it)
			{
			if (j < it && tie[j-1].time_d < tie[j].time_d) ++j;
			if (time_d < tie[j-1].time_d)
				{
				tie[i-1].time_d = tie[j-1].time_d;
				tie[i-1].type = tie[j-1].type;
				tie[i-1].id = tie[j-1].id;
				j += (i=j);
				}
			else
				j = it + 1;
			}
		tie[i-1].time_d = time_d;
		tie[i-1].type = type;
		tie[i-1].id = id;
		}
}
