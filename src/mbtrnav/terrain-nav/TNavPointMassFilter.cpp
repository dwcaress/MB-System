/* FILENAME      : TNavPointMassFilter.cpp
 * AUTHOR        : Debbie Meduna
 * DATE          : 04/24/09
 *
 * LAST MODIFIED : 11/29/10
 * MODIFIED BY   : Debbie Meduna
 * -----------------------------------------------------------------------------
 * Modification History
 * -----------------------------------------------------------------------------
 *****************************************************************************/

#include "TNavPointMassFilter.h"
#include "MathP.h"
#include "TerrainMapDEM.h"
#include "mapio.h"
#include "trn_log.h"

//TNavPointMassFilter::TNavPointMassFilter(char* mapName, char* vehicleSpecs, char* directory, const double* windowVar,
//		const int& mapType) : TNavFilter(mapName, vehicleSpecs, directory, windowVar, mapType) {
TNavPointMassFilter::TNavPointMassFilter(TerrainMap* terrainMap, char* vehicleSpecs, char* directory, const double* windowVar,
		const int& mapType) : TNavFilter(terrainMap, vehicleSpecs, directory, windowVar, mapType) {
	priorPDF = new mapT;
	likeSurf = new mapT;
    for(int i=0;i<4;i++){
        hypBounds[i]=0;
    }
	initVariables();
}


TNavPointMassFilter::~TNavPointMassFilter() {
	if(saveDirectory != NULL) {
		//close data files
		gradientFile.close();
		measFile.close();
		numMeasFile.close();
		likeSurfFile.close();
		postSurfFile.close();
		depthBiasFile.close();
	}
	
	if(corrData != NULL) {
		delete [] corrData;
	}
	corrData = NULL;
	
	if(priorPDF != NULL) {
		delete priorPDF;
	}
	priorPDF = NULL;
	
	if(likeSurf != NULL) {
		delete likeSurf;
	}
	likeSurf = NULL;
}

void TNavPointMassFilter::initFilter(poseT& initNavPose) {
	//initialize pdf information
	this->initPriorPDF(initNavPose);
}


bool TNavPointMassFilter::measUpdate(measT& currMeas) {
	bool successfulMeas = true;
	bool containsNaN = false;
	Matrix postPDF(this->priorPDF->numX, this->priorPDF->numY);
	Matrix measPDF(this->priorPDF->numX, this->priorPDF->numY);
	
	// extract most current navigation position estimate
	double loc[3] = {lastNavPose->x, lastNavPose->y, lastNavPose->z};
	
	// create correlation data vector.  If correlation vector non-empty,
	// continue measurement incorporation
	if(generateMeasCorrData(currMeas)) {
		// define search space and extract submap for correlation
		int checkMap = defineHypBoundsAndMap(loc);
		
		// check that the map extraction worked correctly
		if(checkMap == 1) {
			logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"TerrainNav::Measurement from time = %.2f sec. not included; "
				   "unable to successfully extract a map segment for correlation"
				   , currMeas.time);
			successfulMeas = false;
		} else {
			//update priorPDF
			measPDF = generateCorrelationSurf(containsNaN);
			if(!USE_MAP_NAN && containsNaN) {
				logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"TerrainNav::Measurement from time = %.2f sec. not included; "
					   "encountered NaN values in the correlation map segment\n",
					   currMeas.time);
				return false;
			}
			*(this->likeSurf) = *(this->priorPDF);
			this->likeSurf->depths = measPDF;
			postPDF = SP(measPDF, this->priorPDF->depths);
			
			//If debugging, plot distribution surfaces
#ifdef USE_MATLAB
			mapT mapForPlotting;
			terrainMap->GetMapT(mapForPlotting);
			if(currMeas.time > 0) {
				//plot PDF surfaces in Matlab
				plotMatlabSurf(this->priorPDF->depths,
							   "title('Prior Distribution');", "figure(1)");
				plotMatlabSurf(measPDF, "title('Likelihood Meas. Distribution');",
							   "figure(2);");
				plotMatlabSurf(postPDF, "title('Posterior Distribution');",
							   "figure(3);");
				plotMatlabSurf(mapForPlotting.depths,
							   "title('Underlying Map');", "figure(4);");
			}
#endif
			
			//If postPDF is reasonable, incorporate it into priorPDF
			if(postPDF.Sum() == 0)
				logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"TerrainNav::Measurement from time = %.2f sec. not included;"
					   " the correlation score was too low.\n", currMeas.time);
			else {
				//renormalize priorPDF
				this->priorPDF->depths = postPDF;
				this->priorPDF->depths *= 1.0 / (this->priorPDF->depths.Sum());
				
				//if estimating depth, update parameters
				if(USE_CONTOUR_MATCHING) {
					this->depthBias = this->tempDepthBias;
					totalSumInvVar.SubMatrix(hypBounds[0], hypBounds[1],
											 hypBounds[2], hypBounds[3])
					+= currSumInvVar.SubMatrix(hypBounds[0], hypBounds[1]
											   , hypBounds[2],
											   hypBounds[3]);
					//this->depthBiasFile << setprecision(15) << this->depthBias
					//                << endl;
					if(DEPTH_FILTER_LENGTH > 0) {
						//update totalSumInvVar by removing oldest sumInvVar
						totalSumInvVar.SubMatrix(hypBounds[0], hypBounds[1],
												 hypBounds[2], hypBounds[3]) -=
													 measSumInvVar[this->currMeasPointer].
													 SubMatrix(hypBounds[0], hypBounds[1], hypBounds[2],
															   hypBounds[3]);
															   
						//update entry in measSumInvVar array
						measSumInvVar[this->currMeasPointer].
						SubMatrix(hypBounds[0], hypBounds[1], hypBounds[2],
								  hypBounds[3]) = currSumInvVar.
												  SubMatrix(hypBounds[0], hypBounds[1], hypBounds[2],
															hypBounds[3]);
															
						//update entry in measSumError array
						measSumError[this->currMeasPointer].
						SubMatrix(hypBounds[0], hypBounds[1], hypBounds[2],
								  hypBounds[3]) = currSumError.
												  SubMatrix(hypBounds[0], hypBounds[1], hypBounds[2],
															hypBounds[3]);
															
						//increment currMeasPointer for depth filter arrays
						this->currMeasPointer++;
						
						//ensure that the pointer cycles through the array after
						//reaching the end
						if(this->currMeasPointer >= DEPTH_FILTER_LENGTH) {
							this->currMeasPointer = 0;
						}
						
					}
				}
				
				if(saveDirectory != NULL) {
					this->likeSurfFile << setprecision(15) << measPDF << endl;
					this->postSurfFile << setprecision(15) << this->priorPDF->depths << endl;
				}
			}
		}
	} else {
		successfulMeas = false;
	}
	
	//remove memory associated with correlation data
	delete [] corrData;
	this->numCorr = 0;
	this->corrData = NULL;
	
	//if measurement successfully added, recheck estimator convergence
	//if(successfulMeas)
	//   checkConvergence();
	
	return successfulMeas;
	
}

