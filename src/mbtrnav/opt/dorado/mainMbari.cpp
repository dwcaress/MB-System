#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <semaphore.h>

#include <cstdlib>

#ifdef use_namespace
using namespace std;
#endif

#include "TerrainNav.h"
#include "TerrainNavClient.h"
#include "matrixArrayCalcs.h"
#include "genFilterDefs.h"

#define NANOSEC_PER_SEC (1000000000L)

void runTerrainNav(const Matrix& dataKft, const Matrix& dataMeas,
				   int map_type, char* mapPath,
				   int interpMethod, int kSubSample,
				   int mSubSample, poseT* tercomEst, poseT* mmseEst,
				   bool realTime, int filterType, int dataK_init);

Matrix readDataFromFile(const char* fileName, const int& numRows,
						const int& numCols);

bool assignKearfottEstimate(poseT* currEstimate, Matrix pose);

void assignDVLMeasurement(measT* currMeas, Matrix meas);

void assignIdtMeasurement(measT* currMeas, Matrix meas);

void assignMBMeasurement(measT* currMeas, Matrix meas);

void assignAltMeasurement(measT* currMeas, Matrix meas);

static char* trnHost = NULL;
static int   trnPort = 27027;

int main(int argc, char** argv) {
	bool realTime = false;
	int i, j, idx;
	int numRepeat = 1;
	int map_type = 2;  // 1=GRD, 2=OCTREE
	bool PrintVerboseResults = false;
	
	for(i = 1; i < argc; i++) {
		//If the user passes an argument starting with -r, run the computation
		//loop below in real time.
		if(!strncmp(argv[i], "-r", 2)) {
			realTime = true;
		}
		
		//If the user passes an argument starting with -N, set the number of
		//runs to the next input argument
		if(!strncmp(argv[i], "-N", 2)) {
			numRepeat = atoi(argv[i + 1]);
		}
		
		//If the user passes an argument starting with -h, set the TRN host
		//to the next input argument
		if(!strncmp(argv[i], "-h", 2)) {
			trnHost = strdup(argv[i + 1]);
		}
		
		//If the user passes an argument starting with -p, set the TRN host
		//to the next input argument
		if(!strncmp(argv[i], "-p", 2)) {
			trnPort = atoi(argv[i + 1]);
		}
		
		//If the user passes an argument starting with -m, set the map type;
		//
		if(!strncmp(argv[i], "-m", 2)) {
			map_type = atoi(argv[i + 1]);
		}
		
		if(!strncmp(argv[i], "-v", 2)) {
			PrintVerboseResults = true;
		}
		
	}
	
	printf("Using map_type %d\n", map_type);
	
//initialize memory for different tests
	int numTests = 3;
	int num = 0;
	int* dataK_numRows = new int[numTests];
	int* dataMeas_numRows = new int[numTests];
	int* init_dataK = new int[numTests];
	char** dataK_files = new char*[numTests];
	char** dataMeas_files = new char*[numTests];
	char** map_files = new char*[numTests];
	double** mmseResults = new double*[numTests];
	bool* successful = new bool[numTests*numRepeat];
	
	//for verbose printouts of the results of the tests
	double* finalErrorX = new double[numTests*numRepeat];
	double* finalEstimatorUncertaintyX = new double[numTests*numRepeat];
	double* finalErrorY = new double[numTests*numRepeat];
	double* finalEstimatorUncertaintyY = new double[numTests*numRepeat];
	
	for(i = 0; i < numTests; i++) {
		dataK_files[i] = new char[256];
		dataMeas_files[i] = new char[256];
		map_files[i] = new char[256];
	}
	
	
	//Define paths where the data is stored
	char fileName[512];
	
	
	char dotSlash[] = "./";
	char *dataPath = getenv("TRN_DATAFILES");
	if (!dataPath) dataPath = dotSlash;//"./";
	char *mapPath  = getenv("TRN_MAPFILES");
	if (!mapPath) mapPath = dotSlash;//"./";

	//Define specific file information
	
	//TEST 1: Dvl data from 8/04/08 MAUV dive at Portuguese Ledge
	strcpy(dataK_files[num], "Dive_2008_0804auv/dataFromDive/dataKft_test04all_080408dive.txt");
	strcpy(dataMeas_files[num], "Dive_2008_0804auv/dataFromDive/measData_test04all_080408dive.txt");
	if(map_type == 1) {
		strcpy(map_files[num], "PortugueseLedge/PortugueseLedge20080424TopoUTM_NoNan.grd");
		//strcpy(map_files[num], "PortugueseLedge/PortugueseLedge20080424TopoUTM.grd");
	} else if(map_type == 2) {
		//strcpy(map_files[num], "PortugueseLedge/PortugueseLedgeOctree_SomewhatFilled_1m.bin");
		strcpy(map_files[num], "PortugueseLedge/PortugueseLedgeRemade6-14-2016.bo");
		//strcpy(map_files[num], "PortugueseLedge/PL_2m_filled_compression.bin");
	} else if(map_type == 3){
        //TODO: no guarantee this is a good map -David
		strcpy(map_files[num], "PortugueseLedge/PortugueseLedgeOctree_PlanarFit_2m_filledOne_compression_fullCoverage.pfo");
	} 	else {
		printf("Invalid map_type.  Exiting.\n");
		return -1;
	}
	
	init_dataK[num] = 100;
	dataK_numRows[num] = 5761;//9285;//
	dataMeas_numRows[num] = 2356;//3721;//
	double currResults1[] = {9.2, -15, 0.5}; //{12,-11,0.3};
	mmseResults[num] = currResults1;
	num++;
	
	
	//TEST 2: Dvl data from 5/17/11 BIAUV/MAUV dive at Soquel Canyon
	strcpy(dataK_files[num], "Dive_2011_0411auv/dataFromDive/dataKft_test02all_051711dive.txt");
	strcpy(dataMeas_files[num], "Dive_2011_0411auv/dataFromDive/measData_test02all_051711dive.txt");
	double currResults2[] = {13.8, -6.2, -2.7};
	if(map_type == 1) {
		strcpy(map_files[num], "SoquelCanyon/SoquelCanyonMAUVUTMTopo_061709cut.grd");
		//strcpy(map_files[num],"SoquelCanyon/SoquelCanyonMAUVUTMTopo.grd");
	} else if(map_type == 2) {
		strcpy(map_files[num], "SoquelCanyon/SoquelCanyonOctree_2m.bin");
		//strcpy(map_files[num], "SoquelCanyon/SoquelCanyonOctree_PlanarFit_2m_filledOne_compression_fullCoverage.bin");
		//strcpy(map_files[num], "SoquelCanyon/SC_2m_filled_compression.bin");
	} else if(map_type == 3){
        //TODO: no guarantee this is a good map -David
		strcpy(map_files[num], "SoquelCanyon/SoquelCanyonOctree_PlanarFit_2m_filledOne_compression_fullCoverage.pfo");
	} else {
		printf("Invalid map_type.  Exiting.\n");
		return -1;
	}
	
	init_dataK[num] = 100;
	dataK_numRows[num] = 9000;//10464;
	dataMeas_numRows[num] = 8500;//9015;
	mmseResults[num] = currResults2;
	num++;

	//TEST 3: Imagenex data from 11/4/13 MAUV dive at Portuguese Ledge 
	//**Note: this is just temporary until we have a better Imagenex data set. (i.e. with Kearfott)
	strcpy(dataK_files[num], "Dive_2014_0620auv/dataFromDive/dataKft_test09all_dive20140620.txt");
	strcpy(dataMeas_files[num], "Dive_2014_0620auv/dataFromDive/downIdtData_test09all_dive20140620.txt");
	if(map_type == 1) {
		strcpy(map_files[num], "PortugueseLedge/PortugueseLedge20080424TopoUTM_NoNan.grd");
		//strcpy(map_files[num], "PortugueseLedge/PortugueseLedge20080424TopoUTM.grd");
	} else if(map_type == 2) {
		strcpy(map_files[num], "PortugueseLedge/PortugueseLedgeOctree_SomewhatFilled_1m.bin");
		//strcpy(map_files[num], "PortugueseLedge/PL_2m_filled_compression.bin");
	} else if(map_type == 3){
        //TODO: no guarantee this is a good map -David
        strcpy(map_files[num], "PortugueseLedge/PortugueseLedgeOctree_PlanarFit_2m_filledOne_compression_fullCoverage.pfo");
	} else {
		printf("Invalid map_type.  Exiting.\n");
		return -1;
	}

	init_dataK[num] = 1;
	dataK_numRows[num] = 6200;//8000;//3500;//7700;
	dataMeas_numRows[num] = 4617;//2138;//5418;//2138;//2340;
	double currResults3[] = {9.2, -12.0, 0.5}; //{0, 0, 0}; //Again, temporary until we have a better data set
	mmseResults[num] = currResults3;
	num++;
	
	//initialize data structures
	poseT* tercomEst = new poseT;
	poseT* mmseEst = new poseT;
	Matrix dataKft, dataMeas, measType;
	
	
	

	
	for(i = 0; i < numTests; i++) {
		measType = readDataFromFile(charCat(fileName, dataPath, dataMeas_files[i]),1,1);
		if(int(measType(1,1))==5){
			dataMeas = readDataFromFile(charCat(fileName, dataPath, dataMeas_files[i]),
									dataMeas_numRows[i], 244);
		}
		else{
			dataMeas = readDataFromFile(charCat(fileName, dataPath, dataMeas_files[i]),
									dataMeas_numRows[i], 62);
		}
		dataKft = readDataFromFile(charCat(fileName, dataPath, dataK_files[i]),
								   dataK_numRows[i], 22);
								   
		for(j = 0; j < numRepeat; j++) {

			runTerrainNav(dataKft, dataMeas,
						  map_type, map_files[i], 0, 15, 15,
						  tercomEst, mmseEst, realTime, 3, init_dataK[i]);
						  
		
	
			//check results against expected results
			idx = closestPtUniformArray(mmseEst->time, dataKft(1, 1), dataKft(dataK_numRows[i], 1), dataK_numRows[i]);
			
			if(idx <= 0){
				printf("\n\nidx is %i: (Matrix) dataKft cannot be referenced with that index\n",idx);
				printf("No accuracy test performed\n");
			}else{
				if(PrintVerboseResults){
					finalErrorX[i * numRepeat + j] = fabs(mmseEst->x - dataKft(idx, 7) - mmseResults[i][0]);
					finalEstimatorUncertaintyX[i * numRepeat + j] = sqrt(mmseEst->covariance[0]);
					finalErrorY[i * numRepeat + j] = fabs(mmseEst->y - dataKft(idx, 8) - mmseResults[i][1]);
					finalEstimatorUncertaintyY[i * numRepeat + j] = sqrt(mmseEst->covariance[2]);
				}
				if((fabs(mmseEst->x - dataKft(idx, 7) - mmseResults[i][0])
						<= 1.5 * sqrt(mmseEst->covariance[0])) &&
						(fabs(mmseEst->y - dataKft(idx, 8) - mmseResults[i][1])
						 <= 1.5 * sqrt(mmseEst->covariance[2]))) {
					successful[i * numRepeat + j] = true;
				} else {
					successful[i * numRepeat + j] = false;
				}
			}
			printf("successful[%d, %d] = %d\n", i, j, successful[i * numRepeat + j]);
		}
	}
	
	if(PrintVerboseResults){
		for(i = 0; i < numTests; i++) {
			for(j = 0; j < numRepeat; j++) { 
				printf("Test #%i Trial #%i: \t",i+1,j+1);
				printf("Error: %1.2f, %1.2f \tUncertainty: %1.2f, %1.2f \tRatio: %1.2f, %1.2f\n",
					finalErrorX[i * numRepeat + j], finalErrorY[i * numRepeat + j], finalEstimatorUncertaintyX[i * numRepeat + j], 
					finalEstimatorUncertaintyY[i * numRepeat + j], 
					finalErrorX[i * numRepeat + j] / finalEstimatorUncertaintyX[i * numRepeat + j], 
					finalErrorY[i * numRepeat + j] / finalEstimatorUncertaintyY[i * numRepeat + j]);
				}
			}
	}
	//print results of each test
	int numPassed = 0;
	for(i = 0; i < numTests; i++) {
		for(j = 0; j < numRepeat; j++) { 
			if(successful[i * numRepeat + j]) {
				printf("Test #%i Trial#%i passed\n", i + 1, j + 1);
				numPassed++;
			} else {
				printf("Test #%i Trial#%i failed\n", i + 1, j + 1);
			}
		}
	}
	printf("%i of %i tests passed\n", numPassed, numTests*numRepeat);
	
	//delete allocated memory on the heap
	delete tercomEst;
	delete mmseEst;
	for(i = 0; i < numTests; i++) {
		delete dataK_files[i];
		delete dataMeas_files[i];
	}
	delete [] dataK_files;
	delete [] dataMeas_files;
	delete [] map_files;
	delete [] dataK_numRows;
	delete [] dataMeas_numRows;
	delete [] init_dataK;
	delete [] mmseResults;
	delete [] successful;
	
	delete [] finalErrorX;
	delete [] finalEstimatorUncertaintyX;
	delete [] finalErrorY;
	delete [] finalEstimatorUncertaintyY;
	
	return 0;
}


