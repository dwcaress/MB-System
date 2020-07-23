// These first three lines address
// issue described at
// https://stackoverflow.com/questions/18642155/no-override-found-for-vtkpolydatamapper
#include "vtkAutoInit.h" 
VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType)

#include "CanvasHandler.h"


int main(int argc, char **argv)
{
#ifdef __linux
	putenv((char *)"MESA_GL_VERSION_OVERRIDE=3.2");

	// Fixes decimal point issue in vtkSTLReader
	putenv((char *)"LC_NUMERIC=C");
#endif //LINUX

	CanvasHandler(argc, argv);

	return 0;
}
