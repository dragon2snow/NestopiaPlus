////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstIoScreen.hpp"
#include "NstManager.hpp"
#include "NstManagerPreferences.hpp"
#include "NstManagerMachine.hpp"

namespace Nestopia
{
	namespace Managers
	{
		NST_COMPILE_ASSERT
		(
			IDM_MACHINE_SYSTEM_NTSC == IDM_MACHINE_SYSTEM_AUTO + 1 &&
			IDM_MACHINE_SYSTEM_PAL  == IDM_MACHINE_SYSTEM_AUTO + 2
		);

		Machine::Machine(Emulator& e,const Configuration& cfg,Window::Menu& m,const Preferences& p)
		:
		Manager     ( e, m, this, &Machine::OnEmuEvent ),
		preferences ( p )
		{
			static const Window::Menu::CmdHandler::Entry<Machine> commands[] =
			{
				{ IDM_MACHINE_POWER,       &Machine::OnCmdPower   },
				{ IDM_MACHINE_RESET_SOFT,  &Machine::OnCmdReset   },
				{ IDM_MACHINE_RESET_HARD,  &Machine::OnCmdReset   },
				{ IDM_MACHINE_PAUSE,       &Machine::OnCmdPause   },
				{ IDM_MACHINE_SYSTEM_AUTO, &Machine::OnCmdSystem  },
				{ IDM_MACHINE_SYSTEM_NTSC, &Machine::OnCmdSystem  },
				{ IDM_MACHINE_SYSTEM_PAL,  &Machine::OnCmdSystem  }
			};

			menu.Commands().Add( this, commands );

			static const Window::Menu::PopupHandler::Entry<Machine> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_SYSTEM>::ID, &Machine::OnMenuSystem }
			};

			menu.PopupRouter().Add( this, popups );

			const GenericString type( cfg["machine region"] );
			const uint id = (type == _T("ntsc") ? IDM_MACHINE_SYSTEM_NTSC : type == _T("pal") ? IDM_MACHINE_SYSTEM_PAL : IDM_MACHINE_SYSTEM_AUTO);

			menu[id].Check( IDM_MACHINE_SYSTEM_AUTO, IDM_MACHINE_SYSTEM_PAL );

