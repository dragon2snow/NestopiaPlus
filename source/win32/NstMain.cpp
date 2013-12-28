////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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

#ifdef _MSC_VER

#ifdef NDEBUG
#pragma comment(lib,"emucore")
 #else
  #pragma comment(lib,"emucoredebug")
  #define CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
 #endif

 #if _MSC_VER >= 1400
  #pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
 #endif

#endif

int WINAPI WinMain(HINSTANCE,HINSTANCE,char*,int cmdShow)
{
#if defined(_MSC_VER) && !defined(NDEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	try
	{
		return Nestopia::Application::Main( cmdShow ).Run();
	}
	catch (const Nestopia::Application::Exception& exception)
	{
		exception.Issue( Nestopia::Application::Exception::FINAL );
		return EXIT_FAILURE;
	}
	catch (const std::bad_alloc&)
	{
		Nestopia::Application::Exception( IDS_ERR_OUT_OF_MEMORY ).Issue( Nestopia::Application::Exception::FINAL );
		return EXIT_FAILURE;
	}
	catch (Nestopia::Application::Exception::ExitCode exitcode)
	{
		return exitcode;
	}
#ifdef NDEBUG
	catch (...)
	{
		Nestopia::Application::Exception( IDS_ERR_GENERIC ).Issue( Nestopia::Application::Exception::FINAL );
		return EXIT_FAILURE;
	}
#endif
}
