/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.c	2/18/94
 *    $Id: mb_format.c,v 5.6 2001-06-29 22:48:04 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * $Log: not supported by cvs2svn $
 * Revision 5.5  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.4  2001/04/06  22:05:59  caress
 * Consolidated xse formats into one format.
 *
 * Revision 5.3  2001/03/22  20:45:56  caress
 * Trying to make 5.0.beta0...
 *
 * Revision 5.2  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.1  2000/12/10  20:26:50  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.10  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.9  2000/04/19  21:06:32  caress
 * Added #include mb_io.h
 *
 * Revision 4.8  2000/04/19  20:51:58  caress
 * Added datalist parsing code.
 *
 * Revision 4.7  2000/01/25  01:45:10  caress
 * Null terminate fileroot strings.
 *
 * Revision 4.6  2000/01/20  00:09:23  caress
 * Added function mb_get_format().
 *
 * Revision 4.5  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.4  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.4  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/04/22  17:49:13  caress
 * Removed output messages which went to stdout, messing up
 * Postscript output from mbswath and mbcontour.
 *
 * Revision 4.1  1994/04/22  17:49:13  caress
 * Removed output messages which went to stdout, messing up
 * Postscript output from mbswath and mbcontour.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.5  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.4  1994/02/21  21:35:02  caress
 * Set format alias message to go to stdout if verbose == 1
 * and stderr if verbose > 1.
 *
 * Revision 4.3  1994/02/21  20:54:16  caress
 * Added verbose message for format alias events.
 *
 * Revision 4.2  1994/02/21  19:45:26  caress
 * Made it actually work and added aliasing of old format
 * id's to new format id's.
 *
 * Revision 4.1  1994/02/21  04:55:25  caress
 * Fixed some simple errors.
 *
 * Revision 4.0  1994/02/21  04:52:01  caress
 * Initial version.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"
#include "../../include/mb_swap.h"
#include "../../include/mb_format.h"
#include "../../include/mbsys_simrad.h"
#include "../../include/mbsys_simrad2.h"

static char rcs_id[]="$Id: mb_format.c,v 5.6 2001-06-29 22:48:04 caress Exp $";

