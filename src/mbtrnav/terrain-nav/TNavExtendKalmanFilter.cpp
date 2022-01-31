/* FILENAME      : TNavExtendedKalmanFilter.cpp
 * AUTHOR        : Debbie Meduna
 * DATE          : 04/30/09
 *
 * LAST MODIFIED : 04/30/09 
 * MODIFIED BY   : Debbie Meduna   
 * -----------------------------------------------------------------------------
 * Modification History
 * -----------------------------------------------------------------------------
 *****************************************************************************/


#include "TNavExtendKalmanFilter.h"

TNavExtendKalmanFilter::TNavExtendKalmanFilter(char *mapName, char *vehicleSpecs, char *directory, const double *windowVar) : TNavFilter(mapName, vehicleSpecs, directory, windowVar)
{
   //initialize covariance to defined search space
   Sigma.ReSize(3); 
   Sigma = 0.0;
 
   mu.ReSize(3);
   mu = 0.0;
   
   numCorr = 0;
   corrData = NULL;
}


TNavExtendKalmanFilter::~TNavExtendKalmanFilter()
{
   if(corrData != NULL)
      delete [] corrData;
   corrData = NULL;
}


void TNavExtendKalmanFilter::initFilter(poseT& initNavPose)
{
   //set filter mean to the current navigation pose
   mu(1) = initNavPose.x;
   mu(2) = initNavPose.y;
   mu(3) = initNavPose.z;
   
   //initialize filter variance
   Sigma(1,1) = initWindowVar[0]/9.0;
   Sigma(2,2) = initWindowVar[2]/9.0;
   Sigma(3,3) = pow(2,2);

}

bool TNavExtendKalmanFilter::measUpdate(measT& currMeas)
{
   bool successfulMeas = true;

   // create correlation data vector.  If correlation vector non-empty, 
   // continue measurement incorporation
   if(generateMeasCorrData(&currMeas))
   {
      // define search space and extract submap for correlation
      int checkMap = extractMap(*this->lastNavPose);
      
      // check that the map extraction worked correctly
      if(checkMap == 1)
      {
         logs(TL_OMASK(TL_TNAV_EXT_KALMAN_FILTER, TL_LOG),"TerrainNav::Measurement from time = %.2f sec. not included; "
                "unable to successfully extract a map segment for correlation"
                , currMeas.time);
         successfulMeas = false;
      }
      else
	{
	  successfulMeas = incorporateMeas();

	  if(!successfulMeas)
	    logs(TL_OMASK(TL_TNAV_EXT_KALMAN_FILTER, TL_LOG),"TerrainNav::Measurement from time = %.2f sec. not included;"
		   " not enough valid depth information extracted.\n", currMeas.time);
	}

   }
   else
   {
      logs(TL_OMASK(TL_TNAV_EXT_KALMAN_FILTER, TL_LOG),"TerrainNav::Measurement from time = %.2f sec. not included; there"
	     " are no good beams from the sonar.\n", currMeas.time);
      successfulMeas = false;
   }

   //remove memory associated with correlation data
   delete [] corrData;
   this->numCorr = 0;
   this->corrData = NULL;
  
   return successfulMeas;
}


void TNavExtendKalmanFilter::motionUpdate(poseT& currNavPose)
{
   double dx, dy, dz;
   double dist;
   double cep;
   double currSigmaSq;
   SymmetricMatrix R(3);
   R = 0.0;

   //define delta vehicle motion 
   dx = currNavPose.x - this->lastNavPose->x;
   dy = currNavPose.y - this->lastNavPose->y;
   dz = currNavPose.z - this->lastNavPose->z;

   //update mean with delta vehicle motion
   mu(1) += dx;
   mu(2) += dy;
   mu(3) += dz;

   //Define sigma of the INS drift based on Circular Error Probable
   dist = sqrt(dx*dx + dy*dy);
   cep = (vehicle->driftRate/100.0)*dist;
   currSigmaSq = cep/sqrt(-2*(log(1-0.5)));
   R(1,1) = currSigmaSq;
   R(2,2) = currSigmaSq;
   R(3,3) = 1.0;

   //update filter Sigma with R
   Sigma += R;
}


void TNavExtendKalmanFilter::computeMLE(poseT* mlePose)
{
   mlePose->x = mu(1);
   mlePose->y = mu(2);
   mlePose->z = mu(3);
}


void TNavExtendKalmanFilter::computeMMSE(poseT* mmsePose)
{
   mmsePose->x = mu(1);
   mmsePose->y = mu(2);
   mmsePose->z = mu(3);
   mmsePose->covariance[0] = Sigma(1,1);
   mmsePose->covariance[1] = Sigma(2,1);
   mmsePose->covariance[2] = Sigma(2,2);
   mmsePose->covariance[5] = Sigma(3,3);
}


void TNavExtendKalmanFilter::checkConvergence()
{
   this->converged = true;
}

void TNavExtendKalmanFilter::saveCurrDistrib(ofstream &outputFile)
{
   int i;

   //check that the desired file exists and is open for writing
   if(!outputFile.is_open())
   {
      logs(TL_OMASK(TL_TNAV_EXT_KALMAN_FILTER, TL_LOG),"Error:Tried to write to an unopened file."
             " Ignoring write command.");
      return;
   }

   for(i = 1; i <= 3; i++)
      outputFile << setprecision(15) << mu(i) << '\t' << Sigma(i,1) 
                 << '\t' << Sigma(i,2) << '\t' << Sigma(i,3) << endl;
}

