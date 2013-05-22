/*--------------------------------------------------------------------
 *    The MB-system:	mb_glwdrawa.c	5/22/2007
 *    $Id: mb_glwdrawa.c 1770 2009-10-19 17:16:39Z caress $
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
 * $Log: mb_glwdrawa.c,v $
 * Revision 5.0  2007/06/17 23:16:14  caress
 * Added MBeditviz.
 *
 * Revision 5.0  2007/05/23 16:39:54  caress
 * Added library for Motif OpenGL widget because this library is no longer uniformly available in Xfree86 distributions.
 *
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

/*
 *
 * This file has been slightly modified from the original for use with Mesa
 *
 *     Jeroen van der Zijp
 *
 *     jvz@cyberia.cfdrc.com
 *
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <Xm/PrimitiveP.h>
#include "mb_glwdrawap.h"
#include <assert.h>
#include <stdio.h>

#define mbGLwDrawingAreaWidget             mbGLwMDrawingAreaWidget
#define mbGLwDrawingAreaClassRec           mbGLwMDrawingAreaClassRec
#define mbglwDrawingAreaClassRec         mbglwMDrawingAreaClassRec
#define mbglwDrawingAreaWidgetClass      mbglwMDrawingAreaWidgetClass
#define mbGLwDrawingAreaRec                mbGLwMDrawingAreaRec

#define ATTRIBLIST_SIZE 32

#define offset(field) XtOffset(mbGLwDrawingAreaWidget,mbglwDrawingArea.field)


/* forward definitions */
static void createColormap(mbGLwDrawingAreaWidget w,int offset,XrmValue *value);
static void Initialize(mbGLwDrawingAreaWidget req,mbGLwDrawingAreaWidget neww,ArgList args,Cardinal *num_args);
static void Realize(Widget w,Mask *valueMask,XSetWindowAttributes *attributes);
static void Redraw(mbGLwDrawingAreaWidget w,XEvent *event,Region region);
static void Resize(mbGLwDrawingAreaWidget mbglw);
static void Destroy(mbGLwDrawingAreaWidget mbglw);
static void mbglwInput(mbGLwDrawingAreaWidget mbglw,XEvent *event,String *params,Cardinal *numParams);



static char defaultTranslations[] =
     "<Key>osfHelp:PrimitiveHelp() \n"
    "<KeyDown>:   mbglwInput() \n\
     <KeyUp>:     mbglwInput() \n\
     <BtnDown>:   mbglwInput() \n\
     <BtnUp>:     mbglwInput() \n\
     <BtnMotion>: mbglwInput() ";


static XtActionsRec actions[] = {
  {"mbglwInput",(XtActionProc)mbglwInput},                /* key or mouse input */
  };


/*
 * There is a bit of unusual handling of the resources here.
 * Because Xt insists on allocating the colormap resource when it is
 * processing the core resources (even if we redeclare the colormap
 * resource here, we need to do a little trick.  When Xt first allocates
 * the colormap, we allow it to allocate the default one, since we have
 * not yet determined the appropriate visual (which is determined from
 * resources parsed after the colormap).  We also let it allocate colors
 * in that default colormap.
 *
 * In the initialize proc we calculate the actual visual.  Then, we
 * reobtain the colormap resource using XtGetApplicationResources in
 * the initialize proc.  If requested, we also reallocate colors in
 * that colormap using the same method.
 */