void TNavPointMassFilter::motionUpdate(poseT& currNavPose) {
	int j;
	double dx, dy, elapsedTime;
	Matrix velocity_sf(3, 1), R_sv;
	Matrix velocity_vf(3, 1), R_vi;
	Matrix velocity_if(3, 1), A;
	DiagonalMatrix Cv(3);
	SymmetricMatrix Cx(3);
	Cx = 0.0;
	elapsedTime = currNavPose.time - this->lastNavPose->time;
	
	//If there is valid GPS data AND/OR there is no valid DVL data for dead
	//reckoning, use the stored INS pose information to perform the motion
	//update.  Otherwise, use the stored velocity data (from DVL) and vehicle
	//attitude information to perform the motion update.
	if(currNavPose.gpsValid || !DEAD_RECKON || !lastNavPose->dvlValid) {
		//use recorded vehicle navigation to estimate vehicle motion
		dx = currNavPose.x - this->lastNavPose->x;
		dy = currNavPose.y - this->lastNavPose->y;
		//logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"using recorded navigation values...\n");
		
		//Compute motion sigma
		double cep = (this->vehicle->driftRate / 100.0) * (sqrt(dx * dx + dy * dy));
		double sigmaSq = cep / sqrt(-2 * (log(1 - 0.5)));
		Cx(1, 1) = sigmaSq;
		Cx(2, 2) = sigmaSq;
		Cx = Cx.SymSubMatrix(1, 2);
	} else {
		//use recorded DVL velocities to dead reckon
        double velocity_sf_sigma[3];
		velocity_sf(1, 1) = this->lastNavPose->vx;
		velocity_sf(2, 1) = this->lastNavPose->vy;
		velocity_sf(3, 1) = this->lastNavPose->vz;
		
        double lastAttitude[3] = {lastNavPose->phi, lastNavPose->theta,
            lastNavPose->psi
        };
		velocity_vf = applyRotation(dvlAttitude, velocity_sf);
		velocity_if = applyRotation(lastAttitude, velocity_vf);
		R_sv = getRotMatrix(dvlAttitude);
		R_vi = getRotMatrix(lastAttitude);
		
		//Compute vehicle displacement based on inertial velocity
		dx = velocity_if(1, 1) * elapsedTime;
		dy = velocity_if(2, 1) * elapsedTime;
		
		//Compute motion sigma
		A = R_sv.t() * R_vi.t();
		if(this->lastNavPose->bottomLock) {
			velocity_sf_sigma[0] = fabs(VEL_PER_ERROR * velocity_sf(1, 1) / 100.0);
			velocity_sf_sigma[1] = fabs(VEL_PER_ERROR * velocity_sf(2, 1) / 100.0);
			velocity_sf_sigma[2] = fabs(VEL_PER_ERROR * velocity_sf(3, 1) / 100.0);
		} else {
			velocity_sf_sigma[0] = fabs(WATER_VEL_PER_ERROR * velocity_sf(1, 1) / 100.0);
			velocity_sf_sigma[1] = fabs(WATER_VEL_PER_ERROR * velocity_sf(2, 1) / 100.0);
			velocity_sf_sigma[2] = fabs(WATER_VEL_PER_ERROR * velocity_sf(3, 1) / 100.0);
		}
		//check if velocity data is old and add noise according to a
		//0.01m/s^2 acceleration
		if(this->timeLastDvlValid != lastNavPose->time) {
			velocity_sf_sigma[0] += 0.01 * fabs(lastNavPose->time - this->timeLastDvlValid);
			velocity_sf_sigma[1] += 0.01 * fabs(lastNavPose->time - this->timeLastDvlValid);
			velocity_sf_sigma[2] += 0.01 * fabs(lastNavPose->time - this->timeLastDvlValid);
		}
		Cv(1) = pow(velocity_sf_sigma[0] * elapsedTime, 2);
		Cv(2) = pow(velocity_sf_sigma[1] * elapsedTime, 2);
		Cv(3) = pow(velocity_sf_sigma[2] * elapsedTime, 2);
		
		Cx << A* Cv* A.t();
		Cx = Cx.SymSubMatrix(1, 2);
	}
	
	//shift center of PDF according to vehicle motion
	for(j = 0; j < this->priorPDF->numX; j++) {
		this->priorPDF->xpts[j] += dx;
	}
	
	for(j = 0; j < this->priorPDF->numY; j++) {
		this->priorPDF->ypts[j] += dy;
	}
	
	// convolve PDF up to current time
	if(USE_MOTION_BLUR) {
		this->motionBlur(elapsedTime, sqrt(dx * dx + dy * dy), Cx);
	}
	
	logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"done with TNavPointMassFilter::motionUpdate()...\n");
}


void TNavPointMassFilter::computeMLE(poseT* mlePose) {
	//double maxValue;
	int maxRow, maxCol;
	
	this->priorPDF->depths.Maximum2(maxRow, maxCol);
	//maxValue = this->priorPDF->depths.Maximum2(maxRow, maxCol);
	//logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"TerrainNav::Likelihood surface maximum peak: %.2f\n",maxValue);
	
	mlePose->z = this->lastNavPose->z;
	mlePose->x = this->priorPDF->xpts[maxRow - 1];
	mlePose->y = this->priorPDF->ypts[maxCol - 1];
	
	if(USE_CONTOUR_MATCHING) {
		mlePose->z -= this->depthBias(maxRow, maxCol);
	}
	
	mlePose->time = this->lastNavPose->time;
	
	return;
	
}

