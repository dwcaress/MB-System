/* File: structDefs.cpp
 * -------------------
 * Written by: Debbie Meduna
 * 2014/??/??: Henthorn. Updated to support client/server architecture.
 * 2016/12/19: Henthorn. Updated to pass log dir name and particle file name to
 *             the server during initialization.
 *****************************************************************************/

#include "structDefs.h"
#include "genFilterDefs.h"
#include "trn_common.h"
#include "trn_log.h"
#include "MathP.h"

// Use a macro to standardize extracting the message from an exception
//
#ifdef _QNX
#include "Exception.h"
#define EXP_MSG e.msg
#else
#include "myexcept.h"
#define EXP_MSG e.what()
#endif

#define BEAM_DEBUG 0
#define TEMP_BUF_BYTES 512

/*----------------------------------------------------------------------------
 /InitVars member functions
 /----------------------------------------------------------------------------*/
InitVars::InitVars()
{
    // initialize using compile-time defaults
    _ivars.xyz_sdev.x = X_STDDEV_INIT;
    _ivars.xyz_sdev.y = Y_STDDEV_INIT;
    _ivars.xyz_sdev.z = Z_STDDEV_INIT;
}

// initializing CTOR
InitVars::InitVars(double x, double y, double z)
{
    _ivars.xyz_sdev.x = x;
    _ivars.xyz_sdev.y = y;
    _ivars.xyz_sdev.z = z;
}

InitVars::InitVars(d_triplet_t *xyz)
{
    if(NULL!=xyz){
        _ivars.xyz_sdev.x = xyz->x;
        _ivars.xyz_sdev.y = xyz->y;
        _ivars.xyz_sdev.z = xyz->z;
    }
}

// copy CTOR
InitVars::InitVars(InitVars *xyz)
{
    if(NULL!=xyz){
        _ivars.xyz_sdev.x = xyz->x();
        _ivars.xyz_sdev.y = xyz->y();
        _ivars.xyz_sdev.z = xyz->z();
    }
}

// DTOR
InitVars::~InitVars(){
};

d_triplet_t *InitVars::getXYZ(d_triplet_t *dest){
    d_triplet_t *retval=NULL;
    if(NULL!=dest){
        memcpy(dest, &_ivars.xyz_sdev, sizeof(d_triplet_t));
        retval=dest;
    }
    return retval;
}

int InitVars::setXYZ(d_triplet_t *src){
    int retval=-1;
    if(NULL!=src){
        retval = setXYZ(src->x, src->y, src->z);
    }
    return retval;
}

int InitVars::setXYZ(double x, double y, double z){
    _ivars.xyz_sdev.x = x;
    _ivars.xyz_sdev.y = y;
    _ivars.xyz_sdev.z = z;
    return 0;
}

double InitVars::x(){
    return _ivars.xyz_sdev.x;
}
double InitVars::y(){
    return _ivars.xyz_sdev.y;
}
double InitVars::z(){
    return _ivars.xyz_sdev.z;
}

/*----------------------------------------------------------------------------
/mapT member functions
/----------------------------------------------------------------------------*/
mapT::mapT()
:
depths(),
depthVariance(),
xpts(NULL),
ypts(NULL),
dx(0.),
dy(0.),
xcen(0.),
ycen(0.),
numX(0),
numY(0)
{
}

mapT::~mapT() {
	clean();
}

void mapT::clean() {
	if(xpts != NULL) {
		delete [] xpts;
	}

  if(ypts != NULL) {
		delete [] ypts;
	}

	depths.CleanUp();
	depthVariance.CleanUp();
}
#ifdef WITH_RESAMPLE_MAP
// TODO remove mapT::reSampleMap (unused)
void mapT::reSampleMap(const double newRes) {
	int newNumX, newNumY;
	double* xptsNew;
	double* yptsNew;
	Matrix depthsNew;
	Matrix depthVarNew;

	//Fill in new xpts/ypts vectors
	newNumX = int(round(fabs(xpts[numX - 1] - xpts[0]) / newRes)) + 1;
	newNumY = int(round(fabs(ypts[numY - 1] - ypts[0]) / newRes)) + 1;
	depthsNew.ReSize(newNumX, newNumY);
	depthVariance.ReSize(newNumX, newNumY);

	xptsNew = new double[newNumX];
	yptsNew = new double[newNumY];

	for(int i = 0; i < newNumX; i++) {
		xptsNew[i] = xpts[0] + newRes * i;
	}

	for(int j = 0; j < newNumY; j++) {
		yptsNew[j] = ypts[0] + newRes * j;
	}

	//Fill in new depth values
	if((newRes > dx) | (newRes > dy)) {
        Matrix subDepthMap;

		//TO DO: Fix this so it actually computes correct average for newRes!!
		int subRes = int(ceil(newRes / dx));
		subDepthMap.ReSize(subRes, subRes);
		for(int i = 1; i <= newNumX; i++) {
			for(int j = 1; j <= newNumY; j++) {
				//fill in new depth values by averaging subMatrix
				subDepthMap = depths.SubMatrix((i - 1) * subRes + 1, i * subRes,
											   (j - 1) * subRes + 1, j * subRes);
				depthsNew(i, j) = double((1.0 / (subRes * subRes))) * subDepthMap.Sum();

				//fill in new depth variance values
				subDepthMap = depthVariance.SubMatrix((i - 1) * subRes + 1, i * subRes,
													  (j - 1) * subRes + 1, j * subRes);
				depthVarNew(i, j) = double((1.0 / (subRes * subRes))) * subDepthMap.Sum();
			}
		}
	} else {
		interp2mat(xpts, ypts, depths, xptsNew, yptsNew, depthsNew);
	}

	//Remove old map and assign new values;
	clean();
	dx = newRes;
	dy = newRes;
	numX = newNumX;
	numY = newNumY;
	xpts = xptsNew;
	ypts = yptsNew;
	depths = depthsNew;
	depthVariance = depthVarNew;
	xcen = (xpts[numX - 1] + xpts[0]) / 2.0;
	ycen = (ypts[numY - 1] + ypts[0]) / 2.0;
}
#endif

//subSample the stored map to a lower resolution.
void mapT::subSampleMap(const int subRes) {
	double newResX, newResY;
	int newNumX, newNumY, count, i, j;
	double* xptsNew;
	double* yptsNew;
	Matrix depthsNew;
	Matrix depthVarNew;
	Matrix subDepthMap(subRes, subRes);

	//Fill in new xpts/ypts vectors
	newNumX = int(numX / subRes);
	newNumY = int(numY / subRes);
	depthsNew.ReSize(newNumX, newNumY);
	depthVarNew.ReSize(newNumX, newNumY);

	xptsNew = new double[newNumX];
	yptsNew = new double[newNumY];

	count = 0;
	for(i = 0; i < numX && count < newNumX; i = i + subRes) {
		xptsNew[count] = xpts[i];
		count++;
	}

	count = 0;
	for(j = 0; j < numY && count < newNumY; j = j + subRes) {
		yptsNew[count] = ypts[j];
		count++;
	}

	//Fill in new depths matrix by averaging depths in the cells;
	for(i = 1; i <= newNumX; i++) {
		for(j = 1; j <= newNumY; j++) {
			//fill in new depth values by averaging subMatrix
			subDepthMap = depths.SubMatrix((i - 1) * subRes + 1, i * subRes,
										   (j - 1) * subRes + 1, j * subRes);
			depthsNew(i, j) = double((1.0 / (subRes * subRes))) * subDepthMap.Sum();

			//fill in new depth variance values
			subDepthMap = depths.SubMatrix((i - 1) * subRes + 1, i * subRes,
										   (j - 1) * subRes + 1, j * subRes)
						  - depthsNew(i, j);
			subDepthMap = SP(subDepthMap, subDepthMap);
			depthVarNew(i, j) = double((1.0 / (subRes * subRes))) * subDepthMap.Sum();
		}
	}

	newResX = dx * subRes;
	newResY = dy * subRes;

	//Remove old map and assign new values;
	clean();
	dx = newResX;
	dy = newResY;
	numX = newNumX;
	numY = newNumY;
	xpts = xptsNew;
	ypts = yptsNew;
	depths = depthsNew;
	depthVariance = depthVarNew;
	xcen = (xpts[numX - 1] + xpts[0]) / 2.0;
	ycen = (ypts[numY - 1] + ypts[0]) / 2.0;
}

