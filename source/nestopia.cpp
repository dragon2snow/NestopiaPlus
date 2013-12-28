////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
//
// This file is part of Nestopia.
// 
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#pragma comment(lib,"PdxLibraryDebug")
#else
#pragma comment(lib,"PdxLibrary")
#endif

#include "windows/NstApplication.h"

#include <string>

APPLICATION application;

#include "paradox/PdxFile.h"
#include "NstNes.h"
#include "core/NstNsp.h"

////////////////////////////////////////////////////////////////////////////////////////
// big bang
////////////////////////////////////////////////////////////////////////////////////////

INT WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR RomImageName,INT iCmdShow)
{
 #ifdef _DEBUG

	_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );

 #endif

	if (PDX_FAILED(application.Init( hInstance, RomImageName, iCmdShow )))
		return EXIT_FAILURE;

	return application.Loop();
}

