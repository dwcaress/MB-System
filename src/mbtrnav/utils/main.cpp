#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <semaphore.h>

#ifdef use_namespace
using namespace std;
#endif

#include "TerrainNav.h"
#include "TerrainNavClient.h"
#include "matrixArrayCalcs.h"
#include "genFilterDefs.h"
#include "structDefs.h"
#include "TNavParticleFilter.h"

//#define USE_OCTREE 0

#define NANOSEC_PER_SEC (1000000000L)

void runTerrainNav(const Matrix& dataKft, const Matrix& dataMeas,
				   int map_type, char* mapPath,
				   int interpMethod, int kSubSample,
				   int mSubSample, poseT* tercomEst, poseT* mmseEst,
				   bool realTime, int filterType, ofstream& distribFile,
				   char* savePath, int dataK_init, double& mapResolutionX, double& mapResolutionY);
Matrix readDataFromFile(const char* fileName, const int& numRows,
						const int& numCols);
bool assignKearfottEstimate(poseT* currEstimate, Matrix pose);
void assignDVLMeasurement(measT* currMeas, Matrix meas);
void assignMBMeasurement(measT* currMeas, Matrix meas);
void assignAltMeasurement(measT* currMeas, Matrix meas);
void assignIdtMeasurement(measT* currMeas, Matrix meas);
void saveRunParameters(const char* savePath);

static char* trnHost = NULL;
static int   trnPort = 27027;

static double mapResolutionX, mapResolutionY;//TODO this is a hack to get the map resolution from tnavfilter to the if for successful=true/false

int main(int argc, char** argv) {
	bool realTime = false;
	int i, j;
	int numRepeat = 1;
	int map_type = 1;  // 1=GRD, 2=OCTREE
	char *mapName = NULL, *kftName = NULL, *measName = NULL;

	ofstream trnfile, mmsefile, distribfile, pmfdistribfile;

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
		//port to the next input argument
		if(!strncmp(argv[i], "-p", 2)) {
			trnPort = atoi(argv[i + 1]);
		}
		
		//If the user passes an argument starting with -m, set the map type;
		//
		if(!strncmp(argv[i], "-t", 2)) {
			map_type = atoi(argv[i + 1]);
		}
	}
	

	char map_file[256];

	if (mapName) {
		strcpy(map_file, mapName);
	}
	else {
		if(map_type == 2) {
				strcpy(map_file, //sprintf(map_file,
					//"SoquelCanyon/SoquelCanyon3dPoints.txt"
					//"SoquelCanyon/SoquelCanyon3dPointsDEM.txt"
					//"SoquelCanyon/SoquelCanyonTopo3dPointsDEM_1m.txt"	//Larger SoquelCanyonTopo Map, Interpolated to 1m resolution
					//"NearshoreWall/NearshoreWall3dPointsNEDUTMclean.txt"
					//"PortugueseLedge/ReducedMap.txt"
					//"PortugueseLedge/PortugueseLedge3dPointsDEM.txt"
					//"PortugueseLedge/PortLedgeLongShort.txt"
					//"PortugueseLedge/PortugueseLedge3dPointsNEDUTM.txt"
					//"SoquelCanyon/SoquelCanyonOctree_2m.bin"
					"PortugueseLedge/PortugueseLedgeOctree_SomewhatFilled_1m.bin"
					//"PortugueseLedge/PortugueseLedgeOctree_1m.bin"
				   );
		} else if(map_type == 1) {
			strcpy(map_file, //sprintf(map_file,
					//"SoquelCanyon/SoquelCanyonMAUVUTMTopo.grd"
					//"SoquelCanyon/SoquelCanyon1998em300_20m.grd"
					//"SoquelCanyon/soquelCanyonGW_101309_1mRes.grd"
					//"SoquelCanyon/SoquelCanyonMAUVUTMTopo_061709cut.grd"
					//"SoquelCanyon/SoquelCanyonMAUVUTMTopo_061709cut_Shift30m.grd"  		//Soquel Map with 30m shift, 1m resolution
					//"SoquelCanyon/SoquelCanyonTopoUTM.grd"														//Larger Soquel Map, at 10m (?) resolution
					//"SoquelCanyon/SoquelCanyonTopoUTM_1m.grd"															//Larger Soquel Map, interpolated to 1m resolution
					"PortugueseLedge/PortugueseLedge20080424TopoUTM_NoNan.grd"
					//"PortugueseLedge/PortLedge1998em300_20m.grd"
					//"NearshoreWall/UppermostCanyonTopoUTM.grd"
					//"NearshoreWall/NearshoreWallUTM2.grd"
				   );
		} else if(map_type == 3){
			strcpy(map_file,
					"PortugueseLedge/PortugueseLedgeOctree_PlanarFit_2m_filledOne_compression_fullCoverage.pfo"//TODO: no guarintee this is a good map -David
					//"SoquelCanyon/SoquelCanyonOctree_PlanarFit_2m_filledOne_compression_fullCoverage.pfo"//TODO: no guarintee this is a good map -David
					);
		} else {
			printf("Invalid map_type. Exiting.\n");
			return -1;
		}
	}

	if (trnHost)
		printf("Using client/server arrangement with host %s on port %ld\n", trnHost, trnPort);
	
	if (realTime)
		printf("Running in real-time\n");

	printf("Running test %d times\n", numRepeat);	
	printf("Using map_type %d\n", map_type);
	
