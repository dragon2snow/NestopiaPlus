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

#include "../NstCore.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiMachine.hpp"
#include "NstApiDipSwitches.hpp"
#include "../NstCartridge.hpp"
#include "../vssystem/NstVsSystem.hpp"
#include "../NstMapper.hpp"
#include "../board/NstBrdMmc1.hpp"
#include "../board/NstBrdBtlTek2A.hpp"
#include "../mapper/NstMapper105.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Api
	{
		uint DipSwitches::NumDips() const
		{
			if (emulator.Is(Machine::CARTRIDGE))
			{
				const Core::Cartridge& cartridge = *static_cast<const Core::Cartridge*>(emulator.image);
	
				if (emulator.Is(Machine::VS))
					return cartridge.GetVsSystem()->NumDipSwitches();
	
				switch (cartridge.GetMapper().GetID())
				{
        			case 105:
						return static_cast<const Core::Mapper105&>(cartridge.GetMapper()).NumDipSwitches();
	
					case 90:
					case 209:
						return static_cast<const Core::Boards::BtlTek2A&>(cartridge.GetMapper()).NumDipSwitches();
				}
			}
	
			return 0;
		}
	
		uint DipSwitches::NumValues(uint dip) const
		{
			if (emulator.Is(Machine::CARTRIDGE))
			{
				const Core::Cartridge& cartridge = *static_cast<const Core::Cartridge*>(emulator.image);
	
				if (emulator.Is(Machine::VS))
					return cartridge.GetVsSystem()->NumDipSwitchValues( dip );
	
				switch (cartridge.GetMapper().GetID())
				{
        			case 105:
						return static_cast<const Core::Mapper105&>(cartridge.GetMapper()).NumDipSwitchValues( dip );
	
					case 90:
					case 209:
						return static_cast<const Core::Boards::BtlTek2A&>(cartridge.GetMapper()).NumDipSwitchValues( dip );
				}
			}
	
			return 0;
		}
	
		cstring DipSwitches::GetDipName(uint dip) const
		{
			if (emulator.Is(Machine::CARTRIDGE))
			{
				const Core::Cartridge& cartridge = *static_cast<const Core::Cartridge*>(emulator.image);
	
				if (emulator.Is(Machine::VS))
					return cartridge.GetVsSystem()->GetDipSwitchName( dip );
	
				switch (cartridge.GetMapper().GetID())
				{
        			case 105:
						return static_cast<const Core::Mapper105&>(cartridge.GetMapper()).GetDipSwitchName( dip );
	
					case 90:
					case 209:
						return static_cast<const Core::Boards::BtlTek2A&>(cartridge.GetMapper()).GetDipSwitchName( dip );
				}
			}
	
			return NULL;
		}
	
		cstring DipSwitches::GetValueName(uint dip,uint value) const
		{
			if (emulator.Is(Machine::CARTRIDGE))
			{
				const Core::Cartridge& cartridge = *static_cast<const Core::Cartridge*>(emulator.image);
	
				if (emulator.Is(Machine::VS))
					return cartridge.GetVsSystem()->GetDipSwitchValueName( dip, value );
	
				switch (cartridge.GetMapper().GetID())
				{
        			case 105:
						return static_cast<const Core::Mapper105&>(cartridge.GetMapper()).GetDipSwitchValueName( dip, value );
	
					case 90:
					case 209:
						return static_cast<const Core::Boards::BtlTek2A&>(cartridge.GetMapper()).GetDipSwitchValueName( dip, value );
				}
			}
	
			return NULL;
		}
	
		int DipSwitches::GetValue(uint dip) const
		{
			if (emulator.Is(Machine::CARTRIDGE))
			{
				const Core::Cartridge& cartridge = *static_cast<const Core::Cartridge*>(emulator.image);
	
				if (emulator.Is(Machine::VS))
					return cartridge.GetVsSystem()->GetDipSwitchValue( dip );
	
				switch (cartridge.GetMapper().GetID())
				{
        			case 105:
						return static_cast<const Core::Mapper105&>(cartridge.GetMapper()).GetDipSwitchValue( dip );
	
					case 90:
					case 209:
						return static_cast<const Core::Boards::BtlTek2A&>(cartridge.GetMapper()).GetDipSwitchValue( dip );
				}
			}
	
			return INVALID;
		}
	
		Result DipSwitches::SetValue(uint dip,uint value)
		{
			if (emulator.Is(Machine::CARTRIDGE))
			{
				Core::Cartridge& cartridge = *static_cast<Core::Cartridge*>(emulator.image);
	
				if (emulator.Is(Machine::VS))
					return cartridge.GetVsSystem()->SetDipSwitchValue( dip, value );
	
				switch (cartridge.GetMapper().GetID())
				{
        			case 105:
						return static_cast<Core::Mapper105&>(cartridge.GetMapper()).SetDipSwitchValue( dip, value );
	
					case 90:
					case 209:
						return static_cast<Core::Boards::BtlTek2A&>(cartridge.GetMapper()).SetDipSwitchValue( dip, value );
				}
			}
	
			return RESULT_ERR_NOT_READY;
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
