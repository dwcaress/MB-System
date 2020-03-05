#ifndef MBVIEW_MB3DNavList_H_
#define MBVIEW_MB3DNavList_H_

typedef struct _MB3DNavListData {
	Widget MB3DNavList;
	Widget mbview_navlist_label;
	Widget mbview_pushButton_navlist_delete;
	Widget mbview_pushButton_navlist_dismiss;
	Widget mbview_scrolledWindow_navlist;
	Widget mbview_list_navlist;
} MB3DNavListData;

typedef struct _MB3DNavListData *MB3DNavListDataPtr;

MB3DNavListDataPtr MB3DNavListCreate(MB3DNavListDataPtr, Widget, String, ArgList, Cardinal);

#endif  // MBVIEW_MB3DNavList_H_
