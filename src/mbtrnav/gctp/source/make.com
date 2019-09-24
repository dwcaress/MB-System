$! Check to see if the library exists
$!-----------------------------------
$ open/read/error=cre_lib lib WORLIB:geolib.olb
$ close lib
$ goto build_lib
$!
$! Create the library
$!-------------------
$ cre_lib:
$ lib/create WORLIB:geolib.olb
$!
$! Add the individual routines to the library
$!-------------------------------------------
$ build_lib:
$
$ makesup c_getgrd	WORLIB:geolib.olb
$ makesup c_grderr	WORLIB:geolib.olb
$ makesup c_eval 	WORLIB:geolib.olb
$ makesup c_grdred	WORLIB:geolib.olb
$ makesup c_ptclse	WORLIB:geolib.olb
$ makesup c_ptio	WORLIB:geolib.olb
$ makesup c_ptopen	WORLIB:geolib.olb
$ makesup c_putgrd	WORLIB:geolib.olb
$ makesup c_steplr	WORLIB:geolib.olb
$ makesup c_invert	WORLIB:geolib.olb
$ makesup c_ckptid	WORLIB:geolib.olb
$ makesup c_getrwt	WORLIB:geolib.olb
$ makesup c_putrwt	WORLIB:geolib.olb
$ makesup br_decdeg	WORLIB:geolib.olb
$ makesup br_degdms	WORLIB:geolib.olb
$ makesup br_prostr	WORLIB:geolib.olb
$ makesup c_decdeg	WORLIB:geolib.olb
$ makesup c_degdms	WORLIB:geolib.olb
$ makesup c_prostr	WORLIB:geolib.olb
$ makesup br_proj	WORLIB:geolib.olb
$ makesup br_projon	WORLIB:geolib.olb
$ makesup check_dms	WORLIB:geolib.olb
$ makesup c_proj	WORLIB:geolib.olb
$ makesup c_projon	WORLIB:geolib.olb
$ makesup proj_err	WORLIB:geolib.olb
$ makesup proj_print	WORLIB:geolib.olb
$ makesup proj_report	WORLIB:geolib.olb
$ makesup prt_spcs_zone	WORLIB:geolib.olb
$ makesup spcs_zone	WORLIB:geolib.olb
$ makesup spheroid	WORLIB:geolib.olb
$ makesup c_ckunit	WORLIB:geolib.olb
$ makesup br_ckunit	WORLIB:geolib.olb
$ makesup c_mapedg	WORLIB:geolib.olb
$ makesup digitizer	WORLIB:geolib.olb
$ makesup psupport	WORLIB:geolib.olb
$ makesup alberfor	WORLIB:geolib.olb
$ makesup alberinv	WORLIB:geolib.olb
$ makesup alconfor	WORLIB:geolib.olb
$ makesup alconinv	WORLIB:geolib.olb
$ makesup azimfor	WORLIB:geolib.olb
$ makesup aziminv	WORLIB:geolib.olb
$ makesup cproj		WORLIB:geolib.olb
$ makesup eqconfor	WORLIB:geolib.olb
$ makesup eqconinv	WORLIB:geolib.olb
$ makesup equifor	WORLIB:geolib.olb
$ makesup equiinv	WORLIB:geolib.olb
$ makesup for_init	WORLIB:geolib.olb
$ makesup gctp 		WORLIB:geolib.olb
$ makesup gnomfor	WORLIB:geolib.olb
$ makesup gnominv	WORLIB:geolib.olb
$ makesup goodfor	WORLIB:geolib.olb
$ makesup goodinv	WORLIB:geolib.olb
$ makesup gvnspfor	WORLIB:geolib.olb
$ makesup gvnspinv	WORLIB:geolib.olb
$ makesup hamfor	WORLIB:geolib.olb
$ makesup haminv	WORLIB:geolib.olb
$ makesup imolwfor	WORLIB:geolib.olb
$ makesup imolwinv	WORLIB:geolib.olb
$ makesup inv_init	WORLIB:geolib.olb
$ makesup lamazfor	WORLIB:geolib.olb
$ makesup lamazinv	WORLIB:geolib.olb
$ makesup lamccfor	WORLIB:geolib.olb
$ makesup lamccinv	WORLIB:geolib.olb
$ makesup merfor	WORLIB:geolib.olb
$ makesup merinv	WORLIB:geolib.olb
$ makesup millfor	WORLIB:geolib.olb
$ makesup millinv	WORLIB:geolib.olb
$ makesup molwfor	WORLIB:geolib.olb
$ makesup molwinv	WORLIB:geolib.olb
$ makesup obleqfor	WORLIB:geolib.olb
$ makesup obleqinv	WORLIB:geolib.olb
$ makesup omerfor	WORLIB:geolib.olb
$ makesup omerinv	WORLIB:geolib.olb
$ makesup orthfor	WORLIB:geolib.olb
$ makesup orthinv	WORLIB:geolib.olb
$ makesup paksz		WORLIB:geolib.olb
$ makesup polyfor	WORLIB:geolib.olb
$ makesup polyinv	WORLIB:geolib.olb
$ makesup psfor		WORLIB:geolib.olb
$ makesup psinv		WORLIB:geolib.olb
$ makesup report	WORLIB:geolib.olb
$ makesup robfor	WORLIB:geolib.olb
$ makesup robinv	WORLIB:geolib.olb
$ makesup sinfor	WORLIB:geolib.olb
$ makesup sininv	WORLIB:geolib.olb
$ makesup somfor	WORLIB:geolib.olb
$ makesup sominv	WORLIB:geolib.olb
$ makesup sphdz		WORLIB:geolib.olb
$ makesup sterfor	WORLIB:geolib.olb
$ makesup sterinv	WORLIB:geolib.olb
$ makesup stplnfor	WORLIB:geolib.olb
$ makesup stplninv	WORLIB:geolib.olb
$ makesup tmfor		WORLIB:geolib.olb
$ makesup tminv		WORLIB:geolib.olb
$ makesup untfz		WORLIB:geolib.olb
$ makesup vandgfor	WORLIB:geolib.olb
$ makesup vandginv	WORLIB:geolib.olb
$ makesup wivfor	WORLIB:geolib.olb
$ makesup wivinv	WORLIB:geolib.olb
$ makesup wviifor	WORLIB:geolib.olb
$ makesup wviiinv	WORLIB:geolib.olb
$ exit
