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
#include "NstMapper150.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Mapper150::CartSwitches : public DipSwitches
		{
			uint copyright;

		public:

			CartSwitches()
			: copyright(1) {}

			void SetCopyright(uint value)
			{
				copyright = value;
			}

			uint GetCopyright() const
			{
				return copyright;
			}

		private:

			uint GetValue(uint) const
			{
				return copyright ^ 1;
			}

			void SetValue(uint,uint value)
			{
				copyright = value ^ 1;
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
				return "Copyright";
			}

			cstring GetValueName(uint,uint i) const
			{
				return i ? "Sachen & Hacker" : "Sachen";
			}
		};

		Mapper150::Mapper150(Context& c)
		:
		Mapper       (c,CROM_MAX_1024K|WRAM_DEFAULT|NMT_VERTICAL),
		cartSwitches (c.attribute == ATR_COPYRIGHT ? new CartSwitches : NULL)
		{}

		Mapper150::~Mapper150()
		{
			delete cartSwitches;
		}

		Mapper150::Device Mapper150::QueryDevice(DeviceType type)
		{
			if (type == DEVICE_DIP_SWITCHES)
				return cartSwitches;
			else
				return Mapper::QueryDevice( type );
		}

		void Mapper150::SubReset(const bool hard)
		{
			for (uint i=0x4100; i < 0x6000; ++i)
			{
				switch (i & 0x4101)
				{
					case 0x4100: Map( i, &Mapper150::Peek_4100, &Mapper150::Poke_4100 ); break;
					case 0x4101: Map( i, &Mapper150::Peek_4100, &Mapper150::Poke_4101 ); break;
				}
			}

			if (hard)
			{
				command = 0;
				prg.SwapBank<SIZE_32K,0x0000>(0);
			}
		}

		void Mapper150::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:

						command = state.Read8();
						break;

					case AsciiId<'D','I','P'>::V:

						NST_VERIFY( cartSwitches );

						if (cartSwitches)
							cartSwitches->SetCopyright( state.Read8() & 0x1 );

						break;
				}

				state.End();
			}
		}

		void Mapper150::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'R','E','G'>::V ).Write8( command ).End();

			if (cartSwitches)
				state.Begin( AsciiId<'D','I','P'>::V ).Write8( cartSwitches->GetCopyright() ? 0x1 : 0x0 ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK(Mapper150,4100)
		{
			return (~(command & 0x7) & 0x3F) ^ (cartSwitches ? cartSwitches->GetCopyright() : 0x1);
		}

		NES_POKE_D(Mapper150,4100)
		{
			command = data;
		}

		NES_POKE_D(Mapper150,4101)
		{
			uint banks[2] = {~0U,~0U};

			switch (command & 0x7)
			{
				case 0x2:

					banks[0] = data & 0x1;
					banks[1] = (chr.GetBank<SIZE_8K,0x0000>() & ~0x8U) | (data << 3 & 0x8);
					break;

				case 0x4:

					banks[1] = (chr.GetBank<SIZE_8K,0x0000>() & ~0x4U) | (data << 2 & 0x4);
					break;

				case 0x5:

					banks[0] = data & 0x7;
					break;

				case 0x6:

					banks[1] = (chr.GetBank<SIZE_8K,0x0000>() & ~0x3U) | (data << 0 & 0x3);
					break;

				case 0x7:
				{
					static const byte lut[4][4] =
					{
						{0,1,0,1},
						{0,0,1,1},
						{0,1,1,1},
						{0,0,0,0}
					};

					ppu.SetMirroring( lut[data >> 1 & 0x3] );
					return;
				}
			}

			if (banks[0] != ~0U)
				prg.SwapBank<SIZE_32K,0x0000>( banks[0] );

			if (banks[1] != ~0U)
			{
				ppu.Update();
				chr.SwapBank<SIZE_8K,0x0000>( banks[1] );
			}
		}
	}
}
