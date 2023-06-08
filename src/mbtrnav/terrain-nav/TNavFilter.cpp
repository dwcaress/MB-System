/* FILENAME      : TNavFilter.cpp
 * AUTHOR        : Debbie Meduna
 * DATE          : 05/02/09
 *
 * LAST MODIFIED : 11/30/10
 * MODIFIED BY   : Debbie Meduna
 * -----------------------------------------------------------------------------
 * Modification History
 * -----------------------------------------------------------------------------
 ******************************************************************************/

#include "TNavFilter.h"

#include "TerrainMapOctree.h"
#include "TerrainMapDEM.h"
#include "particleFilterDefs.h"
#include "trn_log.h"

TNavFilter::
//TNavFilter(char* mapName, char* vehicleSpecs, char* directory, const double* windowVar, const int& mapType) {Reload Map Issue
TNavFilter(TerrainMap* terrainMap, char* vehicleSpecs, char* directory, const double* windowVar, const int& mapType)
:SubcloudNIS(0.0),
forceHighGradeFilter(false),
forceLowGradeFilter(false)
{
	int i;
	this->mapType = mapType;
	/* Reload Map Issue
	if(this->mapType == 1) {
		terrainMap = new TerrainMapDEM(mapName);
	} else {
		terrainMap = new TerrainMapOctree(mapName);
	}
	*/
	this->terrainMap = terrainMap;
	vehicle = new vehicleT(vehicleSpecs);
	if(USE_COMPASS_BIAS) {
		compassBias = new compassBiasT;
	} else {
		compassBias = NULL;
	}
	saveDirectory = directory;
	this->useModifiedWeighting = TRN_WT_NONE;  //default to not using.
	// for flat terrain use setModifiedWeighting to change
	//this->octreeMap = NULL;

	//initialize currVar and initWindowVar
	for(i = 0; i < N_COVAR; i++) {
		initWindowVar[i] = windowVar[i];
	}
	currVar[0] = windowVar[0];
	currVar[1] = windowVar[2];

	//Initialize random number generator
    unsigned int seed = seed_randn(NULL);
    logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),
          "Random noise generator initialized with %d", seed);

	initVariables();
}



TNavFilter::
~TNavFilter() {
	//Reload Map Issue:  Map now resides in TerrainNav
	/*
	if(terrainMap != NULL) {
		delete terrainMap;
	}
	terrainMap = NULL;
	*/

	if(lastNavPose != NULL) {
		delete lastNavPose;
	}
	lastNavPose = NULL;

	if(vehicle != NULL) {
		delete vehicle;
	}
	vehicle = NULL;

	if(compassBias != NULL) {
		delete compassBias;
	}
	compassBias = NULL;

#ifdef USE_MATLAB
	engClose(matlabEng);
#endif

}

bool
TNavFilter::
withinRefMap() {
	//This tests if the most recently integrated INS estimate is in the map
	double north, east;
	north = this->lastNavPose->x;
	east = this->lastNavPose->y;

	//TODO: may want to augment this with depth when using Octrees
	//reply: not for Octrees based on DEM data.  For Octrees of 3D objects, 'within' the map no longer means much
	//Octrees now do the exact same test as DEMs: X and Y dimensions, but this may not mean much as Octrees are often expanded/squared out
	return terrainMap->withinRefMap(north, east);
}

bool
TNavFilter::
withinValidMapRegion(const double& north, const double& east) {
	double dx = 0.8 * MAX_RANGE; //0.1*1.5*totalAttemptTime+MAX_RANGE;
	if(terrainMap->withinValidMapRegion(north, east)
			&& terrainMap->withinValidMapRegion(north + sqrt(fabs(initWindowVar[0])) + dx, east)
			&& terrainMap->withinValidMapRegion(north - sqrt(fabs(initWindowVar[0])) - dx, east)
			&& terrainMap->withinValidMapRegion(north, east + sqrt(fabs(initWindowVar[2])) + dx)
			&& terrainMap->withinValidMapRegion(north, east - sqrt(fabs(initWindowVar[2])) - dx)
			&& terrainMap->withinValidMapRegion(north + sqrt(fabs(initWindowVar[0])) + dx, east + sqrt(fabs(initWindowVar[2])) + dx)
			&& terrainMap->withinValidMapRegion(north - sqrt(fabs(initWindowVar[0])) - dx, east + sqrt(fabs(initWindowVar[2])) + dx)
			&& terrainMap->withinValidMapRegion(north + sqrt(fabs(initWindowVar[0])) + dx, east - sqrt(fabs(initWindowVar[2])) - dx)
			&& terrainMap->withinValidMapRegion(north - sqrt(fabs(initWindowVar[0])) - dx, east - sqrt(fabs(initWindowVar[2])) - dx)) {
		return true;
	}

	return false;
}

