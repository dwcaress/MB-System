/*--------------------------------------------------------------------
 *    The MB-system:	mb_glwdrawa.h	5/22/2007
 *    $Id: mb_glwdrawa.h,v 5.0 2007-05-23 16:39:54 caress Exp $
 *
 *    Altered from original code for MB-System by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See notes below for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 *
 * Author:	D. W. Caress
 * Date:	May 22, 2007
 *
 * $Log: not supported by cvs2svn $
 *
 */
/*------------------------------------------------------------------------------*/
/*
 * (c) Copyright 1993, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED 
 * Permission to use, copy, modify, and distribute this software for 
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that 
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. 
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * 
 * US Government Users Restricted Rights 
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(TM) is a trademark of Silicon Graphics, Inc.
 */
#ifndef _mbGLwDrawA_h
#define _mbGLwDrawA_h

#include <GL/glx.h>
#include <GL/gl.h>

/****************************************************************
 *
 * mbGLwDrawingArea widgets
 *
 ****************************************************************/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 attribList	     AttribList		int *		NULL
 visualInfo	     VisualInfo		VisualInfo	NULL
 installColormap     InstallColormap	Boolean		TRUE
 allocateBackground  AllocateColors	Boolean		FALSE
 allocateOtherColors AllocateColors	Boolean		FALSE
 installBackground   InstallBackground	Boolean		TRUE
 exposeCallback      Callback		Pointer		NULL
 ginitCallback       Callback		Pointer		NULL
 inputCallback       Callback		Pointer		NULL
 resizeCallback      Callback		Pointer		NULL

*** The following resources all correspond to the GLX configuration
*** attributes and are used to create the attribList if it is NULL
 bufferSize	     BufferSize		int		0
 level		     Level		int		0
 rgba		     Rgba		Boolean		FALSE
 doublebuffer	     Doublebuffer	Boolean		FALSE
 stereo		     Stereo		Boolean		FALSE
 auxBuffers	     AuxBuffers		int		0
 redSize	     ColorSize		int		1
 greenSize	     ColorSize		int		1
 blueSize	     ColorSize		int		1
 alphaSize	     AlphaSize		int		0
 depthSize	     DepthSize		int		0
 stencilSize	     StencilSize	int		0
 accumRedSize	     AccumColorSize	int		0
 accumGreenSize	     AccumColorSize	int		0
 accumBlueSize	     AccumColorSize	int		0
 accumAlphaSize	     AccumAlphaSize	int		0
*/

#define mbGLwNattribList		"attribList"
#define mbGLwCAttribList		"AttribList"
#define mbGLwNvisualInfo		"visualInfo"
#define mbGLwCVisualInfo		"VisualInfo"
#define mbGLwRVisualInfo		"VisualInfo"

#define mbGLwNinstallColormap	"installColormap"
#define mbGLwCInstallColormap	"InstallColormap"
#define mbGLwNallocateBackground	"allocateBackground"
#define mbGLwNallocateOtherColors	"allocateOtherColors"
#define mbGLwCAllocateColors	"AllocateColors"
#define mbGLwNinstallBackground	"installBackground"
#define mbGLwCInstallBackground	"InstallBackground"

#define mbGLwCCallback		"Callback"
#define mbGLwNexposeCallback	"exposeCallback"
#define mbGLwNginitCallback	"ginitCallback"
#define mbGLwNresizeCallback	"resizeCallback"
#define mbGLwNinputCallback	"inputCallback"

#define mbGLwNbufferSize		"bufferSize"
#define mbGLwCBufferSize		"BufferSize"
#define mbGLwNlevel		"level"
#define mbGLwCLevel		"Level"
#define mbGLwNrgba		"rgba"
#define mbGLwCRgba		"Rgba"
#define mbGLwNdoublebuffer	"doublebuffer"
#define mbGLwCDoublebuffer	"Doublebuffer"
#define mbGLwNstereo		"stereo"
#define mbGLwCStereo		"Stereo"
#define mbGLwNauxBuffers		"auxBuffers"
#define mbGLwCAuxBuffers		"AuxBuffers"
#define mbGLwNredSize		"redSize"
#define mbGLwNgreenSize		"greenSize"
#define mbGLwNblueSize		"blueSize"
#define mbGLwCColorSize		"ColorSize"
#define mbGLwNalphaSize		"alphaSize"
#define mbGLwCAlphaSize		"AlphaSize"
#define mbGLwNdepthSize		"depthSize"
#define mbGLwCDepthSize		"DepthSize"
#define mbGLwNstencilSize		"stencilSize"
#define mbGLwCStencilSize		"StencilSize"
#define mbGLwNaccumRedSize	"accumRedSize"
#define mbGLwNaccumGreenSize	"accumGreenSize"
#define mbGLwNaccumBlueSize	"accumBlueSize"
#define mbGLwCAccumColorSize	"AccumColorSize"
#define mbGLwNaccumAlphaSize	"accumAlphaSize"
#define mbGLwCAccumAlphaSize	"AccumAlphaSize"

#define mbglwMDrawingAreaWidgetClass    mbglwM2DrawingAreaWidgetClass
#define mbglwMDrawingAreaClassRec   mbglwM2DrawingAreaClassRec
#define mbGLwCreateMDrawingArea     mbGLwCreateM2DrawingArea

typedef struct _mbGLwMDrawingAreaClassRec	*mbGLwMDrawingAreaWidgetClass;
typedef struct _mbGLwMDrawingAreaRec	*mbGLwMDrawingAreaWidget;

extern WidgetClass mbglwMDrawingAreaWidgetClass;

/* Callback reasons */
#define mbGLwCR_EXPOSE	XmCR_EXPOSE
#define mbGLwCR_RESIZE	XmCR_RESIZE
#define mbGLwCR_INPUT	XmCR_INPUT

#define mbGLwCR_GINIT	32135	/* Arbitrary number that should neverr clash */

typedef struct 
  {
  int       reason;
  XEvent   *event;
  Dimension width,height;
  } 
  mbGLwDrawingAreaCallbackStruct;

/* front ends to glXMakeCurrent and glXSwapBuffers */
extern void mbGLwDrawingAreaMakeCurrent(Widget w,GLXContext ctx);
extern void mbGLwDrawingAreaSwapBuffers(Widget w);

#ifdef _NO_PROTO
GLAPI Widget mbGLwCreateMDrawingArea();
#else
GLAPI Widget mbGLwCreateMDrawingArea(Widget parent,char *name,ArgList arglist,Cardinal argcount);
#endif

#endif
