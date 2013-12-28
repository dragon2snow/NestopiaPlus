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
#include "NstMapper057.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Mapper57::CartSwitches : public DipSwitches
		{
			uint mode;
			const Attribute type;

			explicit CartSwitches(Attribute t)
			: mode(0), type(t) {}

		public:

			static CartSwitches* Create(const Context& c)
			{
				switch (c.attribute)
				{
					case ATR_103:
					case ATR_GK_54:
					case ATR_GK_L01A:
					case ATR_GK_L02A:
					case ATR_GK_47:

						return new CartSwitches( static_cast<Attribute>(c.attribute) );
				}

				return NULL;
			}

			void SetMode(uint value)
			{
				mode = value & 0x3;
			}

			uint GetMode() const
			{
				return mode;
			}

		private:

			uint GetValue(uint) const
			{
				return mode;
			}

			void SetValue(uint,uint value)
			{
				mode = value;
			}

			uint NumDips() const
			{
				return 1;
			}

			uint NumValues(uint) const
			{
				return 4;
			}

			cstring GetDipName(uint) const
			{
				return "Mode";
			}

			cstring GetValueName(uint,uint i) const
			{
				NST_COMPILE_ASSERT
				(
					ATR_103     == 1 &&
					ATR_GK_54   == 2 &&
					ATR_GK_L01A == 3 &&
					ATR_GK_L02A == 4 &&
					ATR_GK_47   == 5
				);

				static cstring const names[5][4] =
				{
					{ "103-in-1", "105-in-1",   "106-in-1",   "109-in-1"   },
					{ "54-in-1",  "74-in-1",    "84-in-1",    "94-in-1"    },
					{ "6-in-1",   "2-in-1 (1)", "2-in-1 (2)", "2-in-1 (3)" },
					{ "6-in-1",   "2-in-1 (1)", "2-in-1 (2)", "2-in-1 (3)" },
					{ "47-in-1",  "57-in-1",    "67-in-1",    "77-in-1"    },
				};

				return names[type-1][i];
			}
		};

		Mapper57::Mapper57(Context& c)
		:
		Mapper       (c,PROM_MAX_128K|CROM_MAX_128K|WRAM_DEFAULT|NMT_VERTICAL),
		cartSwitches (CartSwitches::Create(c))
		{}

		Mapper57::~Mapper57()
		{
			delete cartSwitches;
		}

		Mapper57::Device Mapper57::QueryDevice(DeviceType type)
		{
			if (type == DEVICE_DIP_SWITCHES)
				return cartSwitches;
			else
				return Mapper::QueryDevice( type );
		}

		void Mapper57::SubReset(const bool hard)
		{
			Map( 0x6000U, &Mapper57::Peek_6000 );

			for (dword i=0x8000; i <= 0xFFFF; i += 0x1000)
			{
				Map( uint(i)+0x000, uint(i)+0x7FF, &Mapper57::Poke_8000 );
				Map( uint(i)+0x800, uint(i)+0xFFF, &Mapper57::Poke_8800 );
			}

			if (hard)
			{
				regs[0] = 0;
				regs[1] = 0;

				NES_DO_POKE(8800,0x8800,0x00);
			}
		}

		void Mapper57::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
				{
					State::Loader::Data<3> data( state );

					regs[0] = data[1];
					regs[1] = data[2];

					if (cartSwitches)
						cartSwitches->SetMode( data[0] );
				}

				state.End();
			}
		}

		void Mapper57::SubSave(State::Saver& state) const
		{
			const byte data[3] =
			{
				cartSwitches ? cartSwitches->GetMode() : 0,
				regs[0],
				regs[1]
			};

			state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK(Mapper57,6000)
		{
			return cartSwitches ? cartSwitches->GetMode() : 0x00;
		}

		void Mapper57::UpdateChr() const
		{
			chr.SwapBank<SIZE_8K,0x0000>( (regs[0] >> 1 & 0x8) | (regs[0] & 0x7) | (regs[1] & 0x3) );
		}

		NES_POKE_D(Mapper57,8000)
		{
			regs[1] = data;
			ppu.Update();
			UpdateChr();
		}

		NES_POKE_D(Mapper57,8800)
		{
			regs[0] = data;
			prg.SwapBanks<SIZE_16K,0x0000>( (data >> 5) & ~(data >> 7), (data >> 5) | (data >> 7) );
			ppu.SetMirroring( (data & 0x8) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			UpdateChr();
		}
	}
}
