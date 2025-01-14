// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <d3d9.h>

void* vtable[119] = {};
bool vtable_init = false;

HWND hwnd = NULL;

BOOL CALLBACK enumWindows(const HWND handle, LPARAM lp)
{
	DWORD procID;
	GetWindowThreadProcessId(handle, &procID);
	if (GetCurrentProcessId() != procID)
		return TRUE;

	hwnd = handle;
	return FALSE;
}

extern "C" __declspec(dllexport) HWND GetProcessWindow()
{
	hwnd = NULL;

	EnumWindows(enumWindows, NULL);

	RECT size;
	if (hwnd == NULL)
		return NULL;

	return hwnd;
}

extern "C" __declspec(dllexport) HWND GetDeviceWindow(IDirect3DDevice9* pDevice)
{
	if (pDevice != nullptr)
	{
		D3DDEVICE_CREATION_PARAMETERS params = {};
		pDevice->GetCreationParameters(&params);
		return params.hFocusWindow;
	}

	return NULL;
}

bool InitD3D9DeviceVtable()
{
	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D)
		return false;

	IDirect3DDevice9* pDummyDevice = nullptr;

	D3DPRESENT_PARAMETERS d3dpp = {};
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetProcessWindow();
	d3dpp.Windowed = (GetWindowLongPtr(d3dpp.hDeviceWindow, GWL_STYLE) & WS_POPUP) != 0 ? FALSE : TRUE;
	HRESULT dummyDevCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);
	if (dummyDevCreated != S_OK)
	{
		d3dpp.Windowed = !d3dpp.Windowed;
		dummyDevCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

		if (dummyDevCreated != S_OK)
		{
			pD3D->Release();
			return false;
		}
	}

	memcpy(vtable, *(void***)(pDummyDevice), sizeof(vtable));
	pDummyDevice->Release();
	pD3D->Release();

	return true;
}

extern "C" __declspec(dllexport) void* GetD3D9Reset()
{
	if (!vtable_init)
		vtable_init = InitD3D9DeviceVtable();
	return vtable[16];
}

extern "C" __declspec(dllexport) void* GetD3D9Present()
{
	if (!vtable_init)
		vtable_init = InitD3D9DeviceVtable();
	return vtable[17];
}

extern "C" __declspec(dllexport) void* GetD3D9EndScene()
{
	if (!vtable_init)
		vtable_init = InitD3D9DeviceVtable();
	return vtable[42];
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

