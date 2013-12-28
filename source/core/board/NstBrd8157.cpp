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

#include "../NstMapper.hpp"
#include "../NstDipSwitches.hpp"
#include "NstBrd8157.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Unl8157::CartSwitches::CartSwitches()
			: mode(0x100) {}

			inline void Unl8157::CartSwitches::SetMode(uint value)
			{
				mode = value ? 0x100 : 0x000;
			}

			inline uint Unl8157::CartSwitches::GetMode() const
			{
				return mode;
			}

			uint Unl8157::CartSwitches::GetValue(uint) const
			{
				return mode ? 0 : 1;
			}

			void Unl8157::CartSwitches::SetValue(uint,uint value)
			{
				mode = value ? 0x000 : 0x100;
			}

			uint Unl8157::CartSwitches::NumDips() const
			{
				return 1;
			}

			uint Unl8157::CartSwitches::NumValues(uint) const
			{
				return 2;
			}

			cstring Unl8157::CartSwitches::GetDipName(uint) const
			{
				return "Mode";
			}

			cstring Unl8157::CartSwitches::GetValueName(uint,uint i) const
			{
				return i ? "20-in-1" : "4-in-1";
			}

			Unl8157::Device Unl8157::QueryDevice(DeviceType type)
			{
				if (type == DEVICE_DIP_SWITCHES)
					return &cartSwitches;
				else
					return Mapper::QueryDevice( type );
			}

			void Unl8157::SubReset(const bool hard)
			{
				Map( 0x8000U, 0xFFFFU, &Unl8157::Peek_Prg, &Unl8157::Poke_Prg );

				trash = 0;

				if (hard)
					NES_DO_POKE(Prg,0x8000,0x00);
			}

			void Unl8157::SubSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'R','E','G'>::V ).Write8( (cartSwitches.GetMode() ? 0x1 : 0x0) | (trash >> 7) ).End();
			}

			void Unl8157::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == AsciiId<'R','E','G'>::V)
					{
						trash = state.Read8();
						cartSwitches.SetMode( trash & 0x1 );
						trash = trash << 7 & 0x100;
					}

					state.End();
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_PEEK_A(Unl8157,Prg)
			{
				return !trash ? prg.Peek( address - 0x8000 ) : 0xFF;
			}

			NES_POKE_A(Unl8157,Prg)
			{
				trash = address & cartSwitches.GetMode();

				prg.SwapBanks<SIZE_16K,0x0000>
				(
					(address >> 2 & 0x18) | (address >> 2 & 0x7),
					(address >> 2 & 0x18) | ((address & 0x200) ? 0x7 : 0x0)
				);

				ppu.SetMirroring( (address & 0x2) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			}
		}
	}
}