// run terrainNav algorithm for a particular set of data
void runTerrainNav(const Matrix& dataKft, const Matrix& dataMeas,
				   int map_type, char* mapFile,
				   int interpMethod, int kSubSample,
				   int mSubSample, poseT* tercomEst, poseT* mmseEst,
				   bool realTime, int filterType, int dataK_init) {
	//initialize measurement and pose structures
	poseT* currEstimate = new poseT;
	measT* currMeas = new measT;
	int dataType;
    // TODO: replace consts w/ macros (what drives the values?)
	currMeas->ranges = new double[120];
	currMeas->alongTrack = new double[120];
	currMeas->crossTrack = new double[120];
	currMeas->altitudes = new double[120];
	currMeas->measStatus = new bool[120];
	currMeas->alphas = new double[120];
	int N = dataKft.Nrows();
	int M = dataMeas.Nrows();
	int i_init = dataK_init;
	int j_init = 1;
    int i, j;
	char filename[512];
	struct timespec semTime;
	
	char dotSlash[] = "./";
	char *dataPath = getenv("TRN_DATAFILES");
	if (!dataPath) dataPath = dotSlash;//"./";
	char *mapPath  = getenv("TRN_MAPFILES");
	if (!mapPath) mapPath = dotSlash;//"./";
	
	//initialize terrainNav object and load map
	TerrainNav* tercom;
    char spec_cfg[]="mappingAUV_specs.cfg";

	if(trnHost) {
		tercom = new TerrainNavClient(trnHost, trnPort,
                                      mapFile, spec_cfg, NULL, NULL, filterType, map_type);
	} else {
	
		snprintf(filename, 512, "%s%s", mapPath, mapFile);
		printf("Loading map file %s\n", filename);
		tercom = new TerrainNav(filename, spec_cfg, filterType, map_type);
	}

	// Changed to be compatible with TRN Client
	tercom->setMapInterpMethod(interpMethod);
	tercom->setInterpMeasAttitude(true);
	
	tercom->setModifiedWeighting(USE_MODIFIED_WEIGHTING);
	tercom->setFilterReinit(ALLOW_FILTER_REINIT);
	printf("Terrain navigation object initialized.\n");
	
	//Run terrainNav over all measurements and odometry
	printf("Initial Conditions: North: %.2f, East %.2f\n", dataKft(2, 7),
		   dataKft(2, 8));
	printf("data loaded...\n");
	
	//Get the current time:
	clock_gettime(CLOCK_REALTIME, &semTime);
	struct timespec startTime, lastTime, now;
	double localTime, elapsed;
	startTime.tv_sec  = semTime.tv_sec;
	startTime.tv_nsec = semTime.tv_nsec;
	lastTime.tv_sec  = semTime.tv_sec;
	lastTime.tv_nsec = semTime.tv_nsec;
	
	// Sampling interval:
	short TsMsec = 500;
	i = i_init;
	j = j_init;
	while(i <= N) {
		// Compute the next wake-up time:
		clock_gettime(CLOCK_REALTIME, &semTime);
		
		lastTime.tv_sec  = semTime.tv_sec;
		lastTime.tv_nsec = semTime.tv_nsec;
		
		localTime = semTime.tv_sec  - startTime.tv_sec +
					(semTime.tv_nsec - startTime.tv_nsec) / 1.e9;
		if(realTime) {
			printf("Time since start = %.2f sec\n", localTime);
		}
		
		// NOTE: Need to subtract a millisecond so that sem_timedwait() times out
		// at the expected interval.
		long deltaMsec = TsMsec - 1;
		
		semTime.tv_sec += (deltaMsec / 1000);
		semTime.tv_nsec += ((deltaMsec % 1000) * 1000000);
		
		// Check for nanosecond rollover
		if(semTime.tv_nsec > NANOSEC_PER_SEC) {
			semTime.tv_sec += (int)(semTime.tv_nsec / NANOSEC_PER_SEC);
			semTime.tv_nsec = semTime.tv_nsec % NANOSEC_PER_SEC;
		}
		
		// Perform motion/measurement updates in time order
		if(j > M || (i <= N && dataKft(i, 1) <= dataMeas(j, 2))) {
			printf("Motion Update.. (t = %.2f)\n", dataKft(i, 1));
			
			assignKearfottEstimate(currEstimate, dataKft.Row(i));
			
			
			tercom->motionUpdate(currEstimate);
			i = i + kSubSample;
			
			//ensure that we always perform the final motion update
			if(i > N && (i - kSubSample < N)) {
				i = N;
			}
			
		} else {
			if(j <= M) {
				dataType = int(dataMeas(j, 1));
				
				//read in current measurement
				switch(dataType) {
					case 1:
						assignDVLMeasurement(currMeas,
											 dataMeas.SubMatrix(j, j, 2, dataMeas.Ncols()));
						break;
						
					case 2:
						assignMBMeasurement(currMeas,
											dataMeas.SubMatrix(j, j, 2, dataMeas.Ncols()));
						currMeas->psi = currEstimate->psi;
						currMeas->x = currEstimate->x;
						currMeas->y = currEstimate->y;
						currMeas->z = currEstimate->z;
						
						break;
						
					case 3:
						assignAltMeasurement(currMeas,
											 dataMeas.SubMatrix(j, j, 2, dataMeas.Ncols()));
						break;
					
					case 5:
						assignIdtMeasurement(currMeas,
												dataMeas.SubMatrix(j,j,2,dataMeas.Ncols()));
						break;
						
					default:
						printf("No valid datatype specified.  Exiting...\n");
						return;
				}
				
				printf("Measurement Update...\n");
				tercom->measUpdate(currMeas, dataType);
				j = j + mSubSample;
				
				//If measurement update happens first or measurement update
				//unsuccessful, skip pose estimation and file saving
				if(i > 1 && tercom->lastMeasSuccessful()) {
					//compute tercom MLE pose estimate
					tercom->estimatePose(tercomEst, 1);
					
					//compute tercom MMSE pose estimate
					tercom->estimatePose(mmseEst, 2);
					
					//display tercom estimate biases
					printf("Estimation Bias (Max. Likelihood): (t = %.2f)\n",
						   tercomEst->time);
					printf("North: %.4f, East: %.4f, Depth: %.4f\n",
						   tercomEst->x - currEstimate->x, tercomEst->y
						   - currEstimate->y, tercomEst->z - currEstimate->z);
					printf("Estimation Bias (Mean): (t = %.2f)\n", mmseEst->time);
					printf("North: %.4f, East: %.4f, Depth: %.4f\n",
						   mmseEst->x - currEstimate->x,
						   mmseEst->y - currEstimate->y,
						   mmseEst->z - currEstimate->z);
					if(filterType == 2)
						printf("Psi Bias & Sigma: %.2f +/- %.3f\n",
							   (mmseEst->psi - currEstimate->psi) * 180.0 / PI,
							   sqrt(mmseEst->covariance[20]) * 180.0 / PI);
							   
					printf("North Sigma: %.2f, East Sigma: %.2f, Depth Sigma: %.2f\n\n",
						   sqrt(mmseEst->covariance[0]),
						   sqrt(mmseEst->covariance[2]),
						   sqrt(mmseEst->covariance[5]));
						   
				}
			}
		}
		
		//get current clock time
		clock_gettime(CLOCK_REALTIME, &now);
		elapsed = now.tv_sec  - lastTime.tv_sec +
				  (now.tv_nsec - lastTime.tv_nsec) / 1.e9;
		if(realTime) {
			printf("Computation time = %.2f msec\n", elapsed * 1000);
		}
	}
	
	elapsed = now.tv_sec  - startTime.tv_sec +
			  (now.tv_nsec - startTime.tv_nsec) / 1.e9;
	printf("Total Elapsed Time: = %.2f sec\n", elapsed);
	
	
	//delete allocated memory
	delete tercom;
	delete currMeas;
	delete currEstimate;
}