			if (id == IDM_MACHINE_SYSTEM_AUTO)
				emulator.AutoSetMode();
			else
				emulator.SetMode( id == IDM_MACHINE_SYSTEM_NTSC ? Nes::Machine::NTSC : Nes::Machine::PAL );
		}

		void Machine::Save(Configuration& cfg) const
		{
			cfg[ "machine region" ] = menu[IDM_MACHINE_SYSTEM_AUTO].Checked() ? _T( "auto" ) :
                                      emulator.Is(Nes::Machine::NTSC) ?         _T( "ntsc" ) :
																				_T( "pal"  );
		}

		void Machine::OnCmdPower(uint)
		{
			if (emulator.Is( Nes::Machine::ON ))
			{
				if
				(
					!preferences[Preferences::CONFIRM_RESET] ||
					Window::User::Confirm( IDS_ARE_YOU_SURE, IDS_MACHINE_POWER_OFF_TITLE )
				)
				{
					if (emulator.Power( false ))
						Io::Screen() << Resource::String(IDS_SCREEN_POWER_OFF);
				}
			}
			else if (emulator.Power( true ))
			{
				Io::Screen() << Resource::String(IDS_SCREEN_POWER_ON);
				emulator.Resume();
			}
		}

		void Machine::OnCmdReset(uint hard)
		{
			if
			(
				!preferences[Preferences::CONFIRM_RESET] ||
				Window::User::Confirm( IDS_ARE_YOU_SURE, IDS_MACHINE_RESET_TITLE )
			)
			{
				if (emulator.Reset( hard == IDM_MACHINE_RESET_HARD ))
					Io::Screen() << Resource::String(hard == IDM_MACHINE_RESET_HARD ? IDS_SCREEN_RESET_HARD : IDS_SCREEN_RESET_SOFT);
			}

			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Machine::OnCmdPause(uint)
		{
			const bool state = !emulator.Paused();

			if (emulator.Pause( state ))
				Io::Screen() << Resource::String(state ? IDS_SCREEN_PAUSE : IDS_SCREEN_RESUME);

			if (!state)
				Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Machine::OnCmdSystem(uint id)
		{
			menu[id].Check( IDM_MACHINE_SYSTEM_AUTO, IDM_MACHINE_SYSTEM_PAL );

			if (id == IDM_MACHINE_SYSTEM_AUTO)
				id = emulator.AutoSetMode();
			else
				id = emulator.SetMode( id == IDM_MACHINE_SYSTEM_NTSC ? Nes::Machine::NTSC : Nes::Machine::PAL );

			if (id)
			{
				id = emulator.Is(Nes::Machine::NTSC) ? IDS_SCREEN_NTSC : IDS_SCREEN_PAL;
				Io::Screen() << Resource::String(id);
			}

			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Machine::OnMenuSystem(Window::Menu::PopupHandler::Param& param)
		{
			if (param.menu[IDM_MACHINE_SYSTEM_AUTO].Unchecked())
				param.menu[emulator.Is(Nes::Machine::NTSC) ? IDM_MACHINE_SYSTEM_NTSC : IDM_MACHINE_SYSTEM_PAL].Check( IDM_MACHINE_SYSTEM_AUTO, IDM_MACHINE_SYSTEM_PAL );
		}

		void Machine::OnEmuEvent(Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_PAUSE:
				case Emulator::EVENT_RESUME:

					menu[ IDM_MACHINE_PAUSE ].Check( event == Emulator::EVENT_PAUSE );
					break;

				case Emulator::EVENT_POWER_ON:
				case Emulator::EVENT_POWER_OFF:
				{
					const bool state = (event == Emulator::EVENT_POWER_ON);

					menu[ IDM_MACHINE_POWER ].Text() << Resource::String( state ? IDS_MENU_POWER_OFF : IDS_MENU_POWER_ON);
					menu[ IDM_MACHINE_RESET_SOFT ].Enable( state );
					menu[ IDM_MACHINE_RESET_HARD ].Enable( state );
					menu[ IDM_MACHINE_PAUSE ].Enable( state );
					break;
				}

				case Emulator::EVENT_NETPLAY_POWER_ON:
				case Emulator::EVENT_NETPLAY_POWER_OFF:
				{
					const bool state = (event == Emulator::EVENT_NETPLAY_POWER_ON && emulator.GetPlayer() == 1);

					menu[ IDM_MACHINE_RESET_SOFT ].Enable( state );
					menu[ IDM_MACHINE_RESET_HARD ].Enable( state );
					break;
				}

				case Emulator::EVENT_LOAD:
				case Emulator::EVENT_UNLOAD:

					menu[ IDM_MACHINE_POWER ].Enable( event == Emulator::EVENT_LOAD );
					break;

				case Emulator::EVENT_NETPLAY_LOAD:

					if (emulator.GetPlayer() == 1 && menu[IDM_MACHINE_SYSTEM_AUTO].Checked())
						emulator.AutoSetMode();

				case Emulator::EVENT_NETPLAY_UNLOAD:
				{
					const bool state = (event == Emulator::EVENT_NETPLAY_UNLOAD);

					menu[ IDM_MACHINE_SYSTEM_AUTO ].Enable( state );
					menu[ IDM_MACHINE_SYSTEM_NTSC ].Enable( state );
					menu[ IDM_MACHINE_SYSTEM_PAL  ].Enable( state );
					break;
				}

				case Emulator::EVENT_INIT:

					menu[ IDM_MACHINE_POWER ].Text() << Resource::String(IDS_MENU_POWER_ON);
					menu[ IDM_MACHINE_POWER ].Disable();
					menu[ IDM_MACHINE_RESET_SOFT ].Disable();
					menu[ IDM_MACHINE_RESET_HARD ].Disable();
					menu[ IDM_MACHINE_PAUSE ].Disable();
					break;
			}
		}
	}
}
