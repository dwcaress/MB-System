#ifndef _GLUT_WINDOWS_3D_H_
#define _GLUT_WINDOWS_3D_H_

#include "GlutWindow.h"
#include <stdlib.h>

class GlutWindow2d : public GlutWindow
{
 public:
   /*! Basic constructor */
   GlutWindow2d(const char *title, int width = 800, int height = 800);
   
   /*! Basic destructor */
   virtual ~GlutWindow2d();

   /*!
     This performs generic graphics matrix setup operations, and
     will be called if the user resizes the window
     @param w Width of the window
     @param h Height of the window
   */
   virtual void reshape(int w, int h);

   /*! This will setup the camera and then call draw().  */
   virtual void display();

   /*!
     Override this function to draw the scene.  This function is called from 
     the display function automatically.
   */
   virtual void draw() = 0;


   /*!
     This function is called when an ascii key is pressed.
     @param key The key that was pressed
     @param x The x location of the cursor when the key was pressed
     @param y The y location of the cursor when the key was pressed
   */
   virtual void processNormalKeys(unsigned char key,int x, int y);

   /*!
     This function is used to perform actions when one of the non-ascii keys
     are presed.  
     
     @param key The key that was pressed
     @param x The x location of the cursor when the key was pressed
     @param y The y location of the cursor when the key was pressed     
   */
   virtual void processSpecialKeys(int key,int x, int y);

   /*!
     This is used in conjunction with mouseMotion to provide the camera 
     movement.  

     @param button The button that was pressed
     @param x The x location of the cursor when the button was pressed
     @param y The y location of the cursor when the button was pressed     
   */
   virtual void mouse(int button, int state, int x, int y);

   /*!
     This routine is called whenever a mouse movement event has occured.
     
     @param x The x location of the mouse
     @param y The y location of the mouse
   */
   virtual void mouseMotion(int x, int y);

   /*!
     Overide this function to change the location from which the scene is 
     rendered.  It currently uses information from the mouse and the special 
     keyboard keys to set its direction
   */
   virtual void positionCamera();

 protected:
   
   /*! Setups and Initializes the window */
   virtual void init();
   
   /*! Factor to adjust the zoom */
   double zoomFactor;

   /*! Remember where the zoom started from */
   double mouseStartZoom;

   /*! The value for the panning in the x direction */
   double panXOffset;
      /*! The value for the panning in the y direction */
   double panYOffset;

   /*! Remember where the pan x offset started from when the mouse is clicked */
   double mousePanXOffset;
   /*! Remember where the pan y offset started from when the mouse is clicked */
   double mousePanYOffset;
   
   /*! Whether or not to draw the zoom circle */
   bool drawZoomCircleFlag;
   /*! The zoom radius for drawing purposes */
   double zoomDrawRadius;


   //***********************************************************
   // ***************** Protected functions ********************
   //***********************************************************
   /*! Draws the zoom circle when changing the zoom factor */
   void drawZoomCircle();
   
   /*! Set the projection parameters */
   void setProjectiveSettings();
   
   void drawText2d(const char *txt, float x, float y, float r, float g, float b, void *font);
   
 private:
   

};



#endif