//Load data from a given file into a matrix structure
Matrix readDataFromFile(const char* fileName, const int& numRows,
						const int& numCols) {
	Matrix data(numRows, numCols);
	
	ifstream datafile;
	printf("Loading %s...\n", fileName);
	
	datafile.open(fileName);
	
	if(datafile.is_open()) {
		int row = 1;
		//char *num;
		char num[128];
		num[127] = '\0';
		while(!datafile.eof() && row <= numRows) {	//check that not at end of file
			for(int col = 1; col <= numCols; col++) {
				datafile >> setprecision(15) >> num;
				
				data(row, col) = atof(num);
				//printf("%.2f  ", data(row,col));
			}
			datafile.ignore(256, '\n');
			row++;
			//printf("\n");
		}
		
		datafile.close();
		
		return data;
	} else {
		printf("Unable to open file %s.  Exiting...\n", fileName);
		exit(0);
	}
	return data;
}

// assign a kearfott pose matrix into a poseT structure.
bool assignKearfottEstimate(poseT* currEstimate, Matrix kftPose) {
	currEstimate->time = kftPose(1, 1);
	currEstimate->dvlValid = kftPose(1, 2);
	currEstimate->gpsValid = kftPose(1, 3);
	currEstimate->bottomLock = kftPose(1, 4);
	currEstimate->x = kftPose(1, 7);	//North
	currEstimate->y = kftPose(1, 8);	//East
	currEstimate->z = kftPose(1, 9); //Depth
	currEstimate->phi = kftPose(1, 10); //-0.74*PI/180;
	currEstimate->theta = kftPose(1, 11); //-1.13*PI/180;
	currEstimate->psi = kftPose(1, 12);
	currEstimate->vx = kftPose(1, 13);
	currEstimate->vy = kftPose(1, 14);
	currEstimate->vz = kftPose(1, 15);
	currEstimate->ax = kftPose(1, 16);
	currEstimate->ay = kftPose(1, 17);
	currEstimate->az = kftPose(1, 18);
	currEstimate->wx = kftPose(1, 19);
	currEstimate->wy = kftPose(1, 20);
	currEstimate->wz = kftPose(1, 21);
	
	return true;
}

