#include "GlutWindow.h"

using namespace std;

/* define the GlutWindow static member*/
GlutWindow* GlutWindow::myWindow;

GlutWindow::GlutWindow(const char* title, int width, int height) {
	myWindow = this;
	
	myTitle = new char[strlen(title)];
	
	strcpy(myTitle, title);
	
	windowWidth = width;
	windowHeight = height;
	
	windowLaunched = false;
}

//********************************************************
/*! Basic destructor */
GlutWindow::~GlutWindow() {
	delete [] myTitle;
	
	if(windowLaunched) {
		glutDestroyWindow(windowID);
	}
}

//********************************************************
/*!

*/
void GlutWindow::idle() {
	//Sleep for 10 milliSeconds so that the thread isn't just
	//wasting cpu time
	usleep(10000);
	glutPostRedisplay();
}

//********************************************************
/*!
  The program will not return from this function call - it will start
  rendering the window
*/
void GlutWindow::run() {
	//initialize the window here because otherwise, the user may
	//setup the window in one thread and then run it in another
	//which will cause a problem.
	init();
	initFunctionPtrs();
	windowLaunched = true;
	glutMainLoop();
	windowLaunched = false;
}

//********************************************************
/*!
  This function is called when an ascii key is pressed.
  @param key The key that was pressed
  @param x The x location of the cursor when the key was pressed
  @param y The y location of the cursor when the key was pressed
*/
void GlutWindow::processNormalKeys(unsigned char key, int x, int y) {
	switch(key) {
	
	
		default:
		
			break;
	}
}

//********************************************************
/*!
  This function is used to perform actions when one of the non-ascii keys
  are presed.  In the base class, the arrow keys are used to move the
  center of the field of view around in the plane, while PAGE_UP &
  PAGE_DOWN are used to change the Z value of the center of view.

  If you would like to perform other options with the special keys, then
  when overriding this function, you may want to consider calling
  GlutWindow::processSpecialKeys(key, x, y) in the overridden function
  to maintain the same base functionality.

  @param key The key that was pressed
  @param x The x location of the cursor when the key was pressed
  @param y The y location of the cursor when the key was pressed
*/
void GlutWindow::processSpecialKeys(int key, int x, int y) {
	switch(key) {
	
		default:
			break;
	}
	
	//Redraw the scene
	glutPostRedisplay();
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
void GlutWindow::mouse(int button, int state, int x, int y) {

}

//********************************************************
/*!
  This routine is called whenever a mouse movement event has occurred.
  If you are overriding it, consider calling Glut_Window::mouse_motion()
  in your descendant classes, to maintain the mouse interface
  @param x The x location of the mouse
  @param y The y location of the mouse
*/
void GlutWindow::mouseMotion(int x, int y) {
	switch(buttonDown) {
	
		default:
			break;
	}
	
	//Redraw the scene
	glutPostRedisplay();
}

//********************************************************
/*!
  Override this function to change the location from which the scene is
  rendered.  It currently uses information from the mouse and the special
  keyboard keys to set its direction
*/
void GlutWindow::positionCamera() {

}

//********************************************************
/*!
  This will draw the given string to the screen at the location given by
  x,y- (0,0 is the bottom left of the window, 1,1 is the top right).
  the color defaults to a gray
*/
void GlutWindow::drawText(const char* txt, float x, float y, float r, float g, float b, void* font) {
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
	glMatrixMode(GL_PROJECTION);
	
	glPushMatrix();
	glLoadIdentity();
	
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_COLOR_BUFFER_BIT);       /* save current colour */
	
	glColor3f(r, g, b);
	
	glRasterPos3f(x, y, 0);
	
	for(unsigned int i = 0; i < strlen(txt); i++) {
		glutBitmapCharacter(font, txt[i]);
	}
	
	glPopAttrib();
	
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glPopAttrib(); // Restore the original matrix mode
	
	if(lightingOn) {
		glEnable(GL_LIGHTING);
	}
	
	error = glGetError();
	
	if(error != GL_NO_ERROR) {
		std::cout << gluErrorString(error) << std::endl;
	}
}

//********************************************************

void GlutWindow::initFunctionPtrs() {
	//Set the function that OpenGL will call to display the contents
	//of the window
	glutDisplayFunc(_display);
	
	//Set the function that OpenGL will call when the user resizes the window
	glutReshapeFunc(_reshape);
	
	//Set the idle functiont that OpenGL will call when the program isn't doing anything
	glutIdleFunc(_idle);
	
	//Set the function that will handle the normal ascii keys
	glutKeyboardFunc(_processNormalKeys);
	
	//Set the function that will handle the special keyobard keys (non-ascii)
	glutSpecialFunc(_processSpecialKeys);
	
	//Set the function that will handle the when a mouse button is clicked
	glutMouseFunc(_mouse);
	
	//Set the function that will handle the mouse motion
	glutMotionFunc(_mouseMotion);
	
}

//********************************************************
/*
void GlutWindow::saveScreenshot(char *filename)
{
   unsigned char pixels[3*windowWidth*windowHeight];
   Image image(windowHeight, windowWidth);

   clock_t start = clock();

   glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
   std::cout<<"getting the pixels took: "<<double(clock() - start)/CLOCKS_PER_SEC<<" seconds"<<std::endl;

   start = clock();

   for(int i = windowHeight-1; i >= 0; i--)
      memcpy((void *)image(i,0), (void *)&pixels[windowWidth*3*(windowHeight- 1 - i)], 3*windowWidth);

   std::cout<<"storing into vw structure took: "<<double(clock() - start)/CLOCKS_PER_SEC<<" seconds"<<std::endl;

   start = clock();

   writePNG(filename, image);

   std::cout<<"saving to file took: "<<double(clock() - start)/CLOCKS_PER_SEC<<" seconds"<<std::endl;
}
*/
