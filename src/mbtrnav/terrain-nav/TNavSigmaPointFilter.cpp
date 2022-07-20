/* FILENAME      : TNavSigmaPointFilter.cpp
 * AUTHOR        : Debbie Meduna
 * DATE          : 04/24/09
 *
 * LAST MODIFIED : 05/20/10
 * MODIFIED BY   : Debbie Meduna
 * -----------------------------------------------------------------------------
 * Modification History
 * -----------------------------------------------------------------------------
 *****************************************************************************/

#include "TNavSigmaPointFilter.h"

TNavSigmaPointFilter::TNavSigmaPointFilter(char *mapName, char *vehicleSpecs, char *directory, const double *windowVar) : TNavFilter(mapName, vehicleSpecs, directory, windowVar),
procNoiseStartIdx(0),
measNoiseStartIdx(0)
{
    initVariables();
}


TNavSigmaPointFilter::~TNavSigmaPointFilter()
{
    if(sigmaPoints != NULL)
        delete [] sigmaPoints;
    sigmaPoints = NULL;

    //close files
    sigmaPointBeforeFile.close();
    sigmaPointAfterFile.close();
}


void TNavSigmaPointFilter::initFilter(poseT& initNavPose)
{
    //set filter mean to the current navigation pose
    mu(1) = initNavPose.x;
    mu(2) = initNavPose.y;
    mu(3) = initNavPose.z;
    int k = 3;
    if(ALLOW_ATTITUDE_SEARCH) {
        mu(k+1) = initNavPose.phi;
        mu(k+2) = initNavPose.theta;
        mu(k+3) = initNavPose.psi;
        k += 3;
    }
    if(SEARCH_GYRO_BIAS) {
        mu(k+1) = initNavPose.wy;
        mu(k+2) = initNavPose.wz;
        k+=2;
    }
    if(AUGMENT_STATE)
    {
        procNoiseStartIdx = k+1;
        measNoiseStartIdx = k+9;
        //add process noise terms
        for(int i = k+1; i <= k+16; i++)
        mu(i) = 0.0;
    }

    //initialize filter variance
    if(initNavPose.covariance[0] == 0) {
        Sigma(1,1) = initWindowVar[0];
        Sigma(2,2) = initWindowVar[2];
        Sigma(3,3) = initWindowVar[5];
        k = 3;
        if(ALLOW_ATTITUDE_SEARCH) {
            Sigma(k+1,k+1) = initWindowVar[9];
            Sigma(k+2,k+2) = initWindowVar[14];
            Sigma(k+3,k+3) = initWindowVar[20];
            k +=3;
        }
        if(SEARCH_GYRO_BIAS) {
            Sigma(k+1,k+1) = initWindowVar[27];
            Sigma(k+2,k+2) = initWindowVar[35];
            k+=2;
        }
        if(AUGMENT_STATE) {
            double velError=0.0;
            if(initNavPose.bottomLock)
                velError = VEL_PER_ERROR/100.0;
            else
                velError = WATER_VEL_PER_ERROR/100.0;
            Sigma(k+1,k+1) = velError*initNavPose.vx;
            Sigma(k+2,k+2) = velError*initNavPose.vy;
            Sigma(k+3,k+3) = velError*initNavPose.vz;
            Sigma.SubMatrix(k+4,k+6,k+4,k+6) << Rv_sub;
            Sigma(k+7,k+7) = DGBIAS_ERROR*DGBIAS_ERROR;
            Sigma(k+8,k+8) = Sigma(k+7,k+7);
        }
    } else {
        Sigma(1,1) = initNavPose.covariance[0]*initNavPose.covariance[0];//pow(20/3.0,2);
        Sigma(2,2) = initNavPose.covariance[1]*initNavPose.covariance[1];//pow(20/3.0,2);
        Sigma(3,3) = initNavPose.covariance[3]*initNavPose.covariance[3];//pow(2,2);
        if(ALLOW_ATTITUDE_SEARCH){
            Sigma(4,4) = initNavPose.covariance[4]*initNavPose.covariance[4];
            Sigma(5,5) = initNavPose.covariance[5]*initNavPose.covariance[5];
            Sigma(6,6) = initNavPose.covariance[6]*initNavPose.covariance[6];
        }
    }

    //initialize sigma points
    initSigmaPoints();
}


