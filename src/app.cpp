#include "stdafx.h"
#include "app.h"
#include "tray.h"
#include "snd.h"
#include "resource.h"


HWND	ghWnd;
HICON	ghAppIcon;


static int	AppMain();
static bool AppInitWindow();
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


#ifdef _DEBUG
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	return AppMain();
}
#else
extern "C" void APIENTRY AppStart()
{
	ExitProcess((UINT) AppMain());
}
#endif


static int AppMain()
{
	if (!IsWindows7SP1OrGreater())
	{
		MessageBox(NULL, L"At least Windows 7 SP1 required", APP_NAME, MB_ICONERROR);
		return EXIT_FAILURE;
	}
	if (!CreateMutex(NULL, TRUE, APP_MUTEXNAME) || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL, L"Already running", APP_NAME, MB_ICONINFORMATION);
		return EXIT_FAILURE;
	}

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	ghAppIcon = (HICON)LoadImage(APP_INSTANCE, MAKEINTRESOURCE(IDI_SNDSWITCH), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);

	if (!AppInitWindow())
	{
		MessageBox(NULL, L"App init failed", APP_NAME, MB_ICONERROR);
		return EXIT_FAILURE;
	}

	//Shrink working set
	SetProcessWorkingSetSizeEx(GetCurrentProcess(),
		(SIZE_T) -1, (SIZE_T) -1, QUOTA_LIMITS_HARDWS_MIN_DISABLE);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CoUninitialize();
	return (int) msg.wParam;
}


static bool AppInitWindow()
{
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = APP_INSTANCE;
	wcex.lpszClassName = APP_CLASSNAME;
	ATOM cls = RegisterClassEx(&wcex);
	if (!cls) return false;

	return CreateWindowEx(0, MAKEINTATOM(cls), NULL, 0,
		0, 0, 0, 0, NULL, NULL, APP_INSTANCE, NULL) != NULL;
}


static void AppShowMenu(int x, int y)
{
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		PlaybackDeviceList slist;
		MENUITEMINFO mii = { sizeof(mii) };
		if (slist.pbcount)
		{
			mii.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
			for (unsigned i = 0; i < slist.pbcount; i++)
			{
				mii.fState = (i == slist.idxdef) ? (MFS_CHECKED | MFS_DISABLED) : MFS_UNCHECKED;
				mii.dwTypeData = (LPWSTR) slist.pbarray[i].name.GetString();
				mii.wID = IDM_DEVICE_START + i;
				InsertMenuItem(hMenu, (UINT) -1, TRUE, &mii);
			}
		}
		else
		{
			mii.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
			mii.fState = MFS_DISABLED;
			mii.dwTypeData = L"(no devices)";
			mii.wID = 0;
			InsertMenuItem(hMenu, (UINT) -1, TRUE, &mii);
		}
		mii.fMask = MIIM_TYPE;
		mii.fType = MFT_SEPARATOR;
		InsertMenuItem(hMenu, (UINT)-1, TRUE, &mii);
		mii.fMask = MIIM_STRING | MIIM_ID;
		mii.dwTypeData = L"Exit";
		mii.wID = IDM_EXIT;
		InsertMenuItem(hMenu, (UINT)-1, TRUE, &mii);
		SetForegroundWindow(ghWnd);
		UINT ret = (UINT)TrackPopupMenuEx(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON, x, y, ghWnd, NULL);
		if (ret == IDM_EXIT)
		{
			PostMessage(ghWnd, WM_CLOSE, 0, 0);
		}
		else if (ret >= IDM_DEVICE_START)
		{
			unsigned index = ret - IDM_DEVICE_START;
			SndSetDefault(slist.pbarray[index].id);
		}
		DestroyMenu(hMenu);
	}
}


static void AppSetNextDevice()
{
	PlaybackDeviceList slist;
	if (slist.pbcount > 1)
	{
		UINT n = slist.idxdef + 1;
		if (n == slist.pbcount) n = 0;
		SndSetDefault(slist.pbarray[n].id);
	}
}


static void AppUpdateTray()
{
	PlaybackDevice pb;
	if (SndGetDefault(pb))
	{
		TrayUpdateIcon(pb.icon, pb.name);
	}
	else
	{
		TrayUpdateIcon(NULL, APP_DESC);
	}
}


static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		ghWnd = hWnd;
		if (!TrayAddIcon())	return -1;
		if (!SndInit())		return -1;
		AppUpdateTray();
		return 0;

	case APPMSG_TRAY:
		switch (LOWORD(lParam))
		{
		case NIN_KEYSELECT:
		case WM_LBUTTONDBLCLK:
			AppSetNextDevice();
			break;
		//case NIN_SELECT:
		case WM_CONTEXTMENU:
			AppShowMenu(LOWORD(wParam), HIWORD(wParam));
			break;
		}
		return 0;
	case APPMSG_SND_CHANGE:
		AppUpdateTray();
		return 0;
	case WM_DESTROY:
		SndUninit();
		TrayRemoveIcon();
		PostQuitMessage(0);
		return 0;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
