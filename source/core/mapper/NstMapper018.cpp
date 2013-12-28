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
#include "../NstClock.hpp"
#include "../NstSoundPlayer.hpp"
#include "NstMapper018.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Sound::Player* Mapper18::DetectSound(dword crc,Cpu& cpu)
		{
			switch (crc)
			{
				case 0x3C361B36UL:

					return Sound::Player::Create
					(
						cpu,
						Sound::Loader::TERAO_NO_DOSUKOI_OOZUMOU,
						Sound::Loader::TERAO_NO_DOSUKOI_OOZUMOU_SAMPLES
					);

				case 0xC2222BB1UL:

					return Sound::Player::Create
					(
						cpu,
						Sound::Loader::MOE_PRO_90_KANDOU_HEN,
						Sound::Loader::MOE_PRO_90_KANDOU_HEN_SAMPLES
					);

				case 0x035CC54BUL:

					return Sound::Player::Create
					(
						cpu,
						Sound::Loader::MOE_PRO_SAIKYOU_HEN,
						Sound::Loader::MOE_PRO_SAIKYOU_HEN_SAMPLES
					);

				case 0x142F7F3FUL:

					return Sound::Player::Create
					(
						cpu,
						Sound::Loader::SHIN_MOERO_PRO_YAKYUU,
						Sound::Loader::SHIN_MOERO_PRO_YAKYUU_SAMPLES
					);
			}

			return NULL;
		}

		void Mapper18::Irq::Reset(const bool hard)
		{
			if (hard)
			{
				mask = 0xFFFFU;
				count = 0;
				latch = 0;
			}
		}

		Mapper18::Mapper18(Context& c)
		:
		Mapper (c),
		irq    (c.cpu),
		sound  (DetectSound(c.prgCrc,c.cpu))
		{}

		Mapper18::~Mapper18()
		{
			delete sound;
		}

		void Mapper18::SubReset(const bool hard)
		{
			reg = 0;
			irq.Reset( hard, hard ? false : irq.IsLineEnabled() );

			for (uint i=0x0000U; i < 0x1000U; i += 0x4)
			{
				Map( 0x8000U + i, &Mapper18::Poke_8000 );
				Map( 0x8001U + i, &Mapper18::Poke_8001 );
				Map( 0x8002U + i, &Mapper18::Poke_8002 );
				Map( 0x8003U + i, &Mapper18::Poke_8003 );
				Map( 0x9000U + i, &Mapper18::Poke_9000 );
				Map( 0x9001U + i, &Mapper18::Poke_9001 );
				Map( 0xA000U + i, &Mapper18::Poke_A000 );
				Map( 0xA001U + i, &Mapper18::Poke_A001 );
				Map( 0xA002U + i, &Mapper18::Poke_A002 );
				Map( 0xA003U + i, &Mapper18::Poke_A003 );
				Map( 0xB000U + i, &Mapper18::Poke_B000 );
				Map( 0xB001U + i, &Mapper18::Poke_B001 );
				Map( 0xB002U + i, &Mapper18::Poke_B002 );
				Map( 0xB003U + i, &Mapper18::Poke_B003 );
				Map( 0xC000U + i, &Mapper18::Poke_C000 );
				Map( 0xC001U + i, &Mapper18::Poke_C001 );
				Map( 0xC002U + i, &Mapper18::Poke_C002 );
				Map( 0xC003U + i, &Mapper18::Poke_C003 );
				Map( 0xD000U + i, &Mapper18::Poke_D000 );
				Map( 0xD001U + i, &Mapper18::Poke_D001 );
				Map( 0xD002U + i, &Mapper18::Poke_D002 );
				Map( 0xD003U + i, &Mapper18::Poke_D003 );
				Map( 0xE000U + i, &Mapper18::Poke_E000 );
				Map( 0xE001U + i, &Mapper18::Poke_E001 );
				Map( 0xE002U + i, &Mapper18::Poke_E002 );
				Map( 0xE003U + i, &Mapper18::Poke_E003 );
				Map( 0xF000U + i, &Mapper18::Poke_F000 );
				Map( 0xF001U + i, &Mapper18::Poke_F001 );
				Map( 0xF002U + i, &Mapper18::Poke_F002 );

				if (sound)
					Map( 0xF003U + i, &Mapper18::Poke_F003 );
			}
		}

		void Mapper18::SubLoad(State::Loader& state)
		{
			if (sound)
				sound->Stop();

			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
				{
					const State::Loader::Data<5> data( state );

					irq.EnableLine( data[0] & 0x1 );

					if      (data[0] & 0x8) irq.unit.mask = 0x000FU;
					else if (data[0] & 0x4) irq.unit.mask = 0x00FFU;
					else if (data[0] & 0x2) irq.unit.mask = 0x0FFFU;
					else                    irq.unit.mask = 0xFFFFU;

					irq.unit.latch = data[1] | (data[2] << 8);
					irq.unit.count = data[3] | (data[4] << 8);
				}
				else if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					NST_VERIFY( sound );
					reg = state.Read8();
				}

				state.End();
			}
		}

		void Mapper18::SubSave(State::Saver& state) const
		{
			const u8 data[5] =
			{
				(irq.IsLineEnabled() ? 0x1 : 0x0) |
				(
					irq.unit.mask == 0x000FU ? 0x8 :
					irq.unit.mask == 0x00FFU ? 0x4 :
					irq.unit.mask == 0x0FFFU ? 0x2 :
                                               0x0
				),
				irq.unit.latch & 0xFF,
				irq.unit.latch >> 8,
				irq.unit.count & 0xFF,
				irq.unit.count >> 8
			};

			state.Begin('I','R','Q','\0').Write( data ).End();

			if (sound)
				state.Begin('R','E','G','\0').Write8( reg ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		template<uint MASK,uint SHIFT>
		void Mapper18::SwapPrg(const uint address,const uint data)
		{
			prg.SwapBank<SIZE_8K>( address, (prg.GetBank<SIZE_8K>(address) & MASK) | ((data & 0xF) << SHIFT) );
		}

		NES_POKE(Mapper18,8000) { SwapPrg<0xF0,0>( 0x0000U, data ); }
		NES_POKE(Mapper18,8001) { SwapPrg<0x0F,4>( 0x0000U, data ); }
		NES_POKE(Mapper18,8002) { SwapPrg<0xF0,0>( 0x2000U, data ); }
		NES_POKE(Mapper18,8003) { SwapPrg<0x0F,4>( 0x2000U, data ); }
		NES_POKE(Mapper18,9000) { SwapPrg<0xF0,0>( 0x4000U, data ); }
		NES_POKE(Mapper18,9001) { SwapPrg<0x0F,4>( 0x4000U, data ); }

		template<uint MASK,uint SHIFT>
		void Mapper18::SwapChr(const uint address,const uint data) const
		{
			ppu.Update();
			chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & MASK) | ((data & 0xF) << SHIFT) );
		}

		NES_POKE(Mapper18,A000) { SwapChr<0xF0,0>( 0x0000U, data ); }
		NES_POKE(Mapper18,A001) { SwapChr<0x0F,4>( 0x0000U, data ); }
		NES_POKE(Mapper18,A002) { SwapChr<0xF0,0>( 0x0400U, data ); }
		NES_POKE(Mapper18,A003) { SwapChr<0x0F,4>( 0x0400U, data ); }
		NES_POKE(Mapper18,B000) { SwapChr<0xF0,0>( 0x0800U, data ); }
		NES_POKE(Mapper18,B001) { SwapChr<0x0F,4>( 0x0800U, data ); }
		NES_POKE(Mapper18,B002) { SwapChr<0xF0,0>( 0x0C00U, data ); }
		NES_POKE(Mapper18,B003) { SwapChr<0x0F,4>( 0x0C00U, data ); }
		NES_POKE(Mapper18,C000) { SwapChr<0xF0,0>( 0x1000U, data ); }
		NES_POKE(Mapper18,C001) { SwapChr<0x0F,4>( 0x1000U, data ); }
		NES_POKE(Mapper18,C002) { SwapChr<0xF0,0>( 0x1400U, data ); }
		NES_POKE(Mapper18,C003) { SwapChr<0x0F,4>( 0x1400U, data ); }
		NES_POKE(Mapper18,D000) { SwapChr<0xF0,0>( 0x1800U, data ); }
		NES_POKE(Mapper18,D001) { SwapChr<0x0F,4>( 0x1800U, data ); }
		NES_POKE(Mapper18,D002) { SwapChr<0xF0,0>( 0x1C00U, data ); }
		NES_POKE(Mapper18,D003) { SwapChr<0x0F,4>( 0x1C00U, data ); }

		NES_POKE(Mapper18,E000)
		{
			irq.Update();
			irq.unit.latch = (irq.unit.latch & 0xFFF0U) | (data & 0xF);
		}

		NES_POKE(Mapper18,E001)
		{
			irq.Update();
			irq.unit.latch = (irq.unit.latch & 0xFF0FU) | ((data & 0xF) << 4);
		}

		NES_POKE(Mapper18,E002)
		{
			irq.Update();
			irq.unit.latch = (irq.unit.latch & 0xF0FFU) | ((data & 0xF) << 8);
		}

		NES_POKE(Mapper18,E003)
		{
			irq.Update();
			irq.unit.latch = (irq.unit.latch & 0x0FFFU) | ((data & 0xF) << 12);
		}

		NES_POKE(Mapper18,F000)
		{
			irq.Update();
			irq.unit.count = irq.unit.latch;
			irq.ClearIRQ();
		}

		NES_POKE(Mapper18,F001)
		{
			irq.Update();

			if      (data & 0x8) irq.unit.mask = 0x000FU;
			else if (data & 0x4) irq.unit.mask = 0x00FFU;
			else if (data & 0x2) irq.unit.mask = 0x0FFFU;
			else                 irq.unit.mask = 0xFFFFU;

			irq.EnableLine( data & 0x1 );
			irq.ClearIRQ();
		}

		NES_POKE(Mapper18,F002)
		{
			ppu.SetMirroring
			(
				(data & 0x3) == 0 ? Ppu::NMT_HORIZONTAL :
				(data & 0x3) == 1 ? Ppu::NMT_VERTICAL :
									Ppu::NMT_ZERO
			);
		}

		NES_POKE(Mapper18,F003)
		{
			NST_ASSERT( sound );

			uint tmp = reg;
			reg = data;

			if ((data & 0x2) < (tmp & 0x2) && (data & 0x1D) == (tmp & 0x1D))
				sound->Play( data >> 2 & 0x1F );
		}

		ibool Mapper18::Irq::Signal()
		{
			return (count & mask) && !(--count & mask);
		}

		void Mapper18::VSync()
		{
			irq.VSync();
		}
	}
}
