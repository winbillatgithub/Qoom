// stdafx.cpp : source file that includes just the standard includes
//	Qoom.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

HWND g_hChatSendEdit; 
HWND g_hMainDlg;
HWND g_hQQParent;

HWND g_hChatHisEditQQ[MAX_QQ_CONCUR_CHAT]; 
HWND g_hChatHisEditMSN[MAX_MSN_CONCUR_CHAT]; 

// For the history chat content.
DWORD g_dwActiveHis;
BOOL  g_bMSNActiveHis;

// For sending message
DWORD g_dwActiveSend;
BOOL g_bMSNActiveSend;

// For corresponding tabctl and data index.
DWORD g_hTabIndexQQ[MAX_QQ_CONCUR_CHAT]; 
DWORD g_hTabIndexMSN[MAX_MSN_CONCUR_CHAT]; 

// For multi-refresh
DWORD g_bHisChangedQQ[MAX_QQ_CONCUR_CHAT];
DWORD g_bHisChangedMSN[MAX_MSN_CONCUR_CHAT];

// Show/Hide window
BOOL  g_bMSNShowed;
BOOL  g_bQQShowed;