//display map values in a more readable format
void mapT::displayMap() {
	int i;

	//print a blank space in upper left corner
	LOGM("%5s", "");
	LOGM("y:");

	//display ypt values
	for(i = 0; i < numY; i++) {
		LOGM("%5.2f", ypts[i]);
	}
	LOGM("\n x: \n");

	//display xpt values and depth values
	for(i = 0; i < numX; i++) {
		LOGM("%5.2f", xpts[i]);
		LOGM("%2s", "");
		for(int j = 0; j < numY; j++) {
			LOGM("%5.2f", depths(i + 1, j + 1));
		}

		LOGM("\n");
	}
}

//copy assignment operator
mapT& mapT::operator=(mapT& rhs) {
	if(this != &rhs) {
		this->clean();

		//copy non-array values
		dx = rhs.dx;
		dy = rhs.dy;
		xcen = rhs.xcen;
		ycen = rhs.ycen;
		numX = rhs.numX;
		numY = rhs.numY;
		depths = rhs.depths;
		depthVariance = rhs.depthVariance;

		//copy array values
		xpts = new double[numX];
		ypts = new double[numY];
		for(int i = 0; i < numX; i++) {
			xpts[i] = rhs.xpts[i];
		}
		for(int j = 0; j < numY; j++) {
			ypts[j] = rhs.ypts[j];
		}

	}
	return(*this);
}


/*----------------------------------------------------------------------------
/poseT member functions
/----------------------------------------------------------------------------*/
poseT::poseT() {
	int i;

	//initialize values to zero
	x = y = z = 0.0;
	vx = vy = vz = ve = 0.0;
	vw_x = vw_y = vw_z = 0.0;
	vn_x = vn_y = vn_z = 0.0;
	wx = wy = wz = 0.0;
	ax = ay = az = 0.0;
	phi = theta = psi = 0.0;
	psi_berg = psi_dot_berg = 0.0;

	time = 0.0;

	dvlValid   = false;
	gpsValid   = false;
	bottomLock = false;

	for(i = 0; i < N_COVAR; i++) {
		covariance[i] = 0.0;
	}
}


//copy assignment operator
poseT& poseT::operator=(const poseT& rhs) {
	if(this != &rhs) {
		//copy non-array values
		x    = rhs.x;     y     = rhs.y;      z    = rhs.z;
		vx   = rhs.vx;    vy    = rhs.vy;     vz   = rhs.vz;    ve = rhs.ve;
		vw_x = rhs.vw_x;  vw_y  = rhs.vw_y;   vw_z = rhs.vw_z;
		vn_x = rhs.vn_x;  vn_y  = rhs.vn_y;   vn_z = rhs.vn_z;
		wx   = rhs.wx;    wy    = rhs.wy;     wz   = rhs.wz;
		ax   = rhs.ax;    ay    = rhs.ay;     az   = rhs.az;
		phi  = rhs.phi;   theta = rhs.theta;  psi  = rhs.psi;
		psi_berg = rhs.psi_berg;   psi_dot_berg = rhs.psi_dot_berg;

		time = rhs.time;

		dvlValid   = rhs.dvlValid;
		gpsValid   = rhs.gpsValid;
		bottomLock = rhs.bottomLock;

		//copy array values
		for(int i = 0; i < N_COVAR; i++) {
			covariance[i] = rhs.covariance[i];
		}
	}

	return(*this);
}

//difference assignment operator
poseT& poseT::operator-=(const poseT& rhs) {

	x    -= rhs.x;     y     -= rhs.y;      z    -= rhs.z;
	vx   -= rhs.vx;    vy    -= rhs.vy;     vz   -= rhs.vz;    ve -= rhs.ve;
	vw_x -= rhs.vw_x;  vw_y  -= rhs.vw_y;   vw_z -= rhs.vw_z;
	vn_x -= rhs.vn_x;  vn_y  -= rhs.vn_y;   vn_z -= rhs.vn_z;
	wx   -= rhs.wx;    wy    -= rhs.wy;     wz   -= rhs.wz;
	ax   -= rhs.ax;    ay    -= rhs.ay;     az   -= rhs.az;
	phi  -= rhs.phi;   theta -= rhs.theta;  psi  -= rhs.psi;
   psi_berg -= rhs.psi_berg;   psi_dot_berg -= rhs.psi_dot_berg;

	time -= rhs.time;

	dvlValid   = (dvlValid && rhs.dvlValid);
	gpsValid   = (gpsValid && rhs.gpsValid);
	bottomLock = (bottomLock && rhs.bottomLock);

	return(*this);
}

//addition assignment operator
poseT& poseT::operator+=(const poseT& rhs) {

	x    += rhs.x;     y     += rhs.y;      z    += rhs.z;
	vx   += rhs.vx;    vy    += rhs.vy;     vz   += rhs.vz;    ve += rhs.ve;
	vw_x += rhs.vw_x;  vw_y  += rhs.vw_y;   vw_z += rhs.vw_z;
	vn_x += rhs.vn_x;  vn_y  += rhs.vn_y;   vn_z += rhs.vn_z;
	wx   += rhs.wx;    wy    += rhs.wy;     wz   += rhs.wz;
	ax   += rhs.ax;    ay    += rhs.ay;     az   += rhs.az;
	phi  += rhs.phi;   theta += rhs.theta;  psi  += rhs.psi;
   psi_berg += rhs.psi_berg;   psi_dot_berg += rhs.psi_dot_berg;

	time += rhs.time;

	dvlValid   = (dvlValid && rhs.dvlValid);
	gpsValid   = (gpsValid && rhs.gpsValid);
	bottomLock = (bottomLock && rhs.bottomLock);

	return(*this);
}

// Returns the number of bytes in serialized poseT when successful.
// Returns < 0 when there is insufficient space (difference between
// required and given)
//
int poseT::serialize(char* buf, int buflen) {
	// Does the buffer have enough space?
	//
	int len = (N_COVAR+25) * sizeof(double) + 3 * sizeof(char);
	if(len > buflen) {
		return (buflen - len);    // Space mismatch is returned to caller
	}

	//printf("Serializing poseT of size: %d\n",len);

	// Copy contents of m into buf
	//
	len = 0;

	memcpy(&buf[len], &x,    sizeof(double)); len += sizeof(double);
	memcpy(&buf[len], &y,    sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &z,    sizeof(double));	len += sizeof(double);

	memcpy(&buf[len], &vx,   sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &vy,   sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &vz,   sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &ve,   sizeof(double));	len += sizeof(double);

	memcpy(&buf[len], &vw_x, sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &vw_y, sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &vw_z, sizeof(double));	len += sizeof(double);

	memcpy(&buf[len], &vn_x, sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &vn_y, sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &vn_z, sizeof(double));	len += sizeof(double);

	memcpy(&buf[len], &wx,   sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &wy,   sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &wz,   sizeof(double));	len += sizeof(double);

	memcpy(&buf[len], &ax,   sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &ay,   sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &az,   sizeof(double));	len += sizeof(double);

	memcpy(&buf[len], &phi,  sizeof(double));	len += sizeof(double);
	memcpy(&buf[len], &theta,sizeof(double)); len += sizeof(double);
	memcpy(&buf[len], &psi,  sizeof(double));	len += sizeof(double);

	memcpy(&buf[len], &psi_berg,     sizeof(double)); len += sizeof(double);
	memcpy(&buf[len], &psi_dot_berg, sizeof(double)); len += sizeof(double);

	memcpy(&buf[len], &time, sizeof(double));	len += sizeof(double);
	//printf("poseT time:%f\n", time);

	memcpy(&buf[len], &covariance, N_COVAR * sizeof(double));
	len += N_COVAR * sizeof(double);

	// Use one byte for serialized booleans
	buf[len++] = dvlValid   ? 0x01 : 0x00;
	buf[len++] = gpsValid   ? 0x01 : 0x00;
	buf[len++] = bottomLock ? 0x01 : 0x00;

	return len;
}