void TNavPointMassFilter::computeMMSE(poseT* mmsePose) {
	//initialize variables
	double xbar=0, ybar=0;
	int row=0, col=0;
	
	//Compute x and y means
	for(int i = 1; i <= this->priorPDF->numX; i++) {
		for(int j = 1; j <= this->priorPDF->numY; j++) {
			xbar += this->priorPDF->xpts[i - 1] * this->priorPDF->depths(i, j);
			ybar += this->priorPDF->ypts[j - 1] * this->priorPDF->depths(i, j);
		}
	}
	mmsePose->z = this->lastNavPose->z;
	
	//Estimate depth bias from most likely location in depthBias matrix
	if(USE_CONTOUR_MATCHING) {
		row = nearest(xbar, this->priorPDF->xpts, this->priorPDF->numX);
		col = nearest(ybar, this->priorPDF->ypts, this->priorPDF->numY);
		mmsePose->z -= this->depthBias(row + 1, col + 1);
	}
	
	//Compute covariance parameters
	mmsePose->covariance[0] = 0;
	mmsePose->covariance[1] = 0;
	mmsePose->covariance[2] = 0;
	mmsePose->covariance[5] = 0;
	for(int i = 1; i <= this->priorPDF->numX; i++) {
		for(int j = 1; j <= this->priorPDF->numY; j++) {
			mmsePose->covariance[0] += pow((this->priorPDF->xpts[i - 1] - xbar), 2) *
									   this->priorPDF->depths(i, j);
			mmsePose->covariance[2] += pow((this->priorPDF->ypts[j - 1] - ybar), 2) *
									   this->priorPDF->depths(i, j);
			mmsePose->covariance[1] += (this->priorPDF->ypts[j - 1] - ybar) *
									   (this->priorPDF->xpts[i - 1] - xbar) * this->priorPDF->depths(i, j);
									   
			if(USE_CONTOUR_MATCHING)
				mmsePose->covariance[5] += pow(this->depthBias(i, j) -
											   this->depthBias(row + 1, col + 1), 2) *
										   this->priorPDF->depths(i, j);
		}
	}
	
	mmsePose->x = xbar;
	mmsePose->y = ybar;
	mmsePose->time = this->lastNavPose->time;
	
	return;
}


void TNavPointMassFilter::checkConvergence() {
	poseT mmse;
	poseT mle;
	double diff;
	
	//compute current mmse and mle pose estimates
	computeMMSE(&mmse);
	computeMLE(&mle);
	
	//check similarity between mean and mle estimates
	diff = sqrt(pow(mmse.x - mle.x, 2) + pow(mmse.y - mle.y, 2));
	if(diff > 10) {
		converged = false;
		return;
	}
	
	//if mean and mle are close, check gaussian-likeness with KL
	double mu[2] = {mmse.x, mmse.y};
	Matrix Cov(2, 2);
	double kl;
	
	Cov(1, 1) = mmse.covariance[0];
	Cov(2, 2) = mmse.covariance[1];
	Cov(1, 2) = mmse.covariance[2];
	Cov(2, 1) = mmse.covariance[3];
	
	kl = computeKLdiv_gaussian_mat(this->priorPDF->xpts, this->priorPDF->ypts,
								   this->priorPDF->depths, mu, Cov);
								   
	logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"Current KL: %.10f\n", kl);
	if(kl < 0.1) {
		converged = true;
	} else {
		converged = false;
	}
	
	
	return;
}

void TNavPointMassFilter::saveCurrDistrib(ofstream& outputFile) {
	//check that the desired file exists and is open for writing
	if(!outputFile.is_open()) {
		logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"Error:Tried to write to an unopened file."
			   " Ignoring write command.");
		return;
	}
	
	outputFile << setprecision(15) << this->priorPDF->depths << endl;
	outputFile << setprecision(15) << this->depthBias << endl;
}


bool TNavPointMassFilter::getCurrPDF(mapT& currPDF) {
	if(priorPDF != NULL) {
		currPDF = *(priorPDF);
	} else {
		return false;
	}
	
	return true;
}


bool TNavPointMassFilter::getLikeSurf(mapT& currLikeSurf) {
	if(likeSurf != NULL) {
		currLikeSurf = *(likeSurf);
	} else {
		return false;
	}
	
	return true;
}

void TNavPointMassFilter::initVariables() {
	//initialize corrData
	numCorr = 0;
	corrData = NULL;
	
	//initialize depth bias variables
	currMeasPointer = 0;
	
	if(saveDirectory != NULL) {
		//load data files for storing results
        char fileName[2048]={0};
		gradientFile.open(charCat(fileName, saveDirectory, "J.txt"));
		measFile.open(charCat(fileName, saveDirectory, "measProj.txt"));
		numMeasFile.open(charCat(fileName, saveDirectory, "numMeas.txt"));
		likeSurfFile.open(charCat(fileName, saveDirectory, "likeSurfs.txt"));
		postSurfFile.open(charCat(fileName, saveDirectory, "postSurfs.txt"));
		depthBiasFile.open(charCat(fileName, saveDirectory, "depthBias.txt"));
	}
	
	//if using convolution type motion blurring, initialize dx_old
	if(USE_MOTION_BLUR && (MOTION_BLUR_METHOD == 1)) {
		dx_old = 0.0;
		Cov_Old.ReSize(2);
		Cov_Old = 0.0;
	}
}

