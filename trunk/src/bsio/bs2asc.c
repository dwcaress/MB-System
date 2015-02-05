/*--------------------------------------------------------------------
 *    The MB-system:	mbbs2asc.c	3/3/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2015 by
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
 *	Copyright (c) 1991 by University of Hawaii.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>

#include "mbbs.h"

#define OUTPUT_ALL		(0)
#define OUTPUT_HDRSONLY		(1)
#define OUTPUT_DATAONLY		(2)

#define TM_JULIAN		(0)
#define TM_CALENDAR		(1)

#define SSLINESZ		(5)

#include <stdlib.h>

char *progname;
int skip, count;
int iomode;
int ignorecnt;
FILE *fp;
int output, tmmode;
int showivisping;
int showsns, showbty, showss;
int showbtyd, showbtyfl, showabi;
int showssd, showssfl;
int pngcntonly;
int smcid;
int slantrng;
BSFile bsf;
SMControl *smctl;
void *smbuf;
int *smind;
XDR xdri;
MemType *databuf;
unsigned int databufsz;
char *sdstr[ACP_NSIDES]= {
	"Port",
	"Starboard"
};

int
main(int argc, char **argv)
{
	char *cp;
	Ping png, *pngp;
	int err, i, j, side, npts, bsi;
	PingData pd;
	float *bty, *ss;
	AuxBeamInfo *abi;
	void usage();
	void pr_bsfhdr(BSFile *), pr_pnghdr(Ping *);

	if ((progname= strrchr(*argv, (int) '/')) != (char *) NULL)
		progname++;
	else
		progname= *argv;

	skip= 0;
	count= BS_UNDEFINED;
	iomode= BS_FILEIO;
	fp= stdout;
	output= OUTPUT_ALL;
	ignorecnt= 0;
	showivisping= 1;
	showsns= showbty= showss= 1;
	showbtyd= showbtyfl= showabi= 1;
	showssd= showssfl= 1;
	pngcntonly= 0;
	tmmode= TM_JULIAN;

	databuf= (MemType *) 0;
	databufsz= 0;

	for (argc--, argv++; argc > 0; argc--, argv++) {
		if (!strcmp(*argv, "-s")) {
			if (argc < 2)
				usage();
			argc--, argv++;
			skip= (int) strtol(*argv, &cp, 10);
			if ((cp == *argv) || (*cp != '\0')) {
				(void) fprintf(stderr, "%s: invalid skip value\n", progname);
				exit(BS_BADARG);
			}
			if (skip < 0) {
				(void) fprintf(stderr, "%s: skip value may not be less than 0\n", progname);
				exit(BS_BADARG);
			}
		}
		else if (!strcmp(*argv, "-c")) {
			if (argc < 2)
				usage();
			argc--, argv++;
			count= (int) strtol(*argv, &cp, 10);
			if ((cp == *argv) || (*cp != '\0')) {
				(void) fprintf(stderr, "%s: invalid count value\n", progname);
				exit(BS_BADARG);
			}
			if (count < 0) {
				(void) fprintf(stderr, "%s: count value may not be less than 0\n", progname);
				exit(BS_BADARG);
			}
		}
		else if (!strcmp(*argv, "-sm")) {
			if (argc < 2)
				usage();
			iomode= BS_SHAREDMEM;
			argc--, argv++;
			smcid= (int) strtol(*argv, &cp, 10);
			if ((cp == *argv) || (*cp != '\0')) {
				(void) fprintf(stderr, "%s: invalid shared memory control block ID\n", progname);
				exit(BS_BADARG);
			}
		}
		else if (!strcmp(*argv, "-f")) {
			if (argc < 2)
				usage();
			argc--, argv++;
			if ((fp= fopen(*argv, "w")) == (FILE *) NULL) {
				(void) fprintf(stderr, "%s: cannot open file '%s'\n", progname, *argv);
				exit(BS_OPEN);
			}
		}
		else if (!strcmp(*argv, "-h"))
			output= OUTPUT_HDRSONLY;
		else if (!strcmp(*argv, "-d"))
			output= OUTPUT_DATAONLY;
		else if (!strcmp(*argv, "-nip"))
			showivisping= 0;
		else if (!strcmp(*argv, "-nsns"))
			showsns= 0;
		else if (!strcmp(*argv, "-nb"))
			showbty= 0;
		else if (!strcmp(*argv, "-nbd"))
			showbtyd= 0;
		else if (!strcmp(*argv, "-nbf"))
			showbtyfl= 0;
		else if (!strcmp(*argv, "-nabi"))
			showabi= 0;
		else if (!strcmp(*argv, "-ns"))
			showss= 0;
		else if (!strcmp(*argv, "-nsd"))
			showssd= 0;
		else if (!strcmp(*argv, "-nsf"))
			showssfl= 0;
		else if (!strcmp(*argv, "-i"))
			ignorecnt= 1;
		else if (!strcmp(*argv, "-pco"))
			pngcntonly= 1;
		else if (!strcmp(*argv, "-jt"))
			tmmode= TM_JULIAN;
		else if (!strcmp(*argv, "-ct"))
			tmmode= TM_CALENDAR;
		else if (!strcmp(*argv, "-H"))
			usage();

		/* ignored options */
		else if (!strcmp(*argv, "-tv")) {
			if (argc < 3)
				usage();
			argc-= 2, argv+= 2;
		}
		else if (!strcmp(*argv, "-spm"))
			continue;
		else if (!strcmp(*argv, "-ppm"))
			continue;
		else if (!strncmp(*argv, "-pb", 3))
			continue;
		else if (!strncmp(*argv, "-pp", 3))
			continue;
		else if (!strncmp(*argv, "-ps", 3))
			continue;
	}

	switch (iomode) {
	case BS_FILEIO:
		xdrstdio_create(&xdri, stdin, XDR_DECODE);

		if ((err= mbbs_rdbsfhdr(&bsf, &xdri)) != BS_SUCCESS) {
			(void) fprintf(stderr, "%s: cannot read BS file header\n", progname);
			exit(err);
		}
		if (bsf.bsf_flags & BS_SSSLANTRNG)
			slantrng= 1;
		else
			slantrng= 0;
		if (pngcntonly) {
			(void) fprintf(fp, "%1d\n", bsf.bsf_count);
			(void) fflush(fp);
			exit(BS_SUCCESS);
		}
		if (ignorecnt == 1)
			;
		else if (count == BS_UNDEFINED) {
			if ((count= bsf.bsf_count-skip) < 0) {
				(void) fprintf(stderr, "%s: skip request exceeds number of available pings\n", progname);
				exit(BS_BADARG);
			}
		}
		else {
			if (skip+count > bsf.bsf_count) {
				(void) fprintf(stderr, "%s: skip and count requests exceed number of available pings\n", progname);
				exit(BS_BADARG);
			}
		}
		if ((err= mbbs_seekpng(skip, &xdri, bsf.bsf_version)) != BS_SUCCESS) {
			(void) fprintf(stderr, "%s: ping seek error\n", progname);
			exit(err);
		}
		if (output != OUTPUT_DATAONLY)
			pr_bsfhdr(&bsf);

		break;
	case BS_SHAREDMEM:
		if (pngcntonly) {
			(void) fprintf(fp, "%1d\n", smctl->sm_count);
			(void) fflush(fp);
			exit(BS_SUCCESS);
		}
		if (ignorecnt == 1) {
			(void) fprintf(stderr, "%s: count cannot be ignored in shared memory mode\n", progname);
			exit(BS_BADARG);
		}
		if ((smctl= (SMControl *) shmat(smcid, (MemType *) 0, (int) 0)) == (SMControl *) -1) {
			(void) fprintf(stderr, "%s: shared memory attach failure\n", progname);
			exit(BS_SYSVIPC);
		}
		if (smctl->sm_slantrng)
			slantrng= 1;
		else
			slantrng= 0;
		if (count == BS_UNDEFINED) {
			if ((count= smctl->sm_count-skip) < 0) {
				(void) fprintf(stderr, "%s: skip request exceeds number of available pings\n", progname);
				smctl->sm_status= BS_BADARG;
				exit(BS_BADARG);
			}
		}
		else if (skip+count > smctl->sm_count) {
			(void) fprintf(stderr, "%s: skip and count requests exceed number of available pings\n", progname);
			smctl->sm_status= BS_BADARG;
			exit(BS_BADARG);
		}
		if ((smind= (int *) shmat(smctl->sm_shmiid, (MemType *) 0, (int) 0)) == (int *) -1) {
			(void) fprintf(stderr, "%s: shared memory attach failure\n", progname);
			smctl->sm_status= BS_SYSVIPC;
			exit(BS_SYSVIPC);
		}
		if ((smbuf= (void *) shmat(smctl->sm_shmdid, (MemType *) 0, (int) 0)) == (void *) -1) {
			(void) fprintf(stderr, "%s: shared memory attach failure\n", progname);
			smctl->sm_status= BS_SYSVIPC;
			exit(BS_SYSVIPC);
		}
		(void) strcpy(smctl->sm_msg, "ASCII output");
		smctl->sm_msgtype= SMC_MSGALTPCT;
		cp= (char *) smbuf;
		cp+= smind[skip];
		pngp= (Ping *) cp;
		break;
	}

	for (i= 0; (i < count) || ((count == BS_UNDEFINED) && (ignorecnt == 1)); i++) {
		switch (iomode) {
		case BS_FILEIO:
			if ((err= mbbs_rdpnghdr(&png, &xdri, bsf.bsf_version)) != BS_SUCCESS) {
				(void) fprintf(stderr, "%s: cannot read header from ping %1d\n", progname, (int) (skip+i));
				exit(err);
			}
			if ((err= mbbs_pngrealloc(&png, &databuf, &databufsz)) != BS_SUCCESS) {
				(void) fprintf(stderr, "%s: memory allocation error for ping %1d\n", progname, (int) (skip+i));
				exit(err);
			}
			if ((err= mbbs_rdpngdata(&png, databuf, &xdri)) != BS_SUCCESS) {
				(void) fprintf(stderr, "%s: cannot read data from ping %1d\n", progname, (int) (skip+i));
				exit(err);
			}
			pngp= &png;
			break;
		case BS_SHAREDMEM:
			cp= (char *) pngp;
			cp+= sizeof(Ping);
			databuf= (float *) cp;
			break;
		}
		if ((err= mbbs_getpngdataptrs(pngp, databuf, &pd)) != BS_SUCCESS) {
			(void) fprintf(stderr, "%s: cannot get ping data pointers\n", progname);
			exit(err);
		}

		(void) fprintf(fp, "\n\nPing %1d ****\n\n", (int) (i+skip));
		if (output != OUTPUT_DATAONLY)
			pr_pnghdr(pngp);

		if ((output != OUTPUT_HDRSONLY) &&
		    (showivisping || mbbs_pngvisible(pngp->png_flags))) {
			if (showsns) {
				(void) fprintf(fp, "\nCompass Data:\n");
				for (j= 0; j < pngp->png_compass.sns_nsamps; j++) {
					if (mbbs_isnanf(pd.pd_compass[j]))
						(void) fprintf(fp, "   ?\n");
					else
						(void) fprintf(fp, "%4.2f\n", pd.pd_compass[j]);
				}

				(void) fprintf(fp, "\nDepth Data:\n");
				for (j= 0; j < pngp->png_depth.sns_nsamps; j++) {
					if (mbbs_isnanf(pd.pd_depth[j]))
						(void) fprintf(fp, "   ?\n");
					else
						(void) fprintf(fp, "%4.2f\n", pd.pd_depth[j]);
				}

				(void) fprintf(fp, "\nPitch Data:\n");
				for (j= 0; j < pngp->png_pitch.sns_nsamps; j++) {
					if (mbbs_isnanf(pd.pd_pitch[j]))
						(void) fprintf(fp, "   ?\n");
					else
						(void) fprintf(fp, "%4.2f\n", pd.pd_pitch[j]);
				}

				(void) fprintf(fp, "\nRoll Data:\n");
				for (j= 0; j < pngp->png_roll.sns_nsamps; j++) {
					if (mbbs_isnanf(pd.pd_roll[j]))
						(void) fprintf(fp, "   ?\n");
					else
						(void) fprintf(fp, "%4.2f\n", pd.pd_roll[j]);
				}
			}

			if (showbty) {
				if (pngp->png_flags & PNG_XYZ)
					bsi= 3;
				else
					bsi= 2;
				for (side= ACP_PORT; side < ACP_NSIDES; side++) {
					(void) fprintf(fp, "\n%s Bathymetry Data:\n[Index]        X        ", side == ACP_PORT ? "Port" : "Starboard");
					if (bsi == 3)
						(void) fprintf(fp, "Y        ");
					(void) fprintf(fp, "Z    ");
					if (showbtyfl)
						(void) fprintf(fp, "Flag    ");
					if ((pngp->png_flags & PNG_ABI) && showabi)
						(void) fprintf(fp, "Beam    SSAT0    SSAT1 ABIFlag");
					(void) fprintf(fp, "\n");
					if ((npts= pngp->png_sides[side].ps_btycount) == 0)
						continue;
					bty= pd.pd_bty[side];
					for (j= 0; j < npts; j++) {
						if (!showbtyd && pd.pd_btyflags[side][j])
							continue;
						(void) fprintf(fp, "[%5d]   %8.2f ", j, bty[bsi*j]);
						if (bsi == 3)
							(void) fprintf(fp, "%8.2f %8.2f", bty[(bsi*j)+1], bty[(bsi*j)+2]);
						else
							(void) fprintf(fp, "%8.2f", bty[(bsi*j)+1]);
						if (showbtyfl)
							(void) fprintf(fp, "%#6x  ", pd.pd_btyflags[side][j]);
						if ((pngp->png_flags & PNG_ABI) && showabi) {
							abi= &(pd.pd_abi[side][j]);
							(void) fprintf(fp, "%6d ", abi->abi_id);
							if (mbbs_isnanf(abi->abi_ssat0))
								(void) fprintf(fp, "       ? ");
							else
								(void) fprintf(fp, "%8.2f ", abi->abi_ssat0);
							if (mbbs_isnanf(abi->abi_ssat1))
								(void) fprintf(fp, "       ?  ");
							else
								(void) fprintf(fp, "%8.2f  ", abi->abi_ssat1);
							(void) fprintf(fp, "%#6x", abi->abi_flags);
						}
						(void) fprintf(fp, "\n");
					}
				}
			}

			if (showss) {
				for (side= ACP_PORT; side < ACP_NSIDES; side++) {
					(void) fprintf(fp, "\n%s Sidescan Data:\n[Index]      Intensity", side == ACP_PORT ? "Port" : "Starboard");
					if (showssfl)
						(void) fprintf(fp, "     Flag");
					(void) fprintf(fp, "\n");
					npts= pngp->png_sides[side].ps_sscount;
					ss= pd.pd_ss[side];
					for (j= 0; j < npts; j++) {
						if (!showssd && pd.pd_ssflags[side][j])
							continue;
						(void) fprintf(fp, "[%5d]   %12.2f", j, ss[j]);
						if (showssfl)
							(void) fprintf(fp, "   %#6x", (unsigned int) pd.pd_ssflags[side][j]);
						(void) fprintf(fp, "\n");
					}
				}
			}
		}

		(void) fprintf(fp, "\n");

		switch (iomode) {
		case BS_FILEIO:
			break;
		case BS_SHAREDMEM:
			smctl->sm_ping= skip+i;
			if (i < count-1) {
				cp= (char *) smbuf;
				cp+= smind[skip+i+1];
				pngp= (Ping *) cp;
			}
			break;
		}
	}

	switch (iomode) {
	case BS_FILEIO:
		break;
	case BS_SHAREDMEM:
		smctl->sm_status= BS_SUCCESS;
		smctl->sm_redraw= SMC_RDRNONE;
		break;
	}

	exit(BS_SUCCESS);
}