static XtResource resources[] = {
  /* The GLX attributes.  Add any new attributes here */

  {mbGLwNbufferSize, mbGLwCBufferSize, XtRInt, sizeof (int),
       offset(bufferSize), XtRImmediate, (XtPointer) 0},
  
  {mbGLwNlevel, mbGLwCLevel, XtRInt, sizeof (int),
       offset(level), XtRImmediate, (XtPointer) 0},
  
  {mbGLwNrgba, mbGLwCRgba, XtRBoolean, sizeof (Boolean),
       offset(rgba), XtRImmediate, (XtPointer) FALSE},
  
  {mbGLwNdoublebuffer, mbGLwCDoublebuffer, XtRBoolean, sizeof (Boolean),
       offset(doublebuffer), XtRImmediate, (XtPointer) FALSE},
  
  {mbGLwNstereo, mbGLwCStereo, XtRBoolean, sizeof (Boolean),
       offset(stereo), XtRImmediate, (XtPointer) FALSE},
  
  {mbGLwNauxBuffers, mbGLwCAuxBuffers, XtRInt, sizeof (int),
       offset(auxBuffers), XtRImmediate, (XtPointer) 0},
  
  {mbGLwNredSize, mbGLwCColorSize, XtRInt, sizeof (int),
       offset(redSize), XtRImmediate, (XtPointer) 1},
  
  {mbGLwNgreenSize, mbGLwCColorSize, XtRInt, sizeof (int),
       offset(greenSize), XtRImmediate, (XtPointer) 1},
  
  {mbGLwNblueSize, mbGLwCColorSize, XtRInt, sizeof (int),
       offset(blueSize), XtRImmediate, (XtPointer) 1},
  
  {mbGLwNalphaSize, mbGLwCAlphaSize, XtRInt, sizeof (int),
       offset(alphaSize), XtRImmediate, (XtPointer) 0},
  
  {mbGLwNdepthSize, mbGLwCDepthSize, XtRInt, sizeof (int),
       offset(depthSize), XtRImmediate, (XtPointer) 0},
  
  {mbGLwNstencilSize, mbGLwCStencilSize, XtRInt, sizeof (int),
       offset(stencilSize), XtRImmediate, (XtPointer) 0},
  
  {mbGLwNaccumRedSize, mbGLwCAccumColorSize, XtRInt, sizeof (int),
       offset(accumRedSize), XtRImmediate, (XtPointer) 0},
  
  {mbGLwNaccumGreenSize, mbGLwCAccumColorSize, XtRInt, sizeof (int),
       offset(accumGreenSize), XtRImmediate, (XtPointer) 0},
  
  {mbGLwNaccumBlueSize, mbGLwCAccumColorSize, XtRInt, sizeof (int),
       offset(accumBlueSize), XtRImmediate, (XtPointer) 0},
  
  {mbGLwNaccumAlphaSize, mbGLwCAccumAlphaSize, XtRInt, sizeof (int),
       offset(accumAlphaSize), XtRImmediate, (XtPointer) 0},
  
  /* the attribute list */
  {mbGLwNattribList, mbGLwCAttribList, XtRPointer, sizeof(int *),
       offset(attribList), XtRImmediate, (XtPointer) NULL},

  /* the visual info */
  {mbGLwNvisualInfo, mbGLwCVisualInfo, mbGLwRVisualInfo, sizeof (XVisualInfo *),
       offset(visualInfo), XtRImmediate, (XtPointer) NULL},

  /* miscellaneous resources */
  {mbGLwNinstallColormap, mbGLwCInstallColormap, XtRBoolean, sizeof (Boolean),
       offset(installColormap), XtRImmediate, (XtPointer) TRUE},

  {mbGLwNallocateBackground, mbGLwCAllocateColors, XtRBoolean, sizeof (Boolean),
       offset(allocateBackground), XtRImmediate, (XtPointer) FALSE},

  {mbGLwNallocateOtherColors, mbGLwCAllocateColors, XtRBoolean, sizeof (Boolean),
       offset(allocateOtherColors), XtRImmediate, (XtPointer) FALSE},

  {mbGLwNinstallBackground, mbGLwCInstallBackground, XtRBoolean, sizeof (Boolean),
       offset(installBackground), XtRImmediate, (XtPointer) TRUE},

  {mbGLwNginitCallback, mbGLwCCallback, XtRCallback, sizeof (XtCallbackList),
       offset(ginitCallback), XtRImmediate, (XtPointer) NULL},

  {mbGLwNinputCallback, mbGLwCCallback, XtRCallback, sizeof (XtCallbackList),
       offset(inputCallback), XtRImmediate, (XtPointer) NULL},

  {mbGLwNresizeCallback, mbGLwCCallback, XtRCallback, sizeof (XtCallbackList),
       offset(resizeCallback), XtRImmediate, (XtPointer) NULL},

  {mbGLwNexposeCallback, mbGLwCCallback, XtRCallback, sizeof (XtCallbackList),
       offset(exposeCallback), XtRImmediate, (XtPointer) NULL},

  /* Changes to Motif primitive resources */
  {XmNtraversalOn, XmCTraversalOn, XmRBoolean, sizeof (Boolean),
   XtOffset (mbGLwDrawingAreaWidget, primitive.traversal_on), XmRImmediate,
   (XtPointer)FALSE},
  
  /* highlighting is normally disabled, as when Motif tries to disable
   * highlighting, it tries to reset the color back to the parent's
   * background (usually Motif blue).  Unfortunately, that is in a
   * different colormap, and doesn't work too well.
   */
  {XmNhighlightOnEnter, XmCHighlightOnEnter, XmRBoolean, sizeof (Boolean),
   XtOffset (mbGLwDrawingAreaWidget, primitive.highlight_on_enter),
   XmRImmediate, (XtPointer) FALSE},
  
  {XmNhighlightThickness, XmCHighlightThickness, XmRHorizontalDimension,
   sizeof (Dimension),
   XtOffset (mbGLwDrawingAreaWidget, primitive.highlight_thickness),
   XmRImmediate, (XtPointer) 0},
  };