void TNavPointMassFilter::initPriorPDF(const poseT& initNavPose) {
	//initialize variables
	ColumnVector mu(2), currX(2);
	SymmetricMatrix Cov(2);
	Matrix invCov(2, 2);
	Matrix error;
	//double covDet;
	Cov = 0.0;

	if(HYP_RES != 0) {
		this->priorPDF->dx = HYP_RES;
		this->priorPDF->dy = HYP_RES;
	} else {
		this->priorPDF->dx = fabs(terrainMap->Getdx());
		this->priorPDF->dy = fabs(terrainMap->Getdy());
	}
	
	//initialize prior pdf to uniform or gaussian depending on initDistribType
	if(this->initDistribType == 0) {
		//uniform distribution
		this->priorPDF->numX = 2 * int(round(sqrt(fabs(initWindowVar[0])) / fabs(this->priorPDF->dx))) + 1;
		this->priorPDF->numY = 2 * int(round(sqrt(fabs(initWindowVar[2])) / fabs(this->priorPDF->dy))) + 1;
		
		this->priorPDF->depths.ReSize(this->priorPDF->numX, this->priorPDF->numY);
		this->priorPDF->depths = 1.0 / (this->priorPDF->numX * this->priorPDF->numY);
	} else {
		//gaussian distribution
		this->priorPDF->numX = 6 * int(round(sqrt(fabs(initWindowVar[0])) / fabs(this->priorPDF->dx))) + 1;
		this->priorPDF->numY = 6 * int(round(sqrt(fabs(initWindowVar[2])) / fabs(this->priorPDF->dy))) + 1;
		
		this->priorPDF->depths.ReSize(this->priorPDF->numX, this->priorPDF->numY);
		
		mu(1) = this->priorPDF->numX * this->priorPDF->dx / 2.0;
		mu(2) = this->priorPDF->numY * this->priorPDF->dy / 2.0;
		Cov(1, 1) = initWindowVar[0];
		Cov(2, 1) = initWindowVar[1];
		Cov(2, 2) = initWindowVar[2];
		
		Cov.Determinant();
		//covDet = Cov.Determinant();
		
		invCov = Cov.i();
		for(int i = 1; i <= this->priorPDF->numX; i++) {
			for(int j = 1; j <= this->priorPDF->numY; j++) {
				currX(1) = this->priorPDF->dx * (i - 1);
				currX(2) = this->priorPDF->dy * (j - 1);
				error = (currX - mu).t() * invCov * (currX - mu);
				this->priorPDF->depths(i, j) = exp(-(0.5) * error.AsScalar());
			}
		}
		this->priorPDF->depths *= (1.0 / this->priorPDF->depths.Sum());
	}
	
	//initialize xpts and ypts
	this->priorPDF->xpts = new double[this->priorPDF->numX];
	this->priorPDF->ypts = new double[this->priorPDF->numY];
	
	for(int i = 0; i < this->priorPDF->numX; i++)
		this->priorPDF->xpts[i] = initNavPose.x +
								  this->priorPDF->dx * (i - (this->priorPDF->numX - 1) / 2);
								  
	for(int i = 0; i < this->priorPDF->numY; i++)
		this->priorPDF->ypts[i] = initNavPose.y +
								  this->priorPDF->dy * (i - (this->priorPDF->numY - 1) / 2);
								  
	//Initialize inverse variance matrix
	Matrix tempPDF(this->priorPDF->numX, this->priorPDF->numY);
	tempPDF = 0.0;
	this->currSumInvVar = 0.0 * tempPDF;
	this->currSumError = 0.0 * tempPDF;
	
	//if computing depth bias, initialize values of depth bias matrices
	if(USE_CONTOUR_MATCHING) {
		this->totalSumInvVar = 0.0 * tempPDF;
		this->depthBias = 0.0 * tempPDF;
		this->tempDepthBias = 0.0 * tempPDF;
		
		if(DEPTH_FILTER_LENGTH != 0) {
			for(int i = 0; i < DEPTH_FILTER_LENGTH; i++) {
				measSumError[i] = 0.0 * tempPDF;
				measSumInvVar[i] = 0.0 * tempPDF;
			}
		}
	}
	*(this->likeSurf) = *(this->priorPDF);
}

bool TNavPointMassFilter::generateMeasCorrData(const measT& currMeas) {
	corrT* newCorr;
	Matrix beamsVF(3, currMeas.numMeas);
	Matrix beamsIF, Rvi;
	int beamIndices[currMeas.numMeas];
	double attitude[3] = {currMeas.phi, currMeas.theta, currMeas.psi};
	int i;
	
	//Define attitude angles either from measurement or from navigation estimate
	if(this->interpMeasAttitude) {
		attitude[0] = lastNavPose->phi;
		attitude[1] = lastNavPose->theta;
		attitude[2] = lastNavPose->psi;
	}
	
	//generate beam direction vectors in vehicle frame
	if(!projectMeasVF(beamsVF, currMeas, beamIndices)) {
		return false;
	}
	
	//ensure that corrData is empty before redefining
	this->numCorr = beamsVF.Ncols();
	if(this->corrData != NULL) {
		delete [] this->corrData;
		this->corrData = NULL;
	}
	
	//rotate measurement vectors from vehicle to inertial frame
	Rvi = getRotMatrix(attitude);
	beamsIF = Rvi.t() * beamsVF;
	
	//add measurement to correlation data vector
	newCorr = new corrT[this->numCorr];
	for(i = 1; i <= this->numCorr; i++) {
		newCorr[i - 1].dx = beamsIF(1, i);
		newCorr[i - 1].dy = beamsIF(2, i);
		newCorr[i - 1].dz = beamsIF(3, i);
		newCorr[i - 1].var = currMeas.covariance[beamIndices[i - 1]];
	}
	
	//TODO: Do we ever want to average the beams???
	//define corrData. If averaging beams, corrData will have size 1.
	if(!AVERAGE) {
		this->corrData = newCorr;
	} else {
		this->corrData = new corrT[1];
		
		for(i = 1; i < numCorr; i++) {
			newCorr[0].dx += newCorr[i].dx;
			newCorr[0].dy += newCorr[i].dy;
			newCorr[0].dz += newCorr[i].dz;
			newCorr[0].var += newCorr[i].var;
		}
		
		newCorr[0].dx = newCorr[0].dx / numCorr;
		newCorr[0].dy = newCorr[0].dy / numCorr;
		newCorr[0].dz = newCorr[0].dz / numCorr;
		newCorr[0].var = newCorr[0].var / numCorr;
		
		corrData[0] = newCorr[0];
		this->numCorr = 1;
		delete [] newCorr;
	}
	
	return true;
}

