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

#ifndef NST_MAPPER_4_H
#define NST_MAPPER_4_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper4 : public Boards::Mmc3
		{
		public:

			Mapper4(Context&);

		private:

			enum Chip
			{
				CHIP_MMC3A,
				CHIP_MMC3B,
				CHIP_MMC6,
				CHIP_MIMIC
			};

			enum
			{
				ATR_TEROM = 1,
				ATR_TFROM,
				ATR_TGROM,
				ATR_TKROM,
				ATR_TLROM,
				ATR_TL1ROM,
				ATR_TLSROM,
				ATR_TR1ROM,
				ATR_TSROM,
				ATR_TVROM,
				ATR_BOARD  = 0x1F,
				ATR_MMC3A  = 1U << 5,
				ATR_MMC6   = 2U << 5,
				ATR_MIMIC1 = 3U << 5,
				ATR_CHIP   = 0xE0
			};

			static Board GetBoard(uint);
			static Chip  GetChip(const Context&);

			void SubReset(bool);
			void SubSave(State::Saver&) const;
			void SubLoad(State::Loader&);

			NES_DECL_POKE( Mmc6_8000 )
			NES_DECL_POKE( Mmc6_A001 )
			NES_DECL_POKE( Mmc6_wRam )
			NES_DECL_PEEK( Mmc6_wRam )

			struct Mmc6
			{
				inline ibool IsWRamEnabled() const;
				inline ibool IsWRamReadable(uint) const;
				inline ibool IsWRamWritable(uint) const;

				enum
				{
					WRAM_ENABLE           = b00100000,
					WRAM_LO_BANK_ENABLED  = b00100000,
					WRAM_LO_BANK_WRITABLE = b00010000,
					WRAM_HI_BANK_ENABLED  = b10000000,
					WRAM_HI_BANK_WRITABLE = b01000000
				};

				uint wRam;
			};

			Mmc6 mmc6;
			const Chip chip;
		};
	}
}

#endif
