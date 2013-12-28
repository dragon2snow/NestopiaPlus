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

#include "NstManager.hpp"
#include "NstDialogBarcodeReader.hpp"
#include "NstManagerBarcodeReader.hpp"

namespace Nestopia
{
	namespace Managers
	{
		BarcodeReader::BarcodeReader(Emulator& e,Window::Menu& m)
		: Manager(e,m,this,&BarcodeReader::OnEmuEvent,IDM_MACHINE_EXT_BARCODE,&BarcodeReader::OnMenuCmd)
		{
		}

		BarcodeReader::~BarcodeReader()
		{
		}

		void BarcodeReader::OnMenuCmd(uint)
		{
			Window::BarcodeReader( emulator, lastCode ).Open();
		}

		void BarcodeReader::OnEmuEvent(Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_INIT:
				case Emulator::EVENT_POWER_ON:
				case Emulator::EVENT_POWER_OFF:

					menu[IDM_MACHINE_EXT_BARCODE].Enable
					(
						event == Emulator::EVENT_POWER_ON &&
						Nes::BarcodeReader(emulator).IsConnected()
					);
					break;
			}
		}
	}
}