Matrix TNavPointMassFilter::generateCorrelationSurf(bool& containsNaN) {
	//Declare variables
	double depthMeas;
	Matrix Like(hypBounds[1] - hypBounds[0] + 1, hypBounds[3] - hypBounds[2] + 1);
	Matrix Esq(hypBounds[1] - hypBounds[0] + 1, hypBounds[3] - hypBounds[2] + 1);
	Matrix Error(hypBounds[1] - hypBounds[0] + 1, hypBounds[3] - hypBounds[2] + 1);
	Matrix SumError(hypBounds[1] - hypBounds[0] + 1, hypBounds[3] - hypBounds[2] + 1);
	Matrix numBeamsCorrelated(hypBounds[1] - hypBounds[0] + 1,
							  hypBounds[3] - hypBounds[2] + 1);
	Matrix currProdInvVar(hypBounds[1] - hypBounds[0] + 1,
						  hypBounds[3] - hypBounds[2] + 1);
						  
	//boolean matrix indicating if the associated likelihood score comes from
	//a gaussian distribution function
	Matrix GaussianProb(hypBounds[1] - hypBounds[0] + 1, hypBounds[3] - hypBounds[2] + 1);
	
	//normalization constants used to sum the likelihood scores
	double alpha = 0;
	double beta = 0;
	int numAlpha = 0;
	int numBeta = 0;

	if(saveDirectory != NULL) {
		//save corrData values to file for debugging
		for(int i = 0; i < numCorr; i++) {
			this->measFile << setprecision(15) << i << '\t';
			this->measFile << corrData[numCorr - i - 1].dx << '\t';
			this->measFile << corrData[numCorr - i - 1].dy << '\t';
			this->measFile << corrData[numCorr - i - 1].dz << endl;
		}
	}
	
	/* Since we are assuming that the sonar and map errors are independent,
	 * we can use that information to perform a faster correlation.  In this
	 * method, the computation time is O(m), where m is the number of beams
	 * correlated.
	 */
	
	//Define matrices MapValues and Zvar which will hold the
	//map comparison depths and their associated variances.
	Matrix MapValues(hypBounds[1] - hypBounds[0] + 1, hypBounds[3] - hypBounds[2] + 1);
	MapValues = 0.0;
	Matrix ZinvVar = MapValues;
	Matrix ZVar = MapValues;
	
	numBeamsCorrelated = numCorr;
	currSumInvVar = 0.0;
	currSumError = 0.0;
	currProdInvVar = 1.0;
	Esq = 0.0;
	Error = 1.0;
	SumError = 0.0;
	double totalNaN = 0;
	containsNaN = false;
	
	//Cycle through all beams to generate squared error matrix
	for(int m = 1; m <= numCorr; m++) {
		depthMeas = lastNavPose->z + corrData[numCorr - m].dz;
		extractDepthCompareValues(MapValues, ZVar, m);
		
#ifdef USE_MATLAB
		//plotMatlabSurf(ZVar,"title('map variance');", "figure(5)");
#endif
		
		//Add current sonar measurement noise to variance matrix
		ZVar += corrData[numCorr - m].var;
		
		//Invert variance matrix for proper weighting
		for(int i = 0; i < ZVar.Nrows(); i++) {
			for(int j = 0; j < ZVar.Ncols(); j++) {
				ZinvVar(i + 1, j + 1) = 1.0 / ZVar(i + 1, j + 1);
			}
		}
		
		//Check if beams intersect NaN values in the map, and remove those
		//beams from the correlation
		int i = 0;
		for(int row = hypBounds[0]; row <= hypBounds[1]; row++) {
			i++;
			int j = 0;
			for(int col = hypBounds[2]; col <= hypBounds[3]; col++) {
				j++;
				
				Error(i, j) = depthMeas - fabs(MapValues(i, j));
				// if(isnan(Error(i, j))) {
				if(ISNIN(Error(i, j))) {
					Error(i, j) = 0;
					numBeamsCorrelated(i, j)--;
					ZinvVar(i, j) = 1.0;
					currSumInvVar(row, col) -= 1.0;
					totalNaN++;
				}
			}
		}
		if(totalNaN > 0) {
			containsNaN = true;
		}
		
		//Weight error terms according to inverse variances
		//SP(A,B) indicates element-wise matrix product of A and B
		this->currSumInvVar.SubMatrix(hypBounds[0], hypBounds[1],
									  hypBounds[2], hypBounds[3]) += ZinvVar;
		currProdInvVar = SP(currProdInvVar, ZinvVar);
		this->currSumError.SubMatrix(hypBounds[0], hypBounds[1],
									 hypBounds[2], hypBounds[3]) += SP(ZinvVar, Error);
		Esq += SP(ZinvVar, SP(Error, Error));
	}
	
	logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"TerrainNav::Minimum Correlation Error: %.4f \n", Esq.Minimum());
	//Generate the Likelihood matrix from the squared error matrix
	int i = 0;
	GaussianProb = 0.0;
	for(int row = hypBounds[0]; row <= hypBounds[1]; row++) {
		i++;
		int j = 0;
		for(int col = hypBounds[2]; col <= hypBounds[3]; col++) {
			j++;
			if(numBeamsCorrelated(i, j) == 0) {
				//If any hypothesis points have no correlated beams, set the
				//probability to the uniform distribution
				Like(i, j) = 1.0 / (Like.Nrows() * Like.Ncols());
				beta += Like(i, j);
				numBeta++;
			} else {
				//normalization constant for gaussian distribution with
				//dimension N = numBeamsCorrelated
				double eta = pow(2 * PI, -0.5 * numBeamsCorrelated(i, j)) *
					  sqrt(currProdInvVar(i, j));
					  
				//compute likelihood estimate based on correlation error
				if(USE_CONTOUR_MATCHING)
					if(DEPTH_FILTER_LENGTH == 0)
						Like(i, j) = generateDepthCorrelation(currSumInvVar(row, col),
															  Esq(i, j),
															  currSumError(i, j)
															  , row, col);
					else
						Like(i, j) = generateDepthFilterCorrelation(currSumInvVar(row, col),
									 Esq(i, j),
									 currSumError(i, j)
									 , row, col);
									 
				else {
					Like(i, j) = eta * exp(-0.5 * Esq(i, j));
				}
				
				alpha += Like(i, j);
				GaussianProb(i, j) = 1;
				numAlpha++;
			}
		}
	}
	
	//Normalize the likelihood surface for a proper probability function
	i = 0;
	for(int row = hypBounds[0]; row <= hypBounds[1]; row++) {
		i++;
		int j = 0;
		for(int col = hypBounds[2]; col <= hypBounds[3]; col++) {
			j++;
			//Normalize distribution
			if(GaussianProb(i, j) && alpha != 0) {
				Like(i, j) = Like(i, j) * (1 - beta) / alpha;
			}
		}
	}
	//If likelihood surface is not the same size as the prior PDF, pad with zeros
	if(Like.Nrows() != this->priorPDF->numX || Like.Ncols() != this->priorPDF->numY) {
		containsNaN = true;
		zeroPad(Like);
	}
	
	if(saveDirectory != NULL) {
		//Compute terrain gradient at current location
		Matrix G(1, 2);
		G = 0.0;

        for( i = 1; i <= numCorr; i++) {
			double north = this->lastNavPose->x + corrData[numCorr - i].dx;
			double east = this->lastNavPose->y + corrData[numCorr - i].dy;
			
			if(this->mapType == 1){ //TODO make this use RayTrace or Query?
				(dynamic_cast<TerrainMapDEM *>(terrainMap))->interpolateGradient(north, east, G);
			}else{
				fprintf(stderr,"\n\nERR: interpolation methods on Octree map invalid\n");
			}
			
			this->gradientFile << setprecision(15) << G << endl;
		}
		
		this->numMeasFile << numCorr << endl;
	}
	
	return Like;
}