bool TNavExtendKalmanFilter::incorporateMeas()
{
  ColumnVector Error(this->numCorr);
  double currTerrainDepth, currVar;
  Matrix Temp(this->numCorr, this->numCorr);
  Matrix K;
  Matrix H1(1,2);
  Matrix H, Htemp(this->numCorr,3);
  Matrix W(this->numCorr, this->numCorr);
  IdentityMatrix I(3);
  W = 0.0;
  Htemp = -1.0;
  
  //Determine valid gradients and compute corresponding error terms
  int numGood = 0;
  for(int i = 1; i <= numCorr; i++)
  {
     //Interpolate depth and gradients
     interpolateDepthAndGradient(mu(1)+corrData[numCorr-i].dx, 
                                 mu(2)+corrData[numCorr-i].dy, 
                                 currTerrainDepth, currVar, H1);

     //Ensure that we have valid gradients and terrain depth
     if(isnan(H1(1,1)) | isnan(H1(1,2)) | isnan(currTerrainDepth))
	continue;

     numGood++;
     Htemp(numGood,1) = H1(1,1);
     Htemp(numGood,2) = H1(1,2);
     W(numGood,numGood) = currVar;
     W(numGood,numGood) += 0.0016*(pow(corrData[numCorr-i].dx,2)+ 
                                   pow(corrData[numCorr-i].dy,2)+
                                   pow(corrData[numCorr-i].dz,2));
     
     //Compute error vector: y - h(mu)
     Error(numGood) = mu(3) + corrData[numCorr-i].dz - fabs(currTerrainDepth);
  }
  
  //Check that we have valid map information for at least one beam:
  if(numGood == 0)
     return false;

  Temp.ReSize(numGood, numGood);
  K.ReSize(3, numGood);
  H = Htemp.SubMatrix(1,numGood,1,3);

  //Compute kalman gain
  Temp = H*Sigma*H.t() + W.SubMatrix(1,numGood,1,numGood);
  K = Sigma*H.t()*Temp.i();

  //update mean and covariance
  mu = mu + K*Error.SubMatrix(1,numGood,1,1);
  Sigma << (I - K*H)*Sigma;

  return true;  
}

bool TNavExtendKalmanFilter::generateMeasCorrData(measT* currMeas)
{
   corrT *newCorr;
   Matrix beamsVF(3,currMeas->numMeas);
   Matrix beamsIF, Rvi;
   double attitude[3] = {currMeas->phi, currMeas->theta, currMeas->psi};

   //Define attitude angles either from measurement or from navigation estimate
   if(this->interpMeasAttitude)
   {
      attitude[0] = lastNavPose->phi;
      attitude[1] = lastNavPose->theta;
      attitude[2] = lastNavPose->psi;
   }
  
   //generate beam direction vectors in vehicle frame
   if(!projectMeasVF(beamsVF,*currMeas))
      return false;

   //ensure that corrData is empty before redefining
   this->numCorr = beamsVF.Ncols();
   if(this->corrData != NULL)
   {
      delete [] this->corrData;
      this->corrData = NULL;
   }

   if(this->numCorr != currMeas->numMeas)
      logs(TL_OMASK(TL_TNAV_EXT_KALMAN_FILTER, TL_LOG),"Excluded %i beam(s) from correlation due to poor sonar data.\n", 
             currMeas->numMeas - this->numCorr);

   //rotate measurement vectors from vehicle to inertial frame
   Rvi = getRotMatrix(attitude);
   beamsIF = Rvi.t()*beamsVF;

   //add measurement to correlation data vector
   newCorr = new corrT[this->numCorr];
   for (int i = 1; i <= this->numCorr; i++)
   {
      newCorr[i-1].dx = beamsIF(1,i);
      newCorr[i-1].dy = beamsIF(2,i);
      newCorr[i-1].dz = beamsIF(3,i);
   }

   //define corrData. If averaging beams, corrData will have size 1.
   if(!AVERAGE)
      this->corrData = newCorr;    
   else
   {
      this->corrData = new corrT[1];

      for (int i = 1; i < numCorr; i++)
      {
         newCorr[0].dx += newCorr[i].dx;
         newCorr[0].dy += newCorr[i].dy;
         newCorr[0].dz += newCorr[i].dz;
      }
		
      newCorr[0].dx = newCorr[0].dx/numCorr;
      newCorr[0].dy = newCorr[0].dy/numCorr;
      newCorr[0].dz = newCorr[0].dz/numCorr;

      corrData[0] = newCorr[0];
      this->numCorr = 1;
      delete [] newCorr;
   }	
  
   return true;  
}


int TNavExtendKalmanFilter::extractMap(const poseT &currPose)
{
   double max_dx, max_dy;
   double xCen, yCen;
   double mapSearch[2];
   double numXdesired, numYdesired;
   int mapStatus;
 
   //Define maximum beam projection distances
   max_dx = 0;
   max_dy = 0;

   for(int i = 0; i < numCorr; i++)
   {
      double dx = fabs(corrData[i].dx);
      if(dx > max_dx)
         max_dx = dx;
           
      double dy = fabs(corrData[i].dy);
      if(dy > max_dy)
         max_dy = dy;
   }

   //Define desired search area in meters
   xCen = currPose.x;
   yCen = currPose.y;    

   numXdesired = 3*sqrt(Sigma(1,1)) + max_dx + 
     2*fabs(terrainMap->refMap->bounds->dx);
   numYdesired = 3*sqrt(Sigma(2,2)) + max_dy + 
     2*fabs(terrainMap->refMap->bounds->dy);  

   mapSearch[0] = 2.0*numXdesired; 
   mapSearch[1] = 2.0*numYdesired; 

   //ask to extract a map based on vehicle location and search bounds
   mapStatus = loadSubMap(xCen, yCen, mapSearch);

   return mapStatus;   
}
