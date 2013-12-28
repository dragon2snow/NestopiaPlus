////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#include "NstApplicationException.hpp"
#include "NstApplicationMain.hpp"

#ifdef NDEBUG
 #pragma comment(lib,"emucore")
#else
 #define CRTDBG_MAP_ALLOC
 #include <crtdbg.h>
 #pragma comment(lib,"emucoredebug")
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

int WINAPI WinMain(HINSTANCE,HINSTANCE,char* cmdLine,int cmdShow)
{
#ifndef NDEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	try
	{
		return Nestopia::Application::Main( cmdLine, cmdShow ).Run();
	}
	catch (const Nestopia::Application::Exception& exception)
	{
		exception.Issue( Nestopia::Application::Exception::FINAL );
		return EXIT_FAILURE;
	}
	catch (const std::bad_alloc&)
	{
		Nestopia::Application::Exception( IDS_ERR_OUT_OF_MEMORY ).Issue();
		return EXIT_FAILURE;
	}
	catch (Nestopia::Application::Exception::ExitCode exitcode)
	{
		return exitcode;
	}
#ifdef NDEBUG
	catch (...)
	{
		Nestopia::Application::Exception( IDS_ERR_GENERIC ).Issue();
		return EXIT_FAILURE;
	}
#endif
}
