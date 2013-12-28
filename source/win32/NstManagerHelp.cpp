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

#include "NstWindowMenu.hpp"
#include "NstDialogAbout.hpp"
#include "NstDialogLicense.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerHelp.hpp"
#include <ShellAPI.h>

namespace Nestopia
{
	namespace Managers
	{
		Help::Help(Emulator& e,Window::Menu& m)
		: emulator(e), menu(m)
		{
			static const Window::Menu::CmdHandler::Entry<Help> commands[] =
			{
				{ IDM_HELP_HELP,    &Help::OnCmdHelp    },
				{ IDM_HELP_ABOUT,   &Help::OnCmdAbout   },
				{ IDM_HELP_LICENSE, &Help::OnCmdLicense }
			};

			m.Commands().Add( this, commands );
			emulator.Events().Add( this, &Help::OnEmuEvent );

			m[IDM_HELP_HELP].Enable( Application::Instance::GetExePath(_T("readme.html")).FileExists() );
		}

		Help::~Help()
		{
			emulator.Events().Remove( this );
		}

		void Help::OnCmdHelp(uint)
		{
			::ShellExecute
			(
				NULL,
				_T("open"),
				Application::Instance::GetExePath(_T("readme.html")).Ptr(),
				NULL,
				NULL,
				SW_SHOWNORMAL
			);
		}

		void Help::OnCmdAbout(uint)
		{
			Window::About().Open();
		}

		void Help::OnCmdLicense(uint)
		{
			Window::License().Open();
		}

		void Help::OnEmuEvent(Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE_ON:
				case Emulator::EVENT_NETPLAY_MODE_OFF:
				{
					const ibool state = (event == Emulator::EVENT_NETPLAY_MODE_OFF);

					menu[IDM_HELP_ABOUT].Enable( state );
					menu[IDM_HELP_LICENSE].Enable( state );
					break;
				}
			}
		}
	}
}