//assign dvl measurement into a measT structure
void assignDVLMeasurement(measT* currMeas, Matrix meas) {
	currMeas->dataType = 1;
	currMeas->time = meas(1, 1);
	currMeas->ranges[0] = meas(1, 16);
	currMeas->ranges[1] = meas(1, 17);
	currMeas->ranges[2] = meas(1, 18);
	currMeas->ranges[3] = meas(1, 19);
	currMeas->numMeas = 4;
	currMeas->phi = meas(1, 14);
	currMeas->theta = meas(1, 13);
	currMeas->psi = meas(1, 15);
	currMeas->measStatus[0] = meas(1, 22);
	currMeas->measStatus[1] = meas(1, 23);
	currMeas->measStatus[2] = meas(1, 24);
	currMeas->measStatus[3] = meas(1, 25);
	currMeas->x = meas(1, 26);
	currMeas->y = meas(1, 27);
	currMeas->z = meas(1, 28);
	
}

//assign imagenex delta t (idt) measurement into meastT structure
void assignIdtMeasurement(measT* currMeas, Matrix meas)
{
 	int i;
	currMeas->dataType = 5;
	currMeas->time = meas(1,1);
	currMeas->numMeas = meas(1,3);
	for(i = 0; i<meas(1,3)-1; i++)
	{
		currMeas->ranges[i] = meas(1,i+4);
		currMeas->measStatus[i] = 1;
 	}

}

