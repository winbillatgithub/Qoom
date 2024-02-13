; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CQoomDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "qoom.h"
LastPage=0

ClassCount=8
Class1=CHyperLink
Class2=CQoomApp
Class3=CAboutDlg
Class4=CQoomDlg
Class5=CXTabCtrl

ResourceCount=10
Resource1=IDD_EXIT_OPTION
Resource2=IDR_MENU_CLOSE
Class6=CQQParentWnd
Resource3=IDD_ABOUTBOX
Class7=CTrayDialog
Resource4=IDD_QOOM_DIALOG
Class8=CExitOptDlg
Resource5=IDR_MENU_TRAY
Resource6=IDD_QOOM_DIALOG (English (U.S.))
Resource7=IDD_EXIT_OPTION (English (U.S.))
Resource8=IDD_ABOUTBOX (English (U.S.))
Resource9=IDR_MENU_CLOSE (English (U.S.))
Resource10=IDR_MENU_TRAY (English (U.S.))

[CLS:CHyperLink]
Type=0
BaseClass=CStatic
HeaderFile=HyperLink.h
ImplementationFile=HyperLink.cpp

[CLS:CQoomApp]
Type=0
BaseClass=CWinApp
HeaderFile=Qoom.h
ImplementationFile=Qoom.cpp
Filter=N

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=QoomDlg.cpp
ImplementationFile=QoomDlg.cpp
Filter=D
VirtualFilter=dWC

[CLS:CQoomDlg]
Type=0
BaseClass=CDialog
HeaderFile=QoomDlg.h
ImplementationFile=QoomDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=ID_BTN_HIDEMSN

[CLS:CXTabCtrl]
Type=0
BaseClass=CTabCtrl
HeaderFile=XTabCtrl.h
ImplementationFile=XTabCtrl.cpp

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=6
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC_MAIL,static,1342308352
Control6=IDC_STATIC_URL,static,1342308352

[DLG:IDD_QOOM_DIALOG]
Type=1
Class=CQoomDlg
ControlCount=11
Control1=IDC_STATIC_EDIT_SEND,static,1082261504
Control2=IDOK,button,1342242817
Control3=ID_BTN_HIDEQQ,button,1342242816
Control4=ID_BTN_HIDEMSN,button,1342242816
Control5=IDCANCEL,button,1342242816
Control6=IDC_TAB_QQ,SysTabControl32,1342193920
Control7=IDC_TAB_MSN,SysTabControl32,1342193922
Control8=IDC_STATIC_EDIT,static,1342308352
Control9=IDC_BTN_REFRESH,button,1073741824
Control10=IDC_STATIC_CUR_QQ,static,1342177294
Control11=IDC_STATIC_CUR_MSN,static,1342177294

[MNU:IDR_MENU_CLOSE]
Type=1
Class=?
Command1=ID_CLOSE_CHAT
CommandCount=1

[CLS:CQQParentWnd]
Type=0
HeaderFile=QQParentWnd.h
ImplementationFile=QQParentWnd.cpp
BaseClass=CFrameWnd
Filter=T
VirtualFilter=fWC
LastObject=CQQParentWnd

[MNU:IDR_MENU_TRAY]
Type=1
Class=?
Command1=IDM_OPEN_QOOM
Command2=IDM_QOOM_EXIT
CommandCount=2

[CLS:CTrayDialog]
Type=0
HeaderFile=TrayDialog.h
ImplementationFile=TrayDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=CTrayDialog
VirtualFilter=dWC

[DLG:IDD_EXIT_OPTION]
Type=1
Class=CExitOptDlg
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDC_CHECK_QQ,button,1342242819
Control3=IDC_CHECK_MSN,button,1342242819
Control4=ID_SELECT_ALL,button,1342242816
Control5=ID_SELECT_NONE,button,1342242816
Control6=IDC_STATIC,static,1342308352

[CLS:CExitOptDlg]
Type=0
HeaderFile=ExitOptDlg.h
ImplementationFile=ExitOptDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDM_QOOM_EXIT

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=?
ControlCount=6
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC_MAIL,static,1342308352
Control6=IDC_STATIC_URL,static,1342308352

[DLG:IDD_QOOM_DIALOG (English (U.S.))]
Type=1
Class=CQoomDlg
ControlCount=11
Control1=IDC_STATIC_EDIT_SEND,static,1082261504
Control2=IDOK,button,1342242817
Control3=ID_BTN_HIDEQQ,button,1342242816
Control4=ID_BTN_HIDEMSN,button,1342242816
Control5=IDCANCEL,button,1342242816
Control6=IDC_TAB_QQ,SysTabControl32,1342193920
Control7=IDC_TAB_MSN,SysTabControl32,1342193922
Control8=IDC_STATIC_EDIT,static,1342308352
Control9=IDC_BTN_REFRESH,button,1073741824
Control10=IDC_STATIC_CUR_QQ,static,1342177294
Control11=IDC_STATIC_CUR_MSN,static,1342177294

[DLG:IDD_EXIT_OPTION (English (U.S.))]
Type=1
Class=?
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDC_CHECK_QQ,button,1342242819
Control3=IDC_CHECK_MSN,button,1342242819
Control4=ID_SELECT_ALL,button,1342242816
Control5=ID_SELECT_NONE,button,1342242816
Control6=IDC_STATIC,static,1342308352

[MNU:IDR_MENU_CLOSE (English (U.S.))]
Type=1
Class=?
Command1=ID_CLOSE_CHAT
CommandCount=1

[MNU:IDR_MENU_TRAY (English (U.S.))]
Type=1
Class=?
Command1=IDM_OPEN_QOOM
Command2=IDM_QOOM_EXIT
CommandCount=2