/*
** The following resources are reobtained using XtGetApplicationResources
** in the initialize proc.
*/

/* The colormap */
static XtResource initializeResources[] = {
  /* reobtain the colormap with the new visual */
  {XtNcolormap, XtCColormap, XtRColormap, sizeof(Colormap),
   XtOffset(mbGLwDrawingAreaWidget, core.colormap),
   XtRCallProc,(XtPointer) createColormap},
  };


/* reallocate any colors we need in the new colormap */
  
/* The background is obtained only if the allocateBackground resource is TRUE*/
static XtResource backgroundResources[] = {
  {XmNbackground, XmCBackground,XmRPixel, 
   sizeof(Pixel),XtOffset(mbGLwDrawingAreaWidget,core.background_pixel),
   XmRString,(XtPointer)"lightgrey"},
   /*XmRCallProc,(XtPointer)_XmBackgroundColorDefault},*/

  {XmNbackgroundPixmap,XmCPixmap,XmRXmBackgroundPixmap, 
   sizeof(Pixmap),XtOffset(mbGLwDrawingAreaWidget,core.background_pixmap),
   XmRImmediate,(XtPointer)XmUNSPECIFIED_PIXMAP},
  };



/* The other colors such as the foreground are allocated only if
 * allocateOtherColors are set.  These resources only exist in Motif.
 */
static XtResource otherColorResources[] = {
  {XmNforeground,XmCForeground,XmRPixel, 
   sizeof(Pixel),XtOffset(mbGLwDrawingAreaWidget,primitive.foreground),
   XmRString,(XtPointer)"lighgrey"},
   /*XmRCallProc, (XtPointer) _XmForegroundColorDefault},*/

  {XmNhighlightColor,XmCHighlightColor,XmRPixel,sizeof(Pixel),
   XtOffset(mbGLwDrawingAreaWidget,primitive.highlight_color),
   XmRString,(XtPointer)"lightgrey"},
   /*XmRCallProc,(XtPointer)_XmHighlightColorDefault},*/

  {XmNhighlightPixmap,XmCHighlightPixmap,XmRPrimHighlightPixmap,
   sizeof(Pixmap),
   XtOffset(mbGLwDrawingAreaWidget,primitive.highlight_pixmap),
   XmRImmediate,(XtPointer)XmUNSPECIFIED_PIXMAP},
   /*XmRCallProc,(XtPointer)_XmPrimitiveHighlightPixmapDefault},*/
  };


