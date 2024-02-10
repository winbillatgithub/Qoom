#include "QQParent.h"
#include "StdAfx.h"


ATOM RegisterQQParentClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= 0; //CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)QQParentWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL; //LoadIcon(hInstance, (LPCTSTR)IDI_DSA);
	wcex.hCursor		= NULL; //LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; // (LPCSTR)IDC_DSA;
	wcex.lpszClassName	= QQ_PARENT_WND_CLASS; //szWindowClass;
	wcex.hIconSm		= NULL; //LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}


HWND InitQQParent(HINSTANCE hInstance, int nCmdShow)
{
   return CreateWindowEx(NULL, //WS_EX_TOOLWINDOW,
	   QQ_PARENT_WND_CLASS, 
	   QQ_PARENT_WND_TITLE,
	   WS_OVERLAPPED,
      CW_USEDEFAULT,
	  0, 
	  CW_USEDEFAULT, 
	  0, NULL, NULL, 
	  NULL, NULL);
}

LRESULT CALLBACK QQParentWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) 
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