// Returns the number of bytes in serialized poseT when successful.
// Returns < 0 when there is insufficient space (difference between
// required and given)
//
int poseT::unserialize(char* buf, int buflen) {
	// Does the buffer have enough space?
	//
	int len = (N_COVAR+25) * sizeof(double) + 3 * sizeof(char);
	if(len > buflen) {
		return (buflen - len);
	}

	//printf("UnSerializing poseT of size: %d\n", len);

	// Copy contents of m into buf
	//
	len = 0;

	memcpy(&x,    &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&y,    &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&z,    &buf[len], sizeof(double));	len += sizeof(double);

	memcpy(&vx,   &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&vy,   &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&vz,   &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&ve,   &buf[len], sizeof(double));	len += sizeof(double);

	memcpy(&vw_x, &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&vw_y, &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&vw_z, &buf[len], sizeof(double));	len += sizeof(double);

	memcpy(&vn_x, &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&vn_y, &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&vn_z, &buf[len], sizeof(double));	len += sizeof(double);

	memcpy(&wx,   &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&wy,   &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&wz,   &buf[len], sizeof(double));	len += sizeof(double);

	memcpy(&ax,   &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&ay,   &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&az,   &buf[len], sizeof(double));	len += sizeof(double);

	memcpy(&phi,  &buf[len], sizeof(double));	len += sizeof(double);
	memcpy(&theta, &buf[len],sizeof(double));	len += sizeof(double);
	memcpy(&psi,  &buf[len], sizeof(double));	len += sizeof(double);

	memcpy(&psi_berg, &buf[len], sizeof(double)); len += sizeof(double);
	memcpy(&psi_dot_berg, &buf[len], sizeof(double)); len += sizeof(double);

	memcpy(&time, &buf[len], sizeof(double));	len += sizeof(double);
	//printf("poseT time:%f\n", time);

	memcpy(&covariance, &buf[len], N_COVAR * sizeof(double));
	len += N_COVAR * sizeof(double);

	dvlValid   = buf[len++] == 0x01;
	gpsValid   = buf[len++] == 0x01;
	bottomLock = buf[len++] == 0x01;

	return len;
}

/*----------------------------------------------------------------------------
/measT member functions
/----------------------------------------------------------------------------*/
measT::measT() {
   time = phi = theta = psi = x = y = z = 0.;
   dataType = numMeas = ping_number = 0;
	covariance = NULL;
	ranges = NULL;
	crossTrack = NULL;
	alongTrack = NULL;
	altitudes = NULL;
	alphas = NULL;
	measStatus = NULL;
	beamNums = NULL;
	ping_number = 0;
	numMeas = 0;
}

/*----------------------------------------------------------------------------
/measT ctor with datatype and numMeas
/----------------------------------------------------------------------------*/

measT::measT(unsigned int nummeas, int datatype)
{
    time = phi = theta = psi = x = y = z = 0.;
    dataType = datatype;
    numMeas = nummeas;
    ping_number = 0;
    size_t asz = numMeas*sizeof(double);
    size_t iasz = numMeas*sizeof(int);
    size_t basz = numMeas*sizeof(bool);

    // use malloc b/c some apps (Replay)
    // need to resize them and realloc
    // is not guaranteed to work correctly
    // with new/delete
    covariance = (double *)malloc(asz);
    memset(covariance, 0, asz);
    ranges = (double *)malloc(asz);
    memset(ranges, 0, asz);
    crossTrack = (double *)malloc(asz);
    memset(crossTrack, 0, asz);
    alongTrack = (double *)malloc(asz);
    memset(alongTrack, 0, asz);
    altitudes = (double *)malloc(asz);
    memset(altitudes, 0, asz);
    alphas = (double *)malloc(asz);
    memset(alphas, 0, asz);
    measStatus = (bool *)malloc(basz);
    memset(measStatus, 0, basz);
    beamNums = (int *)malloc(iasz);
    memset(beamNums, 0, iasz);

}

measT::~measT() {
	// although clean() exists
    // maybe preferable not to
    // call methods in destructor
    free(covariance);
    free(ranges);
    free(crossTrack);
    free(alongTrack);
    free(altitudes);
    free(alphas);
    free(measStatus);
    free(beamNums);
}

// release all dynamic memory resources of the struct
void measT::clean() {
    
    free(covariance);
    free(ranges);
    free(crossTrack);
    free(alongTrack);
    free(altitudes);
    free(alphas);
    free(measStatus);
    free(beamNums);

	covariance = NULL;
	ranges = NULL;
	crossTrack = NULL;
	alongTrack = NULL;
	altitudes = NULL;
	alphas = NULL;
	measStatus = NULL;
	beamNums = NULL;

	time = 0.;

	ping_number = 0;
	numMeas = 0;
}

//copy assignment operator
measT& measT::operator=(const measT& rhs) {
	if(this != &rhs) {
		//if the two measT structs have different datatype or number of
		//measurements, we need to delete and recreate memory for the
		//new measT struct.
        size_t dasz = rhs.numMeas*sizeof(double);
        size_t iasz = rhs.numMeas*sizeof(int);
        size_t basz = rhs.numMeas*sizeof(bool);
		if(numMeas != rhs.numMeas || dataType != rhs.dataType) {
			this->clean();
			if(rhs.dataType == TRN_SENSOR_MB ||
			   rhs.dataType == TRN_SENSOR_HOMER) {

                double *new_cross = (double *)realloc(crossTrack, dasz);
                double *new_along = (double *)realloc(alongTrack, dasz);
                double *new_alts = (double *)realloc(altitudes, dasz);
                double *new_ranges = (double *)realloc(ranges, dasz);

                if(NULL != new_cross){
                    crossTrack = new_cross;
                }else{
                    fprintf(stderr,"%s:%d WARN realloc failed for crossTrack\n",__func__,__LINE__);
                }
                if(NULL != new_along){
                    alongTrack = new_along;
                }else{
                    fprintf(stderr,"%s:%d WARN realloc failed for alongTrack\n",__func__,__LINE__);
                }
                if(NULL != new_alts){
                    altitudes = new_alts;
                }else{
                    fprintf(stderr,"%s:%d WARN realloc failed for altitudes\n",__func__,__LINE__);
                }
                if(NULL != new_ranges){
                    ranges = new_ranges;
                }else{
                    fprintf(stderr,"%s:%d WARN realloc failed for ranges\n",__func__,__LINE__);
                }

            } else {
                double *new_ranges = (double *)realloc(ranges, dasz);
                if(NULL != new_ranges){
                    ranges = new_ranges;
                }else{
                    fprintf(stderr,"%s:%d WARN realloc failed for ranges\n",__func__,__LINE__);
                }
			}

			// MB-sys beam numbers
            if(rhs.dataType == TRN_SENSOR_MB){
                int *new_beamn = (int *)realloc(beamNums, iasz);
                if(NULL != new_beamn){
                    beamNums = new_beamn;
                }else{
                    fprintf(stderr,"%s:%d WARN realloc failed for beamNums\n",__func__,__LINE__);
                }

            }
            double *new_alph = (double *)realloc(alphas, dasz);
            if( NULL != new_alph){
                alphas = new_alph;
            }else{
                fprintf(stderr,"%s:%d WARN realloc failed for alphas\n",__func__,__LINE__);
            }

            bool *new_mstat = (bool *)realloc(measStatus, basz);
            if(NULL != new_mstat){
                measStatus = new_mstat;
            }else{
                fprintf(stderr,"%s:%d WARN realloc failed for measStatus\n",__func__,__LINE__);
            }

		}

		if(rhs.covariance != NULL) {
            double *new_cov = (double *)realloc(covariance, dasz);
            if(NULL!=new_cov){
                covariance = new_cov;
            }else{
                fprintf(stderr,"%s:%d WARN realloc failed for covariance\n",__func__,__LINE__);
            }

		}

		//copy non-array values
		time = rhs.time;
		dataType = rhs.dataType;
		phi = rhs.phi;
		theta = rhs.theta;
		psi = rhs.psi;
		ping_number = rhs.ping_number;
		numMeas = rhs.numMeas;
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;

		//copy array values
		for(int i = 0; i < rhs.numMeas; i++) {
			if(rhs.dataType == TRN_SENSOR_MB ||
			   rhs.dataType == TRN_SENSOR_HOMER) {
				crossTrack[i] = rhs.crossTrack[i];
				alongTrack[i] = rhs.alongTrack[i];
				altitudes[i] = rhs.altitudes[i];
				ranges[i] = rhs.ranges[i];
			} else {
				ranges[i] = rhs.ranges[i];
			}

			// MB-sys beam numbers
			if(rhs.dataType == TRN_SENSOR_MB)
			  beamNums[i] = rhs.beamNums[i];

			alphas[i] = rhs.alphas[i];
			measStatus[i] = rhs.measStatus[i];
			if(rhs.covariance != NULL) {
				covariance[i] = rhs.covariance[i];
			}
		}

	}
	return(*this);
}