bool TNavSigmaPointFilter::measUpdate(measT& currMeas)
{
    Matrix beamsVF(3, currMeas.numMeas);
    DiagonalMatrix Rn(currMeas.numMeas);
    bool successfulMeas = false;

    //project beams into vehicle frame
    successfulMeas = projectMeasVF(beamsVF, currMeas);

    //if projection was successful, load relevant submap and apply
    //measurement update to each particle
    if(successfulMeas) {
        //Load a sub-map for measurement correlation
        int mapStatus = defineAndLoadSubMap(beamsVF);

        // check that the map extraction worked correctly
        if(mapStatus == 1) {
            logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"TerrainNav::Measurement from time = %.2f sec. not included; "
                 "unable to successfully extract a map segment for correlation"
                 , currMeas.time);
            successfulMeas = false;
        } else {
            successfulMeas = incorporateMeas(beamsVF, currMeas);
            if(!successfulMeas)
                logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"TerrainNav::Measurement from time = %.2f sec. not included;"
                     " not enough valid depth information extracted.\n",
                     currMeas.time);
        }
    } else {
        logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"TerrainNav::Measurement from time = %.2f sec. not included; there"
             " are no good beams from the sonar.\n", currMeas.time);
        successfulMeas = false;
    }

    return successfulMeas;
}


