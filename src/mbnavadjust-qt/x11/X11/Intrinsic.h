/*
 * Stands in for the X11 Intrinsics header while compiling the unchanged
 * mbnavadjust_prog.c and the mbview header without X11. It is on the
 * include path of the mbnavadjust-qt-core sources only.
 */
#ifndef MBNAQ_X11_INTRINSIC_H_
#define MBNAQ_X11_INTRINSIC_H_

typedef void *Widget;
typedef void *XtPointer;
typedef void *XtAppContext;
typedef char *String;
typedef char Boolean;
typedef unsigned long Window;
typedef unsigned long Cursor;
typedef struct _XDisplay Display;
typedef union _XEvent XEvent;
typedef struct { int unused; } XVisualInfo;
typedef void *GC;
typedef struct { int unused; } XFontStruct;
typedef struct { int unused; } Visual;
typedef unsigned long Colormap;

#endif