// Returns the number of bytes in serialized measT when successful.
// Returns < 0 when there is insufficient space (difference between
// required and given)
//
int measT::serialize(char* buf, int buflen) {
        // fprintf(stderr, "Serializing measT of type %d...", dataType);
	// Does the buffer have enough space?
	//
	// Fixed length parts
	int nm = numMeas;
	int len = sizeof(this->time) +
						sizeof(dataType) +
						sizeof(phi) +
						sizeof(theta) +
						sizeof(psi) +
						sizeof(x) +
						sizeof(y) +
						sizeof(z) +
						sizeof(ping_number) +
						sizeof(numMeas);

	// altitudes/along/cross-Tracks, or just ranges?
	if(dataType == TRN_SENSOR_MB ||
	   dataType == TRN_SENSOR_HOMER) {
		len += (nm * 4) * sizeof(double);
	} else {
		len += nm * sizeof(double);
	}

	// Status flags
	len += nm * sizeof(char);

	// alphas
	len += sizeof(double);

	// beam numbers
	if(dataType == TRN_SENSOR_MB ) len += nm * sizeof(int);

	// Covariances?
	if(covariance) {
		len += nm * sizeof(double);
	} else {
		dataType = 0 - dataType;    // Signal lack of covariances with a dataType < 0
	}

	// fprintf(stderr, "...with %d measurements, size: %d\n", nm, len);

	// If the buffer has insufficient starage, return the difference
	//
	if(len > buflen) {
		return (buflen - len);
	}

	// Copy contents into buf. Order is significant!
	//
	len = 0;
	memcpy(&buf[len], &time,     sizeof(double));
	len += sizeof(double);
	memcpy(&buf[len], &dataType, sizeof(int));
	len += sizeof(int);
	memcpy(&buf[len], &phi,      sizeof(double));
	len += sizeof(double);
	memcpy(&buf[len], &theta,    sizeof(double));
	len += sizeof(double);
	memcpy(&buf[len], &psi,      sizeof(double));
	len += sizeof(double);
	memcpy(&buf[len], &x,        sizeof(double));
	len += sizeof(double);
	memcpy(&buf[len], &y,        sizeof(double));
	len += sizeof(double);
	memcpy(&buf[len], &z,        sizeof(double));
	len += sizeof(double);
	memcpy(&buf[len], &ping_number,  sizeof(unsigned int));
	len += sizeof(unsigned int);
	memcpy(&buf[len], &numMeas,  sizeof(int));
	len += sizeof(int);

	// Use one byte for serialized booleans
#if BEAM_DEBUG
	printf("\nSend measT %.2f ping# %d\n", time, ping_number);
	int mso = len;

  printf("Starting at offset: %d\n", mso);
	for(int m = 0; m < nm; m++) {
		printf("%x  ", buf[mso++]);
	}
  printf("\n");
#endif

	for(int i = 0; i < nm; i++) {
		buf[len++] = measStatus[i] ? 1 : 0;
#if BEAM_DEBUG
		if (i == nm-1) buf[len-1] = 'R';    // Throw in a recognizable value
		printf("\t\tSend measStatus[%d] == %d\n", i, measStatus[i]);
#endif
	}

	// Copy the arrays
	//
	if(abs(dataType) == TRN_SENSOR_MB ||
	   abs(dataType) == TRN_SENSOR_HOMER) {

#if BEAM_DEBUG
		crossTrack[nm-1] = 1.23; alongTrack[nm-1] = 2.34; altitudes[nm-1] = 3.45; ranges[nm-1] = 4.56;
		printf("\t\t%.2f ping # %d ct: %.2f at: %.2f a: %.2f r: %.2f ",
			this->time, ping_number,
			crossTrack[nm-1], alongTrack[nm-1], altitudes[nm-1], ranges[nm-1]);
#endif

		memcpy(&buf[len], crossTrack, nm * sizeof(double));
		len += nm * sizeof(double);
		memcpy(&buf[len], alongTrack, nm * sizeof(double));
		len += nm * sizeof(double);
#if BEAM_DEBUG
		for (int a = 0; a < nm; a++) altitudes[a] = a+1;
#endif
		memcpy(&buf[len], altitudes,  nm * sizeof(double));
		len += nm * sizeof(double);
		memcpy(&buf[len], ranges,     nm * sizeof(double));
		len += nm * sizeof(double);
	} else {
		memcpy(&buf[len], ranges,     nm * sizeof(double));
		len += nm * sizeof(double);
	}

	// MB-sys beam numbers
	if(abs(dataType) == TRN_SENSOR_MB && beamNums != NULL)
	{
#if BEAM_DEBUG
		beamNums[nm-1] = 27;
#endif
		memcpy(&buf[len], beamNums, nm * sizeof(int));
		len += nm * sizeof(int);
	}

	if(covariance) {
		memcpy(&buf[len], covariance, nm * sizeof(double));
		len += nm * sizeof(double);
		//printf("measT has covariance values\n");
	}

   // alphas
#if BEAM_DEBUG
  alphas[nm-1] = 41.18;   // place a recognizable value here
#endif
  memcpy(&buf[len], alphas, nm * sizeof(double));
	len += nm * sizeof(double);

	// printf("\t\t\tserialize - alphas[0] = %f, len = %d\n", alphas[0], len);

#if BEAM_DEBUG
	printf("\nSend measT length %d with %d beams\n", len, nm);
#endif

	return len;
}

