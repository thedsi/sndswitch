#include "stdafx.h"
#include "app.h"
#include "snd.h"


static IMMDeviceEnumerator*	gpSndEnumerator;


class NotificationClient : public IMMNotificationClient
{
public:
	STDMETHOD(OnDeviceStateChanged)(LPCWSTR pwstrDeviceId, DWORD dwNewState){ return S_OK; }
	STDMETHOD(OnDeviceAdded)(LPCWSTR pwstrDeviceId){ return S_OK; }
	STDMETHOD(OnDeviceRemoved)(LPCWSTR pwstrDeviceId){ return S_OK; }
	STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
	{
		if (flow == eRender && role == eConsole)
		{
			PostMessage(ghWnd, APPMSG_SND_CHANGE, 0, 0);
		}
		return S_OK;
	}
	STDMETHOD(OnPropertyValueChanged)(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) //-V801
	{
		PostMessage(ghWnd, APPMSG_SND_CHANGE, 0, 0);
		return S_OK;
	}
	//Now the evil stuff:
	//IUnknown implementation is not required. These methods are not called by MMDevice API
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject)
	{
		if (ppvObject) ppvObject = NULL;
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 0; }
	STDMETHOD_(ULONG, Release)() { return 0; }
};
static NotificationClient gCallbackClient;


bool SndInit()
{
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
		CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**) &gpSndEnumerator);
	if (FAILED(hr)) return false;
	gpSndEnumerator->RegisterEndpointNotificationCallback(&gCallbackClient);

	
	return true;
}


void SndUninit()
{
	if (gpSndEnumerator)
	{
		gpSndEnumerator->UnregisterEndpointNotificationCallback(&gCallbackClient);
		gpSndEnumerator->Release();
		gpSndEnumerator = nullptr;
	}
}


void PlaybackDevice::Set(IMMDevice *pDev)
{
	FreeIcon();
	//Get Id
	LPWSTR pStrId;
	pDev->GetId(&pStrId);
	id.Assign(pStrId);
	CoTaskMemFree(pStrId);

	IPropertyStore *pstore;
	pDev->OpenPropertyStore(STGM_READ, &pstore);
	if (pstore)
	{
		PROPVARIANT pv;
		PropVariantInit(&pv);
		//Get name
		pstore->GetValue(PKEY_Device_DeviceDesc, &pv);
		name.Assign(pv.pwszVal);
		PropVariantClear(&pv);

		//Get icon
		if (SUCCEEDED(pstore->GetValue(PKEY_DeviceClass_IconPath, &pv)))
		{
			int index = PathParseIconLocation(pv.pwszVal);
			ExtractIconEx(pv.pwszVal, index, NULL, &icon, 1);
		}
		PropVariantClear(&pv);
		pstore->Release();
	}
}


void PlaybackDevice::FreeIcon()
{
	if (icon)
	{
		DestroyIcon(icon);
		icon = NULL;
	}
}


bool SndGetDefault(PlaybackDevice &pb)
{

	IMMDevice *defDevice;
	if (FAILED(gpSndEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defDevice)))
	{
		return false;
	}
	pb.Set(defDevice);
	defDevice->Release();
	return true;
}


WideString SndGetDefaultId()
{
	IMMDevice *defDevice;
	if (FAILED(gpSndEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defDevice)))
	{
		return WideString();
	}
	LPWSTR str;
	defDevice->GetId(&str);
	WideString temp(str);
	CoTaskMemFree(str);
	defDevice->Release();
	return temp;
}


PlaybackDeviceList::PlaybackDeviceList()
{
	WideString pbdef = SndGetDefaultId();

	IMMDeviceCollection *pCollection;

	HRESULT hr = gpSndEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
	if (FAILED(hr)) return;
	idxdef = 0;
	pCollection->GetCount(&pbcount);
	pbarray = new PlaybackDevice[pbcount];
	for (UINT i = 0; i < pbcount; i++)
	{
		IMMDevice *pDev;
		pCollection->Item(i, &pDev);
		pbarray[i].Set(pDev);
		pDev->Release();
	}
	for (UINT i = 0; i < pbcount; i++)
	{
		if (pbarray[i].id == pbdef)
		{
			idxdef = i;
			break;
		}
	}
	pCollection->Release();
}


PlaybackDeviceList::~PlaybackDeviceList()
{
	delete [] pbarray;
}


//Tested on Windows 7 SP1 [AudioSes.dll]

MIDL_INTERFACE("f8679f50-850a-41cf-9c72-430f290290c8") IPolicyConfig : public IUnknown
{
	virtual void Dummy00();
	virtual void Dummy01();
	virtual void Dummy02();
	virtual void Dummy03();
	virtual void Dummy04();
	virtual void Dummy05();
	virtual void Dummy06();
	virtual void Dummy07();
	virtual void Dummy08();
	virtual void Dummy09();

	STDMETHOD(SetDefaultEndpoint)(LPCWSTR deviceid, ERole role);

	virtual void Dummy11();
};
class DECLSPEC_UUID("870af99c-171d-4f9e-af0d-e63df40c2bc9") CPolicyConfigClient;


void SndSetDefault(LPCWSTR id)
{
	HRESULT hr;
	IPolicyConfig *pConfig;
	hr = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL,
		CLSCTX_INPROC_SERVER, __uuidof(IPolicyConfig), (void**) &pConfig);
	if (FAILED(hr)) return;
	pConfig->SetDefaultEndpoint(id, eConsole);
	pConfig->SetDefaultEndpoint(id, eMultimedia);
	pConfig->Release();
}
