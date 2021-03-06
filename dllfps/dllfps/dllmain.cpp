
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
typedef HRESULT(__thiscall *voidFunction)(void*);
intptr_t* gameDirectX;
voidFunction previousEndScene;


//Globals
ofstream ofile;
char dlldir[320];
HMODULE dllHandle;
FILE *file = nullptr;
bool dllLoaded;


uintptr_t** g_deviceFunctionAddresses;

#include <assert.h>
void loguri(string a)
{
	std::ofstream myfile;
	myfile.open("E:\\TEST\\example.txt", std::ofstream::out | std::ofstream::app);
	myfile << a.c_str() << endl;
	myfile.close();

}
HRESULT __fastcall  DXover(void* instance)
{
	loguri("overight");
	return previousEndScene(instance);
}
int vtablehook_unprotect(void* region)
{
#ifdef WIN32
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery((LPCVOID)region, &mbi, sizeof(mbi));
	VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect);
	return mbi.Protect;
#elif _linux_
	mprotect((void*)((intptr_t)region & vtablehook_pagemask), vtablehook_pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);
	return PROT_READ | PROT_EXEC;
#endif
}

void vtablehook_protect(void* region, int protection)
{
#ifdef WIN32
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery((LPCVOID)region, &mbi, sizeof(mbi));
	VirtualProtect(mbi.BaseAddress, mbi.RegionSize, protection, &mbi.Protect);
#elif _linux_
	mprotect((void*)((intptr_t)region & vtablehook_pagemask), vtablehook_pagesize, protection);
#endif
}
void* vtablehook_hook(void* instance, void* hook, int offset) {

	intptr_t vtable = *((intptr_t*)instance);
	intptr_t entry = vtable + sizeof(intptr_t) * offset;
	intptr_t original = *((intptr_t*)entry);

	int original_protection = vtablehook_unprotect((void*)entry);
	previousEndScene = (voidFunction)original;
	*((intptr_t*)entry) = (intptr_t)hook;
	vtablehook_protect((void*)entry, original_protection);

	return (void*)original;
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
DWORD GetD3D9DeviceFunctionAddress(LPVOID methodIndex)
{
	loguri("good3");
	// There are 119 functions defined in the IDirect3DDevice9 interface (including our 3 IUnknown methods QueryInterface, AddRef, and Release)
	const int interfaceMethodCount = 119;

	// If we do not yet have the addresses, we need to create our own Direct3D Device and determine the addresses of each of the methods
	// Note: to create a Direct3D device we need a valid HWND - in this case we will create a temporary window ourselves - but it could be a HWND
	//       passed through as a parameter of this function or some other initialisation export.
	if (!g_deviceFunctionAddresses) {
		loguri("if1");
		// Ensure d3d9.dll is loaded
		HMODULE hMod = LoadLibraryA("d3d9.dll");
		LPDIRECT3D9       pD3D = NULL;
		LPDIRECT3DDEVICE9 pd3dDevice = NULL;

		HWND hWnd = CreateWindowA("BUTTON", "Temporary Window", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, dllHandle, NULL);
		//__try
		{
			pD3D = Direct3DCreate9(D3D_SDK_VERSION);
			if (pD3D == NULL)
			{
				loguri("if2");
				// TO DO: Respond to failure of Direct3DCreate9
				return 0;
			}
			//__try
			{
				D3DDISPLAYMODE d3ddm;

				if (FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
				{
					loguri("if3");
					// TO DO: Respond to failure of GetAdapterDisplayMode
					return 0;
				}

				//
				// Do we support hardware vertex processing? if so, use it. 
				// If not, downgrade to software.
				//
				D3DCAPS9 d3dCaps;
				if (FAILED(pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT,
					D3DDEVTYPE_HAL, &d3dCaps)))
				{
					loguri("if4");
					// TO DO: Respond to failure of GetDeviceCaps
					return 0;
				}

				DWORD dwBehaviorFlags = 0;

				if (d3dCaps.VertexProcessingCaps != 0)
				{
					loguri("if5");
					dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
				}
				else
				{
					loguri("else1");
					dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
				}

				//
				// Everything checks out - create a simple device.
				//

				D3DPRESENT_PARAMETERS d3dpp;
				memset(&d3dpp, 0, sizeof(d3dpp));

				d3dpp.BackBufferFormat = d3ddm.Format;
				d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
				d3dpp.Windowed = TRUE;
				d3dpp.EnableAutoDepthStencil = TRUE;
				d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
				d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

				if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
					dwBehaviorFlags, &d3dpp, &pd3dDevice)))
				{
					loguri("if6");
					// Respond to failure of CreateDevice
					return 0;
				}
				vtablehook_hook(pd3dDevice, (void*)DXover, 42);
				pd3dDevice->EndScene();
				//
				// Device has been created - we can now check for the method addresses
				//

				//__try
				{
					// retrieve a pointer to the VTable
					uintptr_t* pInterfaceVTable = (uintptr_t*)*(uintptr_t*)pd3dDevice;
					g_deviceFunctionAddresses = new uintptr_t*[interfaceMethodCount];

					//add_log("d3d9.dll base address: 0x%x", hMod);

					// Retrieve the addresses of each of the methods (note first 3 IUnknown methods)
					// See d3d9.h IDirect3D9Device to see the list of methods, the order they appear there
					// is the order they appear in the VTable, 1st one is index 0 and so on.
					for (int i = 0; i<interfaceMethodCount; i++) {
						g_deviceFunctionAddresses[i] = (uintptr_t*)pInterfaceVTable[i];

						// Log the address offset
						//add_log("Method [%i] offset: 0x%x", i, pInterfaceVTable[i] - (uintptr_t)hMod);
					}
					loguri("end");
				}
				//__finally {
				//	pd3dDevice->Release();
				//}
			}
			//__finally
			//{
			//	pD3D->Release();
			//}
		}
		//__finally {
		//	DestroyWindow(hWnd);
		//}
	}

	// Return the address of the method requested
	if (g_deviceFunctionAddresses) {
		return 0;
	}
	else {
		return 0;
	}
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
	loguri("good1");
	//*((unsigned int*)0) = 0xDEAD;
	//HookD3D();
	if (dwReason == DLL_PROCESS_ATTACH)
	{ 
		loguri("good2");
		//CreateThread(0, 0, HookD3D, 0, 0, 0);
		// No need to call DllMain for each thread start/stop
		DisableThreadLibraryCalls(hDll);
		CreateThread(nullptr, NULL, GetD3D9DeviceFunctionAddress, nullptr, NULL, nullptr);
		//GetD3D9DeviceFunctionAddress(42);
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



