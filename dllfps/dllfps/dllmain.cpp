
#include "stdafx.h"
// This is the main DLL file.
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)


#pragma comment(lib, "user32.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <windows.h>
#include <fstream>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <tlhelp32.h>

using namespace std;


//Globals
ofstream ofile;
char dlldir[320];
HMODULE dllHandle;
FILE *file = nullptr;
bool dllLoaded;


uintptr_t** g_deviceFunctionAddresses;
typedef void(*voidFunction)();
voidFunction previousEndScene;
void MyEndScene()
{
	fprintf(file, "plm");
	previousEndScene();
}
#include <assert.h>


void loguri(string a)
{
	static ofstream myfile("E:\\TEST\\example.txt");
	myfile << a.c_str();

}
void InitDevice(IDirect3D9 * pD3D, HWND hWindow, IDirect3DDevice9 ** ppDevice, D3DPRESENT_PARAMETERS d3dPP)
{
	d3dPP.BackBufferCount = 1;
	d3dPP.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPP.hDeviceWindow = hWindow;
	d3dPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dPP.BackBufferFormat = D3DFMT_R5G6B5;
	d3dPP.Windowed = TRUE;
	loguri("bun");
	auto x = FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &d3dPP, ppDevice));
	loguri("bun2");
	auto asd = 2;
	asd++;

	//HookDevice(*ppDevice, hWindow);
}

DWORD WINAPI HookD3D(LPVOID asd)
{
	std::abort();
	D3DPRESENT_PARAMETERS d3dPP;
	IDirect3DDevice9 * pDevice = 0;
	HWND hWindow = 0;

	IDirect3D9 * pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	hWindow = FindWindow(NULL, TEXT("World of Warcraft"));

	if (!hWindow) {
		//MessageBox(0, "Failed to obtain WoW hWindow", "Error", 0);
	}

	memset(&d3dPP, 0, sizeof(D3DPRESENT_PARAMETERS));
	InitDevice(pD3D, hWindow, &pDevice, d3dPP);
	return 1;
}
bool WINAPI DllMain(HMODULE hDll, DWORD dwReason, PVOID pvReserved)
{
	loguri("bun");
	//*((unsigned int*)0) = 0xDEAD;
	//HookD3D();
	if (dwReason == DLL_PROCESS_ATTACH)
	{ 

		CreateThread(0, 0, HookD3D, 0, 0, 0);
		// No need to call DllMain for each thread start/stop
		DisableThreadLibraryCalls(hDll);

		dllHandle = hDll;

		// Prepare logging
		//GetModuleFileNameA(hDll, dlldir, 512);


		dllLoaded = true;
		return true;
	}

	else if (dwReason == DLL_PROCESS_DETACH)
	{

		if (!pvReserved) {
			if (g_deviceFunctionAddresses) { delete g_deviceFunctionAddresses; }
		}
	}

	return false;
}