#undef offset


mbGLwDrawingAreaClassRec mbglwDrawingAreaClassRec = {
  { /* core fields */
    /* superclass                */        (WidgetClass) &xmPrimitiveClassRec,
    /* class_name                */        "mbGLwMDrawingArea",
    /* widget_size               */        sizeof(mbGLwDrawingAreaRec),
    /* class_initialize          */        NULL,
    /* class_part_initialize     */        NULL,
    /* class_inited              */        FALSE,
    /* initialize                */        (XtInitProc) Initialize,
    /* initialize_hook           */        NULL,
    /* realize                   */        Realize,
    /* actions                   */        actions,
    /* num_actions               */        XtNumber(actions),
    /* resources                 */        resources,
    /* num_resources             */        XtNumber(resources),
    /* xrm_class                 */        NULLQUARK,
    /* compress_motion           */        TRUE,
    /* compress_exposure         */        TRUE,
    /* compress_enterleave       */        TRUE,
    /* visible_interest          */        TRUE,
    /* destroy                   */        (XtWidgetProc) Destroy,
    /* resize                    */        (XtWidgetProc) Resize,
    /* expose                    */        (XtExposeProc) Redraw,
    /* set_values                */        NULL,
    /* set_values_hook           */        NULL,
    /* set_values_almost         */        XtInheritSetValuesAlmost,
    /* get_values_hook           */        NULL,
    /* accept_focus              */        NULL,
    /* version                   */        XtVersion,
    /* callback_private          */        NULL,
    /* tm_table                  */        defaultTranslations,
    /* query_geometry            */        XtInheritQueryGeometry,
    /* display_accelerator       */        XtInheritDisplayAccelerator,
    /* extension                 */        NULL
  },
  {
    /* border_highlight          */        XmInheritBorderHighlight,
    /* border_unhighlight        */        XmInheritBorderUnhighlight,
    /* translations              */        XtInheritTranslations,
    /* arm_and_activate          */        NULL,
    /* get_resources             */        NULL,
    /* num get_resources         */        0,
    /* extension                 */        NULL,                                
  }
  };

WidgetClass mbglwDrawingAreaWidgetClass=(WidgetClass)&mbglwDrawingAreaClassRec;



static void error(Widget w,char* string){
  char buf[100];
  sprintf(buf,"mbGLwMDrawingArea: %s\n",string);
  XtAppError(XtWidgetToApplicationContext(w),buf);
  }


static void warning(Widget w,char* string){
  char buf[100];
  sprintf (buf, "mbGLwMDraw: %s\n", string);
  XtAppWarning(XtWidgetToApplicationContext(w), buf);
  }



