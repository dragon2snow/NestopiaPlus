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
#include "NstBrdMmc1.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			uint Mmc1::BoardToWRam(Board board)
			{
				switch (board)
				{
					case BRD_GENERIC:

						return WRAM_AUTO;

					case BRD_SAROM:
					case BRD_SJROM:
					case BRD_SKROM:
					case BRD_SNROM:
					case BRD_SUROM:
					case BRD_GENERIC_WRAM:

						return WRAM_8K;

					case BRD_SOROM:

						return WRAM_16K;

					case BRD_SXROM:

						return WRAM_32K;

					default:

						return WRAM_NONE;
				}
			}

			Mmc1::Mmc1(Context& c,Board b,Revision rev)
			:
			Mapper   (c,BoardToWRam(b) | (PROM_MAX_512K|CROM_MAX_128K)),
			revision (rev)
			{
			}

			void Mmc1::ResetRegisters()
			{
				serial.buffer = 0;
				serial.shifter = 0;

				regs[CTRL] = CTRL_RESET;
				regs[CHR0] = 0;
				regs[CHR1] = 0;
				regs[PRG0] = (revision == REV_C ? PRG0_WRAM_DISABLED : 0);
			}

			void Mmc1::SubReset(const bool hard)
			{
				if (wrk.Size() >= SIZE_8K && revision != REV_A)
					Map( WRK_SAFE_PEEK_POKE );

				Map( 0x8000U, 0xFFFFU, &Mmc1::Poke_Prg );

				serial.ready = cpu.GetMasterClockCycle(Serial::RESET_CYCLES);

				if (hard)
				{
					ResetRegisters();

					for (uint i=0; i < 4; ++i)
						UpdateRegisters( i );
				}
			}

			void Mmc1::BaseLoad(State::Loader& state,dword id)
			{
				NST_VERIFY( id == (AsciiId<'M','M','1'>::V) );

				serial.ready = 0;

				if (id == AsciiId<'M','M','1'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<4+2> data( state );

							for (uint i=0; i < 4; ++i)
								regs[i] = data[i] & 0x1F;

							serial.buffer = data[4] & 0x1F;
							serial.shifter = NST_MIN(data[5],5);
						}

						state.End();
					}
				}
			}

			void Mmc1::BaseSave(State::Saver& state) const
			{
				const byte data[4+2] =
				{
					regs[0],
					regs[1],
					regs[2],
					regs[3],
					serial.buffer,
					serial.shifter
				};

				state.Begin( AsciiId<'M','M','1'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Mmc1::UpdatePrg()
			{
				prg.SwapBanks<SIZE_16K,0x0000>
				(
					(regs[CHR0] & 0x10U) | ((regs[PRG0] | 0x0U) & ((regs[CTRL] & uint(CTRL_PRG_SWAP_16K)) ? (regs[CTRL] & uint(CTRL_PRG_SWAP_LOW)) ? 0xF : 0x0 : 0xE)),
					(regs[CHR0] & 0x10U) | ((regs[PRG0] & 0xFU) | ((regs[CTRL] & uint(CTRL_PRG_SWAP_16K)) ? (regs[CTRL] & uint(CTRL_PRG_SWAP_LOW)) ? 0xF : 0x0 : 0x1))
				);
			}

			void Mmc1::UpdateWrk()
			{
				wrk.Source().SetSecurity
				(
					~uint(regs[PRG0]) & PRG0_WRAM_DISABLED,
					~uint(regs[PRG0]) & PRG0_WRAM_DISABLED
				);

				if (wrk.Source().Size() >= SIZE_16K)
					wrk.SwapBank<SIZE_8K,0x0000>( regs[CHR0] >> (2 + (wrk.Source().Size() == SIZE_16K)) );
			}

			void Mmc1::UpdateChr() const
			{
				ppu.Update();

				const uint mode = regs[CTRL] >> 4 & 0x1U;

				chr.SwapBanks<SIZE_4K,0x0000>
				(
					regs[CHR0] & (0x1E | mode),
					regs[CHR0+mode] & 0x1FU | (mode^1)
				);
			}

			void Mmc1::UpdateNmt()
			{
				static const byte lut[4][4] =
				{
					{0,0,0,0},
					{1,1,1,1},
					{0,1,0,1},
					{0,0,1,1}
				};

				ppu.SetMirroring( lut[regs[CTRL] & uint(CTRL_MIRRORING)] );
			}

			void Mmc1::UpdateRegisters(const uint index)
			{
				NST_ASSERT( index < 4 );

				if (index != CHR1)
				{
					UpdatePrg();

					if (wrk.Available())
						UpdateWrk();
				}

				if (index != PRG0)
				{
					if (index == CTRL)
						UpdateNmt();

					UpdateChr();
				}
			}

			NES_POKE(Mmc1,Prg)
			{
				if (cpu.GetMasterClockCycles() >= serial.ready)
				{
					if (!(data & Serial::RESET_BIT))
					{
						serial.buffer |= (data & 0x1) << serial.shifter++;

						if (serial.shifter != 5)
							return;

						serial.shifter = 0;
						data = serial.buffer;
						serial.buffer = 0;

						address = address >> 13 & 0x3;

						if (regs[address] != data)
						{
							regs[address] = data;
							UpdateRegisters( address );
						}
					}
					else
					{
						serial.ready = cpu.GetMasterClockCycles() + cpu.GetMasterClockCycle(Serial::RESET_CYCLES);
						serial.buffer = 0;
						serial.shifter = 0;

						if ((regs[CTRL] & uint(CTRL_RESET)) != CTRL_RESET)
						{
							regs[CTRL] |= uint(CTRL_RESET);
							UpdateRegisters( CTRL );
						}
					}
				}
				else
				{
					// looks like there's some protection from rapid writes on register
					// reset, otherwise games like 'AD&D Hillsfar' and 'Bill & Ted' will break

					NST_DEBUG_MSG("MMC1 PRG write ignored!");
				}
			}

			void Mmc1::VSync()
			{
				if (serial.ready <= cpu.GetMasterClockFrameCycles())
					serial.ready = 0;
				else
					serial.ready -= cpu.GetMasterClockFrameCycles();
			}
		}
	}
}
