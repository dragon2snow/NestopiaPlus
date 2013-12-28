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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include <Windows.h>
#include "windows/NstApplication.h"
#include "windows/NstFileManager.h"
#include "windows/NstConfigFile.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PDX_CDECL NewHandler()
{
	throw ("Out of memory!");
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL AllowOnlyOneInstance()
{
	PDXSTRING filename;

	UTILITIES::GetExeDir( filename );
	filename << "Nestopia.cfg";

	CONFIGFILE cfg;

	if (cfg.Load( filename, FALSE ) && cfg["preferences allow multiple instances"] == "yes") 
		return FALSE;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL TooManyInstances(CHAR* const CmdLine)
{
	HANDLE hMutex = ::CreateMutex( NULL, TRUE, "Nestopia Instance" );

	if (::GetLastError() == ERROR_ALREADY_EXISTS) 
	{
		HWND hWnd = ::FindWindow( NST_CLASS_NAME, NULL );

		if (hWnd && CmdLine)
		{
			for (CHAR* begin=CmdLine; *begin != '\0'; )
			{
				if (*begin++ == '\"')
				{
					while (*begin == ' ')
						++begin;

					for (const CHAR* end=begin+1; *end != '\0'; ++end)
					{
						if (*end == '\"')
						{
							for (--end; *end == ' '; --end);
							
							COPYDATASTRUCT cds;

							cds.dwData = NST_WM_CMDLINE;
							cds.lpData = PDX_CAST(PVOID,begin);
							cds.cbData = end + 1 - begin;

							::SendMessage( hWnd, WM_COPYDATA, WPARAM(hWnd), LPARAM(LPVOID(&cds)) );
							break;
						}
					}
					break;
				}
			}
		}

		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
// big bang
////////////////////////////////////////////////////////////////////////////////////////

INT WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR CmdLine,INT iCmdShow)
{
	if (AllowOnlyOneInstance() && TooManyInstances( CmdLine ))
		return EXIT_SUCCESS;

	INT ExitCode = EXIT_FAILURE;

	try
	{
        #ifdef _DEBUG
		_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );
        #endif

		std::set_new_handler( NewHandler );

		{
			APPLICATION app( hInstance, CmdLine, iCmdShow );
			ExitCode = app.Run();
		}		
	}
	catch (EXCEPTION)
	{
		::MessageBox
		( 
			NULL, 
			EXCEPTION::Msg(),
			UTILITIES::IdToString(IDS_APP_ERROR).String(),
			MB_OK|MB_ICONERROR 
		);
	}
	catch (const CHAR* msg)
	{
		::MessageBox
		( 
	     	NULL, 
			msg, 
			UTILITIES::IdToString(IDS_APP_ERROR).String(),
			MB_OK|MB_ICONERROR 
		);
	}
	catch (...)
	{
		::MessageBox
		(
	       	NULL,
			UTILITIES::IdToString(IDS_APP_UNHANDLED_ERROR).String(),
			UTILITIES::IdToString(IDS_APP_ERROR).String(),
			MB_OK|MB_ICONERROR 
		);
	}

	return ExitCode;
}