/* Initialize the attribList based on the attributes */
static void createAttribList(mbGLwDrawingAreaWidget w){
  int *ptr;
  w->mbglwDrawingArea.attribList = (int*)XtMalloc(ATTRIBLIST_SIZE*sizeof(int));
  if(!w->mbglwDrawingArea.attribList){
    error((Widget)w,"Unable to allocate attribute list");
    }
  ptr = w->mbglwDrawingArea.attribList;
  *ptr++ = GLX_BUFFER_SIZE;
  *ptr++ = w->mbglwDrawingArea.bufferSize;
  *ptr++ = GLX_LEVEL;
  *ptr++ = w->mbglwDrawingArea.level;
  if(w->mbglwDrawingArea.rgba) *ptr++ = GLX_RGBA;
  if(w->mbglwDrawingArea.doublebuffer) *ptr++ = GLX_DOUBLEBUFFER;
  if(w->mbglwDrawingArea.stereo) *ptr++ = GLX_STEREO;
  *ptr++ = GLX_AUX_BUFFERS;
  *ptr++ = w->mbglwDrawingArea.auxBuffers;
  *ptr++ = GLX_RED_SIZE;
  *ptr++ = w->mbglwDrawingArea.redSize;
  *ptr++ = GLX_GREEN_SIZE;
  *ptr++ = w->mbglwDrawingArea.greenSize;
  *ptr++ = GLX_BLUE_SIZE;
  *ptr++ = w->mbglwDrawingArea.blueSize;
  *ptr++ = GLX_ALPHA_SIZE;
  *ptr++ = w->mbglwDrawingArea.alphaSize;
  *ptr++ = GLX_DEPTH_SIZE;
  *ptr++ = w->mbglwDrawingArea.depthSize;
  *ptr++ = GLX_STENCIL_SIZE;
  *ptr++ = w->mbglwDrawingArea.stencilSize;
  *ptr++ = GLX_ACCUM_RED_SIZE;
  *ptr++ = w->mbglwDrawingArea.accumRedSize;
  *ptr++ = GLX_ACCUM_GREEN_SIZE;
  *ptr++ = w->mbglwDrawingArea.accumGreenSize;
  *ptr++ = GLX_ACCUM_BLUE_SIZE;
  *ptr++ = w->mbglwDrawingArea.accumBlueSize;
  *ptr++ = GLX_ACCUM_ALPHA_SIZE;
  *ptr++ = w->mbglwDrawingArea.accumAlphaSize;
  *ptr++ = None;
  assert((ptr-w->mbglwDrawingArea.attribList)<ATTRIBLIST_SIZE);
  }



/* Initialize the visualInfo based on the attribute list */
static void createVisualInfo(mbGLwDrawingAreaWidget w){
  assert(w->mbglwDrawingArea.attribList);
  w->mbglwDrawingArea.visualInfo=glXChooseVisual(XtDisplay(w),XScreenNumberOfScreen(XtScreen(w)),w->mbglwDrawingArea.attribList);
  if(!w->mbglwDrawingArea.visualInfo) error((Widget)w,"requested visual not supported");
  }



/* Initialize the colormap based on the visual info.
 * This routine maintains a cache of visual-infos to colormaps.  If two
 * widgets share the same visual info, they share the same colormap.
 * This function is called by the callProc of the colormap resource entry.
 */
static void createColormap(mbGLwDrawingAreaWidget w,int offset,XrmValue *value){
  static struct cmapCache { Visual *visual; Colormap cmap; } *cmapCache;
  static int cacheEntries=0;
  static int cacheMalloced=0;
  register int i;
    
  assert(w->mbglwDrawingArea.visualInfo);

  /* see if we can find it in the cache */
  for(i=0; i<cacheEntries; i++){
    if(cmapCache[i].visual==w->mbglwDrawingArea.visualInfo->visual){
      value->addr=(XtPointer)(&cmapCache[i].cmap);
      return;
      }
    }

  /* not in the cache, create a new entry */
  if(cacheEntries >= cacheMalloced){
    /* need to malloc a new one.  Since we are likely to have only a
     * few colormaps, we allocate one the first time, and double
     * each subsequent time.
     */
    if(cacheMalloced==0){
      cacheMalloced=1;
      cmapCache=(struct cmapCache*)XtMalloc(sizeof(struct cmapCache));
      }
    else{
      cacheMalloced<<=1;
      cmapCache=(struct cmapCache*)XtRealloc((char*)cmapCache,sizeof(struct cmapCache)*cacheMalloced);
      }
    }
       
  cmapCache[cacheEntries].cmap=XCreateColormap(XtDisplay(w),
                                               RootWindow(XtDisplay(w),
                                               w->mbglwDrawingArea.visualInfo->screen),
                                               w->mbglwDrawingArea.visualInfo->visual,
                                               AllocNone);
  cmapCache[cacheEntries].visual=w->mbglwDrawingArea.visualInfo->visual;
  value->addr=(XtPointer)(&cmapCache[cacheEntries++].cmap);
  }