bool
TNavFilter::
withinValidMapRegionPoint(const double& north, const double& east) {
	if(terrainMap->withinValidMapRegion(north, east)) {
		return true;//TODO why does this exist
	}

	return false;
}

bool
TNavFilter::
isConverged() {
	//TODO: check if the filter is converged
	return converged;
}

bool
TNavFilter::
findMeasSensorIndex(int measType, int& sensorIndex) {
	sensorIndex = 0;

	//associate measurement with correct sensor type
	while((sensorIndex < vehicle->numSensors) &&
          (measType != vehicle->sensors[sensorIndex].type)) {

	   //logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"findMeasSensorIndex:: checking measType =%d, against "
	   //	"sensorType = %d.\n",measType,
	   //	vehicle->sensors[sensorIndex].type);

	    sensorIndex++;
	}

	//if there was no corresponding sensor for this vehicle,
	//return false
	if(sensorIndex >= vehicle->numSensors) {
	  logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"findMeasSensorIndex:: Error: "
		 "measType = %d , sensorIndex = %d, "
		 "NumSensors = %d\n", measType,
		 sensorIndex,
		 vehicle->numSensors);
		return false;
	}

	return true;
}


Matrix
TNavFilter::
applyRotation(const double* attitude,  const Matrix& beamsVF) {
	Matrix beamsMF = beamsVF;
	double cphi = cos(attitude[0]);
	double sphi = sin(attitude[0]);
	double ctheta = cos(attitude[1]);
	double stheta = sin(attitude[1]);
	double cpsi = cos(attitude[2]);
	double spsi = sin(attitude[2]);
	double stheta_sphi = stheta * sphi;
	double stheta_cphi = stheta * cphi;
	int i;

	double R11 = cpsi * ctheta;
	double R12 = spsi * ctheta;
	double R13 = -stheta;
	double R21 = -spsi * cphi + cpsi * stheta_sphi;
	double R22 = cpsi * cphi + spsi * stheta_sphi;
	double R23 = ctheta * sphi;
	double R31 = spsi * sphi + cpsi * stheta_cphi;
	double R32 = -cpsi * sphi + spsi * stheta_cphi;
	double R33 = ctheta * cphi;

	for(i = 1; i <= beamsVF.Ncols(); i++) {
		beamsMF(1, i) = R11 * beamsVF(1, i) + R21 * beamsVF(2, i) + R31 * beamsVF(3, i);

		beamsMF(2, i) = R12 * beamsVF(1, i) + R22 * beamsVF(2, i) + R32 * beamsVF(3, i);

		beamsMF(3, i) = R13 * beamsVF(1, i) + R23 * beamsVF(2, i) + R33 * beamsVF(3, i);
	}

	return beamsMF;
}

bool
TNavFilter::
getTerrainMap(mapT& currMap) {
	return terrainMap->GetMapT(currMap);
}

bool
TNavFilter::
getTerrainMapBounds(double* currMapBounds) {
	return terrainMap->GetMapBounds(currMapBounds);
}



void
TNavFilter::
increaseInitSearchWin(double* windowVarIncrement) {
	int i;
	for(i = 0; i < N_COVAR; i++) {
		this->initWindowVar[i] += windowVarIncrement[i];
	}

	return;
}


