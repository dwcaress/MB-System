#include "GlutWindow2d.h"
#include <iostream>
using namespace std;
//********************************************************
/*! Basic constructor */
GlutWindow2d::GlutWindow2d(const char* title, int width, int height) : GlutWindow(title, width, height) {
	//initialize the zoom factor to nothing
	zoomFactor = 1.0;
	
	//initialize the panning offsets
	panXOffset = 0;
	panYOffset = 0;
	
	drawZoomCircleFlag = false;
	zoomDrawRadius = 0;
}


//********************************************************
/*! Basic destructor */
GlutWindow2d::~GlutWindow2d() {

}

//********************************************************
/*!
  This performs generic graphics matrix setup operations, and
  will be called if the user resizes the window
  @param w Width of the window
  @param h Height of the window
*/
void GlutWindow2d::reshape(int w, int h) {
	//need to set the viewable area on the screen to the dimensions of the window
	glViewport(0, 0, w, h);
	
	//I am setting a state where I am editing the projection matrix...
	glMatrixMode(GL_PROJECTION);
	
	//Clearing the projection matrix...
	glLoadIdentity();
	
	//Creating an orthoscopic view matrix going from -1 -> 1 in each
	//dimension on the screen (x, y, z).
	//void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	
	glOrtho(-w / 2 * zoomFactor + panXOffset, w / 2 * zoomFactor + panXOffset, -h / 2 * zoomFactor + panYOffset,
			h / 2 * zoomFactor + panYOffset, -1, 1);
	//glOrtho(-1, 1, -1, 1, -1, 1);
	
	
	//Now editing the model-view matrix.
	glMatrixMode(GL_MODELVIEW);
	
	//Clearing the model-view matrix.
	glLoadIdentity();
	
	windowWidth = w;
	windowHeight = h;
}

//********************************************************
/*!
  This will setup the camera and then call draw().
*/
void GlutWindow2d::display() {
	//Now editing the model-view matrix.
	//glMatrixMode(GL_MODELVIEW);
	
	//clear the screen to the clear color (default black)
	glClear(GL_COLOR_BUFFER_BIT);
	
	//glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//set the default object color to white
	glColor3f(1, 1, 1);
	
	glPushMatrix();
	
	//position the camera at the correct location
	positionCamera();
	
	//call the overridden function to draw the current scene
	draw();
	
	glPopMatrix();
	
	//force execution of the previous GL commands in finite time
	glFlush();
	
	glutSwapBuffers();
}

//********************************************************
/*!
  This function is called when an ascii key is pressed.
  @param key The key that was pressed
  @param x The x location of the cursor when the key was pressed
  @param y The y location of the cursor when the key was pressed
*/
void GlutWindow2d::processNormalKeys(unsigned char key, int x, int y) {
	switch(key) {
		case 27: //esc
			exit(0);
			
			break;
			
		case 'r': //resets all of the zoom and pan parameters
			zoomFactor = 1.0;
			panXOffset = 0;
			panYOffset = 0;
			
			setProjectiveSettings();
			break;
			
		default:
		
			break;
	}
	
	GlutWindow::processNormalKeys(key, x, y);
}

//********************************************************
/*!
  This function is used to perform actions when one of the non-ascii keys
  are presed.  In the base class, the arrow keys are used to move the
  center of the field of view around in the plane, while PAGE_UP &
  PAGE_DOWN are used to change the Z value of the center of view.

  If you would like to perform other options with the special keys, then
  when overriding this function, you may want to consider calling
  GlutWindow2d::processSpecialKeys(key, x, y) in the overridden function
  to maintain the same base functionality.

  @param key The key that was pressed
  @param x The x location of the cursor when the key was pressed
  @param y The y location of the cursor when the key was pressed
*/
void GlutWindow2d::processSpecialKeys(int key, int x, int y) {
	switch(key) {
		case GL_UP_KEY:
		
			break;
			
		case GL_DOWN_KEY:
		
			break;
			
		case GL_LEFT_KEY:
		
			break;
			
		case GL_RIGHT_KEY:
		
			break;
			
		case GL_PUP_KEY:
		
			break;
			
		case GL_PDN_KEY:
		
			break;
			
		default:
			break;
	}
	
	//Redraw the scene occurs in the base process special keys
	GlutWindow::processSpecialKeys(key, x, y);
}

//********************************************************
/*!
  This is used in conjunction with mouseMotion to provide the camera
  movement.  Moving the mouse with the left button down rotates the camera
  around a fixed point (specified by viewX, viewY, viewZ).  Moving the
  mouse with the right button down zooms the camera in and out

  If you would like to perform other operations with the mouse, override
  this function, you may want to consider calling Glut_Window::mouse()
  in that routine to maintain the mouse interface.
  @param button The button that was pressed
  @param x The x location of the cursor when the button was pressed
  @param y The y location of the cursor when the button was pressed
*/
void GlutWindow2d::mouse(int button, int state, int x, int y) {
	if(state == GLUT_UP) {
		buttonDown = -1;
		//enable the flag to not draw the zoom circle
		drawZoomCircleFlag = false;
	} else {
		//store the button
		buttonDown = button;
		switch(buttonDown) {
			case 0:
				//adjust the mouse positions to be from the center of the screen
				mouseStartX = x - windowWidth / 2;
				mouseStartY = (windowHeight - y) - windowHeight / 2;
				mouseStartZoom = zoomFactor;
				
				//store the current radius for drawing purposes
				zoomDrawRadius = sqrt(x * x + y * y);
				//enable the flag to draw the zoom circle
				drawZoomCircleFlag = true;
				break;
				
			case 2:
				mousePanXOffset = panXOffset;
				mousePanYOffset = panYOffset;
				
				mouseStartX = x;
				mouseStartY = y;
				
				break;
		}
	}
	
	GlutWindow::mouse(button, state, x, y);
}

