#include "ParticlePlot.h"

//****************************************************************************************

void* viewerThreadFunc(void* viewer) {
	((ParticlePlot*) viewer)->run();
	
	return NULL;
}

//****************************************************************************************

ParticlePlot::ParticlePlot(const char* title, int width, int height) : GlutWindow2d(title, width, height) {
	viewerThreadID = NULL;
	vertices = NULL;
	colors = NULL;
	size = 0;
	pthread_mutex_init(&dataLock, NULL);
	
	followPositionFlag = false;
	cameraCenterPosition.x = 0;
	cameraCenterPosition.y = 0;
	refX = 0;
	refY = 0;
	for(int i = 0; i < 4; i++) {
		mapBoundaries[i].x = 0;
		mapBoundaries[i].y = 0;
	}
	
}

//****************************************************************************************

/*! Basic destructor */
ParticlePlot::~ParticlePlot() {
	if(vertices != NULL) {
		delete [] vertices;
	}
	vertices = NULL;
	
	if(colors != NULL) {
		delete [] colors;
	}
	colors = NULL;
	
	pthread_mutex_destroy(&dataLock);
}

//****************************************************************************************

/*!
  This function reinitializes the plotter by removing all reference data.
*/
void ParticlePlot::reinitPlotter() {
	if(vertices != NULL) {
		delete [] vertices;
	}
	vertices = NULL;
	
	if(colors != NULL) {
		delete [] colors;
	}
	colors = NULL;
	size = 0;
	
	followPositionFlag = false;
	cameraCenterPosition.x = 0;
	cameraCenterPosition.y = 0;
	refX = 0;
	refY = 0;
	
	// pthread_mutex_lock(&dataLock);
	referencePath.clear();
	vehiclePath.clear();
	//pthread_mutex_unlock(&dataLock);
	
}

//****************************************************************************************

/*!
  This function will spawn the viewer as a separate thread and start the viewer.
*/
void ParticlePlot::spawnAsThread() {
	viewerThreadID = new pthread_t;
	pthread_create(viewerThreadID, NULL, viewerThreadFunc, (void*) this);
}

//****************************************************************************************

/*!
  This function needs to be overridden to draw the scene.  This function is called from
  the display function automatically.
*/

void ParticlePlot::draw() {
	glPushMatrix();
	
	positionCamera();
	
	glPushMatrix();
	
	pthread_mutex_lock(&dataLock);
	
	//if the user wants to follow a certain point, then translate over that position
	if(followPositionFlag) {
		glTranslated(-cameraCenterPosition.x, -cameraCenterPosition.y, 0);
	}
	
	glColor3f(1.0, 0.0, 1.0);
	
	
	
	drawAxes();
	if(vertices != NULL) {
		//set lighting
		//glEnable(GL_LIGHTING);
		//glEnable(GL_LIGHT0);
		//GLfloat al[] = {0.2, 0.2, 0.2, 1.0};
		//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, al);
		//GLfloat light_specular[]={-1.0,1.0,-1.0,1.0};
		//glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);
		//GLfloat light_position[] = {-1.0,1.0,-1.0,0.0};
		//glLightfv(GL_LIGHT0,GL_POSITION,light_position);
		
		//draw the map
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		
		glColorPointer(3, GL_DOUBLE, 0, colors);
		glVertexPointer(3, GL_DOUBLE, 0, vertices);
		glDrawArrays(GL_QUADS, 0, numQuads * 4);
		
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		
		//draw the particles
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_POINTS);
		for(int i = 0; i < MAX_PARTICLES; i++) {
			glVertex3d(particles[i].y, particles[i].x, 0.0);
		}
		glEnd();
		
		//draw the map boundaries
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_LINE_LOOP);
		for(int i = 0; i < 4; i++) {
			glVertex2d(mapBoundaries[i].y, mapBoundaries[i].x);
		}
		glEnd();
	}
	
	if(vehiclePath.size() > 1) {
		//draw vehicle path
		glColor3f(1, 1, 1);
		glBegin(GL_LINES);
		for(unsigned int i = 0; i < vehiclePath.size() - 1; i++) {
			glVertex2d(vehiclePath[i].y - this->refY, vehiclePath[i].x - this->refX);
			glVertex2d(vehiclePath[i + 1].y - this->refY, vehiclePath[i + 1].x - this->refX);
		}
		glEnd();
	}
	//draw reference path
	if(referencePath.size() > 1) {
		glColor3f(0.5, 0.5, 0.5);
		glBegin(GL_LINES);
		for(unsigned int i = 0; i < referencePath.size() - 1; i++) {
			glVertex2d(referencePath[i].y - this->refY, referencePath[i].x - this->refX);
			glVertex2d(referencePath[i + 1].y - this->refY, referencePath[i + 1].x - this->refX);
		}
		glEnd();
	}
	
	char temp[64];
	sprintf(temp, "(%0.2f, %0.2f)", mapCoords1.x, mapCoords1.y);
	drawText2d(temp, mapCoords1.x, mapCoords1.y - 10, 1, 1, 1, NULL);
	
	if(displayString.size() != 0) {
		drawText(displayString.c_str(), 0, 0.005, 1, 1, 1, GLUT_BITMAP_HELVETICA_12);
	}
	
	pthread_mutex_unlock(&dataLock);
	
	glPopMatrix();
	
	// If the user is changing the zoom factor, draw the zoom circle
	if(drawZoomCircleFlag) {
		drawZoomCircle();
	}
	glPopMatrix();
}