//initialize memory for different tests
	int numTests = 1;
	int num = 0;
	int* dataK_numRows = new int[numTests];
	int* dataMeas_numRows = new int[numTests];
	int* init_dataK = new int[numTests];
	char** dataK_files = new char*[numTests];
	char** dataMeas_files = new char*[numTests];
	char** savePath = new char*[numTests];
	
	for(i = 0; i < numTests; i++) {
		dataK_files[i] = new char[256];
		dataMeas_files[i] = new char[256];
		savePath[i] = new char[256];
	}
	
	//Define paths where the data is stored
	char fileName[512];
	char dotSlash[] = "./";
	char *dataPath = getenv("TRN_DATAFILES");
	if (!dataPath) dataPath = dotSlash;//"./";
	char *mapPath  = getenv("TRN_MAPFILES");
	if (!mapPath) mapPath = dotSlash;//"./";
	
	
	//Define specific file information
	
	strcpy(dataK_files[num], "Dive_2014_0620auv/dataFromDive/dataKft_test09all_dive20140620.txt");
	strcpy(dataMeas_files[num], "Dive_2014_0620auv/dataFromDive/measData_test09all_dive20140620.txt");
	//strcpy(dataMeas_files[num], "Dive_2014_0620auv/dataFromDive/downIdtData_test09all_dive20140620.txt");
	strcpy(savePath[num], "U:/ResultsDump/");
	init_dataK[num] = 1;
	dataK_numRows[num] = 7469;
	dataMeas_numRows[num] = 4617;//3972;
	
	
	/*
	strcpy(dataK_files[num], "Dive_2014_0515auv/dataFromDive/dataKft_test05all_dive20140515.txt");
	strcpy(dataMeas_files[num], "Dive_2014_0515auv/dataFromDive/measData_test05all_dive20140515.txt");
	//strcpy(dataMeas_files[num], "Dive_2014_0515auv/dataFromDive/downIdtData_test05all_dive20140515.txt");
	strcpy(savePath[num], "U:/ResultsDump/");
	init_dataK[num] = 1;
	dataK_numRows[num] = 7293;
	dataMeas_numRows[num] = 4079;
	*/
	
	/*
	//TEST 11/04/13: Portuguese Ledge
	strcpy(dataK_files[num], "Dive_2014_0620auv/dataFromDive/dataKft_test09all_dive20140620.txt");
	//strcpy(dataMeas_files[num], "Dive_2014_0620auv/dataFromDive/measData_test09all_rangeCorrected_dive20140620.txt");
	strcpy(dataMeas_files[num], "Dive_2014_0620auv/dataFromDive/downIdtData_test09all_dive20140620.txt");
	strcpy(savePath[num], "/home/shouts/Documents/ResultsDump/");
	init_dataK[num] = 1;
	dataK_numRows[num] = 7469;
	dataMeas_numRows[num] = 3972;//4617;//
	*/
	
