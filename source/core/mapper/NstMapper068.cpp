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
#include "NstMapper068.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Mapper68::DoubleCassette
		{
			enum {SIGNAL = 1784};

			u8 prgBank;
			const u8 mainBank;
			u16 counter;

			DoubleCassette(uint b)
			: mainBank(b) {}

		public:

			static DoubleCassette* Create(dword crc)
			{
				switch (crc)
				{
					case 0xEA0753E4UL: // Nantettatte!! Baseball '91 Kaimaku Hen
					case 0xDEB6D3C8UL: // Nantettatte!! Baseball OB Allstar Hen

						return new DoubleCassette( 0x8 );

					case 0x1A23F270UL: // Nantettatte!! Baseball '91 Kaimaku Hen (UNIF)
					case 0x3B9E30F1UL: // Nantettatte!! Baseball OB Allstar Hen (UNIF)

						return new DoubleCassette( 0x0 );
				}

				return NULL;
			}

			uint Reset()
			{
				counter = SIGNAL;
				prgBank = mainBank;
				return mainBank;
			}

			void SaveState(State::Saver& state)
			{
				state.Write8( prgBank ).Write16( counter );
			}

			void LoadState(State::Loader& state)
			{
				prgBank = state.Read8() & 0xF;
				counter = state.Read16();

				if (counter > SIGNAL)
					counter = SIGNAL;
			}

			uint Swap(uint data)
			{
				prgBank = (data & 0x7) | (~data & 0x8);
				return prgBank^mainBank;
			}

			uint Begin()
			{
				counter = 0;
				return prgBank^mainBank;
			}

			uint End()
			{
				return (prgBank & 0x8) && counter < SIGNAL && ++counter == SIGNAL ? (mainBank ? prgBank : prgBank & 0x7) | 0x10 : 0;
			}
		};

		Mapper68::Mapper68(Context& c)
		:
		Mapper         (c),
		doubleCassette (DoubleCassette::Create(c.prgCrc))
		{}

		Mapper68::~Mapper68()
		{
			delete doubleCassette;
		}

		void Mapper68::SubReset(const bool hard)
		{
			if (hard)
			{
				regs.ctrl = 0;
				regs.nmt[0] = Regs::BANK_OFFSET;
				regs.nmt[1] = Regs::BANK_OFFSET;
			}

			Map( 0x8000U, 0x8FFFU, CHR_SWAP_2K_0        );
			Map( 0x9000U, 0x9FFFU, CHR_SWAP_2K_1        );
			Map( 0xA000U, 0xAFFFU, CHR_SWAP_2K_2        );
			Map( 0xB000U, 0xBFFFU, CHR_SWAP_2K_3        );
			Map( 0xC000U, 0xCFFFU, &Mapper68::Poke_C000 );
			Map( 0xD000U, 0xDFFFU, &Mapper68::Poke_D000 );
			Map( 0xE000U, 0xEFFFU, &Mapper68::Poke_E000 );

			if (doubleCassette)
			{
				const uint bank = doubleCassette->Reset();
				prg.SwapBanks<SIZE_16K,0x0000U>( bank + 0x0, bank + 0x7 );

				Map( 0x6000U,          &Mapper68::Poke_6000 );
				Map( 0x8000U, 0xBFFFU, &Mapper68::Peek_8000 );
				Map( 0xF000U, 0xFFFFU, &Mapper68::Poke_F000 );
			}
			else
			{
				Map( 0xF000U, 0xFFFFU, PRG_SWAP_16K );
			}
		}

		void Mapper68::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('R','E','G','\0'):
					{
						const State::Loader::Data<3> data( state );

						regs.ctrl = data[0];
						regs.nmt[0] = data[1] | Regs::BANK_OFFSET;
						regs.nmt[1] = data[2] | Regs::BANK_OFFSET;

						break;
					}

					case NES_STATE_CHUNK_ID('D','B','C','\0'):

						NST_VERIFY( doubleCassette );

						if (doubleCassette)
							doubleCassette->LoadState( state );

						break;
				}

				state.End();
			}
		}

		void Mapper68::SubSave(State::Saver& state) const
		{
			const u8 data[3] =
			{
				regs.ctrl,
				regs.nmt[0] & ~uint(Regs::BANK_OFFSET),
				regs.nmt[1] & ~uint(Regs::BANK_OFFSET)
			};

			state.Begin('R','E','G','\0').Write( data ).End();

			if (doubleCassette)
			{
				state.Begin('D','B','C','\0');
				doubleCassette->SaveState( state );
				state.End();
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper68::UpdateMirroring() const
		{
			ppu.Update();

			static const uchar select[4][4] =
			{
				{0,1,0,1},
				{0,0,1,1},
				{0,0,0,0},
				{1,1,1,1}
			};

			const uint isCrom = (regs.ctrl & Regs::CTRL_CROM) >> 4;
			const uchar (&index)[4] = select[regs.ctrl & Regs::CTRL_MIRRORING];

			for (uint i=0; i < 4; ++i)
				nmt.Source( isCrom ).SwapBank<SIZE_1K>( i * SIZE_1K, isCrom ? regs.nmt[index[i]] : index[i] );
		}

		NES_POKE(Mapper68,6000)
		{
			if (data == 0x00)
				prg.SwapBank<SIZE_16K,0x0000U>( doubleCassette->Begin() );
		}

		NES_PEEK(Mapper68,8000)
		{
			if (const uint bank = doubleCassette->End())
				prg.SwapBank<SIZE_16K,0x0000U>( bank & 0xF );

			return prg.Peek( address - 0x8000U );
		}

		NES_POKE(Mapper68,C000)
		{
			regs.nmt[0] = Regs::BANK_OFFSET | data;
			UpdateMirroring();
		}

		NES_POKE(Mapper68,D000)
		{
			regs.nmt[1] = Regs::BANK_OFFSET | data;
			UpdateMirroring();
		}

		NES_POKE(Mapper68,E000)
		{
			regs.ctrl = data;
			UpdateMirroring();
		}

		NES_POKE(Mapper68,F000)
		{
			prg.SwapBank<SIZE_16K,0x0000U>( doubleCassette->Swap(data) );
		}
	}
}