void TNavPointMassFilter::extractDepthCompareValues(Matrix& depthMat,
		Matrix& varMat,
		const int measNum) {
	double locX, locY;
	double* hypX = NULL;
	double* hypY = NULL;

	//If the hypothesis resolution matches the map resolution, we can do
	//a slightly faster depth extraction and interpolation method
	if(HYP_RES == 0 && this->terrainMap->GetInterpMethod() == 0) {
		locX = corrData[numCorr - measNum].dx;
		locY = corrData[numCorr - measNum].dy;
		
		
		mapT mapForComparison;
		terrainMap->GetMapT(mapForComparison);
		
		//define map bounds for particular relative beam location:
        int bounds[4]={0};
		bounds[0] = closestPtUniformArray(locX + priorPDF->xpts[hypBounds[0] - 1],
										  mapForComparison.xpts[0],
										  mapForComparison.xpts[mapForComparison.numX - 1],
										  mapForComparison.numX) + 1;
		bounds[1] = bounds[0] + hypBounds[1] - hypBounds[0];
		
		bounds[2] = closestPtUniformArray(locY + priorPDF->ypts[hypBounds[2] - 1],
										  mapForComparison.ypts[0],
										  mapForComparison.ypts[mapForComparison.numY - 1],
										  mapForComparison.numY) + 1;
		bounds[3] = bounds[2] + hypBounds[3] - hypBounds[2];
		
		
		//check that we are within the map boundaries
		while(bounds[3] > mapForComparison.depths.Ncols()) {
			bounds[3]--;
			bounds[2]--;
		}
		while(bounds[1] >  mapForComparison.depths.Nrows()) {
			bounds[1]--;
			bounds[0]--;
		}
		
		//extract correlation map and variance for current beam
		depthMat = mapForComparison.depths.SubMatrix(bounds[0], bounds[1],
				   bounds[2], bounds[3]);
				   
		varMat = mapForComparison.depthVariance.SubMatrix(bounds[0], bounds[1],
				 bounds[2], bounds[3]);
	} else {
	
		//Determine the interpolation locations associated with the hypothesis points
		hypX = new double[depthMat.Nrows()];
		hypY = new double[depthMat.Ncols()];
		
		int i = 0;
		for(int row = hypBounds[0]; row <= hypBounds[1]; row++) {
			//determine hypothesized projected beam X location
			locX = corrData[numCorr - measNum].dx + priorPDF->xpts[row - 1];
			hypX[i] = locX;
			i++;
			int j = 0;
			for(int col = hypBounds[2]; col <= hypBounds[3]; col++) {
				//determine hypothesized projected beam Y location
				locY = corrData[numCorr - measNum].dy + priorPDF->ypts[col - 1];
				hypY[j] = locY;
				j++;
			}
		}
		
		//perform map interpolation to hypothesis points
		if(this->mapType == 1){ //TODO make this use RayTrace or Query?
			(dynamic_cast<TerrainMapDEM *> (terrainMap))->interpolateDepthMat(hypX, hypY, depthMat, varMat);
		}else{
            fprintf(stderr,"\n\nERR: interpolation methods on Octree map invalid - exiting\n");
		}
			
		//remove memory allocated on the heap
		delete [] hypX;
		delete [] hypY;
	}
	
}

double TNavPointMassFilter::generateDepthCorrelation(double invVarSum,
		double sqCorrError,
		double corrError, int row,
		int col) {
	double oldDepthError, newDepthError, corrValue;
	double newTotalInvVar = this->totalSumInvVar(row, col) + invVarSum;
	
	oldDepthError = this->totalSumInvVar(row, col) *
					pow(this->depthBias(row, col), 2);
					
	//update depth bias values with new measurement.
	this->tempDepthBias(row, col) = (1.0 / newTotalInvVar) *
									(this->depthBias(row, col) * this->totalSumInvVar(row, col) + corrError);
	newDepthError = newTotalInvVar * pow(this->tempDepthBias(row, col), 2);
	
	//calculate likelihood equation.
	corrValue = exp(-0.5 * (sqCorrError - newDepthError + oldDepthError));
	
	return corrValue;
}

