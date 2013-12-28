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

#include "resource/resource.h"
#include "NstObjectHeap.hpp"
#include "NstWindowParam.hpp"
#include "NstResourceFile.hpp"
#include "NstIoStream.hpp"
#include "NstManager.hpp"
#include "NstDialogLauncher.hpp"
#include "NstManagerLauncher.hpp"
#include "../core/api/NstApiCartridge.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Launcher::Launcher(Emulator& e,const Configuration& cfg,Window::Menu& m,const Paths& paths,Window::Custom& window)
		:
		Manager    ( e, m, this, &Launcher::OnEmuEvent ),
		fullscreen ( false ),
		dialog     ( new Window::Launcher(Nes::Cartridge(ImportDatabase(e)).GetDatabase(),paths,cfg) )
		{
			state[FITS] = true;
			state[AVAILABLE] = true;

			static const Window::MsgHandler::HookEntry<Launcher> hooks[] =
			{
				{ WM_ACTIVATE,      &Launcher::OnActivate      },
				{ WM_DISPLAYCHANGE, &Launcher::OnDisplayChange }
			};

			window.Messages().Hooks().Add( this, hooks );
			menu.Commands().Add( IDM_FILE_LAUNCHER, this, &Launcher::OnMenu );

			Application::Instance::Events::Add( this, &Launcher::OnAppEvent );
		}

		Launcher::~Launcher()
		{
		}

		Nes::Emulator& Launcher::ImportDatabase(Nes::Emulator& emulator)
		{
			Collection::Buffer buffer;

			if (Resource::File( IDR_IMAGEDATABASE, _T("ImageDatabase") ).Uncompress( buffer ))
			{
				Io::Stream::Input stream( buffer );
				Nes::Cartridge( emulator ).GetDatabase().Load( stream );
			}

			return emulator;
		}

		void Launcher::Save(Configuration& cfg,bool saveSize,bool saveFiles) const
		{
			dialog->Save( cfg, saveSize, saveFiles );
		}

		void Launcher::Update()
		{
			if (!state[FITS] || !state[AVAILABLE])
				dialog->Close();

			menu[IDM_FILE_LAUNCHER].Enable( state[FITS] && state[AVAILABLE] );
		}

		void Launcher::OnMenu(uint)
		{
			emulator.Wait();
			dialog->Open( fullscreen );
		}

		void Launcher::OnActivate(Window::Param& param)
		{
			if (param.Activator().Entering() && !param.Activator().Minimized())
				dialog->Synchronize( param.hWnd );
		}

		void Launcher::OnDisplayChange(Window::Param& param)
		{
			state[FITS] =
			(
				LOWORD(param.lParam) >= MIN_DIALOG_WIDTH &&
				HIWORD(param.lParam) >= MIN_DIALOG_HEIGHT
			);

			Update();
		}

		void Launcher::OnEmuEvent(Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE_ON:
				case Emulator::EVENT_NETPLAY_MODE_OFF:

					state[AVAILABLE] = (event == Emulator::EVENT_NETPLAY_MODE_OFF);
					Update();
					break;
			}
		}

		void Launcher::OnAppEvent(Application::Instance::Event event,const void*)
		{
			switch (event)
			{
				case Application::Instance::EVENT_FULLSCREEN:
				case Application::Instance::EVENT_DESKTOP:

					fullscreen = (event == Application::Instance::EVENT_FULLSCREEN);
					dialog->Close();
					break;
			}
		}
	}
}