static void Initialize(mbGLwDrawingAreaWidget req,mbGLwDrawingAreaWidget neww,ArgList args,Cardinal *num_args){

  /* fix size */
  if(req->core.width==0) neww->core.width=100;
  if(req->core.height==0) neww->core.width=100;

  /* create the attribute list if needed */
  neww->mbglwDrawingArea.myList=FALSE;
  if(neww->mbglwDrawingArea.attribList==NULL){
    neww->mbglwDrawingArea.myList=TRUE;
    createAttribList(neww);
    }

  /* Gotta have it */
  assert(neww->mbglwDrawingArea.attribList);

  /* determine the visual info if needed */
  neww->mbglwDrawingArea.myVisual=FALSE;
  if(neww->mbglwDrawingArea.visualInfo==NULL){
    neww->mbglwDrawingArea.myVisual=TRUE;
    createVisualInfo(neww);
    }

  /* Gotta have that too */
  assert(neww->mbglwDrawingArea.visualInfo);

  neww->core.depth=neww->mbglwDrawingArea.visualInfo->depth;

  /* Reobtain the colormap and colors in it using XtGetApplicationResources*/
  XtGetApplicationResources((Widget)neww,neww,initializeResources,XtNumber(initializeResources),args,*num_args);

  /* obtain the color resources if appropriate */
  if(req->mbglwDrawingArea.allocateBackground){
    XtGetApplicationResources((Widget)neww,neww,backgroundResources,XtNumber(backgroundResources),args,*num_args);
    }

  if(req->mbglwDrawingArea.allocateOtherColors){
    XtGetApplicationResources((Widget)neww,neww,otherColorResources,XtNumber(otherColorResources),args,*num_args);
    }
  }



static void Realize(Widget w,Mask *valueMask,XSetWindowAttributes *attributes){
  register mbGLwDrawingAreaWidget mbglw=(mbGLwDrawingAreaWidget)w;
  mbGLwDrawingAreaCallbackStruct cb;
  Widget parentShell;
  Status status;
  Window windows[2],*windowsReturn,*windowList;
  int countReturn,i;
   
  /* if we haven't requested that the background be both installed and
   * allocated, don't install it.
   */
  if(!(mbglw->mbglwDrawingArea.installBackground && mbglw->mbglwDrawingArea.allocateBackground)){
    *valueMask&=~CWBackPixel;
    }
 
  XtCreateWindow(w,(unsigned int)InputOutput,mbglw->mbglwDrawingArea.visualInfo->visual,*valueMask,attributes);

  /* if appropriate, call XSetWMColormapWindows to install the colormap */
  if(mbglw->mbglwDrawingArea.installColormap){

    /* Get parent shell */
    for(parentShell=XtParent(w); parentShell&&!XtIsShell(parentShell); parentShell=XtParent(parentShell));

    if(parentShell && XtWindow(parentShell)){

      /* check to see if there is already a property */
      status=XGetWMColormapWindows(XtDisplay(parentShell),XtWindow(parentShell),&windowsReturn,&countReturn);
            
      /* if no property, just create one */
      if(!status){
        windows[0]=XtWindow(w);
        windows[1]=XtWindow(parentShell);
        XSetWMColormapWindows(XtDisplay(parentShell),XtWindow(parentShell),windows,2);
        }

      /* there was a property, add myself to the beginning */
      else{
        windowList=(Window *)XtMalloc((sizeof(Window))*(countReturn+1));
        windowList[0]=XtWindow(w);
        for(i=0; i<countReturn; i++) windowList[i+1]=windowsReturn[i];
        XSetWMColormapWindows(XtDisplay(parentShell),XtWindow(parentShell),windowList,countReturn+1);
        XtFree((char*)windowList);
        XtFree((char*)windowsReturn);
        }
      }
    else{
      warning(w,"Could not set colormap property on parent shell");
      }
    }

  /* Invoke callbacks */
  cb.reason=mbGLwCR_GINIT;
  cb.event=NULL;
  cb.width=mbglw->core.width;
  cb.height=mbglw->core.height;
  XtCallCallbackList((Widget)mbglw,mbglw->mbglwDrawingArea.ginitCallback,&cb);
  }