void
TNavFilter::
initVariables() {
	vehicle->displayVehicleInfo();
	lastNavPose = NULL;
	interpMeasAttitude = false;
	converged = false;
	timeLastDvlValid = 0;
	windowedNIS = 0;
	measVariance = -1;
	totalAttemptTime = 0;
	int i;
	for(i = 0; i < 3; i++) {
		currentVel[i] = 0.0;
	}
	for(i = 0; i < NIS_WINDOW_LENGTH; i++) {
		windowedNISlog[i] = 0.0;
	}

	//initialize distribution type to Uniform
	initDistribType = 0;

        //initialize the type of distrib file to save
        setDistribToSave(SAVE_PARTICLES);

#ifdef USE_MATLAB
	//start Matlab engine
	if(!(matlabEng = engOpen("\0"))) {
		fprintf(stderr, "\nCan't start MATLAB engine\n");
		exit(0);   // Only when running within a matlab test or sim
	}
#endif

	//determine dvl sensor attitude
	int sensorIndex = 0;

	//associate measurement with correct sensor type
	while((sensorIndex < vehicle->numSensors) &&
          (1 != vehicle->sensors[sensorIndex].type)) {
		sensorIndex++;
	}

	dvlAttitude = vehicle->T_sv[sensorIndex].rotation;
	dvlRotMatrix = getRotMatrix(dvlAttitude);

}

//bool TNavFilter::projectMeasVF(Matrix &beamsVF, measT& currMeas)
bool
TNavFilter::
projectMeasVF(Matrix& beamsVF, const measT& currMeas, int* beamIndices) {
	Matrix trans_sv(3, 1);
	Matrix Rsv;
	Matrix beamsSF(3, currMeas.numMeas);
	int measSensor = 0;

	//check that a valid sensor index can be found for current measurement
	if(!findMeasSensorIndex(currMeas.dataType, measSensor)) {
		logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"TNavFilter:: Invalid measurement type %d. Unable to add"
		       " measurement\n", currMeas.dataType);
		return false;
	}

	//project beams into sensor frame
	if(projectMeasSF(beamsSF, currMeas, beamIndices)) {
		//project beams into vehicle frame
		//--------------------------------------------------------------------------
		Rsv = getRotMatrix(vehicle->T_sv[measSensor].rotation);
		trans_sv << vehicle->T_sv[measSensor].translation;

		beamsVF.ReSize(3, beamsSF.Ncols());
        int i=0;
		for(i = 1; i <= beamsSF.Ncols(); i++) {
			beamsVF.SubMatrix(1, 3, i, i) = Rsv.t() * beamsSF.SubMatrix(1, 3, i, i) + trans_sv;
		}

		return true;
	} else {
		return false;
	}
}

#define NO_DATATYPE -101

