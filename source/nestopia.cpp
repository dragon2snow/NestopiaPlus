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
#pragma comment(lib,"CoreDebug")
#else
#pragma comment(lib,"PdxLibrary")
#pragma comment(lib,"core")
#endif

#include <new>
#include "windows/NstApplication.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PDX_CDECL NewHandler()
{
	throw ("Out of memory!");
}

////////////////////////////////////////////////////////////////////////////////////////
// big bang
////////////////////////////////////////////////////////////////////////////////////////

INT WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR CmdLine,INT iCmdShow)
{
	INT ExitCode = EXIT_FAILURE;

	try
	{
        #ifdef _DEBUG
		_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );
        #endif

		std::set_new_handler( NewHandler );

		_control87( _PC_24, MCW_PC );
		{
			APPLICATION app( hInstance, CmdLine, iCmdShow );
			ExitCode = app.Run();
		}		
		_control87( _CW_DEFAULT, 0xFFFFFUL );
	}
	catch (const CHAR* msg)
	{
		MessageBox
		( 
	     	NULL, 
			msg, 
			"Nestopia Error!",
			MB_OK|MB_ICONERROR 
		);
	}
	catch (...)
	{
		MessageBox
		(
	       	NULL,
			"Unhandled error! Call the Ghostbusters!",
			"Nestopia Error!", 
			MB_OK|MB_ICONERROR 
		);
	}

	return ExitCode;
}