void TNavSigmaPointFilter::motionUpdate(poseT& currNavPose)
{
    int i;
    SymmetricMatrix R;
    double velocity_vf[3];
    double velocity_if[3];
    poseT diffPose = currNavPose;
    double cTheta, psiDot;
    double currAttitude[3];

    //define delta vehicle motion
    diffPose -= *lastNavPose;

    //compute new motion uncertainty matrix
    if(AUGMENT_STATE) {
        R = computeMotionSigmaAugmentState(diffPose);
        Sigma.SymSubMatrix(procNoiseStartIdx,procNoiseStartIdx+numStates-1) << R;
    }else{
        R = computeMotionSigma(diffPose);
    }

    //recompute sigma points
    updateSigmaPointPoses();

    //Apply motion update to all sigma points
    for(i = 0; i < numSigmaPoints; i++) {
        //define velocity
        velocity_vf[0] = lastNavPose->vx;
        velocity_vf[1] = lastNavPose->vy;
        velocity_vf[2] = lastNavPose->vz;

        //Update z
        sigmaPoints[i].pose(3) += diffPose.z;
        if(AUGMENT_STATE){
            sigmaPoints[i].pose(3) += sigmaPoints[i].pose(procNoiseStartIdx+3);
        }
        //if there is valid GPS data, use the stored INS pose information to perform
        //the motion update.  Otherwise perform a dead-reckoning motion update
        if(diffPose.gpsValid) {
            //update sigma points with delta vehicle motion
            sigmaPoints[i].pose(1) += diffPose.x;
            sigmaPoints[i].pose(2) += diffPose.y;
            if(AUGMENT_STATE)
            {
                sigmaPoints[i].pose(1) += sigmaPoints[i].pose(procNoiseStartIdx);
                sigmaPoints[i].pose(2) += sigmaPoints[i].pose(procNoiseStartIdx+1);
            }
        } else {
            if(AUGMENT_STATE) {
                velocity_vf[0] += sigmaPoints[i].pose(procNoiseStartIdx);
                velocity_vf[1] += sigmaPoints[i].pose(procNoiseStartIdx+1);
                velocity_vf[2] += sigmaPoints[i].pose(procNoiseStartIdx+2);
            }
            if(ALLOW_ATTITUDE_SEARCH) {
                currAttitude[0] = sigmaPoints[i].pose(4);
                currAttitude[1] = sigmaPoints[i].pose(5);
                currAttitude[2] = sigmaPoints[i].pose(6);
            } else {
                currAttitude[0] = lastNavPose->phi;
                currAttitude[1] = lastNavPose->theta;
                currAttitude[2] = lastNavPose->psi;
            }
            applyVecRotation(currAttitude, velocity_vf, velocity_if);
            sigmaPoints[i].pose(1) += velocity_if[0]*diffPose.time;
            sigmaPoints[i].pose(2) += velocity_if[1]*diffPose.time;
        }

        //Perform attitude and gyro bias motion updates
        if(ALLOW_ATTITUDE_SEARCH) {
            //Update psi
            if(SEARCH_GYRO_BIAS) {
                cTheta = cos(sigmaPoints[i].pose(5));
                psiDot = (sin(sigmaPoints[i].pose(4))/cTheta)*
                (lastNavPose->wy - sigmaPoints[i].pose(7)) +
                (cos(sigmaPoints[i].pose(4))/cTheta)*
                (lastNavPose->wz - sigmaPoints[i].pose(8));
                sigmaPoints[i].pose(6) += psiDot*diffPose.time;
                if(AUGMENT_STATE) {
                    sigmaPoints[i].pose(7) += sigmaPoints[i].pose(procNoiseStartIdx+6);
                    sigmaPoints[i].pose(8) += sigmaPoints[i].pose(procNoiseStartIdx+7);
                }
            } else{
                sigmaPoints[i].pose(6) += diffPose.psi;
            }

            //Update phi & theta
            sigmaPoints[i].pose(4) += diffPose.phi;
            sigmaPoints[i].pose(5) += diffPose.theta;
            if(AUGMENT_STATE) {
                sigmaPoints[i].pose(4) += sigmaPoints[i].pose(procNoiseStartIdx+4);
                sigmaPoints[i].pose(5) += sigmaPoints[i].pose(procNoiseStartIdx+5);
            }
        }
    }
    //save old sigmapoints
    saveCurrDistrib(sigmaPointBeforeFile);

    //update mean according to weighted sigma points
    mu = 0.0;
    for(i = 0; i < numSigmaPoints; i++)
    mu += sigmaPoints[i].weight_m*sigmaPoints[i].pose;

    //update Sigma based on motion updated sigma points
    SymmetricMatrix temp(numStates);
    ColumnVector tempCol;
    temp = 0;
    for(i = 0; i < numSigmaPoints; i++) {
        tempCol = sigmaPoints[i].pose.Rows(1,numStates) - mu.Rows(1,numStates);
        temp << temp + sigmaPoints[i].weight_c*tempCol*tempCol.t();
    }
    Sigma.SymSubMatrix(1,numStates) << temp;

    if(!AUGMENT_STATE) {
        //update filter Sigma with R
        Sigma += R;
    }
    saveCurrDistrib(sigmaPointAfterFile);

    //update sigma point poses with new mu and sigma
    //updateSigmaPointPoses();
}


void TNavSigmaPointFilter::computeMLE(poseT* mlePose)
{
    mlePose->x = mu(1);
    mlePose->y = mu(2);
    mlePose->z = mu(3);
    if(ALLOW_ATTITUDE_SEARCH) {
        mlePose->phi = mu(4);
        mlePose->theta = mu(5);
        mlePose->psi = mu(6);
    }
}


void TNavSigmaPointFilter::computeMMSE(poseT* mmsePose)
{
    mmsePose->x = mu(1);
    mmsePose->y = mu(2);
    mmsePose->z = mu(3);
    mmsePose->covariance[0] = Sigma(1,1);
    mmsePose->covariance[1] = Sigma(2,1);
    mmsePose->covariance[2] = Sigma(2,2);
    mmsePose->covariance[5] = Sigma(3,3);
    if(ALLOW_ATTITUDE_SEARCH) {
        mmsePose->phi = mu(4);
        mmsePose->theta = mu(5);
        mmsePose->psi = mu(6);
        mmsePose->covariance[9] = Sigma(4,4);
        mmsePose->covariance[14] = Sigma(5,5);
        mmsePose->covariance[20] = Sigma(6,6);
    }
}