//********************************************************
/*!
  This routine is called whenever a mouse movement event has occurred.
  If you are overriding it, consider calling Glut_Window::mouse_motion()
  in your descendant classes, to maintain the mouse interface
  @param x The x location of the mouse
  @param y The y location of the mouse
*/
void GlutWindow2d::mouseMotion(int x, int y) {
	switch(buttonDown) {
		case -1:
		
			return;
			
		case 2:
			panXOffset = mousePanXOffset + (mouseStartX - x) * zoomFactor;
			//the y pan offset update equation is negative since y goes from 0 to windowHeight from top to bottom
			panYOffset = mousePanYOffset - (mouseStartY - y) * zoomFactor;
			
			setProjectiveSettings();
			
			break;
			
		case 0:
			//adjust the mouse positions to be from the center of the screen
			x -= windowWidth / 2;
			y = (windowHeight - y) - windowHeight / 2;
			
			double origDist = sqrt(mouseStartX * mouseStartX + mouseStartY * mouseStartY);
			double currDist = sqrt(x * x + y * y);
			
			//store the current radius for drawing purposes
			zoomDrawRadius = currDist;
			
			//adjust the zoom factor based upon the ratio of the radii
			zoomFactor = mouseStartZoom * (origDist / currDist);
			
			
			setProjectiveSettings();
			
			break;
			
			
			
	}
	
	//redraw occurs in the base
	GlutWindow::mouseMotion(x, y);
}

//********************************************************
/*!
  Override this function to change the location from which the scene is
  rendered.  It currently uses information from the mouse and the special
  keyboard keys to set its direction
*/
void GlutWindow2d::positionCamera() {

}

//********************************************************

void GlutWindow2d::init() {
	char* dummy_argv[1];
	int dummy_argc = 1;
	dummy_argv[0] = new char[64];
	strcpy(dummy_argv[0], "dummy");
	
	//Initialize glut
	glutInit(&dummy_argc, dummy_argv);
	
	//set the display mode to double-buffered, R
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	
	//set the window size
	glutInitWindowSize(windowWidth, windowHeight);
	
	//set the window position
	glutInitWindowPosition(10, 10);
	
	// Create the window and save the window ID
	windowID = glutCreateWindow(myTitle);
	
	//Disabling the depth test (z will not be used in 2D)
	glDisable(GL_DEPTH_TEST);
	
	//Sets the clear color for when OpenGL clears the entire screen
	glClearColor(0, 0, 0, 1.0f);
	
	delete [] dummy_argv[0];
}

//********************************************************

/*! Draws the zoom circle when changing the zoom factor */
void GlutWindow2d::drawZoomCircle() {
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_LINE_LOOP);
	for(unsigned int theta = 0; theta < 360; theta += 1) {
		glVertex2f(cos(theta * M_PI / 180)*zoomDrawRadius * zoomFactor + panXOffset,
				   sin(theta * M_PI / 180)*zoomDrawRadius * zoomFactor + panYOffset);
	}
	
	glEnd();
}

//********************************************************

/*! Set the projection parameters */
void GlutWindow2d::setProjectiveSettings() {
	//update the zoom
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-windowWidth / 2 * zoomFactor + panXOffset, windowWidth / 2 * zoomFactor + panXOffset,
			-windowHeight / 2 * zoomFactor + panYOffset, windowHeight / 2 * zoomFactor + panYOffset, -1, 1);
	//put it back in GL_MODELVIEW
	glMatrixMode(GL_MODELVIEW);
}

//********************************************************
/*!
  This will draw the given string to the screen at the location given by
  x,y- (relative to the current window properties).
  the color defaults to a gray
*/
void GlutWindow2d::drawText2d(const char* txt, float x, float y, float r, float g, float b, void* font) {
	if(font == NULL) {
		font = GLUT_BITMAP_HELVETICA_10;
	}
	
	GLboolean lightingOn;
	GLenum error;
	
	lightingOn = glIsEnabled(GL_LIGHTING);        /* lighting on? */
	
	if(lightingOn) {
		glDisable(GL_LIGHTING);
	}
	
	glPushAttrib(GL_TRANSFORM_BIT); // Save the current matrix mode
	
	glPushAttrib(GL_COLOR_BUFFER_BIT);       /* save current colour */
	
	glColor3f(r, g, b);
	
	glRasterPos3f(x, y, 0);
	
	for(unsigned int i = 0; i < strlen(txt); i++) {
		glutBitmapCharacter(font, txt[i]);
	}
	
	glPopAttrib();
	
	glPopAttrib(); // Restore the original matrix mode
	
	if(lightingOn) {
		glEnable(GL_LIGHTING);
	}
	
	error = glGetError();
	
	if(error != GL_NO_ERROR) {
		std::cout << gluErrorString(error) << std::endl;
	}
}