// Returns the number of bytes in serialized measT when successful.
// Returns < 0 when there is insufficient space (difference between
// required and given)
//
int measT::unserialize(char* buf, int buflen) { //TODO buflen is unused?
	// Copy contents of m into buf
	//
	int len = 0;
	clean();

	// Order is significant - must match serialize!
	//
	memcpy(&time,    &buf[len], sizeof(double));
	len += sizeof(double);
	memcpy(&dataType, &buf[len], sizeof(int));
	len += sizeof(int);
	memcpy(&phi,     &buf[len], sizeof(double));
	len += sizeof(double);
	memcpy(&theta,   &buf[len], sizeof(double));
	len += sizeof(double);
	memcpy(&psi,     &buf[len], sizeof(double));
	len += sizeof(double);
	memcpy(&x,       &buf[len], sizeof(double));
	len += sizeof(double);
	memcpy(&y,       &buf[len], sizeof(double));
	len += sizeof(double);
	memcpy(&z,       &buf[len], sizeof(double));
	len += sizeof(double);
	memcpy(&ping_number, &buf[len], sizeof(unsigned int));
	len += sizeof(unsigned int);
	memcpy(&numMeas, &buf[len], sizeof(int));
	len += sizeof(int);

	int nm = numMeas;
	//printf("UnSerializing measT with %d measurements, fixed len:%d...", nm, len);

	if(nm > 0) {

		// Serialized booleans are single bytes
		//
#if BEAM_DEBUG
		printf("\nRecv measT %.2f ping# %d\n", time, ping_number);
		int mso = len;
    printf("Starting at offset: %d\n", mso);
		for(int m = 0; m < nm; m++) {
			printf("%x  ", buf[mso++]);
		}
	  printf("\n");
#endif

        bool *new_mstat = (bool *)realloc(measStatus, nm*sizeof(bool));
        if(NULL != new_mstat){
            measStatus = new_mstat;
        }else{
            fprintf(stderr,"%s:%d WARN realloc failed for measStatus\n",__func__,__LINE__);
        }

		for(int i = 0; i < nm; i++) {
			measStatus[i] = (buf[len++] != 0 ? true : false);
#if BEAM_DEBUG
			printf("\t\tRecv measStatus[%d] == %d\n", i, measStatus[i]);
#endif
		}
        size_t dasz = nm*sizeof(double);
        size_t iasz = nm*sizeof(int);

        // Using ranges or tracks and altitudes?
		//
		if(abs(dataType) == TRN_SENSOR_MB ||
		   abs(dataType) == TRN_SENSOR_HOMER) {
            double *new_cross = (double *)realloc(crossTrack, dasz);
            double *new_along = (double *)realloc(alongTrack, dasz);
            double *new_alts = (double *)realloc(altitudes, dasz);
            double *new_ranges = (double *)realloc(ranges, dasz);

			// Again, order is significant
            if(NULL != new_cross){
                crossTrack = new_cross;
                memcpy(crossTrack, &buf[len], nm * sizeof(double));
                len += nm * sizeof(double);
            }else{
                fprintf(stderr,"%s:%d WARN realloc failed for crossTrack\n",__func__,__LINE__);
            }
            if(NULL != new_along){
                alongTrack = new_along;
                memcpy(alongTrack, &buf[len], nm * sizeof(double));
                len += nm * sizeof(double);
            }else{
                fprintf(stderr,"%s:%d WARN realloc failed for alongTrack\n",__func__,__LINE__);
            }
            if(NULL != new_alts){
                altitudes = new_alts;
                memcpy(altitudes,  &buf[len], nm * sizeof(double));
                len += nm * sizeof(double);
            }else{
                fprintf(stderr,"%s:%d WARN realloc failed for altitudes\n",__func__,__LINE__);
            }
            if(NULL != new_ranges){
                ranges = new_ranges;
                memcpy(ranges,     &buf[len], nm * sizeof(double));
                len += nm * sizeof(double);
            }else{
                fprintf(stderr,"%s:%d WARN realloc failed for ranges\n",__func__,__LINE__);
            }
		} else {
            double *new_ranges = (double *)realloc(ranges, dasz);
            if(NULL != new_ranges){
                ranges = new_ranges;
                memcpy(ranges,     &buf[len], nm * sizeof(double));
                len += nm * sizeof(double);
            }else{
                fprintf(stderr,"%s:%d WARN realloc failed for ranges\n",__func__,__LINE__);
            }
		}

		// MB-sys beam numbers
		if(abs(dataType) == TRN_SENSOR_MB) {
            int *new_beamn = (int *)realloc(beamNums, iasz);
            if(NULL != new_beamn){
                beamNums = new_beamn;
                memcpy(beamNums,  &buf[len], nm * sizeof(int));
                len += nm * sizeof(int);
            }else{
                fprintf(stderr,"%s:%d WARN realloc failed for beamNums\n",__func__,__LINE__);
            }
		}


		// A dataType of less than zero is a signal that there
		// are no covariances in this measT
		//
		if(dataType >= 0) {
            double *new_cov = (double *)realloc(covariance, dasz);
            if(NULL!=new_cov){
                covariance = new_cov;
                memcpy(covariance, &buf[len], nm * sizeof(double));
                len += nm * sizeof(double);
            }else{
                fprintf(stderr,"%s:%d WARN realloc failed for covariance\n",__func__,__LINE__);
            }
			//printf("measT has covariance values\n");

#if BEAM_DEBUG
			printf("\t\t%.2f ping # %d ct: %.2f at: %.2f a: %.2f r: %.2f beam %d\n",
				this->time, ping_number,
				crossTrack[nm-1], alongTrack[nm-1], altitudes[nm-1], ranges[nm-1],
				beamNums[nm-1]);
#endif


		} else {
			dataType = 0 - dataType;
		}
		if( dataType < 0 )
		  {
		    fprintf(stderr,"measT::unserialize dataType[%d]\n", dataType);
		  }
		// Alphas
        double *new_alph = (double *)realloc(alphas, dasz);
        if( NULL != new_alph){
            alphas = new_alph;
            memcpy(alphas, &buf[len], nm * sizeof(double));
            len += nm * sizeof(double);
        }else{
            fprintf(stderr,"%s:%d WARN realloc failed for alphas\n",__func__,__LINE__);
        }
		// printf("\t\t\tunserialize - alphas[0] = %f, len = %d\n", alphas[0], len);

	}

	return len;
}

/*----------------------------------------------------------------------------
/transformT member functions
/----------------------------------------------------------------------------*/
void transformT::displayTransformInfo() {
	LOGM("Rotation angles (phi, theta, psi): \n (%f ,%f, %f)\n",
		   rotation[0] * 180 / PI, rotation[1] * 180 / PI, rotation[2] * 180 / PI);
	LOGM("Translation vector [dx, dy, dz]: \n (%f ,%f, %f)\n", translation[0], translation[1],
		   translation[2]);
}

/*----------------------------------------------------------------------------
/sensorT member functions
/----------------------------------------------------------------------------*/
sensorT::sensorT()
:
numBeams(0),
percentRangeError(0.),
beamWidth(0.),
type(0),
T_bs(NULL)
{
    memset(name,0,256);
    memset(filename,0,300);
}

sensorT::sensorT(char* fileName)
:
numBeams(0),
percentRangeError(0.),
beamWidth(0.),
type(0),
T_bs(NULL)
{
    memset(name,0,256);
    memset(filename,0,300);
    // TODO: orig ctor indicated fileName arg unused
    // call parseSensorSpecs?
    //parseSensorSpecs(fileName);
}

sensorT::~sensorT() {
	if(T_bs != NULL) {
		delete [] T_bs;
	}
}

