#pragma once

#include "StdInclude.h"

class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize(LPCTSTR name);
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int& width, int& height);
	void ShutdownWindows();

	TCHAR m_applicationName[MAXNAMELEN];
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputClass* m_Input;
	GraphicsClass* m_Graphics;
};

namespace SBR
{
	class ConfigWindow
	{
	public:
		ConfigWindow();
		ConfigWindow(const ConfigWindow&);
		~ConfigWindow();

		HWND Initialize();
		void Shutdown();

	private:
		bool WriteSettingsToFile();
		int m_configWidth = 800;
		int m_configHeight = 600;
	};
}