void TNavSigmaPointFilter::checkConvergence()
{
    this->converged = true;
}

void TNavSigmaPointFilter::saveCurrDistrib(ofstream &outputFile)
{
    int i,j;

    //check that the desired file exists and is open for writing
    if(!outputFile.is_open()) {
        logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"Error:Tried to write to an unopened file."
             " Ignoring write command.");
        return;
    }

    //write mu vector and sigma matrix to file
    for(i = 1; i <= numAugStates; i++){
        outputFile << setprecision(15) << mu(i);
        for(j = 1; j <= numAugStates; j++)
        outputFile << setprecision(15) << '\t' << Sigma(i,j);
        outputFile << '\t' << endl;
    }

    //write sigma points to file
    for(i = 0; i < numSigmaPoints; i++) {
        outputFile << setprecision(15) << sigmaPoints[i].weight_m << '\t'
        << sigmaPoints[i].weight_c << '\t' << sigmaPoints[i].pose(1)
        << '\t' << sigmaPoints[i].pose(2) << '\t' << sigmaPoints[i].pose(3);
        if(ALLOW_ATTITUDE_SEARCH)
            outputFile << '\t' << sigmaPoints[i].pose(4)  << '\t' << sigmaPoints[i].pose(5)
            << '\t' << sigmaPoints[i].pose(6);
        if(SEARCH_GYRO_BIAS)
            outputFile << '\t' << sigmaPoints[i].pose(7) << '\t'
            << sigmaPoints[i].pose(8);
        outputFile << endl;
    }
}

void TNavSigmaPointFilter::initVariables()
{
    numStates = 3;

    //initialize number of states
    if(ALLOW_ATTITUDE_SEARCH)
        numStates += 3;

    if(SEARCH_GYRO_BIAS)
        numStates += 2;

    numAugStates = numStates;
    if(AUGMENT_STATE)
        numAugStates += 16;

    //initialize mean and covariance to defined search space
    Sigma.ReSize(numAugStates);
    Sigma = 0.0;

    mu.ReSize(numAugStates);
    mu = 0.0;

    //initialize sigma points
    numSigmaPoints = numAugStates*2+1;
    sigmaPoints = NULL;
    lambda = ALPHA*ALPHA*(numAugStates+KAPPA)-numAugStates;

    //initialize process noise sub-matrix
    Rv_sub.ReSize(3);
    Rv_sub(1) = DZ_STDDEV*DZ_STDDEV;
    Rv_sub(2) = DPHI_STDDEV*DPHI_STDDEV;
    Rv_sub(3) = DTHETA_STDDEV*DTHETA_STDDEV;

    //open files for saving
    sigmaPointBeforeFile.open("sigmaPointBefore.txt");
    sigmaPointAfterFile.open("sigmaPointAfter.txt");
}

void TNavSigmaPointFilter::initSigmaPoints()
{
    int i;

    //initialize sigma point array
    sigmaPoints = new sigmaPointT[numSigmaPoints];

    //initialize sigma point pose vectors based on search space
    for(i = 0; i < numSigmaPoints; i++)
    sigmaPoints[i].pose.ReSize(numAugStates);

    //initialize sigma point weight information
    /*sigmaPoints[0].weight_m = 1.0 - numStates/pow(SIGMA_FACTOR,2);
     sigmaPoints[0].weight_c = sigmaPoints[0].weight_m;
     for(i = 1; i < numSigmaPoints; i++)
     {
     sigmaPoints[i].weight_m = 1.0/(2.0*pow(SIGMA_FACTOR,2));
     sigmaPoints[i].weight_c = sigmaPoints[i].weight_m;
     }*/

    //initialize sigma point weights based on the Scaled Unscented Transform
    //described in Van der Merwe's thesis and developed by Julier.
    sigmaPoints[0].weight_m = lambda/(numAugStates + lambda);
    sigmaPoints[0].weight_c = sigmaPoints[0].weight_m + (1-ALPHA*ALPHA+BETA);
    for(i = 1; i < numSigmaPoints; i++) {
        sigmaPoints[i].weight_m = 1.0/(2.0*(numAugStates+lambda));
        sigmaPoints[i].weight_c = sigmaPoints[i].weight_m;
    }

    //initialize sigma point pose information
    updateSigmaPointPoses();

}

