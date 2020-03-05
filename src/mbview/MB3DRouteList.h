#ifndef MBVIEW_MB3DRouteList_H_
#define MBVIEW_MB3DRouteList_H_

typedef struct _MB3DRouteListData {
	Widget MB3DRouteList;
	Widget mbview_routelist_label;
	Widget mbview_pushButton_routelist_delete;
	Widget mbview_pushButton_routelist_dismiss;
	Widget mbview_scrolledWindow_routelist;
	Widget mbview_list_routelist;
} MB3DRouteListData;

typedef struct _MB3DRouteListData *MB3DRouteListDataPtr;

MB3DRouteListDataPtr MB3DRouteListCreate(
    MB3DRouteListDataPtr, Widget, String, ArgList, Cardinal);

#endif  // MBVIEW_MB3DRouteList_H_
