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
#include "NstMapper233.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Mapper233::CartSwitches : public DipSwitches
		{
			enum Type
			{
				TYPE_22_20_IN_1,
				TYPE_GENERIC
			};

			uint hiPrg;
			const Type type;

		public:

			explicit CartSwitches(const Context& c)
			:
			hiPrg (0x00),
			type  (c.attribute == ATR_22_20_IN_1 ? TYPE_22_20_IN_1 : TYPE_GENERIC)
			{}

			void EnableHiPrg(bool enable)
			{
				hiPrg = enable ? 0x20 : 0x00;
			}

			uint GetHiPrg() const
			{
				return hiPrg;
			}

		private:

			uint GetValue(uint) const
			{
				return hiPrg ? 1 : 0;
			}

			void SetValue(uint,uint value)
			{
				hiPrg = value ? 0x20 : 0x00;
			}

			uint NumDips() const
			{
				return 1;
			}

			uint NumValues(uint) const
			{
				return 2;
			}

			cstring GetDipName(uint) const
			{
				return "Mode";
			}

			cstring GetValueName(uint,uint i) const
			{
				return i ? (type == TYPE_22_20_IN_1 ? "20-in-1" : "2") :
                           (type == TYPE_22_20_IN_1 ? "22-in-1" : "1");
			}
		};

		Mapper233::Mapper233(Context& c)
		:
		Mapper       (c,PROM_MAX_1024K|CROM_MAX_8K|WRAM_DEFAULT|NMT_ZERO),
		cartSwitches (prg.Source().Size() == SIZE_1024K ? new CartSwitches(c) : NULL)
		{}

		Mapper233::~Mapper233()
		{
			delete cartSwitches;
		}

		Mapper233::Device Mapper233::QueryDevice(DeviceType type)
		{
			if (type == DEVICE_DIP_SWITCHES)
				return cartSwitches;
			else
				return Mapper::QueryDevice( type );
		}

		void Mapper233::SubReset(bool)
		{
			Map( 0x8000U, 0xFFFFU, &Mapper233::Poke_Prg );
			NES_DO_POKE(Prg,0x8000,0x00);
		}

		void Mapper233::SubSave(State::Saver& state) const
		{
			if (cartSwitches)
				state.Begin( AsciiId<'R','E','G'>::V ).Write8( cartSwitches->GetHiPrg() ? 0x1 : 0x0 ).End();
		}

		void Mapper233::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
				{
					NST_VERIFY( cartSwitches );

					if (cartSwitches)
						cartSwitches->EnableHiPrg( state.Read8() & 0x1 );
				}

				state.End();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_D(Mapper233,Prg)
		{
			const uint hiPrg = cartSwitches ? cartSwitches->GetHiPrg() : 0x00;

			if (data & 0x20)
				prg.SwapBanks<SIZE_16K,0x0000>( hiPrg | (data & 0x1F), hiPrg | (data & 0x1F) );
			else
				prg.SwapBank<SIZE_32K,0x0000>( (hiPrg >> 1) | (data >> 1 & 0xF) );

			static const byte lut[4][4] =
			{
				{0,0,0,0},
				{0,1,0,1},
				{0,0,1,1},
				{1,1,1,1}
			};

			ppu.SetMirroring( lut[data >> 6] );
		}
	}
}
