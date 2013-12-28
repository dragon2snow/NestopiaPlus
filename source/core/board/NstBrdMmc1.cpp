////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Mmc1::Mmc1(Context& c,uint settings,Revision rev)
			:
			Mapper   (c,settings),
			revision (rev)
			{
			}

			void Mmc1::ClearRegisters()
			{
				serial.buffer = 0;
				serial.shifter = 0;

				regs[CTRL] = CTRL_RESET;
				regs[CHR0] = 0;
				regs[CHR1] = 0;
				regs[PRG0] = (revision == REV_1C ? PRG0_WRAM_DISABLED : 0);
			}

			void Mmc1::SubReset(const bool hard)
			{
				if (wrk.HasRam())
				{
					if (revision == REV_1A && wrk.RamSize() != SIZE_16K)
					{
						Map( WRK_PEEK_POKE );
					}
					else Map
					(
						0x6000U, 0x7FFFU,
						wrk.RamSize() == SIZE_16K ? revision == REV_1A ? &Mmc1::Peek_wRam_SoRom_1a : &Mmc1::Peek_wRam_SoRom : &Mmc1::Peek_wRam,
						wrk.RamSize() == SIZE_16K ? revision == REV_1A ? &Mmc1::Poke_wRam_SoRom_1a : &Mmc1::Poke_wRam_SoRom : &Mmc1::Poke_wRam
					);
				}

				Map( 0x8000U, 0xFFFFU, &Mmc1::Poke_Prg );

				serial.time = -2;

				if (hard)
				{
					ClearRegisters();
					UpdatePrg();
					UpdateChr();
					UpdateMirroring();
				}
			}

			void Mmc1::BaseLoad(State::Loader& state,dword id)
			{
				NST_VERIFY( id == NES_STATE_CHUNK_ID('M','M','1','\0') );

				serial.time = -2;

				if (id == NES_STATE_CHUNK_ID('M','M','1','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
						{
							const State::Loader::Data<4+2> data( state );

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
				const u8 data[4+2] =
				{
					regs[0],
					regs[1],
					regs[2],
					regs[3],
					serial.buffer,
					serial.shifter
				};

				state.Begin('M','M','1','\0').Begin('R','E','G','\0').Write( data ).End().End();
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Mmc1::UpdatePrg()
			{
				const uint base = (prg.Source().Size() < SIZE_512K) ? 0 : (regs[PRG1] & PRG1_256K_BANK);
				const uint bank = regs[PRG0] & PRG0_BANK;

				if (regs[CTRL] & CTRL_PRG_SWAP_16K)
				{
					prg.SwapBanks<SIZE_16K,0x0000U>
					(
						base | ((regs[CTRL] & CTRL_PRG_SWAP_LOW) ? bank : 0x0),
						base | ((regs[CTRL] & CTRL_PRG_SWAP_LOW) ? 0xF : bank)
					);
				}
				else
				{
					prg.SwapBank<SIZE_32K,0x0000U>( (base | bank) >> 1 );
				}
			}

			void Mmc1::UpdateChr() const
			{
				ppu.Update();

				chr.SwapBanks<SIZE_4K,0x0000U>
				(
					(regs[CTRL] & CTRL_CHR_SWAP_4K) ? regs[CHR0] : regs[CHR0] & 0x1E,
					(regs[CTRL] & CTRL_CHR_SWAP_4K) ? regs[CHR1] : regs[CHR0] | 0x01
				);
			}

			void Mmc1::UpdateMirroring() const
			{
				static const uchar lut[4][4] =
				{
					{0,0,0,0},
					{1,1,1,1},
					{0,1,0,1},
					{0,0,1,1}
				};

				ppu.SetMirroring( lut[regs[CTRL] & CTRL_MIRRORING] );
			}

			void Mmc1::UpdateRegister0()
			{
				UpdateMirroring();
				UpdateChr();
				UpdatePrg();
			}

			void Mmc1::UpdateRegister1()
			{
				if (prg.Source().Size() >= SIZE_512K)
					UpdatePrg();

				UpdateChr();
			}

			void Mmc1::UpdateRegister2()
			{
				UpdateChr();
			}

			void Mmc1::UpdateRegister3()
			{
				UpdatePrg();
			}

			NES_POKE(Mmc1,Prg)
			{
				const iword time = cpu.GetAutoClockCycles();

				if ((time - serial.time) >= 2)
				{
					if (!(data & Serial::RESET))
					{
						serial.buffer |= (data & 0x1) << serial.shifter++;

						if (serial.shifter != 5)
							return;

						serial.shifter = 0;
						data = serial.buffer;
						serial.buffer = 0;

						address = (address >> 13) & 0x3;

						if (regs[address] != data)
						{
							regs[address] = data;

							switch (address)
							{
								case 0: UpdateRegister0(); break;
								case 1: UpdateRegister1(); break;
								case 2: UpdateRegister2(); break;
								case 3: UpdateRegister3(); break;
							}
						}
					}
					else
					{
						serial.time = time;
						serial.buffer = 0;
						serial.shifter = 0;

						if ((regs[CTRL] & CTRL_RESET) != CTRL_RESET)
						{
							regs[CTRL] |= CTRL_RESET;
							UpdateRegister0();
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

			inline bool Mmc1::IsWRamEnabled() const
			{
				return !(regs[PRG0] & PRG0_WRAM_DISABLED);
			}

			NES_PEEK(Mmc1,wRam)
			{
				NST_VERIFY( IsWRamEnabled() );
				return IsWRamEnabled() ? wrk[0][address - 0x6000U] : (address >> 8);
			}

			NES_POKE(Mmc1,wRam)
			{
				NST_VERIFY( IsWRamEnabled() );

				if (IsWRamEnabled())
					wrk[0][address - 0x6000U] = data;
			}

			NES_PEEK(Mmc1,wRam_SoRom_1a)
			{
				if (regs[CTRL] & CTRL_CHR_SWAP_4K)
					address -= (regs[CHR0] & 0x10) ? 0x4000 : 0x6000;
				else
					address -= (regs[CHR0] & 0x08) ? 0x4000 : 0x6000;

				return wrk[0][address];
			}

			NES_PEEK(Mmc1,wRam_SoRom)
			{
				NST_VERIFY( IsWRamEnabled() );

				return IsWRamEnabled() ? NES_CALL_PEEK(Mmc1,wRam_SoRom_1a,address) : address >> 8;
			}

			NES_POKE(Mmc1,wRam_SoRom_1a)
			{
				if (regs[CTRL] & CTRL_CHR_SWAP_4K)
					address -= (regs[CHR0] & 0x10) ? 0x4000 : 0x6000;
				else
					address -= (regs[CHR0] & 0x08) ? 0x4000 : 0x6000;

				wrk[0][address] = data;
			}

			NES_POKE(Mmc1,wRam_SoRom)
			{
				NST_VERIFY( IsWRamEnabled() );

				if (IsWRamEnabled())
					NES_CALL_POKE(Mmc1,wRam_SoRom_1a,address,data);
			}

			void Mmc1::VSync()
			{
				serial.time -= (iword) cpu.GetAutoClockFrameCycles();

				if (serial.time < -2)
					serial.time = -2;
			}
		}
	}
}
