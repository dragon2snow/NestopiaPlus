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

#include "NstObjectHeap.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstResourceFile.hpp"
#include "NstIoStream.hpp"
#include "NstIoFile.hpp"
#include "NstIoZip.hpp"
#include "NstDialogLauncher.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerLauncher.hpp"
#include "../core/api/NstApiCartridge.hpp"

namespace Nestopia
{
	using namespace Managers;

	Launcher::Launcher(Emulator& e,const Configuration& cfg,Window::Menu& m,const Paths& paths,Window::Custom& window)
	: 
	menu     ( m ),
	emulator ( e ),
	dialog   ( new Window::Launcher(Nes::Cartridge(ImportDatabase(e)).GetDatabase(),paths,cfg) )
	{
		state[FITS] = TRUE;
		state[AVAILABLE] = TRUE;
		window.Messages().Hooks().Add( WM_DISPLAYCHANGE, this, &Launcher::OnDisplayChange );
		m.Commands().Add( IDM_FILE_LAUNCHER, this, &Launcher::OnMenu );
		emulator.Events().Add( this, &Launcher::OnEmuEvent );
	}

	Launcher::~Launcher()
	{
		emulator.Events().Remove( this );
	}

	Nes::Emulator& Launcher::ImportDatabase(Nes::Emulator& emulator)
	{
		Collection::Buffer buffer;

		if (Resource::File( IDR_IMAGEDATABASE1, "ImageDatabase" ).Uncompress( buffer ))
		{
			Io::Stream::Input stream( buffer );
			Nes::Cartridge( emulator ).GetDatabase().Load( stream );
		}

		return emulator;
	}

	void Launcher::Save(Configuration& cfg,ibool saveFiles) const
	{
		dialog->Save( cfg, saveFiles );
	}

	void Launcher::Update()
	{
		if (!state[FITS] || !state[AVAILABLE])
			dialog->Close();

		menu[IDM_FILE_LAUNCHER].Enable( state[FITS] && state[AVAILABLE] );
	}

	void Launcher::OnMenu(uint)
	{
		dialog->Open();
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
}
