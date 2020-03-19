/* File: structDefs.cpp
 * -------------------
 * Written by: Debbie Meduna
 * 2014/??/??: Henthorn. Updated to support client/server architecture.
 * 2016/12/19: Henthorn. Updated to pass log dir name and particle file name to
 *             the server during initialization.
 *****************************************************************************/

#include "structDefs.h"

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

/*----------------------------------------------------------------------------
/mapT member functions
/----------------------------------------------------------------------------*/
mapT::mapT() {
	xpts = NULL;
	ypts = NULL;
	numX = 0;
	numY = 0;
}

mapT::~mapT() {
	clean();
}

void mapT::clean() {
	if(xpts != NULL) {
		delete [] xpts;
		xpts = NULL;
	}

  if(ypts != NULL) {
		delete [] ypts;
		ypts = NULL;
	}

	depths.CleanUp();
	depthVariance.CleanUp();
}

void mapT::reSampleMap(const double newRes) {
	int newNumX, newNumY;
	double* xptsNew;
	double* yptsNew;
	Matrix depthsNew;
	Matrix depthVarNew;
	int i, j, subRes;
	Matrix subDepthMap;

	//Fill in new xpts/ypts vectors
	newNumX = int(round(fabs(xpts[numX - 1] - xpts[0]) / newRes)) + 1;
	newNumY = int(round(fabs(ypts[numY - 1] - ypts[0]) / newRes)) + 1;
	depthsNew.ReSize(newNumX, newNumY);
	depthVariance.ReSize(newNumX, newNumY);

	xptsNew = new double[newNumX];
	yptsNew = new double[newNumY];

	for(i = 0; i < newNumX; i++) {
		xptsNew[i] = xpts[0] + newRes * i;
	}

	for(j = 0; j < newNumY; j++) {
		yptsNew[j] = ypts[0] + newRes * j;
	}

	//Fill in new depth values
	if((newRes > dx) | (newRes > dy)) {
		//TO DO: Fix this so it actually computes correct average for newRes!!
		subRes = int(ceil(newRes / dx));
		subDepthMap.ReSize(subRes, subRes);
		for(i = 1; i <= newNumX; i++) {
			for(j = 1; j <= newNumY; j++) {
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
poseT& poseT::operator=(poseT& rhs) {
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
poseT& poseT::operator-=(poseT& rhs) {

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
poseT& poseT::operator+=(poseT& rhs) {

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

measT::~measT() {
	// although clean() exists
    // maybe preferable not to
    // call methods in destructor
    delete [] covariance;
    delete [] ranges;
    delete [] crossTrack;
    delete [] alongTrack;
    delete [] altitudes;
    delete [] alphas;
    delete [] measStatus;
    delete [] beamNums;
}

// release all dynamic memory resources of the struct
void measT::clean() {
	if(covariance != NULL) {
		delete [] covariance;
	}
	covariance = NULL;

	if(ranges != NULL) {
		delete [] ranges;
	}
	ranges = NULL;

	if(crossTrack != NULL) {
		delete [] crossTrack;
	}
	crossTrack = NULL;

	if(alongTrack != NULL) {
		delete [] alongTrack;
	}
	alongTrack = NULL;

	if(altitudes != NULL) {
		delete [] altitudes;
	}
	altitudes = NULL;

	if(alphas != NULL) {
		delete [] alphas;
	}
	alphas = NULL;

	if(measStatus != NULL) {
		delete [] measStatus;
	}
	measStatus = NULL;

	if(beamNums != NULL) {
		delete [] beamNums;
	}
	beamNums = NULL;

	time = 0.;

	ping_number = 0;
	numMeas = 0;
}

//copy assignment operator
measT& measT::operator=(measT& rhs) {
	int i;
	if(this != &rhs) {
		//if the two measT structs have different datatype or number of
		//measurements, we need to delete and recreate memory for the
		//new measT struct.
		if(numMeas != rhs.numMeas || dataType != rhs.dataType) {
			this->clean();
			if(rhs.dataType == TRN_SENSOR_MB ||
			   rhs.dataType == TRN_SENSOR_HOMER) {
				crossTrack = new double[rhs.numMeas];
				alongTrack = new double[rhs.numMeas];
				altitudes = new double[rhs.numMeas];
				ranges = new double[rhs.numMeas];
			} else {
				ranges = new double[rhs.numMeas];
			}

			// MB-sys beam numbers
			if(rhs.dataType == TRN_SENSOR_MB)
			  beamNums = new int[rhs.numMeas];

   		alphas = new double[rhs.numMeas];
			measStatus = new bool[rhs.numMeas];
		}

		if(rhs.covariance != NULL) {
			if (covariance) delete [] covariance;
			covariance = new double[rhs.numMeas];
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
		for(i = 0; i < rhs.numMeas; i++) {
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

		measStatus = new bool[nm];
		for(int i = 0; i < nm; i++) {
			measStatus[i] = (buf[len++] != 0 ? true : false);
#if BEAM_DEBUG
			printf("\t\tRecv measStatus[%d] == %d\n", i, measStatus[i]);
#endif
		}
		// Using ranges or tracks and altitudes?
		//
		if(abs(dataType) == TRN_SENSOR_MB ||
		   abs(dataType) == TRN_SENSOR_HOMER) {
			crossTrack = new double[nm];
			alongTrack = new double[nm];
			altitudes  = new double[nm];
			ranges     = new double[nm];

			// Again, order is significant
			//
			memcpy(crossTrack, &buf[len], nm * sizeof(double));
			len += nm * sizeof(double);
			memcpy(alongTrack, &buf[len], nm * sizeof(double));
			len += nm * sizeof(double);
			memcpy(altitudes,  &buf[len], nm * sizeof(double));
			len += nm * sizeof(double);
			memcpy(ranges,     &buf[len], nm * sizeof(double));
			len += nm * sizeof(double);

		} else {
			ranges = new double[nm];
			memcpy(ranges,     &buf[len], nm * sizeof(double));
			len += nm * sizeof(double);
		}

		// MB-sys beam numbers
		if(abs(dataType) == TRN_SENSOR_MB) {
			beamNums = new int[nm];
			memcpy(beamNums,  &buf[len], nm * sizeof(int));
			len += nm * sizeof(int);
		}


		// A dataType of less than zero is a signal that there
		// are no covariances in this measT
		//
		if(dataType >= 0) {
			covariance = new double[nm];
			memcpy(covariance, &buf[len], nm * sizeof(double));
			len += nm * sizeof(double);
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
		//
   	alphas = new double[nm];
		memcpy(alphas, &buf[len], nm * sizeof(double));
		len += nm * sizeof(double);

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
sensorT::sensorT() {
	T_bs = NULL;
}

sensorT::sensorT(char* fileName) { //TODO filename unused?
	T_bs = NULL;
}

sensorT::~sensorT() {
	if(T_bs != NULL) {
		delete [] T_bs;
	}
	T_bs = NULL;
}

// throws an exception if the specs file could not be opened
//
void sensorT::parseSensorSpecs(char* fileName) {
	fstream sensorFile;
	char temp[512];
	int i;

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

			for(i = 0; i < numBeams; i++) {
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
			for(i = 0; i < numBeams; i++)
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
			for(i = 0; i < numBeams; i++) {
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
			for(i = 0; i < numBeams; i++) {
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
      sprintf(temp, "Error opening sensor file %s...\n", fileName);
      fprintf(stderr, "%s", temp);
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
vehicleT::vehicleT() {
	T_sv = NULL;
	sensors = NULL;
}

vehicleT::vehicleT(char* fileName) {
	T_sv = NULL;
	sensors = NULL;
	parseVehicleSpecs(fileName);
}

vehicleT::~vehicleT() {
	if(T_sv != NULL) {
		delete [] T_sv;
	}
	T_sv = NULL;

	if(sensors != NULL) {
		delete [] sensors;
	}
	sensors = NULL;
}

// throws an exception if the specs file could not be opened
//
void vehicleT::parseVehicleSpecs(char* fileName) {
	fstream vehicleFile;
	char temp[512];
	char temp2[512];
	char sensorFile[1024];
	char* sensorPath;

   try
   {
      vehicleFile.open(fileName);
      if(vehicleFile.is_open()) {
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
           sensorPath = strstr(sensorFile, name);

           //determine sensor file name
           sprintf(temp2, "%s%s", sensors[i].name, "_specs.cfg\0");
           strcpy(sensorPath, temp2);

           //parse sensor file
           sensors[i].parseSensorSpecs(sensorFile);
           LOGM("Sensor %d is of type %d.\n",
                 i, sensors[i].type);
        }

        vehicleFile.close();
     } else {
        sprintf(temp,"Error opening file %s...\n", fileName);
        fprintf(stderr, "%s", temp);
        throw Exception(temp);
     }

   }  // try block

   catch (Exception e)
   {
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

    sprintf(bp,"Vehicle name: %s\n", name);
    bp=obuf+strlen(obuf);
    sprintf(bp,"Number of sensors: %i\n\n", numSensors);
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
	: msg_type(0), parameter(0), vdr(0.0),
	  mapname(NULL), cfgname(NULL), particlename(NULL), logname(NULL) {
}

commsT::commsT(char type)
	: msg_type(type), parameter(0), vdr(0.0),
	  mapname(NULL), cfgname(NULL), particlename(NULL), logname(NULL) {
}

commsT::commsT(char type, int param)
	: msg_type(type), parameter(param), vdr(0.0),
	  mapname(NULL), cfgname(NULL), particlename(NULL), logname(NULL) {
}

commsT::commsT(char type, int param, float dr)// TODO char param unused
	: msg_type(type), parameter(0), vdr(dr),
	  mapname(NULL), cfgname(NULL), particlename(NULL), logname(NULL) {
}

commsT::commsT(char type, int param, char* map, char* cfg,
	            char* partfile, char* logdir)
	: msg_type(type), parameter(param), vdr(0.0),
	  mapname(NULL), cfgname(NULL), particlename(NULL), logname(NULL) {
	mapname = strdup(map);
	cfgname = strdup(cfg);
	logname = strdup(logdir);
	particlename = strdup(partfile);
}

commsT::commsT(char type, int param, measT& m)
	: msg_type(type), parameter(param), vdr(0.0),
	  mapname(NULL), cfgname(NULL), particlename(NULL), logname(NULL) {
	// Measure update message?
	//
	if(msg_type == TRN_MEAS) {
		mt = m;
	} else {
		fprintf(stderr,"MU msg NOT created\n");
	}
	//printf("MU msg created\n");
}

commsT::commsT(char type, poseT& p)
	: msg_type(type), parameter(0), vdr(0.0),
	  mapname(NULL), cfgname(NULL), particlename(NULL), logname(NULL) {
	// Measure update message?
	//
	if((msg_type == TRN_MOTN || msg_type == TRN_MLE || msg_type == TRN_MMSE ||
			msg_type == TRN_ACK)) {
		pt = p;
	} else {
		fprintf(stderr,"EP/ACK msg NOT created\n");
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
		strcpy(buf + len, mapname);
		len += strlen(mapname) + 1;
		strcpy(buf + len, cfgname);
		len += strlen(cfgname) + 1;
		strcpy(buf + len, particlename);
		len += strlen(particlename) + 1;
		strcpy(buf + len, logname);
		len += strlen(logname) + 1;
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
		mapname = strdup(buf + len);
		len += strlen(mapname) + 1;
		cfgname = strdup(buf + len);
		len += strlen(cfgname) + 1;
		particlename = strdup(buf + len);
		len += strlen(particlename) + 1;
		logname = strdup(buf + len);
		len += strlen(logname) + 1;
        LOGM("commsT::serialize - setting log name [%s]\n",logname);

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
			sprintf(buf, "commsT {type:%c|parameter:%d|vdr:%f|map:%s|cfg:%s|poseT time:%.2f|measT time:%.2f|numMeas:%d}",
					msg_type, parameter, vdr, mapname, cfgname, pt.time, mt.time, mt.numMeas);
//			int len = sprintf(buf, "commsT {type:%c|parameter:%d|vdr:%f|map:%s|cfg:%s|poseT time:%.2f|measT time:%.2f|numMeas:%d}",
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
    mapname=NULL;
    cfgname=NULL;
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
