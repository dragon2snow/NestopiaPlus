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

#include "NstManagerEmulator.hpp"
#include "NstManagerTapeRecorder.hpp"
#include "NstDialogTapeRecorder.hpp"
#include "../core/api/NstApiTapeRecorder.hpp"

namespace Nestopia
{
	namespace Managers
	{
		TapeRecorder::TapeRecorder(Emulator& e,const Configuration& cfg,Window::Menu& m,const Paths& paths)
		:
		dialog   (new Window::TapeRecorder(cfg,paths)),
		emulator (e),
		menu     (m)
		{
			static const Window::Menu::CmdHandler::Entry<TapeRecorder> commands[] =
			{
				{ IDM_MACHINE_EXT_TAPE_FILE,   &TapeRecorder::OnCmdFile   },
				{ IDM_MACHINE_EXT_TAPE_RECORD, &TapeRecorder::OnCmdRecord },
				{ IDM_MACHINE_EXT_TAPE_PLAY,   &TapeRecorder::OnCmdPlay   },
				{ IDM_MACHINE_EXT_TAPE_STOP,   &TapeRecorder::OnCmdStop   }
			};

			static const Window::Menu::PopupHandler::Entry<TapeRecorder> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_EXT,IDM_POS_MACHINE_EXT_TAPE>::ID, &TapeRecorder::OnMenuTape }
			};

			m.Commands().Add( this, commands );
			m.PopupRouter().Add( this, popups );
			emulator.Events().Add( this, &TapeRecorder::OnEmuEvent );
		}

		TapeRecorder::~TapeRecorder()
		{
			emulator.Events().Remove( this );
		}

		void TapeRecorder::Save(Configuration& cfg) const
		{
			dialog->Save( cfg );
		}

		const Path TapeRecorder::GetFile(Path imagePath) const
		{
			if (dialog->UseImageNaming())
			{
				imagePath.Extension() = _T("tp");
				return imagePath;
			}
			else
			{
				return dialog->GetCustomFile();
			}
		}

		void TapeRecorder::OnCmdFile(uint)
		{
			dialog->Open();
		}

		void TapeRecorder::OnCmdRecord(uint)
		{
			Nes::TapeRecorder(emulator).Record();
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void TapeRecorder::OnCmdPlay(uint)
		{
			Nes::TapeRecorder(emulator).Play();
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void TapeRecorder::OnCmdStop(uint)
		{
			Nes::TapeRecorder(emulator).Stop();
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void TapeRecorder::OnMenuTape(Window::Menu::PopupHandler::Param& param)
		{
			const Nes::TapeRecorder tapeRecorder( emulator );
			const ibool stopped = tapeRecorder.IsStopped();

			param.menu[ IDM_MACHINE_EXT_TAPE_PLAY   ].Enable( stopped && tapeRecorder.CanPlay() );
			param.menu[ IDM_MACHINE_EXT_TAPE_RECORD ].Enable( stopped && tapeRecorder.IsConnected() );
			param.menu[ IDM_MACHINE_EXT_TAPE_STOP   ].Enable( !stopped );
		}

		void TapeRecorder::OnEmuEvent(Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_LOAD:
				case Emulator::EVENT_UNLOAD:

					menu[ IDM_MACHINE_EXT_TAPE_FILE ].Enable( event == Emulator::EVENT_UNLOAD );
					break;

				case Emulator::EVENT_NETPLAY_MODE_ON:
				case Emulator::EVENT_NETPLAY_MODE_OFF:

					menu[IDM_POS_MACHINE][IDM_POS_MACHINE_EXT][IDM_POS_MACHINE_EXT_TAPE].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
					break;
			}
		}
	}
}