void ParticlePlot::setData(Matrix& heightValues, double* xPos, double* yPos, particleT* currParticles, const double& refX,
						   const double& refY) {
	pthread_mutex_lock(&dataLock);
	
	//set internal refX/refY
	this->refX = refX;
	this->refY = refY;
	
	setMap(heightValues, xPos, yPos);
	
	setParticles(currParticles);
	
	pthread_mutex_unlock(&dataLock);
}

void ParticlePlot::setDataMapOnly(Matrix& heightValues, double* xPos, double* yPos, const double& refX, const double& refY) {
	pthread_mutex_lock(&dataLock);
	
	//set internal refX/refY
	this->refX = refX;
	this->refY = refY;
	
	setMap(heightValues, xPos, yPos);
	
	pthread_mutex_unlock(&dataLock);
}

//****************************************************************************************

void ParticlePlot::setMap(Matrix& heightValues, double* xPos, double* yPos) {
	//default cell size, this is only used on the boundary
	double defaultCellSize = 1.0;
	
	int index = 0; //temporary variable to store which cell we are drawing now
	int posI, posJ;
	int numEntriesPerQuad = 12; //number of vertices times the number of points. i.e 4*3
	
	//heightValues matrix has North as x, East as y: need to flip for visualization
	heightValues = heightValues.t();
	
	//temp variables to hold the interp color
	float r;
	float g;
	float b;
	
	double minHeight = 1e100;
	double maxHeight = -1e100;
	
	int numRows = heightValues.Nrows();
	int numCols = heightValues.Ncols();
	
	//distance to the adjacent cells
	double diffPrevRow;
	double diffPrevCol;
	double diffNextRow;
	double diffNextCol;
	
	//if the arrays have already been created, delete them.
	if(vertices != NULL) {
		delete [] vertices;
	}
	vertices = new double [numRows * numCols * 4 * 3];
	if(colors != NULL) {
		delete [] colors;
	}
	colors = new double [numRows * numCols * 4 * 3];
	
	//assign the number of quads that are being draw as well as the size of the vectors
	numQuads = numCols * numRows;
	size = numRows * numCols * 4 * 3;
	
	
	//change xPos, yPos vectors to be relative to refX, refY
	for(int i = 0; i < numCols; i++) {
		xPos[i] -= refX;
	}
	for(int j = 0; j < numRows; j++) {
		yPos[j] -= refY;
	}
	
	//find the min and max of the height values
	for(int i = 1; i <= numRows; i++) {
		for(int j = 1; j <= numCols; j++) {
			minHeight = min(fabs(heightValues(i, j)), minHeight);
			maxHeight = max(fabs(heightValues(i, j)), maxHeight);
		}
	}
	if(USE_CONST_SCALE) {
		minHeight = MIN_H;
		maxHeight = MAX_H;
	}
	
	for(int i = 1; i <= numRows; i++) {
		for(int j = 1; j <= numCols; j++) {
			posI = i - 1;
			posJ = j - 1;
			
			//calculate the size of each cell, this just ensures that each quad will line up exactly
			//with the previous one.
			if(posI > 0) {
				diffPrevCol = yPos[posI] - yPos[posI - 1];
			} else {
				diffPrevCol = defaultCellSize;
			}
			
			if(posJ > 0) {
				diffPrevRow = xPos[posJ] - xPos[posJ - 1];
			} else {
				diffPrevRow = defaultCellSize;
			}
			
			if(posI < numRows - 1) {
				diffNextCol = yPos[posI + 1] - yPos[posI];
			} else {
				diffNextCol = defaultCellSize;
			}
			
			if(posJ < numCols - 1) {
				diffNextRow = xPos[posJ + 1] - xPos[posJ];
			} else {
				diffNextRow = defaultCellSize;
			}
			
			
			//calculate the vertices
			vertices[numEntriesPerQuad * index] = yPos[posI] + diffNextCol / 2.0;
			vertices[numEntriesPerQuad * index + 1] = xPos[posJ] - diffPrevRow / 2.0;
			vertices[numEntriesPerQuad * index + 2] = 0.0;
			
			vertices[numEntriesPerQuad * index + 3] = yPos[posI] - diffPrevCol / 2.0;
			vertices[numEntriesPerQuad * index + 4] = xPos[posJ] - diffPrevRow / 2.0;
			vertices[numEntriesPerQuad * index + 5] = 0.0;
			
			vertices[numEntriesPerQuad * index + 6] = yPos[posI] - diffPrevCol / 2.0;
			vertices[numEntriesPerQuad * index + 7] = xPos[posJ] + diffNextRow / 2.0;
			vertices[numEntriesPerQuad * index + 8] = 0.0;
			
			vertices[numEntriesPerQuad * index + 9] = yPos[posI] + diffNextCol / 2.0;
			vertices[numEntriesPerQuad * index + 10] = xPos[posJ] + diffNextRow / 2.0;
			vertices[numEntriesPerQuad * index + 11] = 0.0;
			
			//calculate the colors
			/*
			r = -1.0*(fabs(heightValues(i,j))-minHeight)/(maxHeight-minHeight) + 1.0;
			g = 0.5;
			b = 1.0*(fabs(heightValues(i,j))-minHeight)/(maxHeight-minHeight);
			*/
			computeRGB(r, g, b, heightValues(i, j), minHeight, maxHeight);
			//cout<<r<<" "<<g<<" "<<b<<endl;
			//assign the colors
			colors[numEntriesPerQuad * index] = r;
			colors[numEntriesPerQuad * index + 1] = g;
			colors[numEntriesPerQuad * index + 2] = b;
			
			colors[numEntriesPerQuad * index + 3] = r;
			colors[numEntriesPerQuad * index + 4] = g;
			colors[numEntriesPerQuad * index + 5] = b;
			
			colors[numEntriesPerQuad * index + 6] = r;
			colors[numEntriesPerQuad * index + 7] = g;
			colors[numEntriesPerQuad * index + 8] = b;
			
			colors[numEntriesPerQuad * index + 9] = r;
			colors[numEntriesPerQuad * index + 10] = g;
			colors[numEntriesPerQuad * index + 11] = b;
			
			index++;
		}
	}
	
	//set the follow position
	cameraCenterPosition.x = (yPos[0] - 0.5);
	cameraCenterPosition.y = (xPos[0] - 0.5);
	
	//set the coordinate to display on the screen
	mapCoords1.x = yPos[0];
	mapCoords1.y = xPos[0];
	
	//change xPos, yPos vectors back to be non-relative to refX, refY
	for(int i = 0; i < numCols; i++) {
		xPos[i] += this->refX;
	}
	for(int j = 0; j < numRows; j++) {
		yPos[j] += this->refY;
	}
	
}