//	//TEST 11/04/13: Portuguese Ledge
//	strcpy(dataK_files[num], "Dive_2013_1104auv/dataFromDive/dataKft_test07all_dive20131104.txt");
//	//strcpy(dataMeas_files[num], "Dive_2013_1104auv/dataFromDive/measData_test07all_rangeCorrected_dive20131104.txt");
//	strcpy(dataMeas_files[num], "Dive_2013_1104auv/dataFromDive/downIdtData_test07all_dive20131104.txt");
//	strcpy(savePath[num], "/home/shouts/Documents/ResultsDump/");
//	init_dataK[num] = 1;
//	dataK_numRows[num] = 7445;
//	dataMeas_numRows[num] = 2138;//1669;
	
//TEST 0: Dvl data from 2/21/13 MAUV dive at Nearshore Wall
	/*strcpy(dataK_files[num],"Dive_2013_0221auv/dataFromDive/dataKft_test00all_022113dive.txt");
	strcpy(dataMeas_files[num],"Dive_2013_0221auv/dataFromDive/measData_test00all_022113dive.txt");
	strcpy(savePath[num],"/home/shouts/Documents/ResultsDump/");
	init_dataK[num] = 20;
	dataK_numRows[num] = 31786;//5761;
	dataMeas_numRows[num] = 22054;//2356;
	num++;*/
	
//	//TEST 0.5: Dvl data from 12/5/12 MAUV dive over cliff near Soquel Canyon
//  strcpy(dataK_files[num],"Dive_2012_1205auv/dataFromDive/dataKft_test09all_120512dive.txt");
//  strcpy(dataMeas_files[num],"Dive_2012_1205auv/dataFromDive/measData_test09all_120512dive.txt");
//  strcpy(savePath[num],"/home/shouts/Documents/ResultsDump/");
//  init_dataK[num] = 20;
//  dataK_numRows[num] = 17988;
//  dataMeas_numRows[num] = 9190;
//  num++;


	//TEST 1: Dvl data from 8/04/08 MAUV dive at Portuguese Ledge
	/*strcpy(dataK_files[num],"Dive_2008_0804auv/dataFromDive/dataKft_test04all_080804dive.txt");
	strcpy(dataMeas_files[num],"Dive_2008_0804auv/dataFromDive/measData_test04all_080804dive.txt");
	strcpy(savePath[num],"/home/shouts/Documents/ResultsDump/");
	init_dataK[num] = 2500;
	dataK_numRows[num] = 9285;//5761;
	dataMeas_numRows[num] = 3721;//2356;
	num++;*/
	
	//TEST DIVE 07/08/13
	/*strcpy(dataK_files[num],"Dive_2013_0708auv/dataFromDive/dataKft_test11all_dive070813.txt");
	strcpy(dataMeas_files[num],"Dive_2013_0708auv/dataFromDive/measData_test11all_rangeCorrected_dive070813.txt");
	strcpy(savePath[num],"/home/shouts/Documents/ResultsDump/");
	init_dataK[num] = 1;
	dataK_numRows[num] = 9900;
	dataMeas_numRows[num] = 8300;*/
	
	//TEST 2: Dvl data from 11/18/10 BIAUV/MAUV dive at Soquel Canyon
//	strcpy(dataK_files[num], "Dive_2010_1118auv/dataFromDive/dataKft_test03all_111810dive.txt");
//	strcpy(dataMeas_files[num], "Dive_2010_1118auv/dataFromDive/measData_test03all_rangeCorrected_111810dive.txt");
//	strcpy(savePath[num], "/home/shouts/Documents/ResultsDump/");
//	init_dataK[num] = 1;
//	dataK_numRows[num] = 9776;
//	dataMeas_numRows[num] = 8697;
	
