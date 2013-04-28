#include "Pch.h"
#include "Common.h"
#include "resource.h"

int g_WinWidth = 1152;
int g_WinHeight	= 640;

HWND gMainWnd;

IDirect3D9* gD3d;
IDirect3DDevice9* gDevice;

bool gHasFocus;
int gKey;
bool gKeyDown[KEY_MAX];

void RenderInit();
void RenderShutdown();

void GetPresentParams(D3DPRESENT_PARAMETERS* pp)
{
	RECT rc;
	GetClientRect(gMainWnd, &rc);

	g_WinWidth = Max<int>(rc.right - rc.left, 16);
	g_WinHeight = Max<int>(rc.bottom - rc.top, 16);

	pp->Windowed				= TRUE;
	pp->SwapEffect				= D3DSWAPEFFECT_DISCARD;
	pp->BackBufferWidth			= g_WinWidth;
	pp->BackBufferHeight		= g_WinHeight;
	pp->BackBufferFormat		= D3DFMT_A8R8G8B8;
	pp->hDeviceWindow			= gMainWnd;
	pp->PresentationInterval	= D3DPRESENT_INTERVAL_ONE;
}

void ResetDevice()
{
	if (gDevice) {
		D3DPRESENT_PARAMETERS pp = { };
		GetPresentParams(&pp);
		gDevice->Reset(&pp);
	}
}

void DoFrame()
{
	if (gDevice)
	{
		gDevice->BeginScene();

		void RenderPreUpdate();
		RenderPreUpdate();

		void GameUpdate();
		GameUpdate();

		void RenderGame();
		RenderGame();

		gKey = 0;

		gDevice->EndScene();
		gDevice->Present(0, 0, 0, 0);
	}
}

int which_key(int c, bool shifted) {
	switch(c) {
		case VK_UP:		return KEY_UP;
		case VK_DOWN:	return KEY_DOWN;
		case VK_LEFT:	return KEY_LEFT;
		case VK_RIGHT:	return KEY_RIGHT;

		case ' ':
		case VK_TAB:
		case VK_RETURN:
		return shifted ? KEY_ALT_FIRE : KEY_FIRE;

		case 'R':		return KEY_RESET;
		case VK_F1:		return shifted ? KEY_CHEAT : KEY_MODE;

		case '0':		return KEY_0;
		case '1':		return KEY_1;
		case '2':		return KEY_2;
		case '3':		return KEY_3;
		case '4':		return KEY_4;
		case '5':		return KEY_5;
		case '6':		return KEY_6;
		case '7':		return KEY_7;
		case '8':		return KEY_8;
		case '9':		return KEY_9;

		case 27:
			#ifdef _DEBUG
				PostQuitMessage(0);
			#else
				return KEY_RESET;
			#endif
		break;
	}

	return KEY_NONE;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_ACTIVATE:
			gHasFocus = wparam != WA_INACTIVE;

			if (!gHasFocus) {
				memset(gKeyDown, 0, sizeof(gKeyDown));
			}
		break;

		case WM_KEYDOWN: {
			int got_key = which_key(LOWORD(wparam), (GetKeyState(VK_SHIFT) & 0x8000) != 0);

			if (got_key) {
				if (!gKeyDown[got_key])
					gKey = got_key;

				gKeyDown[got_key] = true;
			}
		}
		break;

		case WM_KEYUP: {
			int got_key = which_key(LOWORD(wparam), (GetKeyState(VK_SHIFT) & 0x8000) != 0);

			if (got_key) {
				gKeyDown[got_key] = false;
			}
		}
		break;

		case WM_SYSKEYDOWN:
			if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(VK_F4) & 0x8000))
			{
				PostQuitMessage(0);
			}
		return 0;

		case WM_SIZE:
			RenderShutdown();
			gpu::Shutdown();

			ResetDevice();

			gpu::Init();
			RenderInit();

			DoFrame();
		return 0;

		case WM_CLOSE:
			PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// Window

	WNDCLASSEX wc = { sizeof(wc) };

	wc.lpszClassName = L"MainWnd";
	wc.lpfnWndProc = MainWndProc;
	wc.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_LD24));
	wc.hCursor = LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW));

	RegisterClassEx(&wc);

	DWORD	style	= WS_OVERLAPPEDWINDOW;
	DWORD	styleEx = WS_EX_OVERLAPPEDWINDOW;
	RECT	rcWin	= { 0, 0, g_WinWidth, g_WinHeight };

	RECT rcDesk;
	GetClientRect(GetDesktopWindow(), &rcDesk);

	AdjustWindowRectEx(&rcWin, style, FALSE, styleEx);
	OffsetRect(&rcWin, ((rcDesk.right - rcDesk.left) - g_WinWidth) / 2, ((rcDesk.bottom - rcDesk.top) - g_WinHeight) / 2);

	gMainWnd = CreateWindowEx(styleEx, wc.lpszClassName, L"LD26 - Gravity Worm", style, rcWin.left, rcWin.top, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, 0, 0, 0, 0);

	// Graphics

	if ((gD3d = Direct3DCreate9(D3D_SDK_VERSION)) == 0)
	{
		Panic("D3DCreate failed - do you have D3D9 installed?");
	}

	D3DPRESENT_PARAMETERS pp = { };

	GetPresentParams(&pp);

	if (FAILED(gD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, gMainWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &gDevice)))
	{
		Panic("D3D CreateDevice failed - do you have D3D9 installed?");
	}

	gpu::Init();

	// Keys

	int locKeyW = MapVirtualKey(0x11, MAPVK_VSC_TO_VK);
	int locKeyS = MapVirtualKey(0x1F, MAPVK_VSC_TO_VK);
	int locKeyA = MapVirtualKey(0x1E, MAPVK_VSC_TO_VK);
	int locKeyD = MapVirtualKey(0x20, MAPVK_VSC_TO_VK);
	int locKeyZ = MapVirtualKey(0x2C, MAPVK_VSC_TO_VK);

	// Main

	RenderInit();

	void GameInit();
	GameInit();

	ShowWindow(gMainWnd, SW_SHOWNORMAL);

	// Audio
	
	SoundInit();

	for(;;)
	{
		MSG msg;
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			DoFrame();
			Sleep(gHasFocus ? 0 : 250);
		}
	}

	ShowWindow(gMainWnd, SW_HIDE);
	ExitProcess(0);
	
	gDevice->SetVertexDeclaration(0);
	gDevice->SetVertexShader(0);
	gDevice->SetPixelShader(0);
	gDevice->SetTexture(0, 0);

	SoundShutdown();

	RenderShutdown();

	gDevice->Release();
	gD3d->Release();

	DestroyWindow(gMainWnd);

	return 0;
}