//bool TNavFilter::projectMeasSF(Matrix &beamsSF, measT& currMeas)
bool
TNavFilter::
projectMeasSF(Matrix& beamsSF, const measT& currMeas, int* beamIndices) {
   int measSensor = 0;
   int numGoodBeams = 0;
   int i, n_unknown;
   double theta, psi;
   Matrix dr_bs(3, 1);

   //check that a valid sensor index can be found for current measurement
   if(!findMeasSensorIndex(currMeas.dataType, measSensor)) {
      logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"TNavFilter:: Invalid measurement type. Unable to add"
	   " measurement\n");
      return false;
   }

   //translational vector between beam and sensor frames
   dr_bs << vehicle->sensors[measSensor].T_bs[0].translation;

   //project beams into sensor frame
   //---------------------------------------------------------------------------
   for(n_unknown = NO_DATATYPE, i = 1; i <= currMeas.numMeas; i++)
   {
      //only include beams that are reasonable/good
      if(!currMeas.measStatus[i - 1]) {
         continue;
      }
      logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),
      	            "TNavFilter:: ping # %ld currMeas.measStatus[%d] = %d",
	             			currMeas.ping_number, i-1, currMeas.measStatus[i-1]);

      switch(currMeas.dataType) { //1: DVL, 2: Multibeam, 3: Single Beam, 5: Imagenex delta t multibeam
	 case TRN_SENSOR_DVL:
	    theta = vehicle->sensors[measSensor].T_bs[i - 1].rotation[1];
	    psi = vehicle->sensors[measSensor].T_bs[i - 1].rotation[2];

	    //project beams into sensor frame
	    //Note that this is projecting a beam that is default in +z direction
	    beamsSF(1, numGoodBeams + 1) = sin(theta) * cos(psi) * currMeas.ranges[i - 1];
	    beamsSF(2, numGoodBeams + 1) = sin(theta) * sin(psi) * currMeas.ranges[i - 1];
	    beamsSF(3, numGoodBeams + 1) = cos(theta) * currMeas.ranges[i - 1];

//         //overwrite measT data to ensure first numGoodBeams entries are valid
//         if(i != numGoodBeams+1)
//            currMeas.ranges[numGoodBeams] = currMeas.ranges[i-1];
	    break;

	 case TRN_SENSOR_MB:
	    //ignore beams with missing/NaN data values
	    /*if(isnan(currMeas.crossTrack[i-1]))
	      continue;*/

	    beamsSF(1, numGoodBeams + 1) = currMeas.alongTrack[i - 1];
	    beamsSF(2, numGoodBeams + 1) = currMeas.crossTrack[i - 1];
	    beamsSF(3, numGoodBeams + 1) = currMeas.altitudes[i - 1];

//         //overwrite measT data to ensure first numGoodBeams entries are valid
//         if(i != numGoodBeams+1)
//         {
//            currMeas.alongTrack[numGoodBeams] = currMeas.alongTrack[i-1];
//            currMeas.crossTrack[numGoodBeams] = currMeas.crossTrack[i-1];
//            currMeas.altitudes[numGoodBeams] = currMeas.altitudes[i-1];
//         }
	    break;

	 case TRN_SENSOR_PENCIL:
	    beamsSF(1, numGoodBeams + 1) = cos(currMeas.theta) * currMeas.ranges[0];
	    beamsSF(2, numGoodBeams + 1) = 0;
	    beamsSF(3, numGoodBeams + 1) = sin(currMeas.theta) * currMeas.ranges[0];
	    break;

	 case TRN_SENSOR_DELTAT:
	    //TODO: copied from DVL case, need to verify that this is correct - maybe
	    //needs phi as well or don't need psi
	    //phi = vehicle->sensors[measSensor].T_bs[i - 1].rotation[0];
	    theta = vehicle->sensors[measSensor].T_bs[i - 1].rotation[1];
	    psi = vehicle->sensors[measSensor].T_bs[i - 1].rotation[2];

	    //project beams into sensor frame
	    //Note that this is projecting a beam that is default in +z direction
	    beamsSF(1, numGoodBeams + 1) = sin(theta) * cos(psi) * currMeas.ranges[i - 1];
	    beamsSF(2, numGoodBeams + 1) = sin(theta) * sin(psi) * currMeas.ranges[i - 1];
	    beamsSF(3, numGoodBeams + 1) = cos(theta) * currMeas.ranges[i - 1];
	    break;

	 default:
	    logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"TNavFilter::Invalid measurement type specified. Exiting...\n");
	    n_unknown = currMeas.dataType;
	    continue;    // Do not add to beamIndices, go to next beam

      }

      //translation component of transformation into sensor frame
      beamsSF.SubMatrix(1, 3, numGoodBeams + 1, numGoodBeams + 1) += dr_bs;

//      //overwrite measT data to ensure first numGoodBeams entries are all valid
//      if(i != numGoodBeams+1)
//      {
//         currMeas.covariance[numGoodBeams] = currMeas.covariance[i-1];
//         currMeas.measStatus[numGoodBeams] = currMeas.measStatus[i-1];
//      }

      beamIndices[numGoodBeams] = i - 1;

      numGoodBeams++;
   }

   // Report invalid datatypes in beam vector
   //
   if (n_unknown != NO_DATATYPE)
      logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"TNavFilter::One or more invalid datatypes specified (e.g., %d)\n",
	   currMeas.dataType);

   //If no beams were valid, return false
   if(numGoodBeams == 0) {
      logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"TNavFilter::Measurement from time = %.2f sec, ping # %u not included; there"
	   " are no good beams from the sonar.\n", currMeas.time, currMeas.ping_number);
      return false;
   }

   //If some beams were rejected, display message to the screen
   if(numGoodBeams != currMeas.numMeas)
      logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"TNavFilter::Excluded %i beam(s) from correlation due to poor"
	   " sonar data.\n", currMeas.numMeas - numGoodBeams);
   logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"TNavFilter::%i good beams.\n", numGoodBeams);

   //Take only subset of beamsSF that has valid beams.
   beamsSF = beamsSF.Columns(1, numGoodBeams);

   return true;
}