//  //TEST 2A: Dvl data from 10/30/13 MAUV dive at Soquel Canyon
//  strcpy(dataK_files[num],"Dive_2012_1030auv/dataFromDive/dataKft_test20all_103012dive.txt");
//  strcpy(dataMeas_files[num],"Dive_2012_1030auv/dataFromDive/measData_test20all_103012dive.txt");
//  strcpy(savePath[num],"/home/shouts/Documents/ResultsDump/");
//  init_dataK[num] = 1;
//  dataK_numRows[num] = 30752;
//  dataMeas_numRows[num] = 15774;



	//TEST 08 - 07/19/10
	/*
	dataK_numRows[num] = 670;
	dataMeas_numRows[num] = 670;
	init_dataK[num] = 1;
	strcpy(dataK_files[num],"2010_07_19diveFiles/dataKft_test08_071910dive.txt");
	strcpy(dataMeas_files[num],"2010_07_19diveFiles/measData_test08_071910dive.txt");
	strcpy(savePath[num],"/home/debbie/Research_Files/mbariCode/dataFiles/2010_07_19diveFiles/results/test08/");
	num++; */
	
	//initialize data structures
	poseT* tercomEst = new poseT;
	poseT* mmseEst = new poseT;
	Matrix dataKft, dataMeas, measType;
	
	for(i = 0; i < numTests; i++) {
		trnfile.open(charCat(fileName, savePath[i], "allMle.txt"));
		mmsefile.open(charCat(fileName, savePath[i], "allMmse.txt"));
		distribfile.open(charCat(fileName, savePath[i], "allFinalHist.txt"));
		pmfdistribfile.open(charCat(fileName, savePath[i], "allFinalPDF.txt"));
		
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
			double mapResolutionX, mapResolutionY;
			
			runTerrainNav(dataKft, dataMeas,
							map_type, map_file, 1, 15, 15,
						  tercomEst, mmseEst, realTime, 2, distribfile,
						  savePath[i], init_dataK[i], mapResolutionX, mapResolutionY);
						  
			//interp method: 0-nearest neighbor, 1-bilinear
			trnfile << setprecision(15) << tercomEst->x << "\t" << tercomEst->y
					<< "\t" << tercomEst->z << "\t" << tercomEst->phi
					<< "\t" << tercomEst->theta << "\t" << tercomEst->psi
					<< "\t" << tercomEst->time << endl;
			mmsefile << setprecision(15) << mmseEst->x << " " << mmseEst->y
					 << " " << mmseEst->z << "\t" << mmseEst->phi << "\t"
					 << mmseEst->theta << "\t" << mmseEst->psi << "\t"
					 << mmseEst->covariance[0] << "\t" << mmseEst->covariance[2]
					 << "\t" << mmseEst->covariance[1] << "\t"
					 << mmseEst->covariance[5] << "\t" << mmseEst->covariance[9]
					 << "\t" << mmseEst->covariance[14] << "\t"
					 << mmseEst->covariance[20] << endl;
		}
		
		saveRunParameters(savePath[i]);
		
		trnfile.close();
		mmsefile.close();
		distribfile.close();
		pmfdistribfile.close();
	}
	
	//delete allocated memory on the heap
	delete tercomEst;
	delete mmseEst;
	for(i = 0; i < numTests; i++) {
		delete dataK_files[i];
		delete dataMeas_files[i];
		delete savePath[i];
	}
	delete [] dataMeas_files;
	delete [] dataK_files;
	delete [] savePath;
	delete [] dataK_numRows;
	delete [] dataMeas_numRows;
	delete [] init_dataK;
	
	return 0;
}


