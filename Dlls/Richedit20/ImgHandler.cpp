#include "ImgHandler.h"
#include "StdAfx.h"
#include "RichEd20.h"
#include "myRichEditOle.h"


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

#include <ocidl.h> //IPicture
#define WRITE_DISK

/*
     * Predefined Clipboard Formats
#define CF_TEXT             1
#define CF_BITMAP           2
#define CF_METAFILEPICT     3
#define CF_SYLK             4
#define CF_DIF              5
#define CF_TIFF             6
#define CF_OEMTEXT          7
#define CF_DIB              8
#define CF_PALETTE          9
#define CF_PENDATA          10
#define CF_RIFF             11
#define CF_WAVE             12
#define CF_UNICODETEXT      13
#define CF_ENHMETAFILE      14
#if(WINVER >= 0x0400)
#define CF_HDROP            15
#define CF_LOCALE           16
#define CF_MAX              17
#endif // WINVER >= 0x0400 
#define CF_OWNERDISPLAY     0x0080
#define CF_DSPTEXT          0x0081
#define CF_DSPBITMAP        0x0082
#define CF_DSPMETAFILEPICT  0x0083
#define CF_DSPENHMETAFILE   0x008E

  typedef struct tagFORMATETC 
{ 
    CLIPFORMAT      cfFormat; 
    DVTARGETDEVICE  *ptd; 
    DWORD           dwAspect; 
    LONG            lindex; 
    DWORD           tymed; 
}FORMATETC, *LPFORMATETC;

  typedef [transmit_as(long)] enum tagTYMED 
{ 
    TYMED_HGLOBAL     = 1, 
    TYMED_FILE        = 2, 
    TYMED_ISTREAM     = 4, 
    TYMED_ISTORAGE    = 8, 
    TYMED_GDI         = 16, 
    TYMED_MFPICT      = 32, 
    TYMED_ENHMF       = 64, 
    TYMED_NULL        = 0 
} TYMED; 


  // Mapping Modes 
#define MM_TEXT             1
#define MM_LOMETRIC         2
#define MM_HIMETRIC         3
#define MM_LOENGLISH        4
#define MM_HIENGLISH        5
#define MM_TWIPS            6
#define MM_ISOTROPIC        7
#define MM_ANISOTROPIC      8 //MSN

*/
void ParseDataObject(IDataObject* lpDataObject)
{
	DWORD dwCF[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
	                0x0080, 0x0081, 0x0082, 0x0083, 0x008E};
	DWORD dwTM[] = {1, 2, 4, 8, 16, 32, 64, 0};
	int dimCF = sizeof(dwCF)/sizeof(dwCF[0]);
	int dimTM = sizeof(dwTM)/sizeof(dwTM[0]);
	for(int i = 0; i < dimCF; i++)
	{
		for(int j = 0; j < dimTM; j++)
		{
			FORMATETC fm; //in
		    fm.cfFormat = dwCF[i];			// Clipboard format 
	        fm.ptd = NULL;							// Target Device = Screen
	        fm.dwAspect = DVASPECT_CONTENT;			// Level of detail = Full content
	        fm.lindex = -1;							// Index = Not applicaple
	        fm.tymed = dwTM[j];
            STGMEDIUM stgm; //out
		    HRESULT hr =  lpDataObject->GetData(&fm, &stgm);
		    if(FAILED(hr)) 
			{
				continue;
			}
			else
			{
				PopMsg(_T("%d, %d"), i, j);
			}
		}
	}
}