// throws an exception if the specs file could not be opened
//
void sensorT::parseSensorSpecs(char* fileName) {
	fstream sensorFile;
	char temp[512];

	strcpy(filename, fileName);
	sensorFile.open(fileName);
	if(sensorFile.is_open()) {
		//read in sensor name
		sensorFile.ignore(256, ':');
		sensorFile.getline(name, 256);

		//read in sensor type
		sensorFile.ignore(256, ':');
		sensorFile.getline(temp, 256);
		type = atoi(temp);
		LOGM("parseSensorSpecs parsing sensor of type %d.\n",
		       type);

		//read in number of beams
		sensorFile.ignore(256, ':');
		sensorFile.getline(temp, 256);
		numBeams = atoi(temp);

		//read in percent range error
		sensorFile.ignore(256, ':');
		sensorFile.getline(temp, 256);
		percentRangeError = atof(temp);

		//read in beam width
		sensorFile.ignore(256, ':');
		sensorFile.getline(temp, 256);
		beamWidth = atof(temp) * PI / 180.0;

		//read in beam information
		T_bs = new transformT[numBeams];

		if(type == 2) {
			sensorFile.ignore(256, ':');
			sensorFile.getline(temp, 256);
			T_bs[0].rotation[1] = atof(temp) * PI / 180.0;

			sensorFile.ignore(256, ':');
			sensorFile.getline(temp, 256);
			double dphi = atof(temp) * PI / 180.0;

			sensorFile.ignore(256, ':');
			sensorFile.getline(temp, 256);
			T_bs[0].rotation[2] = atof(temp) * PI / 180.0;

			sensorFile.ignore(256, ':');
			sensorFile.getline(temp, 256);
			double dpsi = atof(temp) * PI / 180.0;

			for(int i = 0; i < numBeams; i++) {
				T_bs[i].rotation[1] = T_bs[0].rotation[1] + i * dphi;
				T_bs[i].rotation[2] = T_bs[0].rotation[2] + i * dpsi;
				T_bs[i].rotation[0] = 0.0;
				T_bs[i].translation[0] = 0.0;
				T_bs[i].translation[1] = 0.0;
				T_bs[i].translation[2] = 0.0;
			}
		}

		else if(type==5)
		{
			sensorFile.ignore(256,':');
			sensorFile.getline(temp,256);
			double init_phi = atof(temp)*PI/180.0;

			sensorFile.ignore(256,':');
			sensorFile.getline(temp,256);
			double dphi = atof(temp)*PI/180.0;

			// Centered on middle beam pointing down with beam 1 at the back
			for(int i = 0; i < numBeams; i++)
			{
				T_bs[i].rotation[1] = init_phi - dphi*numBeams/2 + i*dphi;
				T_bs[i].rotation[2] = 0.0;
				T_bs[i].rotation[0] = 0.0;
				T_bs[i].translation[0] = 0.0;
				T_bs[i].translation[1] = 0.0;
				T_bs[i].translation[2] = 0.0;
			}
		}
		// DVL and others
			else {
			//beam pitch angle
			sensorFile.ignore(256, ':');
			for(int i = 0; i < numBeams; i++) {
				if(i < numBeams - 1) {
					sensorFile.getline(temp, 10, ',');
				} else {
					sensorFile.getline(temp, 10);
				}
				T_bs[i].rotation[1] = atof(temp) * PI / 180.0;
				T_bs[i].rotation[0] = 0.0;
				T_bs[i].translation[0] = 0.0;
				T_bs[i].translation[1] = 0.0;
				T_bs[i].translation[2] = 0.0;
			}

			//beam yaw angle
			sensorFile.ignore(256, ':');
			for(int i = 0; i < numBeams; i++) {
				if(i < numBeams - 1) {
					sensorFile.getline(temp, 10, ',');
				} else {
					sensorFile.getline(temp, 10);
				}
				T_bs[i].rotation[2] = atof(temp) * PI / 180.0;
			}
		}

		sensorFile.close();

	} else {
      // Throw exception on file open error rather than exit
      snprintf(temp, 512, "Error opening sensor file %s...\n", fileName);
      fprintf(stderr, "%s: %s", __func__, temp);
      throw Exception(temp);
	}

	return;
}

void sensorT::displaySensorInfo() {
	LOGM("Sensor name: %s\n", name);
	LOGM("Sensor type: %i\n", type);
	LOGM("Number of beams per measurement: %i\n", numBeams);
}


/*----------------------------------------------------------------------------
/vehicleT member functions
/----------------------------------------------------------------------------*/
vehicleT::vehicleT()
:
numSensors(0),
driftRate(0.),
T_sv(NULL),
sensors(NULL)
{
    memset(name,0,256);
}

vehicleT::vehicleT(char* fileName)
:
numSensors(0),
driftRate(0.),
T_sv(NULL),
sensors(NULL)
{
    memset(name,0,256);
	parseVehicleSpecs(fileName);
}

vehicleT::~vehicleT() {
	if(T_sv != NULL) {
		delete [] T_sv;
	}

	if(sensors != NULL) {
		delete [] sensors;
	}
}

// throws an exception if the specs file could not be opened
//
void vehicleT::parseVehicleSpecs(char* fileName){
    fstream vehicleFile;

    try {
        char temp[TEMP_BUF_BYTES]={0};
        vehicleFile.open(fileName);
        if(vehicleFile.is_open()) {
            char temp2[TEMP_BUF_BYTES]={0};
            char sensorFile[1024]={0};
            //read in vehicleName
            vehicleFile.ignore(256, ':');
            vehicleFile.getline(name, 256);

            //read in number of sensors
            vehicleFile.ignore(256, ':');
            vehicleFile.getline(temp, 256);
            numSensors = atoi(temp);

            //read in INS drift rate
            vehicleFile.ignore(256, ':');
            vehicleFile.getline(temp, 256);
            driftRate = atof(temp);

            //read in sensor information
            sensors = new sensorT[numSensors];
            T_sv = new transformT[numSensors];

            for(int i = 0; i < numSensors; i++) {
                //sensor name
                vehicleFile.ignore(256, ':');
                vehicleFile.getline(sensors[i].name, 256);

                //sensor orientation offset
                vehicleFile.ignore(256, ':');
                vehicleFile.getline(temp, 10, ',');
                T_sv[i].rotation[0] = atof(temp) * PI / 180.0;
                vehicleFile.getline(temp, 10, ',');
                T_sv[i].rotation[1] = atof(temp) * PI / 180.0;
                vehicleFile.getline(temp, 10);
                T_sv[i].rotation[2] = atof(temp) * PI / 180.0;

                //sensor translational offset
                vehicleFile.ignore(256, ':');
                vehicleFile.getline(temp, 10, ',');
                T_sv[i].translation[0] = atof(temp);
                vehicleFile.getline(temp, 10, ',');
                T_sv[i].translation[1] = atof(temp);
                vehicleFile.getline(temp, 10);
                T_sv[i].translation[2] = atof(temp);

                //extract file directory
                strcpy(sensorFile, fileName);
                char *sensorPath = strstr(sensorFile, name);

                //determine sensor file name
                snprintf(temp2, TEMP_BUF_BYTES, "%s%s", sensors[i].name, "_specs.cfg\0");
                strcpy(sensorPath, temp2);

                //parse sensor file
                sensors[i].parseSensorSpecs(sensorFile);
                LOGM("Sensor %d is of type %d.\n",
                     i, sensors[i].type);
            }

            vehicleFile.close();
        } else {
            snprintf(temp,TEMP_BUF_BYTES, "Error opening file %s...\n", fileName);
            fprintf(stderr, "%s: %s", __func__, temp);
            throw Exception(temp);
        }

    } catch (Exception& e){
        // Report, close specs file, re-throw
        //
        fprintf(stderr, "structDefs::parseVehicleSpecs() - %s\n", EXP_MSG);
        if (vehicleFile.is_open()) vehicleFile.close();
        throw(e);
    }

    return;
}

void vehicleT::displayVehicleInfo() {
	int i;
    static char obuf[1024];
    char *bp=obuf;
    memset(obuf,0,1024);

    snprintf(bp, 1024, "Vehicle name: %s\n", name);
    bp=obuf+strlen(obuf);
    snprintf(bp, 1024-(bp-obuf+1), "Number of sensors: %i\n\n", numSensors);
    bp=obuf+strlen(obuf);
    LOGM(obuf);
//	LOGM("Vehicle name: %s\n", name);
//	LOGM("Number of sensors: %i\n\n", numSensors);

	for(i = 0; i < numSensors; i++) {
		LOGM("Sensor #%i: \n", i + 1);
		sensors[i].displaySensorInfo();

		LOGM("Sensor #%i to vehicle transformation information: \n", i + 1);
		T_sv[i].displayTransformInfo();
		LOGM("\n");
	}

}

/*----------------------------------------------------------------------------
/commsT member functions
/----------------------------------------------------------------------------*/

commsT::commsT()
:
msg_type(0),
parameter(0),
vdr(0.0),
pt(),
mt(),
xyz_sdev(),
est_nav_ofs(),
mapname(NULL),
cfgname(NULL),
particlename(NULL),
logname(NULL)
{
}

