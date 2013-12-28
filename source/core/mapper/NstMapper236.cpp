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
#include "NstMapper236.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper236::CartSwitches::CartSwitches(const Context& c)
		:
		mode (c.attribute == ATR_800_IN_1 ? 0x6 : c.attribute == ATR_76_IN_1 ? 0xD : 0x0),
		type (c.attribute < NUM_ATTRIBUTES ? static_cast<Attribute>(c.attribute) : ATR_GENERIC)
		{}

		inline void Mapper236::CartSwitches::SetMode(uint value)
		{
			mode = value;
		}

		inline uint Mapper236::CartSwitches::GetMode() const
		{
			return mode;
		}

		uint Mapper236::CartSwitches::GetValue(uint) const
		{
			return mode;
		}

		void Mapper236::CartSwitches::SetValue(uint,uint value)
		{
			mode = value;
		}

		uint Mapper236::CartSwitches::NumDips() const
		{
			return 1;
		}

		uint Mapper236::CartSwitches::NumValues(uint) const
		{
			return 16;
		}

		cstring Mapper236::CartSwitches::GetDipName(uint) const
		{
			return "Mode";
		}

		cstring Mapper236::CartSwitches::GetValueName(uint,uint i) const
		{
			NST_COMPILE_ASSERT
			(
				ATR_GENERIC  == 0 &&
				ATR_800_IN_1 == 1 &&
				ATR_76_IN_1  == 2
			);

			static cstring const names[3][16] =
			{
				{
					"1",  "2",  "3",  "4",
					"5",  "6",  "7",  "8",
					"9",  "10", "11", "12",
					"13", "14", "15", "16"
				},
				{
					"76-in-1",   "150-in-1",     "168-in-1",     "190-in-1",
					"400-in-1",  "500-in-1",     "800-in-1",     "1200-in-1",
					"2000-in-1", "5000-in-1",    "300-in-1",     "1500-in-1",
					"3000-in-1", "1010000-in-1", "5010000-in-1", "10000000-in-1"
				},
				{
					"4-in-1",  "5-in-1",  "6-in-1",  "77-in-1",
					"22-in-1", "38-in-1", "44-in-1", "46-in-1",
					"52-in-1", "55-in-1", "63-in-1", "66-in-1",
					"68-in-1", "70-in-1", "32-in-1", "80-in-1"
				}
			};

			return names[type][i];
		}

		Mapper236::Mapper236(Context& c)
		:
		Mapper       (c,CROM_MAX_64K|WRAM_DEFAULT|NMT_VERTICAL),
		cartSwitches (c)
		{}

		Mapper236::Device Mapper236::QueryDevice(DeviceType type)
		{
			if (type == DEVICE_DIP_SWITCHES)
				return &cartSwitches;
			else
				return Mapper::QueryDevice( type );
		}

		void Mapper236::SubReset(bool)
		{
			mode = 0x0;

			Map( 0x8000U, 0xFFFFU, &Mapper236::Peek_Prg, &Mapper236::Poke_Prg );

			NES_DO_POKE(Prg,0x8000,0x00);
			NES_DO_POKE(Prg,0xC000,0x00);
		}

		void Mapper236::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
				{
					const uint data = state.Read8();
					mode = data >> 4 & 0x1;
					cartSwitches.SetMode( data & 0xF );
				}

				state.End();
			}
		}

		void Mapper236::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( cartSwitches.GetMode() | (mode << 4) ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK_A(Mapper236,Prg)
		{
			return prg.Peek( mode ? (address & 0x7FF0) | cartSwitches.GetMode() : address - 0x8000 );
		}

		NES_POKE_A(Mapper236,Prg)
		{
			uint banks[2] =
			{
				prg.GetBank<SIZE_16K,0x0000>(),
				prg.GetBank<SIZE_16K,0x4000>()
			};

			if (address < 0xC000)
			{
				ppu.SetMirroring( (address & 0x20) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );

				if (chr.Source().Writable())
				{
					banks[0] = (banks[0] & 0x7) | (address << 3 & 0x38);
					banks[1] = (banks[1] & 0x7) | (address << 3 & 0x38);
				}
				else
				{
					chr.SwapBank<SIZE_8K,0x0000>( address & 0x7 );
					return;
				}
			}
			else switch (address & 0x30)
			{
				case 0x00: mode = 0x0; banks[0] = (banks[0] & 0x38) | (address & 0x7); banks[1] = banks[0] | 0x7; break;
				case 0x10: mode = 0x1; banks[0] = (banks[0] & 0x38) | (address & 0x7); banks[1] = banks[0] | 0x7; break;
				case 0x20: mode = 0x0; banks[0] = (banks[0] & 0x38) | (address & 0x6); banks[1] = banks[0] | 0x1; break;
				case 0x30: mode = 0x0; banks[0] = (banks[0] & 0x38) | (address & 0x7); banks[1] = banks[0] | 0x0; break;
			}

			prg.SwapBanks<SIZE_16K,0x0000>( banks[0], banks[1] );
		}
	}
}