BOOL GeneralObjHandler(IRichEditOle* pReo)
{
	int nCount = pReo->GetObjectCount();
	for(int i = nCount-1; i >= 0; i--) //Intentional Reverse Query for Pos
	{
		REOBJECT* ro = new REOBJECT;
		ro->cbStruct = sizeof(REOBJECT);
		HRESULT hr = pReo->GetObject(i, ro, REO_GETOBJ_ALL_INTERFACES); //REO_GETOBJ_POLEOBJ); //REO_GETOBJ_ALL_INTERFACES);
		if(FAILED(hr)) 
		{
			ReportErrEx(_T("GeneralObjHandler GetObject Failed %d"), i);
			continue;
		}

		IDataObject* lpDataObject;
		//LPDATAOBJECT lpDataObject;
	    hr = (ro->poleobj)->QueryInterface(IID_IDataObject, (void **)&lpDataObject);
        if(FAILED(hr)) 
		{
			::ReportErrEx(_T("Get IID_IDataObject Failed %d"), i);
			continue;
		}
         
		STGMEDIUM stgm; //out
		FORMATETC fm; //in
		fm.cfFormat = CF_METAFILEPICT;			// Clipboard format 
	    fm.ptd = NULL;							// Target Device = Screen
	    fm.dwAspect = DVASPECT_CONTENT;			// Level of detail = Full content
	    fm.lindex = -1;							// Index = Not applicaple
	    fm.tymed = TYMED_MFPICT;

		hr =  lpDataObject->GetData(&fm, &stgm);
		if(FAILED(hr)) 
		{
			continue;
		}

		//TYMED_MFPICT: 
        //The storage medium is a metafile (HMETAFILE). Use the Windows or WIN32 functions
		//to access the metafile's data. If the STGMEDIUM punkForRelease member is NULL, 
		//the destination process should use DeleteMetaFile to delete the bitmap. 
        //hMetaFilePict
        //Metafile handle. The tymed member is TYMED_MFPICT.
        HMETAFILEPICT  hMetaFilePict = stgm.hMetaFilePict;
        LPMETAFILEPICT pMFP = (LPMETAFILEPICT) GlobalLock (hMetaFilePict);
		//PopMsg(_T("%d %d %d"), pMFP->mm, pMFP->xExt, pMFP->yExt);
		int cx = 19; //pMFP->xExt;
		int cy = 19; //pMFP->yExt;
		HWND hWnd = ::GetDesktopWindow();
		HDC hDC = ::GetDC(hWnd);
		HDC hMemDC = ::CreateCompatibleDC(hDC);
		HBITMAP hMemBmp = ::CreateCompatibleBitmap(hDC, cx, cy);
        HBITMAP hPrevBmp = (HBITMAP)::SelectObject(hMemDC, hMemBmp);
        ::PlayMetaFile(hMemDC, pMFP->hMF);
		CopyMetaFile(pMFP->hMF, _T("C:\\x.wmf")); //Just write it as WMF file
         
#ifdef WRITE_DISK
		TCHAR szIndex[32];
#ifndef _UNICODE
    sprintf(szIndex, _T("C:\\%02d"), i); 
#else
	swprintf(szIndex, _T("C:\\%02d"), i); 
#endif
		TCHAR szFilename[MAX_PATH];
		::CreateFileName(szFilename, szIndex, _T(".bmp"));
		HANDLE hFile = ::CreateFile(szFilename, GENERIC_WRITE, 0,
		        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	    if(hFile == INVALID_HANDLE_VALUE)
		{
			continue;
		}
		DWORD dwWritten;
#endif

		//need file header 
		BITMAPFILEHEADER bmfh;
	    bmfh.bfType = 0x4d42;  // 'BM'
		int  nColorTableEntries = 0;
	    int nSizeHdr = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColorTableEntries;
	    bmfh.bfSize = 0;
        bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
		   sizeof(RGBQUAD) * nColorTableEntries;	
#ifdef WRITE_DISK
		::WriteFile(hFile, (LPVOID)&bmfh, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
#endif

		BITMAP			bm;
    	// get bitmap information
		::GetObject(hMemBmp, sizeof(bm), &bm);
    	int nBitCount = bm.bmBitsPixel; //TRUE coloe ONLY!!! Warning!!!
	    BITMAPINFOHEADER* lpBMIH = (LPBITMAPINFOHEADER) new 
		  char[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColorTableEntries];
 	    lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
	    lpBMIH->biWidth = bm.bmWidth;
	    lpBMIH->biHeight = bm.bmHeight;
	    lpBMIH->biPlanes = 1;
	    lpBMIH->biBitCount = nBitCount;
    	lpBMIH->biCompression = BI_RGB;
	    lpBMIH->biSizeImage = 0;
	    lpBMIH->biXPelsPerMeter = 0;
	    lpBMIH->biYPelsPerMeter = 0;
	    lpBMIH->biClrUsed = nColorTableEntries;
	    lpBMIH->biClrImportant = nColorTableEntries;

		DWORD dwCount = ((DWORD) lpBMIH->biWidth * lpBMIH->biBitCount) / 32;
		if(((DWORD) lpBMIH->biWidth * lpBMIH->biBitCount) % 32) {
			dwCount++;
		}
	    dwCount *= 4;
	    dwCount = dwCount * lpBMIH->biHeight;	
        LPVOID lpImage = ::VirtualAlloc(NULL, dwCount, MEM_COMMIT, PAGE_READWRITE);
		BOOL result = GetDIBits(hMemDC, (HBITMAP)hMemBmp,
				0L,		// start scan line
				(DWORD)bm.bmHeight,	// # of scan lines
				(LPBYTE)lpImage, 			// address for bitmap bits
				(LPBITMAPINFO)lpBMIH,		// address of bitmapinfo
				(DWORD)DIB_RGB_COLORS  // use rgb for color table
	            );		

#ifdef WRITE_DISK
        ::WriteFile(hFile, lpBMIH, sizeof(BITMAPINFOHEADER), &dwWritten, NULL); 
        ::WriteFile(hFile, lpImage, dwCount, &dwWritten, NULL);
#endif
		::VirtualFree(lpImage, 0, MEM_RELEASE);

#ifdef WRITE_DISK
		::CloseHandle(hFile);
#endif	

        ::SelectObject(hMemDC, hPrevBmp);
		::DeleteObject(hMemBmp);
		::DeleteDC(hMemDC);
		::ReleaseDC(hWnd, hDC);
		::GlobalUnlock(hMetaFilePict);

		ro->poleobj->Release(); //GetObject Called AddRef so Release here
		delete ro;
	}
	return TRUE;
}


