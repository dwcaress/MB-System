#ifndef MBVIEW_MB3DSiteList_H_
#define MBVIEW_MB3DSiteList_H_

typedef struct _MB3DSiteListData {
	Widget MB3DSiteList;
	Widget mbview_pushButton_sitelist_dismiss;
	Widget mbview_pushButton_sitelist_delete;
	Widget mbview_sitelist_label;
	Widget mbview_scrolledWindow_sitelist;
	Widget mbview_list_sitelist;
} MB3DSiteListData;

typedef struct _MB3DSiteListData *MB3DSiteListDataPtr;

MB3DSiteListDataPtr MB3DSiteListCreate(
    MB3DSiteListDataPtr, Widget, String, ArgList, Cardinal);

#endif  // MBVIEW_MB3DSiteList_H_
