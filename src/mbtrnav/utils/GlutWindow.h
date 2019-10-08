#ifndef _GLUT_WINDOW_H_
#define _GLUT_WINDOW_H_

#include <GL/freeglut.h>
#include <iostream>
#include <pthread.h>
#include <math.h>
#include <string.h>
//#include "imageIO/Image.h"
//#include "imageIO/SaveImagePNG.h"

#define GL_F1_KEY 1
#define GL_F2_KEY 2
#define GL_F3_KEY 3
#define GL_UP_KEY 101    // the up arrow key
#define GL_DOWN_KEY 103  // the down arrow key
#define GL_LEFT_KEY 100  // the left arrow key
#define GL_RIGHT_KEY 102 // the right arrow key
#define GL_PUP_KEY 104   // the page up key
#define GL_PDN_KEY 105   // the page down key
#define GL_HOME_KEY 106
#define GL_END_KEY 107

/*! \brief Basic GlutWindow class that creates a window and initializes 
 *         everything.  It also implements a basic user input motion to 
 *         move around in the environment.  
 *
 *
 *  \note: Need to override the draw() function which is called automatically 
 *  from within display
 ********************************************************/

class GlutWindow
{
 public:
   
   /*! Basic constructor */
   GlutWindow(const char *title, int width = 800, int height = 800);
   
   /*! Basic destructor */
   virtual ~GlutWindow();

   /*! This returns the width of the glut window */
   inline int getWidth() const { return windowWidth; };

   /*! This returns the height of the glut window */
   inline int getHeight() const { return windowHeight; };

   /*! returns the status of the button */
   inline int getButton() const { return buttonDown; };

   /*!
     This performs generic graphics matrix setup operations, and
     will be called if the user resizes the window
     @param w Width of the window
     @param h Height of the window
   */
   virtual void reshape(int w, int h) = 0;

   /*! Function that is called when glut is in an "idle state" */
   virtual void idle();

   /*!
     The program will not return from this function call - it will start
     rendering the window
   */
   virtual void run();

   /*! This will setup the camera and then call draw().  */
   virtual void display() = 0;

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

     If you would like to perform other options with the special keys, then 
     when overriding this function, you may want to consider calling 
     GlutWindow::processSpecialKeys(key, x, y) in the overridden function
     to maintain the same base functionality.
     
     @param key The key that was pressed
     @param x The x location of the cursor when the key was pressed
     @param y The y location of the cursor when the key was pressed     
   */
   virtual void processSpecialKeys(int key,int x, int y);

   /*!
     This is used in conjunction with mouseMotion to provide the camera 
     movement.  

     If you would like to perform other operations with the mouse, override
     this function, you may want to consider calling Glut_Window::mouse() 
     in that routine to maintain the mouse interface.
     @param button The button that was pressed
     @param x The x location of the cursor when the button was pressed
     @param y The y location of the cursor when the button was pressed     
   */
   virtual void mouse(int button, int state, int x, int y);

   /*!
     This routine is called whenever a mouse movement event has occured.
     If you are overriding it, consider calling Glut_Window::mouse_motion()
     in your descendant classes, to maintain the mouse interface
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
   
   /*!
      This will draw the given string to the screen at the location given by
      x,y- (0,0 is the bottom left of the window, 1,1 is the top right).
      the color defaults to a gray*/
   void drawText(const char *txt, float x, float y, float r, float g, float b, void *font);
   
protected:
   
   /*! Initializes all of the Glut Function pointers */
   void initFunctionPtrs();

   /*! Any initialization specific to any inherited incarnation */
   virtual void init() = 0;

   /*!
     These are static member functions that provide an interface to the glut callbacks. 
     The glut callbacks require static functions.

     These functions call the appropriate virtual functions
   */
   inline static void _reshape(int w,int h) { myWindow->reshape(w, h); };
   inline static void _idle() { myWindow->idle(); };
   inline static void _display () { myWindow->display(); };
   inline static void _mouse(int button, int state, int x, int y) { myWindow->mouse(button, state, x, y); };
   inline static void _processNormalKeys(unsigned char key,int x, int y) { myWindow->processNormalKeys(key, x, y); };
   inline static void _processSpecialKeys(int key,int x, int y) { myWindow->processSpecialKeys(key, x, y); };
   inline static void _mouseMotion(int x, int y) { myWindow->mouseMotion(x, y); };

   inline double degToRad(const double &angle) { return angle/M_PI; };

   /*!
     A static pointer to the glut window, this is used in the static member functions so that they can call 
     the appropriate class member functions.
    */
   static GlutWindow *myWindow;

   /*! ID of the window */
   int windowID;

   /*! Window Title */
   char *myTitle;

   /*! Window Height */
   int windowHeight;

   /*! Window Width */
   int windowWidth;
   
   /*! Which mouse button is pressed */
   int buttonDown;

   /*! The x location of the mouse when the button is pressed */
   int mouseStartX;

   /*! The x location of the mouse when the button is pressed */
   int mouseStartY;

   //*! Whether the window was launched.  Needed in the destructor to destroy the window or not.
   bool windowLaunched;

   /*! Saves the current screen shot to the filname */
   //void saveScreenshot(char *filename);
};


#endif
