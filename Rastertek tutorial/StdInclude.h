#pragma once

#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#ifndef _UNICODE
#define _UNICODE
#endif // !_UNICODE

#include <tchar.h>
#include <windows.h>
#include <VersionHelpers.h>

#define SafeDelete(x) { delete x; x = 0; }
#define ReleaseCOM(x) {if(x){ x->Release(); x=0;}}

// Error checking TODO cambiare return false con qualcosa di più significativo
#if defined(DEBUG) || defined(_DEBUG)
#ifndef HR
#define HR(x) {HRESULT hr=(x); if(FAILED(hr)){return false;}}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif

constexpr int MAXNAMELEN = 256;

class SystemClass;
class InputClass;
class GraphicsClass;

// TODO implementare una classe di log/errore per avere un valore di ritorno dalle funzioni più significativo
// TODO implementare una classe timer per cronometro e data
// TODO implementare una classe per incapsulare file I/O

// Da Rubicon creazione e gestione della finestra di configurazione
// Da TheXGame classe Log
// Da Direct3DInit enumerazione dei devices per DirectX11