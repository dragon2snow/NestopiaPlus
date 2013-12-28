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

#include "NstObjectHeap.hpp"
#include "NstWindowMenu.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogPreferences.hpp"
#include "NstManagerPreferences.hpp"

namespace Nestopia
{
	using namespace Managers;

	NST_COMPILE_ASSERT
	(
		Preferences::START_IN_FULLSCREEN      == Window::Preferences::START_IN_FULLSCREEN &&
		Preferences::SUPPRESS_WARNINGS        == Window::Preferences::SUPPRESS_WARNINGS &&
		Preferences::FIRST_UNLOAD_ON_EXIT     == Window::Preferences::FIRST_UNLOAD_ON_EXIT &&
		Preferences::CONFIRM_EXIT             == Window::Preferences::CONFIRM_EXIT &&
		Preferences::RUN_IN_BACKGROUND        == Window::Preferences::RUN_IN_BACKGROUND &&
		Preferences::AUTOSTART_EMULATION      == Window::Preferences::AUTOSTART_EMULATION &&
		Preferences::SAVE_LOGFILE             == Window::Preferences::SAVE_LOGFILE &&
		Preferences::AUTOCORRECT_IMAGES       == Window::Preferences::AUTOCORRECT_IMAGES &&
		Preferences::ALLOW_MULTIPLE_INSTANCES == Window::Preferences::ALLOW_MULTIPLE_INSTANCES &&
		Preferences::SAVE_SETTINGS            == Window::Preferences::SAVE_SETTINGS &&
		Preferences::SAVE_LAUNCHER            == Window::Preferences::SAVE_LAUNCHER &&
		Preferences::CONFIRM_RESET            == Window::Preferences::CONFIRM_RESET &&
		Preferences::SAVE_CHEATS              == Window::Preferences::SAVE_CHEATS &&
		Preferences::SAVE_NETPLAY_GAMELIST    == Window::Preferences::SAVE_NETPLAY_GAMELIST &&
		Preferences::SAVE_WINDOWPOS           == Window::Preferences::SAVE_WINDOWPOS &&
		Preferences::SAVE_LAUNCHERSIZE        == Window::Preferences::SAVE_LAUNCHERSIZE &&
		Preferences::NUM_SETTINGS             == Window::Preferences::NUM_SETTINGS
	);

	NST_COMPILE_ASSERT
	(
		Preferences::PRIORITY_NORMAL       == Window::Preferences::PRIORITY_NORMAL &&
		Preferences::PRIORITY_ABOVE_NORMAL == Window::Preferences::PRIORITY_ABOVE_NORMAL &&
		Preferences::PRIORITY_HIGH         == Window::Preferences::PRIORITY_HIGH
	);

	Preferences::Preferences(Emulator& e,const Configuration& cfg,Window::Menu& m)
	: dialog( new Window::Preferences(e,cfg) ), menu(m), emulator(e), inFullscreen(false)
	{
		m.Commands().Add( IDM_OPTIONS_PREFERENCES, this, &Preferences::OnCmdOptions );

		Instance::Events::Add( this, &Preferences::OnAppEvent );
		emulator.Events().Add( this, &Preferences::OnEmuEvent );

		UpdateSettings();
	}

	Preferences::~Preferences()
	{
		emulator.Events().Remove( this );
		Instance::Events::Remove( this );
	}

	void Preferences::Save(Configuration& cfg) const
	{
		dialog->Save( cfg );
	}

	void Preferences::UpdateMenuColor() const
	{
		if (inFullscreen)
		{
			if (dialog->GetSettings().menuLookFullscreen.enabled)
				menu.SetColor( dialog->GetSettings().menuLookFullscreen.color );
			else
				menu.ResetColor();
		}
		else
		{
			if (dialog->GetSettings().menuLookDesktop.enabled)
				menu.SetColor( dialog->GetSettings().menuLookDesktop.color );
			else
				menu.ResetColor();
		}
	}

	void Preferences::UpdateSettings()
	{
		settings.flags = dialog->GetSettings();
		settings.priority = (Priority) dialog->GetSettings().priority;
		UpdateMenuColor();
	}

	void Preferences::OnCmdOptions(uint)
	{
		dialog->Open();
		UpdateSettings();
	}

	void Preferences::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
			case Emulator::EVENT_NETPLAY_LOAD:

				settings.flags[RUN_IN_BACKGROUND] = true;
				settings.flags[AUTOSTART_EMULATION] = true;
				settings.flags[CONFIRM_RESET] = false;

				if (settings.priority == PRIORITY_NORMAL)
					settings.priority = PRIORITY_ABOVE_NORMAL;

				break;

			case Emulator::EVENT_NETPLAY_UNLOAD:

				UpdateSettings();
				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				menu[IDM_OPTIONS_PREFERENCES].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				break;
		}
	}

	void Preferences::OnAppEvent(Instance::Event event,const void*)
	{
		switch (event)
		{
			case Instance::EVENT_FULLSCREEN:

				inFullscreen = true;
				UpdateMenuColor();
				break;

			case Instance::EVENT_DESKTOP:

				inFullscreen = false;
				UpdateMenuColor();
				break;
		}
	}
}
