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
#include "../NstCrc32.hpp"
#include "NstBrdMmc3.hpp"
#include "NstBrdFk23C.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			#if NST_MSVC >= 1200
			#pragma warning( push )
			#pragma warning( disable : 4309 )
			#endif

			class Fk23C::CartSwitches : public DipSwitches
			{
				enum Type
				{
					CRC_4_IN_1  = 0x38BA830E,
					CRC_15_IN_1 = 0x30FF6159,
					CRC_18_IN_1 = 0x83A38A2F,
					CRC_20_IN_1 = 0xFD9D1925
				};

				uint mode;
				const Type type;

				explicit CartSwitches(Type t)
				: mode(0), type(t) {}

			public:

				static CartSwitches* Create(const Context& c)
				{
					switch (const dword crc = Crc32::Compute(c.prg.Mem(),c.prg.Size()))
					{
						case CRC_4_IN_1:
						case CRC_15_IN_1:
						case CRC_18_IN_1:
						case CRC_20_IN_1:

							return new CartSwitches( static_cast<Type>(crc) );
					}

					return NULL;
				}

				void SetMode(uint value)
				{
					const uint num = (type == CRC_4_IN_1 ? 1 : 7);
					mode = NST_MIN(value,num);
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
					return type == CRC_4_IN_1 ? 2 : 8;
				}

				cstring GetDipName(uint) const
				{
					return "Mode";
				}

				cstring GetValueName(uint,uint i) const
				{
					switch (type)
					{
						case CRC_4_IN_1:
						{
							static const char names[2][8] =
							{
								"4-in-1",
								"7-in-1"
							};

							return names[i];
						}

						case CRC_15_IN_1:
						case CRC_20_IN_1:
						{
							static const char names[8][9] =
							{
								"15-in-1",
								"80-in-1",
								"160-in-1",
								"20-in-1",
								"99-in-1",
								"210-in-1",
								"25-in-1",
								"260-in-1"
							};

							return names[i];
						}

						case CRC_18_IN_1:
						{
							static const char names[8][9] =
							{
								"18-in-1",
								"58-in-1",
								"160-in-1",
								"15-in-1",
								"52-in-1",
								"180-in-1",
								"30-in-1",
								"288-in-1"
							};

							return names[i];
						}
					}

					return NULL;
				}
			};

			#if NST_MSVC >= 1200
			#pragma warning( pop )
			#endif

			Fk23C::Fk23C(Context& c)
			:
			Mmc3         (c,BRD_GENERIC,PROM_MAX_1024K|CROM_MAX_1024K|WRAM_DEFAULT),
			cartSwitches (CartSwitches::Create(c))
			{}

			Fk23C::~Fk23C()
			{
				delete cartSwitches;
			}

			void Fk23C::SubReset(const bool hard)
			{
				for (uint i=0; i < 4; ++i)
					exRegs[i] = 0x00;

				for (uint i=4; i < 8; ++i)
					exRegs[i] = 0xFF;

				unromChr = 0x0;

				Mmc3::SubReset( hard );

				Map( 0x5001U, 0x5FFFU, &Fk23C::Poke_5001 );
				Map( 0x8000U, 0xFFFFU, &Fk23C::Poke_Prg  );
			}

			Fk23C::Device Fk23C::QueryDevice(DeviceType type)
			{
				if (type == DEVICE_DIP_SWITCHES)
					return cartSwitches;
				else
					return Mapper::QueryDevice( type );
			}

			void Fk23C::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == AsciiId<'R','E','G'>::V)
					{
						State::Loader::Data<9> data( state );

						for (uint i=0; i < 8; ++i)
							exRegs[i] = data[i];

						unromChr = data[8] & 0x3;

						if (cartSwitches)
							cartSwitches->SetMode( data[8] >> 2 & 0x7 );
					}

					state.End();
				}
			}

			void Fk23C::SubSave(State::Saver& state) const
			{
				const byte data[] =
				{
					exRegs[0],
					exRegs[1],
					exRegs[2],
					exRegs[3],
					exRegs[4],
					exRegs[5],
					exRegs[6],
					exRegs[7],
					unromChr | (cartSwitches ? cartSwitches->GetMode() << 2 : 0)
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Fk23C::UpdatePrg()
			{
				if ((exRegs[0] & 0x7U) == 4)
				{
					prg.SwapBank<SIZE_32K,0x0000>( exRegs[1] >> 1 );
				}
				else if ((exRegs[0] & 0x7U) == 3)
				{
					prg.SwapBanks<SIZE_16K,0x0000>( exRegs[1], exRegs[1] );
				}
				else
				{
					const uint exBanks[2] =
					{
						(exRegs[0] & 0x2U) ? 0x3FU >> (exRegs[0] & 0x3U) : 0xFF,
						(exRegs[0] & 0x2U) ? exRegs[1] << 1 : 0x00
					};

					const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

					prg.SwapBanks<SIZE_8K,0x0000>
					(
						(banks.prg[i] & exBanks[0]) | exBanks[1],
						(banks.prg[1] & exBanks[0]) | exBanks[1],
						(exRegs[3] & 0x2U) ? exRegs[4] : (banks.prg[i^2] & exBanks[0]) | exBanks[1],
						(exRegs[3] & 0x2U) ? exRegs[5] : (banks.prg[3]   & exBanks[0]) | exBanks[1]
					);
				}
			}

			void Fk23C::UpdateChr() const
			{
				ppu.Update();

				if (exRegs[0] & 0x40U)
				{
					chr.SwapBank<SIZE_8K,0x0000>( exRegs[2] | unromChr );
				}
				else
				{
					uint base = (exRegs[2] & 0x7FU) << 2;
					const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

					chr.SwapBanks<SIZE_2K>( 0x0000 ^ swap, base | banks.chr[0], base | banks.chr[1] );
					base <<= 1;
					chr.SwapBanks<SIZE_1K>( 0x1000 ^ swap, base | banks.chr[2], base | banks.chr[3], base | banks.chr[4], base | banks.chr[5] );

					if (exRegs[3] & 0x2U)
					{
						chr.SwapBank<SIZE_1K,0x0400>( base | exRegs[6] );
						chr.SwapBank<SIZE_1K,0x0C00>( base | exRegs[7] );
					}
				}
			}

			NES_POKE_AD(Fk23C,5001)
			{
				if (address & (1U << ((cartSwitches ? cartSwitches->GetMode() : 0) + 4)))
				{
					exRegs[address & 0x3] = data;
					Fk23C::UpdatePrg();
					Fk23C::UpdateChr();
				}
			}

			NES_POKE_AD(Fk23C,Prg)
			{
				if (exRegs[0] & 0x40U)
				{
					unromChr = (exRegs[0] & 0x20U) ? 0x0 : data & 0x3;
					Fk23C::UpdateChr();
				}
				else switch (address & 0xE001)
				{
					case 0x8000: Mmc3::NES_DO_POKE(8000,address,data); break;
					case 0x8001:

						if (exRegs[3] << 2 & (regs.ctrl0 & 0x8))
						{
							exRegs[4 | regs.ctrl0 & 0x3] = data;
							Fk23C::UpdatePrg();
							Fk23C::UpdateChr();
						}
						else
						{
							Mmc3::NES_DO_POKE(8001,address,data);
						}
						break;

					case 0xA000:
					case 0xA001: Mmc3::NES_DO_POKE(A001,address,data); break;
					case 0xC000: Mmc3::NES_DO_POKE(C000,address,data); break;
					case 0xC001: Mmc3::NES_DO_POKE(C001,address,data); break;
					case 0xE000: Mmc3::NES_DO_POKE(E000,address,data); break;
					case 0xE001: Mmc3::NES_DO_POKE(E001,address,data); break;

					NST_UNREACHABLE
				}
			}
		}
	}
}
