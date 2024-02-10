#ifndef RECV_WIN
#define RECV_WIN

/************************************
  REVISION LOG ENTRY
  Revision By: Zhang, Zhefu
  E-mail: codetiger@hotmail.com
  Revised on 10/2/2003 
  Comment: This is program code accompanying "COM Interface Hooking and Its Application"
           written by Zhefu Zhang posted on www.codeguru.com 
           You are free to reuse the code on the base of keeping this comment
		   All Right Reserved by author		   
 ************************************/

#include <windows.h>

ATOM				MyRegisterClass(HINSTANCE hInstance = NULL);
HWND				InitInstance(HINSTANCE hInst = NULL, int nCmdShow = SW_HIDE);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

#endif