double TNavPointMassFilter::generateDepthFilterCorrelation(double invVarSum,
		double sqCorrError,
		double sumCorrError,
		int row, int col) {
	double corrValue;
	double sumInvVar = 0;
	double sumMeasError = 0;
	int i;
	
	//Compute depth bias by summing the filter arrays
	for(i = 0; i < DEPTH_FILTER_LENGTH; i++) {
		if(i == currMeasPointer) {
			sumMeasError += sumCorrError;
			sumInvVar += invVarSum;
		} else {
			sumMeasError += measSumError[i](row, col);
			sumInvVar += measSumInvVar[i](row, col);
		}
	}
	
	this->tempDepthBias(row, col) = (1.0 / sumInvVar) * sumMeasError;
	
	//calculate likelihood equation.
	corrValue = exp(-0.5 * (sqCorrError - 2 * this->tempDepthBias(row, col) * sumCorrError +
							pow(this->tempDepthBias(row, col), 2) * invVarSum));
							
	return corrValue;
}


int TNavPointMassFilter::defineHypBoundsAndMap(const double* loc) {
	/************************************************************************
	 Map boundary definitions (normal map):
	
	 * * * * * * * * * * * * * * * * * *  *--- max(|min_dy|, |max_dy|)
	 *  * * * * * * * * * * * * * * * **  *---
	 *  *	^		          							*  *
	 *  *	|initWindowVar[0]	          *  *
	 *  *	|		          							*  *
	 *  *---------    o(xCen,yCen)	  	*  *
	 *  *				  										*  *
	 *  *		   			initWindowVar[2]  *  *
	 *  *		   			|------------->		*  *
	 *  * * * * * * * * * * * * * * *  *  *
	 * * * * * * * * * * * * * * * * * *  *
					  |  |
					  <--> max(|min_dx|, |max_dx|)
	
	 Here min_dx, min_dy, max_dx and max_dy refer to the min and max deviations
	 in x and y of the projected measurement beams from the vehicle center. All
	 values in the above figure are in units of distance (meters).
	************************************************************************/
	double max_dx, max_dy;
	double xCen, yCen;
	double mapSearch[2];
	double numXdesired, numYdesired;
	int mapStatus;
	
	//Define maximum beam projection distances
	max_dx = 0;
	max_dy = 0;
	
	for(int i = 0; i < numCorr; i++) {
		double dx = fabs(corrData[i].dx);
		if(dx > max_dx) {
			max_dx = dx;
		}
		
        double dy = fabs(corrData[i].dy);
		if(dy > max_dy) {
			max_dy = dy;
		}
	}
	
	//Define desired search area in meters
	xCen = loc[0];
	yCen = loc[1];
	
	numXdesired = sqrt(fabs(initWindowVar[0])) + max_dx + 2 * fabs(terrainMap->Getdx());
	numYdesired = sqrt(fabs(initWindowVar[2])) + max_dy + 2 * fabs(terrainMap->Getdy());
	
	mapSearch[0] = 2.0 * numXdesired;
	mapSearch[1] = 2.0 * numYdesired;
	
	//ask to extract a map based on vehicle location and search bounds
	mapStatus = terrainMap->loadSubMap(xCen, yCen, mapSearch);
	
	//hypBounds: indicates the subIndices of priorPDF for which we have extracted
	//           map information. If we are able to extract a full correlation
	//           map, then these bounds should match the size of priorPDF.
	hypBounds[0] = 1;
	hypBounds[1] = this->priorPDF->numX;
	hypBounds[2] = 1;
	hypBounds[3] = this->priorPDF->numY;
	
	
	//If we can't extract a correlation map for our entire search area, redfine
	//hypBounds to specify the subIndices of priorPDF for which we have
	//extracted map information.
	if(mapStatus == MAPBOUNDS_NEAR_EDGE) {
		
		mapT mapForNearEdgeCase;
		terrainMap->GetMapT(mapForNearEdgeCase);
		
		//Define subIndices of priorPDF for which we have map correlation data
		hypBounds[0] = 2 + nearest(mapForNearEdgeCase.xpts[0] + max_dx,
								   this->priorPDF->xpts, this->priorPDF->numX);
		hypBounds[1] = nearest(mapForNearEdgeCase.xpts[mapForNearEdgeCase.numX - 1] -
							   max_dx, this->priorPDF->xpts,
							   this->priorPDF->numX);
		hypBounds[2] = 2 + nearest(mapForNearEdgeCase.ypts[0] + max_dy,
								   this->priorPDF->ypts, this->priorPDF->numY);
		hypBounds[3] = nearest(mapForNearEdgeCase.ypts[mapForNearEdgeCase.numY - 1] -
							   max_dy, this->priorPDF->ypts,
							   this->priorPDF->numY);
							   
		//check that the bounding box has non-zero size:
		if(hypBounds[1] - hypBounds[0] <= 0 || hypBounds[3] - hypBounds[2] <= 0) {
			logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"TerrainNav:: Could not extract enough map to perform the"
				   " desired correlation.\n");
			mapStatus = MAPBOUNDS_OUT_OF_BOUNDS;
		}
	}
	
	return mapStatus;
}


void TNavPointMassFilter::zeroPad(Matrix& Like) {
	Matrix tempLike = Like;
	double beta;
	Like.ReSize(this->priorPDF->numX, this->priorPDF->numY);
	
	logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"Truncated correlation area; padding the region outside correlation "
		   "bounds.\n");
		   
	//set the region of likelihood grid outside the correlation bounds to
	//uniform probability
	Like = 1.0 / (this->priorPDF->numX * this->priorPDF->numY);
	beta = (this->priorPDF->numX * this->priorPDF->numY - tempLike.Nrows() *
			tempLike.Ncols()) / (this->priorPDF->numY * this->priorPDF->numX);
	Like.SubMatrix(hypBounds[0], hypBounds[1], hypBounds[2], hypBounds[3]) =
		tempLike * (1 - beta);
		
	return;
}


void TNavPointMassFilter::motionBlur(const double dt, const double dx,
									 const SymmetricMatrix Cov) { //TODO dx is unused?
	Matrix oldPDF = this->priorPDF->depths;
	//bool convolved = true;
	switch(MOTION_BLUR_METHOD) {
		case 1:
			this->Cov_Old += Cov;
			motionBlur_convolve(this->Cov_Old);
			//convolved = motionBlur_convolve(this->Cov_Old);
			break;
			
		case 2:
			motionBlur_FPEexplicit(dt, Cov);
			break;
			
		default:
			return;
	}
	
	return;
}

