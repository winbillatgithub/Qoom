#ifndef QQPARENT
#define QQPARENT

//#include <windows.h>
#include "stdafx.h"

ATOM				RegisterQQParentClass(HINSTANCE hInstance = NULL);
HWND				InitQQParent(HINSTANCE hInst = NULL, int nCmdShow = SW_HIDE);
LRESULT CALLBACK	QQParentWndProc(HWND, UINT, WPARAM, LPARAM);

#endif