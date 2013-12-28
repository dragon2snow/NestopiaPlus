////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDialog.hpp"
#include "NstWindowMenu.hpp"
#include "NstApplicationMain.hpp"

namespace Nestopia
{
	using Application::Main;

	Main::Main(const int cmdShow)
	: 
	menu          ( IDR_MENU ),
	preferences   ( emulator, instance.GetConfiguration(), menu ),
	logfile       ( emulator, menu, preferences ),
	paths         ( emulator, instance.GetConfiguration(), menu ),
	recentFiles   ( emulator, instance.GetConfiguration(), menu ),
	recentDirs    ( emulator, instance.GetConfiguration(), menu ),
	window        ( emulator, instance.GetConfiguration(), menu, paths, preferences, cmdShow ),
	machine       ( emulator, instance.GetConfiguration(), menu, preferences ),
	netplay		  ( emulator, instance.GetConfiguration(), menu, paths, window.Get() ),
	launcher      ( emulator, instance.GetConfiguration(), menu, paths, window.Get() ),
	fds			  ( emulator, instance.GetConfiguration(), menu, paths ),
	tapeRecorder  ( emulator, instance.GetConfiguration(), menu, paths ),
	dipSwitches   ( emulator, menu ),
	barcodeReader ( emulator, menu ),
	nsf			  ( emulator, instance.GetConfiguration(), menu ),
	movie		  ( emulator, menu, paths ),
	cheats		  ( emulator, instance.GetConfiguration(), menu, paths ),
	saveStates	  ( emulator, instance.GetConfiguration(), menu, paths, window ),
	imageInfo     ( emulator, menu ),
	help          ( emulator, menu ),
	inesHeader    ( emulator, menu, paths ),
	files         ( emulator, menu, paths, preferences, movie, tapeRecorder, cheats, saveStates, window )
	{
		static const Window::MsgHandler::Entry<Main> messages[] =
		{
			{ WM_CLOSE,           &Main::OnWinClose           },
			{ WM_QUERYENDSESSION, &Main::OnWinQueryEndSession }
		};

		window.Get().Messages().Add( this, messages );
		menu.Commands().Add( IDM_FILE_QUIT, this, &Main::OnCmdFileExit );

		instance.GetConfiguration().Reset();

		if (instance.GetConfiguration().GetStartupFile().Length())
			Instance::Launch( instance.GetConfiguration().GetStartupFile().Ptr() );
	}

	Main::~Main()
	{
		emulator.Unload();
	}

	void Main::Save()
	{
		Configuration& cfg = instance.GetConfiguration();

		preferences.Save  ( cfg );
		recentFiles.Save  ( cfg );
		recentDirs.Save   ( cfg );
		paths.Save        ( cfg );
		launcher.Save     ( cfg, preferences[Managers::Preferences::SAVE_LAUNCHER] );
		netplay.Save      ( cfg, preferences[Managers::Preferences::SAVE_NETPLAY_GAMELIST] );
		fds.Save          ( cfg );
		tapeRecorder.Save ( cfg );
		nsf.Save          ( cfg );
		window.Save       ( cfg );
		machine.Save	  ( cfg );

		if (preferences[Managers::Preferences::SAVE_CHEATS])
			cheats.Save( cfg );

		if (preferences[Managers::Preferences::SAVE_SETTINGS])
			cfg.EnableSaving();
	}

	int Main::Run()
	{
		int exitCode = window.Run();
		emulator.Unload();

		if (exitCode == EXIT_SUCCESS)
			Save();

		return exitCode;
	}

	ibool Main::FirstUnloadOnExit()
	{
		return 
		(
	     	preferences[Managers::Preferences::FIRST_UNLOAD_ON_EXIT] && 
			emulator.Is(Nes::Machine::IMAGE)
		);
	}

	ibool Main::IsOkToExit() const
	{
		return 
		(
	       	!preferences[Managers::Preferences::CONFIRM_EXIT] || 
			Window::User::Confirm( IDS_ARE_YOU_SURE, IDS_TITLE_EXIT )
		);
	}

	void Main::Exit()
	{
		if (!netplay.Close())
		{
			if (FirstUnloadOnExit())
			{
				window.Get().SendCommand( IDM_FILE_CLOSE );
			}
			else if (IsOkToExit()) 
			{
				emulator.Unload();
				::PostQuitMessage( EXIT_SUCCESS );
			}
		}
	}

	ibool Main::OnWinClose(Window::Param&)
	{
		Exit();
		return TRUE;
	}

	ibool Main::OnWinQueryEndSession(Window::Param& param)
	{
		netplay.Close();

		if (emulator.Is(Nes::Machine::IMAGE))
			window.Get().SendCommand( IDM_FILE_CLOSE );

		param.lParam = !emulator.Is(Nes::Machine::IMAGE);
		return TRUE;
	}

	void Main::OnCmdFileExit(uint) 
	{ 
		Exit();
	}
}