bool TNavPointMassFilter::motionBlur_convolve(const SymmetricMatrix Cov) {
	int numPtsX, numPtsY, i, j;
	
	//determine size of blurring matrix to see if blurring is possible.
	numPtsX = int(3 * (sqrt(Cov(1, 1))) / fabs(this->priorPDF->dx));
	numPtsY = int(3 * (sqrt(Cov(2, 2))) / fabs(this->priorPDF->dy));
	
	//wait until uncertainty is on the order of the map spacing before
	//performing the convolution
	if(numPtsX < 2 || numPtsY < 2) {
		return false;
	} else {
		this->Cov_Old = 0.0;
	}
	
	ColumnVector mu(2), currX(2);
	mu(1) = numPtsX * this->priorPDF->dx / 2.0;
	mu(2) = numPtsY * this->priorPDF->dy / 2.0;
	Matrix convPDF(numPtsX + 1, numPtsY + 1);
	Matrix invCov = Cov.i();
	Matrix newPDF(this->priorPDF->numX, this->priorPDF->numY);
	Matrix error;
	
	//generate gaussian blurring matrix;
	for(i = 1; i <= numPtsX + 1; i++) {
		for(j = 1; j <= numPtsY + 1; j++) {
			currX(1) = this->priorPDF->dx * (i - 1);
			currX(2) = this->priorPDF->dy * (j - 1);
			error = (currX - mu).t() * invCov * (currX - mu);
			convPDF(i, j) = (1.0 / (2.0 * PI * Cov.Determinant())) *
							exp(-(0.5) * error.AsScalar());
		}
	}
	
	//try using uniform blurring matrix;
	//convPDF = 1.0;
	convPDF *= (1.0 / convPDF.Sum());
	
	//convolve current PDF with gaussian blur PDF
	newPDF = conv2(this->priorPDF->depths, convPDF);
	
	newPDF *= (1.0 / newPDF.Sum());
	this->priorPDF->depths = newPDF;
	return true;
}


void TNavPointMassFilter::motionBlur_FPEexplicit(const double dt, const SymmetricMatrix Cov) {
	Matrix newPDF(this->priorPDF->numX, this->priorPDF->numY);
	double alpha = dt / (8.0 * (fabs(this->priorPDF->dx) *
								fabs(this->priorPDF->dy)));
	logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"alpha (should be less than 0.25): %f \n", alpha);
	
	//Assign indexing arrays
	int* up, *down, *left, *right;
	up = new int[this->priorPDF->numX];
	down = new int[this->priorPDF->numX];
	left = new int[this->priorPDF->numY];
	right = new int[this->priorPDF->numY];
	
	//Define indexing vectors
	up[this->priorPDF->numX - 1] = this->priorPDF->numX;
	down[0] = 1;
	for(int i = 0; i < this->priorPDF->numX - 1; i++) {
		up[i] = i + 2;
		down[i + 1] = i + 1;
	}
	
	left[this->priorPDF->numY - 1] = this->priorPDF->numY;
	right[0] = 1;
	for(int j = 0; j < this->priorPDF->numY - 1; j++) {
		left[j] = j + 2;
		right[j + 1] = j + 1;
	}
	
	//Define convolved PDF using Euler explicit differencing
	for(int row = 0; row < this->priorPDF->numX; row++) {
		for(int col = 0; col < this->priorPDF->numY; col++)
			newPDF(row + 1, col + 1) = this->priorPDF->depths(row + 1, col + 1) +
									   alpha * (Cov(1, 1) * (this->priorPDF->depths(row + 1, left[col]) +
												this->priorPDF->depths(row + 1, right[col]))
												+ Cov(2, 2) * (this->priorPDF->depths(up[row], col + 1) +
														this->priorPDF->depths(down[row], col + 1))
												+ Cov(1, 2) * (this->priorPDF->depths(up[row], right[col]) +
														this->priorPDF->depths(down[row], left[col]) -
														this->priorPDF->depths(down[row], right[col]) -
														this->priorPDF->depths(up[row], left[col]))
												- (2 * Cov(1, 1) + 2 * Cov(2, 2)) * this->priorPDF->depths(row + 1, col + 1));
	}
	
	//Ensure new PDF is normalized to 1
	newPDF *= (1.0 / newPDF.Sum());
	this->priorPDF->depths = newPDF;
	
	delete [] up;
	delete [] down;
	delete [] left;
	delete [] right;
	
	return;
}


void TNavPointMassFilter::plotMatlabSurf(const Matrix& Surf,
		const char* plotTitle,
		const char* figureNum) {
#ifdef USE_MATLAB
	mxArray* A = NULL;
	
	//copy contents of Surf into Matlab variable A
	A = mxCreateDoubleMatrix(Surf.Ncols(), Surf.Nrows(), mxREAL);
	memcpy((void*)mxGetPr(A), (void*) Surf.Store(),
		   Surf.Storage()*sizeof(double));
		   
	//plot surface using Matlab commands
	engPutVariable(matlabEng, "A", A);
	engEvalString(matlabEng, figureNum);
	engEvalString(matlabEng, "h = gcf;");
	//engEvalString(matlabEng, "h = surf(A, 'Linestyle', 'none');");
	engEvalString(matlabEng, "h = imagesc(A); colorbar;");
	//engEvalString(matlabEng, "set(h,'FaceColor','interp','FaceLighting','gouraud');");
	engEvalString(matlabEng, plotTitle);
	//engEvalString(matlabEng, "alpha(.6)");
	engEvalString(matlabEng, "xlabel('North(m)');");
	engEvalString(matlabEng, "ylabel('East(m)');");
	
	//remove memory in Matlab
	mxDestroyArray(A);
	
#else
	logs(TL_OMASK(TL_TNAV_POINT_MASS_FILTER, TL_LOG),"Could not generate plot; Matlab is not set to be used");
	
#endif
	
}
