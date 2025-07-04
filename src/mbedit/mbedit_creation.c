/*--------------------------------------------------------------------
 *    The MB-system:	mbedit_creation.c	1993
 *
 *    Copyright (c) 1993-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/BulletinB.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/FileSB.h>
#include <Xm/CascadeB.h>
#include <Xm/DrawingA.h>
#include <Xm/MainW.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/BulletinB.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/FileSB.h>
#include <Xm/CascadeB.h>
#include <Xm/DrawingA.h>

#define DECLARE_BX_GLOBALS

#include "mbedit_creation.h"

#ifndef SANS
#define SANS "helvetica"
#endif
#ifndef SERIF
#define SERIF "times"
#endif
#ifndef MONO
#define MONO "courier"
#endif

// Convenience functions from utilities file.
extern void RegisterBxConverters(XtAppContext);
extern XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);

// Declarations for callbacks and handlers.
extern void do_quit(Widget, XtPointer, XtPointer);
extern void do_filelist_remove(Widget, XtPointer, XtPointer);
extern void do_editlistselection(Widget, XtPointer, XtPointer);
extern void BxUnmanageCB(Widget, XtPointer, XtPointer);
extern void do_output_edit_filelist(Widget, XtPointer, XtPointer);
extern void do_output_browse_filelist(Widget, XtPointer, XtPointer);
extern void do_reset_filters(Widget, XtPointer, XtPointer);
extern void do_set_filters(Widget, XtPointer, XtPointer);
extern void do_check_median_ltrack(Widget, XtPointer, XtPointer);
extern void do_check_median_xtrack(Widget, XtPointer, XtPointer);
extern void do_y_interval(Widget, XtPointer, XtPointer);
extern void do_x_interval(Widget, XtPointer, XtPointer);
extern void do_buffer_hold(Widget, XtPointer, XtPointer);
extern void do_buffer_size(Widget, XtPointer, XtPointer);
extern void do_load_ok(Widget, XtPointer, XtPointer);
extern void do_load_ok_with_save(Widget, XtPointer, XtPointer);
extern void do_goto_apply(Widget, XtPointer, XtPointer);
extern void do_load_check(Widget, XtPointer, XtPointer);
extern void do_output_edit(Widget, XtPointer, XtPointer);
extern void do_output_browse(Widget, XtPointer, XtPointer);
extern void do_event(Widget, XtPointer, XtPointer);
extern void do_expose(Widget, XtPointer, XtPointer);
extern void do_end(Widget, XtPointer, XtPointer);
extern void do_start(Widget, XtPointer, XtPointer);
extern void do_flag_view(Widget, XtPointer, XtPointer);
extern void do_unflag_all(Widget, XtPointer, XtPointer);
extern void do_unflag_view(Widget, XtPointer, XtPointer);
extern void BxManageCB(Widget, XtPointer, XtPointer);
extern void do_next_buffer(Widget, XtPointer, XtPointer);
extern void do_done(Widget, XtPointer, XtPointer);
extern void do_forward(Widget, XtPointer, XtPointer);
extern void do_reverse(Widget, XtPointer, XtPointer);
extern void do_scale_x(Widget, XtPointer, XtPointer);
extern void do_scale_y(Widget, XtPointer, XtPointer);
extern void do_number_pings(Widget, XtPointer, XtPointer);
extern void do_number_step(Widget, XtPointer, XtPointer);
extern void do_view_mode(Widget, XtPointer, XtPointer);
extern void do_show_flaggedsoundings(Widget, XtPointer, XtPointer);
extern void do_show_flaggedprofiles(Widget, XtPointer, XtPointer);
extern void do_show_flags(Widget, XtPointer, XtPointer);
extern void do_show_detects(Widget, XtPointer, XtPointer);
extern void do_show_pulsetypes(Widget, XtPointer, XtPointer);
extern void do_show_time(Widget, XtPointer, XtPointer);
extern void do_reverse_keys(Widget, XtPointer, XtPointer);
extern void do_reverse_mouse(Widget, XtPointer, XtPointer);
extern void do_mode_toggle(Widget, XtPointer, XtPointer);
extern void do_mode_pick(Widget, XtPointer, XtPointer);
extern void do_mode_erase(Widget, XtPointer, XtPointer);
extern void do_mode_restore(Widget, XtPointer, XtPointer);
extern void do_mode_grab(Widget, XtPointer, XtPointer);
extern void do_mode_info(Widget, XtPointer, XtPointer);

// Create the window_mbedit hierarchy of widgets.
Widget Createwindow_mbedit(Widget parent) {
	Arg args[256];
	Boolean argok = False;

	// Register the converters for the widgets.
	RegisterBxConverters(XtWidgetToApplicationContext(parent));
	XtInitializeWidgetClass((WidgetClass)xmMainWindowWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmDialogShellWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmFormWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmPushButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmLabelWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmToggleButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmScrolledWindowWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmListWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmBulletinBoardWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmBulletinBoardWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmScaleWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmSeparatorWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmTextFieldWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmFileSelectionBoxWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmCascadeButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmDrawingAreaWidgetClass);

	Cardinal ac = 0;
	XtSetArg(args[ac], XmNx, 114);
	ac++;
	XtSetArg(args[ac], XmNy, 631);
	ac++;
	XtSetArg(args[ac], XmNwidth, 1014);
	ac++;
	XtSetArg(args[ac], XmNheight, 663);
	ac++;
	window_mbedit = XmCreateMainWindow(parent, (char *)"window_mbedit", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNnoResize, False);
	ac++;
	XtSetArg(args[ac], XmNmarginHeight, 0);
	ac++;
	XtSetArg(args[ac], XmNmarginWidth, 0);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNwidth, 1014);
	ac++;
	XtSetArg(args[ac], XmNheight, 663);
	ac++;
	Widget mbedit_bboard = XmCreateBulletinBoard(window_mbedit, (char *)"mbedit_bboard", args, ac);
	XtManageChild(mbedit_bboard);

	ac = 0;
	XtSetArg(args[ac], XmNmarginHeight, 0);
	ac++;
	XtSetArg(args[ac], XmNmarginWidth, 0);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 1040);
	ac++;
	XtSetArg(args[ac], XmNheight, 154);
	ac++;
	Widget controls_mbedit = XmCreateBulletinBoard(mbedit_bboard, (char *)"controls_mbedit", args, ac);
	XtManageChild(controls_mbedit);

	ac = 0;
	XtSetArg(args[ac], XmNpacking, XmPACK_TIGHT);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 51);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	Widget menuBar_file = XmCreateMenuBar(controls_mbedit, (char *)"menuBar_file", args, ac);
	XtManageChild(menuBar_file);

	ac = 0;
	Widget cascadeButton_file;
	{
		XmString tmp0 = (XmString)BX_CONVERT(menuBar_file, (char *)"File", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 5);
		ac++;
		XtSetArg(args[ac], XmNy, 5);
		ac++;
		XtSetArg(args[ac], XmNwidth, 41);
		ac++;
		XtSetArg(args[ac], XmNheight, 24);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(menuBar_file, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		cascadeButton_file = XmCreateCascadeButton(menuBar_file, (char *)"cascadeButton_file", args, ac);
		XtManageChild(cascadeButton_file);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 135);
	ac++;
	XtSetArg(args[ac], XmNheight, 54);
	ac++;
	Widget pulldownMenu_file = XmCreatePulldownMenu(XtParent(cascadeButton_file), (char *)"pulldownMenu_file", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_file, (char *)"Open", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_file, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_file = XmCreatePushButton(pulldownMenu_file, (char *)"pushButton_file", args, ac);
		XtManageChild(pushButton_file);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_file, XmNactivateCallback, BxManageCB, (XtPointer) "bulletinBoard_file");

	ac = 0;
	Widget separator10 = XmCreateSeparator(pulldownMenu_file, (char *)"separator10", args, ac);
	XtManageChild(separator10);

	ac = 0;
	Widget pushButton_filelist;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_file, (char *)"File Selection List", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_file, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_filelist = XmCreatePushButton(pulldownMenu_file, (char *)"pushButton_filelist", args, ac);
		XtManageChild(pushButton_filelist);

		XmStringFree(tmp0);
	}

	Widget form_filelist;
	XtAddCallback(pushButton_filelist, XmNactivateCallback, BxManageCB, (XtPointer) "form_filelist");

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, pulldownMenu_file);
	ac++;
	XtSetValues(cascadeButton_file, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"End", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 510);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_end = XmCreatePushButton(controls_mbedit, (char *)"pushButton_end", args, ac);
		XtManageChild(pushButton_end);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_end, XmNactivateCallback, do_end, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Start", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 300);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_start = XmCreatePushButton(controls_mbedit, (char *)"pushButton_start", args, ac);
		XtManageChild(pushButton_start);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_start, XmNactivateCallback, do_start, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Flag View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 550);
		ac++;
		XtSetArg(args[ac], XmNy, 120);
		ac++;
		XtSetArg(args[ac], XmNwidth, 120);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget pushButton_flag_view = XmCreatePushButton(controls_mbedit, (char *)"pushButton_flag_view", args, ac);
		XtManageChild(pushButton_flag_view);
		XtAddCallback(pushButton_flag_view, XmNactivateCallback, do_flag_view, (XtPointer)0);

		XmStringFree(tmp0);
	}


	ac = 0;
	XtSetArg(args[ac], XmNpacking, XmPACK_TIGHT);
	ac++;
	XtSetArg(args[ac], XmNx, 90);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 58);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	Widget menuBar_view = XmCreateMenuBar(controls_mbedit, (char *)"menuBar_view", args, ac);
	XtManageChild(menuBar_view);

	ac = 0;
	Widget cascadeButton_view;
	{
		XmString tmp0 = (XmString)BX_CONVERT(menuBar_view, (char *)"View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 5);
		ac++;
		XtSetArg(args[ac], XmNy, 5);
		ac++;
		XtSetArg(args[ac], XmNwidth, 48);
		ac++;
		XtSetArg(args[ac], XmNheight, 24);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(menuBar_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		cascadeButton_view = XmCreateCascadeButton(menuBar_view, (char *)"cascadeButton_view", args, ac);
		XtManageChild(cascadeButton_view);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 247);
	ac++;
	XtSetArg(args[ac], XmNheight, 490);
	ac++;
	Widget pulldownMenu_view = XmCreatePulldownMenu(XtParent(cascadeButton_view), (char *)"pulldownMenu_view", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Waterfall View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_view_waterfall = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_view_waterfall", args, ac);
		XtManageChild(toggleButton_view_waterfall);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_view_waterfall, XmNvalueChangedCallback, do_view_mode, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Alongtrack View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_view_alongtrack = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_view_alongtrack", args, ac);
		XtManageChild(toggleButton_view_alongtrack);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_view_alongtrack, XmNvalueChangedCallback, do_view_mode, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Acrosstrack View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_view_acrosstrack =
		    XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_view_acrosstrack", args, ac);
		XtManageChild(toggleButton_view_acrosstrack);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_view_acrosstrack, XmNvalueChangedCallback, do_view_mode, (XtPointer)0);

	ac = 0;
	Widget separator2 = XmCreateSeparator(pulldownMenu_view, (char *)"separator2", args, ac);
	XtManageChild(separator2);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Show Flagged Soundings", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_flaggedsoundings_on = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_flaggedsoundings_on", args, ac);
		XtManageChild(toggleButton_show_flaggedsoundings_on);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_flaggedsoundings_on, XmNvalueChangedCallback, do_show_flaggedsoundings, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Show Flagged Profile", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_flaggedprofiles_on = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_flaggedprofiles_on", args, ac);
		XtManageChild(toggleButton_show_flaggedprofiles_on);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_flaggedprofiles_on, XmNvalueChangedCallback, do_show_flaggedprofiles, (XtPointer)0);

	ac = 0;
	Widget separator8 = XmCreateSeparator(pulldownMenu_view, (char *)"separator8", args, ac);
	XtManageChild(separator8);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Show Flag States", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_flags = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_flags", args, ac);
		XtManageChild(toggleButton_show_flags);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_flags, XmNvalueChangedCallback, do_show_flags, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Show Bottom Detect Algorithms", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_detects = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_detects", args, ac);
		XtManageChild(toggleButton_show_detects);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_detects, XmNvalueChangedCallback, do_show_detects, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Show Source Pulse Types", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_pulsetypes = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_pulsetypes", args, ac);
		XtManageChild(toggleButton_show_pulsetypes);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_pulsetypes, XmNvalueChangedCallback, do_show_pulsetypes, (XtPointer)0);

	ac = 0;
	Widget separator9 = XmCreateSeparator(pulldownMenu_view, (char *)"separator9", args, ac);
	XtManageChild(separator9);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Wide Bathymetry Profiles", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_wideplot = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_wideplot", args, ac);
		XtManageChild(toggleButton_show_wideplot);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_wideplot, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Print Time Stamps", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_time = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_time", args, ac);
		XtManageChild(toggleButton_show_time);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_time, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Ping Interval", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_interval = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_interval", args, ac);
		XtManageChild(toggleButton_show_interval);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_interval, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Longitude", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_lon = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_lon", args, ac);
		XtManageChild(toggleButton_show_lon);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_lon, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Latitude", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_latitude = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_latitude", args, ac);
		XtManageChild(toggleButton_show_latitude);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_latitude, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Heading", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_heading = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_heading", args, ac);
		XtManageChild(toggleButton_show_heading);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_heading, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Speed", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_speed = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_speed", args, ac);
		XtManageChild(toggleButton_show_speed);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_speed, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Center Beam Depth", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_depth = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_depth", args, ac);
		XtManageChild(toggleButton_show_depth);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_depth, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Sonar Altitude", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_altitude = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_altitude", args, ac);
		XtManageChild(toggleButton_show_altitude);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_altitude, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Sonar Depth", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_sensordepth = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_sensordepth", args, ac);
		XtManageChild(toggleButton_show_sensordepth);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_sensordepth, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Roll", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_roll = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_roll", args, ac);
		XtManageChild(toggleButton_show_roll);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_roll, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Pitch", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_pitch = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_pitch", args, ac);
		XtManageChild(toggleButton_show_pitch);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_pitch, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_view, (char *)"Plot Heave", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_show_heave = XmCreateToggleButton(pulldownMenu_view, (char *)"toggleButton_show_heave", args, ac);
		XtManageChild(toggleButton_show_heave);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_show_heave, XmNvalueChangedCallback, do_show_time, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, pulldownMenu_view);
	ac++;
	XtSetValues(cascadeButton_view, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Unflag Forward", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 850);
		ac++;
		XtSetArg(args[ac], XmNy, 120);
		ac++;
		XtSetArg(args[ac], XmNwidth, 120);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget pushButton_unflag_all = XmCreatePushButton(controls_mbedit, (char *)"pushButton_unflag_all", args, ac);
		XtManageChild(pushButton_unflag_all);
		XtAddCallback(pushButton_unflag_all, XmNactivateCallback, do_unflag_all, (XtPointer)0);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Unflag View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 700);
		ac++;
		XtSetArg(args[ac], XmNy, 120);
		ac++;
		XtSetArg(args[ac], XmNwidth, 120);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget pushButton_unflag_view = XmCreatePushButton(controls_mbedit, (char *)"pushButton_unflag_view", args, ac);
		XtManageChild(pushButton_unflag_view);
		XtAddCallback(pushButton_unflag_view, XmNactivateCallback, do_unflag_view, (XtPointer)0);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 170);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 87);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	Widget menuBar_controls = XmCreateMenuBar(controls_mbedit, (char *)"menuBar_controls", args, ac);
	XtManageChild(menuBar_controls);

	ac = 0;
	Widget cascadeButton_controls;
	{
		XmString tmp0 = (XmString)BX_CONVERT(menuBar_controls, (char *)"Controls", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 5);
		ac++;
		XtSetArg(args[ac], XmNy, 5);
		ac++;
		XtSetArg(args[ac], XmNwidth, 77);
		ac++;
		XtSetArg(args[ac], XmNheight, 24);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(menuBar_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		cascadeButton_controls = XmCreateCascadeButton(menuBar_controls, (char *)"cascadeButton_controls", args, ac);
		XtManageChild(cascadeButton_controls);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 240);
	ac++;
	XtSetArg(args[ac], XmNheight, 150);
	ac++;
	Widget pulldownMenu_controls = XmCreatePulldownMenu(XtParent(cascadeButton_controls), (char *)"pulldownMenu_controls", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Go To Specified Time...", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget pushButton_goto = XmCreatePushButton(pulldownMenu_controls, (char *)"pushButton_goto", args, ac);
		XtManageChild(pushButton_goto);
		XtAddCallback(pushButton_goto, XmNactivateCallback, BxManageCB, (XtPointer) "bulletinBoard_goto");

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Buffer Controls...", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget pushButton_buffer = XmCreatePushButton(pulldownMenu_controls, (char *)"pushButton_buffer", args, ac);
		XtManageChild(pushButton_buffer);
		XtAddCallback(pushButton_buffer, XmNactivateCallback, BxManageCB, (XtPointer) "bulletinBoard_buffer");

		XmStringFree(tmp0);
	}

	ac = 0;
	Widget bulletinBoard_annotation;  // WARNING: Used again way later.
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Annotation...", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget pushButton_annotation = XmCreatePushButton(pulldownMenu_controls, (char *)"pushButton_annotation", args, ac);
		XtManageChild(pushButton_annotation);
		XtAddCallback(pushButton_annotation, XmNactivateCallback, BxManageCB, (XtPointer) "bulletinBoard_annotation");

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Filters...", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget pushButton_filters =
			XmCreatePushButton(pulldownMenu_controls, (char *)"pushButton_filters", args, ac);
		XtManageChild(pushButton_filters);
		XtAddCallback(pushButton_filters, XmNactivateCallback, BxManageCB, (XtPointer) "bulletinBoard_filters");

		XmStringFree(tmp0);
	}


	ac = 0;
	Widget separator7 = XmCreateSeparator(pulldownMenu_controls, (char *)"separator7", args, ac);
	XtManageChild(separator7);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Reverse Right/Left Key Macros", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		toggleButton_reverse_keys = XmCreateToggleButton(pulldownMenu_controls, (char *)"toggleButton_reverse_keys", args, ac);
		XtManageChild(toggleButton_reverse_keys);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_reverse_keys, XmNvalueChangedCallback, do_reverse_keys, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Reverse Mouse Buttons", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		toggleButton_reverse_mouse = XmCreateToggleButton(pulldownMenu_controls, (char *)"toggleButton_reverse_mouse", args, ac);
		XtManageChild(toggleButton_reverse_mouse);

		XmStringFree(tmp0);
	}

	XtAddCallback(toggleButton_reverse_mouse, XmNvalueChangedCallback, do_reverse_mouse, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, pulldownMenu_controls);
	ac++;
	XtSetValues(cascadeButton_controls, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"About", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 930);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget pushButton_about = XmCreatePushButton(controls_mbedit, (char *)"pushButton_about", args, ac);
		XtManageChild(pushButton_about);
		XtAddCallback(pushButton_about, XmNactivateCallback, BxManageCB, (XtPointer) "bulletinBoard_about");

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Next Buffer", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 620);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_next = XmCreatePushButton(controls_mbedit, (char *)"pushButton_next", args, ac);
		XtManageChild(pushButton_next);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_next, XmNactivateCallback, do_next_buffer, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Done", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 720);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_done = XmCreatePushButton(controls_mbedit, (char *)"pushButton_done", args, ac);
		XtManageChild(pushButton_done);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_done, XmNactivateCallback, do_done, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Forward", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 440);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_forward = XmCreatePushButton(controls_mbedit, (char *)"pushButton_forward", args, ac);
		XtManageChild(pushButton_forward);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_forward, XmNactivateCallback, do_forward, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Reverse", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 370);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_reverse = XmCreatePushButton(controls_mbedit, (char *)"pushButton_reverse", args, ac);
		XtManageChild(pushButton_reverse);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_reverse, XmNactivateCallback, do_reverse, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Quit", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 810);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget pushButton_quit =
			XmCreatePushButton(controls_mbedit, (char *)"pushButton_quit", args, ac);
		XtManageChild(pushButton_quit);
		XtAddCallback(pushButton_quit, XmNactivateCallback, do_quit, (XtPointer)0);

		XmStringFree(tmp0);
	}


	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Acrosstrack Width (m):  1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 30);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_scale_x_label = XmCreateLabel(controls_mbedit, (char *)"slider_scale_x_label", args, ac);
		XtManageChild(slider_scale_x_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 1000);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 20000);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 210);
	ac++;
	XtSetArg(args[ac], XmNy, 40);
	ac++;
	XtSetArg(args[ac], XmNwidth, 260);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	slider_scale_x = XmCreateScale(controls_mbedit, (char *)"slider_scale_x", args, ac);
	XtManageChild(slider_scale_x);
	XtAddCallback(slider_scale_x, XmNvalueChangedCallback, do_scale_x, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"20000", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 470);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 60);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_scale_x_max_label = XmCreateLabel(controls_mbedit, (char *)"slider_scale_x_max_label", args, ac);
		XtManageChild(slider_scale_x_max_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Vertical Exaggeration: 0.01", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_scale_y_label = XmCreateLabel(controls_mbedit, (char *)"slider_scale_y_label", args, ac);
		XtManageChild(slider_scale_y_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNdecimalPoints, 2);
	ac++;
	XtSetArg(args[ac], XmNvalue, 100);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 2000);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 210);
	ac++;
	XtSetArg(args[ac], XmNy, 80);
	ac++;
	XtSetArg(args[ac], XmNwidth, 260);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	slider_scale_y = XmCreateScale(controls_mbedit, (char *)"slider_scale_y", args, ac);
	XtManageChild(slider_scale_y);
	XtAddCallback(slider_scale_y, XmNvalueChangedCallback, do_scale_y, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"20.00", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 470);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNwidth, 65);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_scale_y_max_label = XmCreateLabel(controls_mbedit, (char *)"slider_scale_y_max_label", args, ac);
		XtManageChild(slider_scale_y_max_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Pings shown:   1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 550);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_number_pings_label = XmCreateLabel(controls_mbedit, (char *)"slider_number_pings_label", args, ac);
		XtManageChild(slider_number_pings_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 10);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 20);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 670);
	ac++;
	XtSetArg(args[ac], XmNy, 40);
	ac++;
	XtSetArg(args[ac], XmNwidth, 290);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	slider_number_pings = XmCreateScale(controls_mbedit, (char *)"slider_number_pings", args, ac);
	XtManageChild(slider_number_pings);
	XtAddCallback(slider_number_pings, XmNvalueChangedCallback, do_number_pings, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"20", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 960);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 50);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_num_pings_max_label = XmCreateLabel(controls_mbedit, (char *)"slider_num_pings_max_label", args, ac);
		XtManageChild(slider_num_pings_max_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Pings to step:  1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 550);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_number_step_label = XmCreateLabel(controls_mbedit, (char *)"slider_number_step_label", args, ac);
		XtManageChild(slider_number_step_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 5);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 20);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 670);
	ac++;
	XtSetArg(args[ac], XmNy, 80);
	ac++;
	XtSetArg(args[ac], XmNwidth, 290);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	slider_number_step = XmCreateScale(controls_mbedit, (char *)"slider_number_step", args, ac);
	XtManageChild(slider_number_step);
	XtAddCallback(slider_number_step, XmNvalueChangedCallback, do_number_step, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"20", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 960);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNwidth, 50);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_number_max_step_label = XmCreateLabel(controls_mbedit, (char *)"slider_number_max_step_label", args, ac);
		XtManageChild(slider_number_max_step_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(controls_mbedit, (char *)"Mode:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNmarginWidth, 0);
		ac++;
		XtSetArg(args[ac], XmNx, 70);
		ac++;
		XtSetArg(args[ac], XmNy, 120);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(controls_mbedit, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget setting_mode_label = XmCreateLabel(controls_mbedit, (char *)"setting_mode_label", args, ac);
		XtManageChild(setting_mode_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNnumColumns, 1);
	ac++;
	XtSetArg(args[ac], XmNpacking, XmPACK_TIGHT);
	ac++;
	XtSetArg(args[ac], XmNradioBehavior, True);
	ac++;
	XtSetArg(args[ac], XmNspacing, 0);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 120);
	ac++;
	XtSetArg(args[ac], XmNy, 120);
	ac++;
	XtSetArg(args[ac], XmNwidth, 405);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	Widget setting_mode = XmCreateRowColumn(controls_mbedit, (char *)"setting_mode", args, ac);
	XtManageChild(setting_mode);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(setting_mode, (char *)"Toggle", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 75);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		setting_mode_toggle_toggle = XmCreateToggleButton(setting_mode, (char *)"setting_mode_toggle_toggle", args, ac);
		XtManageChild(setting_mode_toggle_toggle);

		XmStringFree(tmp0);
	}

	XtAddCallback(setting_mode_toggle_toggle, XmNvalueChangedCallback, do_mode_toggle, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(setting_mode, (char *)"Pick", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		setting_mode_toggle_pick = XmCreateToggleButton(setting_mode, (char *)"setting_mode_toggle_pick", args, ac);
		XtManageChild(setting_mode_toggle_pick);

		XmStringFree(tmp0);
	}

	XtAddCallback(setting_mode_toggle_pick, XmNvalueChangedCallback, do_mode_pick, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(setting_mode, (char *)"Erase", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		setting_mode_toggle_erase = XmCreateToggleButton(setting_mode, (char *)"setting_mode_toggle_erase", args, ac);
		XtManageChild(setting_mode_toggle_erase);

		XmStringFree(tmp0);
	}

	XtAddCallback(setting_mode_toggle_erase, XmNvalueChangedCallback, do_mode_erase, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(setting_mode, (char *)"Restore", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		setting_mode_toggle_restore = XmCreateToggleButton(setting_mode, (char *)"setting_mode_toggle_restore", args, ac);
		XtManageChild(setting_mode_toggle_restore);

		XmStringFree(tmp0);
	}

	XtAddCallback(setting_mode_toggle_restore, XmNvalueChangedCallback, do_mode_restore, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(setting_mode, (char *)"Grab", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 62);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		setting_mode_toggle_grab = XmCreateToggleButton(setting_mode, (char *)"setting_mode_toggle_grab", args, ac);
		XtManageChild(setting_mode_toggle_grab);

		XmStringFree(tmp0);
	}

	XtAddCallback(setting_mode_toggle_grab, XmNvalueChangedCallback, do_mode_grab, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(setting_mode, (char *)"Info", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 54);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		setting_mode_toggle_info = XmCreateToggleButton(setting_mode, (char *)"setting_mode_toggle_info", args, ac);
		XtManageChild(setting_mode_toggle_info);

		XmStringFree(tmp0);
	}

	XtAddCallback(setting_mode_toggle_info, XmNvalueChangedCallback, do_mode_info, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNborderWidth, 1);
	ac++;
	XtSetArg(args[ac], XmNbackground, BX_CONVERT(mbedit_bboard, (char *)"white", XmRPixel, 0, &argok));
	if (argok)
		ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 150);
	ac++;
	XtSetArg(args[ac], XmNwidth, 1010);
	ac++;
	XtSetArg(args[ac], XmNheight, 510);
	ac++;
	canvas_mbedit = XmCreateDrawingArea(mbedit_bboard, (char *)"canvas_mbedit", args, ac);
	XtManageChild(canvas_mbedit);
	XtAddCallback(canvas_mbedit, XmNinputCallback, do_event, (XtPointer)0);
	XtAddCallback(canvas_mbedit, XmNexposeCallback, do_expose, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Open Source Swath Sonar Data File");
	ac++;
	XtSetArg(args[ac], XmNdeleteResponse, XmUNMAP);
	ac++;
	XtSetArg(args[ac], XmNwidth, 529);
	ac++;
	XtSetArg(args[ac], XmNheight, 489);
	ac++;
	Widget xmDialogShell_file = XmCreateDialogShell(window_mbedit, (char *)"xmDialogShell_file", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNautoUnmanage, False);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 1016);
	ac++;
	XtSetArg(args[ac], XmNy, 1092);
	ac++;
	XtSetArg(args[ac], XmNwidth, 529);
	ac++;
	XtSetArg(args[ac], XmNheight, 489);
	ac++;
	Widget bulletinBoard_file = XtCreateWidget((char *)"bulletinBoard_file", xmBulletinBoardWidgetClass, xmDialogShell_file, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNmarginHeight, 0);
	ac++;
	XtSetArg(args[ac], XmNmarginWidth, 0);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNwidth, 500);
	ac++;
	XtSetArg(args[ac], XmNheight, 400);
	ac++;
	fileSelectionBox = XmCreateFileSelectionBox(bulletinBoard_file, (char *)"fileSelectionBox", args, ac);
	XtManageChild(fileSelectionBox);
	XtAddCallback(fileSelectionBox, XmNokCallback, do_load_check, (XtPointer)0);
	XtAddCallback(fileSelectionBox, XmNokCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_file");
	XtAddCallback(fileSelectionBox, XmNcancelCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_file");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_file, (char *)"MBIO Format ID:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 430);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_file, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		textfield_format_label = XmCreateLabel(bulletinBoard_file, (char *)"textfield_format_label", args, ac);
		XtManageChild(textfield_format_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNvalue, "41");
	ac++;
	XtSetArg(args[ac], XmNcolumns, 3);
	ac++;
	XtSetArg(args[ac], XmNmaxLength, 3);
	ac++;
	XtSetArg(args[ac], XmNx, 140);
	ac++;
	XtSetArg(args[ac], XmNy, 420);
	ac++;
	XtSetArg(args[ac], XmNheight, 40);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(bulletinBoard_file, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textfield_format = XmCreateTextField(bulletinBoard_file, (char *)"textfield_format", args, ac);
	XtManageChild(textfield_format);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_file, (char *)"Output Mode:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNmarginWidth, 0);
		ac++;
		XtSetArg(args[ac], XmNx, 210);
		ac++;
		XtSetArg(args[ac], XmNy, 430);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_file, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		setting_output_label = XmCreateLabel(bulletinBoard_file, (char *)"setting_output_label", args, ac);
		XtManageChild(setting_output_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNnumColumns, 1);
	ac++;
	XtSetArg(args[ac], XmNpacking, XmPACK_COLUMN);
	ac++;
	XtSetArg(args[ac], XmNradioBehavior, True);
	ac++;
	XtSetArg(args[ac], XmNspacing, 0);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmVERTICAL);
	ac++;
	XtSetArg(args[ac], XmNx, 310);
	ac++;
	XtSetArg(args[ac], XmNy, 420);
	ac++;
	XtSetArg(args[ac], XmNwidth, 122);
	ac++;
	XtSetArg(args[ac], XmNheight, 62);
	ac++;
	setting_output = XmCreateRowColumn(bulletinBoard_file, (char *)"setting_output", args, ac);
	XtManageChild(setting_output);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(setting_output, (char *)"Output Edits", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 116);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(setting_output, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		setting_output_toggle_edit = XmCreateToggleButton(setting_output, (char *)"setting_output_toggle_edit", args, ac);
		XtManageChild(setting_output_toggle_edit);

		XmStringFree(tmp0);
	}

	XtAddCallback(setting_output_toggle_edit, XmNvalueChangedCallback, do_output_edit, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(setting_output, (char *)"Browse Only", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(setting_output, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		setting_output_toggle_browse = XmCreateToggleButton(setting_output, (char *)"setting_output_toggle_browse", args, ac);
		XtManageChild(setting_output_toggle_browse);

		XmStringFree(tmp0);
	}

	XtAddCallback(setting_output_toggle_browse, XmNvalueChangedCallback, do_output_browse, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Go To Specified Time");
	ac++;
	XtSetArg(args[ac], XmNdeleteResponse, XmUNMAP);
	ac++;
	XtSetArg(args[ac], XmNwidth, 196);
	ac++;
	XtSetArg(args[ac], XmNheight, 346);
	ac++;
	Widget xmDialogShell_goto = XmCreateDialogShell(window_mbedit, (char *)"xmDialogShell_goto", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 1164);
	ac++;
	XtSetArg(args[ac], XmNwidth, 196);
	ac++;
	XtSetArg(args[ac], XmNheight, 346);
	ac++;
	Widget bulletinBoard_goto = XtCreateWidget((char *)"bulletinBoard_goto", xmBulletinBoardWidgetClass, xmDialogShell_goto, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNvalue, "1");
	ac++;
	XtSetArg(args[ac], XmNcolumns, 2);
	ac++;
	XtSetArg(args[ac], XmNx, 90);
	ac++;
	XtSetArg(args[ac], XmNy, 90);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textfield_day = XmCreateTextField(bulletinBoard_goto, (char *)"textfield_day", args, ac);
	XtManageChild(textfield_day);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_goto, (char *)"Cancel", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 270);
		ac++;
		XtSetArg(args[ac], XmNwidth, 75);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget button_goto_cancel = XmCreatePushButton(bulletinBoard_goto, (char *)"button_goto_cancel", args, ac);
		XtManageChild(button_goto_cancel);
		XtAddCallback(button_goto_cancel, XmNactivateCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_goto");

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_goto, (char *)"Apply", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 270);
		ac++;
		XtSetArg(args[ac], XmNwidth, 75);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget button_goto_apply = XmCreatePushButton(bulletinBoard_goto, (char *)"button_goto_apply", args, ac);
		XtManageChild(button_goto_apply);
		XtAddCallback(button_goto_apply, XmNactivateCallback, do_goto_apply, (XtPointer)0);
		XtAddCallback(button_goto_apply, XmNactivateCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_goto");

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNvalue, "0");
	ac++;
	XtSetArg(args[ac], XmNcolumns, 2);
	ac++;
	XtSetArg(args[ac], XmNx, 90);
	ac++;
	XtSetArg(args[ac], XmNy, 210);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textfield_second = XmCreateTextField(bulletinBoard_goto, (char *)"textfield_second", args, ac);
	XtManageChild(textfield_second);

	ac = 0;
	XtSetArg(args[ac], XmNvalue, "1");
	ac++;
	XtSetArg(args[ac], XmNcolumns, 2);
	ac++;
	XtSetArg(args[ac], XmNx, 90);
	ac++;
	XtSetArg(args[ac], XmNy, 170);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textfield_minute = XmCreateTextField(bulletinBoard_goto, (char *)"textfield_minute", args, ac);
	XtManageChild(textfield_minute);

	ac = 0;
	XtSetArg(args[ac], XmNvalue, "1");
	ac++;
	XtSetArg(args[ac], XmNcolumns, 2);
	ac++;
	XtSetArg(args[ac], XmNx, 90);
	ac++;
	XtSetArg(args[ac], XmNy, 130);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textfield_hour = XmCreateTextField(bulletinBoard_goto, (char *)"textfield_hour", args, ac);
	XtManageChild(textfield_hour);

	ac = 0;
	XtSetArg(args[ac], XmNvalue, "1");
	ac++;
	XtSetArg(args[ac], XmNcolumns, 2);
	ac++;
	XtSetArg(args[ac], XmNx, 90);
	ac++;
	XtSetArg(args[ac], XmNy, 50);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textfield_month = XmCreateTextField(bulletinBoard_goto, (char *)"textfield_month", args, ac);
	XtManageChild(textfield_month);

	ac = 0;
	XtSetArg(args[ac], XmNvalue, "1994");
	ac++;
	XtSetArg(args[ac], XmNcolumns, 4);
	ac++;
	XtSetArg(args[ac], XmNx, 90);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textfield_year = XmCreateTextField(bulletinBoard_goto, (char *)"textfield_year", args, ac);
	XtManageChild(textfield_year);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_goto, (char *)"Second:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 210);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget textfield_second_label = XmCreateLabel(bulletinBoard_goto, (char *)"textfield_second_label", args, ac);
		XtManageChild(textfield_second_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_goto, (char *)"Minute:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 170);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget textfield_minute_label = XmCreateLabel(bulletinBoard_goto, (char *)"textfield_minute_label", args, ac);
		XtManageChild(textfield_minute_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_goto, (char *)"Hour:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 30);
		ac++;
		XtSetArg(args[ac], XmNy, 130);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget textfield_hour_label = XmCreateLabel(bulletinBoard_goto, (char *)"textfield_hour_label", args, ac);
		XtManageChild(textfield_hour_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_goto, (char *)"Day:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 40);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNwidth, 40);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget textfield_day_label = XmCreateLabel(bulletinBoard_goto, (char *)"textfield_day_label", args, ac);
		XtManageChild(textfield_day_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_goto, (char *)"Month:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 30);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget textfield_month_label = XmCreateLabel(bulletinBoard_goto, (char *)"textfield_month_label", args, ac);
		XtManageChild(textfield_month_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_goto, (char *)"Year:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 40);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_goto, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget textfield_year_label = XmCreateLabel(bulletinBoard_goto, (char *)"textfield_year_label", args, ac);
		XtManageChild(textfield_year_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "About MBedit");
	ac++;
	XtSetArg(args[ac], XmNdeleteResponse, XmUNMAP);
	ac++;
	XtSetArg(args[ac], XmNwidth, 473);
	ac++;
	XtSetArg(args[ac], XmNheight, 501);
	ac++;
	Widget xmDialogShell_about = XmCreateDialogShell(window_mbedit, (char *)"xmDialogShell_about", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 1086);
	ac++;
	XtSetArg(args[ac], XmNwidth, 473);
	ac++;
	XtSetArg(args[ac], XmNheight, 501);
	ac++;
	Widget bulletinBoard_about =
	    XtCreateWidget((char *)"bulletinBoard_about", xmBulletinBoardWidgetClass, xmDialogShell_about, args, ac);

	ac = 0;
	Widget label_about_create1;
	{
		XmString tmp0 =
		    (XmString)BX_CONVERT(bulletinBoard_about, (char *)"David W. Caress    and    Dale N. Chayes", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-140-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 260);
		ac++;
		XtSetArg(args[ac], XmNwidth, 450);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		label_about_create1 = XmCreateLabel(bulletinBoard_about, (char *)"label_about_create1", args, ac);
		XtManageChild(label_about_create1);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 220);
	ac++;
	XtSetArg(args[ac], XmNwidth, 450);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	Widget separator1 = XmCreateSeparator(bulletinBoard_about, (char *)"separator1", args, ac);
	XtManageChild(separator1);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-140-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 190);
		ac++;
		XtSetArg(args[ac], XmNy, 410);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 35);
		ac++;
		Widget pushButton_about_dismiss = XmCreatePushButton(bulletinBoard_about, (char *)"pushButton_about_dismiss", args, ac);
		XtManageChild(pushButton_about_dismiss);
		XtAddCallback(pushButton_about_dismiss, XmNactivateCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_about");

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)"Created by:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-140-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 240);
		ac++;
		XtSetArg(args[ac], XmNwidth, 450);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		Widget label_about_create = XmCreateLabel(bulletinBoard_about, (char *)"label_about_create", args, ac);
		XtManageChild(label_about_create);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about,
		                            (char *)":::t\"Lamont-Doherty\":t\"Earth Observatory\"\"of Columbia University\"",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-120-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 250);
		ac++;
		XtSetArg(args[ac], XmNy, 280);
		ac++;
		XtSetArg(args[ac], XmNwidth, 190);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		Widget label_about_lamont = XmCreateLabel(bulletinBoard_about, (char *)"label_about_lamont", args, ac);
		XtManageChild(label_about_lamont);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)":::t\"Monterey Bay\":t\"Aquarium\"\"Research Institute\"",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-120-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 60);
		ac++;
		XtSetArg(args[ac], XmNy, 280);
		ac++;
		XtSetArg(args[ac], XmNwidth, 160);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		Widget label_about_columbia = XmCreateLabel(bulletinBoard_about, (char *)"label_about_columbia", args, ac);
		XtManageChild(label_about_columbia);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)"MB-System", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-240-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 120);
		ac++;
		XtSetArg(args[ac], XmNwidth, 450);
		ac++;
		XtSetArg(args[ac], XmNheight, 38);
		ac++;
		Widget label_about_mbsystem = XmCreateLabel(bulletinBoard_about, (char *)"label_about_mbsystem", args, ac);
		XtManageChild(label_about_mbsystem);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)"An Open Source Software Package", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-140-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 160);
		ac++;
		XtSetArg(args[ac], XmNwidth, 450);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		Widget label_about_mbpub = XmCreateLabel(bulletinBoard_about, (char *)"label_about_mbpub", args, ac);
		XtManageChild(label_about_mbpub);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)"One Component of the", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-140-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNwidth, 450);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		Widget label_about_component = XmCreateLabel(bulletinBoard_about, (char *)"label_about_component", args, ac);
		XtManageChild(label_about_component);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)"for Processing and Display of Swath Sonar Data", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-140-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 190);
		ac++;
		XtSetArg(args[ac], XmNwidth, 450);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		Widget label_about_for = XmCreateLabel(bulletinBoard_about, (char *)"label_about_for", args, ac);
		XtManageChild(label_about_for);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 70);
	ac++;
	XtSetArg(args[ac], XmNwidth, 450);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	Widget separator = XmCreateSeparator(bulletinBoard_about, (char *)"separator", args, ac);
	XtManageChild(separator);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)":::t\"MB-System Release 4.6\"\"April 14, 1999\"", XmRXmString,
		                            0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-medium-r-*-*-*-140-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 340);
		ac++;
		XtSetArg(args[ac], XmNwidth, 450);
		ac++;
		XtSetArg(args[ac], XmNheight, 70);
		ac++;
		label_about_version = XmCreateLabel(bulletinBoard_about, (char *)"label_about_version", args, ac);
		XtManageChild(label_about_version);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)"Interactive Swath Bathymetry Editor", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-180-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 40);
		ac++;
		XtSetArg(args[ac], XmNwidth, 450);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		Widget label_about_function = XmCreateLabel(bulletinBoard_about, (char *)"label_about_function", args, ac);
		XtManageChild(label_about_function);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)"MBedit", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_about, (char *)"-*-" SERIF "-bold-r-*-*-*-240-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 450);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		Widget label_about_mbedit = XmCreateLabel(bulletinBoard_about, (char *)"label_about_mbedit", args, ac);
		XtManageChild(label_about_mbedit);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Please Wait...");
	ac++;
	XtSetArg(args[ac], XmNmwmInputMode, MWM_INPUT_MODELESS);
	ac++;
	XtSetArg(args[ac], XmNdeleteResponse, XmUNMAP);
	ac++;
	XtSetArg(args[ac], XmNwidth, 496);
	ac++;
	XtSetArg(args[ac], XmNheight, 112);
	ac++;
	Widget xmDialogShell_message = XmCreateDialogShell(window_mbedit, (char *)"xmDialogShell_message", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_NONE);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 1281);
	ac++;
	XtSetArg(args[ac], XmNwidth, 496);
	ac++;
	XtSetArg(args[ac], XmNheight, 112);
	ac++;
	bulletinBoard_message =
	    XtCreateWidget((char *)"bulletinBoard_message", xmBulletinBoardWidgetClass, xmDialogShell_message, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_message, (char *)"Thank you for your patience.", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 40);
		ac++;
		XtSetArg(args[ac], XmNwidth, 360);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_message, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget label_pleasewait = XmCreateLabel(bulletinBoard_message, (char *)"label_pleasewait", args, ac);
		XtManageChild(label_pleasewait);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_message, (char *)"MBedit is loading data...", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 480);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_message, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		label_message = XmCreateLabel(bulletinBoard_message, (char *)"label_message", args, ac);
		XtManageChild(label_message);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Use MBedit edit save file?");
	ac++;
	XtSetArg(args[ac], XmNdeleteResponse, XmUNMAP);
	ac++;
	XtSetArg(args[ac], XmNwidth, 431);
	ac++;
	XtSetArg(args[ac], XmNheight, 177);
	ac++;
	Widget xmDialogShell_editsave = XmCreateDialogShell(window_mbedit, (char *)"xmDialogShell_editsave", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 1248);
	ac++;
	XtSetArg(args[ac], XmNwidth, 431);
	ac++;
	XtSetArg(args[ac], XmNheight, 177);
	ac++;
	bulletinBoard_editsave =
	    XtCreateWidget((char *)"bulletinBoard_editsave", xmBulletinBoardWidgetClass, xmDialogShell_editsave, args, ac);

	ac = 0;
	{
		XmString tmp0 =
		    (XmString)BX_CONVERT(bulletinBoard_editsave, (char *)"An edit save file exists for the specified input data file...",
		                         XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 410);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_editsave, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget label_editsave_one = XmCreateLabel(bulletinBoard_editsave, (char *)"label_editsave_one", args, ac);
		XtManageChild(label_editsave_one);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_editsave, (char *)"Do you want to apply the saved edits to the data?",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 40);
		ac++;
		XtSetArg(args[ac], XmNwidth, 410);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_editsave, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget label_editsave_two = XmCreateLabel(bulletinBoard_editsave, (char *)"label_editsave_two", args, ac);
		XtManageChild(label_editsave_two);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_editsave, (char *)"No", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 250);
		ac++;
		XtSetArg(args[ac], XmNy, 80);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_editsave, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget pushButton_editsave_no = XmCreatePushButton(bulletinBoard_editsave, (char *)"pushButton_editsave_no", args, ac);
		XtManageChild(pushButton_editsave_no);
		XtAddCallback(pushButton_editsave_no, XmNactivateCallback, do_load_ok, (XtPointer)0);

		XmStringFree(tmp0);
	}

	ac = 0;
	Widget pushButton_editsave_yes;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_editsave, (char *)"Yes", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 80);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_editsave, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		pushButton_editsave_yes = XmCreatePushButton(bulletinBoard_editsave, (char *)"pushButton_editsave_yes", args, ac);
		XtManageChild(pushButton_editsave_yes);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_editsave_yes, XmNactivateCallback, do_load_ok_with_save, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Error");
	ac++;
	XtSetArg(args[ac], XmNwidth, 311);
	ac++;
	XtSetArg(args[ac], XmNheight, 205);
	ac++;
	Widget xmDialogShell_error = XmCreateDialogShell(window_mbedit, (char *)"xmDialogShell_error", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 1234);
	ac++;
	XtSetArg(args[ac], XmNwidth, 311);
	ac++;
	XtSetArg(args[ac], XmNheight, 205);
	ac++;
	bulletinBoard_error =
	    XtCreateWidget((char *)"bulletinBoard_error", xmBulletinBoardWidgetClass, xmDialogShell_error, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_error, (char *)"You probably do not have write", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 40);
		ac++;
		XtSetArg(args[ac], XmNwidth, 290);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_error, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		label_error_two = XmCreateLabel(bulletinBoard_error, (char *)"label_error_two", args, ac);
		XtManageChild(label_error_two);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_error, (char *)"Unable to open output file.", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 290);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_error, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		label_error_one = XmCreateLabel(bulletinBoard_error, (char *)"label_error_one", args, ac);
		XtManageChild(label_error_one);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_error, (char *)"permission in this directory!", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNwidth, 290);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_error, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		label_error_three = XmCreateLabel(bulletinBoard_error, (char *)"label_error_three", args, ac);
		XtManageChild(label_error_three);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_error, (char *)"OK", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 110);
		ac++;
		XtSetArg(args[ac], XmNy, 110);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_error, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget pushButton_error = XmCreatePushButton(bulletinBoard_error, (char *)"pushButton_error", args, ac);
		XtManageChild(pushButton_error);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Buffer Controls");
	ac++;
	XtSetArg(args[ac], XmNwidth, 536);
	ac++;
	XtSetArg(args[ac], XmNheight, 186);
	ac++;
	Widget dialogShell_buffer = XmCreateDialogShell(window_mbedit, (char *)"dialogShell_buffer", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 1244);
	ac++;
	XtSetArg(args[ac], XmNwidth, 536);
	ac++;
	XtSetArg(args[ac], XmNheight, 186);
	ac++;
	Widget bulletinBoard_buffer =
	    XtCreateWidget((char *)"bulletinBoard_buffer", xmBulletinBoardWidgetClass, dialogShell_buffer, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_buffer, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 230);
		ac++;
		XtSetArg(args[ac], XmNy, 110);
		ac++;
		XtSetArg(args[ac], XmNwidth, 67);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_buffer, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		Widget pushButton_buffer_dismiss = XmCreatePushButton(bulletinBoard_buffer, (char *)"pushButton_buffer_dismiss", args, ac);
		XtManageChild(pushButton_buffer_dismiss);
		XtAddCallback(pushButton_buffer_dismiss, XmNactivateCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_buffer");

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_buffer, (char *)"5000", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 460);
		ac++;
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNwidth, 65);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_buffer, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_buffer_hold_max_label = XmCreateLabel(bulletinBoard_buffer, (char *)"slider_buffer_hold_max_label", args, ac);
		XtManageChild(slider_buffer_hold_max_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 100);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 5000);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 190);
	ac++;
	XtSetArg(args[ac], XmNy, 60);
	ac++;
	XtSetArg(args[ac], XmNwidth, 260);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(
	    args[ac], XmNfontList,
	    BX_CONVERT(bulletinBoard_buffer, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	slider_buffer_hold = XmCreateScale(bulletinBoard_buffer, (char *)"slider_buffer_hold", args, ac);
	XtManageChild(slider_buffer_hold);
	XtAddCallback(slider_buffer_hold, XmNvalueChangedCallback, do_buffer_hold, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_buffer, (char *)"Buffer Retain Size:         1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_buffer, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_buffer_hold_label = XmCreateLabel(bulletinBoard_buffer, (char *)"slider_buffer_hold_label", args, ac);
		XtManageChild(slider_buffer_hold_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_buffer, (char *)"5000", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 460);
		ac++;
		XtSetArg(args[ac], XmNy, 20);
		ac++;
		XtSetArg(args[ac], XmNwidth, 65);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_buffer, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_buffer_size_max_label = XmCreateLabel(bulletinBoard_buffer, (char *)"slider_buffer_size_max_label", args, ac);
		XtManageChild(slider_buffer_size_max_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 5000);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 5000);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 190);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNwidth, 260);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(
	    args[ac], XmNfontList,
	    BX_CONVERT(bulletinBoard_buffer, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	slider_buffer_size = XmCreateScale(bulletinBoard_buffer, (char *)"slider_buffer_size", args, ac);
	XtManageChild(slider_buffer_size);
	XtAddCallback(slider_buffer_size, XmNvalueChangedCallback, do_buffer_size, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_buffer, (char *)"Data Buffer Size:            1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 20);
		ac++;
		XtSetArg(args[ac], XmNwidth, 170);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard_buffer, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		slider_buffer_size_label = XmCreateLabel(bulletinBoard_buffer, (char *)"slider_buffer_size_label", args, ac);
		XtManageChild(slider_buffer_size_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNwidth, 536);
	ac++;
	XtSetArg(args[ac], XmNheight, 179);
	ac++;
	Widget dialogShell_annotation = XmCreateDialogShell(window_mbedit, (char *)"dialogShell_annotation", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(dialogShell_annotation, (char *)"Annotation", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNdialogTitle, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
		ac++;
		XtSetArg(args[ac], XmNx, 0);
		ac++;
		XtSetArg(args[ac], XmNy, 1247);
		ac++;
		XtSetArg(args[ac], XmNwidth, 536);
		ac++;
		XtSetArg(args[ac], XmNheight, 179);
		ac++;
		bulletinBoard_annotation =
		    XtCreateWidget((char *)"bulletinBoard_annotation", xmBulletinBoardWidgetClass, dialogShell_annotation, args, ac);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_annotation, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 230);
		ac++;
		XtSetArg(args[ac], XmNy, 100);
		ac++;
		XtSetArg(args[ac], XmNwidth, 67);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget pushButton_annotation_dismiss =
		    XmCreatePushButton(bulletinBoard_annotation, (char *)"pushButton_annotation_dismiss", args, ac);
		XtManageChild(pushButton_annotation_dismiss);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_annotation, (char *)"1000", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 460);
		ac++;
		XtSetArg(args[ac], XmNy, 60);
		ac++;
		XtSetArg(args[ac], XmNwidth, 65);
		ac++;
		XtSetArg(args[ac], XmNheight, 15);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		slider_y_max_interval_label = XmCreateLabel(bulletinBoard_annotation, (char *)"slider_y_max_interval_label", args, ac);
		XtManageChild(slider_y_max_interval_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 250);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 1000);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 200);
	ac++;
	XtSetArg(args[ac], XmNy, 50);
	ac++;
	XtSetArg(args[ac], XmNwidth, 250);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(
	    args[ac], XmNfontList,
	    BX_CONVERT(bulletinBoard_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	slider_y_interval = XmCreateScale(bulletinBoard_annotation, (char *)"slider_y_interval", args, ac);
	XtManageChild(slider_y_interval);
	XtAddCallback(slider_y_interval, XmNvalueChangedCallback, do_y_interval, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_annotation, (char *)"Y Axis Tick Interval (m): 1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 60);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		slider_y_interval_label = XmCreateLabel(bulletinBoard_annotation, (char *)"slider_y_interval_label", args, ac);
		XtManageChild(slider_y_interval_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_annotation, (char *)"5000", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 460);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 65);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		slider_x_max_interval_label = XmCreateLabel(bulletinBoard_annotation, (char *)"slider_x_max_interval_label", args, ac);
		XtManageChild(slider_x_max_interval_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 1000);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 5000);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 200);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNwidth, 250);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(
	    args[ac], XmNfontList,
	    BX_CONVERT(bulletinBoard_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	slider_x_interval = XmCreateScale(bulletinBoard_annotation, (char *)"slider_x_interval", args, ac);
	XtManageChild(slider_x_interval);
	XtAddCallback(slider_x_interval, XmNvalueChangedCallback, do_x_interval, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_annotation, (char *)"X Axis Tick Interval (m): 1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		slider_x_interval_label = XmCreateLabel(bulletinBoard_annotation, (char *)"slider_x_interval_label", args, ac);
		XtManageChild(slider_x_interval_label);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Filters");
	ac++;
	XtSetArg(args[ac], XmNwidth, 430);
	ac++;
	XtSetArg(args[ac], XmNheight, 311);
	ac++;
	Widget dialogShell_filters =
		XmCreateDialogShell(window_mbedit, (char *)"dialogShell_filters", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNautoUnmanage, False);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 1181);
	ac++;
	XtSetArg(args[ac], XmNwidth, 430);
	ac++;
	XtSetArg(args[ac], XmNheight, 311);
	ac++;
	Widget bulletinBoard_filters =
	    XtCreateWidget((char *)"bulletinBoard_filters", xmBulletinBoardWidgetClass, dialogShell_filters, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNscrollingPolicy, XmAUTOMATIC);
	ac++;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNwidth, 410);
	ac++;
	XtSetArg(args[ac], XmNheight, 230);
	ac++;
	Widget scrolledWindow_filters =
		XmCreateScrolledWindow(bulletinBoard_filters, (char *)"scrolledWindow_filters", args, ac);
	XtManageChild(scrolledWindow_filters);

	ac = 0;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNwidth, 375);
	ac++;
	XtSetArg(args[ac], XmNheight, 810);
	ac++;
	Widget bulletinBoard_scrollfilters =
		XmCreateBulletinBoard(scrolledWindow_filters, (char *)"bulletinBoard_scrollfilters", args, ac);
	XtManageChild(bulletinBoard_scrollfilters);

	ac = 0;
	XtSetArg(args[ac], XmNx, 90);
	ac++;
	XtSetArg(args[ac], XmNy, 70);
	ac++;
	XtSetArg(args[ac], XmNwidth, 16);
	ac++;
	XtSetArg(args[ac], XmNheight, 16);
	ac++;
	XtSetArg(args[ac], XmNisHomogeneous, False);
	ac++;
	radioBox_mediancalc = XmCreateRadioBox(bulletinBoard_scrollfilters, (char *)"radioBox_mediancalc", args, ac);
	XtManageChild(radioBox_mediancalc);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)"Median Alongtrack Dimension", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, 1);
		ac++;
		XtSetArg(args[ac], XmNvalue, 1);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 20);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 140);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		scale_median_local_ltrack = XmCreateScale(bulletinBoard_scrollfilters, (char *)"scale_median_local_ltrack", args, ac);
		XtManageChild(scale_median_local_ltrack);

		XmStringFree(tmp0);
	}

	XtAddCallback(scale_median_local_ltrack, XmNvalueChangedCallback, do_check_median_ltrack, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)"Median Acrosstrack Dimension", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, 1);
		ac++;
		XtSetArg(args[ac], XmNvalue, 5);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 80);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		scale_median_local_xtrack = XmCreateScale(bulletinBoard_scrollfilters, (char *)"scale_median_local_xtrack", args, ac);
		XtManageChild(scale_median_local_xtrack);

		XmStringFree(tmp0);
	}

	XtAddCallback(scale_median_local_xtrack, XmNvalueChangedCallback, do_check_median_xtrack, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 650);
	ac++;
	XtSetArg(args[ac], XmNwidth, 350);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	Widget separator6 = XmCreateSeparator(bulletinBoard_scrollfilters, (char *)"separator6", args, ac);
	XtManageChild(separator6);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)"End Flagging Angle (deg)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNvalue, 1000);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 10000);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 580);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		scale_filters_cutangleend = XmCreateScale(bulletinBoard_scrollfilters, (char *)"scale_filters_cutangleend", args, ac);
		XtManageChild(scale_filters_cutangleend);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)"Start Flagging Angle (deg)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNvalue, 1000);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 10000);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 520);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		scale_filters_cutanglestart = XmCreateScale(bulletinBoard_scrollfilters, (char *)"scale_filters_cutanglestart", args, ac);
		XtManageChild(scale_filters_cutanglestart);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)":::t\"Flag by\":t\"Beam\"\"Angle\"", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 530);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		toggleButton_filters_cutangle =
		    XmCreateToggleButton(bulletinBoard_scrollfilters, (char *)"toggleButton_filters_cutangle", args, ac);
		XtManageChild(toggleButton_filters_cutangle);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 500);
	ac++;
	XtSetArg(args[ac], XmNwidth, 350);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	Widget separator5 = XmCreateSeparator(bulletinBoard_scrollfilters, (char *)"separator5", args, ac);
	XtManageChild(separator5);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)"End Flagging Distance (m)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNvalue, 1000);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 10000);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 440);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		scale_filters_cutdistanceend =
		    XmCreateScale(bulletinBoard_scrollfilters, (char *)"scale_filters_cutdistanceend", args, ac);
		XtManageChild(scale_filters_cutdistanceend);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)"Start Flagging Distance (m)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNvalue, 1000);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 10000);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 380);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		scale_filters_cutdistancestart =
		    XmCreateScale(bulletinBoard_scrollfilters, (char *)"scale_filters_cutdistancestart", args, ac);
		XtManageChild(scale_filters_cutdistancestart);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)":::t\"Flag by\"\"Distance\"", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 390);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		toggleButton_filters_cutdistance =
		    XmCreateToggleButton(bulletinBoard_scrollfilters, (char *)"toggleButton_filters_cutdistance", args, ac);
		XtManageChild(toggleButton_filters_cutdistance);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 210);
	ac++;
	XtSetArg(args[ac], XmNwidth, 350);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	Widget separator4 = XmCreateSeparator(bulletinBoard_scrollfilters, (char *)"separator4", args, ac);
	XtManageChild(separator4);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)"End Flagging Beam Number", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNvalue, 10);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 100);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 290);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		scale_filters_cutbeamend = XmCreateScale(bulletinBoard_scrollfilters, (char *)"scale_filters_cutbeamend", args, ac);
		XtManageChild(scale_filters_cutbeamend);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)"Start Flagging Beam Number", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNvalue, 10);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 100);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 230);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		scale_filters_cutbeamstart = XmCreateScale(bulletinBoard_scrollfilters, (char *)"scale_filters_cutbeamstart", args, ac);
		XtManageChild(scale_filters_cutbeamstart);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)":::t\"Flag by\":t\"Beam\"\"Number\"", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 240);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		toggleButton_filters_cutbeam =
		    XmCreateToggleButton(bulletinBoard_scrollfilters, (char *)"toggleButton_filters_cutbeam", args, ac);
		XtManageChild(toggleButton_filters_cutbeam);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 360);
	ac++;
	XtSetArg(args[ac], XmNwidth, 350);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	Widget separator3 = XmCreateSeparator(bulletinBoard_scrollfilters, (char *)"separator3", args, ac);
	XtManageChild(separator3);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)"Beams from Center Threshold", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNvalue, 10);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 100);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 670);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		scale_filters_wrongside = XmCreateScale(bulletinBoard_scrollfilters, (char *)"scale_filters_wrongside", args, ac);
		XtManageChild(scale_filters_wrongside);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)":::t\"Wrong\":t\"Side\"\"Filter\"", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 680);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		toggleButton_filters_wrongside =
		    XmCreateToggleButton(bulletinBoard_scrollfilters, (char *)"toggleButton_filters_wrongside", args, ac);
		XtManageChild(toggleButton_filters_wrongside);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)"% Median Depth Threshold ", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, 1);
		ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 0);
		ac++;
		XtSetArg(args[ac], XmNvalue, 10);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		scale_filters_medianspike = XmCreateScale(bulletinBoard_scrollfilters, (char *)"scale_filters_medianspike", args, ac);
		XtManageChild(scale_filters_medianspike);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_scrollfilters, (char *)":::t\"Median\":t\"Spike\"\"Filter\"", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 20);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_scrollfilters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		toggleButton_filters_medianspike =
		    XmCreateToggleButton(bulletinBoard_scrollfilters, (char *)"toggleButton_filters_medianspike", args, ac);
		XtManageChild(toggleButton_filters_medianspike);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_filters, (char *)"Reset", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 160);
		ac++;
		XtSetArg(args[ac], XmNy, 250);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_filters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget pushButton_filters_reset = XmCreatePushButton(bulletinBoard_filters, (char *)"pushButton_filters_reset", args, ac);
		XtManageChild(pushButton_filters_reset);
		XtAddCallback(pushButton_filters_reset, XmNactivateCallback, do_reset_filters, (XtPointer)0);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_filters, (char *)"Apply", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 50);
		ac++;
		XtSetArg(args[ac], XmNy, 250);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_filters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget pushButton_filters_apply = XmCreatePushButton(bulletinBoard_filters, (char *)"pushButton_filters_apply", args, ac);
		XtManageChild(pushButton_filters_apply);
		XtAddCallback(pushButton_filters_apply, XmNactivateCallback, do_set_filters, (XtPointer)0);

		XmStringFree(tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_filters, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 270);
		ac++;
		XtSetArg(args[ac], XmNy, 250);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_filters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		Widget pushButton_filters_dismiss = XmCreatePushButton(bulletinBoard_filters, (char *)"pushButton_filters_dismiss", args, ac);
		XtManageChild(pushButton_filters_dismiss);
		XtAddCallback(pushButton_filters_dismiss, XmNactivateCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_filters");

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNwidth, 343);
	ac++;
	XtSetArg(args[ac], XmNheight, 580);
	ac++;
	Widget dialogShell_filelist = XmCreateDialogShell(window_mbedit, (char *)"dialogShell_filelist", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(dialogShell_filelist, (char *)"Files Available for Editing", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNdialogTitle, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNautoUnmanage, False);
		ac++;
		XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
		ac++;
		XtSetArg(args[ac], XmNx, 167);
		ac++;
		XtSetArg(args[ac], XmNy, 1022);
		ac++;
		XtSetArg(args[ac], XmNwidth, 343);
		ac++;
		XtSetArg(args[ac], XmNheight, 580);
		ac++;
		form_filelist = XtCreateWidget((char *)"form_filelist", xmFormWidgetClass, dialogShell_filelist, args, ac);

		XmStringFree(tmp0);
	}

	ac = 0;
	Widget pushButton_filelist_remove;
	{
		XmString tmp0 = (XmString)BX_CONVERT(form_filelist, (char *)":::t\"Remove\":t\"Selected\"\"File\"", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 110);
		ac++;
		XtSetArg(args[ac], XmNy, 510);
		ac++;
		XtSetArg(args[ac], XmNwidth, 104);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(form_filelist, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_filelist_remove = XmCreatePushButton(form_filelist, (char *)"pushButton_filelist_remove", args, ac);
		XtManageChild(pushButton_filelist_remove);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_filelist_remove, XmNactivateCallback, do_filelist_remove, (XtPointer)0);

	ac = 0;
	Widget pushButton_filelist_edit;
	{
		XmString tmp0 = (XmString)BX_CONVERT(form_filelist, (char *)":::t\"Edit\":t\"Selected\"\"File\"", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 510);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(form_filelist, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_filelist_edit = XmCreatePushButton(form_filelist, (char *)"pushButton_filelist_edit", args, ac);
		XtManageChild(pushButton_filelist_edit);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_filelist_edit, XmNactivateCallback, do_editlistselection, (XtPointer)0);

	ac = 0;
	Widget setting_output_label_filelist;
	{
		XmString tmp0 = (XmString)BX_CONVERT(form_filelist, (char *)"Output Mode:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNmarginWidth, 0);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(form_filelist, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		setting_output_label_filelist = XmCreateLabel(form_filelist, (char *)"setting_output_label_filelist", args, ac);
		XtManageChild(setting_output_label_filelist);

		XmStringFree(tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNnumColumns, 1);
	ac++;
	XtSetArg(args[ac], XmNpacking, XmPACK_COLUMN);
	ac++;
	XtSetArg(args[ac], XmNradioBehavior, True);
	ac++;
	XtSetArg(args[ac], XmNspacing, 0);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 110);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNwidth, 238);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	setting_output_filelist = XmCreateRowColumn(form_filelist, (char *)"setting_output_filelist", args, ac);
	XtManageChild(setting_output_filelist);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(setting_output_filelist, (char *)"Output Edits", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 116);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(setting_output_filelist, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		setting_output_toggle_edit_filelist =
		    XmCreateToggleButton(setting_output_filelist, (char *)"setting_output_toggle_edit_filelist", args, ac);
		XtManageChild(setting_output_toggle_edit_filelist);

		XmStringFree(tmp0);
	}

	XtAddCallback(setting_output_toggle_edit_filelist, XmNvalueChangedCallback, do_output_edit_filelist, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(setting_output_filelist, (char *)"Browse Only", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 116);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(setting_output_filelist, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		setting_output_toggle_browse_filelist =
		    XmCreateToggleButton(setting_output_filelist, (char *)"setting_output_toggle_browse_filelist", args, ac);
		XtManageChild(setting_output_toggle_browse_filelist);

		XmStringFree(tmp0);
	}

	XtAddCallback(setting_output_toggle_browse_filelist, XmNvalueChangedCallback, do_output_browse_filelist, (XtPointer)0);

	ac = 0;
	Widget pushButton_filelist_dismiss;
	{
		XmString tmp0 = (XmString)BX_CONVERT(form_filelist, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 250);
		ac++;
		XtSetArg(args[ac], XmNy, 510);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(form_filelist, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_filelist_dismiss = XmCreatePushButton(form_filelist, (char *)"pushButton_filelist_dismiss", args, ac);
		XtManageChild(pushButton_filelist_dismiss);

		XmStringFree(tmp0);
	}

	XtAddCallback(pushButton_filelist_dismiss, XmNactivateCallback, BxUnmanageCB, (XtPointer) "form_filelist");

	ac = 0;
	XtSetArg(args[ac], XmNscrollingPolicy, XmAPPLICATION_DEFINED);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 50);
	ac++;
	XtSetArg(args[ac], XmNwidth, 339);
	ac++;
	XtSetArg(args[ac], XmNheight, 450);
	ac++;
	Widget scrolledWindow_filelist =
		XmCreateScrolledWindow(form_filelist, (char *)"scrolledWindow_filelist", args, ac);
	XtManageChild(scrolledWindow_filelist);

	ac = 0;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(scrolledWindow_filelist, (char *)"-*-" MONO "-*-r-*-*-*-90-*-*-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	XtSetArg(args[ac], XmNselectionPolicy, XmSINGLE_SELECT);
	ac++;
	XtSetArg(args[ac], XmNwidth, 339);
	ac++;
	XtSetArg(args[ac], XmNheight, 450);
	ac++;
	list_filelist = XmCreateList(scrolledWindow_filelist, (char *)"list_filelist", args, ac);
	XtManageChild(list_filelist);
	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNleftWidget, pushButton_filelist_edit);
	ac++;
	XtSetValues(pushButton_filelist_remove, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetValues(pushButton_filelist_edit, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetValues(setting_output_label_filelist, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 0);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 3);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNleftWidget, setting_output_label_filelist);
	ac++;
	XtSetValues(setting_output_filelist, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 13);
	ac++;
	XtSetValues(pushButton_filelist_dismiss, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNbottomWidget, pushButton_filelist_remove);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 0);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 4);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 6);
	ac++;
	XtSetArg(args[ac], XmNtopWidget, setting_output_filelist);
	ac++;
	XtSetValues(scrolledWindow_filelist, args, ac);

	return (window_mbedit);
}
