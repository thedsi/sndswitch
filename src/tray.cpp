#include "stdafx.h"
#include "app.h"
#include "tray.h"


bool TrayAddIcon()
{
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = ghWnd;
	nid.hIcon = ghAppIcon;
	nid.uCallbackMessage = APPMSG_TRAY;
	lstrcpyW(nid.szTip, APP_DESC);
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
	nid.uID = 0;
	if (!Shell_NotifyIcon(NIM_ADD, &nid)) return false;
	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid) != FALSE;
}


void TrayRemoveIcon()
{
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = ghWnd;
	nid.uID = 0;
	Shell_NotifyIcon(NIM_DELETE, &nid);
}


void TrayUpdateIcon(HICON icon, LPCWSTR desc)
{
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = ghWnd;
	nid.hIcon = icon ? icon : ghAppIcon;
	lstrcpynW(nid.szTip, desc, sizeof(nid.szTip) / sizeof(nid.szTip[0]));
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP;
	nid.uID = 0;
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}
