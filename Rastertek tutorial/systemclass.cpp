#include "systemclass.h"
#include "inputclass.h"
#include "graphicsclass.h"

namespace
{
	SystemClass* ApplicationHandle = 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, msg, wParam, lParam);
		}
	}
}

#pragma region SystemClass

SystemClass::SystemClass()
{
	m_Input = NULL;
	m_Graphics = NULL;
}

SystemClass::SystemClass(const SystemClass &)
{
}

SystemClass::~SystemClass()
{
}

bool SystemClass::Initialize(LPCTSTR name)
{
	int clientWidth = 0;
	int clientHeight = 0;
	bool result;
	if (_tcscpy_s(m_applicationName, MAXNAMELEN, name) != 0)
	{
		return false;
	}
	InitializeWindows(clientWidth, clientHeight);
	m_Input = new InputClass;
	if (!m_Input)
	{
		return false;
	}
	m_Input->Initialize();
	m_Graphics = new GraphicsClass;
	if (!m_Graphics)
	{
		return false;
	}
	result = m_Graphics->Initialize(clientWidth, clientHeight, m_hwnd);
	if (!result)
	{
		return false;
	}
	return true;
}

void SystemClass::Shutdown()
{
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		SafeDelete(m_Graphics);
	}
	if (m_Input)
	{
		SafeDelete(m_Input);
	}
	ShutdownWindows();
	return;
}

void SystemClass::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	bool done, result;
	done = false;
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			result = Frame();
			if (!result)
			{
				done = true;
			}
		}
	}
	return;
}

LRESULT SystemClass::MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYDOWN:
		{
			m_Input->KeyDown((unsigned int)wParam);
			return 0;
		}
		case WM_KEYUP:
		{
			m_Input->KeyUp((unsigned int)wParam);
			return 0;
		}
		default:
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
}

bool SystemClass::Frame()
{
	bool result;
	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}
	result = m_Graphics->Frame();
	if (!result)
	{
		return false;
	}
	return true;
}

void SystemClass::InitializeWindows(int& clientWidth, int& clientHeight)
{
	WNDCLASSEX wc;

	ApplicationHandle = this;
	m_hinstance = GetModuleHandle(NULL);

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
	int desktopHeight = GetSystemMetrics(SM_CYSCREEN);
	// float AspectRatio = static_cast<float> (desktopWidth) / desktopHeight;
	DEVMODE dmScreenSettings;
	int posX, posY;

	if (FULL_SCREEN)
	{
		clientWidth = desktopWidth;
		clientHeight = desktopHeight;

		ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));
		dmScreenSettings.dmSize = sizeof(DEVMODE);
		dmScreenSettings.dmPelsWidth = (unsigned int)desktopWidth;
		dmScreenSettings.dmPelsHeight = (unsigned int)desktopHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT; 
		// TODO verificare che questi settaggi siano compatibili
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
		posX = posY = 0;
	}
	else // Windowed
	{
		clientWidth = 800;
		clientHeight = 600; 

		posX = (desktopWidth - clientWidth) / 2;
		posY = (desktopHeight - clientHeight) / 2;
	}

	m_hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		m_applicationName,
		m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX,
		posY,
		clientWidth,
		clientHeight,
		NULL,
		NULL,
		m_hinstance,
		NULL
	);

	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);
	ShowCursor(false);
	// UpdateWindow(m_hwnd);
}

void SystemClass::ShutdownWindows()
{
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;
	ApplicationHandle = NULL;
	return;
}

#pragma endregion SystemClass

namespace SBR
{
	class Control
	{

	};
}

#pragma region ConfigWindow

SBR::ConfigWindow::ConfigWindow()
{
}

SBR::ConfigWindow::ConfigWindow(const SBR::ConfigWindow &)
{
}

SBR::ConfigWindow::~ConfigWindow()
{
}

HWND SBR::ConfigWindow::Initialize()
{
	LPCTSTR title = TEXT("Rubicon Configurator");

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH));
	wc.lpszMenuName = 0;
	wc.lpszClassName = title;
	wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
	assert(RegisterClassEx(&wc));
	return 0;
}

void SBR::ConfigWindow::Shutdown()
{

}

bool SBR::ConfigWindow::WriteSettingsToFile()
{
	return false;
}

#pragma endregion ConfigWindow
