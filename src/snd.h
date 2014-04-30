#pragma once

#include "util.h"


class PlaybackDevice
{
private:
	void FreeIcon();

public:
	PlaybackDevice() = default;
	~PlaybackDevice()
	{
		FreeIcon();
	}

	WideString id;
	WideString name;
	HICON	   icon = NULL;

	void Set(IMMDevice *pDev);
};


class PlaybackDeviceList
{
public:
	PlaybackDevice *pbarray = nullptr;
	UINT pbcount = 0;
	UINT idxdef = 0;

	PlaybackDeviceList();
	~PlaybackDeviceList();
};


bool SndInit();
void SndUninit();
bool SndGetDefault(PlaybackDevice &pb);
void SndSetDefault(LPCWSTR id);