/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/* determine constant small m */
#include <math.h>
#include <projects.h>
	double
pj_msfn(double sinphi, double cosphi, double es) {
	return (cosphi / sqrt (1. - es * sinphi * sinphi));
}