Matrix
TNavFilter::
getRotMatrix(const double* attitude) {
	Matrix R(3, 3);
	double cphi = cos(attitude[0]);
	double sphi = sin(attitude[0]);
	double ctheta = cos(attitude[1]);
	double stheta = sin(attitude[1]);
	double cpsi = cos(attitude[2]);
	double spsi = sin(attitude[2]);
	double stheta_sphi = stheta * sphi;
	double stheta_cphi = stheta * cphi;

	R(1, 1) = cpsi * ctheta;
	R(1, 2) = spsi * ctheta;
	R(1, 3) = -stheta;
	R(2, 1) = -spsi * cphi + cpsi * stheta_sphi;
	R(2, 2) = cpsi * cphi + spsi * stheta_sphi;
	R(2, 3) = ctheta * sphi;
	R(3, 1) = spsi * sphi + cpsi * stheta_cphi;
	R(3, 2) = -cpsi * sphi + spsi * stheta_cphi;
	R(3, 3) = ctheta * cphi;

	return R;
}


Matrix
TNavFilter::
applyDVLrotation(const Matrix& beamsSF) {
	Matrix beamsVF = beamsSF;
	int i;

	for(i = 1; i <= beamsSF.Ncols(); i++) {
		beamsVF(1, i) = dvlRotMatrix(1, 1) * beamsSF(1, i) + dvlRotMatrix(2, 1) * beamsSF(2, i) + dvlRotMatrix(3, 1) * beamsSF(3, i);

		beamsVF(2, i) = dvlRotMatrix(1, 2) * beamsSF(1, i) + dvlRotMatrix(2, 2) * beamsSF(2, i) + dvlRotMatrix(3, 2) * beamsSF(3, i);

		beamsVF(3, i) = dvlRotMatrix(1, 3) * beamsSF(1, i) + dvlRotMatrix(2, 3) * beamsSF(2, i) + dvlRotMatrix(3, 3) * beamsSF(3, i);
	}

	return beamsVF;
}