// run terrainNav algorithm for a particular set of data
void runTerrainNav(const Matrix& dataKft, const Matrix& dataMeas,
				   int map_type, char* mapFile, 
				   int interpMethod, int kSubSample,
				   int mSubSample, poseT* tercomEst, poseT* mmseEst,
				   bool realTime, int filterType, ofstream& distribFile,
				   char* savePath, int dataK_init, double& mapResolutionX, double& mapResolutionY) {
	//initialize measurement and pose structures
	poseT* currEstimate = new poseT;
	measT* currMeas = new measT;
	int dataType;
//	currMeas->ranges = new double[4];
//	currMeas->alongTrack = new double[20];
//	currMeas->crossTrack = new double[20];
//	currMeas->altitudes = new double[20];
//	currMeas->measStatus = new bool[20];
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
	sem_t semaphore;
	struct timespec semTime;
	
	sem_init(&semaphore, 0, 0);
	
	char filename[512];
		
	char dotSlash[] = "./";
	char *dataPath = getenv("TRN_DATAFILES");
	if (!dataPath) dataPath = dotSlash;//"./";
	char *mapPath  = getenv("TRN_MAPFILES");
	if (!mapPath) mapPath = dotSlash;//"./";
	
	//set up files for saving data
	bool saveResults = true;//false; //
	
	char fileName[200];
	ofstream tfile, mfile, pfile, ffile;
	if(saveResults) {
		if(filterType == 2) {
			tfile.open(charCat(fileName, savePath, "tercomEst_pf.txt")); 
			mfile.open(charCat(fileName, savePath, "mmseEst_pf.txt")); 
			pfile.open(charCat(fileName, savePath, "propPoses_pf.txt")); 
			ffile.open(charCat(fileName, savePath, "allParticles.txt"));
		} else {
			tfile.open(charCat(fileName, savePath, "tercomEst.txt"));
			mfile.open(charCat(fileName, savePath, "mmseEst.txt"));
			pfile.open(charCat(fileName, savePath, "propPoses.txt"));
			ffile.open(charCat(fileName, savePath, "likeSurfs.txt"));
		}
	}
	

	//initialize terrainNav object and load map
	TerrainNav* tercom;
	if(trnHost) {
		tercom = new TerrainNavClient(trnHost, trnPort,
									  mapFile, (char*)"mappingAUV_specs.cfg", filterType, map_type);
	} else {
	
		sprintf(filename, "%s%s", mapPath, mapFile);
		printf("Loading map file %s\n", filename);
		tercom = new TerrainNav(filename, (char*)"mappingAUV_specs.cfg", filterType, map_type,savePath);
	}				
						
	//set random number generator for test cases
	//srand(2000);
//	tercom->tNavFilter->setMapInterpMethod(interpMethod);
//	tercom->tNavFilter->setInterpMeasAttitude(true);
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
			//assignWatVelocity(currEstimate, watVel.Row(i));
			
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
					case TRN_SENSOR_DVL:
						assignDVLMeasurement(currMeas,
											 dataMeas.SubMatrix(j, j, 2, dataMeas.Ncols()));
						break;
						
					case TRN_SENSOR_MB:
						assignMBMeasurement(currMeas,
											dataMeas.SubMatrix(j, j, 2, dataMeas.Ncols()));
						currMeas->psi = currEstimate->psi;
						currMeas->x = currEstimate->x;
						currMeas->y = currEstimate->y;
						currMeas->z = currEstimate->z;
						
						break;
						
					case TRN_SENSOR_PENCIL:
						assignAltMeasurement(currMeas,
											 dataMeas.SubMatrix(j, j, 2, dataMeas.Ncols()));
						break;
					
					case TRN_SENSOR_DELTAT:
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
						   
					if(saveResults) {
						tfile << setprecision(15) << tercomEst->x << " "
							  << tercomEst->y << " " << tercomEst->z << " "
							  << tercomEst->phi << " " << tercomEst->theta
							  << " " << tercomEst->psi << " " << tercomEst->time
							  << endl;
							  
						pfile << setprecision(15) << currEstimate->x << " "
							  << currEstimate->y << " " << currEstimate->z << " "
							  << currEstimate->phi << " " << currEstimate->theta
							  << " " << currEstimate->psi << " "
							  << currEstimate->time << endl;
						mfile << setprecision(15) << mmseEst->x << " "
							  << mmseEst->y << " " << mmseEst->z << " "
							  << mmseEst->phi << " " << mmseEst->theta << " "
							  << mmseEst->psi;
						if(SEARCH_GYRO_BIAS) {
							mfile << " " << mmseEst->wy << " " << mmseEst->wz;
						}
						mfile << " " << mmseEst->covariance[0]
							  << " " << mmseEst->covariance[2]
							  << " " << mmseEst->covariance[1]
							  << " " << mmseEst->covariance[5]
							  << " " << mmseEst->covariance[9]
							  << " " << mmseEst->covariance[14]
							  << " " << mmseEst->covariance[20];
						if(SEARCH_GYRO_BIAS) {
							mfile << "\t" << mmseEst->covariance[27];
							mfile << "\t" << mmseEst->covariance[35];
						}
						mfile << endl;
						
						if(filterType == 2 && SAVE_PARTICLES) {
							tercom->tNavFilter->saveCurrDistrib(ffile);
						}
					}
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
	
	//save filter and estimates if created
	if(i > i_init && saveResults) {
		//compute tercom MLE pose estimate
		tercom->estimatePose(tercomEst, 1);
		
		//compute tercom MMSE pose estimate
		tercom->estimatePose(mmseEst, 2);
		
		//compute and save tercom MLE pose estimate
		tfile << setprecision(15) << tercomEst->x << " "
			  << tercomEst->y << " " << tercomEst->z << " "
			  << tercomEst->phi << " " << tercomEst->theta
			  << " " << tercomEst->psi << " " << tercomEst->time
			  << endl;
			  
		pfile << setprecision(15) << currEstimate->x << " "
			  << currEstimate->y << " " << currEstimate->z << " "
			  << currEstimate->phi << " " << currEstimate->theta
			  << " " << currEstimate->psi << " "
			  << currEstimate->time << endl;
		mfile << setprecision(15) << mmseEst->x << " "
			  << mmseEst->y << " " << mmseEst->z << " "
			  << mmseEst->phi << " " << mmseEst->theta << " "
			  << mmseEst->psi;
		if(SEARCH_GYRO_BIAS) {
			mfile << " " << mmseEst->wy << " " << mmseEst->wz;
		}
		mfile << " " << mmseEst->covariance[0]
			  << " " << mmseEst->covariance[2]
			  << " " << mmseEst->covariance[1]
			  << " " << mmseEst->covariance[5]
			  << " " << mmseEst->covariance[9]
			  << " " << mmseEst->covariance[14]
			  << " " << mmseEst->covariance[20];
		if(SEARCH_GYRO_BIAS) {
			mfile << "\t" << mmseEst->covariance[27];
			mfile << "\t" << mmseEst->covariance[35];
		}
		mfile << endl;
		
		if(filterType == 2 && SAVE_PARTICLES) {
			tercom->tNavFilter->saveCurrDistrib(ffile);
		}
		
		//save final filter distribution
		tercom->tNavFilter->saveCurrDistrib(distribFile);
	}
	
	elapsed = now.tv_sec  - startTime.tv_sec +
			  (now.tv_nsec - startTime.tv_nsec) / 1.e9;
	printf("Total Elapsed Time: = %.2f sec\n", elapsed);
	
	pfile.close();
	tfile.close();
	mfile.close();
	ffile.close();
	
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
	currMeas->dataType = TRN_SENSOR_DVL;
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

//assign multibeam measurement into a measT structure
void assignMBMeasurement(measT* currMeas, Matrix meas) {

	currMeas->dataType = TRN_SENSOR_MB;
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
	currMeas->dataType = TRN_SENSOR_PENCIL;
	currMeas->time = meas(1, 1);
	currMeas->ranges[0] = meas(1, 3);
	
	for(i = 1; i < 4; i++) {
		currMeas->ranges[i] = 0.0;
	}
	currMeas->numMeas = 1;
	currMeas->measStatus[0] = meas(1, 4);
	currMeas->theta = -meas(1, 2);
}

//assign imagenex delta t (idt) measurement into meastT structure
void assignIdtMeasurement(measT* currMeas, Matrix meas)
{
 	int i;
	currMeas->dataType = TRN_SENSOR_DELTAT;
	currMeas->time = meas(1,1);
	currMeas->numMeas = meas(1,3);
	for(i = 0; i<meas(1,3)-1; i++)
	{
		currMeas->ranges[i] = meas(1,i+4);
		currMeas->measStatus[i] = 1;
 	}

}


//save simulation parameters
void saveRunParameters(const char* savePath) {
	char fileName[256];
	FILE* sfile;
	sfile = fopen(charCat(fileName, savePath, "filterParams.txt"), "w");
	
	//if unable to open parameter file, exit function.
	if(sfile == NULL) {
		return;
	}
	
	//Save TNavFilter settings
	fprintf(sfile, "\n\nTNavFilter Settings:\n");
	fprintf(sfile, "--------------------\n");
	fprintf(sfile, "VEL_PER_ERROR: \t\t %.2f \n", VEL_PER_ERROR);
	fprintf(sfile, "WATER_VEL_PER_ERROR: \t %.2f \n", WATER_VEL_PER_ERROR);
	fprintf(sfile, "VEL_STDDEV: \t\t %.2f \n", VEL_STDDEV);
	fprintf(sfile, "USE_MAP_NAN: \t\t %i \n", USE_MAP_NAN);
	fprintf(sfile, "USE_COMPASS_BIAS: \t %i \n", USE_COMPASS_BIAS);
	fprintf(sfile, "HOMER_RANGE_PER_ERROR: \t %.2f \n", HOMER_RANGE_PER_ERROR);
	fprintf(sfile, "USE_RANGE_CORR: \t %i \n", USE_RANGE_CORR);
	fprintf(sfile, "USE_ACCEL: \t\t %i \n", USE_ACCEL);
	
	//Save TerrainNav settings
	fprintf(sfile, "\n\nTerrainNav Settings:\n");
	fprintf(sfile, "--------------------\n");
	fprintf(sfile, "MEAS_BUFFER_SIZE: \t %i \n", MEAS_BUFFER_SIZE);
	fprintf(sfile, "MAX_INTERP_TIME: \t %.2f \n", MAX_INTERP_TIME);
	fprintf(sfile, "MAX_RANGE: \t\t %.2f \n", MAX_RANGE);
	fprintf(sfile, "MIN_RANGE: \t\t %.2f \n", MIN_RANGE);
	fprintf(sfile, "MAX_VEL: \t\t %.2f \n", MAX_VEL);
	fprintf(sfile, "MAX_ACCEL: \t\t %.2f \n", MAX_ACCEL);
	fprintf(sfile, "MAX_DRDT: \t\t %.2f \n", MAX_DRDT);
	
	//Save TerrainMap settings
	fprintf(sfile, "\n\nTerrainMap Settings:\n");
	fprintf(sfile, "--------------------\n");
	fprintf(sfile, "VARIOGRAM FRACTAL DIM: \t\n");
	fprintf(sfile, "VARIOGRAM ALPHA: \t\n");
	
	//Save ParticleFilterDefs
	fprintf(sfile, "\n\nParticle Filter Defs:\n");
	fprintf(sfile, "-----------------------\n");
	
	fprintf(sfile, "MAX_PARTICLES: \t\t %i \n", MAX_PARTICLES);
	fprintf(sfile, "MOVING_TERRAIN: \t %i \n", MOVING_TERRAIN);
	fprintf(sfile, "USE_AUG_MCL: \t\t %i \n", USE_AUG_MCL);
	fprintf(sfile, "USE_CONTOUR_MATCHING: \t %i \n", USE_CONTOUR_MATCHING);
	fprintf(sfile, "INTEG_PHI_THETA: \t %i \n", INTEG_PHI_THETA);
	fprintf(sfile, "ALLOW_ATTITUDE_SEARCH: \t %i \n", ALLOW_ATTITUDE_SEARCH);
	fprintf(sfile, "ALLOW_FILTER_REINIT: \t %i \n", ALLOW_FILTER_REINIT);
	fprintf(sfile, "USE_MODIFIED_WEIGHTING: \t %i \n", USE_MODIFIED_WEIGHTING);
	fprintf(sfile, "SEARCH_COMPASS_BIAS: \t %i \n", SEARCH_COMPASS_BIAS);
	fprintf(sfile, "SEARCH_ALIGN_STATE: \t %i \n", SEARCH_ALIGN_STATE);
	fprintf(sfile, "SEARCH_GYRO_BIAS: \t %i \n", SEARCH_GYRO_BIAS);
	fprintf(sfile, "SEARCH_GYRO_Y: \t\t %i \n", SEARCH_GYRO_Y);
	fprintf(sfile, "SEARCH_DVL_ERRORS: \t %i \n", SEARCH_DVL_ERRORS);
	fprintf(sfile, "SAVE_PARTICLES: \t %i \n", SAVE_PARTICLES);
	fprintf(sfile, "X_STDDEV_INIT: \t\t %.2f \n", X_STDDEV_INIT);
	fprintf(sfile, "Y_STDDEV_INIT: \t\t %.2f \n", Y_STDDEV_INIT);
	fprintf(sfile, "Z_STDDEV_INIT: \t\t %.2f \n", Z_STDDEV_INIT);
	fprintf(sfile, "PHI_STDDEV_INIT (^o): \t\t %.2f \n", PHI_STDDEV_INIT * 180.0 / PI);
	fprintf(sfile, "THETA_STDDEV_INIT (^o): \t %.2f \n", THETA_STDDEV_INIT * 180.0 / PI);
	fprintf(sfile, "PSI_STDDEV_INIT (^o): \t\t %.2f \n", PSI_STDDEV_INIT * 180.0 / PI);
	fprintf(sfile, "COMPASS_BIAS_STDDEV_INIT (^o): \t %.2f \n", COMPASS_BIAS_STDDEV_INIT * 180.0 / PI);
	fprintf(sfile, "PHI_ALIGN_ERROR_STDDEV_INIT (^o): \t %.2f \n", PHI_ALIGN_ERROR_STDDEV_INIT * 180.0 / PI);
	fprintf(sfile, "THETA_ALIGN_ERROR_STDDEV_INIT (^o): \t %.2f \n",  THETA_ALIGN_ERROR_STDDEV_INIT * 180.0 / PI);
	fprintf(sfile, "PSI_ALIGN_ERROR_STDDEV_INIT (^o): \t %.2f \n", PSI_ALIGN_ERROR_STDDEV_INIT * 180.0 / PI);
	fprintf(sfile, "GYRO_BIAS_STDDEV_INIT (^o/s): \t %.2f \n", GYRO_BIAS_STDDEV_INIT * 180.0 / PI);
	fprintf(sfile, "DVL_SF_STDDEV_INIT (m/s): \t %.2f \n", DVL_SF_STDDEV_INIT);
	fprintf(sfile, "DVL_BIAS_STDDEV_INIT (m/s): \t %.2f \n", DVL_BIAS_STDDEV_INIT);
	fprintf(sfile, "DZ_STDDEV: \t\t %.2f \n", DZ_STDDEV);
	fprintf(sfile, "DPHI_STDDEV (^o): \t %.2f \n", DPHI_STDDEV * 180.0 / PI);
	fprintf(sfile, "DTHETA_STDDEV (^o): \t %.2f \n", DTHETA_STDDEV * 180.0 / PI);
	fprintf(sfile, "DPSI_STDDEV (^o): \t %.2f \n", DPSI_STDDEV * 180.0 / PI);
	fprintf(sfile, "DPSI_RATE_FACTOR_STDDEV: \t %.2f \n", DPSI_RATE_FACTOR_STDDEV * 180.0 / PI);
	fprintf(sfile, "DALIGN_STDDEV: \t\t\t %.2f \n",  DALIGN_STDDEV * 180.0 / PI);
	fprintf(sfile, "DGBIAS_ERROR (^o/sqrt(s)): \t %.6f \n", DGBIAS_ERROR * 180.0 / PI);
	fprintf(sfile, "DDVLSF_STDDEV: \t\t %.4f \n",  DDVLSF_STDDEV);
	fprintf(sfile, "DDVLBIAS_STDDEV: \t %.4f \n", DDVLBIAS_STDDEV);
	fprintf(sfile, "MIN_EFF_SAMP_SIZE: \t %.2f \n",  MIN_EFF_SAMP_SIZE);
	
	fclose(sfile);
}

