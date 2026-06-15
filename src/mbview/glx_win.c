#include <stdio.h>
#include <windows.h>
#include <Wingdi.h>

void glXSwapBuffers(void *lixo, HDC device) {
fprintf(stderr, "MERDA Swapp\n");
	SwapBuffers(device);
}

void glXMakeCurrent(void *lixo, HDC device, HGLRC context){
fprintf(stderr, "MERDA Make\n");
	wglMakeCurrent(device, context);
}

void glXDestroyContext(void *lixo, HGLRC context){
fprintf(stderr, "MERDA Destroy\n");
	wglDeleteContext(context);
}

void glXCreateContext(void *lixo, HGLRC context, int dumb1, int dumb2){
fprintf(stderr, "MERDA Create\n");
	wglCreateContext(context);
}

void glXChooseVisual(void *lixo, HDC device, void *ppfd){
fprintf(stderr, "MERDA Choose\n");
	ChoosePixelFormat(device, ppfd);
}