//************************************************************************

void ParticlePlot::setParticles(particleT* currParticles) {
	int i;
	for(i = 0; i < MAX_PARTICLES; i++) {
		particles[i].x = currParticles[i].position[0] - this->refX;
		particles[i].y = currParticles[i].position[1] - this->refY;
		particles[i].z = currParticles[i].position[2];
	}
}

//************************************************************************

/*!
  Appends this x,y position onto the vehicle path
*/
void ParticlePlot::addVehiclePath(const double& x, const double& y) {
	pthread_mutex_lock(&dataLock);
	
	Point2d point;
	point.x = x;
	point.y = y;
	vehiclePath.push_back(point);
	
	pthread_mutex_unlock(&dataLock);
	
}
//************************************************************************

/*!
  Appends this x,y onto the reference path
*/
void ParticlePlot::addReferencePath(const double& x, const double& y) {
	pthread_mutex_lock(&dataLock);
	
	Point2d point;
	point.x = x;
	point.y = y;
	referencePath.push_back(point);
	
	pthread_mutex_unlock(&dataLock);
}

//************************************************************************
/*!
  Set current map boundaries
*/
void ParticlePlot::setMapBoundary(const double& minX, const double& maxX,
								  const double& minY, const double& maxY) {
	mapBoundaries[0].x = minX - this->refX;
	mapBoundaries[0].y = minY - this->refY;
	
	mapBoundaries[1].x = minX - this->refX;
	mapBoundaries[1].y = maxY - this->refY;
	
	mapBoundaries[2].x = maxX - this->refX;
	mapBoundaries[2].y = maxY - this->refY;
	
	mapBoundaries[3].x = maxX - this->refX;
	mapBoundaries[3].y = minY - this->refY;
	
}

