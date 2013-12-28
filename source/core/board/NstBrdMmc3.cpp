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
#include "NstBrdMmc3.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Mmc3::Irq::Irq(Cpu& cpu,Ppu& ppu,IrqDelay delay,bool persistant)
			: Clock::A12<BaseIrq>(cpu,ppu,SIGNAL_DURATION,delay,persistant) {}

			uint Mmc3::BoardToWRam(Board board,uint settings)
			{
				switch (board)
				{
					case BRD_GENERIC:

						return settings;

					case BRD_TKROM:
					case BRD_TSROM:

						return (settings & ~uint(WRAM_SETTINGS)) | WRAM_8K;

					default:

						return (settings & ~uint(WRAM_SETTINGS)) | WRAM_NONE;
				}
			}

			Mmc3::Mmc3(Context& c,Board b,uint settings,Revision revision)
			:
			Mapper (c,BoardToWRam(b,settings)),
			irq    (c.cpu,c.ppu,Irq::NO_IRQ_DELAY,revision != REV_A)
			{
			}

			void Mmc3::Regs::Reset()
			{
				ctrl0 = 0;
				ctrl1 = 0;
			}

			void Mmc3::Banks::Reset()
			{
				chr[0] = 0x0;
				chr[1] = 0x1;
				chr[2] = 0x4;
				chr[3] = 0x5;
				chr[4] = 0x6;
				chr[5] = 0x7;

				prg[0] = 0x00;
				prg[1] = 0x01;
				prg[2] = 0xFE;
				prg[3] = 0xFF;
			}

			void Mmc3::BaseIrq::Reset(const bool hard)
			{
				if (hard)
				{
					count = 0;
					latch = 0;
					reload = false;
					enabled = false;
				}
			}

			void Mmc3::SubReset(const bool hard)
			{
				if (hard)
				{
					regs.Reset();
					banks.Reset();
					wrk.Source().SetSecurity( false, false );
				}

				irq.Reset( hard, hard || irq.IsLineEnabled() );

				for (uint i=0x0000; i < 0x2000; i += 0x2)
				{
					Map( 0x8000 + i, &Mmc3::Poke_8000 );
					Map( 0x8001 + i, &Mmc3::Poke_8001 );
					Map( 0xA001 + i, &Mmc3::Poke_A001 );
					Map( 0xC000 + i, &Mmc3::Poke_C000 );
					Map( 0xC001 + i, &Mmc3::Poke_C001 );
					Map( 0xE000 + i, &Mmc3::Poke_E000 );
					Map( 0xE001 + i, &Mmc3::Poke_E001 );
				}

				if (mirroring != Ppu::NMT_FOURSCREEN)
				{
					for (uint i=0xA000; i < 0xC000; i += 0x2)
						Map( i, NMT_SWAP_HV );
				}

				if (wrk.Size() >= SIZE_8K)
					Map( WRK_SAFE_PEEK_POKE );

				UpdatePrg();
				UpdateChr();
			}

			void Mmc3::BaseIrq::LoadState(State::Loader& state)
			{
				State::Loader::Data<3> data( state );

				enabled = data[0] & 0x1;
				reload = data[0] & 0x2;
				count = data[1];
				latch = data[2];
			}

			void Mmc3::BaseIrq::SaveState(State::Saver& state,const dword id) const
			{
				const byte data[3] =
				{
					(enabled ? 0x1U : 0x0U) | (reload ? 0x2U : 0x0U),
					count,
					latch
				};

				state.Begin( id ).Write( data ).End();
			}

			void Mmc3::BaseLoad(State::Loader& state,const dword id)
			{
				NST_VERIFY( id == (AsciiId<'M','M','3'>::V) );

				if (id == AsciiId<'M','M','3'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:
							{
								State::Loader::Data<12> data( state );

								regs.ctrl0 = data[0];
								regs.ctrl1 = data[1];
								banks.prg[0] = data[2];
								banks.prg[1] = data[3];
								banks.prg[2] = data[4];
								banks.prg[3] = data[5];
								banks.chr[0] = data[6];
								banks.chr[1] = data[7];
								banks.chr[2] = data[8];
								banks.chr[3] = data[9];
								banks.chr[4] = data[10];
								banks.chr[5] = data[11];

								break;
							}

							case AsciiId<'I','R','Q'>::V:

								irq.unit.LoadState( state );
								break;
						}

						state.End();
					}
				}
			}

			void Mmc3::BaseSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'M','M','3'>::V );

				{
					const byte data[12] =
					{
						regs.ctrl0,
						regs.ctrl1,
						banks.prg[0],
						banks.prg[1],
						banks.prg[2],
						banks.prg[3],
						banks.chr[0],
						banks.chr[1],
						banks.chr[2],
						banks.chr[3],
						banks.chr[4],
						banks.chr[5]
					};

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
				}

				irq.unit.SaveState( state, AsciiId<'I','R','Q'>::V );

				state.End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE(Mmc3,8000)
			{
				const uint diff = regs.ctrl0 ^ data;
				regs.ctrl0 = data;

				if (diff & Regs::CTRL0_XOR_PRG)
					UpdatePrg();

				if (diff & Regs::CTRL0_XOR_CHR)
					UpdateChr();
			}

			NES_POKE(Mmc3,8001)
			{
				address = regs.ctrl0 & Regs::CTRL0_MODE;

				if (address < 6)
				{
					if (address < 2)
						data >>= 1;

					if (banks.chr[address] != data)
					{
						banks.chr[address] = data;
						UpdateChr();
					}
				}
				else
				{
					if (banks.prg[address-6] != data)
					{
						banks.prg[address-6] = data;
						UpdatePrg();
					}
				}
			}

			NES_POKE(Mmc3,A001)
			{
				regs.ctrl1 = data;

				wrk.Source().SetSecurity
				(
					(data & Regs::CTRL1_WRAM_ENABLED),
					(data & Regs::CTRL1_WRAM) == Regs::CTRL1_WRAM_ENABLED
				);
			}

			NES_POKE(Mmc3,C000)
			{
				irq.Update();
				irq.unit.SetLatch( data );
			}

			NES_POKE(Mmc3,C001)
			{
				irq.Update();
				irq.unit.Reload();
			}

			NES_POKE(Mmc3,E000)
			{
				irq.Update();
				irq.unit.Disable( cpu );
			}

			NES_POKE(Mmc3,E001)
			{
				irq.Update();
				irq.unit.Enable();
			}

			void Mmc3::UpdatePrg()
			{
				const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

				prg.SwapBanks<SIZE_8K,0x0000>( banks.prg[i], banks.prg[1], banks.prg[i^2], banks.prg[3] );
			}

			void Mmc3::UpdateChr() const
			{
				ppu.Update();

				const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

				chr.SwapBanks<SIZE_2K>( 0x0000 ^ swap, banks.chr[0], banks.chr[1] );
				chr.SwapBanks<SIZE_1K>( 0x1000 ^ swap, banks.chr[2], banks.chr[3], banks.chr[4], banks.chr[5] );
			}

			void Mmc3::VSync()
			{
				irq.VSync();
			}
		}
	}
}
