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
#include "NstWindowMenu.hpp"
#include "NstDialogDipSwitches.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerDipSwitches.hpp"

namespace Nestopia
{
	using namespace Managers;

	DipSwitches::DipSwitches(Emulator& e,Window::Menu& m)
	: emulator(e), menu(m), dialog( new Window::DipSwitches(emulator) )
	{
		m.Commands().Add( IDM_MACHINE_OPTIONS_DIPSWITCHES, this, &DipSwitches::OnCmd );
		emulator.Events().Add( this, &DipSwitches::OnEmuEvent );
	}

	DipSwitches::~DipSwitches()
	{
		emulator.Events().Remove( this );
	}

	void DipSwitches::OnCmd(uint)
	{
		dialog->Open();
	}

	void DipSwitches::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
			case Emulator::EVENT_LOAD:
			case Emulator::EVENT_UNLOAD:
		
				menu[IDM_MACHINE_OPTIONS_DIPSWITCHES].Enable( event == Emulator::EVENT_LOAD && dialog->NumDips() );
				break;
		}
	}
}