void
usage()
{
	(void) fprintf(stderr, "usage: %s [ -f filenm] [ -h | -d ] [ -nip ] [ -nsns ] [ -nb ] [ -nbd ] [ -nbf ] [ -nabi ] [ -ns ] [ -nsd ] [ -nsf ] [ -i ] [ -pco ] [ -jt | -ct ] [ -s skip ] [ -c count ] [ -H ] -sm shmcid | < bsfile > ASCIIfile\n", progname);
	exit(BS_BADARG);
}

void
pr_bsfhdr(BSFile *bsf)
{
	(void) fprintf(fp, "Format Version: ");
	switch (bsf->bsf_version) {
	case MR1_VERSION_1_0:
		(void) fprintf(fp, "MR1 1.0\n");
		break;
	case MR1_VERSION_2_0:
		(void) fprintf(fp, "MR1 2.0\n");
		break;
	case BS_VERSION_1_0:
		(void) fprintf(fp, "BS 1.0\n");
		break;
	case BS_VERSION_1_1:
		(void) fprintf(fp, "BS 1.1\n");
		break;
	case BS_VERSION_1_2:
		(void) fprintf(fp, "BS 1.2\n");
		break;
	case BS_VERSION_1_3:
		(void) fprintf(fp, "BS 1.3\n");
		break;
	case BS_VERSION_1_4:
		(void) fprintf(fp, "BS 1.4\n");
		break;
	default:
		(void) fprintf(fp, "Unknown\n");
		break;
	}
	(void) fprintf(fp, "Ping Count: %d\n", bsf->bsf_count);

	(void) fprintf(fp, "Flags: %#x", bsf->bsf_flags);
	if (bsf->bsf_flags) {
		(void) fprintf(fp, " (");
		if (bsf->bsf_flags & BS_SSSLANTRNG)
			(void) fprintf(fp, " SSSLANTRNG");
		if (bsf->bsf_flags & BS_MSCPINGDELRST)
			(void) fprintf(fp, " BS_MSCPINGDELRST");
		if (bsf->bsf_flags & BS_MSCNAVEDIT)
			(void) fprintf(fp, " BS_MSCNAVEDIT");
		if (bsf->bsf_flags & BS_MSCEDGETRIM)
			(void) fprintf(fp, " BS_MSCEDGETRIM");
		(void) fprintf(fp, " )");
	}
	(void) fprintf(fp, "\n");

	(void) fprintf(fp, "Acquisition Instrument: ");
	switch (bsf->bsf_inst) {
	case BS_INST_UNDEFINED:
		(void) fprintf(fp, "?\n");
		break;
	case BS_INST_MR1:
		(void) fprintf(fp, "MR1\n");
		break;
	case BS_INST_SEAMAPB:
		(void) fprintf(fp, "Seamap-B\n");
		break;
	case BS_INST_IMI30:
		(void) fprintf(fp, "IMI-30\n");
		break;
	case BS_INST_IMI12:
		(void) fprintf(fp, "IMI-12\n");
		break;
	case BS_INST_DSL120A:
		(void) fprintf(fp, "DSL-120A\n");
		break;
	case BS_INST_SEAMAPC:
		(void) fprintf(fp, "Seamap-C\n");
		break;
	case BS_INST_SCAMP:
		(void) fprintf(fp, "SCAMP\n");
		break;
	case BS_INST_EM120:
		(void) fprintf(fp, "K/S EM120\n");
		break;
	case BS_INST_EM1002:
		(void) fprintf(fp, "K/S EM1002\n");
		break;
	case BS_INST_EM300:
		(void) fprintf(fp, "K/S EM300\n");
		break;
	case BS_INST_EM3000:
		(void) fprintf(fp, "K/S EM3000\n");
		break;
	case BS_INST_EM3002:
		(void) fprintf(fp, "K/S EM3002\n");
		break;
	case BS_INST_EM3000D:
		(void) fprintf(fp, "K/S EM3000D\n");
		break;
	case BS_INST_EM3002D:
		(void) fprintf(fp, "K/S EM3002D\n");
		break;
	case BS_INST_EM2000:
		(void) fprintf(fp, "K/S EM2000\n");
		break;
	case BS_INST_EM122:
		(void) fprintf(fp, "K/S EM122\n");
		break;
	case BS_INST_EM302:
		(void) fprintf(fp, "K/S EM302\n");
		break;
	case BS_INST_EM710:
		(void) fprintf(fp, "K/S EM710\n");
		break;
	case BS_INST_SM2000:
		(void) fprintf(fp, "K/S SM2000\n");
		break;
	case BS_INST_RESON8101:
		(void) fprintf(fp, "Reson 8101\n");
		break;
	case BS_INST_RESON8111:
		(void) fprintf(fp, "Reson 8111\n");
		break;
	case BS_INST_RESON8124:
		(void) fprintf(fp, "Reson 8124\n");
		break;
	case BS_INST_RESON8125:
		(void) fprintf(fp, "Reson 8125\n");
		break;
	case BS_INST_RESON8150:
		(void) fprintf(fp, "Reson 8150\n");
		break;
	case BS_INST_RESON8160:
		(void) fprintf(fp, "Reson 8160\n");
		break;
	case BS_INST_AMS120:
		(void) fprintf(fp, "AMS-120\n");
		break;
	case BS_INST_REMUS:
		(void) fprintf(fp, "Remus\n");
		break;
	case BS_INST_KLEIN5000:
		(void) fprintf(fp, "Klein 5000\n");
		break;
	case BS_INST_SEABEAM2000:
		(void) fprintf(fp, "SeaBeam 2000\n");
		break;
	case BS_INST_SEABEAM2100:
		(void) fprintf(fp, "SeaBeam 2100\n");
		break;
	case BS_INST_SEABEAM3012:
		(void) fprintf(fp, "SeaBeam 3012\n");
		break;
	case BS_INST_SSI:
		(void) fprintf(fp, "SSI\n");
		break;
	case BS_INST_SAICLLS:
		(void) fprintf(fp, "SAIC Laser Line Scan\n");
		break;
	case BS_INST_EDGETECHSS:
		(void) fprintf(fp, "Edgetech Sidescan\n");
		break;
	case BS_INST_EDGETECHSSM:
		(void) fprintf(fp, "Edgetech Mid-Frequency Sidescan\n");
		break;
	case BS_INST_EDGETECHSSH:
		(void) fprintf(fp, "Edgetech High-Frequency Sidescan\n");
		break;
	case BS_INST_EDGETECHSB:
		(void) fprintf(fp, "Edgetech Subbottom\n");
		break;
	default:
		(void) fprintf(fp, "%1d (unrecognized)\n", bsf->bsf_inst);
		break;
	}

	(void) fprintf(fp, "Source Format: ");
	switch (bsf->bsf_srcformat) {
	case BS_SFMT_UNDEFINED:
		(void) fprintf(fp, "?\n");
		break;
	case BS_SFMT_MR1:
		(void) fprintf(fp, "MR1\n");
		break;
	case BS_SFMT_TTS:
		(void) fprintf(fp, "TTS\n");
		break;
	case BS_SFMT_GSF:
		(void) fprintf(fp, "GSF\n");
		break;
	case BS_SFMT_GSFDUAL:
		(void) fprintf(fp, "GSF Dual\n");
		break;
	case BS_SFMT_XTF:
		(void) fprintf(fp, "XTF\n");
		break;
	case BS_SFMT_SIMRADEM:
		(void) fprintf(fp, "Simrad EM\n");
		break;
	case BS_SFMT_SIMRADMPB:
		(void) fprintf(fp, "Simrad MPB\n");
		break;
	case BS_SFMT_OIC:
		(void) fprintf(fp, "OIC\n");
		break;
	case BS_SFMT_OICLLS:
		(void) fprintf(fp, "OIC Laser Line Scan\n");
		break;
	case BS_SFMT_MSTIFF:
		(void) fprintf(fp, "MSTIFF\n");
		break;
	case BS_SFMT_SIOSB2000:
		(void) fprintf(fp, "SIO SB2000\n");
		break;
	case BS_SFMT_SSIV21:
		(void) fprintf(fp, "SSI V21\n");
		break;
	case BS_SFMT_XSE:
		(void) fprintf(fp, "XSE\n");
		break;
	case BS_SFMT_JSF:
		(void) fprintf(fp, "JSF\n");
		break;
	default:
		(void) fprintf(fp, "%1d (unrecognized)\n", bsf->bsf_srcformat);
		break;
	}

	if (bsf->bsf_srcfilenm != (char *) NULL)
		(void) fprintf(fp, "Source File: %s\n", bsf->bsf_srcfilenm);
	else
		(void) fprintf(fp, "Source File: ?\n");

	if (bsf->bsf_log != (char *) NULL)
		(void) fprintf(fp, "Log: %s\n", bsf->bsf_log);
	else
		(void) fprintf(fp, "Log:\n");

	return;
}

