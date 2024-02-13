#ifndef TXT_HANDLER
#define TXT_HANDLER

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
#include "myTextServ.h"

typedef DWORD (CALLBACK *EDITSTREAMCALLBACK)(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

typedef struct _editstream
{
	DWORD dwCookie;		/* user value passed to callback as first parameter */
	DWORD dwError;		/* last error */
	EDITSTREAMCALLBACK pfnCallback;
} EDITSTREAM;
#define EM_STREAMOUT			(WM_USER + 74)
/* stream formats */

#define SF_TEXT			0x0001
#define SF_RTF			0x0002
#define SF_RTFNOOBJS	0x0003		/* outbound only */
#define SF_TEXTIZED		0x0004		/* outbound only */
#define SF_UNICODE		0x0010		/* Unicode file of some kind */



BOOL GeneralTxtHandler(ITextServices* lpTs, LPVOID pView, DWORD dwIndex);

#endif
