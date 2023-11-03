/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit_creation.c	6/24/95
 *
 *    Copyright (c) 1995-2023 by
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
#ifndef SANS
#define SANS "helvetica"
#endif
#ifndef SERIF
#define SERIF "times"
#endif
#ifndef MONO
#define MONO "courier"
#endif

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
#include <Xm/TextF.h>
#include <Xm/BulletinB.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
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
#include <Xm/TextF.h>
#include <Xm/BulletinB.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/FileSB.h>
#include <Xm/CascadeB.h>
#include <Xm/DrawingA.h>

/*
 * Global declarations are now stored in the header file.
 *
 * If DECLARE_BX_GLOBALS is defined then this header file
 * declares the globals, otherwise it just externs them.
 */
#define DECLARE_BX_GLOBALS

#include "mbnavedit_creation.h"

void RegisterBxConverters(XtAppContext);
XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);

void BxExitCB(Widget, XtPointer, XtPointer);
void do_filelist_remove(Widget, XtPointer, XtPointer);
void do_editlistselection(Widget, XtPointer, XtPointer);
void BxUnmanageCB(Widget, XtPointer, XtPointer);
void do_toggle_output_on_filelist(Widget, XtPointer, XtPointer);
void do_toggle_output_off_filelist(Widget, XtPointer, XtPointer);
void do_offset_apply(Widget, XtPointer, XtPointer);
void do_deletebadtimetag_apply(Widget, XtPointer, XtPointer);
void do_timeinterpolation_apply(Widget, XtPointer, XtPointer);
void do_useprevious_no(Widget, XtPointer, XtPointer);
void do_useprevious_yes(Widget, XtPointer, XtPointer);
void do_meantimewindow(Widget, XtPointer, XtPointer);
void do_modeling_apply(Widget, XtPointer, XtPointer);
void do_driftlat(Widget, XtPointer, XtPointer);
void do_driftlon(Widget, XtPointer, XtPointer);
void do_model_mode(Widget, XtPointer, XtPointer);
void do_timestep(Widget, XtPointer, XtPointer);
void do_timespan(Widget, XtPointer, XtPointer);
void do_fileselection_ok(Widget, XtPointer, XtPointer);
void do_fileselection_nomatch(Widget, XtPointer, XtPointer);
void do_fileselection_cancel(Widget, XtPointer, XtPointer);
void do_fileselection_filter(Widget, XtPointer, XtPointer);
void do_toggle_output_on(Widget, XtPointer, XtPointer);
void do_toggle_output_off(Widget, XtPointer, XtPointer);
void do_end(Widget, XtPointer, XtPointer);
void do_start(Widget, XtPointer, XtPointer);
void do_interpolationrepeats(Widget, XtPointer, XtPointer);
void do_unflag(Widget, XtPointer, XtPointer);
void do_flag(Widget, XtPointer, XtPointer);
void do_toggle_org_sensordepth(Widget, XtPointer, XtPointer);
void do_toggle_sensordepth(Widget, XtPointer, XtPointer);
void do_button_use_dr(Widget, XtPointer, XtPointer);
void do_toggle_dr_lat(Widget, XtPointer, XtPointer);
void do_toggle_dr_lon(Widget, XtPointer, XtPointer);
void do_toggle_org_time(Widget, XtPointer, XtPointer);
void do_toggle_time(Widget, XtPointer, XtPointer);
void do_nextbuffer(Widget, XtPointer, XtPointer);
void do_done(Widget, XtPointer, XtPointer);
void do_forward(Widget, XtPointer, XtPointer);
void do_reverse(Widget, XtPointer, XtPointer);
void BxManageCB(Widget, XtPointer, XtPointer);
void do_toggle_vru(Widget, XtPointer, XtPointer);
void do_set_interval(Widget, XtPointer, XtPointer);
void do_showall(Widget, XtPointer, XtPointer);
void do_revert(Widget, XtPointer, XtPointer);
void do_interpolation(Widget, XtPointer, XtPointer);
void do_button_use_cmg(Widget, XtPointer, XtPointer);
void do_toggle_show_cmg(Widget, XtPointer, XtPointer);
void do_toggle_org_heading(Widget, XtPointer, XtPointer);
void do_button_use_smg(Widget, XtPointer, XtPointer);
void do_toggle_show_smg(Widget, XtPointer, XtPointer);
void do_toggle_org_speed(Widget, XtPointer, XtPointer);
void do_toggle_org_lat(Widget, XtPointer, XtPointer);
void do_toggle_org_lon(Widget, XtPointer, XtPointer);
void do_toggle_speed(Widget, XtPointer, XtPointer);
void do_toggle_heading(Widget, XtPointer, XtPointer);
void do_toggle_lat(Widget, XtPointer, XtPointer);
void do_toggle_lon(Widget, XtPointer, XtPointer);
void BxSetValuesCB(Widget, XtPointer, XtPointer);
void do_toggle_pick(Widget, XtPointer, XtPointer);
void do_toggle_select(Widget, XtPointer, XtPointer);
void do_toggle_deselect(Widget, XtPointer, XtPointer);
void do_toggle_selectall(Widget, XtPointer, XtPointer);
void do_toggle_deselectall(Widget, XtPointer, XtPointer);
void do_resize(Widget, XtPointer, XtPointer);
void do_event(Widget, XtPointer, XtPointer);
void do_expose(Widget, XtPointer, XtPointer);

/**
 * Create the mainWindow hierarchy of widgets.
 */
