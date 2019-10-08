#ifndef _PARTICLE_PLOT_H_
#define _PARTICLE_PLOT_H_


#include "GlutWindow2d.h"
#include "ColorHelper.h"
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

#include <newmatap.h>
#include <newmatio.h>
#include "TNavParticleFilter.h"
#include "particleFilterDefs.h"
#include "matrixArrayCalcs.h"

#define USE_CONST_SCALE 0
#define MIN_H 50
#define MAX_H 500

struct Point3d
{
  double x, y, z;
};

struct Point2d
{
  double x,y;
};

class ParticlePlot : public GlutWindow2d
{
 public:
   
   /*! Basic constructor */
   ParticlePlot(const char *title, int width = 800, int height = 800);
   
   /*! Basic destructor */
   virtual ~ParticlePlot();

   /*! Reinitiatlization function */
   void reinitPlotter();
   
   /*!This function will spawn the viewer as a separate thread and start the 
    * viewer.
    */
   void spawnAsThread();

   /*!
     This function is called from the display function automatically.
   */
   virtual void draw();
   
   /*!
     sets the map data and the particle data.
     /param heightValues - the matrix of height values for the position in xPos and yPos
     /param xPos - the x position of the height values
     /param yPos - the y position of the height values
     /param currParticles - a pointer to the current list of particles
     /param refX - the x position of where to reference the map and particles from
     /param refY - the y position of where to reference the map and particles from
   */
   void setData(Matrix& heightValues, double *xPos, double *yPos, particleT* currParticles, const double &refX, const double &refY);

   void setDataMapOnly(Matrix& heightValues, double *xPos, double *yPos, const double &refX, const double &refY);


   /*!
     Sets the map and creates the open gl vertices and color lists
   */
   void setMap(Matrix& heightValues, double *xPos, double *yPos);
   
   /*!
     Copys the particles in the viewer
   */
   void setParticles(particleT* currParticles);

   /*!
     Appends this x,y position onto the vehicle path
   */
   void addVehiclePath(const double &x, const double &y);
   
   /*!
     Appends this x,y onto the reference path
   */
   void addReferencePath(const double &x, const double &y);
   
   /*!
     Set current map boundaries
   */
   void setMapBoundary(const double &minX, const double &maxX,
		       const double &minY, const double &maxY);
   /*!
     Sets the display string
   */
   void setString(const std::string &str);

   /*!
     This function is called when an ascii key is pressed.
     @param key The key that was pressed
     @param x The x location of the cursor when the key was pressed
     @param y The y location of the cursor when the key was pressed
   */
   void processNormalKeys(unsigned char key,int x, int y);


protected:

   //! the handle of the thread
   pthread_t *viewerThreadID;

   //! the vertices of the grid
   double *vertices;
   //! the color of each vertex in the grid
   double *colors;
   //! the size of the vertices and color arrays
   int size;
   //! the number of quads in the grid we are trying to display
   int numQuads;

   //! current north/east reference point
   double refX, refY;

   //! the particles to display on the screen
   Point3d particles[MAX_PARTICLES];
   
   //! the mutex lock for the data so that the drawer isn't trying to display it while the user is setting it
   pthread_mutex_t dataLock;
   
   //! whether or not to follow a position on the screen
   bool followPositionFlag;
   //! if the user wants to follow a position on the screen, this specifies that position
   Point2d cameraCenterPosition;

   //! the following two are two sequences of paths to display on the screen.
   std::vector<Point2d> vehiclePath;
   std::vector<Point2d> referencePath;
   
   //! coordinates of places to display the text on the screen
   Point2d mapCoords1;
   Point2d mapCoords2;

   //! bounding vertices of current search map
   Point2d mapBoundaries[4];

   //! string to display at the bottom of the screen
   std::string displayString;

   //! draws the axes and grid on the screen
   void drawAxes();
};


#endif