commsT::commsT(char type)
:
msg_type(type),
parameter(0),
vdr(0.0),
pt(),
mt(),
xyz_sdev(),
est_nav_ofs(),
mapname(NULL),
cfgname(NULL),
particlename(NULL),
logname(NULL)
{
}

commsT::commsT(char type, int param)
:
msg_type(type),
parameter(param),
vdr(0.0),
pt(),
mt(),
xyz_sdev(),
est_nav_ofs(),
mapname(NULL),
cfgname(NULL),
particlename(NULL),
logname(NULL)
{
}

commsT::commsT(char type, int param, float dr)// TODO char param unused
:
msg_type(type),
parameter(0),
vdr(dr),
pt(),
mt(),
xyz_sdev(),
est_nav_ofs(),
mapname(NULL),
cfgname(NULL),
particlename(NULL),
logname(NULL)
{
}

commsT::commsT(char type, int param, char* map, char* cfg,
	            char* partfile, char* logdir)
:
msg_type(type),
parameter(param),
vdr(0.0),
pt(),
mt(),
xyz_sdev(),
est_nav_ofs(),
mapname(NULL),
cfgname(NULL),
particlename(NULL),
logname(NULL)
{
    mapname = STRDUPNULL(map);
    cfgname = STRDUPNULL(cfg);
    logname = STRDUPNULL(logdir);
    particlename = STRDUPNULL(partfile);

    if (!(map && cfg && logdir && partfile)){
        fprintf(stderr,"%s: WARNING - converted NULL parameters to empty string\n",
                __func__);
    }
}

commsT::commsT(char type, int param, const measT& m)
:
msg_type(type),
parameter(param),
vdr(0.0),
pt(),
mt(),
xyz_sdev(),
est_nav_ofs(),
mapname(NULL),
cfgname(NULL),
particlename(NULL),
logname(NULL)
{
    // Measure update message?
    //
    if(msg_type == TRN_MEAS) {
        mt = m;
    } else {
        fprintf(stderr,"%s: MU msg NOT created\n", __func__);
    }
    //printf("MU msg created\n");
}

commsT::commsT(char type, const poseT& p)
:
msg_type(type),
parameter(0),
vdr(0.0),
pt(),
mt(),
xyz_sdev(),
est_nav_ofs(),
mapname(NULL),
cfgname(NULL),
particlename(NULL),
logname(NULL)
{
	// Measure update message?
	//
	if((msg_type == TRN_MOTN || msg_type == TRN_MLE || msg_type == TRN_MMSE ||
			msg_type == TRN_ACK)) {
		pt = p;
	} else {
		fprintf(stderr,"%s, EP/ACK msg NOT created\n", __func__);
	}
}

commsT::commsT(char type, double x, double y, double z)
:
msg_type(type),
parameter(0),
vdr(0.0),
pt(),
mt(),
xyz_sdev(),
est_nav_ofs(),
mapname(NULL),
cfgname(NULL),
particlename(NULL),
logname(NULL)
{
    if( (msg_type == TRN_SET_INITSTDDEVXYZ) ||
       (msg_type == TRN_GET_INITSTDDEVXYZ)) {
        xyz_sdev.x = x;
        xyz_sdev.y = y;
        xyz_sdev.z = z;
    }else if( (msg_type == TRN_SET_ESTNAVOFS) ||
             (msg_type == TRN_GET_ESTNAVOFS)) {
        est_nav_ofs.x = x;
        est_nav_ofs.y = y;
        est_nav_ofs.z = z;
    }else {
        fprintf(stderr,"%s: SET_INITSTDDEVXYZ/SET_ESTNAVOFS msg NOT created\n", __func__);
    }
}

commsT::commsT(char type, int param, double ofs_x, double ofs_y, double ofs_z)
:
msg_type(type),
parameter(param),
vdr(0.0),
pt(),
mt(),
xyz_sdev(),
est_nav_ofs(),
mapname(NULL),
cfgname(NULL),
particlename(NULL),
logname(NULL)
{
    if(msg_type == TRN_FILT_REINIT_OFFSET) {
        est_nav_ofs.x = ofs_x;
        est_nav_ofs.y = ofs_y;
        est_nav_ofs.z = ofs_z;
    } else {
        fprintf(stderr,"%s: FILT_REINIT_OFFSET msg NOT created\n", __func__);
    }
}

commsT::commsT(char type, int param, double ofs_x, double ofs_y, double ofs_z,
               double sdev_x, double sdev_y, double sdev_z)
:
msg_type(type),
parameter(param),
vdr(0.0),
pt(),
mt(),
xyz_sdev(),
est_nav_ofs(),
mapname(NULL),
cfgname(NULL),
particlename(NULL),
logname(NULL) {
    if(msg_type == TRN_FILT_REINIT_BOX) {
        xyz_sdev.x = sdev_x;
        xyz_sdev.y = sdev_y;
        xyz_sdev.z = sdev_z;
        est_nav_ofs.x = ofs_x;
        est_nav_ofs.y = ofs_y;
        est_nav_ofs.z = ofs_z;
    } else {
        fprintf(stderr,"%s: FILT_REINIT_BOX msg NOT created\n", __func__);
    }
}

commsT::~commsT() {
	// release resources
    // allocated w/ strdup
    // though clean() exists, maybe
    // preferable not to call methods
    // in destructor
	if(mapname) {
		free(mapname);
	}
	if(cfgname) {
        free(cfgname);
	}
	if(particlename) {
		free(particlename);
	}
	if(logname) {
		free(logname);
	}
}

int commsT::serialize(char* buf, int buf_length) {
	//printf("Serializing commsT\n");
	int len = 0;
	unsigned int ml;
	char* p_ml;
	memcpy(buf + len, &msg_type,  sizeof(msg_type));
	len += sizeof(msg_type);
	memcpy(buf + len, &parameter, sizeof(parameter));
	len += sizeof(parameter);
	p_ml = buf + len;
	len += sizeof(unsigned int); // reserve spot for msg length

	// Estimated position message?
	//
	if(msg_type == TRN_MOTN || msg_type == TRN_MLE || msg_type == TRN_MMSE) {
		len += pt.serialize(buf + len, buf_length - len);
	}
	// Measure update message?
	//
	else if(msg_type == TRN_MEAS) {
		len += mt.serialize(buf + len, buf_length - len);
	}
	// Vehicle drift rate?
	//
	else if(msg_type == TRN_SET_VDR) {
		memcpy(buf + len, &vdr, sizeof(vdr));
		len += sizeof(vdr);
	}
	// Initialization message?
	//
	else if(msg_type == TRN_INIT) {
        if(NULL==mapname){
            strcpy(buf + len, "");
            len += 1;
        }else{
            strcpy(buf + len, mapname);
            len += strlen(mapname) + 1;
        }
        if(NULL==cfgname){
            strcpy(buf + len, "");
            len += 1;
        }else{
            strcpy(buf + len, cfgname);
            len += strlen(cfgname) + 1;
        }
        if(NULL==particlename){
            strcpy(buf + len, "");
            len += 1;
        }else{
            strcpy(buf + len, particlename);
            len += strlen(particlename) + 1;
        }
        if(NULL==logname){
            strcpy(buf + len, "");
            len += 1;
        }else{
            strcpy(buf + len, logname);
            len += strlen(logname) + 1;
        }
	}
    // Set xyz_sdev_init message?
    //
    else if(msg_type == TRN_SET_INITSTDDEVXYZ || msg_type == TRN_GET_INITSTDDEVXYZ ) {
        memcpy(buf + len, &xyz_sdev, sizeof(xyz_sdev));
        len += sizeof(xyz_sdev);
    }
    else if(msg_type == TRN_SET_ESTNAVOFS ||
            msg_type == TRN_GET_ESTNAVOFS ||
            msg_type == TRN_FILT_REINIT_OFFSET ) {
        memcpy(buf + len, &est_nav_ofs, sizeof(est_nav_ofs));
        len += sizeof(est_nav_ofs);
    }
    else if(msg_type == TRN_FILT_REINIT_BOX) {
        memcpy(buf + len, &xyz_sdev, sizeof(xyz_sdev));
        len += sizeof(xyz_sdev);
        memcpy(buf + len, &est_nav_ofs, sizeof(est_nav_ofs));
        len += sizeof(est_nav_ofs);
    }

	ml = len - sizeof(msg_type) + sizeof(parameter) - sizeof(unsigned int);
	memcpy(p_ml, &ml, sizeof(ml));

	return len;
}