//assign multibeam measurement into a measT structure
void assignMBMeasurement(measT* currMeas, Matrix meas) {

	currMeas->dataType = 2;
	if(AVERAGE) {
		currMeas->numMeas = 2;
		int j = 0;
		for(int i = 10; i < 12; i++) {
			currMeas->alongTrack[j] = meas(1, (i - 1) * 3 + 2);
			currMeas->crossTrack[j] = meas(1, (i - 1) * 3 + 3);
			currMeas->altitudes[j] = meas(1, (i - 1) * 3 + 4);
			j++;
		}
	} else {
		currMeas->numMeas = (meas.Ncols() - 1) / 3;
		int j = 0;
		for(int i = 2; i < meas.Ncols(); i = i + 3) {
			currMeas->alongTrack[j] = meas(1, i);
			currMeas->crossTrack[j] = meas(1, i + 1);
			currMeas->altitudes[j] = meas(1, i + 2);
			j++;
		}
	}
	
	for(int i  = 0; i < currMeas->numMeas; i++) {
		currMeas->measStatus[i] = true;
	}
	
	//measurement frame accounts for phi and theta, but not for psi
	currMeas->phi = 0;
	currMeas->theta = 0;
	currMeas->psi = 0;
	currMeas->time = meas(1, 1);
	
}

//assign altimeter measurement into a measT structure
void assignAltMeasurement(measT* currMeas, Matrix meas) {
	int i;
	currMeas->dataType = 3;
	currMeas->time = meas(1, 1);
	currMeas->ranges[0] = meas(1, 3);
	
	for(i = 1; i < 4; i++) {
		currMeas->ranges[i] = 0.0;
	}
	currMeas->numMeas = 1;
	currMeas->measStatus[0] = meas(1, 4);
	currMeas->theta = -meas(1, 2);
}


