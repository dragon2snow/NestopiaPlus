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
#include "NstMapper060.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Mapper60::CartSwitches : public DipSwitches
		{
			uint mode;
			const Attribute type;

		public:

			explicit CartSwitches(const Context& c)
			: mode(0), type(static_cast<Attribute>(c.attribute)) {}

			static bool CanCreate(const Context& c)
			{
				switch (c.attribute)
				{
					case ATR_STD:
					case ATR_6_IN_1:
					case ATR_65_IN_1:
					case ATR_55_IN_1:
					case ATR_54_IN_1:
					case ATR_12_IN_1:
					case ATR_28_IN_1:

						return true;
				}

				return false;
			}

			void SetMode(uint value)
			{
				mode = value;
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
					ATR_STD     == 0 &&
					ATR_6_IN_1  == 1 &&
					ATR_65_IN_1 == 2 &&
					ATR_55_IN_1 == 3 &&
					ATR_54_IN_1 == 4 &&
					ATR_12_IN_1 == 5 &&
					ATR_28_IN_1 == 6
				);

				static cstring const names[7][4] =
				{
					{ "1",       "2",       "3",       "4"        },
					{ "6-in-1",  "15-in-1", "35-in-1", "43-in-1"  },
					{ "65-in-1", "75-in-1", "85-in-1", "95-in-1"  },
					{ "55-in-1", "65-in-1", "75-in-1", "85-in-1"  },
					{ "54-in-1", "64-in-1", "74-in-1", "84-in-1"  },
					{ "12-in-1", "66-in-1", "77-in-1", "88-in-1"  },
					{ "28-in-1", "46-in-1", "63-in-1", "118-in-1" }
				};

				return names[type][i];
			}
		};

		Mapper60::Mapper60(Context& c)
		:
		Mapper       (c,(PROM_MAX_128K|CROM_MAX_64K|WRAM_DEFAULT) | (CartSwitches::CanCreate(c) ? NMT_VERTICAL : NMT_DEFAULT)),
		cartSwitches (CartSwitches::CanCreate(c) ? new CartSwitches(c) : NULL)
		{}

		Mapper60::~Mapper60()
		{
			delete cartSwitches;
		}

		Mapper60::Device Mapper60::QueryDevice(DeviceType type)
		{
			if (type == DEVICE_DIP_SWITCHES)
				return cartSwitches;
			else
				return Mapper::QueryDevice( type );
		}

		void Mapper60::SubReset(const bool hard)
		{
			if (cartSwitches)
			{
				Map( 0x8000U, 0xFFFFU, &Mapper60::Peek_Prg, &Mapper60::Poke_Prg );

				if (hard)
					NES_DO_POKE(Prg,0x8000,0x00);
			}
			else
			{
				if (hard)
					resetSwitch = 0;
				else
					resetSwitch = (resetSwitch + 1) & 0x3;

				chr.SwapBank<SIZE_8K,0x0000>( resetSwitch );
				prg.SwapBanks<SIZE_16K,0x0000>( resetSwitch, resetSwitch );
			}
		}

		void Mapper60::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'R','E','G'>::V)
				{
					const uint data = state.Read8();

					if (cartSwitches)
					{
						cartMode = data << 1 & 0x100;
						cartSwitches->SetMode( data & 0x3 );
					}
					else
					{
						resetSwitch = data & 0x3;
					}
				}

				state.End();
			}
		}

		void Mapper60::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( cartSwitches ? (cartSwitches->GetMode() | cartMode >> 1) : resetSwitch ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK_A(Mapper60,Prg)
		{
			return !cartMode ? prg.Peek( address - 0x8000 ) : cartSwitches->GetMode();
		}

		NES_POKE_A(Mapper60,Prg)
		{
			cartMode = address & 0x100;
			ppu.SetMirroring( (address & 0x8) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			prg.SwapBanks<SIZE_16K,0x0000>( (address >> 4) & ~(~address >> 7 & 0x1), (address >> 4) | (~address >> 7 & 0x1) );
			chr.SwapBank<SIZE_8K,0x0000>( address );
		}
	}
}