Widget CreatemainWindow(Widget parent) {
	Boolean argok = False;
	Widget mainWindow;
	Widget dialogShell_filelist;
	Widget form_filelist;
	Widget pushButton_filelist_remove;
	Widget pushButton_filelist_edit;
	Widget setting_output_label_filelist;
	Widget setting_output_filelist;
	Widget pushButton_filelist_dismiss;
	Widget scrolledWindow_filelist;
	Widget xmDialogShell_offset;
	Widget form_offset;
	Widget label_offset_lat;
	Widget label_offset_lon;
	Widget dialogShell_deletebadtimetag;
	Widget label_deletetimetag;
	Widget pushButton_deletebadtimetag_dismiss;
	Widget pushButton_deletebadtimetag_apply;
	Widget xmDialogShell_timeinterpolation;
	Widget label_timeinterpolation;
	Widget pushButton_timeinterpolation_dismiss;
	Widget pushButton_timeinterpolation_apply;
	Widget xmDialogShell_useprevious;
	Widget label_useprevious;
	Widget pushButton_useprevious_no;
	Widget pushButton_useprevious_yes;
	Widget xmDialogShell_modeling;
	Widget bulletinBoard_modeling;
	Widget separator5;
	Widget pushButton_modeling_apply;
	Widget separator4;
	Widget label_modeling_acceleration;
	Widget label_modeling_speed;
	Widget label_modeling_inversion;
	Widget pushButton_modeling_dismiss;
	Widget separator3;
	Widget separator2;
	Widget label_modeling_mode;
	Widget radioBox_modeling;
	Widget xmDialogShell_timestepping;
	Widget bulletinBoard_timestepping;
	Widget pushButton_timestepping_dismiss;
	Widget xmDialogShell_error;
	Widget pushButton_error;
	Widget xmDialogShell_message;
	Widget label_pleasewait;
	Widget xmDialogShell_about;
	Widget bulletinBoard_about;
	Widget label_about_create1;
	Widget separator1;
	Widget pushButton_about_dismiss;
	Widget label_about_create;
	Widget label_about_lamont;
	Widget label_about_columbia;
	Widget label_about_mbsystem;
	Widget label_about_mbpub;
	Widget label_about_component;
	Widget label_about_for;
	Widget separator;
	Widget label_about_function;
	Widget label_about_mbedit;
	Widget menuBar_file;
	Widget cascadeButton_file;
	Widget pulldownMenu_file;
	Widget pushButton_filelist;
	Widget menuBar_controls;
	Widget cascadeButton_controls;
	Widget pulldownMenu_controls;
	Widget pushButton_controls_timespan;
	Widget pushButton_controls_modeling;
	Widget pushButton_controls_offset;
	Widget pushButton_nextbuffer;
	Widget pushButton_quit;
	Widget pushButton_about;
	Widget pushButton_set_interval;
	Widget pushButton_showall;

	/**
	 * Register the converters for the widgets.
	 */
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
	XtInitializeWidgetClass((WidgetClass)xmTextFieldWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmBulletinBoardWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmScaleWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmSeparatorWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmFileSelectionBoxWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmBulletinBoardWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmCascadeButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmDrawingAreaWidgetClass);

	Cardinal ac = 0;
	Arg args[256];
	XtSetArg(args[ac], XmNx, 964);
	ac++;
	XtSetArg(args[ac], XmNy, 300);
	ac++;
	XtSetArg(args[ac], XmNwidth, 1024);
	ac++;
	XtSetArg(args[ac], XmNheight, 683);
	ac++;
	mainWindow = XmCreateMainWindow(parent, (char *)"mainWindow", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNwidth, 1024);
	ac++;
	XtSetArg(args[ac], XmNheight, 683);
	ac++;
	bulletinBoard = XmCreateBulletinBoard(mainWindow, (char *)"bulletinBoard", args, ac);
	XtManageChild(bulletinBoard);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"End", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 390);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		pushButton_end = XmCreatePushButton(bulletinBoard, (char *)"pushButton_end", args, ac);
		XtManageChild(pushButton_end);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_end, XmNactivateCallback, do_end, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Start", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 180);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		pushButton_start = XmCreatePushButton(bulletinBoard, (char *)"pushButton_start", args, ac);
		XtManageChild(pushButton_start);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_start, XmNactivateCallback, do_start, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNwidth, 51);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	menuBar_file = XmCreateMenuBar(bulletinBoard, (char *)"menuBar_file", args, ac);
	XtManageChild(menuBar_file);

	ac = 0;
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

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 135);
	ac++;
	XtSetArg(args[ac], XmNheight, 52);
	ac++;
	pulldownMenu_file = XmCreatePulldownMenu(XtParent(cascadeButton_file), (char *)"pulldownMenu_file", args, ac);

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

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_file, XmNactivateCallback, BxManageCB, (XtPointer) "bulletinBoard_fileselection");
	XtAddCallback(pushButton_file, XmNactivateCallback, BxSetValuesCB, (XtPointer) "label_format.labelString=MBIO Format ID:");

	ac = 0;
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

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_filelist, XmNactivateCallback, BxManageCB, (XtPointer) "form_filelist");

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, pulldownMenu_file);
	ac++;
	XtSetValues(cascadeButton_file, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Interpolate Repeats", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 110);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		pushButton_interpolaterepeats = XmCreatePushButton(bulletinBoard, (char *)"pushButton_interpolaterepeats", args, ac);
		XtManageChild(pushButton_interpolaterepeats);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_interpolaterepeats, XmNactivateCallback, do_interpolationrepeats, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Unflag", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 530);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_unflag = XmCreatePushButton(bulletinBoard, (char *)"pushButton_unflag", args, ac);
		XtManageChild(pushButton_unflag);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_unflag, XmNactivateCallback, do_unflag, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Flag", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 460);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_flag = XmCreatePushButton(bulletinBoard, (char *)"pushButton_flag", args, ac);
		XtManageChild(pushButton_flag);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_flag, XmNactivateCallback, do_flag, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNx, 80);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNwidth, 87);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	menuBar_controls = XmCreateMenuBar(bulletinBoard, (char *)"menuBar_controls", args, ac);
	XtManageChild(menuBar_controls);

	ac = 0;
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

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 137);
	ac++;
	XtSetArg(args[ac], XmNheight, 124);
	ac++;
	pulldownMenu_controls = XmCreatePulldownMenu(XtParent(cascadeButton_controls), (char *)"pulldownMenu_controls", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Time Stepping", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		pushButton_controls_timespan =
		    XmCreatePushButton(pulldownMenu_controls, (char *)"pushButton_controls_timespan", args, ac);
		XtManageChild(pushButton_controls_timespan);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_controls_timespan, XmNactivateCallback, BxManageCB, (XtPointer) "bulletinBoard_timestepping");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Nav Modeling", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		pushButton_controls_modeling =
		    XmCreatePushButton(pulldownMenu_controls, (char *)"pushButton_controls_modeling", args, ac);
		XtManageChild(pushButton_controls_modeling);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_controls_modeling, XmNactivateCallback, BxManageCB, (XtPointer) "bulletinBoard_modeling");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Time Interpolation", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		pushButton_controls_timeinterpolation =
		    XmCreatePushButton(pulldownMenu_controls, (char *)"pushButton_controls_timeinterpolation", args, ac);
		XtManageChild(pushButton_controls_timeinterpolation);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_controls_timeinterpolation, XmNactivateCallback, BxManageCB,
	              (XtPointer) "bulletinBoard_timeinterpolation");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Delete Bad Times", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		pushButton_controls_deletebadtimetag =
		    XmCreatePushButton(pulldownMenu_controls, (char *)"pushButton_controls_deletebadtimetag", args, ac);
		XtManageChild(pushButton_controls_deletebadtimetag);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_controls_deletebadtimetag, XmNactivateCallback, BxManageCB,
	              (XtPointer) "bulletinBoard_deletebadtimetag");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(pulldownMenu_controls, (char *)"Position Offset", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(pulldownMenu_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		pushButton_controls_offset = XmCreatePushButton(pulldownMenu_controls, (char *)"pushButton_controls_offset", args, ac);
		XtManageChild(pushButton_controls_offset);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_controls_offset, XmNactivateCallback, BxManageCB, (XtPointer) "form_offset");

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, pulldownMenu_controls);
	ac++;
	XtSetValues(cascadeButton_controls, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show Original Data", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 480);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_org_sensordepth = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_org_sensordepth", args, ac);
		XtManageChild(toggleButton_org_sensordepth);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_org_sensordepth, XmNvalueChangedCallback, do_toggle_org_sensordepth, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Sonar Depth Plot", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 460);
		ac++;
		XtSetArg(args[ac], XmNwidth, 140);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_sensordepth = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_sensordepth", args, ac);
		XtManageChild(toggleButton_sensordepth);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_sensordepth, XmNvalueChangedCallback, do_toggle_sensordepth, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Use Solution", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 360);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 100);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		pushButton_solution = XmCreatePushButton(bulletinBoard, (char *)"pushButton_solution", args, ac);
		XtManageChild(pushButton_solution);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_solution, XmNactivateCallback, do_button_use_dr, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show Dead Reckoning", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 250);
		ac++;
		XtSetArg(args[ac], XmNwidth, 159);
		ac++;
		XtSetArg(args[ac], XmNheight, 26);
		ac++;
		toggleButton_dr_lat = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_dr_lat", args, ac);
		XtManageChild(toggleButton_dr_lat);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_dr_lat, XmNvalueChangedCallback, do_toggle_dr_lat, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show Dead Reckoning", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 180);
		ac++;
		XtSetArg(args[ac], XmNwidth, 159);
		ac++;
		XtSetArg(args[ac], XmNheight, 26);
		ac++;
		toggleButton_dr_lon = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_dr_lon", args, ac);
		XtManageChild(toggleButton_dr_lon);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_dr_lon, XmNvalueChangedCallback, do_toggle_dr_lon, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show Original Data", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 110);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_org_time = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_org_time", args, ac);
		XtManageChild(toggleButton_org_time);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_org_time, XmNvalueChangedCallback, do_toggle_org_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Time Interval Plot", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNstringDirection, XmSTRING_DIRECTION_L_TO_R);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_time = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_time", args, ac);
		XtManageChild(toggleButton_time);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_time, XmNvalueChangedCallback, do_toggle_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Next Buffer", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 480);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 88);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		pushButton_nextbuffer = XmCreatePushButton(bulletinBoard, (char *)"pushButton_nextbuffer", args, ac);
		XtManageChild(pushButton_nextbuffer);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_nextbuffer, XmNactivateCallback, do_nextbuffer, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Done", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 760);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		pushButton_done = XmCreatePushButton(bulletinBoard, (char *)"pushButton_done", args, ac);
		XtManageChild(pushButton_done);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_done, XmNactivateCallback, do_done, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Forward", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 320);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 69);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		pushButton_forward = XmCreatePushButton(bulletinBoard, (char *)"pushButton_forward", args, ac);
		XtManageChild(pushButton_forward);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_forward, XmNactivateCallback, do_forward, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Reverse", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 250);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 69);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		pushButton_reverse = XmCreatePushButton(bulletinBoard, (char *)"pushButton_reverse", args, ac);
		XtManageChild(pushButton_reverse);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_reverse, XmNactivateCallback, do_reverse, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Quit", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 840);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 70);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		pushButton_quit = XmCreatePushButton(bulletinBoard, (char *)"pushButton_quit", args, ac);
		XtManageChild(pushButton_quit);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_quit, XmNactivateCallback, BxExitCB, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"About", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 920);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		pushButton_about = XmCreatePushButton(bulletinBoard, (char *)"pushButton_about", args, ac);
		XtManageChild(pushButton_about);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_about, XmNactivateCallback, BxManageCB, (XtPointer) "bulletinBoard_about");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Roll, Pitch, and Heave Plots", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 510);
		ac++;
		XtSetArg(args[ac], XmNwidth, 191);
		ac++;
		XtSetArg(args[ac], XmNheight, 26);
		ac++;
		toggleButton_vru = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_vru", args, ac);
		XtManageChild(toggleButton_vru);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_vru, XmNvalueChangedCallback, do_toggle_vru, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Pick Zoom", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 670);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		pushButton_set_interval = XmCreatePushButton(bulletinBoard, (char *)"pushButton_set_interval", args, ac);
		XtManageChild(pushButton_set_interval);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_set_interval, XmNactivateCallback, do_set_interval, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show All", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 580);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		pushButton_showall = XmCreatePushButton(bulletinBoard, (char *)"pushButton_showall", args, ac);
		XtManageChild(pushButton_showall);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_showall, XmNactivateCallback, do_showall, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Revert", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 270);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		pushButton_revert = XmCreatePushButton(bulletinBoard, (char *)"pushButton_revert", args, ac);
		XtManageChild(pushButton_revert);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_revert, XmNactivateCallback, do_revert, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Interpolate", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		pushButton_interpolate = XmCreatePushButton(bulletinBoard, (char *)"pushButton_interpolate", args, ac);
		XtManageChild(pushButton_interpolate);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_interpolate, XmNactivateCallback, do_interpolation, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNpacking, XmPACK_TIGHT);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 600);
		ac++;
		XtSetArg(args[ac], XmNy, 40);
		ac++;
		XtSetArg(args[ac], XmNwidth, 414);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNisHomogeneous, False);
		ac++;
		radioBox = XmCreateRadioBox(bulletinBoard, (char *)"radioBox", args, ac);
		XtManageChild(radioBox);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox, (char *)"Pick", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(radioBox, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNwidth, 51);
		ac++;
		XtSetArg(args[ac], XmNheight, 34);
		ac++;
		toggleButton_pick = XmCreateToggleButton(radioBox, (char *)"toggleButton_pick", args, ac);
		XtManageChild(toggleButton_pick);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_pick, XmNvalueChangedCallback, do_toggle_pick, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox, (char *)"Select", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(radioBox, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNwidth, 64);
		ac++;
		XtSetArg(args[ac], XmNheight, 34);
		ac++;
		toggleButton_select = XmCreateToggleButton(radioBox, (char *)"toggleButton_select", args, ac);
		XtManageChild(toggleButton_select);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_select, XmNvalueChangedCallback, do_toggle_select, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox, (char *)"Deselect", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(radioBox, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNwidth, 78);
		ac++;
		XtSetArg(args[ac], XmNheight, 34);
		ac++;
		toggleButton_deselect = XmCreateToggleButton(radioBox, (char *)"toggleButton_deselect", args, ac);
		XtManageChild(toggleButton_deselect);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_deselect, XmNvalueChangedCallback, do_toggle_deselect, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox, (char *)"Select All", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(radioBox, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNwidth, 93);
		ac++;
		XtSetArg(args[ac], XmNheight, 34);
		ac++;
		toggleButton_selectall = XmCreateToggleButton(radioBox, (char *)"toggleButton_selectall", args, ac);
		XtManageChild(toggleButton_selectall);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_selectall, XmNvalueChangedCallback, do_toggle_selectall, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox, (char *)"Deselect All", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(radioBox, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 34);
		ac++;
		toggleButton_deselectall = XmCreateToggleButton(radioBox, (char *)"toggleButton_deselectall", args, ac);
		XtManageChild(toggleButton_deselectall);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_deselectall, XmNvalueChangedCallback, do_toggle_deselectall, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Use Course-Made-Good", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 430);
		ac++;
		XtSetArg(args[ac], XmNwidth, 180);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		pushButton_heading_cmg = XmCreatePushButton(bulletinBoard, (char *)"pushButton_heading_cmg", args, ac);
		XtManageChild(pushButton_heading_cmg);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_heading_cmg, XmNactivateCallback, do_button_use_cmg, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show Course-Made-Good", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 410);
		ac++;
		XtSetArg(args[ac], XmNwidth, 190);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_show_cmg = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_show_cmg", args, ac);
		XtManageChild(toggleButton_show_cmg);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_show_cmg, XmNvalueChangedCallback, do_toggle_show_cmg, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show Original Data", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 390);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_org_heading = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_org_heading", args, ac);
		XtManageChild(toggleButton_org_heading);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_org_heading, XmNvalueChangedCallback, do_toggle_org_heading, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Use Speed-Made-Good", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 340);
		ac++;
		XtSetArg(args[ac], XmNwidth, 180);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		pushButton_speed_smg = XmCreatePushButton(bulletinBoard, (char *)"pushButton_speed_smg", args, ac);
		XtManageChild(pushButton_speed_smg);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_speed_smg, XmNactivateCallback, do_button_use_smg, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show Speed-Made-Good", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 320);
		ac++;
		XtSetArg(args[ac], XmNwidth, 180);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_show_smg = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_show_smg", args, ac);
		XtManageChild(toggleButton_show_smg);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_show_smg, XmNvalueChangedCallback, do_toggle_show_smg, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show Original Data", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 300);
		ac++;
		XtSetArg(args[ac], XmNwidth, 180);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_org_speed = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_org_speed", args, ac);
		XtManageChild(toggleButton_org_speed);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_org_speed, XmNvalueChangedCallback, do_toggle_org_speed, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show Original Data", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 230);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_org_lat = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_org_lat", args, ac);
		XtManageChild(toggleButton_org_lat);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_org_lat, XmNvalueChangedCallback, do_toggle_org_lat, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Show Original Data", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 160);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_org_lon = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_org_lon", args, ac);
		XtManageChild(toggleButton_org_lon);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_org_lon, XmNvalueChangedCallback, do_toggle_org_lon, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Speed Plot", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 280);
		ac++;
		XtSetArg(args[ac], XmNwidth, 160);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_speed = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_speed", args, ac);
		XtManageChild(toggleButton_speed);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_speed, XmNvalueChangedCallback, do_toggle_speed, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Heading Plot", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 370);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_heading = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_heading", args, ac);
		XtManageChild(toggleButton_heading);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_heading, XmNvalueChangedCallback, do_toggle_heading, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Latitude Plot", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 210);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_lat = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_lat", args, ac);
		XtManageChild(toggleButton_lat);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_lat, XmNvalueChangedCallback, do_toggle_lat, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard, (char *)"Longitude Plot", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
		ac++;
		XtSetArg(args[ac], XmNstringDirection, XmSTRING_DIRECTION_L_TO_R);
		ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 140);
		ac++;
		XtSetArg(args[ac], XmNwidth, 140);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		toggleButton_lon = XmCreateToggleButton(bulletinBoard, (char *)"toggleButton_lon", args, ac);
		XtManageChild(toggleButton_lon);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_lon, XmNvalueChangedCallback, do_toggle_lon, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNscrollingPolicy, XmAUTOMATIC);
	ac++;
	XtSetArg(args[ac], XmNx, 210);
	ac++;
	XtSetArg(args[ac], XmNy, 80);
	ac++;
	XtSetArg(args[ac], XmNwidth, 800);
	ac++;
	XtSetArg(args[ac], XmNheight, 590);
	ac++;
	scrolledWindow = XmCreateScrolledWindow(bulletinBoard, (char *)"scrolledWindow", args, ac);
	XtManageChild(scrolledWindow);

	ac = 0;
	XtSetArg(args[ac], XmNborderWidth, 1);
	ac++;
	XtSetArg(args[ac], XmNbackground, BX_CONVERT(scrolledWindow, (char *)"white", XmRPixel, 0, &argok));
	if (argok)
		ac++;
	XtSetArg(args[ac], XmNwidth, 767);
	ac++;
	XtSetArg(args[ac], XmNheight, 2000);
	ac++;
	drawingArea = XmCreateDrawingArea(scrolledWindow, (char *)"drawingArea", args, ac);
	XtManageChild(drawingArea);
	XtAddCallback(drawingArea, XmNresizeCallback, do_resize, (XtPointer)0);
	XtAddCallback(drawingArea, XmNinputCallback, do_event, (XtPointer)0);
	XtAddCallback(drawingArea, XmNexposeCallback, do_expose, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "About MBnavedit");
	ac++;
	XtSetArg(args[ac], XmNdeleteResponse, XmUNMAP);
	ac++;
	XtSetArg(args[ac], XmNwidth, 481);
	ac++;
	XtSetArg(args[ac], XmNheight, 466);
	ac++;
	xmDialogShell_about = XmCreateDialogShell(mainWindow, (char *)"xmDialogShell_about", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 1040);
	ac++;
	XtSetArg(args[ac], XmNy, 1104);
	ac++;
	XtSetArg(args[ac], XmNwidth, 481);
	ac++;
	XtSetArg(args[ac], XmNheight, 466);
	ac++;
	bulletinBoard_about =
	    XtCreateWidget((char *)"bulletinBoard_about", xmBulletinBoardWidgetClass, xmDialogShell_about, args, ac);

	ac = 0;
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
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		label_about_create1 = XmCreateLabel(bulletinBoard_about, (char *)"label_about_create1", args, ac);
		XtManageChild(label_about_create1);

		XmStringFree((XmString)tmp0);
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
	separator1 = XmCreateSeparator(bulletinBoard_about, (char *)"separator1", args, ac);
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
		XtSetArg(args[ac], XmNy, 420);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 35);
		ac++;
		pushButton_about_dismiss = XmCreatePushButton(bulletinBoard_about, (char *)"pushButton_about_dismiss", args, ac);
		XtManageChild(pushButton_about_dismiss);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_about_dismiss, XmNactivateCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_about");

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
		label_about_create = XmCreateLabel(bulletinBoard_about, (char *)"label_about_create", args, ac);
		XtManageChild(label_about_create);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about,
		                            (char *)":::t\"Lamont-Doherty \":t\"Earth Observatory\"\"of Columbia University\"",
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
		XtSetArg(args[ac], XmNy, 290);
		ac++;
		XtSetArg(args[ac], XmNwidth, 190);
		ac++;
		XtSetArg(args[ac], XmNheight, 70);
		ac++;
		label_about_lamont = XmCreateLabel(bulletinBoard_about, (char *)"label_about_lamont", args, ac);
		XtManageChild(label_about_lamont);

		XmStringFree((XmString)tmp0);
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
		XtSetArg(args[ac], XmNy, 290);
		ac++;
		XtSetArg(args[ac], XmNwidth, 160);
		ac++;
		XtSetArg(args[ac], XmNheight, 70);
		ac++;
		label_about_columbia = XmCreateLabel(bulletinBoard_about, (char *)"label_about_columbia", args, ac);
		XtManageChild(label_about_columbia);

		XmStringFree((XmString)tmp0);
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
		label_about_mbsystem = XmCreateLabel(bulletinBoard_about, (char *)"label_about_mbsystem", args, ac);
		XtManageChild(label_about_mbsystem);

		XmStringFree((XmString)tmp0);
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
		label_about_mbpub = XmCreateLabel(bulletinBoard_about, (char *)"label_about_mbpub", args, ac);
		XtManageChild(label_about_mbpub);

		XmStringFree((XmString)tmp0);
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
		label_about_component = XmCreateLabel(bulletinBoard_about, (char *)"label_about_component", args, ac);
		XtManageChild(label_about_component);

		XmStringFree((XmString)tmp0);
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
		XtSetArg(args[ac], XmNwidth, 460);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		label_about_for = XmCreateLabel(bulletinBoard_about, (char *)"label_about_for", args, ac);
		XtManageChild(label_about_for);

		XmStringFree((XmString)tmp0);
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
	separator = XmCreateSeparator(bulletinBoard_about, (char *)"separator", args, ac);
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
		XtSetArg(args[ac], XmNy, 360);
		ac++;
		XtSetArg(args[ac], XmNwidth, 450);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		label_about_version = XmCreateLabel(bulletinBoard_about, (char *)"label_about_version", args, ac);
		XtManageChild(label_about_version);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)"Interactive Navigation Editor", XmRXmString, 0, &argok);
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
		XtSetArg(args[ac], XmNheight, 23);
		ac++;
		label_about_function = XmCreateLabel(bulletinBoard_about, (char *)"label_about_function", args, ac);
		XtManageChild(label_about_function);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_about, (char *)"MBnavedit", XmRXmString, 0, &argok);
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
		label_about_mbedit = XmCreateLabel(bulletinBoard_about, (char *)"label_about_mbedit", args, ac);
		XtManageChild(label_about_mbedit);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Please Wait...");
	ac++;
	XtSetArg(args[ac], XmNmwmInputMode, MWM_INPUT_MODELESS);
	ac++;
	XtSetArg(args[ac], XmNdeleteResponse, XmUNMAP);
	ac++;
	XtSetArg(args[ac], XmNwidth, 379);
	ac++;
	XtSetArg(args[ac], XmNheight, 86);
	ac++;
	xmDialogShell_message = XmCreateDialogShell(mainWindow, (char *)"xmDialogShell_message", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_NONE);
	ac++;
	XtSetArg(args[ac], XmNx, 1091);
	ac++;
	XtSetArg(args[ac], XmNy, 1294);
	ac++;
	XtSetArg(args[ac], XmNwidth, 379);
	ac++;
	XtSetArg(args[ac], XmNheight, 86);
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
		label_pleasewait = XmCreateLabel(bulletinBoard_message, (char *)"label_pleasewait", args, ac);
		XtManageChild(label_pleasewait);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_message, (char *)"MBvelocitytool is loading data...", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
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
		label_message = XmCreateLabel(bulletinBoard_message, (char *)"label_message", args, ac);
		XtManageChild(label_message);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Error");
	ac++;
	XtSetArg(args[ac], XmNwidth, 311);
	ac++;
	XtSetArg(args[ac], XmNheight, 161);
	ac++;
	xmDialogShell_error = XmCreateDialogShell(mainWindow, (char *)"xmDialogShell_error", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 1125);
	ac++;
	XtSetArg(args[ac], XmNy, 1256);
	ac++;
	XtSetArg(args[ac], XmNwidth, 311);
	ac++;
	XtSetArg(args[ac], XmNheight, 161);
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

		XmStringFree((XmString)tmp0);
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

		XmStringFree((XmString)tmp0);
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

		XmStringFree((XmString)tmp0);
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
		pushButton_error = XmCreatePushButton(bulletinBoard_error, (char *)"pushButton_error", args, ac);
		XtManageChild(pushButton_error);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Select Input Data File");
	ac++;
	XtSetArg(args[ac], XmNallowShellResize, False);
	ac++;
	XtSetArg(args[ac], XmNwidth, 606);
	ac++;
	XtSetArg(args[ac], XmNheight, 557);
	ac++;
	xmDialogShell_fileselection = XmCreateDialogShell(mainWindow, (char *)"xmDialogShell_fileselection", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_NONE);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 1058);
	ac++;
	XtSetArg(args[ac], XmNwidth, 606);
	ac++;
	XtSetArg(args[ac], XmNheight, 557);
	ac++;
	bulletinBoard_fileselection =
	    XtCreateWidget((char *)"bulletinBoard_fileselection", xmBulletinBoardWidgetClass, xmDialogShell_fileselection, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_fileselection, (char *)"Output Mode:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_fileselection, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 290);
		ac++;
		XtSetArg(args[ac], XmNy, 480);
		ac++;
		XtSetArg(args[ac], XmNwidth, 100);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		label_output_mode = XmCreateLabel(bulletinBoard_fileselection, (char *)"label_output_mode", args, ac);
		XtManageChild(label_output_mode);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 390);
	ac++;
	XtSetArg(args[ac], XmNy, 480);
	ac++;
	XtSetArg(args[ac], XmNwidth, 167);
	ac++;
	XtSetArg(args[ac], XmNheight, 65);
	ac++;
	XtSetArg(args[ac], XmNisHomogeneous, False);
	ac++;
	radioBox_output = XmCreateRadioBox(bulletinBoard_fileselection, (char *)"radioBox_output", args, ac);
	XtManageChild(radioBox_output);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox_output, (char *)"Output Edited Data", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(radioBox_output, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 161);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		toggleButton_output_on = XmCreateToggleButton(radioBox_output, (char *)"toggleButton_output_on", args, ac);
		XtManageChild(toggleButton_output_on);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_output_on, XmNvalueChangedCallback, do_toggle_output_on, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox_output, (char *)"Browse Only", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(radioBox_output, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 161);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		toggleButton_output_off = XmCreateToggleButton(radioBox_output, (char *)"toggleButton_output_off", args, ac);
		XtManageChild(toggleButton_output_off);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_output_off, XmNvalueChangedCallback, do_toggle_output_off, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(bulletinBoard_fileselection, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1",
	                    XmRFontList, 0, &argok));
	if (argok)
		ac++;
	XtSetArg(args[ac], XmNx, 140);
	ac++;
	XtSetArg(args[ac], XmNy, 480);
	ac++;
	XtSetArg(args[ac], XmNwidth, 100);
	ac++;
	XtSetArg(args[ac], XmNheight, 35);
	ac++;
	textField_format = XmCreateTextField(bulletinBoard_fileselection, (char *)"textField_format", args, ac);
	XtManageChild(textField_format);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_fileselection, (char *)"MBIO Format ID:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_fileselection, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 480);
		ac++;
		XtSetArg(args[ac], XmNwidth, 120);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		label_format = XmCreateLabel(bulletinBoard_fileselection, (char *)"label_format", args, ac);
		XtManageChild(label_format);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_fileselection, (char *)"", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNdialogTitle, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNlistVisibleItemCount, 13);
		ac++;
		XtSetArg(args[ac], XmNtextFontList,
		         BX_CONVERT(bulletinBoard_fileselection, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNlabelFontList,
		         BX_CONVERT(bulletinBoard_fileselection, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNbuttonFontList,
		         BX_CONVERT(bulletinBoard_fileselection, (char *)"-*-" SANS "-bold-r-normal--14-140-75-75-p-82-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNautoUnmanage, False);
		ac++;
		XtSetArg(args[ac], XmNnoResize, True);
		ac++;
		XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_NONE);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 590);
		ac++;
		XtSetArg(args[ac], XmNheight, 470);
		ac++;
		fileSelectionBox = XmCreateFileSelectionBox(bulletinBoard_fileselection, (char *)"fileSelectionBox", args, ac);
		XtManageChild(fileSelectionBox);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(fileSelectionBox, XmNokCallback, do_fileselection_ok, (XtPointer)0);
	XtAddCallback(fileSelectionBox, XmNokCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_fileselection");
	XtAddCallback(fileSelectionBox, XmNnoMatchCallback, do_fileselection_nomatch, (XtPointer)0);
	XtAddCallback(fileSelectionBox, XmNcancelCallback, do_fileselection_cancel, (XtPointer)0);
	XtAddCallback(fileSelectionBox, XmNcancelCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_fileselection");
	XtAddCallback(fileSelectionBox, XmNapplyCallback, do_fileselection_filter, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Controls");
	ac++;
	XtSetArg(args[ac], XmNwidth, 491);
	ac++;
	XtSetArg(args[ac], XmNheight, 195);
	ac++;
	xmDialogShell_timestepping = XmCreateDialogShell(mainWindow, (char *)"xmDialogShell_timestepping", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(xmDialogShell_timestepping, (char *)"Time Stepping", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNdialogTitle, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNautoUnmanage, False);
		ac++;
		XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
		ac++;
		XtSetArg(args[ac], XmNx, 1035);
		ac++;
		XtSetArg(args[ac], XmNy, 1239);
		ac++;
		XtSetArg(args[ac], XmNwidth, 491);
		ac++;
		XtSetArg(args[ac], XmNheight, 195);
		ac++;
		bulletinBoard_timestepping = XtCreateWidget((char *)"bulletinBoard_timestepping", xmBulletinBoardWidgetClass,
		                                            xmDialogShell_timestepping, args, ac);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_timestepping, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 200);
		ac++;
		XtSetArg(args[ac], XmNy, 140);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_timestepping, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		pushButton_timestepping_dismiss =
		    XmCreatePushButton(bulletinBoard_timestepping, (char *)"pushButton_timestepping_dismiss", args, ac);
		XtManageChild(pushButton_timestepping_dismiss);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_timestepping_dismiss, XmNactivateCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_timestepping");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_timestepping, (char *)"500", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_timestepping, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 440);
		ac++;
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNwidth, 40);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		label_timestep_2 = XmCreateLabel(bulletinBoard_timestepping, (char *)"label_timestep_2", args, ac);
		XtManageChild(label_timestep_2);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(bulletinBoard_timestepping, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList, 0,
	                    &argok));
	if (argok)
		ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 180);
	ac++;
	XtSetArg(args[ac], XmNy, 70);
	ac++;
	XtSetArg(args[ac], XmNwidth, 260);
	ac++;
	XtSetArg(args[ac], XmNheight, 50);
	ac++;
	scale_timestep = XmCreateScale(bulletinBoard_timestepping, (char *)"scale_timestep", args, ac);
	XtManageChild(scale_timestep);
	XtAddCallback(scale_timestep, XmNvalueChangedCallback, do_timestep, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_timestepping, (char *)"Time Step (sec):  1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_timestepping, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNwidth, 170);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		label_timestep_1 = XmCreateLabel(bulletinBoard_timestepping, (char *)"label_timestep_1", args, ac);
		XtManageChild(label_timestep_1);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_timestepping, (char *)"500", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_timestepping, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 440);
		ac++;
		XtSetArg(args[ac], XmNy, 30);
		ac++;
		XtSetArg(args[ac], XmNwidth, 40);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		label_timespan_2 = XmCreateLabel(bulletinBoard_timestepping, (char *)"label_timespan_2", args, ac);
		XtManageChild(label_timespan_2);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_timestepping, (char *)" ", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_timestepping, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNstringDirection, XmSTRING_DIRECTION_L_TO_R);
		ac++;
		XtSetArg(args[ac], XmNx, 180);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 50);
		ac++;
		scale_timespan = XmCreateScale(bulletinBoard_timestepping, (char *)"scale_timespan", args, ac);
		XtManageChild(scale_timespan);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(scale_timespan, XmNvalueChangedCallback, do_timespan, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_timestepping, (char *)"TIme Span Shown (sec):  1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_timestepping, (char *)"-*-" SANS "-bold-r-*-*-*-120-75-75-p-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 30);
		ac++;
		XtSetArg(args[ac], XmNwidth, 170);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		label_timespan_1 = XmCreateLabel(bulletinBoard_timestepping, (char *)"label_timespan_1", args, ac);
		XtManageChild(label_timespan_1);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Navigation Modeling");
	ac++;
	XtSetArg(args[ac], XmNwidth, 492);
	ac++;
	XtSetArg(args[ac], XmNheight, 548);
	ac++;
	xmDialogShell_modeling = XmCreateDialogShell(mainWindow, (char *)"xmDialogShell_modeling", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(xmDialogShell_modeling, (char *)"Navigation Modeling", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNdialogTitle, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNautoUnmanage, False);
		ac++;
		XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
		ac++;
		XtSetArg(args[ac], XmNx, 1035);
		ac++;
		XtSetArg(args[ac], XmNy, 1063);
		ac++;
		XtSetArg(args[ac], XmNwidth, 492);
		ac++;
		XtSetArg(args[ac], XmNheight, 548);
		ac++;
		bulletinBoard_modeling =
		    XtCreateWidget((char *)"bulletinBoard_modeling", xmBulletinBoardWidgetClass, xmDialogShell_modeling, args, ac);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 =
		    (XmString)BX_CONVERT(bulletinBoard_modeling, (char *)"Gaussian Mean Time Window (seconds)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, 1);
		ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 1);
		ac++;
		XtSetArg(args[ac], XmNvalue, 100);
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
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 100);
		ac++;
		XtSetArg(args[ac], XmNwidth, 470);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		scale_meantimewindow = XmCreateScale(bulletinBoard_modeling, (char *)"scale_meantimewindow", args, ac);
		XtManageChild(scale_meantimewindow);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(scale_meantimewindow, XmNvalueChangedCallback, do_meantimewindow, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 70);
	ac++;
	XtSetArg(args[ac], XmNwidth, 470);
	ac++;
	XtSetArg(args[ac], XmNheight, 30);
	ac++;
	separator5 = XmCreateSeparator(bulletinBoard_modeling, (char *)"separator5", args, ac);
	XtManageChild(separator5);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_modeling, (char *)"Apply", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 30);
		ac++;
		XtSetArg(args[ac], XmNy, 390);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		pushButton_modeling_apply = XmCreatePushButton(bulletinBoard_modeling, (char *)"pushButton_modeling_apply", args, ac);
		XtManageChild(pushButton_modeling_apply);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_modeling_apply, XmNactivateCallback, do_modeling_apply, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 450);
	ac++;
	XtSetArg(args[ac], XmNwidth, 470);
	ac++;
	XtSetArg(args[ac], XmNheight, 30);
	ac++;
	separator4 = XmCreateSeparator(bulletinBoard_modeling, (char *)"separator4", args, ac);
	XtManageChild(separator4);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_modeling, (char *)"Acceleration:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 160);
		ac++;
		XtSetArg(args[ac], XmNy, 410);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		label_modeling_acceleration = XmCreateLabel(bulletinBoard_modeling, (char *)"label_modeling_acceleration", args, ac);
		XtManageChild(label_modeling_acceleration);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_modeling, (char *)"Speed Deviation:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 170);
		ac++;
		XtSetArg(args[ac], XmNy, 360);
		ac++;
		XtSetArg(args[ac], XmNwidth, 140);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		label_modeling_speed = XmCreateLabel(bulletinBoard_modeling, (char *)"label_modeling_speed", args, ac);
		XtManageChild(label_modeling_speed);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_modeling, (char *)"Inversion Penalty Weighting:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 330);
		ac++;
		XtSetArg(args[ac], XmNwidth, 220);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		label_modeling_inversion = XmCreateLabel(bulletinBoard_modeling, (char *)"label_modeling_inversion", args, ac);
		XtManageChild(label_modeling_inversion);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 320);
	ac++;
	XtSetArg(args[ac], XmNy, 410);
	ac++;
	XtSetArg(args[ac], XmNwidth, 140);
	ac++;
	XtSetArg(args[ac], XmNheight, 40);
	ac++;
	XtSetArg(
	    args[ac], XmNfontList,
	    BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textField_modeling_acceleration =
	    XmCreateTextField(bulletinBoard_modeling, (char *)"textField_modeling_acceleration", args, ac);
	XtManageChild(textField_modeling_acceleration);

	ac = 0;
	XtSetArg(args[ac], XmNx, 320);
	ac++;
	XtSetArg(args[ac], XmNy, 360);
	ac++;
	XtSetArg(args[ac], XmNwidth, 140);
	ac++;
	XtSetArg(args[ac], XmNheight, 40);
	ac++;
	XtSetArg(
	    args[ac], XmNfontList,
	    BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textField_modeling_speed = XmCreateTextField(bulletinBoard_modeling, (char *)"textField_modeling_speed", args, ac);
	XtManageChild(textField_modeling_speed);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_modeling, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 190);
		ac++;
		XtSetArg(args[ac], XmNy, 490);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		pushButton_modeling_dismiss = XmCreatePushButton(bulletinBoard_modeling, (char *)"pushButton_modeling_dismiss", args, ac);
		XtManageChild(pushButton_modeling_dismiss);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_modeling_dismiss, XmNactivateCallback, BxUnmanageCB, (XtPointer) "bulletinBoard_modeling");

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 310);
	ac++;
	XtSetArg(args[ac], XmNwidth, 470);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	separator3 = XmCreateSeparator(bulletinBoard_modeling, (char *)"separator3", args, ac);
	XtManageChild(separator3);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_modeling, (char *)"Dead Reckoning Latitude Drift (deg/hr)", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, -1000);
		ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 5);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 1000);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 250);
		ac++;
		XtSetArg(args[ac], XmNwidth, 470);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		scale_driftlat = XmCreateScale(bulletinBoard_modeling, (char *)"scale_driftlat", args, ac);
		XtManageChild(scale_driftlat);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(scale_driftlat, XmNvalueChangedCallback, do_driftlat, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_modeling, (char *)"Dead Reckoning Longitude Drift (deg/hr)", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, -1000);
		ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 5);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 1000);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 180);
		ac++;
		XtSetArg(args[ac], XmNwidth, 470);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		scale_driftlon = XmCreateScale(bulletinBoard_modeling, (char *)"scale_driftlon", args, ac);
		XtManageChild(scale_driftlon);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(scale_driftlon, XmNvalueChangedCallback, do_driftlon, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 160);
	ac++;
	XtSetArg(args[ac], XmNwidth, 470);
	ac++;
	XtSetArg(args[ac], XmNheight, 30);
	ac++;
	separator2 = XmCreateSeparator(bulletinBoard_modeling, (char *)"separator2", args, ac);
	XtManageChild(separator2);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_modeling, (char *)"Navigation Modeling Mode:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 200);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		label_modeling_mode = XmCreateLabel(bulletinBoard_modeling, (char *)"label_modeling_mode", args, ac);
		XtManageChild(label_modeling_mode);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNpacking, XmPACK_TIGHT);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 20);
	ac++;
	XtSetArg(args[ac], XmNy, 40);
	ac++;
	XtSetArg(args[ac], XmNwidth, 434);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNisHomogeneous, False);
	ac++;
	radioBox_modeling = XmCreateRadioBox(bulletinBoard_modeling, (char *)"radioBox_modeling", args, ac);
	XtManageChild(radioBox_modeling);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox_modeling, (char *)"Off", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 48);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(radioBox_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_modeling_off = XmCreateToggleButton(radioBox_modeling, (char *)"toggleButton_modeling_off", args, ac);
		XtManageChild(toggleButton_modeling_off);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_modeling_off, XmNvalueChangedCallback, do_model_mode, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox_modeling, (char *)"Gaussian Mean", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 135);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(radioBox_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_modeling_meanfilter =
		    XmCreateToggleButton(radioBox_modeling, (char *)"toggleButton_modeling_meanfilter", args, ac);
		XtManageChild(toggleButton_modeling_meanfilter);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_modeling_meanfilter, XmNvalueChangedCallback, do_model_mode, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox_modeling, (char *)"Dead Reckoning", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 143);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(radioBox_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_modeling_dr = XmCreateToggleButton(radioBox_modeling, (char *)"toggleButton_modeling_dr", args, ac);
		XtManageChild(toggleButton_modeling_dr);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_modeling_dr, XmNvalueChangedCallback, do_model_mode, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(radioBox_modeling, (char *)"Inversion", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 93);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(radioBox_modeling, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		toggleButton_modeling_inversion =
		    XmCreateToggleButton(radioBox_modeling, (char *)"toggleButton_modeling_inversion", args, ac);
		XtManageChild(toggleButton_modeling_inversion);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_modeling_inversion, XmNvalueChangedCallback, do_model_mode, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNwidth, 503);
	ac++;
	XtSetArg(args[ac], XmNheight, 126);
	ac++;
	xmDialogShell_useprevious = XmCreateDialogShell(mainWindow, (char *)"xmDialogShell_useprevious", args, ac);

	ac = 0;
	{
		XmString tmp0 =
		    (XmString)BX_CONVERT(xmDialogShell_useprevious, (char *)"Use previously edited navigation?", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNdialogTitle, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNautoUnmanage, True);
		ac++;
		XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
		ac++;
		XtSetArg(args[ac], XmNx, 1029);
		ac++;
		XtSetArg(args[ac], XmNy, 1274);
		ac++;
		XtSetArg(args[ac], XmNwidth, 503);
		ac++;
		XtSetArg(args[ac], XmNheight, 126);
		ac++;
		bulletinBoard_useprevious =
		    XtCreateWidget((char *)"bulletinBoard_useprevious", xmBulletinBoardWidgetClass, xmDialogShell_useprevious, args, ac);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_useprevious,
		                            (char *)":::t\"Previously edited navigation exists for the specified input file.\"\"Do you "
		                                    "want to use the previously edited navigation?\"",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 480);
		ac++;
		XtSetArg(args[ac], XmNheight, 50);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_useprevious, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		label_useprevious = XmCreateLabel(bulletinBoard_useprevious, (char *)"label_useprevious", args, ac);
		XtManageChild(label_useprevious);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_useprevious, (char *)"No", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 290);
		ac++;
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_useprevious, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		pushButton_useprevious_no = XmCreatePushButton(bulletinBoard_useprevious, (char *)"pushButton_useprevious_no", args, ac);
		XtManageChild(pushButton_useprevious_no);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_useprevious_no, XmNactivateCallback, do_useprevious_no, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_useprevious, (char *)"Yes", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 110);
		ac++;
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_useprevious, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		pushButton_useprevious_yes =
		    XmCreatePushButton(bulletinBoard_useprevious, (char *)"pushButton_useprevious_yes", args, ac);
		XtManageChild(pushButton_useprevious_yes);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_useprevious_yes, XmNactivateCallback, do_useprevious_yes, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Time Interpolation");
	ac++;
	XtSetArg(args[ac], XmNwidth, 307);
	ac++;
	XtSetArg(args[ac], XmNheight, 149);
	ac++;
	xmDialogShell_timeinterpolation = XmCreateDialogShell(mainWindow, (char *)"xmDialogShell_timeinterpolation", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNautoUnmanage, False);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 1127);
	ac++;
	XtSetArg(args[ac], XmNy, 1262);
	ac++;
	XtSetArg(args[ac], XmNwidth, 307);
	ac++;
	XtSetArg(args[ac], XmNheight, 149);
	ac++;
	bulletinBoard_timeinterpolation = XtCreateWidget((char *)"bulletinBoard_timeinterpolation", xmBulletinBoardWidgetClass,
	                                                 xmDialogShell_timeinterpolation, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_timeinterpolation,
		                            (char *)":::t\"Click \\\"Apply\\\" to interpolate duplicate \":t\"time stamps. Non-duplicate "
		                                    "time stamps \"\"will not be affected.\"",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 290);
		ac++;
		XtSetArg(args[ac], XmNheight, 70);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_timeinterpolation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		label_timeinterpolation = XmCreateLabel(bulletinBoard_timeinterpolation, (char *)"label_timeinterpolation", args, ac);
		XtManageChild(label_timeinterpolation);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_timeinterpolation, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 160);
		ac++;
		XtSetArg(args[ac], XmNy, 80);
		ac++;
		XtSetArg(args[ac], XmNwidth, 130);
		ac++;
		XtSetArg(args[ac], XmNheight, 50);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_timeinterpolation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_timeinterpolation_dismiss =
		    XmCreatePushButton(bulletinBoard_timeinterpolation, (char *)"pushButton_timeinterpolation_dismiss", args, ac);
		XtManageChild(pushButton_timeinterpolation_dismiss);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_timeinterpolation_dismiss, XmNactivateCallback, BxUnmanageCB,
	              (XtPointer) "bulletinBoard_timeinterpolation");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_timeinterpolation, (char *)"Apply", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 80);
		ac++;
		XtSetArg(args[ac], XmNwidth, 130);
		ac++;
		XtSetArg(args[ac], XmNheight, 50);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_timeinterpolation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_timeinterpolation_apply =
		    XmCreatePushButton(bulletinBoard_timeinterpolation, (char *)"pushButton_timeinterpolation_apply", args, ac);
		XtManageChild(pushButton_timeinterpolation_apply);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_timeinterpolation_apply, XmNactivateCallback, do_timeinterpolation_apply, (XtPointer)0);
	XtAddCallback(pushButton_timeinterpolation_apply, XmNactivateCallback, BxUnmanageCB,
	              (XtPointer) "bulletinBoard_timeinterpolation");

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Delete Bad Times");
	ac++;
	XtSetArg(args[ac], XmNwidth, 310);
	ac++;
	XtSetArg(args[ac], XmNheight, 149);
	ac++;
	dialogShell_deletebadtimetag = XmCreateDialogShell(mainWindow, (char *)"dialogShell_deletebadtimetag", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNautoUnmanage, False);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 1126);
	ac++;
	XtSetArg(args[ac], XmNy, 1262);
	ac++;
	XtSetArg(args[ac], XmNwidth, 310);
	ac++;
	XtSetArg(args[ac], XmNheight, 149);
	ac++;
	bulletinBoard_deletebadtimetag = XtCreateWidget((char *)"bulletinBoard_deletebadtimetag", xmBulletinBoardWidgetClass,
	                                                dialogShell_deletebadtimetag, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_deletebadtimetag,
		                            (char *)":::t\"Click \\\"Apply\\\" to delete duplicate \":t\"or reverse time stamps. "
		                                    "Non-duplicate \"\"time stamps will not be affected.\"",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 290);
		ac++;
		XtSetArg(args[ac], XmNheight, 70);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_deletebadtimetag, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		label_deletetimetag = XmCreateLabel(bulletinBoard_deletebadtimetag, (char *)"label_deletetimetag", args, ac);
		XtManageChild(label_deletetimetag);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_deletebadtimetag, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 160);
		ac++;
		XtSetArg(args[ac], XmNy, 80);
		ac++;
		XtSetArg(args[ac], XmNwidth, 130);
		ac++;
		XtSetArg(args[ac], XmNheight, 50);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_deletebadtimetag, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_deletebadtimetag_dismiss =
		    XmCreatePushButton(bulletinBoard_deletebadtimetag, (char *)"pushButton_deletebadtimetag_dismiss", args, ac);
		XtManageChild(pushButton_deletebadtimetag_dismiss);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_deletebadtimetag_dismiss, XmNactivateCallback, BxUnmanageCB,
	              (XtPointer) "bulletinBoard_deletebadtimetag");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(bulletinBoard_deletebadtimetag, (char *)"Apply", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 80);
		ac++;
		XtSetArg(args[ac], XmNwidth, 130);
		ac++;
		XtSetArg(args[ac], XmNheight, 50);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(bulletinBoard_deletebadtimetag, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_deletebadtimetag_apply =
		    XmCreatePushButton(bulletinBoard_deletebadtimetag, (char *)"pushButton_deletebadtimetag_apply", args, ac);
		XtManageChild(pushButton_deletebadtimetag_apply);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_deletebadtimetag_apply, XmNactivateCallback, do_deletebadtimetag_apply, (XtPointer)0);
	XtAddCallback(pushButton_deletebadtimetag_apply, XmNactivateCallback, BxUnmanageCB,
	              (XtPointer) "bulletinBoard_timeinterpolation");

	ac = 0;
	XtSetArg(args[ac], XmNwidth, 401);
	ac++;
	XtSetArg(args[ac], XmNheight, 174);
	ac++;
	xmDialogShell_offset = XmCreateDialogShell(mainWindow, (char *)"xmDialogShell_offset", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(xmDialogShell_offset, (char *)"Position Offset", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNdialogTitle, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
		ac++;
		XtSetArg(args[ac], XmNx, 1080);
		ac++;
		XtSetArg(args[ac], XmNy, 1250);
		ac++;
		XtSetArg(args[ac], XmNwidth, 401);
		ac++;
		XtSetArg(args[ac], XmNheight, 174);
		ac++;
		form_offset = XtCreateWidget((char *)"form_offset", xmFormWidgetClass, xmDialogShell_offset, args, ac);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(form_offset, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 210);
		ac++;
		XtSetArg(args[ac], XmNy, 120);
		ac++;
		XtSetArg(args[ac], XmNwidth, 118);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(form_offset, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_offset_dismiss = XmCreatePushButton(form_offset, (char *)"pushButton_offset_dismiss", args, ac);
		XtManageChild(pushButton_offset_dismiss);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_offset_dismiss, XmNactivateCallback, BxUnmanageCB, (XtPointer) "form_offset");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(form_offset, (char *)"Apply", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 50);
		ac++;
		XtSetArg(args[ac], XmNy, 120);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(form_offset, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		pushButton_offset_apply = XmCreatePushButton(form_offset, (char *)"pushButton_offset_apply", args, ac);
		XtManageChild(pushButton_offset_apply);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_offset_apply, XmNactivateCallback, do_offset_apply, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNx, 240);
	ac++;
	XtSetArg(args[ac], XmNy, 60);
	ac++;
	XtSetArg(args[ac], XmNwidth, 149);
	ac++;
	XtSetArg(args[ac], XmNheight, 40);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(form_offset, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textField_lat_offset = XmCreateTextField(form_offset, (char *)"textField_lat_offset", args, ac);
	XtManageChild(textField_lat_offset);

	ac = 0;
	XtSetArg(args[ac], XmNx, 240);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNwidth, 149);
	ac++;
	XtSetArg(args[ac], XmNheight, 40);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(form_offset, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
	if (argok)
		ac++;
	textField_lon_offset = XmCreateTextField(form_offset, (char *)"textField_lon_offset", args, ac);
	XtManageChild(textField_lon_offset);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(form_offset, (char *)"Latitude Offset (degrees):", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 0);
		ac++;
		XtSetArg(args[ac], XmNy, 60);
		ac++;
		XtSetArg(args[ac], XmNwidth, 230);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(form_offset, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		label_offset_lat = XmCreateLabel(form_offset, (char *)"label_offset_lat", args, ac);
		XtManageChild(label_offset_lat);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(form_offset, (char *)"Longitude Offset (degrees):", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 220);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(form_offset, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		label_offset_lon = XmCreateLabel(form_offset, (char *)"label_offset_lon", args, ac);
		XtManageChild(label_offset_lon);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNwidth, 343);
	ac++;
	XtSetArg(args[ac], XmNheight, 580);
	ac++;
	dialogShell_filelist = XmCreateDialogShell(mainWindow, (char *)"dialogShell_filelist", args, ac);

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
		XtSetArg(args[ac], XmNx, 1109);
		ac++;
		XtSetArg(args[ac], XmNy, 1047);
		ac++;
		XtSetArg(args[ac], XmNwidth, 343);
		ac++;
		XtSetArg(args[ac], XmNheight, 580);
		ac++;
		form_filelist = XtCreateWidget((char *)"form_filelist", xmFormWidgetClass, dialogShell_filelist, args, ac);

		XmStringFree((XmString)tmp0);
	}

	ac = 0;
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

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_filelist_remove, XmNactivateCallback, do_filelist_remove, (XtPointer)0);

	ac = 0;
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

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(pushButton_filelist_edit, XmNactivateCallback, do_editlistselection, (XtPointer)0);

	ac = 0;
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

		XmStringFree((XmString)tmp0);
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
		toggleButton_output_on_filelist =
		    XmCreateToggleButton(setting_output_filelist, (char *)"toggleButton_output_on_filelist", args, ac);
		XtManageChild(toggleButton_output_on_filelist);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_output_on_filelist, XmNvalueChangedCallback, do_toggle_output_on_filelist, (XtPointer)0);

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
		toggleButton_output_off_filelist =
		    XmCreateToggleButton(setting_output_filelist, (char *)"toggleButton_output_off_filelist", args, ac);
		XtManageChild(toggleButton_output_off_filelist);

		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(toggleButton_output_off_filelist, XmNvalueChangedCallback, do_toggle_output_off_filelist, (XtPointer)0);

	ac = 0;
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

		XmStringFree((XmString)tmp0);
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
	scrolledWindow_filelist = XmCreateScrolledWindow(form_filelist, (char *)"scrolledWindow_filelist", args, ac);
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

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 14);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 210);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 73);
	ac++;
	XtSetValues(pushButton_offset_dismiss, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 14);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 50);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 50);
	ac++;
	XtSetArg(args[ac], XmNrightWidget, pushButton_offset_dismiss);
	ac++;
	XtSetValues(pushButton_offset_apply, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 0);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 240);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 12);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNtopWidget, textField_lon_offset);
	ac++;
	XtSetValues(textField_lat_offset, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 12);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNleftWidget, label_offset_lon);
	ac++;
	XtSetValues(textField_lon_offset, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 0);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 0);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNtopWidget, label_offset_lon);
	ac++;
	XtSetValues(label_offset_lat, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetValues(label_offset_lon, args, ac);

	return (mainWindow);
}