//************************************************************************

/*!
  Sets the display string
*/
void ParticlePlot::setString(const std::string& str) {
	pthread_mutex_lock(&dataLock);
	
	displayString = str;
	
	pthread_mutex_unlock(&dataLock);
}

//************************************************************************

/*!
  This function is called when an ascii key is pressed.
  @param key The key that was pressed
  @param x The x location of the cursor when the key was pressed
  @param y The y location of the cursor when the key was pressed
*/
void ParticlePlot::processNormalKeys(unsigned char key, int x, int y) {
	switch(key) {
	
		case 'f':
			//turns on or off followModeFlag
			followPositionFlag = !followPositionFlag;
			
			
			break;
	}
	
	GlutWindow2d::processNormalKeys(key, x, y);
}


//************************************************************************

//*! Draws the axes on the screen
void ParticlePlot::drawAxes() {
	double delta = 100;
	double dist = delta;
	
	double xDelta = 0;
	double yDelta = 0;
	if(followPositionFlag) {
		xDelta = cameraCenterPosition.x;
		yDelta = cameraCenterPosition.y;
	}
	
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_LINES);
	
	//*****************************************************
	//draw the horizontal and vertical axes at (0,0)
	glVertex2f(-windowWidth / 2.0 * zoomFactor + panXOffset + xDelta, 0.0);
	glVertex2f(windowWidth / 2.0 * zoomFactor + panXOffset + xDelta, 0.0);
	
	glVertex2f(0.0, -windowHeight / 2.0 * zoomFactor + panYOffset + yDelta);
	glVertex2f(0.0, windowHeight / 2.0 * zoomFactor + panYOffset + yDelta);
	//*****************************************************
	
	//*****************************************************
	//draw the horizontal lines
	glColor3f(0.15, 0.15, 0.15);
	while(dist < windowHeight / 2.0 * zoomFactor + fabs(yDelta) + fabs(panYOffset)) {
		glVertex2f(-windowWidth / 2.0 * zoomFactor + panXOffset + xDelta, -dist);
		glVertex2f(windowWidth / 2.0 * zoomFactor + panXOffset + xDelta, -dist);
		glVertex2f(-windowWidth / 2.0 * zoomFactor + panXOffset + xDelta, dist);
		glVertex2f(windowWidth / 2.0 * zoomFactor + panXOffset + xDelta, dist);
		
		dist += delta;
	}
	
	//draw the vertical lines
	dist = delta;
	while(dist < windowWidth / 2.0 * zoomFactor + fabs(xDelta) + fabs(panXOffset)) {
		glVertex2f(-dist, -windowHeight / 2.0 * zoomFactor + panYOffset + yDelta);
		glVertex2f(-dist, windowHeight / 2.0 * zoomFactor + panYOffset + yDelta);
		glVertex2f(dist, -windowHeight / 2.0 * zoomFactor + panYOffset + yDelta);
		glVertex2f(dist, windowHeight / 2.0 * zoomFactor + panYOffset + yDelta);
		
		dist += delta;
	}
	//******************************************************
	glEnd();
	
}