void TNavSigmaPointFilter::updateSigmaPointPoses()
{
    //LowerTriangularMatrix C;
    SymmetricMatrix C;
    int i;
    //double scaleFactor = SIGMA_FACTOR;
    double scaleFactor = sqrt(numAugStates + lambda);

    //compute Cholesky decomposition of current Sigma
    //C = Cholesky(Sigma);

    //compute Sigma sqrt
    C = computeMatrixSqrt(Sigma);

    //fill in mean for first sigma point
    sigmaPoints[0].pose = mu;

    //fill in surrounding points
    for(i = 1; i <= numAugStates; i++)
    sigmaPoints[i].pose = mu + scaleFactor*C.Column(i);

    for(i = numAugStates+1; i <= 2*numAugStates; i++)
    sigmaPoints[i].pose = mu - scaleFactor*C.Column(i-numAugStates);
}


bool TNavSigmaPointFilter::incorporateMeas(Matrix& beamsVF, measT& currMeas)
{
    Matrix beamsMF;
    int numBeams = beamsVF.Ncols();
    Matrix E(numBeams, this->numSigmaPoints);
    double mapZ, mapVar;
    Matrix Pz(numBeams, numBeams);
    Matrix Pxz(this->numStates, numBeams);
    Matrix K, Et;
    double ranges[numBeams];
    ColumnVector Ebar(numBeams);
    double currAttitude[3];
    DiagonalMatrix Rn(numBeams);
    int i, j;
    Rn = 0.0;
    Pz = 0.0;
    Ebar = 0.0;
    E = 0.0;
    Pxz = 0.0;

    //If augmenting the state, recompute sigma points for range noise
    if(AUGMENT_STATE) {
        for(i = 0; i < numBeams; i++) {
            Rn(i+1) = currMeas.covariance[i];
            ranges[i] = currMeas.ranges[i];
        }
        Sigma.SymSubMatrix(measNoiseStartIdx,measNoiseStartIdx+numBeams-1) << Rn;

        //update sigma points based on new measurement noise
        updateSigmaPointPoses();
    }

    //compute expected measured terrain height for all sigma points
    for(i = 0; i < numSigmaPoints; i++) {
        //if augmenting the state, add range noise prior to beam projection
        if(AUGMENT_STATE) {
            for(j = 0; j < numBeams; j++)
            currMeas.ranges[j] = ranges[j];
            for(j = 0; j < numBeams; j++)
            currMeas.ranges[j] += sigmaPoints[i].pose(measNoiseStartIdx+j);
            
            //reproject beams given noisy ranges
            projectMeasVF(beamsVF, currMeas);
        }

        //rotate beams from vehicle frame to the map frame
        if(ALLOW_ATTITUDE_SEARCH) {
            currAttitude[0] = sigmaPoints[i].pose(4);
            currAttitude[1] = sigmaPoints[i].pose(5);
            currAttitude[2] = sigmaPoints[i].pose(6);
            beamsMF = applyRotation(currAttitude, beamsVF);
        } else{
            beamsMF = beamsVF;
        }

        //Determine terrain depth for each sigma point measurement
        for(j = 0; j < numBeams; j++) {
            //Interpolate depth
            interpolateDepth(sigmaPoints[i].pose(1)+beamsMF(1,j+1),
                             sigmaPoints[i].pose(2)+beamsMF(2,j+1),
                             mapZ, mapVar);

            //If the interpolated depth is a valid number, use it in correlation
            // if(!isnan(mapZ))
            if(!ISNIN(mapZ)) {
                Rn(j+1) = mapVar;
                if(!AUGMENT_STATE)
                    Rn(j+1) += currMeas.covariance[j];

                //Compute expected measurement error for current sigma point
                //e = h(x,n)-z
                E(j+1,i+1) = fabs(mapZ)-sigmaPoints[i].pose(3)-beamsMF(3,j+1);
            } else {
                //if we don't want to use nan values, don't incorporate this measurement
                if(!USE_MAP_NAN) {
                    logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"TerrainNav::Measurement from time = %.2f sec. not included; "
                         "encountered NaN values in the correlation map segment\n",
                         currMeas.time);
                    return false;
                }
            }
        }
        Pz += sigmaPoints[i].weight_c*Rn;
        Ebar += sigmaPoints[i].weight_m*E.Column(i+1);
    }
    //compute covariance and cross-covariance matrices
    Et = E.t();
    for(i = 0; i < numSigmaPoints; i++) {
        Pz << Pz + sigmaPoints[i].weight_c*(E.Column(i+1)-Ebar)*
        (Et.Row(i+1) - Ebar.t());
        Pxz << Pxz + sigmaPoints[i].weight_c*(mu.Rows(1,numStates)-sigmaPoints[i].pose.Rows(1,numStates))*
        (Et.Row(i+1) - Ebar.t());
    }
    
    //Compute kalman gain
    K = Pxz*Pz.i();

    //update mean and covariance
    mu.Rows(1,numStates) += K*Ebar;
    Sigma.SymSubMatrix(1,numStates) << Sigma.SymSubMatrix(1,numStates)- K*Pz*K.t();

    //update sigma point poses
    //updateSigmaPointPoses();

    return true;
}


