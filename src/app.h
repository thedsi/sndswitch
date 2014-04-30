#pragma once


extern "C" extern char __ImageBase;
#define APP_INSTANCE ((HINSTANCE)&__ImageBase)


#define APP_CLASSNAME L"sndswitch"
#define APP_MUTEXNAME L"{9124B8E0-6F43-4C07-AABC-BF809BD47290}"
#define APP_NAME      L"sndswitch"
#define APP_DESC      L"sndswitch"


#define APPMSG_TRAY			  (WM_APP + 0)
#define APPMSG_SND_CHANGE	  (WM_APP + 1)


#define IDM_EXIT 1
#define IDM_DEVICE_START 100


extern HWND		ghWnd;
extern HICON	ghAppIcon;