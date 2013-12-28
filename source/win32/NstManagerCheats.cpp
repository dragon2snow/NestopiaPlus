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
#include "NstIoNsp.hpp"
#include "NstManagerEmulator.hpp"
#include "NstWindowMenu.hpp"
#include "NstDialogCheats.hpp"
#include "NstManagerCheats.hpp"

namespace Nestopia
{
	using namespace Managers;

	Cheats::Cheats(Emulator& e,const Configuration& cfg,Window::Menu& m,const Paths& paths)
	:
	emulator ( e ),
	menu     ( m ),
	dialog   ( new Window::Cheats(e,cfg,paths) )
	{
		m.Commands().Add( IDM_OPTIONS_CHEATS, this, &Cheats::OnCmdOptions );
		emulator.Events().Add( this, &Cheats::OnEmuEvent );
		UpdateCodes();
	}

	Cheats::~Cheats()
	{
		emulator.Events().Remove( this );
	}

	void Cheats::Save(Configuration& cfg) const
	{
		dialog->Save( cfg );
	}

	void Cheats::Save(Io::Nsp::Context& context) const
	{
		dialog->Save( context );
	}

	void Cheats::Load(const Io::Nsp::Context& context) const
	{
		if (!context.cheats.empty())
		{
			dialog->Load( context );
			UpdateCodes();
		}
	}

	void Cheats::UpdateCodes() const
	{
		Nes::Cheats cheats( emulator );

		cheats.ClearCodes();

		for (uint type=0; type < Window::Cheats::NUM_CODE_TYPES; ++type)
		{
			for (uint i=0, n=dialog->GetNumCodes( type ); i < n; ++i)
			{
				if (dialog->IsCodeEnabled( type, i ))
					cheats.SetCode( dialog->GetCode( type, i ) );
			}
		}
	}

	void Cheats::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
			case Emulator::EVENT_UNLOAD:

				dialog->ResetRamSearch();

				if (dialog->ClearTemporaryCodes())
					UpdateCodes();

				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				menu[IDM_OPTIONS_CHEATS].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				break;
		}
	}

	void Cheats::OnCmdOptions(uint)
	{
		dialog->Open();
		UpdateCodes();
	}
}
