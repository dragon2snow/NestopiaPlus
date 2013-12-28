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
#include "NstManagerEmulator.hpp"
#include "NstManagerNsf.hpp"
#include "../core/api/NstApiNsf.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Nsf::Nsf(Emulator& e,const Configuration& cfg,Window::Menu& m)
		:
		emulator ( e ),
		menu     ( m )
		{
			static const Window::Menu::CmdHandler::Entry<Nsf> commands[] =
			{
				{ IDM_MACHINE_NSF_PLAY,                     &Nsf::OnCmd          },
				{ IDM_MACHINE_NSF_STOP,                     &Nsf::OnCmd          },
				{ IDM_MACHINE_NSF_NEXT,                     &Nsf::OnCmd          },
				{ IDM_MACHINE_NSF_PREV,                     &Nsf::OnCmd          },
				{ IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND, &Nsf::OnCmdPlayInBkg }
			};

			m.Commands().Add( this, commands );
			emulator.Events().Add( this, &Nsf::OnEmuEvent );

			menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].Check( cfg["nsf in background"] != Configuration::NO );
		}

		Nsf::~Nsf()
		{
			emulator.Events().Remove( this );
		}

		void Nsf::Save(Configuration& cfg) const
		{
			cfg["nsf in background"].YesNo() = menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].Checked();
		}

		void Nsf::OnEmuEvent(Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_NSF_STOP:
				case Emulator::EVENT_NSF_NEXT:
				case Emulator::EVENT_NSF_PREV:
				case Emulator::EVENT_NSF_PLAY:
				case Emulator::EVENT_POWER_ON:
				case Emulator::EVENT_POWER_OFF:
				case Emulator::EVENT_INIT:
				{
					const ibool on = emulator.Is(Nes::Machine::SOUND,Nes::Machine::ON);
					const Nes::Nsf nsf( emulator );

					menu[ IDM_MACHINE_NSF_PLAY ].Enable( on && !nsf.IsPlaying() );
					menu[ IDM_MACHINE_NSF_STOP ].Enable( on && nsf.IsPlaying() );
					menu[ IDM_MACHINE_NSF_NEXT ].Enable( on && nsf.GetCurrentSong() + 1 < (int) nsf.GetNumSongs() );
					menu[ IDM_MACHINE_NSF_PREV ].Enable( on && nsf.GetCurrentSong() > 0 );
					break;
				}

				case Emulator::EVENT_NETPLAY_MODE_ON:
				case Emulator::EVENT_NETPLAY_MODE_OFF:

					menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
					break;
			}
		}

		void Nsf::OnCmd(uint cmd)
		{
			NST_ASSERT( emulator.Is(Nes::Machine::SOUND,Nes::Machine::ON) );

			switch (cmd)
			{
				case IDM_MACHINE_NSF_PLAY: emulator.PlaySong(); break;
				case IDM_MACHINE_NSF_STOP: emulator.StopSong(); break;
				case IDM_MACHINE_NSF_NEXT: emulator.SelectNextSong(); break;
				case IDM_MACHINE_NSF_PREV: emulator.SelectPrevSong(); break;
			}

			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Nsf::OnCmdPlayInBkg(uint)
		{
			menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].ToggleCheck();
		}
	}
}
