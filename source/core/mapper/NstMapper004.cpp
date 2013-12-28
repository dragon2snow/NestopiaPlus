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
#include "NstMapper004.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper4::Board Mapper4::GetBoard(uint attribute)
		{
			switch (attribute & ATR_BOARD)
			{
				case ATR_TEROM:  return BRD_TEROM;
				case ATR_TFROM:  return BRD_TFROM;
				case ATR_TGROM:  return BRD_TGROM;
				case ATR_TKROM:  return BRD_TKROM;
				case ATR_TLROM:  return BRD_TLROM;
				case ATR_TL1ROM: return BRD_TL1ROM;
				case ATR_TLSROM: return BRD_TLSROM;
				case ATR_TR1ROM: return BRD_TR1ROM;
				case ATR_TSROM:  return BRD_TSROM;
				case ATR_TVROM:  return BRD_TVROM;
			}

			return BRD_GENERIC;
		}

		Mapper4::Chip Mapper4::GetChip(const Context& c)
		{
			switch (c.attribute & ATR_CHIP)
			{
				case ATR_MMC3A:  return CHIP_MMC3A;
				case ATR_MMC6:   return CHIP_MMC6;
				case ATR_MIMIC1: return CHIP_MIMIC;
			}

			if (c.wrk.Size() == SIZE_1K && (c.attribute & ATR_BOARD) == 0)
				return CHIP_MMC6;

			return CHIP_MMC3B;
		}

		Mapper4::Mapper4(Context& c)
		: Mmc3
		(
			c,
			GetBoard(c.attribute),
			GetChip(c) == CHIP_MMC6  ? PROM_MAX_512K|CROM_MAX_256K|WRAM_1K :
			GetChip(c) == CHIP_MIMIC ? PROM_MAX_512K|CROM_MAX_256K|WRAM_NONE :
                                       PROM_MAX_512K|CROM_MAX_256K|WRAM_AUTO,
			GetChip(c) == CHIP_MMC3A ? REV_A :
                                       REV_B
		),
		chip (GetChip(c))
		{}

		void Mapper4::SubReset(const bool hard)
		{
			Mmc3::SubReset( hard );

			if (chip == CHIP_MMC6)
			{
				mmc6.wRam = 0;

				Map( 0x7000U, 0x7FFFU, &Mapper4::Peek_Mmc6_wRam, &Mapper4::Poke_Mmc6_wRam );

				for (uint i=0xA001U; i < 0xC000U; i += 0x2)
					Map( i, &Mapper4::Poke_Mmc6_A001 );
			}
			else if (chip == CHIP_MIMIC)
			{
				Map( 0xA000U, 0xFFFFU, NOP_POKE );
			}
		}

		void Mapper4::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					NST_VERIFY( chip == CHIP_MMC6 );

					if (chip == CHIP_MMC6)
						mmc6.wRam = state.Read8();
				}

				state.End();
			}
		}

		void Mapper4::SubSave(State::Saver& state) const
		{
			if (chip == CHIP_MMC6)
				state.Begin('R','E','G','\0').Write8( mmc6.wRam ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		inline ibool Mapper4::Mmc6::IsWRamEnabled() const
		{
			return wRam & (WRAM_LO_BANK_ENABLED|WRAM_HI_BANK_ENABLED);
		}

		inline ibool Mapper4::Mmc6::IsWRamReadable(uint address) const
		{
			return (wRam >> (address >> 8 & 0x2)) & 0x20;
		}

		inline ibool Mapper4::Mmc6::IsWRamWritable(uint address) const
		{
			return ((wRam >> (address >> 8 & 0x2)) & 0x30) == 0x30;
		}

		NES_POKE(Mapper4,Mmc6_wRam)
		{
			NST_VERIFY( mmc6.IsWRamWritable( address ) );

			if (mmc6.IsWRamWritable( address ))
				wrk[0][address & 0x3FFU] = data;
		}

		NES_PEEK(Mapper4,Mmc6_wRam)
		{
			NST_VERIFY( mmc6.IsWRamEnabled() );

			if (mmc6.IsWRamEnabled())
				return mmc6.IsWRamReadable( address ) ? wrk[0][address & 0x3FFU] : 0x00;
			else
				return address >> 8;
		}

		NES_POKE(Mapper4,Mmc6_A001)
		{
			if ((mmc6.wRam & 0x1) | (regs.ctrl0 & Mmc6::WRAM_ENABLE))
				mmc6.wRam = data | 0x1;
		}
	}
}