int TNavSigmaPointFilter::defineAndLoadSubMap(const Matrix &beamsVF)
{
    double max_dx, max_dy;
    double Nmin, Nmax, Emin, Emax;
    double maxAttitude[3] = {0,0,0};
    Matrix beamsMF;
    double mapSearch[2];
    double numXdesired, numYdesired;
    int mapStatus;

    //Find max position and attitude of sigma points
    Nmin = mu(1);
    Nmax = Nmin;
    Emin = mu(2);
    Emax = Emin;

    for(int i = 0; i < numSigmaPoints; i++) {
        Nmin = min(Nmin, sigmaPoints[i].pose(1));
        Nmax = max(Nmax, sigmaPoints[i].pose(1));
        Emin = min(Emin, sigmaPoints[i].pose(2));
        Emax = max(Emax, sigmaPoints[i].pose(2));
        if(ALLOW_ATTITUDE_SEARCH) {
            for(int j = 0; j < 3; j++)
            maxAttitude[j] = max(maxAttitude[j], fabs(sigmaPoints[i].pose(4+j)));
        }
    }

    beamsMF = applyRotation(maxAttitude, beamsVF);

    //Define maximum beam projection distances
    max_dx = 0;
    max_dy = 0;

    for(int i = 0; i < beamsMF.Ncols(); i++) {
        double dx = fabs(beamsMF(1,i+1));
        if(dx > max_dx)
            max_dx = dx;

        double dy = fabs(beamsMF(2,i+1));
        if(dy > max_dy)
            max_dy = dy;
    }

    //Define desired search area in meters
    numXdesired = (Nmax-Nmin)/2.0 + 1.5*max_dx +
    2*fabs(terrainMap->refMap->bounds->dx);
    numYdesired = (Emax-Emin)/2.0 + 1.5*max_dy +
    2*fabs(terrainMap->refMap->bounds->dy);

    mapSearch[0] = 2.0*numXdesired;
    mapSearch[1] = 2.0*numYdesired;

    //ask to extract a map based on vehicle location and search bounds
    mapStatus = loadSubMap(mu(1), mu(2), mapSearch);

    return mapStatus;
}