void
pr_pnghdr(Ping *png)
{
	int side;
	PingSide *ps;
	char *ss;
	time_t t;
	struct tm *tmp;
	long msec;
	int mbbs_isnanf(float);
	int mbbs_isnand(double);

	(void) fprintf(fp, "Flags: %#x", png->png_flags);
	if (png->png_flags) {
		(void) fprintf(fp, " (");
		if (png->png_flags & PNG_XYZ)
			(void) fprintf(fp, " XYZ");
		if (png->png_flags & PNG_ABI)
			(void) fprintf(fp, " ABI");
		if (png->png_flags & PNG_BTYSSFLAGSABSENT)
			(void) fprintf(fp, " BTYSSFLAGSABSENT");
		if (png->png_flags & PNG_HIDE)
			(void) fprintf(fp, " HIDE");
		if (png->png_flags & PNG_LOWQUALITY)
			(void) fprintf(fp, " LOWQUALITY");
		if (png->png_flags & PNG_MSCHIDE)
			(void) fprintf(fp, " MSCHIDE");
		(void) fprintf(fp, " )");
	}
	(void) fprintf(fp, "\n");

	t= (time_t) png->png_tm.tv_sec;
	tmp= gmtime(&t);
	msec= (long) (png->png_tm.tv_usec/1000);
	switch (tmmode) {
	case TM_JULIAN:
		(void) fprintf(fp, "Time: %1d/%03d %02d:%02d:%02d.%03ld\n", (int) (tmp->tm_year+1900), (int) (tmp->tm_yday+1), tmp->tm_hour, tmp->tm_min, tmp->tm_sec, msec);
		break;
	case TM_CALENDAR:
		(void) fprintf(fp, "Time: %1d/%1d/%1d %02d:%02d:%02d.%03ld\n", (int) (tmp->tm_year+1900), (int) (tmp->tm_mon+1), tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec, msec);
		break;
	}

	if (!showivisping && !mbbs_pngvisible(png->png_flags))
		return;

	if (mbbs_isnanf(png->png_period))
		(void) fprintf(fp, "Ping Period: ?\n");
	else
		(void) fprintf(fp, "Ping Period: %4.2f\n", png->png_period);

	if (mbbs_isnand(png->png_slon))
		(void) fprintf(fp, "Ship Longitude: ?\n");
	else
		(void) fprintf(fp, "Ship Longitude: %12.10f\n", png->png_slon);
	if (mbbs_isnand(png->png_slat))
		(void) fprintf(fp, "Ship Latitude: ?\n");
	else
		(void) fprintf(fp, "Ship Latitude: %12.10f\n", png->png_slat);
	if (mbbs_isnanf(png->png_scourse))
		(void) fprintf(fp, "Ship Course: ?\n");
	else
		(void) fprintf(fp, "Ship Course: %4.2f\n", png->png_scourse);
	if (mbbs_isnanf(png->png_laybackrng))
		(void) fprintf(fp, "Layback Range: ?\n");
	else
		(void) fprintf(fp, "Layback Range: %4.2f\n", png->png_laybackrng);
	if (mbbs_isnanf(png->png_laybackbrg))
		(void) fprintf(fp, "Layback Bearing: ?\n");
	else
		(void) fprintf(fp, "Layback Bearing: %4.2f\n", png->png_laybackbrg);
	if (mbbs_isnand(png->png_tlon))
		(void) fprintf(fp, "Towfish Longitude: ?\n");
	else
		(void) fprintf(fp, "Towfish Longitude: %12.10f\n", png->png_tlon);
	if (mbbs_isnand(png->png_tlat))
		(void) fprintf(fp, "Towfish Latitude: ?\n");
	else
		(void) fprintf(fp, "Towfish Latitude: %12.10f\n", png->png_tlat);
	if (mbbs_isnanf(png->png_tcourse))
		(void) fprintf(fp, "Towfish Course: ?\n");
	else
		(void) fprintf(fp, "Towfish Course: %4.2f\n", png->png_tcourse);
	if (mbbs_isnanf(png->png_compass.sns_int))
		(void) fprintf(fp, "Compass Sample Interval: ?\n");
	else
		(void) fprintf(fp, "Compass Sample Interval: %5.3f\n", png->png_compass.sns_int);
	(void) fprintf(fp, "Compass Sample Count: %1d\n", png->png_compass.sns_nsamps);
	if (mbbs_isnanf(png->png_compass.sns_repval))
		(void) fprintf(fp, "Compass Representative Value: ?\n");
	else
		(void) fprintf(fp, "Compass Representative Value: %4.2f\n", png->png_compass.sns_repval);
	if (mbbs_isnanf(png->png_depth.sns_int))
		(void) fprintf(fp, "Depth Sample Interval: ?\n");
	else
		(void) fprintf(fp, "Depth Sample Interval: %5.3f\n", png->png_depth.sns_int);
	(void) fprintf(fp, "Depth Sample Count: %1d\n", png->png_depth.sns_nsamps);
	if (mbbs_isnanf(png->png_depth.sns_repval))
		(void) fprintf(fp, "Depth Representative Value: ?\n");
	else
		(void) fprintf(fp, "Depth Representative Value: %4.2f\n", png->png_depth.sns_repval);
	if (mbbs_isnanf(png->png_pitch.sns_int))
		(void) fprintf(fp, "Pitch Sample Interval: ?\n");
	else
		(void) fprintf(fp, "Pitch Sample Interval: %5.3f\n", png->png_pitch.sns_int);
	(void) fprintf(fp, "Pitch Sample Count: %1d\n", png->png_pitch.sns_nsamps);
	if (mbbs_isnanf(png->png_pitch.sns_repval))
		(void) fprintf(fp, "Pitch Representative Value: ?\n");
	else
		(void) fprintf(fp, "Pitch Representative Value: %4.2f\n", png->png_pitch.sns_repval);
	if (mbbs_isnanf(png->png_roll.sns_int))
		(void) fprintf(fp, "Roll Sample Interval: ?\n");
	else
		(void) fprintf(fp, "Roll Sample Interval: %5.3f\n", png->png_roll.sns_int);
	(void) fprintf(fp, "Roll Sample Count: %1d\n", png->png_roll.sns_nsamps);
	if (mbbs_isnanf(png->png_roll.sns_repval))
		(void) fprintf(fp, "Roll Representative Value: ?\n");
	else
		(void) fprintf(fp, "Roll Representative Value: %4.2f\n", png->png_roll.sns_repval);
	if (mbbs_isnanf(png->png_temp))
		(void) fprintf(fp, "Temperature: ?\n");
	else
		(void) fprintf(fp, "Temperature: %6.4f\n", png->png_temp);
	if (slantrng)
		(void) fprintf(fp, "Sidescan Increment: %7.5f\n", png->png_ssincr);
	else
		(void) fprintf(fp, "Sidescan Increment: %5.3f\n", png->png_ssincr);
	switch (png->png_ssyoffsetmode) {
	case PNG_SSYOM_UNKNOWN:
		(void) fprintf(fp, "Sidescan Along-Track Offset Mode: Unknown\n");
		break;
	case PNG_SSYOM_CONSTANT:
		(void) fprintf(fp, "Sidescan Along-Track Offset Mode: Constant\n");
		break;
	case PNG_SSYOM_USEBTYY:
		(void) fprintf(fp, "Sidescan Along-Track Offset Mode: Use Bathymetry Y-Offsets\n");
		break;
	}
	if (mbbs_isnanf(png->png_alt))
		(void) fprintf(fp, "Altitude: ?\n");
	else
		(void) fprintf(fp, "Altitude: %4.2f\n", png->png_alt);
	if (mbbs_isnanf(png->png_magcorr))
		(void) fprintf(fp, "Magnetic Correction: ?\n");
	else
		(void) fprintf(fp, "Magnetic Correction: %4.2f\n", png->png_magcorr);
	if (mbbs_isnanf(png->png_sndvel))
		(void) fprintf(fp, "Sound Velocity: ?\n");
	else
		(void) fprintf(fp, "Sound Velocity: %4.2f\n", png->png_sndvel);
	if (mbbs_isnanf(png->png_cond))
		(void) fprintf(fp, "Conductivity: ?\n");
	else
		(void) fprintf(fp, "Conductivity: %6.4f\n", png->png_cond);
	if (mbbs_isnanf(png->png_magx))
		(void) fprintf(fp, "Magnetic Field X: ?\n");
	else
		(void) fprintf(fp, "Magnetic Field X: %4.2f\n", png->png_magx);
	if (mbbs_isnanf(png->png_magy))
		(void) fprintf(fp, "Magnetic Field Y: ?\n");
	else
		(void) fprintf(fp, "Magnetic Field Y: %4.2f\n", png->png_magy);
	if (mbbs_isnanf(png->png_magz))
		(void) fprintf(fp, "Magnetic Field Z: ?\n");
	else
		(void) fprintf(fp, "Magnetic Field Z: %4.2f\n", png->png_magz);

	for (side= ACP_PORT; side < ACP_NSIDES; side++) {
		ps= &(png->png_sides[side]);
		ss= sdstr[side];
		if (mbbs_isnanf(ps->ps_xmitpwr))
			(void) fprintf(fp, "%s Transmit Power: ?\n", ss);
		else
			(void) fprintf(fp, "%s Transmit Power: %4.2f\n", ss, ps->ps_xmitpwr);
		if (mbbs_isnanf(ps->ps_gain))
			(void) fprintf(fp, "%s Gain: ?\n", ss);
		else
			(void) fprintf(fp, "%s Gain: %4.2f\n", ss, ps->ps_gain);
		if (mbbs_isnanf(ps->ps_pulse))
			(void) fprintf(fp, "%s Pulse Length: ?\n", ss);
		else
			(void) fprintf(fp, "%s Pulse Length: %4.2f\n", ss, ps->ps_pulse);
		if (mbbs_isnanf(ps->ps_bdrange))
			(void) fprintf(fp, "%s Bottom Detect Range: ?\n", ss);
		else
			(void) fprintf(fp, "%s Bottom Detect Range: %4.2f\n", ss, ps->ps_bdrange);
		(void) fprintf(fp, "%s Bathymetry Count: %1d\n", ss, ps->ps_btycount);
		if (slantrng)
			(void) fprintf(fp, "%s Sidescan Across-Track Offset: %7.5f\n", ss, ps->ps_ssxoffset);
		else
			(void) fprintf(fp, "%s Sidescan Across-Track Offset: %5.3f\n", ss, ps->ps_ssxoffset);
		(void) fprintf(fp, "%s Sidescan Count: %1d\n", ss, ps->ps_sscount);
		(void) fprintf(fp, "%s Sidescan Nadir Mask: %4.2f\n", ss, ps->ps_ssndrmask);
		if (mbbs_isnanf(ps->ps_ssyoffset))
			(void) fprintf(fp, "Sidescan Along-Track Offset: ?\n");
		else
			(void) fprintf(fp, "Sidescan Along-Track Offset: %4.2f\n", ps->ps_ssyoffset);
	}

	return;
}