/* modifyBeamDir //TODO delete? commented by David with no adverse effects
void
TNavFilter::
modifyBeamDir(Matrix& beamsMF, const double* vehPose) {
	int i;
	Matrix H(1, 2);
	double mapIntersect[3];
	double n[3];
	double phi, alpha, d;
	//double p, phi, alpha, d;
	double norm, normU;
	double testVal, zVar;
	double minorAxis[3];
	double majorAxis[3];
	double u[3];

	for(i = 1; i <= beamsMF.Ncols(); i++) {
		//find initial mapIntersect in N,E,D frame
		mapIntersect[0] = beamsMF(1, i) + vehPose[0];
		mapIntersect[1] = beamsMF(2, i) + vehPose[1];
		mapIntersect[2] = 0.0;

		//Assess terrain normal
		interpolateDepthAndGradient(mapIntersect[0], mapIntersect[1],// TODO this isn't in here anymore, it moved to terrainMapDEM
									mapIntersect[2], zVar, H);
		if(isnan(H(1, 1)) || isnan(H(1, 2))) {
			continue;
		}

		n[0] = H(1, 1);
		n[1] = H(1, 2);
		n[2] = -1.0;

		//normalize terrain normal
		norm = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		n[0] = n[0] / norm;
		n[1] = n[1] / norm;
		n[2] = n[2] / norm;

		//Define original norm
		normU = sqrt(beamsMF(1, i) * beamsMF(1, i) + beamsMF(2, i) * beamsMF(2, i)
					 + beamsMF(3, i) * beamsMF(3, i));

		//Determine if normal vector is within the beam cone (if yes,
		//this is the closest-direction.
		testVal = -n[0] * beamsMF(1, i) - n[1] * beamsMF(2, i) - n[2] * beamsMF(3, i) -
				  normU * cos(vehicle->sensors[0].beamWidth / 2.0);

		if(testVal >= 0) {
	*/		//TODO: what is p? why isn't it used anywhere?
			/*   p = -((mapIntersect[0]-vehPose[0])*n[0] +
			         (mapIntersect[1]-vehPose[1])*n[1] +
			         (mapIntersect[2]-vehPose[2])*n[2]);
			*/
	/*		//reweight beamsMF to have same length as before
			beamsMF(1, i) = -n[0] * normU;
			beamsMF(2, i) = -n[1] * normU;
			beamsMF(3, i) = -n[2] * normU;
			continue;
		}

		//If closest point is not within the beam cone, we will use
		//the closest point within the intersection ellipse

		//minorAxis = nxu,  majorAxis = minorUxn
		u[0] = beamsMF(1, i) / normU;
		u[1] = beamsMF(2, i) / normU;
		u[2] = beamsMF(3, i) / normU;

		//compute minor axis
		computeArrayCrossProd(n, u, minorAxis);
		norm = sqrt(minorAxis[0] * minorAxis[0] + minorAxis[1] * minorAxis[1] +
					minorAxis[2] * minorAxis[2]);
		minorAxis[0] = minorAxis[0] / norm;
		minorAxis[1] = minorAxis[1] / norm;
		minorAxis[2] = minorAxis[2] / norm;

		//compute major axis
		computeArrayCrossProd(minorAxis, n, majorAxis);
		norm = sqrt(majorAxis[0] * majorAxis[0] + majorAxis[1] * majorAxis[1] +
					majorAxis[2] * majorAxis[2]);
		majorAxis[0] = majorAxis[0] / norm;
		majorAxis[1] = majorAxis[1] / norm;
		majorAxis[2] = majorAxis[2] / norm;

		//determine angle between u and major axis
		phi = acos(-(u[0] * majorAxis[0] + u[1] * majorAxis[1] + u[2] * majorAxis[2]));
		if(phi > PI / 2) {
			majorAxis[0] = -majorAxis[0];
			majorAxis[1] = -majorAxis[1];
			majorAxis[2] = -majorAxis[2];
			phi = PI - phi;
		}

		//determine length of major axis
		alpha = PI - phi - vehicle->sensors[0].beamWidth / 2.0;
		d = sin(vehicle->sensors[0].beamWidth / 2.0) * normU / sin(alpha);

		//redefine beamsMF
		beamsMF(1, i) += d * majorAxis[0];
		beamsMF(2, i) += d * majorAxis[1];
		beamsMF(3, i) += d * majorAxis[2];

		//reweight beamsMF to have same length as before
		norm = sqrt(beamsMF(1, i) * beamsMF(1, i) + beamsMF(2, i) * beamsMF(2, i)
					+ beamsMF(3, i) * beamsMF(3, i));
		beamsMF(1, i) = beamsMF(1, i) * normU / norm;
		beamsMF(2, i) = beamsMF(2, i) * normU / norm;
		beamsMF(3, i) = beamsMF(3, i) * normU / norm;
	}
}
*/
/* modifyBeamDirExtensive //TODO delete? commented by David with no adverse effects
void
TNavFilter::
modifyBeamDirExtensive(Matrix& beamsMF, const double* vehPose) {
	int i, j, k;
	int indexX, indexY;
	double norm, normU;
	double testVal, r, var;
	double u[3];
	double u_new[3];
	double nearestPt[3];
	double mapIntersect[3];
	double testPt[3];
	double minDist = 0;

	mapT mapForBeamDirExtensive;
	terrainMap->GetMapT(mapForBeamDirExtensive);

	for(i = 1; i <= beamsMF.Ncols(); i++) {
		//Define original norm
		normU = sqrt(beamsMF(1, i) * beamsMF(1, i) + beamsMF(2, i) * beamsMF(2, i)
					 + beamsMF(3, i) * beamsMF(3, i));
		//Define normalized direction vector
		u[0] = beamsMF(1, i) / normU;
		u[1] = beamsMF(2, i) / normU;
		u[2] = beamsMF(3, i) / normU;

		//find initial mapIntersect in N,E,D frame
		computeMapRayIntersection(vehPose, u, r, var);
		mapIntersect[0] = u[0] * r + vehPose[0];
		mapIntersect[1] = u[1] * r + vehPose[1];
		mapIntersect[2] = u[2] * r + vehPose[2];
		indexX = closestPtUniformArray(mapIntersect[0], mapForBeamDirExtensive.xpts[0],
									   mapForBeamDirExtensive.xpts[mapForBeamDirExtensive.numX - 1], mapForBeamDirExtensive.numX);
		indexY = closestPtUniformArray(mapIntersect[1], mapForBeamDirExtensive.ypts[0],
									   mapForBeamDirExtensive.ypts[mapForBeamDirExtensive.numY - 1], mapForBeamDirExtensive.numY);
		//logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"indexX: %i, indexY: %i, range: %f\n", indexX, indexY, normU);

		//Find closest point in the terrain within beam cone
		nearestPt[0] = mapForBeamDirExtensive.xpts[0] - vehPose[0];
		nearestPt[1] = mapForBeamDirExtensive.ypts[0] - vehPose[1];
		nearestPt[2] = mapForBeamDirExtensive.depths(1, 1) - vehPose[2];
		minDist = sqrt(nearestPt[0] * nearestPt[0] + nearestPt[1] * nearestPt[1]
					   + nearestPt[2] * nearestPt[2]);
		for(j = max(indexX - int(normU), 0); j < indexX + int(normU) &&
				j < mapForBeamDirExtensive.numX; j++) {
			for(k = max(indexY - int(normU), 0); k < indexY + int(normU) &&
					k < mapForBeamDirExtensive.numY; k++) {
				testPt[0] = mapForBeamDirExtensive.xpts[j] - vehPose[0];
				testPt[1] = mapForBeamDirExtensive.ypts[k] - vehPose[1];
				testPt[2] = mapForBeamDirExtensive.depths(j + 1, k + 1) - vehPose[2];
				norm = sqrt(testPt[0] * testPt[0] + testPt[1] * testPt[1]
							+ testPt[2] * testPt[2]);

				//check if test point is within the cone
				testVal = testPt[0] * u[0] + testPt[1] * u[1] + testPt[2] * u[2] -
						  norm * cos(vehicle->sensors[0].beamWidth / 2.0);
				if(testVal >= 0) {
					if(norm < minDist) {
						minDist = norm;
						nearestPt[0] = testPt[0];
						nearestPt[1] = testPt[1];
						nearestPt[2] = testPt[2];
					}
				}
			}
		}

		u_new[0] = nearestPt[0] / minDist;
		u_new[1] = nearestPt[1] / minDist;
		u_new[2] = nearestPt[2] / minDist;

		beamsMF(1, i) = u_new[0] * normU;
		beamsMF(2, i) = u_new[1] * normU;
		beamsMF(3, i) = u_new[2] * normU;
	}

}
*/