SymmetricMatrix TNavSigmaPointFilter::computeMotionSigmaAugmentState(const poseT& diffPose)
{
    SymmetricMatrix R(numStates);
    R = 0.0;

    if(SEARCH_GYRO_BIAS) {
        double gyroStddev = 0.0;
        //compute gyro bias uncertainty
        if(diffPose.time > 0)
            gyroStddev = DGBIAS_ERROR/sqrt(diffPose.time);

        //Fill in gyro bias variance
        R(7,7) = gyroStddev;
        R(8,8) = gyroStddev;
    }
    //Fill in fixed variance variables (z, phi, theta)
    R.SubMatrix(4,6,4,6) = Rv_sub;

    //if there is valid GPS data, compute x/y uncertainty based on CEP drift
    if(diffPose.gpsValid) {
        //Define sigma of the INS drift based on Circular Error Probable
        double dist = sqrt(diffPose.x*diffPose.x + diffPose.y*diffPose.y);
        double cep = (vehicle->driftRate/100.0)*dist;
        double currSigmaSq = cep/sqrt(-2*(log(1-0.5)));
        R(1,1) = currSigmaSq;
        R(2,2) = currSigmaSq;
    } else {
        double velStddev = 0.0;
        if(lastNavPose->bottomLock)
            velStddev = VEL_PER_ERROR/100.0;
        else
            velStddev = WATER_VEL_PER_ERROR/100.0;
        R(1,1) = pow(lastNavPose->vx*velStddev,2);
        R(2,2) = pow(lastNavPose->vy*velStddev,2);
        R(3,3) = pow(lastNavPose->vz*velStddev,2);
    }

    return R;
}


SymmetricMatrix TNavSigmaPointFilter::computeMotionSigma(const poseT& diffPose)
{
    SymmetricMatrix R(numStates);
    R = 0.0;

    if(SEARCH_GYRO_BIAS) {
        double gyroStddev = 0.0;
        //compute gyro bias uncertainty
        if(diffPose.time > 0)
            gyroStddev = DGBIAS_ERROR/sqrt(diffPose.time);

        //Fill in gyro bias variance
        R(7,7) = gyroStddev;
        R(8,8) = gyroStddev;
    }

    //if there is valid GPS data, compute x/y uncertainty based on CEP drift
    if(diffPose.gpsValid) {
        //Define sigma of the INS drift based on Circular Error Probable
        double dist = sqrt(diffPose.x*diffPose.x + diffPose.y*diffPose.y);
        double cep = (vehicle->driftRate/100.0)*dist;
        double currSigmaSq = cep/sqrt(-2*(log(1-0.5)));
        R(1,1) = currSigmaSq;
        R(2,2) = currSigmaSq;
    } else {
        DiagonalMatrix Cv(3);
        double lastAttitude[3] = {lastNavPose->phi,lastNavPose->theta,
            lastNavPose->psi};
        double velStddev = 0.0;

        Matrix R_vi;
        R_vi = getRotMatrix(lastAttitude);
        if(lastNavPose->bottomLock)
            velStddev = VEL_PER_ERROR/100.0;
        else
            velStddev = WATER_VEL_PER_ERROR/100.0;
        Cv(1) = pow(lastNavPose->vx*velStddev*diffPose.time,2);
        Cv(2) = pow(lastNavPose->vy*velStddev*diffPose.time,2);
        Cv(3) = pow(lastNavPose->vz*velStddev*diffPose.time,2);
        R.SubMatrix(1,3,1,3) << R_vi.t()*Cv*R_vi;
    }

    //fill in z,phi and theta and psivariances
    R.SubMatrix(3,5,3,5) = Rv_sub;
    R(6,6) = DPSI_STDDEV*DPSI_STDDEV;

    return R;
}
