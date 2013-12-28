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
#include "NstMapper068.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Mapper68::DoubleCassette
		{
			enum {SIGNAL = 1784};

			uint prgBank;
			uint counter;

		public:

			void Reset()
			{
				counter = SIGNAL;
				prgBank = 0;
			}

			void SaveState(State::Saver& state,const dword chunk)
			{
				state.Begin( chunk ).Write8( prgBank ).Write16( counter ).End();
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
				return prgBank;
			}

			uint Begin()
			{
				counter = 0;
				return prgBank;
			}

			uint End()
			{
				return (prgBank & 0x8 && counter < SIGNAL && ++counter == SIGNAL) ? (prgBank & 0x7 | 0x10) : 0;
			}
		};

		Mapper68::Mapper68(Context& c)
		:
		Mapper         (c,CROM_MAX_512K|NMT_VERTICAL),
		doubleCassette (c.attribute == ATR_DOUBLECASSETTE ? new DoubleCassette : NULL)
		{
		}

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
				doubleCassette->Reset();
				prg.SwapBanks<SIZE_16K,0x0000>( 0x0, 0x7 );

				Map( 0x6000U,          &Mapper68::Poke_6000 );
				Map( 0x8000U, 0xBFFFU, &Mapper68::Peek_8000 );
				Map( 0xF000U, 0xFFFFU, &Mapper68::Poke_F000 );
			}
			else
			{
				Map( 0xF000U, 0xFFFFU, PRG_SWAP_16K_0 );
			}
		}

		void Mapper68::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						State::Loader::Data<3> data( state );

						regs.ctrl = data[0];
						regs.nmt[0] = data[1] | Regs::BANK_OFFSET;
						regs.nmt[1] = data[2] | Regs::BANK_OFFSET;

						break;
					}

					case AsciiId<'D','B','C'>::V:

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
			const byte data[3] =
			{
				regs.ctrl,
				regs.nmt[0] & ~uint(Regs::BANK_OFFSET),
				regs.nmt[1] & ~uint(Regs::BANK_OFFSET)
			};

			state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();

			if (doubleCassette)
				doubleCassette->SaveState( state, AsciiId<'D','B','C'>::V );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper68::UpdateMirroring() const
		{
			ppu.Update();

			static const byte select[4][4] =
			{
				{0,1,0,1},
				{0,0,1,1},
				{0,0,0,0},
				{1,1,1,1}
			};

			const uint isCrom = (regs.ctrl & Regs::CTRL_CROM) >> 4;
			const byte (&index)[4] = select[regs.ctrl & Regs::CTRL_MIRRORING];

			for (uint i=0; i < 4; ++i)
				nmt.Source( isCrom ).SwapBank<SIZE_1K>( i * SIZE_1K, isCrom ? regs.nmt[index[i]] : index[i] );
		}

		NES_POKE_D(Mapper68,6000)
		{
			if (data == 0x00)
				prg.SwapBank<SIZE_16K,0x0000>( doubleCassette->Begin() );
		}

		NES_PEEK_A(Mapper68,8000)
		{
			if (const uint bank = doubleCassette->End())
				prg.SwapBank<SIZE_16K,0x0000>( bank & 0xF );

			return prg.Peek( address - 0x8000 );
		}

		NES_POKE_D(Mapper68,C000)
		{
			regs.nmt[0] = Regs::BANK_OFFSET | data;
			UpdateMirroring();
		}

		NES_POKE_D(Mapper68,D000)
		{
			regs.nmt[1] = Regs::BANK_OFFSET | data;
			UpdateMirroring();
		}

		NES_POKE_D(Mapper68,E000)
		{
			regs.ctrl = data;
			UpdateMirroring();
		}

		NES_POKE_D(Mapper68,F000)
		{
			prg.SwapBank<SIZE_16K,0x0000>( doubleCassette->Swap(data) );
		}
	}
}