/*--------------------------------------------------------------------*/
int mb_format_register(int verbose, 
			int *format, 
			char *mbio_ptr, 
			int *error)
{
	char	*function_name = "mb_format_register";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:  %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       format:    %d\n",*format);
		}
		
	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* check for old format id and provide alias if needed */
	if (*format > 0 && *format < 10)
		{
		/* find current format value */
		i = format_alias_table[*format];

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
		
	/* handle old Simrad EM12 and EM121 formats */
	else if (*format == 52 || *format == 55)
		*format = 51;
		
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
	else if (*format == MBF_MBLDEOIH)
		{
		status = mbr_register_mbldeoih(verbose, mbio_ptr, error); 
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
	else if (*format == MBF_MGD77DAT)
		{
		status = mbr_register_mgd77dat(verbose, mbio_ptr, error); 
		}
	else if (*format == MBF_MBARIROV)
		{
		status = mbr_register_mbarirov(verbose, mbio_ptr, error); 
		}
	else if (*format == MBF_MBPRONAV)
		{
		status = mbr_register_mbpronav(verbose, mbio_ptr, error); 
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
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr,"dbg2       format_alloc:       %d\n",mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %d\n",mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %d\n",mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %d\n",mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %d\n",mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %d\n",mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %d\n",mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %d\n",mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %d\n",mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %d\n",mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %d\n",mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %d\n",mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %d\n",mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %d\n",mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %d\n",mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       copyrecord:         %d\n",mb_io_ptr->mb_io_copyrecord);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       format:    %d\n",*format);
		}

	/* check for old format id and provide alias if needed */
	if (*format > 0 && *format < 10)
		{
		/* find current format value */
		i = format_alias_table[*format];

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
	/* handle old Simrad EM12 and EM121 formats */
	else if (*format == 52 || *format == 55)
		*format = 51;

	/* look for a corresponding format */
	if (*format == MBF_SBSIOMRG)
		{
		status = mbr_info_sbsiomrg(verbose, system, 
			beams_bath_max, beams_amp_max, pixels_ss_max, 
			format_name, system_name, format_description, 
			numfile, filetype, 
			variable_beams, traveltime, beam_flagging, 
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
			nav_source, heading_source, vru_source, 
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
		*beamwidth_xtrack = 0.0;
		*beamwidth_ltrack = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
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
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system, 
			&beams_bath_max, &beams_amp_max, &pixels_ss_max, 
			format_name, system_name, format_description, 
			&numfile, &filetype, 
			&variable_beams, &traveltime, &beam_flagging, 
			&nav_source, &heading_source, &vru_source, 
			&beamwidth_xtrack, &beamwidth_ltrack, 
			error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
  static char rcs_id[]="$Id: mb_format.c,v 5.6 2001-06-29 22:48:04 caress Exp $";
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
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, system, 
			&beams_bath_max, &beams_amp_max, &pixels_ss_max, 
			format_name, system_name, format_description, 
			&numfile, &filetype, 
			&variable_beams, &traveltime, &beam_flagging, 
			&nav_source, &heading_source, &vru_source, 
			&beamwidth_xtrack, &beamwidth_ltrack, 
			error);
	if (status == MB_FAILURE)
		{
		*system = MB_SYS_NONE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
  static char rcs_id[]="$Id: mb_format.c,v 5.6 2001-06-29 22:48:04 caress Exp $";
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
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system, 
			beams_bath_max, beams_amp_max, pixels_ss_max, 
			format_name, system_name, format_description, 
			&numfile, &filetype, 
			&variable_beams, &traveltime, &beam_flagging, 
			&nav_source, &heading_source, &vru_source, 
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
  static char rcs_id[]="$Id: mb_format.c,v 5.6 2001-06-29 22:48:04 caress Exp $";
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
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system, 
			&beams_bath_max, &beams_amp_max, &pixels_ss_max, 
			format_name, system_name, description, 
			&numfile, &filetype, 
			&variable_beams, &traveltime, &beam_flagging, 
			&nav_source, &heading_source, &vru_source, 
			&beamwidth_xtrack, &beamwidth_ltrack, 
			error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
  static char rcs_id[]="$Id: mb_format.c,v 5.6 2001-06-29 22:48:04 caress Exp $";
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
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system, 
			&beams_bath_max, &beams_amp_max, &pixels_ss_max, 
			format_name, system_name, format_description, 
			&numfile, &filetype, 
			variable_beams, traveltime, beam_flagging, 
			&nav_source, &heading_source, &vru_source, 
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		int *nav_source, int *heading_source, int *vru_source, 
		int *error)
{
  static char rcs_id[]="$Id: mb_format.c,v 5.6 2001-06-29 22:48:04 caress Exp $";
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system, 
			&beams_bath_max, &beams_amp_max, &pixels_ss_max, 
			format_name, system_name, format_description, 
			&numfile, &filetype, 
			&variable_beams, &traveltime, &beam_flagging, 
			nav_source, heading_source, vru_source, 
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
  static char rcs_id[]="$Id: mb_format.c,v 5.6 2001-06-29 22:48:04 caress Exp $";
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

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}

	/* set the message and status */
	status = mb_format_info(verbose, format, &system, 
			&beams_bath_max, &beams_amp_max, &pixels_ss_max, 
			format_name, system_name, format_description, 
			&numfile, &filetype, 
			&variable_beams, &traveltime, &beam_flagging, 
			&nav_source, &heading_source, &vru_source, 
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
	char	buffer[6];
	short	*type1, *type2;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       filename:  %s\n",filename);
		}
		
	/* set format not found */
	found = MB_NO;
	*format = 0;
	
	/* first look for MB suffix convention */
	if (strlen(filename) > 6)
	    i = strlen(filename) - 6;
	else
	    i = 0;
	if ((suffix = strstr(&filename[i],".mb")) != NULL)
	    {
	    suffix_len = strlen(suffix);
	    if (suffix_len >= 4 && suffix_len <= 6)
		{
		if (fileroot != NULL)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    }
		sscanf(suffix, ".mb%d", format);
		found = MB_YES;
		}
	    }

	/* look for mbnavedit and mbnavadjust navigation suffixes */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".nv")) != NULL)
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
	    if ((suffix = strstr(&filename[i],".dls")) != NULL)
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
	    if ((suffix = strstr(&filename[i],".rec")) != NULL)
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
	    if ((suffix = strstr(&filename[i],".xse")) != NULL)
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

	/* look for Simrad Mermaid suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 8)
		i = strlen(filename) - 8;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],"_raw.all")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 8)
		    {
		    /* examine the first datagram to determine
			whether data is old or new Simrad format */
		    if ((checkfp = fopen(filename,"r")) != NULL)
			{
			type1 = (short *) &buffer[0];
			type2 = (short *) &buffer[4];
			if (fread(buffer,1,6,checkfp) == 6)
			    {
#ifdef BYTESWAPPED
			    *type1 = (short) mb_swap_short(*type1);
			    *type2 = (short) mb_swap_short(*type2);
#endif
			    if (*type2 == EM_START
				|| *type2 == EM_STOP
				|| *type2 == EM_PARAMETER)
				*format = MBF_EMOLDRAW;
			    else if (*type2 == EM2_START
				|| *type2 == EM2_STOP
				|| *type1 == EM2_STOP2
				|| *type1 == EM2_OFF
				|| *type1 == EM2_ON
				|| *type2 == EM2_RUN_PARAMETER)
				*format = MBF_EM300RAW;
			    else if (*type1 == EM_START
				|| *type1 == EM_STOP
				|| *type1 == EM_PARAMETER)
				*format = MBF_EMOLDRAW;
			    else if (*type1 == EM2_START
				|| *type1 == EM2_STOP
				|| *type1 == EM2_STOP2
				|| *type1 == EM2_OFF
				|| *type1 == EM2_ON
				|| *type1 == EM2_RUN_PARAMETER)
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
	    if ((suffix = strstr(&filename[i],".merged")) != NULL)
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

	/* look for MBARI format suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 18)
		i = strlen(filename) - 8;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],"tibr.txt")) != NULL)
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

	/* look for STN Atlas raw format suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) >= 5)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".fsw")) != NULL)
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

	/* check for old format id and provide alias if needed */
	if (found == MB_YES && *format > 0 && *format < 10)
	    {
	    /* find current format value */
	    i = format_alias_table[*format];

	    /* print output debug statements */
	    if (verbose >= 0)
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		char **datalist,
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       datalist:      %d\n",*datalist);
		fprintf(stderr,"dbg2       path:          %s\n",path);
		fprintf(stderr,"dbg2       look_processed:%s\n",look_processed);
		}

	/* allocate memory for datalist structure */
	if (status = mb_malloc(verbose,sizeof(struct mb_datalist_struct),
				datalist,error) == MB_SUCCESS)
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
			datalist_ptr->weight_set = MB_NO;
			datalist_ptr->weight = 0.0;
			datalist_ptr->datalist = NULL;
			}
		
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       datalist:      %d\n",*datalist);
		if (*datalist != NULL)
			{
			fprintf(stderr,"dbg2       datalist->open:       %d\n",datalist_ptr->open);
			fprintf(stderr,"dbg2       datalist->fp:         %d\n",datalist_ptr->fp);
			fprintf(stderr,"dbg2       datalist->recursion:  %d\n",datalist_ptr->recursion);
			fprintf(stderr,"dbg2       datalist->path:       %s\n",datalist_ptr->path);
			fprintf(stderr,"dbg2       datalist->datalist:   %d\n",datalist_ptr->datalist);
			}
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:        %d\n",status);
		}

	return(status);
}
		
