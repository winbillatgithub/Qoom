#ifndef MY_RICHEDITOLE
#define MY_RICHEDITOLE

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

#include "MyTextServ.h"

#define EM_GETOLEINTERFACE		(WM_USER + 60)
typedef struct _reobject
{
	DWORD			cbStruct;			// Size of structure
	LONG			cp;					// Character position of object
	CLSID			clsid;				// Class ID of object
	LPOLEOBJECT		poleobj;			// OLE object interface
	LPSTORAGE		pstg;				// Associated storage interface
	LPOLECLIENTSITE	polesite;			// Associated client site interface
	SIZEL			sizel;				// Size of object (may be 0,0)
	DWORD			dvaspect;			// Display aspect to use
	DWORD			dwFlags;			// Object status flags
	DWORD			dwUser;				// Dword for user's use
} REOBJECT;

typedef struct _charrange
{
	LONG	cpMin;
	LONG	cpMax;
} CHARRANGE;

class IRichEditOle : public IUnknown
{
public:
    // *** IUnknown methods ***
    virtual HRESULT  __stdcall QueryInterface(REFIID riid, LPVOID FAR * lplpObj) = 0;
    virtual ULONG    __stdcall AddRef()  = 0;
    virtual ULONG    __stdcall Release() = 0;
public:
    // *** IRichEditOle methods ***
    virtual HRESULT  __stdcall GetClientSite(LPOLECLIENTSITE FAR * lplpolesite) = 0;
	virtual LONG     __stdcall GetObjectCount() = 0;
	virtual LONG     __stdcall GetLinkCount() = 0;
	virtual HRESULT  __stdcall GetObject(LONG iob, REOBJECT FAR * lpreobject, DWORD dwFlags) = 0;
    virtual HRESULT  __stdcall InsertObject(REOBJECT FAR * lpreobject) = 0;
	virtual HRESULT  __stdcall ConvertObject(LONG iob, REFCLSID rclsidNew, LPCSTR lpstrUserTypeNew) = 0;
	virtual HRESULT  __stdcall ActivateAs(REFCLSID rclsid, REFCLSID rclsidAs) = 0;
	virtual HRESULT  __stdcall SetHostNames(LPCSTR lpstrContainerApp, LPCSTR lpstrContainerObj) = 0;
	virtual HRESULT  __stdcall SetLinkAvailable(LONG iob, BOOL fAvailable) = 0;
	virtual HRESULT  __stdcall SetDvaspect(LONG iob, DWORD dvaspect) = 0;
	virtual HRESULT  __stdcall HandsOffStorage(LONG iob) = 0;
	virtual HRESULT  __stdcall SaveCompleted(LONG iob, LPSTORAGE lpstg) = 0;
	virtual HRESULT  __stdcall InPlaceDeactivate() = 0;
	virtual HRESULT  __stdcall ContextSensitiveHelp(BOOL fEnterMode) = 0;
	virtual HRESULT  __stdcall GetClipboardData(CHARRANGE FAR * lpchrg, DWORD reco,
									LPDATAOBJECT FAR * lplpdataobj) = 0;
	virtual HRESULT  __stdcall ImportDataObject(LPDATAOBJECT lpdataobj,
									CLIPFORMAT cf, HGLOBAL hMetaPict) = 0;
};

// Flags to specify which interfaces should be returned in the structure above
#define REO_GETOBJ_NO_INTERFACES	(0x00000000L)
#define REO_GETOBJ_POLEOBJ			(0x00000001L)
#define REO_GETOBJ_PSTG				(0x00000002L)
#define REO_GETOBJ_POLESITE			(0x00000004L)
#define REO_GETOBJ_ALL_INTERFACES	(0x00000007L)

#endif