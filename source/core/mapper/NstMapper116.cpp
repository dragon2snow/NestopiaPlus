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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper116.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper116::Mapper116(Context& c)
		:
		Mapper (c),
		irq    (c.cpu,c.ppu)
		{}

		void Mapper116::SubReset(const bool hard)
		{
			irq.Reset( hard, hard || irq.IsLineEnabled() );

			if (hard)
			{
				mode = 0;

				vrc2.prg[0] = 0x0;
				vrc2.prg[1] = 0x1;
				vrc2.nmt = 0;

				for (uint i=0; i < 8; ++i)
					vrc2.chr[i] = i;

				mmc3.ctrl = 0;
				mmc3.nmt = 0;

				mmc3.banks[0] = 0x0;
				mmc3.banks[1] = 0x1;
				mmc3.banks[2] = 0x4;
				mmc3.banks[3] = 0x5;
				mmc3.banks[4] = 0x6;
				mmc3.banks[5] = 0x7;

				mmc3.banks[6] = 0x3C;
				mmc3.banks[7] = 0x3D;
				mmc3.banks[8] = 0xFE;
				mmc3.banks[9] = 0xFF;

				mmc1.buffer = 0;
				mmc1.shifter = 0;

				mmc1.regs[0] = 0x4|0x8;
				mmc1.regs[1] = 0;
				mmc1.regs[2] = 0;
				mmc1.regs[3] = 0;
			}

			for (uint i=0x4100U; i < 0x6000U; i += 0x200)
				Map( i + 0x00, i + 0xFF, &Mapper116::Poke_4100 );

			Map( 0x8000U, 0x8FFFU, &Mapper116::Poke_8000 );
			Map( 0x9000U, 0x9FFFU, &Mapper116::Poke_9000 );
			Map( 0xA000U, 0xAFFFU, &Mapper116::Poke_A000 );
			Map( 0xB000U, 0xBFFFU, &Mapper116::Poke_B000 );
			Map( 0xC000U, 0xCFFFU, &Mapper116::Poke_C000 );
			Map( 0xD000U, 0xDFFFU, &Mapper116::Poke_D000 );
			Map( 0xE000U, 0xEFFFU, &Mapper116::Poke_E000 );
			Map( 0xF000U, 0xFFFFU, &Mapper116::Poke_F000 );

			UpdatePrg();
			UpdateNmt();
			UpdateChr();
		}

		void Mapper116::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('R','E','G','\0'):

						mode = state.Read8();
						break;

					case NES_STATE_CHUNK_ID('V','R','2','\0'):

						state.Read( blockVrc2 );
						break;

					case NES_STATE_CHUNK_ID('M','M','3','\0'):

						state.Read( blockMmc3 );
						break;

					case NES_STATE_CHUNK_ID('M','M','1','\0'):

						state.Read( blockMmc1 );
						break;

					case NES_STATE_CHUNK_ID('I','R','Q','\0'):

						irq.unit.LoadState( state );
						break;
				}

				state.End();
			}

			UpdatePrg();
			UpdateNmt();
			UpdateChr();
		}

		void Mapper116::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( mode ).End();
			state.Begin('V','R','2','\0').Write( blockVrc2 ).End();
			state.Begin('M','M','3','\0').Write( blockMmc3 ).End();
			state.Begin('M','M','1','\0').Write( blockMmc1 ).End();
			irq.unit.SaveState( State::Saver::Subset(state,'I','R','Q','\0').Ref() );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper116::UpdatePrg()
		{
			switch (mode & 0x3)
			{
				case 0x0:

					prg.SwapBanks<SIZE_8K,0x0000U>( vrc2.prg[0], vrc2.prg[1], 0x1E, 0x1F );
					break;

				case 0x1:
				{
					const uint i = (mmc3.ctrl & 0x40) >> 5;
					prg.SwapBanks<SIZE_8K,0x0000U>( mmc3.banks[6+i], mmc3.banks[6+1], mmc3.banks[6+(i^2)], mmc3.banks[6+3] );
					break;
				}

				case 0x2:
				{
					const uint bank = mmc1.regs[3] & 0xF;

					if (mmc1.regs[0] & 0x8)
						prg.SwapBanks<SIZE_16K,0x0000U>( ((mmc1.regs[0] & 0x4) ? bank : 0x0), ((mmc1.regs[0] & 0x4) ? 0xF : bank) );
					else
						prg.SwapBank<SIZE_32K,0x0000U>( bank >> 1 );

					break;
				}
			}
		}

		void Mapper116::UpdateChr() const
		{
			const uint base = (mode & 0x4) << 6;

			switch (mode & 0x3)
			{
				case 0x0:

					chr.SwapBanks<SIZE_1K,0x0000U>( base|vrc2.chr[0], base|vrc2.chr[1], base|vrc2.chr[2], base|vrc2.chr[3], base|vrc2.chr[4], base|vrc2.chr[5], base|vrc2.chr[6], base|vrc2.chr[7] );
					break;

				case 0x1:
				{
					const uint swap = (mmc3.ctrl & 0x80) << 5;
					chr.SwapBanks<SIZE_2K>( 0x0000U ^ swap, (base >> 1)|mmc3.banks[0], (base >> 1)|mmc3.banks[1] );
					chr.SwapBanks<SIZE_1K>( 0x1000U ^ swap, base|mmc3.banks[2], base|mmc3.banks[3], base|mmc3.banks[4], base|mmc3.banks[5] );
					break;
				}

				case 0x2:

					chr.SwapBanks<SIZE_4K,0x0000U>( (mmc1.regs[0] & 0x10) ? mmc1.regs[1] : mmc1.regs[1] & 0x1E, (mmc1.regs[0] & 0x10) ? mmc1.regs[2] : mmc1.regs[1] | 0x01 );
					break;
			}
		}

		void Mapper116::UpdateNmt() const
		{
			uint nmt;

			switch (mode & 0x3)
			{
				case 0x0:

					nmt = (vrc2.nmt & 0x1) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL;
					break;

				case 0x1:

					nmt = (mmc3.nmt & 0x1) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL;
					break;

				case 0x2:

					switch (mmc1.regs[0] & 0x3)
					{
						case 0x0: nmt = Ppu::NMT_ZERO;       break;
						case 0x1: nmt = Ppu::NMT_ONE;        break;
						case 0x2: nmt = Ppu::NMT_VERTICAL;   break;
						default:  nmt = Ppu::NMT_HORIZONTAL; break;
					}
					break;

				default: return;
			}

			ppu.SetMirroring( nmt );
		}

		void Mapper116::Poke_Vrc2_8000(uint address,uint data)
		{
			NST_ASSERT( (mode & 0x3) == 0 );

			data &= 0x1F;
			address = address >> 13 & 0x1;

			if (vrc2.prg[address] != data)
			{
				vrc2.prg[address] = data;
				UpdatePrg();
			}
		}

		void Mapper116::Poke_Vrc2_9000(uint address,uint data)
		{
			NST_ASSERT( (mode & 0x3) == 0 );

			data &= 0x1;

			if (vrc2.nmt != data)
			{
				vrc2.nmt = data;
				UpdateNmt();
			}
		}

		void Mapper116::Poke_Vrc2_B000(uint address,uint data)
		{
			NST_ASSERT( (mode & 0x3) == 0 );

			data = (data & 0xF) << (address << 1 & 0x4);
			address = ((address - 0xB000U) >> 11 & 0x6) | (address & 0x1);

			if (vrc2.chr[address] != data)
			{
				vrc2.chr[address] = data;
				ppu.Update();
				UpdateChr();
			}
		}

		void Mapper116::Poke_Mmc3_8000(uint address,uint data)
		{
			NST_ASSERT( (mode & 0x3) == 1 );

			if (address & 0x1)
			{
				address = mmc3.ctrl & 0x7;

				if (address < 2)
					data >>= 1;

				if (mmc3.banks[address] != data)
				{
					mmc3.banks[address] = data;

					if (address < 6)
					{
						ppu.Update();
						UpdateChr();
					}
					else
					{
						UpdatePrg();
					}
				}
			}
			else
			{
				address = mmc3.ctrl ^ data;
				mmc3.ctrl = data;

				if (address & 0x40)
					UpdatePrg();

				if (address & (0x80|0x7))
				{
					ppu.Update();
					UpdateChr();
				}
			}
		}

		void Mapper116::Poke_Mmc3_A000(uint address,uint data)
		{
			NST_ASSERT( (mode & 0x3) == 1 );

			if (!(address & 0x1))
			{
				if (mmc3.nmt != data)
				{
					mmc3.nmt = data;
					UpdateNmt();
				}
			}
		}

		void Mapper116::Poke_Mmc3_C000(uint address,uint data)
		{
			NST_ASSERT( (mode & 0x3) == 1 );

			irq.Update();

			if (address & 0x1)
				irq.unit.Reload();
			else
				irq.unit.SetLatch( data );
		}

		void Mapper116::Poke_Mmc3_E000(uint address,uint data)
		{
			NST_ASSERT( (mode & 0x3) == 1 );

			irq.Update();

			if (address & 0x1)
				irq.unit.Enable();
			else
				irq.unit.Disable( cpu );
		}

		void Mapper116::Poke_Mmc1_8000(uint address,uint data)
		{
			NST_ASSERT( (mode & 0x3) == 2 );

			if (!(data & 0x80))
			{
				mmc1.buffer |= (data & 0x1) << mmc1.shifter++;

				if (mmc1.shifter != 5)
					return;

				mmc1.shifter = 0;
				data = mmc1.buffer;
				mmc1.buffer = 0;

				address = address >> 13 & 0x3;

				if (mmc1.regs[address] != data)
				{
					mmc1.regs[address] = data;

					UpdatePrg();
					UpdateNmt();
					UpdateChr();
				}
			}
			else
			{
				mmc1.buffer = 0;
				mmc1.shifter = 0;

				if ((mmc1.regs[0] & (0x4|0x8)) != (0x4|0x8))
				{
					mmc1.regs[0] |= (0x4|0x8);

					UpdatePrg();
					UpdateNmt();
					UpdateChr();
				}
			}
		}

		NES_POKE(Mapper116,4100)
		{
			if (mode != data)
			{
				mode = data;

				if ((data & 0x3) != 1)
					irq.unit.Disable( cpu );

				UpdatePrg();
				UpdateNmt();
				UpdateChr();
			}
		}

		NES_POKE(Mapper116,8000)
		{
			switch (mode & 0x3)
			{
				case 0x0: Poke_Vrc2_8000( address, data ); break;
				case 0x1: Poke_Mmc3_8000( address, data ); break;
				case 0x2: Poke_Mmc1_8000( address, data ); break;
			}
		}

		NES_POKE(Mapper116,9000)
		{
			switch (mode & 0x3)
			{
				case 0x0: Poke_Vrc2_9000( address, data ); break;
				case 0x1: Poke_Mmc3_8000( address, data ); break;
				case 0x2: Poke_Mmc1_8000( address, data ); break;
			}
		}

		NES_POKE(Mapper116,A000)
		{
			switch (mode & 0x3)
			{
				case 0x0: Poke_Vrc2_8000( address, data ); break;
				case 0x1: Poke_Mmc3_A000( address, data ); break;
				case 0x2: Poke_Mmc1_8000( address, data ); break;
			}
		}

		NES_POKE(Mapper116,B000)
		{
			switch (mode & 0x3)
			{
				case 0x0: Poke_Vrc2_B000( address, data ); break;
				case 0x1: Poke_Mmc3_A000( address, data ); break;
				case 0x2: Poke_Mmc1_8000( address, data ); break;
			}
		}

		NES_POKE(Mapper116,C000)
		{
			switch (mode & 0x3)
			{
				case 0x0: Poke_Vrc2_B000( address, data ); break;
				case 0x1: Poke_Mmc3_C000( address, data ); break;
				case 0x2: Poke_Mmc1_8000( address, data ); break;
			}
		}

		NES_POKE(Mapper116,D000)
		{
			switch (mode & 0x3)
			{
				case 0x0: Poke_Vrc2_B000( address, data ); break;
				case 0x1: Poke_Mmc3_C000( address, data ); break;
				case 0x2: Poke_Mmc1_8000( address, data ); break;
			}
		}

		NES_POKE(Mapper116,E000)
		{
			switch (mode & 0x3)
			{
				case 0x0: Poke_Vrc2_B000( address, data ); break;
				case 0x1: Poke_Mmc3_E000( address, data ); break;
				case 0x2: Poke_Mmc1_8000( address, data ); break;
			}
		}

		NES_POKE(Mapper116,F000)
		{
			switch (mode & 0x3)
			{
				case 0x0: break;
				case 0x1: Poke_Mmc3_E000( address, data ); break;
				case 0x2: Poke_Mmc1_8000( address, data ); break;
			}
		}

		void Mapper116::VSync()
		{
			irq.VSync();
		}
	}
}