/*--------------------------------------------------------------------*/
int mb_datalist_close(int verbose,
		char **datalist, int *error)
{
	/* local variables */
	char	*function_name = "mb_datalist_close";
	int	status = MB_SUCCESS;
	struct mb_datalist_struct *datalist_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       datalist:      %d\n",*datalist);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       datalist:      %d\n",*datalist);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:        %d\n",status);
		}

	return(status);
}
		
/*--------------------------------------------------------------------*/
int mb_datalist_read(int verbose,
		char *datalist,
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       datalist:      %d\n",datalist);
		}

	/* get datalist pointer */
	datalist_ptr = (struct mb_datalist_struct *) datalist;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       datalist_ptr->open:       %d\n",datalist_ptr->open);
		fprintf(stderr,"dbg2       datalist_ptr->fp:         %d\n",datalist_ptr->fp);
		fprintf(stderr,"dbg2       datalist_ptr->recursion:  %d\n",datalist_ptr->recursion);
		fprintf(stderr,"dbg2       datalist_ptr->path:       %s\n",datalist_ptr->path);
		fprintf(stderr,"dbg2       datalist_ptr->datalist:   %d\n",datalist_ptr->datalist);
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
			    if (buffer_ptr != buffer)
				{
				rdone = MB_YES;
				done = MB_YES;
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
				/* look for special command */
				else if (strncmp(buffer,"$PROCESSED",10) == 0)
				{
				if (datalist_ptr->look_processed 
						== MB_DATALIST_LOOK_UNSET)
						datalist_ptr->look_processed = MB_DATALIST_LOOK_YES;
				}
				else if (strncmp(buffer,"$RAW",4) == 0)
				{
				if (datalist_ptr->look_processed 
						== MB_DATALIST_LOOK_UNSET)
						datalist_ptr->look_processed = MB_DATALIST_LOOK_NO;
				}
			    else if (buffer[0] != '#')
				{
				/* read datalist item */
				nscan = sscanf(buffer,"%s %d %lf",path,format,weight);

				/* get path */
				if (nscan >= 1 && path[0] != '/'
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
				    mb_pr_get_ofile(verbose, path, 
					    &pfile_specified, pfile, error);

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
					    	fprintf(stderr, "\tFile:     %s\n\tDatalist: %s\n", path, datalist_ptr->path);					    
					    	}
						}
				    }
				    
				/* set weight value - recursive weight from above
				   overrides local weight */
				if (nscan >= 2 && file_ok == MB_YES)
				    {
				    /* use recursive weight from above */
				    if (datalist_ptr->weight_set == MB_YES)
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
				    if (status = mb_datalist_open(verbose,
						    (char *)&(datalist_ptr->datalist), path,
							datalist_ptr->look_processed, error)
					    == MB_SUCCESS)
					{
					datalist2_ptr = (struct mb_datalist_struct *) datalist_ptr->datalist;
					datalist2_ptr->recursion =
						datalist_ptr->recursion + 1;
					rdone = MB_YES;
					
					/* set weight to recursive value if available */
					if (datalist_ptr->weight_set == MB_YES)
					    {
					    datalist2_ptr->weight_set = MB_YES;
					    datalist2_ptr->weight = datalist_ptr->weight;
					    }
					
					/* else set weight to local value if available */
					else if (nscan >= 3)
					    {
					    datalist2_ptr->weight_set = MB_YES;
					    datalist2_ptr->weight = *weight;
					    }
					
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
					    (char *)datalist_ptr->datalist,
					    path,
					    format,
					    weight,
					    error);
			    
			    /* if datalist read fails close it */
			    if (status == MB_FAILURE)
				{
				status = mb_datalist_close(verbose,
					    (char *)&(datalist_ptr->datalist),
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
