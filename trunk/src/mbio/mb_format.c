/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.c	2/18/94
 *    $Id$
 *
 *    Copyright (c) 1993-2015 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_format.c contains several functions associated with getting
 * information about data formats.
 *
 * Author:	D. W. Caress
 * Date:	Februrary 18, 1994
 *
 */

/* standard include files */
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

/* mbio include files */
#include "mb_define.h"
#include "mb_status.h"
#include "mb_process.h"
#include "mb_io.h"
#include "mb_swap.h"
#include "mb_format.h"
#include "mbsys_simrad.h"
#include "mbsys_simrad2.h"
#include "mbsys_simrad3.h"
#include "mbsys_jstar.h"

/* Alias table for old (pre-version 4.0) format id's */
static int format_alias_table[] =
	{
	0,	/* NULL */
	11,	/* MBF_SBSIOMRG */
	12,	/* MBF_SBSIOCEN */
	13,	/* MBF_SBSIOLSI */
	14,	/* MBF_SBURICEN */
	21,	/* MBF_HSATLRAW */
	22,	/* MBF_HSLDEDMB */
	23,	/* MBF_HSURICEN */
	24,	/* MBF_HSLDEOIH */
	71,	/* MBF_MBLDEOIH */
	};

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mb_format_register(int verbose,
			int *format,
			void *mbio_ptr,
			int *error)
{
	char	*function_name = "mb_format_register";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:    %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:  %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       format:    %d\n",*format);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* check for old format id and provide alias if needed */
	if (*format > 0
	    && (*format < 10 || *format == 44
		|| *format == 52 || *format == 55))
	    {
	    /* replace original mbio id's */
	    if (*format < 10)
		i = format_alias_table[*format];

	    /* handle incorrectly identified SeaBeam 2120 data */
	    else if (*format == 44)
		i = MBF_L3XSERAW;

	    /* handle old Simrad EM12 and EM121 formats */
	    else if (*format == 52 || *format == 55)
		i = MBF_EMOLDRAW;
		
	    else
	        i = 0;
		
	    /* print output debug statements */
	    if (verbose >= 2)
		    {
		    fprintf(stderr,"\ndbg2  Old format id aliased to current value in MBIO function <%s>\n",
			    function_name);
		    fprintf(stderr,"dbg2  Old format value:\n");
		    fprintf(stderr,"dbg2       format:     %d\n",*format);
		    fprintf(stderr,"dbg2  Current format value:\n");
		    fprintf(stderr,"dbg2       format:     %d\n",i);
		    }

	    /* set new format value */
	    *format = i;
	    }

	/* set format value */
	mb_io_ptr->format = *format;

	/* look for a corresponding format */
	if (*format == MBF_SBSIOMRG)
		{
		status = mbr_register_sbsiomrg(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SBSIOCEN)
		{
		status = mbr_register_sbsiocen(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SBSIOLSI)
		{
		status = mbr_register_sbsiolsi(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SBURICEN)
		{
		status = mbr_register_sburicen(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SBURIVAX)
		{
		status = mbr_register_sburivax(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SBSIOSWB)
		{
		status = mbr_register_sbsioswb(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SBIFREMR)
		{
		status = mbr_register_sbifremr(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HSLDEDMB)
		{
		status = mbr_register_hsldedmb(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HSURICEN)
		{
		status = mbr_register_hsuricen(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HSATLRAW)
		{
		status = mbr_register_hsatlraw(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HSLDEOIH)
		{
		status = mbr_register_hsldeoih(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HSURIVAX)
		{
		status = mbr_register_hsurivax(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HSUNKNWN)
		{
		status = mbr_register_hsunknwn(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SB2000SB)
		{
		status = mbr_register_sb2000sb(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SB2000SS)
		{
		status = mbr_register_sb2000ss(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SB2100RW)
		{
		status = mbr_register_sb2100rw(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SB2100B1)
		{
		status = mbr_register_sb2100b1(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SB2100B2)
		{
		status = mbr_register_sb2100b2(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_EMOLDRAW)
		{
		status = mbr_register_emoldraw(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_EM12IFRM)
		{
		status = mbr_register_em12ifrm(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_EM12DARW)
		{
		status = mbr_register_em12darw(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_EM300RAW)
		{
		status = mbr_register_em300raw(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_EM300MBA)
		{
		status = mbr_register_em300mba(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_EM710RAW)
		{
		status = mbr_register_em710raw(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_EM710MBA)
		{
		status = mbr_register_em710mba(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MR1PRHIG)
		{
		status = mbr_register_mr1prhig(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MR1ALDEO)
		{
		status = mbr_register_mr1aldeo(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MR1BLDEO)
		{
		status = mbr_register_mr1bldeo(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MR1PRVR2)
		{
		status = mbr_register_mr1prvr2(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MBLDEOIH)
		{
		status = mbr_register_mbldeoih(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MBNETCDF)
		{
		status = mbr_register_mbnetcdf(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MBNCDFXT)
		{
		status = mbr_register_mbnetcdf(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_CBAT9001)
		{
		status = mbr_register_cbat9001(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_CBAT8101)
		{
		status = mbr_register_cbat8101(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HYPC8101)
		{
		status = mbr_register_hypc8101(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_XTFR8101)
		{
		status = mbr_register_xtfr8101(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_RESON7KR)
		{
		status = mbr_register_reson7kr(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_BCHRTUNB)
		{
		status = mbr_register_bchrtunb(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_ELMK2UNB)
		{
		status = mbr_register_elmk2unb(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_BCHRXUNB)
		{
		status = mbr_register_bchrxunb(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HSMDARAW)
		{
		status = mbr_register_hsmdaraw(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HSMDLDIH)
		{
		status = mbr_register_hsmdldih(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_DSL120PF)
		{
		status = mbr_register_dsl120pf(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_DSL120SF)
		{
		status = mbr_register_dsl120sf(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_GSFGENMB)
		{
		status = mbr_register_gsfgenmb(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MSTIFFSS)
		{
		status = mbr_register_mstiffss(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_EDGJSTAR)
		{
		status = mbr_register_edgjstar(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_EDGJSTR2)
		{
		status = mbr_register_edgjstr2(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_OICGEODA)
		{
		status = mbr_register_oicgeoda(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_OICMBARI)
		{
		status = mbr_register_oicmbari(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_OMGHDCSJ)
		{
		status = mbr_register_omghdcsj(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SEGYSEGY)
		{
		status = mbr_register_segysegy(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MGD77DAT)
		{
		status = mbr_register_mgd77dat(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_ASCIIXYZ)
		{
		status = mbr_register_asciixyz(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_ASCIIYXZ)
		{
		status = mbr_register_asciiyxz(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HYDROB93)
		{
		status = mbr_register_hydrob93(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HYDROB93)
		{
		status = mbr_register_hydrob93(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MBARIROV)
		{
		status = mbr_register_mbarirov(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MBARROV2)
		{
		status = mbr_register_mbarrov2(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_MBPRONAV)
		{
		status = mbr_register_mbpronav(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_NVNETCDF)
		{
		status = mbr_register_nvnetcdf(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_ASCIIXYT)
		{
		status = mbr_register_asciixyt(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_ASCIIYXT)
		{
		status = mbr_register_asciiyxt(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_L3XSERAW)
		{
		status = mbr_register_l3xseraw(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HS10JAMS)
		{
		status = mbr_register_hs10jams(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HSDS2RAW)
		{
		status = mbr_register_hsds2raw(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HSDS2LAM)
		{
		status = mbr_register_hsds2lam(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SAMESURF)
		{
		status = mbr_register_samesurf(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_IMAGE83P)
		{
		status = mbr_register_image83p(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_IMAGEMBA)
		{
		status = mbr_register_imagemba(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HIR2RNAV)
		{
		status = mbr_register_hir2rnav(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_HYSWEEP1)
		{
		status = mbr_register_hysweep1(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_XTFB1624)
		{
		status = mbr_register_xtfb1624(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SWPLSSXI)
		{
		status = mbr_register_swplssxi(verbose, mbio_ptr, error);
		}
	else if (*format == MBF_SWPLSSXP)
		{
		status = mbr_register_swplssxp(verbose, mbio_ptr, error);
		}
 	else if (*format == MBF_3DDEPTHP)
		{
		status = mbr_register_3ddepthp(verbose, mbio_ptr, error);
		}
 	else if (*format == MBF_WASSPENL)
		{
		status = mbr_register_wasspenl(verbose, mbio_ptr, error);
		}
 	else if (*format == MBF_MGD77TXT)
		{
		status = mbr_register_mgd77txt(verbose, mbio_ptr, error);
		}
 	else if (*format == MBF_MGD77TAB)
		{
		status = mbr_register_mgd77tab(verbose, mbio_ptr, error);
		}
 	else if (*format == MBF_PHOTGRAM)
		{
		status = mbr_register_photgram(verbose, mbio_ptr, error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:             %d\n",*format);
		fprintf(stderr,"dbg2       system:             %d\n",mb_io_ptr->system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",mb_io_ptr->beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",mb_io_ptr->beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",mb_io_ptr->pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",mb_io_ptr->format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",mb_io_ptr->system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",mb_io_ptr->format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",mb_io_ptr->numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",mb_io_ptr->filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",mb_io_ptr->variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",mb_io_ptr->traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",mb_io_ptr->beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",mb_io_ptr->nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",mb_io_ptr->vru_source);
		fprintf(stderr,"dbg2       svp_source:         %d\n",mb_io_ptr->svp_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr,"dbg2       format_alloc:       %p\n",(void *)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %p\n",(void *)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %p\n",(void *)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %p\n",(void *)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %p\n",(void *)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %p\n",(void *)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %p\n",(void *)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %p\n",(void *)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %p\n",(void *)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %p\n",(void *)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %p\n",(void *)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %p\n",(void *)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %p\n",(void *)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %p\n",(void *)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %p\n",(void *)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       detects:            %p\n",(void *)mb_io_ptr->mb_io_detects);
		fprintf(stderr,"dbg2       copyrecord:         %p\n",(void *)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:             %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_format_info(int verbose,
			int *format,
			int *system,
			int *beams_bath_max,
			int *beams_amp_max,
			int *pixels_ss_max,
			char *format_name,
			char *system_name,
			char *format_description,
			int *numfile,
			int *filetype,
			int *variable_beams,
			int *traveltime,
			int *beam_flagging,
			int *nav_source,
			int *heading_source,
			int *vru_source,
			int *svp_source,
			double *beamwidth_xtrack,
			double *beamwidth_ltrack,
			int *error)
{
	char	*function_name = "mb_format_info";
	int	status;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:    %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       format:    %d\n",*format);
		}

	/* check for old format id and provide alias if needed */
	if (*format > 0
	    && (*format < 10 || *format == 44
		|| *format == 52 || *format == 55))
	    {
	    /* replace original mbio id's */
	    if (*format < 10)
		i = format_alias_table[*format];

	    /* handle incorrectly identified SeaBeam 2120 data */
	    else if (*format == 44)
		i = MBF_L3XSERAW;

	    /* handle old Simrad EM12 and EM121 formats */
	    else if (*format == 52 || *format == 55)
		i = MBF_EMOLDRAW;
		
	    else
	        i = 0;

	    /* print output debug statements */
	    if (verbose >= 2)
		    {
		    fprintf(stderr,"\ndbg2  Old format id aliased to current value in MBIO function <%s>\n",
			    function_name);
		    fprintf(stderr,"dbg2  Old format value:\n");
		    fprintf(stderr,"dbg2       format:     %d\n",*format);
		    fprintf(stderr,"dbg2  Current format value:\n");
		    fprintf(stderr,"dbg2       format:     %d\n",i);
		    }

	    /* set new format value */
	    *format = i;
	    }

	/* look for a corresponding format */
	if (*format == MBF_SBSIOMRG)
		{
		status = mbr_info_sbsiomrg(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SBSIOCEN)
		{
		status = mbr_info_sbsiocen(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SBSIOLSI)
		{
		status = mbr_info_sbsiolsi(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SBURICEN)
		{
		status = mbr_info_sburicen(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SBURIVAX)
		{
		status = mbr_info_sburivax(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SBSIOSWB)
		{
		status = mbr_info_sbsioswb(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SBIFREMR)
		{
		status = mbr_info_sbifremr(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HSLDEDMB)
		{
		status = mbr_info_hsldedmb(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HSURICEN)
		{
		status = mbr_info_hsuricen(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HSATLRAW)
		{
		status = mbr_info_hsatlraw(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HSLDEOIH)
		{
		status = mbr_info_hsldeoih(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HSURIVAX)
		{
		status = mbr_info_hsurivax(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HSUNKNWN)
		{
		status = mbr_info_hsunknwn(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SB2000SB)
		{
		status = mbr_info_sb2000sb(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SB2000SS)
		{
		status = mbr_info_sb2000ss(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SB2100RW)
		{
		status = mbr_info_sb2100rw(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SB2100B1)
		{
		status = mbr_info_sb2100b1(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SB2100B2)
		{
		status = mbr_info_sb2100b2(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_EMOLDRAW)
		{
		status = mbr_info_emoldraw(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_EM12IFRM)
		{
		status = mbr_info_em12ifrm(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_EM12DARW)
		{
		status = mbr_info_em12darw(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_EM300RAW)
		{
		status = mbr_info_em300raw(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_EM300MBA)
		{
		status = mbr_info_em300mba(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_EM710RAW)
		{
		status = mbr_info_em710raw(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_EM710MBA)
		{
		status = mbr_info_em710mba(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MR1PRHIG)
		{
		status = mbr_info_mr1prhig(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MR1ALDEO)
		{
		status = mbr_info_mr1aldeo(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MR1BLDEO)
		{
		status = mbr_info_mr1bldeo(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MR1PRVR2)
		{
		status = mbr_info_mr1prvr2(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MBLDEOIH)
		{
		status = mbr_info_mbldeoih(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MBNETCDF)
		{
		status = mbr_info_mbnetcdf(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MBNCDFXT)
		{
		status = mbr_info_mbncdfxt(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_CBAT9001)
		{
		status = mbr_info_cbat9001(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_CBAT8101)
		{
		status = mbr_info_cbat8101(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HYPC8101)
		{
		status = mbr_info_hypc8101(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_XTFR8101)
		{
		status = mbr_info_xtfr8101(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_RESON7KR)
		{
		status = mbr_info_reson7kr(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_BCHRTUNB)
		{
		status = mbr_info_bchrtunb(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_ELMK2UNB)
		{
		status = mbr_info_elmk2unb(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_BCHRXUNB)
		{
		status = mbr_info_bchrxunb(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HSMDARAW)
		{
		status = mbr_info_hsmdaraw(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HSMDLDIH)
		{
		status = mbr_info_hsmdldih(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_DSL120PF)
		{
		status = mbr_info_dsl120pf(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_DSL120SF)
		{
		status = mbr_info_dsl120sf(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_GSFGENMB)
		{
		status = mbr_info_gsfgenmb(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MSTIFFSS)
		{
		status = mbr_info_mstiffss(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_EDGJSTAR)
		{
		status = mbr_info_edgjstar(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_EDGJSTR2)
		{
		status = mbr_info_edgjstr2(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_OICGEODA)
		{
		status = mbr_info_oicgeoda(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_OICMBARI)
		{
		status = mbr_info_oicmbari(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_OMGHDCSJ)
		{
		status = mbr_info_omghdcsj(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SEGYSEGY)
		{
		status = mbr_info_segysegy(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MGD77DAT)
		{
		status = mbr_info_mgd77dat(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_ASCIIXYZ)
		{
		status = mbr_info_asciixyz(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_ASCIIYXZ)
		{
		status = mbr_info_asciiyxz(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HYDROB93)
		{
		status = mbr_info_hydrob93(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MBARIROV)
		{
		status = mbr_info_mbarirov(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MBARROV2)
		{
		status = mbr_info_mbarrov2(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_MBPRONAV)
		{
		status = mbr_info_mbpronav(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_NVNETCDF)
		{
		status = mbr_info_nvnetcdf(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_ASCIIXYT)
		{
		status = mbr_info_asciixyt(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_ASCIIYXT)
		{
		status = mbr_info_asciiyxt(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_L3XSERAW)
		{
		status = mbr_info_l3xseraw(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HS10JAMS)
		{
		status = mbr_info_hs10jams(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HSDS2RAW)
		{
		status = mbr_info_hsds2raw(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HSDS2LAM)
		{
		status = mbr_info_hsds2lam(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SAMESURF)
		{
		status = mbr_info_samesurf(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_IMAGE83P)
		{
		status = mbr_info_image83p(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_IMAGEMBA)
		{
		status = mbr_info_imagemba(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HIR2RNAV)
		{
		status = mbr_info_hir2rnav(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_HYSWEEP1)
		{
		status = mbr_info_hysweep1(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_XTFB1624)
		{
		status = mbr_info_xtfb1624(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SWPLSSXI)
		{
		status = mbr_info_swplssxi(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
	else if (*format == MBF_SWPLSSXP)
		{
		status = mbr_info_swplssxp(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
		}
 	else if (*format == MBF_3DDEPTHP)
		{
		status = mbr_info_3ddepthp(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error); 
		}
 	else if (*format == MBF_WASSPENL)
		{
		status = mbr_info_wasspenl(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error); 
		}
 	else if (*format == MBF_MGD77TXT)
		{
		status = mbr_info_mgd77txt(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error); 
		}
 	else if (*format == MBF_MGD77TAB)
		{
		status = mbr_info_mgd77tab(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error); 
		}
 	else if (*format == MBF_PHOTGRAM)
		{
		status = mbr_info_photgram(verbose, system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			numfile, filetype,
			variable_beams, traveltime, beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error); 
		}
	else if (*format == MBF_DATALIST)
		{
		*format = MBF_DATALIST;
		*system = MB_SYS_NONE;
		*beams_bath_max = 0;
		*beams_amp_max = 0;
		*pixels_ss_max = 0;
		strcpy(format_name,"MBF_DATALIST");
		strcpy(system_name,"MB_SYS_DATALIST");
		strcpy(format_description,"MBF_DATALIST");
		strncpy(format_description, "Format name:          MBF_DATALIST\nInformal Description: Datalist\nAttributes:           List of swath data files, each filename \n\tfollowed by MB-System format id.\n", MB_DESCRIPTION_LENGTH);
		*numfile = 0;
		*filetype = 0;
		*variable_beams = MB_NO;
		*traveltime = MB_NO;
		*beam_flagging = MB_NO;
		*nav_source = MB_DATA_NONE;
		*heading_source = MB_DATA_NONE;
		*vru_source = MB_DATA_NONE;
		*svp_source = MB_DATA_NONE;
		*beamwidth_xtrack = 0.0;
		*beamwidth_ltrack = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
		}
	else
		{
		*format = MBF_NONE;
		*system = MB_SYS_NONE;
		*beams_bath_max = 0;
		*beams_amp_max = 0;
		*pixels_ss_max = 0;
		format_name[0] = '\0';
		system_name[0] = '\0';
		format_description[0] = '\0';
		*numfile = 0;
		*filetype = 0;
		*variable_beams = MB_NO;
		*traveltime = MB_NO;
		*beam_flagging = MB_NO;
		*nav_source = MB_DATA_NONE;
		*heading_source = MB_DATA_NONE;
		*vru_source = MB_DATA_NONE;
		*svp_source = MB_DATA_NONE;
		*beamwidth_xtrack = 0.0;
		*beamwidth_ltrack = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:             %d\n",*format);
		fprintf(stderr,"dbg2       system:             %d\n",*system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",*beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",*beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",*pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",*numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",*filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",*variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",*traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",*beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",*nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",*vru_source);
		fprintf(stderr,"dbg2       svp_source:         %d\n",*svp_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",*beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",*beamwidth_ltrack);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:             %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_format(int verbose, int *format, int *error)
{
	char	*function_name = "mb_format";
	int	status;

	/* format parameters */
	int	system;		/* system id */
	int	beams_bath_max;	/* maximum number of bathymetry beams */
	int	beams_amp_max;	/* maximum number of amplitude beams
					- either 0 or = beams_bath */
	int	pixels_ss_max;	/* maximum number of sidescan pixels */
	char	format_name[MB_NAME_LENGTH];
	char	system_name[MB_NAME_LENGTH];
	char	format_description[MB_DESCRIPTION_LENGTH];
	int	numfile;	/* the number of parallel files required for i/o */
	int	filetype;	/* type of files used (normal, xdr, or gsf) */
	int	variable_beams; /* if true then number of beams variable */
	int	traveltime;	/* if true then traveltime and angle data supported */
	int	beam_flagging;	/* if true then beam flagging supported */
	int	nav_source;	/* data record types containing the primary navigation */
	int	heading_source;	/* data record types containing the primary heading */
	int	vru_source;	/* data record types containing the primary vru */
	int	svp_source;	/* data record types containing the primary svp */
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system,
			&beams_bath_max, &beams_amp_max, &pixels_ss_max,
			format_name, system_name, format_description,
			&numfile, &filetype,
			&variable_beams, &traveltime, &beam_flagging,
			&nav_source, &heading_source, &vru_source, &svp_source,
			&beamwidth_xtrack, &beamwidth_ltrack,
			error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_format_system(int verbose, int *format, int *system, int *error)
{
  static char rcs_id[]="$Id$";
	char	*function_name = "mb_format_system";
	int	status;

	/* format parameters */
	int	beams_bath_max;	/* maximum number of bathymetry beams */
	int	beams_amp_max;	/* maximum number of amplitude beams
					- either 0 or = beams_bath */
	int	pixels_ss_max;	/* maximum number of sidescan pixels */
	char	format_name[MB_NAME_LENGTH];
	char	system_name[MB_NAME_LENGTH];
	char	format_description[MB_DESCRIPTION_LENGTH];
	int	numfile;	/* the number of parallel files required for i/o */
	int	filetype;	/* type of files used (normal, xdr, or gsf) */
	int	variable_beams; /* if true then number of beams variable */
	int	traveltime;	/* if true then traveltime and angle data supported */
	int	beam_flagging;	/* if true then beam flagging supported */
	int	nav_source;	/* data record types containing the primary navigation */
	int	heading_source;	/* data record types containing the primary heading */
	int	vru_source;	/* data record types containing the primary vru */
	int	svp_source;	/* data record types containing the primary svp */
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, system,
			&beams_bath_max, &beams_amp_max, &pixels_ss_max,
			format_name, system_name, format_description,
			&numfile, &filetype,
			&variable_beams, &traveltime, &beam_flagging,
			&nav_source, &heading_source, &vru_source, &svp_source,
			&beamwidth_xtrack, &beamwidth_ltrack,
			error);
	if (status == MB_FAILURE)
		{
		*system = MB_SYS_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:      %d\n",*format);
		fprintf(stderr,"dbg2       system:      %d\n",*system);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_format_dimensions(int verbose, int *format,
		int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max,
		int *error)
{
  static char rcs_id[]="$Id$";
	char	*function_name = "mb_format_dimensions";
	int	status;

	/* format parameters */
	int	system;		/* sonar system id */
	char	format_name[MB_NAME_LENGTH];
	char	system_name[MB_NAME_LENGTH];
	char	format_description[MB_DESCRIPTION_LENGTH];
	int	numfile;	/* the number of parallel files required for i/o */
	int	filetype;	/* type of files used (normal, xdr, or gsf) */
	int	variable_beams; /* if true then number of beams variable */
	int	traveltime;	/* if true then traveltime and angle data supported */
	int	beam_flagging;	/* if true then beam flagging supported */
	int	nav_source;	/* data record types containing the primary navigation */
	int	heading_source;	/* data record types containing the primary heading */
	int	vru_source;	/* data record types containing the primary vru */
	int	svp_source;	/* data record types containing the primary svp */
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system,
			beams_bath_max, beams_amp_max, pixels_ss_max,
			format_name, system_name, format_description,
			&numfile, &filetype,
			&variable_beams, &traveltime, &beam_flagging,
			&nav_source, &heading_source, &vru_source, &svp_source,
			&beamwidth_xtrack, &beamwidth_ltrack,
			error);
	if (status == MB_FAILURE)
		{
		*beams_bath_max = 0;
		*beams_amp_max = 0;
		*pixels_ss_max = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:         %d\n",*format);
		fprintf(stderr,"dbg2       beams_bath_max: %d\n",*beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:  %d\n",*beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:  %d\n",*pixels_ss_max);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		fprintf(stderr,"dbg2       error:          %d\n",*error);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_format_description(int verbose, int *format, char *description, int *error)
{
  static char rcs_id[]="$Id$";
	char	*function_name = "mb_format_description";
	int	status;

	/* format parameters */
	int	system;		/* sonar system id */
	int	beams_bath_max;	/* maximum number of bathymetry beams */
	int	beams_amp_max;	/* maximum number of amplitude beams
					- either 0 or = beams_bath */
	int	pixels_ss_max;	/* maximum number of sidescan pixels */
	char	format_name[MB_NAME_LENGTH];
	char	system_name[MB_NAME_LENGTH];
	int	numfile;	/* the number of parallel files required for i/o */
	int	filetype;	/* type of files used (normal, xdr, or gsf) */
	int	variable_beams; /* if true then number of beams variable */
	int	traveltime;	/* if true then traveltime and angle data supported */
	int	beam_flagging;	/* if true then beam flagging supported */
	int	nav_source;	/* data record types containing the primary navigation */
	int	heading_source;	/* data record types containing the primary heading */
	int	vru_source;	/* data record types containing the primary vru */
	int	svp_source;	/* data record types containing the primary svp */
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system,
			&beams_bath_max, &beams_amp_max, &pixels_ss_max,
			format_name, system_name, description,
			&numfile, &filetype,
			&variable_beams, &traveltime, &beam_flagging,
			&nav_source, &heading_source, &vru_source, &svp_source,
			&beamwidth_xtrack, &beamwidth_ltrack,
			error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:      %d\n",*format);
		fprintf(stderr,"dbg2       description: %s\n",description);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_format_flags(int verbose, int *format,
		int *variable_beams, int *traveltime, int *beam_flagging,
		int *error)
{
  static char rcs_id[]="$Id$";
	char	*function_name = "mb_format_flags";
	int	status;

	/* format parameters */
	int	system;		/* sonar system id */
	int	beams_bath_max;	/* maximum number of bathymetry beams */
	int	beams_amp_max;	/* maximum number of amplitude beams
					- either 0 or = beams_bath */
	int	pixels_ss_max;	/* maximum number of sidescan pixels */
	char	format_name[MB_NAME_LENGTH];
	char	system_name[MB_NAME_LENGTH];
	char	format_description[MB_DESCRIPTION_LENGTH];
	int	numfile;	/* the number of parallel files required for i/o */
	int	filetype;	/* type of files used (normal, xdr, or gsf) */
	int	nav_source;	/* data record types containing the primary navigation */
	int	heading_source;	/* data record types containing the primary heading */
	int	vru_source;	/* data record types containing the primary vru */
	int	svp_source;	/* data record types containing the primary svp */
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system,
			&beams_bath_max, &beams_amp_max, &pixels_ss_max,
			format_name, system_name, format_description,
			&numfile, &filetype,
			variable_beams, traveltime, beam_flagging,
			&nav_source, &heading_source, &vru_source, &svp_source,
			&beamwidth_xtrack, &beamwidth_ltrack,
			error);
	if (status == MB_FAILURE)
		{
		*variable_beams = MB_NO;
		*traveltime = MB_NO;
		*beam_flagging = MB_NO;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:         %d\n",*format);
		fprintf(stderr,"dbg2       variable_beams: %d\n",*variable_beams);
		fprintf(stderr,"dbg2       traveltime:     %d\n",*traveltime);
		fprintf(stderr,"dbg2       beam_flagging:  %d\n",*beam_flagging);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		fprintf(stderr,"dbg2       error:          %d\n",*error);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_format_source(int verbose, int *format,
		int *nav_source, int *heading_source,
		int *vru_source, int *svp_source,
		int *error)
{
  static char rcs_id[]="$Id$";
	char	*function_name = "mb_format_source";
	int	status;

	/* format parameters */
	int	system;		/* sonar system id */
	int	beams_bath_max;	/* maximum number of bathymetry beams */
	int	beams_amp_max;	/* maximum number of amplitude beams
					- either 0 or = beams_bath */
	int	pixels_ss_max;	/* maximum number of sidescan pixels */
	char	format_name[MB_NAME_LENGTH];
	char	system_name[MB_NAME_LENGTH];
	char	format_description[MB_DESCRIPTION_LENGTH];
	int	numfile;	/* the number of parallel files required for i/o */
	int	filetype;	/* type of files used (normal, xdr, or gsf) */
	int	variable_beams; /* if true then number of beams variable */
	int	traveltime;	/* if true then traveltime and angle data supported */
	int	beam_flagging;	/* if true then beam flagging supported */
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system,
			&beams_bath_max, &beams_amp_max, &pixels_ss_max,
			format_name, system_name, format_description,
			&numfile, &filetype,
			&variable_beams, &traveltime, &beam_flagging,
			nav_source, heading_source, vru_source, svp_source,
			&beamwidth_xtrack, &beamwidth_ltrack,
			error);
	if (status == MB_FAILURE)
		{
		*nav_source = MB_DATA_NONE;
		*heading_source = MB_DATA_NONE;
		*vru_source = MB_DATA_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:         %d\n",*format);
		fprintf(stderr,"dbg2       nav_source:     %d\n",*nav_source);
		fprintf(stderr,"dbg2       heading_source: %d\n",*heading_source);
		fprintf(stderr,"dbg2       vru_source:     %d\n",*vru_source);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		fprintf(stderr,"dbg2       error:          %d\n",*error);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_format_beamwidth(int verbose, int *format,
		double *beamwidth_xtrack, double *beamwidth_ltrack,
		int *error)
{
  static char rcs_id[]="$Id$";
	char	*function_name = "mb_format_beamwidth";
	int	status;

	/* format parameters */
	int	system;		/* sonar system id */
	int	beams_bath_max;	/* maximum number of bathymetry beams */
	int	beams_amp_max;	/* maximum number of amplitude beams
					- either 0 or = beams_bath */
	int	pixels_ss_max;	/* maximum number of sidescan pixels */
	char	format_name[MB_NAME_LENGTH];
	char	system_name[MB_NAME_LENGTH];
	char	format_description[MB_DESCRIPTION_LENGTH];
	int	numfile;	/* the number of parallel files required for i/o */
	int	filetype;	/* type of files used (normal, xdr, or gsf) */
	int	variable_beams; /* if true then number of beams variable */
	int	traveltime;	/* if true then traveltime and angle data supported */
	int	beam_flagging;	/* if true then beam flagging supported */
	int     nav_source;     /* data record types containing the primary navigation */
        int     heading_source;	/* data record types containing the primary heading */
        int     vru_source;	/* data record types containing the primary vru */
        int     svp_source;	/* data record types containing the primary svp */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system,
			&beams_bath_max, &beams_amp_max, &pixels_ss_max,
			format_name, system_name, format_description,
			&numfile, &filetype,
			&variable_beams, &traveltime, &beam_flagging,
			&nav_source, &heading_source, &vru_source, &svp_source,
			beamwidth_xtrack, beamwidth_ltrack,
			error);
	if (status == MB_FAILURE)
		{
		*beamwidth_xtrack = 0.0;
		*beamwidth_ltrack = 0.0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:           %d\n",*format);
		fprintf(stderr,"dbg2       beamwidth_xtrack: %f\n",*beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack: %f\n",*beamwidth_ltrack);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		fprintf(stderr,"dbg2       error:          %d\n",*error);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_get_format(int verbose, char *filename, char *fileroot,
		    int *format, int *error)
{
	char	*function_name = "mb_get_format";
	int	status = MB_SUCCESS;
	int	found = MB_NO;
	char	*suffix;
	int	suffix_len;
	FILE	*checkfp;
	mb_path	parfile;
	mb_path	dummy;
	int	pformat;
	struct	stat statbuf;
	char	buffer[MB_COMMENT_MAXLINE];
	char	*result;

	short	*shortptr;
	short	type1, sonar1, type2, sonar2, type1swap, sonar1swap, type2swap, sonar2swap;
	int	nsonar, nlow, nhigh, subsystem, size;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:    %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       filename:  %s\n",filename);
		}

	/* set format not found */
	found = MB_NO;
	*format = 0;

	/* first look for MB suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 6)
		i = strlen(filename) - 6;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".mb")) != NULL
		    || (suffix = strstr(&filename[i],".MB")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len >= 4 && suffix_len <= 6)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    if (sscanf(suffix, ".mb%d", format) > 0
			    || sscanf(suffix, ".MB%d", format) > 0)
			found = MB_YES;
		    }
		}
	    }

	/* look for mbnavedit navigation suffix */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".nve")) != NULL
	    	|| (suffix = strstr(&filename[i],".MVE")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_MBPRONAV;
		    found = MB_YES;
		    }
		}
	    }

	/* look for mbnavadjust navigation suffixes */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".na")) != NULL
	    	|| (suffix = strstr(&filename[i],".NA")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_MBPRONAV;
		    found = MB_YES;
		    }
		}
	    }

	/* look for "fast bath" or .fbt suffix */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".fbt")) != NULL
	    	|| (suffix = strstr(&filename[i],".FBT")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_MBLDEOIH;
		    found = MB_YES;
		    }
		}
	    }

	/* look for "fast filtered bath" or .ffb suffix */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".ffb")) != NULL
	    	|| (suffix = strstr(&filename[i],".FFB")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_MBLDEOIH;
		    found = MB_YES;
		    }
		}
	    }

	/* look for "fast filtered amplitude" or .ffa suffix */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".ffa")) != NULL
	    	|| (suffix = strstr(&filename[i],".FFA")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_MBLDEOIH;
		    found = MB_YES;
		    }
		}
	    }

	/* look for "fast filtered sidescan" or .ffs suffix */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".ffs")) != NULL
	    	|| (suffix = strstr(&filename[i],".FFS")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_MBLDEOIH;
		    found = MB_YES;
		    }
		}
	    }

	/* look for "fast nav" or .fnv suffix */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".fnv")) != NULL || (suffix = strstr(&filename[i],".FNV")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_MBPRONAV;
		    found = MB_YES;
		    }
		}
	    }

	/* look for datalist suffixes */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".dls")) != NULL
	    	|| (suffix = strstr(&filename[i],".DLS")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = -1;
		    found = MB_YES;
		    }
		}
	    }

	/* look for SeaBeam suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".rec")) != NULL
	    	|| (suffix = strstr(&filename[i],".REC")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_SB2100RW;
		    found = MB_YES;
		    }
		}
	    }

	/* look for L3 XSE suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".xse")) != NULL
	    	|| (suffix = strstr(&filename[i],".XSE")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_L3XSERAW;
		    found = MB_YES;
		    }
		}
	    }

	/* look for old Simrad Mermaid suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 8)
		i = strlen(filename) - 8;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],"_raw.all")) != NULL
	    	|| (suffix = strstr(&filename[i],"-raw.all")) != NULL
		|| (suffix = strstr(&filename[i],"_RAW.ALL")) != NULL
	    	|| (suffix = strstr(&filename[i],"-RAW.ALL")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 8)
		    {
		    /* examine the first datagram to determine
			whether data is old or new Simrad format */
		    if ((checkfp = fopen(filename,"r")) != NULL)
			{
			if (fread(buffer,1,8,checkfp) == 8)
			    {
			    shortptr = (short *) &buffer[0];
			    type1 = *shortptr;
			    type1swap = (short) mb_swap_short(type1);
			    shortptr = (short *) &buffer[2];
			    sonar1 = *shortptr;
			    sonar1swap = (short) mb_swap_short(sonar1);

			    shortptr = (short *) &buffer[4];
			    type2 = *shortptr;
			    type2swap = (short) mb_swap_short(type2);
			    shortptr = (short *) &buffer[6];
			    sonar2 = *shortptr;
			    sonar2swap = (short) mb_swap_short(sonar2);
			    if (sonar2 == MBSYS_SIMRAD3_EM710 || sonar2swap == MBSYS_SIMRAD3_EM710
			    	|| sonar2 == MBSYS_SIMRAD3_EM302 || sonar2swap == MBSYS_SIMRAD3_EM302
			    	|| sonar2 == MBSYS_SIMRAD3_EM122 || sonar2swap == MBSYS_SIMRAD3_EM122
			    	|| sonar2 == MBSYS_SIMRAD3_EM2040 || sonar2swap == MBSYS_SIMRAD3_EM2040
			    	|| sonar2 == MBSYS_SIMRAD3_EM2045 || sonar2swap == MBSYS_SIMRAD3_EM2045)
				*format = MBF_EM710RAW;
			    else if (type2 == EM_START
				|| type2 == EM_STOP
				|| type2 == EM_PARAMETER)
				*format = MBF_EMOLDRAW;
			    else if (type2 == EM2_START
				|| type2 == EM2_STOP
				|| type2 == EM2_STOP2
				|| type2 == EM2_STATUS
				|| type2 == EM2_ON
				|| type2 == EM2_RUN_PARAMETER)
				*format = MBF_EM300RAW;
			    else if (type2swap == EM_START
				|| type2swap == EM_STOP
				|| type2swap == EM_PARAMETER)
				*format = MBF_EMOLDRAW;
			    else if (type2swap == EM2_START
				|| type2swap == EM2_STOP
				|| type2swap == EM2_STOP2
				|| type2swap == EM2_STATUS
				|| type2swap == EM2_ON
				|| type2swap == EM2_RUN_PARAMETER)
				*format = MBF_EM300RAW;
			    else if (type1 == EM_START
				|| type1 == EM_STOP
				|| type1 == EM_PARAMETER)
				*format = MBF_EMOLDRAW;
			    else if (type1 == EM2_START
				|| type1 == EM2_STOP
				|| type1 == EM2_STOP2
				|| type1 == EM2_STATUS
				|| type1 == EM2_ON
				|| type1 == EM2_RUN_PARAMETER)
				*format = MBF_EM300RAW;
			    else if (type1swap == EM_START
				|| type1swap == EM_STOP
				|| type1swap == EM_PARAMETER)
				*format = MBF_EMOLDRAW;
			    else if (type1swap == EM2_START
				|| type1swap == EM2_STOP
				|| type1swap == EM2_STOP2
				|| type1swap == EM2_STATUS
				|| type1swap == EM2_ON
				|| type1swap == EM2_RUN_PARAMETER)
				*format = MBF_EM300RAW;
			    else
				*format = MBF_EM300RAW;
			    }
			fclose(checkfp);
			}
		    else
			*format = MBF_EM300RAW;
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    found = MB_YES;
		    }
		}
	    }

	/* look for newer Simrad Mermaid suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".all")) != NULL
	    	|| (suffix = strstr(&filename[i],".ALL")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    /* examine the first datagram to determine
			whether data is old or new Simrad format */
		    if ((checkfp = fopen(filename,"r")) != NULL)
			{
			if (fread(buffer,1,8,checkfp) == 8)
			    {
			    shortptr = (short *) &buffer[0];
			    type1 = *shortptr;
			    type1swap = (short) mb_swap_short(type1);
			    shortptr = (short *) &buffer[2];
			    sonar1 = *shortptr;
			    sonar1swap = (short) mb_swap_short(sonar1);

			    shortptr = (short *) &buffer[4];
			    type2 = *shortptr;
			    type2swap = (short) mb_swap_short(type2);
			    shortptr = (short *) &buffer[6];
			    sonar2 = *shortptr;
			    sonar2swap = (short) mb_swap_short(sonar2);
			    if (sonar2 == MBSYS_SIMRAD3_EM710 || sonar2swap == MBSYS_SIMRAD3_EM710
			    	|| sonar2 == MBSYS_SIMRAD3_EM302 || sonar2swap == MBSYS_SIMRAD3_EM302
			    	|| sonar2 == MBSYS_SIMRAD3_EM122 || sonar2swap == MBSYS_SIMRAD3_EM122
			    	|| sonar2 == MBSYS_SIMRAD3_EM2040 || sonar2swap == MBSYS_SIMRAD3_EM2040
			    	|| sonar2 == MBSYS_SIMRAD3_EM2045 || sonar2swap == MBSYS_SIMRAD3_EM2045
			    	|| sonar2 == MBSYS_SIMRAD3_M3 || sonar2swap == MBSYS_SIMRAD3_M3)
				*format = MBF_EM710RAW;
			    else if (type2 == EM_START
				|| type2 == EM_STOP
				|| type2 == EM_PARAMETER)
				*format = MBF_EMOLDRAW;
			    else if (type2 == EM2_START
				|| type2 == EM2_STOP
				|| type2 == EM2_STOP2
				|| type2 == EM2_STATUS
				|| type2 == EM2_ON
				|| type2 == EM2_RUN_PARAMETER)
				*format = MBF_EM300RAW;
			    else if (type2swap == EM_START
				|| type2swap == EM_STOP
				|| type2swap == EM_PARAMETER)
				*format = MBF_EMOLDRAW;
			    else if (type2swap == EM2_START
				|| type2swap == EM2_STOP
				|| type2swap == EM2_STOP2
				|| type2swap == EM2_STATUS
				|| type2swap == EM2_ON
				|| type2swap == EM2_RUN_PARAMETER)
				*format = MBF_EM300RAW;
			    else if (type1 == EM_START
				|| type1 == EM_STOP
				|| type1 == EM_PARAMETER)
				*format = MBF_EMOLDRAW;
			    else if (type1 == EM2_START
				|| type1 == EM2_STOP
				|| type1 == EM2_STOP2
				|| type1 == EM2_STATUS
				|| type1 == EM2_ON
				|| type1 == EM2_RUN_PARAMETER)
				*format = MBF_EM300RAW;
			    else if (type1swap == EM_START
				|| type1swap == EM_STOP
				|| type1swap == EM_PARAMETER)
				*format = MBF_EMOLDRAW;
			    else if (type1swap == EM2_START
				|| type1swap == EM2_STOP
				|| type1swap == EM2_STOP2
				|| type1swap == EM2_STATUS
				|| type1swap == EM2_ON
				|| type1swap == EM2_RUN_PARAMETER)
				*format = MBF_EM300RAW;
			    else
				*format = MBF_EM300RAW;
			    }
			fclose(checkfp);
			}
		    else
		    	*format = MBF_EM300RAW;
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    found = MB_YES;
		    }
		}
	    }

	/* look for JHC format suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 7)
		i = strlen(filename) - 7;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".merged")) != NULL
	    	|| (suffix = strstr(&filename[i],".MERGED")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 7)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_OMGHDCSJ;
		    found = MB_YES;
		    }
		}
	    }

	/* look for Hypac format suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".HAL")) != NULL
	    	|| (suffix = strstr(&filename[i],".hal")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    if (fileroot != NULL)
			{
			strncpy(fileroot, filename, strlen(filename)-suffix_len);
			fileroot[strlen(filename)-suffix_len] = '\0';
			}
		    *format = MBF_HYPC8101;
		    found = MB_YES;
		    }
		}
	    }

	/* look for MBARI format suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 18)
		i = strlen(filename) - 8;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],"tibr.txt")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],"docr.txt")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],"vnta.txt")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],"ptlo.txt")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],"wfly.txt")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MBARIROV;
		found = MB_YES;
		}
	    }

	/* look for MBARI edited format suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 24)
		i = strlen(filename) - 14;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],"tibredited.txt")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],"docredited.txt")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],"vntaedited.txt")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],"ptloedited.txt")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],"wflyedited.txt")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MBARIROV;
		found = MB_YES;
		}
	    }

	/* look for NIO Hydrosweep DS raw format suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 4)
		i = strlen(filename) - 3;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".hs")) != NULL
	    	|| (suffix = strstr(&filename[i],".HS")) != NULL)
		suffix_len = 3;
	    else
		suffix_len = 0;
	    if (suffix_len == 3)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_HSATLRAW;
		found = MB_YES;
		}
	    }

	/* look for STN Atlas raw format suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".fsw")) != NULL
	    	|| (suffix = strstr(&filename[i],".FSW")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_HSDS2RAW;
		found = MB_YES;
		}
	    }

	/* look for Triton-ELics XTF format suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".xtf")) != NULL
	    	|| (suffix = strstr(&filename[i],".XTF")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_XTFR8101;
		found = MB_YES;
		}
	    }

	/* look for MGD77 suffix conventions */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 7)
		i = strlen(filename) - 6;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".mgd77")) != NULL
	    	|| (suffix = strstr(&filename[i],".MGD77")) != NULL)
		suffix_len = 6;
	    else
		suffix_len = 0;
	    if (suffix_len == 4 || suffix_len == 6)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MGD77DAT;
		found = MB_YES;
		}
	    }

	/* look for MGD77 suffix conventions */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".a77")) != NULL
	    	|| (suffix = strstr(&filename[i],".A77")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MGD77TXT;
		found = MB_YES;
		}
	    }

	/* look for MGD77T suffix conventions */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 6)
		i = strlen(filename) - 5;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".m77t")) != NULL
	    	|| (suffix = strstr(&filename[i],".M77T")) != NULL)
		suffix_len = 5;
	    else
		suffix_len = 0;
	    if (suffix_len == 5)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_PHOTGRAM;
		found = MB_YES;
		}
	    }

	/* look for segy suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 6)
		i = strlen(filename) - 5;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".segy")) != NULL
	    	|| (suffix = strstr(&filename[i],".SEGY")) != NULL)
		suffix_len = 5;
	    else
		suffix_len = 0;
	    if (suffix_len == 5)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_SEGYSEGY;
		found = MB_YES;
		}
	    }

	/* look for IFREMER Trismus format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 4)
		i = strlen(filename) - 3;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".so")) != NULL
	    	|| (suffix = strstr(&filename[i],".SO")) != NULL)
		suffix_len = 3;
	    else
		suffix_len = 0;
	    if (suffix_len == 3)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_EM12IFRM;
		found = MB_YES;
		}
	    }

	/* look for IFREMER netCDF format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".mbb")) != NULL
	    	|| (suffix = strstr(&filename[i],".MBB")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MBNETCDF;
		found = MB_YES;
		}
	    }

	/* look for IFREMER netCDF format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".mbg")) != NULL
	    	|| (suffix = strstr(&filename[i],".MBG")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MBNETCDF;
		found = MB_YES;
		}
	    }

	/* look for IFREMER netCDF navigation format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".nvi")) != NULL
	    	|| (suffix = strstr(&filename[i],".NVI")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_NVNETCDF;
		found = MB_YES;
		}
	    }

	/* look for SAME SURF format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".six")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".SIX")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".sda")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".SDA")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_SAMESURF;
		found = MB_YES;
		}
	    }

	/* look for xyz sounding format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".xyz")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".XYZ")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_ASCIIXYZ;
		found = MB_YES;
		}
	    }

	/* look for yxz sounding format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".yxz")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".YXZ")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_ASCIIYXZ;
		found = MB_YES;
		}
	    }

	/* look for xyt sounding format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".xyt")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".XYT")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_ASCIIXYT;
		found = MB_YES;
		}
	    }

	/* look for yxt sounding format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".yxt")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".YXT")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_ASCIIYXT;
		found = MB_YES;
		}
	    }

	/* look for binary HYDRO93 sounding format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".b93")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".B93")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_HYDROB93;
		found = MB_YES;
		}
	    }

	/* look for SIO SB2000 prefix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 6
		&& strncmp(filename, "SSmed.", 6) == 0)
		{
		if (fileroot != NULL)
		    {
		    strcpy(fileroot, filename);
		    }
		*format = MBF_SB2000SS;
		found = MB_YES;
		}
	    }

	/* look for Marine Sonic MSTIFF format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".MST")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".mst")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MSTIFFSS;
		found = MB_YES;
		}
	    }

	/* look for Edgetech Jstar format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".JSF")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".jsf")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_EDGJSTAR;
		found = MB_YES;

		/* examine the file to determine if sidescan data is
			high frequency (*format = MBF_EDGJSTR2 = 133) rather than
			low frequency (*format = MBF_EDGJSTAR = 132)
			- if both low and high frequency are present treat format
				as low frequency */
		if ((checkfp = fopen(filename,"r")) != NULL)
		    {
		    /* loop over reading data until four full sidescan sonar records are read */
		    done = MB_NO;
		    nsonar = 0;
		    nlow = 0;
		    nhigh = 0;
		    while (done == MB_NO)
			{
			/* read message header */
			if (fread(buffer, MBSYS_JSTAR_MESSAGE_SIZE, 1, checkfp) == 1)
			    {
			    /* extract and check the subsystem value from the message header */
			    subsystem = (int)((mb_u_char) buffer[7]);
			    if (subsystem > 0)
			    	{
				nsonar++;
				}
			    if (subsystem == 20)
			    	nlow++;
			    if (subsystem == 21)
			    	nhigh++;
			    if (nsonar >= 4)
			    	done = MB_YES;

			    /* read and discard the rest of the record */
			    if (done == MB_NO)
			    	{
				mb_get_binary_int(MB_YES, &buffer[12], &size);
				for (i=0;i<size;i++)
			    	    {
				    if (fread(buffer, 1, 1, checkfp) != 1)
					    done = MB_YES;
				    }
				}
			    }

			/* end of file */
			else
			    {
			    done = MB_YES;
			    }
			}
		    fclose(checkfp);
		    if (nlow == 0 && nhigh > 0)
			*format = MBF_EDGJSTR2;
		    else
			*format = MBF_EDGJSTAR;
		    }
		}
	    }

	/* look for HMRG MR1 and BS format conventions */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 6)
		i = strlen(filename) - 5;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".BTYW")) != NULL)
		suffix_len = 5;
	    else if ((suffix = strstr(&filename[i],".btyw")) != NULL)
		suffix_len = 5;
	    else
		suffix_len = 0;
	    if (suffix_len == 5)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MR1PRVR2;
		found = MB_YES;
		}
	    }
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 7)
		i = strlen(filename) - 6;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".BTYWT")) != NULL)
		suffix_len = 6;
	    else if ((suffix = strstr(&filename[i],".btywt")) != NULL)
		suffix_len = 6;
	    else
		suffix_len = 0;
	    if (suffix_len == 6)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MR1PRVR2;
		found = MB_YES;
		}
	    }
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 12)
		i = strlen(filename) - 11;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".BTYWT-DSAN")) != NULL)
		suffix_len = 11;
	    else if ((suffix = strstr(&filename[i],".btywt-dsan")) != NULL)
		suffix_len = 11;
	    else
		suffix_len = 0;
	    if (suffix_len == 11)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MR1PRVR2;
		found = MB_YES;
		}
	    }
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 7)
		i = strlen(filename) - 6;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".BSFIN")) != NULL)
		suffix_len = 6;
	    else if ((suffix = strstr(&filename[i],".bsfin")) != NULL)
		suffix_len = 6;
	    else
		suffix_len = 0;
	    if (suffix_len == 6)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_MR1PRVR2;
		found = MB_YES;
		}
	    }

	/* look for a CARIS GSF export *.gsf format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".gsf")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".GSF")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_GSFGENMB;
		found = MB_YES;
		}
	    }

	/* look for a SAIC GSF *.d0X format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".d0")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_GSFGENMB;
		found = MB_YES;
		}
	    }

	/* look for a Reson 7K multibeam *.s7k format convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".s7k")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".S7K")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".7k")) != NULL)
		suffix_len = 3;
	    else if ((suffix = strstr(&filename[i],".7K")) != NULL)
		suffix_len = 3;
	    else
		suffix_len = 0;
	    if (suffix_len == 4 || suffix_len == 3)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_RESON7KR;
		found = MB_YES;
		}
	    }

	/* look for a Imagex multibeam .83p format convention*/
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".83p")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".83P")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_IMAGE83P;
		found = MB_YES;
		}
	    }

	/* look for an R2R navigation format convention*/
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 8)
		i = strlen(filename) - 7;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".r2rnav")) != NULL)
		suffix_len = 7;
	    else if ((suffix = strstr(&filename[i],".R2RNAV")) != NULL)
		suffix_len = 7;
	    else
		suffix_len = 0;
	    if (suffix_len == 7)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_HIR2RNAV;
		found = MB_YES;
		}
	    }

	/* look for a HYSWEEP *.HSX file format convention*/
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".hsx")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".HSX")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_HYSWEEP1;
		found = MB_YES;
		}
	    }

	/* look for a SEA SWATHplus *.sxi file format convention*/
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".sxi")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".SXI")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_SWPLSSXI;
		found = MB_YES;
		}
	    }

	/* look for a SEA SWATHplus *.sxp file format convention*/
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".sxp")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".SXP")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_SWPLSSXP;
		found = MB_YES;
		}
	    }

	/* look for a 3DatDepth *.raa file format convention*/
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".raa")) != NULL)
		suffix_len = 4;
	    else if ((suffix = strstr(&filename[i],".RAA")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_3DDEPTHP;
		found = MB_YES;
		}
	    }

	/* look for a WASSP *.000 file format convention*/
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".000")) != NULL)
		suffix_len = 4;
	    else
		suffix_len = 0;
	    if (suffix_len == 4)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_WASSPENL;
		found = MB_YES;
		}
	    }

	/* look for a WASSP *.nwsf file format convention*/
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 6)
		i = strlen(filename) - 5;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".nwsf")) != NULL)
		suffix_len = 5;
	    else
		suffix_len = 0;
	    if (suffix_len == 5)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		*format = MBF_WASSPENL;
		found = MB_YES;
		}
	    }

	/* finally check for parameter file */
	sprintf(parfile, "%s.par", filename);
	if (stat(parfile, &statbuf) == 0)
	    {
	    if ((checkfp = fopen(parfile,"r")) != NULL)
		{
		while ((result = fgets(buffer,MBP_FILENAMESIZE,checkfp)) == buffer)
		    {
		    if (buffer[0] != '#')
			{
			if (strlen(buffer) > 0)
			    {
			    if (buffer[strlen(buffer)-1] == '\n')
				    buffer[strlen(buffer)-1] = '\0';
			    }

			if (strncmp(buffer, "FORMAT", 6) == 0)
			    {
			    sscanf(buffer, "%s %d", dummy, &pformat);
			    if (pformat != 0)
		    		{
				*format = pformat;
				if (found == MB_NO)
				    {
				    if (fileroot != NULL)
					strcpy(fileroot, filename);
				    found = MB_YES;
				    }
				}
			    }
			}
		    }
		fclose(checkfp);
		}
	    }

	/* check for old format id and provide alias if needed */
	if (found == MB_YES && *format > 0
	    && (*format < 10 || *format == 44
		|| *format == 52 || *format == 55))
	    {
	    /* replace original mbio id's */
	    if (*format < 10)
		i = format_alias_table[*format];

	    /* handle incorrectly identified SeaBeam 2120 data */
	    else if (*format == 44)
		i = MBF_L3XSERAW;

	    /* handle old Simrad EM12 and EM121 formats */
	    else if (*format == 52 || *format == 55)
		i = MBF_EMOLDRAW;

	    /* print output debug statements */
	    if (verbose >= 2)
		    {
		    fprintf(stderr,"\ndbg2  Old format id aliased to current value in MBIO function <%s>\n",
			    function_name);
		    fprintf(stderr,"dbg2  Old format value:\n");
		    fprintf(stderr,"dbg2       format:     %d\n",*format);
		    fprintf(stderr,"dbg2  Current format value:\n");
		    fprintf(stderr,"dbg2       format:     %d\n",i);
		    }

	    /* set new format value */
	    *format = i;
	    }

	/* set error if needed */
	if (found == MB_NO)
	    {
	    *error = MB_ERROR_BAD_FORMAT;
	    status = MB_FAILURE;
	    *format = 0;
	    if (fileroot != NULL)
		    strcpy(fileroot, filename);
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		if (fileroot != NULL)
		    fprintf(stderr,"dbg2       fileroot:   %s\n",fileroot);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_datalist_open(int verbose,
		void **datalist,
		char *path,
		int look_processed, int *error)
{
	/* local variables */
	char	*function_name = "mb_datalist_open";
	int	status = MB_SUCCESS;
	struct mb_datalist_struct *datalist_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:        %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       datalist:      %p\n",(void *)*datalist);
		fprintf(stderr,"dbg2       path:          %s\n",path);
		fprintf(stderr,"dbg2       look_processed:%d\n",look_processed);
		}

	/* allocate memory for datalist structure */
	if ((status = mb_mallocd(verbose,__FILE__,__LINE__,sizeof(struct mb_datalist_struct),
				datalist,error)) == MB_SUCCESS)
		{
		/* get datalist pointer */
		datalist_ptr = (struct mb_datalist_struct *) *datalist;

		if ((datalist_ptr->fp = fopen(path,"r")) == NULL)
			{
			mb_free(verbose,datalist,error);
			status = MB_FAILURE;
			*error = MB_ERROR_OPEN_FAIL;
			}
		else
			{
			strcpy(datalist_ptr->path,path);
			datalist_ptr->open = MB_YES;
			datalist_ptr->recursion = 0;
			datalist_ptr->look_processed = look_processed;
			datalist_ptr->local_weight = MB_YES;
			datalist_ptr->weight_set = MB_NO;
			datalist_ptr->weight = 0.0;
			datalist_ptr->datalist = NULL;
			}

		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       datalist:      %p\n",(void *)*datalist);
		if (*datalist != NULL)
			{
			fprintf(stderr,"dbg2       datalist->open:       %d\n",datalist_ptr->open);
			fprintf(stderr,"dbg2       datalist->fp:         %p\n",(void *)datalist_ptr->fp);
			fprintf(stderr,"dbg2       datalist->recursion:  %d\n",datalist_ptr->recursion);
			fprintf(stderr,"dbg2       datalist->path:       %s\n",datalist_ptr->path);
			fprintf(stderr,"dbg2       datalist->datalist:   %p\n",(void *)datalist_ptr->datalist);
			}
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:        %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mb_datalist_close(int verbose,
		void **datalist, int *error)
{
	/* local variables */
	char	*function_name = "mb_datalist_close";
	int	status = MB_SUCCESS;
	struct mb_datalist_struct *datalist_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:        %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       datalist:      %p\n",(void *)*datalist);
		}

	/* close file */
	if (*datalist != NULL)
		{
		/* get datalist pointer */
		datalist_ptr = (struct mb_datalist_struct *) *datalist;

		/* close file */
		if (datalist_ptr->open == MB_YES)
			{
			fclose(datalist_ptr->fp);
			}
		}

	/* deallocate structure */
	if (*datalist != NULL)
		{
		status = mb_free(verbose,datalist,error);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       datalist:      %p\n",(void *)*datalist);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:        %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mb_datalist_read(int verbose,
		void *datalist,
		char *path, int *format, double *weight,
		int *error)
{
	/* local variables */
	char	*function_name = "mb_datalist_read";
	int	status = MB_SUCCESS;
	struct mb_datalist_struct *datalist_ptr;
	char	ppath[MB_PATH_MAXLINE];
	int	pstatus;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:        %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       datalist:      %p\n",(void *)datalist);
		}

	/* get datalist pointer */
	datalist_ptr = (struct mb_datalist_struct *) datalist;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       datalist_ptr->open:       %d\n",datalist_ptr->open);
		fprintf(stderr,"dbg2       datalist_ptr->fp:         %p\n",(void *)datalist_ptr->fp);
		fprintf(stderr,"dbg2       datalist_ptr->recursion:  %d\n",datalist_ptr->recursion);
		fprintf(stderr,"dbg2       datalist_ptr->path:       %s\n",datalist_ptr->path);
		fprintf(stderr,"dbg2       datalist_ptr->datalist:   %p\n",(void *)datalist_ptr->datalist);
		fprintf(stderr,"dbg2       datalist_ptr->look_processed:   %d\n",datalist_ptr->look_processed);
		}

	/* call mb_datalist_read2() */
	status = mb_datalist_read2(verbose, datalist, &pstatus, path, ppath, format, weight, error);

	/* deal with pstatus */
	if (status == MB_SUCCESS && *error == MB_ERROR_NO_ERROR)
		{
		if (pstatus == MB_PROCESSED_USE)
			{
			strcpy(path, ppath);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       path:        %s\n",path);
		fprintf(stderr,"dbg2       format:      %d\n",*format);
		fprintf(stderr,"dbg2       weight:      %f\n",*weight);
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mb_datalist_readorg(int verbose,
		void *datalist,
		char *path, int *format, double *weight,
		int *error)
{
	/* local variables */
	char	*function_name = "mb_datalist_read";
	int	status = MB_SUCCESS;
	struct mb_datalist_struct *datalist_ptr;
	struct mb_datalist_struct *datalist2_ptr;
	char	buffer[MB_PATH_MAXLINE];
	char	root[MB_PATH_MAXLINE];
	char	tmpstr[MB_PATH_MAXLINE];
	char	pfile[MB_PATH_MAXLINE];
	int	pfile_specified;
	char	*buffer_ptr;
	int	len;
	int	nscan, done, rdone;
	int	pformat;
	struct stat file_status;
	int	fstat, file_ok;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:        %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       datalist:      %p\n",(void *)datalist);
		}

	/* get datalist pointer */
	datalist_ptr = (struct mb_datalist_struct *) datalist;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       datalist_ptr->open:       %d\n",datalist_ptr->open);
		fprintf(stderr,"dbg2       datalist_ptr->fp:         %p\n",(void *)datalist_ptr->fp);
		fprintf(stderr,"dbg2       datalist_ptr->recursion:  %d\n",datalist_ptr->recursion);
		fprintf(stderr,"dbg2       datalist_ptr->path:       %s\n",datalist_ptr->path);
		fprintf(stderr,"dbg2       datalist_ptr->datalist:   %p\n",(void *)datalist_ptr->datalist);
		fprintf(stderr,"dbg2       datalist_ptr->look_processed:   %d\n",datalist_ptr->look_processed);
		}

	/* loop over reading from datalist_ptr */
	done = MB_NO;
	if (datalist_ptr->open == MB_YES
		&& done == MB_NO)
		{
		while (done == MB_NO)
		    {
		    /* if recursive datalist closed read current datalist */
		    if (datalist_ptr->datalist == NULL)
			{
			rdone = MB_NO;
			while (rdone == MB_NO)
			    {
			    buffer_ptr = fgets(buffer,MB_PATH_MAXLINE,datalist_ptr->fp);

			    /* deal with end of datalist file */
			    if (buffer_ptr != buffer)
				{
				rdone = MB_YES;
				done = MB_YES;
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}

			    /* look for special $PROCESSED command */
			    else if (strncmp(buffer,"$PROCESSED",10) == 0)
				{
				if (datalist_ptr->look_processed
						== MB_DATALIST_LOOK_UNSET)
						datalist_ptr->look_processed = MB_DATALIST_LOOK_YES;
				}

			    /* look for special $RAW command */
			    else if (strncmp(buffer,"$RAW",4) == 0)
				{
				if (datalist_ptr->look_processed
						== MB_DATALIST_LOOK_UNSET)
						datalist_ptr->look_processed = MB_DATALIST_LOOK_NO;
				}

			    /* look for special $NOLOCALWEIGHT command */
			    else if (strncmp(buffer,"$NOLOCALWEIGHT",14) == 0)
				{
				datalist_ptr->local_weight = MB_NO;
				}

			    /* get filename */
			    else if (buffer[0] != '#')
				{
				/* read datalist item */
				nscan = sscanf(buffer,"%s %d %lf",path,format,weight);

				/* get path */
				if (nscan >= 1 && path[0] != '/'
					&& strrchr(datalist_ptr->path,'/') != NULL
					&& (len = strrchr(datalist_ptr->path,'/')
						    - datalist_ptr->path + 1) > 1)
				    {
				    strcpy(tmpstr,path);
				    strncpy(path,datalist_ptr->path,len);
				    path[len] = '\0';
				    strcat(path,tmpstr);
				    }

				/* guess format if no format specified */
				if (nscan == 1)
				    {
				    fstat = mb_get_format(verbose, path, root, &pformat, error);

				    /* if no format specified set it */
				    if (nscan == 1 && pformat != 0)
					    {
					    nscan = 2;
					    *format = pformat;
					    }
				    }

				/* check for processed file if requested */
				if (datalist_ptr->look_processed
					== MB_DATALIST_LOOK_YES)
				    {
				    pfile[0] = '\0';
				    mb_pr_get_ofile(verbose, path,
					    &pfile_specified, pfile, error);
				    if (strlen(pfile) > 0 && pfile[0] != '/'
					    && strrchr(path,'/') != NULL
					    && (len = strrchr(path,'/')
							- path + 1) > 1)
					{
					strcpy(tmpstr,pfile);
					strncpy(pfile,path,len);
					pfile[len] = '\0';
					strcat(pfile,tmpstr);
					}

				    if (pfile_specified == MB_YES)
					{
					if ((fstat = stat(pfile, &file_status)) == 0
					    && (file_status.st_mode & S_IFMT) != S_IFDIR
					    && file_status.st_size > 0)
					    strcpy(path,pfile);
					}
				    }

				/* check if file or datalist can be opened */
				if (nscan >= 2)
				    {
				    fstat = stat(path, &file_status);
				    if (fstat == 0
						&& (file_status.st_mode & S_IFMT) != S_IFDIR
						&& file_status.st_size > 0)
						file_ok = MB_YES;
				    else
						{
						file_ok = MB_NO;
						/* print warning if verbose > 0 */
						if (verbose > 0)
					    	{
					    	fprintf(stderr, "MBIO Warning: Datalist entry skipped because it could not be opened!\n");
					    	fprintf(stderr, "\tDatalist: %s\n", datalist_ptr->path);
					    	fprintf(stderr, "\tFile:     %s\n", path);
					    	}
						}
				    }

				/* set weight value - recursive weight from above
				   overrides local weight as long as local_weight == MB_YES */
				if (nscan >= 2 && file_ok == MB_YES)
				    {
				    /* use recursive weight from above unless prohibited */
				    if (datalist_ptr->weight_set == MB_YES
				    	&& (datalist_ptr->local_weight == MB_NO
						|| nscan != 3))
					*weight = datalist_ptr->weight;

				    /* else if weight not locally specified set to 1.0 */
				    else if (nscan != 3)
					    *weight = 1.0;
				    }

				/* deal with file */
				if (nscan >= 2 && file_ok == MB_YES && *format >= 0)
				    {
				    /* set done */
				    done = MB_YES;
				    rdone = MB_YES;
				    }

				/* deal with recursive datalist */
				else if (nscan >= 2 && file_ok == MB_YES && *format == -1
					&& datalist_ptr->recursion < MB_DATALIST_RECURSION_MAX)
				    {
				    if ((status = mb_datalist_open(verbose,
						    (void **)&(datalist_ptr->datalist), path,
							datalist_ptr->look_processed, error))
					    == MB_SUCCESS)
					{
					datalist2_ptr = (struct mb_datalist_struct *) datalist_ptr->datalist;
					datalist2_ptr->recursion =
						datalist_ptr->recursion + 1;
					datalist2_ptr->local_weight
					    		= datalist_ptr->local_weight;
					rdone = MB_YES;

					/* set weight to recursive value if available */
					if (nscan >= 3
						&& (datalist_ptr->weight_set == MB_NO
							|| datalist_ptr->local_weight == MB_YES))
					    {
					    datalist2_ptr->weight_set = MB_YES;
					    datalist2_ptr->weight = *weight;
					    }

					else if (datalist_ptr->weight_set == MB_YES)
					    {
					    datalist2_ptr->weight_set = MB_YES;
					    datalist2_ptr->weight = datalist_ptr->weight;
					    }

					/* else set weight to local value if available */
					/* else do not set weight */
					else
					    {
					    datalist2_ptr->weight_set = MB_NO;
					    datalist2_ptr->weight = 0.0;
					    }
					}
				    else
					{
					status = MB_SUCCESS;
					*error = MB_ERROR_NO_ERROR;
					}
				    }
				}
			    }
			}

		    /* if open read next entry from recursive datalist */
		    if (done == MB_NO
			&& datalist_ptr->open == MB_YES
			&& datalist_ptr->datalist != NULL)
			{
			datalist2_ptr = (struct mb_datalist_struct *) datalist_ptr->datalist;
			if (datalist2_ptr->open == MB_YES)
			    {
			    /* recursively call mb_read_datalist */
			    status = mb_datalist_read(verbose,
					    (void *)datalist_ptr->datalist,
					    path,
					    format,
					    weight,
					    error);

			    /* if datalist read fails close it */
			    if (status == MB_FAILURE)
				{
				status = mb_datalist_close(verbose,
					    (void **)&(datalist_ptr->datalist),
					    error);
				}
			    else
				{
				done = MB_YES;
				}
			    }
			}
		    }
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       path:        %s\n",path);
		fprintf(stderr,"dbg2       format:      %d\n",*format);
		fprintf(stderr,"dbg2       weight:      %f\n",*weight);
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mb_datalist_read2(int verbose,
		void *datalist,
		int *pstatus, char *path, char *ppath, int *format, double *weight,
		int *error)
{
	/* local variables */
	char	*function_name = "mb_datalist_read2";
	int	status = MB_SUCCESS;
	struct mb_datalist_struct *datalist_ptr;
	struct mb_datalist_struct *datalist2_ptr;
	char	buffer[MB_PATH_MAXLINE];
	char	root[MB_PATH_MAXLINE];
	char	tmpstr[MB_PATH_MAXLINE];
	char	pfile[MB_PATH_MAXLINE];
	int	pfile_specified;
	char	*buffer_ptr;
	int	len;
	int	nscan, done, rdone;
	int	pformat;
	struct stat file_status;
	int	fstat, file_ok;
	int	rawspecified = MB_NO;
	int	processedspecified = MB_NO;
	int	istart;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:        %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       datalist:      %p\n",(void *)datalist);
		}

	/* get datalist pointer */
	datalist_ptr = (struct mb_datalist_struct *) datalist;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       datalist_ptr->open:             %d\n",datalist_ptr->open);
		fprintf(stderr,"dbg2       datalist_ptr->fp:               %p\n",(void *)datalist_ptr->fp);
		fprintf(stderr,"dbg2       datalist_ptr->recursion:        %d\n",datalist_ptr->recursion);
		fprintf(stderr,"dbg2       datalist_ptr->path:             %s\n",datalist_ptr->path);
		fprintf(stderr,"dbg2       datalist_ptr->datalist:         %p\n",(void *)datalist_ptr->datalist);
		fprintf(stderr,"dbg2       datalist_ptr->look_processed:   %d\n",datalist_ptr->look_processed);
		}

	/* loop over reading from datalist_ptr */
	done = MB_NO;
	if (datalist_ptr->open == MB_YES
		&& done == MB_NO)
		{
		while (done == MB_NO)
		    {
		    /* if recursive datalist closed read current datalist */
		    if (datalist_ptr->datalist == NULL)
			{
			rdone = MB_NO;
			while (rdone == MB_NO)
			    {
			    buffer_ptr = fgets(buffer,MB_PATH_MAXLINE,datalist_ptr->fp);

			    /* deal with end of datalist file */
			    if (buffer_ptr != buffer)
				{
				rdone = MB_YES;
				done = MB_YES;
				*pstatus = MB_PROCESSED_NONE;
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}

			    /* look for special $PROCESSED command */
			    else if (strncmp(buffer,"$PROCESSED",10) == 0)
				{
				if (datalist_ptr->look_processed
						== MB_DATALIST_LOOK_UNSET)
						datalist_ptr->look_processed = MB_DATALIST_LOOK_YES;
				}

			    /* look for special $RAW command */
			    else if (strncmp(buffer,"$RAW",4) == 0)
				{
				if (datalist_ptr->look_processed
						== MB_DATALIST_LOOK_UNSET)
						datalist_ptr->look_processed = MB_DATALIST_LOOK_NO;
				}

			    /* look for special $NOLOCALWEIGHT command */
			    else if (strncmp(buffer,"$NOLOCALWEIGHT",14) == 0)
				{
				datalist_ptr->local_weight = MB_NO;
				}

			    /* get filename */
			    else if (buffer[0] != '#')
				{
				/* check for R: and P: prefixes on paths. If either prefix is found,
					then the file path is for a raw file, and the R: or P: indicates
					whether the raw or processed file should be used. These prefixes
					override the datalist_ptr->look_processed value. In general these
					prefixes are placed in datalists by mbgrid and mbmosaic to indicate
					which file was used in gridding/mosaicing */
				if (buffer[1] == ':')
					{
					if (strncmp(buffer, "R:", 2) == 0)
						{
						istart = 2;
						rawspecified = MB_YES;
						}
					else if (strncmp(buffer, "P:", 2) == 0)
						{
						istart = 2;
						processedspecified = MB_YES;
						}
					else
						istart = 0;
					}
				else
					istart = 0;

				/* read datalist item */
				nscan = sscanf(&(buffer[istart]),"%s %d %lf",path,format,weight);

				/* get path */
				if (nscan >= 1 && path[0] != '/'
					&& strrchr(datalist_ptr->path,'/') != NULL
					&& (len = strrchr(datalist_ptr->path,'/')
						    - datalist_ptr->path + 1) > 1)
				    {
				    strcpy(tmpstr,path);
				    strncpy(path,datalist_ptr->path,len);
				    path[len] = '\0';
				    strcat(path,tmpstr);
				    }

				/* guess format if no format specified */
				if (nscan == 1)
				    {
				    fstat = mb_get_format(verbose, path, root, &pformat, error);

				    /* if no format specified set it */
				    if (nscan == 1 && pformat != 0)
					    {
					    nscan = 2;
					    *format = pformat;
					    }
				    }

				/* check if file or datalist can be opened */
				if (nscan >= 2)
				    {
				    fstat = stat(path, &file_status);
				    if (fstat == 0
					    && (file_status.st_mode & S_IFMT) != S_IFDIR
					    && file_status.st_size > 0)
					    {
					    file_ok = MB_YES;
					    }
				    else
					    {
					    file_ok = MB_NO;
					    /* print warning if verbose > 0 */
					    if (verbose > 0)
						    {
						    fprintf(stderr, "MBIO Warning: Datalist entry skipped because it could not be opened!\n");
						    fprintf(stderr, "\tDatalist: %s\n", datalist_ptr->path);
						    fprintf(stderr, "\tFile:     %s\n", path);
						    }
					    }
				    }

				/* check for processed file */
				*pstatus = MB_PROCESSED_NONE;
				pfile[0] = '\0';
				if (file_ok == MB_YES)
				    {
				    mb_pr_get_ofile(verbose, path,
					    &pfile_specified, pfile, error);
				    if (strlen(pfile) > 0 && pfile[0] != '/'
					    && strrchr(path,'/') != NULL
					    && (len = strrchr(path,'/')
							- path + 1) > 1)
					{
					strcpy(tmpstr,pfile);
					strncpy(pfile,path,len);
					pfile[len] = '\0';
					strcat(pfile,tmpstr);
					}

				    if (pfile_specified == MB_YES)
					{
					if ((fstat = stat(pfile, &file_status)) == 0
					    && (file_status.st_mode & S_IFMT) != S_IFDIR
					    && file_status.st_size > 0)
					    {
					    strcpy(ppath,pfile);
					    if (datalist_ptr->look_processed
						== MB_DATALIST_LOOK_YES)
						*pstatus = MB_PROCESSED_USE;
					    else
						*pstatus = MB_PROCESSED_EXIST;
					    }
					}

				    /* apply processed or raw prefixes */
				    if (*pstatus ==  MB_PROCESSED_EXIST
				    		&& processedspecified == MB_YES)
					*pstatus = MB_PROCESSED_USE;
				    else if (*pstatus ==  MB_PROCESSED_USE
				    		&& rawspecified == MB_YES)
					*pstatus = MB_PROCESSED_EXIST;
				    }

				/* set weight value - recursive weight from above
				   overrides local weight as long as local_weight == MB_YES */
				if (nscan >= 2 && file_ok == MB_YES)
				    {
				    /* use recursive weight from above unless prohibited */
				    if (datalist_ptr->weight_set == MB_YES
				    	&& (datalist_ptr->local_weight == MB_NO
						|| nscan != 3))
					*weight = datalist_ptr->weight;

				    /* else if weight not locally specified set to 1.0 */
				    else if (nscan != 3)
					    *weight = 1.0;
				    }

				/* deal with file */
				if (nscan >= 2 && file_ok == MB_YES && *format >= 0)
				    {
				    /* set done */
				    done = MB_YES;
				    rdone = MB_YES;
				    }

				/* deal with recursive datalist */
				else if (nscan >= 2 && file_ok == MB_YES && *format == -1
					&& datalist_ptr->recursion < MB_DATALIST_RECURSION_MAX)
				    {
				    if ((status = mb_datalist_open(verbose,
						    (void **)&(datalist_ptr->datalist), path,
							datalist_ptr->look_processed, error))
					    == MB_SUCCESS)
					{
					datalist2_ptr = (struct mb_datalist_struct *) datalist_ptr->datalist;
					datalist2_ptr->recursion =
						datalist_ptr->recursion + 1;
					datalist2_ptr->local_weight
					    		= datalist_ptr->local_weight;
					rdone = MB_YES;

					/* set weight to recursive value if available */
					if (nscan >= 3
						&& (datalist_ptr->weight_set == MB_NO
							|| datalist_ptr->local_weight == MB_YES))
					    {
					    datalist2_ptr->weight_set = MB_YES;
					    datalist2_ptr->weight = *weight;
					    }

					else if (datalist_ptr->weight_set == MB_YES)
					    {
					    datalist2_ptr->weight_set = MB_YES;
					    datalist2_ptr->weight = datalist_ptr->weight;
					    }

					/* else set weight to local value if available */
					/* else do not set weight */
					else
					    {
					    datalist2_ptr->weight_set = MB_NO;
					    datalist2_ptr->weight = 0.0;
					    }
					}
				    else
					{
					status = MB_SUCCESS;
					*error = MB_ERROR_NO_ERROR;
					}
				    }
				}
			    }
			}

		    /* if open read next entry from recursive datalist */
		    if (done == MB_NO
			&& datalist_ptr->open == MB_YES
			&& datalist_ptr->datalist != NULL)
			{
			datalist2_ptr = (struct mb_datalist_struct *) datalist_ptr->datalist;
			if (datalist2_ptr->open == MB_YES)
			    {
			    /* recursively call mb_read_datalist */
			    status = mb_datalist_read2(verbose,
					    (void *)datalist_ptr->datalist,
					    pstatus,
					    path,
					    ppath,
					    format,
					    weight,
					    error);

			    /* if datalist read fails close it */
			    if (status == MB_FAILURE)
				{
				status = mb_datalist_close(verbose,
					    (void **)&(datalist_ptr->datalist),
					    error);
				}
			    else
				{
				done = MB_YES;
				}
			    }
			}
		    }
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       pstatus      %d\n",*pstatus);
		fprintf(stderr,"dbg2       path:        %s\n",path);
		fprintf(stderr,"dbg2       ppath:       %s\n",ppath);
		fprintf(stderr,"dbg2       format:      %d\n",*format);
		fprintf(stderr,"dbg2       weight:      %f\n",*weight);
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int cvt_to_nix_path(char *path)
{
	/* Replace back slashes by slashes and trim first two chars in paths like "C:/path" */
	size_t k, len = strlen(path);
	
	if (len == 0) return(0);
	for (k = 0; k < len; k++)
		if (path[k] == '\\') path[k] = '/';

	if (path[1] == ':')
		{
		for (k = 0; k < len - 2; k++)
			path[k] = path[k + 2];
		path[len - 2] = '\0';	/* Make sure it's null terminated */
		}

	return(0);
}
/*--------------------------------------------------------------------*/
int mb_get_relative_path(int verbose,
		char *path,
		char *ipwd,
		int *error)
{
	/* local variables */
	char	*function_name = "mb_get_relative_path";
	int	status = MB_SUCCESS;
	char	relativepath[MB_PATH_MAXLINE] = {""};
	char	pwd[MB_PATH_MAXLINE] = {""};
	int	pathlen;
	int	pwdlen;
	int	same, isame, ndiff;
	char	*bufptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:        %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       path:          %s\n",path);
		fprintf(stderr,"dbg2       ipwd:          %s\n",ipwd);
		}

	/* get string lengths */
	pathlen = strlen(path);
	pwdlen = strlen(ipwd);

#ifdef WIN32
	/* The approximation here is to try to make a Windows path like a unix one and expect that
	   the same algorithm still applies. Off course, there probably many ways this can go wrong
	   because we trim the first 2 chars in strings like C:\blabla and don't put it back. But
	   test have shown that it was maybe not necessary.
	*/
	cvt_to_nix_path(path);
	cvt_to_nix_path(ipwd);
#endif

	/* if path doesn't start with '/' not an absolute path */
	if (pathlen > 0 && path[0] != '/' )
	    {
	    strncpy(relativepath,path,MB_PATH_MAXLINE);
	    bufptr = getcwd(path, MB_PATH_MAXLINE);
#ifdef WIN32
		cvt_to_nix_path(path);
		cvt_to_nix_path(bufptr);
#endif
	    if (bufptr == NULL || strlen(path) + pathlen + 1 >= MB_PATH_MAXLINE)
	        {
	        strcpy(path,relativepath);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
		}
	    else
	        {
		strcat(path,"/");
		   strcat(path,relativepath);
		}
	    }

	/* if path doesn't start with '/' not an absolute path */
	if (pwdlen == 0)
	     pwd[0] = '\0';
	else if (ipwd[0] == '/' )
	     strncpy(pwd,ipwd,MB_PATH_MAXLINE);
	else
	    {
	    bufptr = getcwd(pwd, MB_PATH_MAXLINE);
#ifdef WIN32
		cvt_to_nix_path(pwd);
		cvt_to_nix_path(bufptr);
#endif
	    if (bufptr == NULL || strlen(pwd) + pwdlen + 1 >= MB_PATH_MAXLINE)
	        {
	        strncpy(pwd,ipwd,MB_PATH_MAXLINE);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
		}
	    else
	        {
		strcat(pwd,"/");
		strcat(pwd,ipwd);
		}
	    }

	mb_get_shortest_path(verbose,path,error);
	mb_get_shortest_path(verbose,pwd,error);

	pathlen = strlen(path);
	pwdlen = strlen(pwd);

	/* try to get best relative path */
	if (pathlen > 0 && pwdlen > 0)
	    {
	    /* look for last identical slash-terminated section */
	    same = MB_YES;
	    isame = 0;
	    for (i=0;i<MIN(pathlen, pwdlen) && same==MB_YES;i++)
		{
		if (path[i] == pwd[i])
		    {
		    if (path[i] == '/')
			isame = i;
		    else if (i == pwdlen - 1
				&& pathlen > i
				&& path[i+1] == '/')
			isame = i + 1;
		    }
		else
		    {
		    same = MB_NO;
		    }
		}

	    /* if more in common than first '/' look through pwd */
	    if (isame > 0)
		{
		/* look for number of different directories in pwd */
		ndiff = 0;
		for (i=isame;i<pwdlen-1;i++)
		    {
		    if (pwd[i] == '/')
			ndiff++;
		    }

		/* now make relative path if possible */
		relativepath[0] = '\0';
		for (i=0;i<ndiff;i++)
		    {
		    strcat(relativepath, "../");
		    }
		if (pathlen > isame + 1)
		    strcat(relativepath, &path[isame+1]);

		/* overwrite original path with relative path if possible */
		if (strlen(relativepath) > 0)
		    strcpy(path, relativepath);

		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}

	    /* else keep original path */
	    else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	    }

	/* no error even if no path or pwd */
	else if (pathlen <= 0 || pwdlen <= 0)
	    {
	    status = MB_SUCCESS;
	    *error = MB_ERROR_NO_ERROR;
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       path:          %s\n",path);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mb_get_shortest_path(int verbose,
		char *path,
		int *error)
{
	/* local variables */
	char	*function_name = "mb_get_shortest_path";
	int	status = MB_SUCCESS;
	char	tmppath[MB_PATH_MAXLINE];
	char	lasttoken[MB_PATH_MAXLINE];
	char	*result;
	int	lasttokenordinary;
	int	done, change;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:        %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       path:          %s\n",path);
		}

#ifdef WIN32
	/* No path hammerings here. Too dangerous */
	//return(status);
#endif

	/* loop until no changes are made */
	done = MB_NO;
	while (done == MB_NO)
	    {
	    /* set no change made */
	    change = MB_NO;

	    /* copy the path */
	    strncpy(tmppath, path, MB_PATH_MAXLINE);

	    /* step through path */
	    path[0] = '\0';
	    if (tmppath[0] == '/')
		strcpy(path, "/");
	    result = strtok(tmppath, "/");
	    lasttokenordinary = MB_NO;
	    while (result != NULL)
		{
		if (strcmp("..", result) == 0)
		    {
		    if (lasttokenordinary == MB_NO)
			{
			strcat(path, "../");
			}
		    else
			{
			change = MB_YES;
			}
		    lasttokenordinary = MB_NO;
		    }
		else if (strcmp(".", result) == 0)
		     {
		     }
		else
		    {
		    if (lasttokenordinary == MB_YES)
			{
			strcat(path, lasttoken);
			strcat(path, "/");
			}
		    lasttokenordinary = MB_YES;
		    strcpy(lasttoken, result);
		    }
		result = strtok(NULL, "/");
		}
	    if (lasttokenordinary == MB_YES)
		strcat(path, lasttoken);

	    /* if change not made set done = MB_YES */
	    if (change == MB_NO)
		done = MB_YES;

	    }

	/* no error even if no path */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       path:          %s\n",path);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/

int mb_get_basename(int verbose,
		char *path,
		int *error)
{
	/* local variables */
	char	*function_name = "mb_get_basename";
	int	status = MB_SUCCESS;
	char	tmppath[MB_PATH_MAXLINE];
	char	*result;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:        %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       path:          %s\n",path);
		}

	/* copy the path */
	strncpy(tmppath, path, MB_PATH_MAXLINE);

	/* return everything after the last '/' */
	result = strrchr(tmppath,'/');
	if (result != NULL  && strlen(result) > 1)
		strcpy(path,&result[1]);

	/* remove .fbt .fnv .inf .esf suffix if present */
	if (strlen(path) > 4)
		{
		i = strlen(path) - 4;
		if ((result = strstr(&path[i],".fbt")) != NULL)
			{
			path[i] = '\0';
			}
		else if ((result = strstr(&path[i],".fnv")) != NULL)
			{
			path[i] = '\0';
			}
		else if ((result = strstr(&path[i],".inf")) != NULL)
			{
			path[i] = '\0';
			}
		else if ((result = strstr(&path[i],".esf")) != NULL)
			{
			path[i] = '\0';
			}
		}

	/* no error even if no path */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       path:          %s\n",path);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