int commsT::unserialize(char* buf, int buf_length) {
	//printf("Unserializing commsT\n");
	int len = 0;
	unsigned int ml;

	memcpy(&msg_type,  buf + len, sizeof(msg_type));
	len += sizeof(msg_type);
	//printf("msg_type:%c\n", msg_type);
	memcpy(&parameter, buf + len, sizeof(parameter));
	len += sizeof(parameter);
	//printf("parameter:%d\n", parameter);
	memcpy(&ml, buf + len, sizeof(ml));
	len += sizeof(ml);
	//printf("remaining:%d\n", ml);

	// Estimated position message?
	//
	if((msg_type == TRN_MOTN || msg_type == TRN_MLE || msg_type == TRN_MMSE) && ml > 0) {
		//printf("Tell poseT to unserialize itself at buf[%d]\n", len);
		len += pt.unserialize(buf + len, buf_length - len);
	}
	// Measure update message?
	//
	else if((msg_type == TRN_MEAS) && ml > 0) {
		//printf("Tell measT to unserialize itself at buf[%d]\n", len);
		len += mt.unserialize(buf + len, buf_length - len);
	}
	// Vehicle drift rate?
	//
	else if(msg_type == TRN_SET_VDR) {
		memcpy(&vdr, buf + len, sizeof(vdr));
		len += sizeof(vdr);
	}
	// Initialization message?
	//
	else if(msg_type == TRN_INIT) {
		mapname = STRDUPNULL(buf + len);
		len += strlen(mapname) + 1;
		cfgname = STRDUPNULL(buf + len);
		len += strlen(cfgname) + 1;
		particlename = STRDUPNULL(buf + len);
		len += strlen(particlename) + 1;
		logname = STRDUPNULL(buf + len);
		len += strlen(logname) + 1;
      LOGM("commsT::serialize - setting log name [%s]\n",logname);

	}
    // Set xyz_sdev_init message?
    //
    else if(msg_type == TRN_SET_INITSTDDEVXYZ || msg_type == TRN_GET_INITSTDDEVXYZ) {
        memcpy(&xyz_sdev, buf + len, sizeof(xyz_sdev));
        len += sizeof(xyz_sdev);
    }
    else if(msg_type == TRN_SET_ESTNAVOFS ||
            msg_type == TRN_GET_ESTNAVOFS ||
            msg_type == TRN_FILT_REINIT_OFFSET) {
        memcpy(&est_nav_ofs, buf + len, sizeof(est_nav_ofs));
        len += sizeof(est_nav_ofs);
    }
    else if(msg_type == TRN_FILT_REINIT_BOX) {
        memcpy(&xyz_sdev, buf + len, sizeof(xyz_sdev));
        len += sizeof(xyz_sdev);
        memcpy(&est_nav_ofs, buf + len, sizeof(est_nav_ofs));
        len += sizeof(est_nav_ofs);
    }

	return len;
}

// Write a string representation of the object
//
char* commsT::to_s(char* buf, int buflen) {
	if(buf) {
		if(buflen > 250) {
			if(msg_type != TRN_INIT) {
				mapname = NULL;
				cfgname = NULL;
			}
			snprintf(buf, buflen, "commsT {type:%c|parameter:%d|vdr:%f|map:%s|cfg:%s|poseT time:%.2f|measT time:%.2f|numMeas:%d|xyz:%lf,%lf,%lf|ofs:%lf,%lf,%lf}",
					msg_type, parameter, vdr, mapname, cfgname, pt.time, mt.time, mt.numMeas,xyz_sdev.x,xyz_sdev.y,xyz_sdev.z,est_nav_ofs.x,est_nav_ofs.y,est_nav_ofs.z);
//			int len = snprintf(buf, buflen, "commsT {type:%c|parameter:%d|vdr:%f|map:%s|cfg:%s|poseT time:%.2f|measT time:%.2f|numMeas:%d}",
//				msg_type, parameter, vdr, mapname, cfgname, pt.time, mt.time, mt.numMeas);
//			printf("%d\n", len);
//			if (msg_type == TRN_MEAS) printf("alphas[0] = %f\n", mt.alphas[0]);
		}
	}

	return buf;
}

// Clear state
//
void commsT::clean() {
	msg_type = '*';
	parameter = 0;
	mt.clean();
    // release resources
    // allocated w/ strdup
    if(mapname) {
        free(mapname);
    }
    if(cfgname) {
        free(cfgname);
    }
    if(particlename) {
        free(particlename);
    }
    mapname=NULL;
    cfgname=NULL;
    particlename=NULL;
}

// Release resources
// (generally called once per connection cycle)
void commsT::release() {
    mt.clean();
    // release resources
    // allocated w/ strdup
    if(mapname) {
        free(mapname);
    }
    if(cfgname) {
        free(cfgname);
    }
    if(particlename) {
        free(particlename);
    }
    if(logname) {
        free(logname);
    }
    mapname=NULL;
    cfgname=NULL;
    particlename=NULL;
    logname=NULL;
}

typedef struct tname_entry_s{
    char type;
    const char *name;
}tname_entry_t;

static tname_entry_t type_names[]={
    {TRN_INIT,"TRN_INIT"},
    {TRN_MEAS,"TRN_MEAS"},
    {TRN_MOTN,"TRN_MOTN"},
    {TRN_MLE,"TRN_MLE"},
    {TRN_MMSE,"TRN_MMSE"},
    {TRN_SET_MW,"TRN_SET_MW"},
    {TRN_SET_FR,"TRN_SET_FR"},
    {TRN_SET_IMA,"TRN_SET_IMA"},
    {TRN_SET_VDR,"TRN_SET_VDR"},
    {TRN_SET_MIM,"TRN_SET_MIM"},
    {TRN_FILT_GRD,"TRN_FILT_GRD"},
    {TRN_ACK,"TRN_ACK"},
    {TRN_BYE,"TRN_BYE"},
    {TRN_OUT_MEAS,"TRN_OUT_MEAS"},
    {TRN_LAST_MEAS,"TRN_LAST_MEAS"},
    {TRN_IS_CONV,"TRN_IS_CONV"},
    {TRN_FILT_TYPE,"TRN_FILT_TYPE"},
    {TRN_FILT_STATE,"TRN_FILT_STATE"},
    {TRN_N_REINITS,"TRN_N_REINITS"},
    {TRN_FILT_REINIT,"TRN_FILT_REINIT"},
    {TRN_FILT_REINIT_OFFSET,"TRN_FILT_REINIT_OFFSET"},
    {TRN_FILT_REINIT_BOX,"TRN_FILT_REINIT_BOX"},
    {TRN_SET_INITSTDDEVXYZ,"TRN_SET_INITSTDDEVXYZ"},
    {TRN_GET_INITSTDDEVXYZ,"TRN_GET_INITSTDDEVXYZ"},
    {TRN_SET_ESTNAVOFS,"TRN_SET_ESTNAVOFS"},
    {TRN_GET_ESTNAVOFS,"TRN_GET_ESTNAVOFS"},
    {TRN_INIT,"TRN_INIT"},
    {'\0',"?"}
};

const char *commsT::typestr(char type)
{
    tname_entry_t *tp = &type_names[0];
    while(tp->type!='\0'){
        if(tp->type == type)
            break;
        tp++;
    }

    return tp->name;
}