static void Redraw(mbGLwDrawingAreaWidget w,XEvent *event,Region region){
  mbGLwDrawingAreaCallbackStruct cb;
  if(!XtIsRealized((Widget)w)) return;
  cb.reason=mbGLwCR_EXPOSE;
  cb.event=event;
  cb.width=w->core.width;
  cb.height=w->core.height;
  XtCallCallbackList((Widget)w,w->mbglwDrawingArea.exposeCallback,&cb);
  }



static void Resize(mbGLwDrawingAreaWidget mbglw){
  mbGLwDrawingAreaCallbackStruct cb;
  if(!XtIsRealized((Widget)mbglw)) return;
  cb.reason=mbGLwCR_RESIZE;
  cb.event=NULL;
  cb.width=mbglw->core.width;
  cb.height=mbglw->core.height;
  XtCallCallbackList((Widget)mbglw,mbglw->mbglwDrawingArea.resizeCallback,&cb);
  }



static void Destroy(mbGLwDrawingAreaWidget mbglw){
  Window *windowsReturn;
  Widget parentShell;
  Status status;
  int countReturn;
  register int i;

  if(mbglw->mbglwDrawingArea.myList && mbglw->mbglwDrawingArea.attribList){
    XtFree((XtPointer)mbglw->mbglwDrawingArea.attribList);
    }

  if(mbglw->mbglwDrawingArea.myVisual && mbglw->mbglwDrawingArea.visualInfo){
    XtFree((XtPointer)mbglw->mbglwDrawingArea.visualInfo);
    }

  /* if my colormap was installed, remove it */
  if(mbglw->mbglwDrawingArea.installColormap){

    /* Get parent shell */
    for(parentShell=XtParent(mbglw); parentShell&&!XtIsShell(parentShell); parentShell=XtParent(parentShell));

    if(parentShell && XtWindow(parentShell)){

      /* make sure there is a property */
      status=XGetWMColormapWindows(XtDisplay(parentShell),XtWindow(parentShell),&windowsReturn,&countReturn);
            
      /* if no property, just return.  If there was a property, continue */
      if(status){

        /* search for a match */
        for(i=0; i<countReturn; i++){
          if(windowsReturn[i]==XtWindow(mbglw)){

            /* we found a match, now copy the rest down */
            for(i++; i<countReturn; i++){ windowsReturn[i-1]=windowsReturn[i]; }

            XSetWMColormapWindows(XtDisplay(parentShell),XtWindow(parentShell),windowsReturn,countReturn-1);
            break; 
            }
          }
        XtFree((char *)windowsReturn);
        }
      }
    }
  }



/* Action routine for keyboard and mouse events */
static void mbglwInput(mbGLwDrawingAreaWidget mbglw,XEvent *event,String *params,Cardinal *numParams){
  mbGLwDrawingAreaCallbackStruct cb;
  cb.reason=mbGLwCR_INPUT;
  cb.event=event;
  cb.width=mbglw->core.width;
  cb.height=mbglw->core.height;
  XtCallCallbackList((Widget)mbglw,mbglw->mbglwDrawingArea.inputCallback,&cb);
  }



/* Create routine */
Widget mbGLwCreateMDrawingArea(Widget parent, char *name,ArgList arglist,Cardinal argcount){
  return XtCreateWidget(name,mbglwMDrawingAreaWidgetClass, parent, arglist,argcount);
  }

