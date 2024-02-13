#ifndef QOOM_THREAD
#define QOOM_THREAD

//Used by Monitor Thread
extern HANDLE g_hMMF;
extern HANDLE g_hReadEvent;
extern HANDLE g_hWriteEvent;
extern HANDLE g_hThreadEvent;

BOOL InitThread(LPVOID lpParam);
BOOL ExitThread();

#endif
