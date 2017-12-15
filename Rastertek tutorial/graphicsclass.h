#pragma once
#include "StdInclude.h"
#include "d3dclass.h"
#include "CameraClass.h"
#include "ModelClass.h"
#include "ShaderClass.h"

class ModelClass;

// TODO creare una variabile bool FullScreenEnabled e una funzione callback OnFullScreen() che viene chiamata con un input mappabile(tasto F ?)
// TODO tutte queste variabili devono sparire dentro la classe appropriata

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

private: 
	bool Render();

	D3DClass* m_D3D;
	CameraClass* m_Camera;
	ModelClass* m_Model;
	ShaderClass* m_Shader;
};