void
TNavFilter::
calculateNIS(SymmetricMatrix& measCov, ColumnVector& meanDiff, double& nisVal,
			 const measT& currMeas, int* beamIndices) {
	SymmetricMatrix covMatrix(measCov.Ncols());
	covMatrix = measCov;

//	int goodIndexCounter = 1;

	//Add the variance due to the range sensor
	for(int j = 0; j < measCov.Ncols(); j++) {
		covMatrix(j + 1, j + 1) += currMeas.covariance[beamIndices[j]];

//		if (currMeas.measStatus[j])  //If the measurement is good, add it's covariance in
//		{
//			if (goodIndexCounter <= covMatrix.Ncols())
//			{
// 		     logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"TNavFilter:: adding measurement index %i \n", goodIndexCounter);
//			covMatrix(goodIndexCounter,goodIndexCounter) += currMeas.covariance[j];
//			goodIndexCounter++;
//			}
//			else
//			{
// 		     logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"TNavFilter:: measurements do not line up!!! \n");
//			 break;
//			}
//		}
	}

	//Compute Err' * inv(Sigma) * Err
	Matrix nisMatrix;
	nisMatrix =  meanDiff.t() * covMatrix.i() * meanDiff;
	nisVal = nisMatrix(1, 1);
	//NIS value needs to be normalized by the number of beams that are used
	nisVal = nisVal/measCov.Ncols();
}

void
TNavFilter::
updateNISwindow(const double& nisVal) {
	int i;
	windowedNIS = 0;

    for(i = 0; i < NIS_WINDOW_LENGTH-1; i++) {
		windowedNISlog[i] = windowedNISlog[i + 1];
		windowedNIS += windowedNISlog[i] / NIS_WINDOW_LENGTH;
	}
	windowedNISlog[NIS_WINDOW_LENGTH - 1] = nisVal; //check this
	windowedNIS += nisVal / NIS_WINDOW_LENGTH;
}

unsigned int
TNavFilter::
setDistribToSave(unsigned int distrib)
{
  logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"setDistribToSave(%d)", distrib);
  // If type not one of the recognized options, use the default
  if (PARTICLESTOFILE == distrib || HISTOGRAMTOFILE == distrib)
    _distribType = distrib;
  else
    _distribType = SAVE_PARTICLES;

  logs(TL_OMASK(TL_TNAV_FILTER, TL_LOG),"setDistribToSave set to %d", _distribType);
  return _distribType;
}
