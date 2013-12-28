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
#include "../board/NstBrdMmc1.hpp"
#include "NstMapper001.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		uint Mapper1::GetWrkSize(const Context& c)
		{
			if (c.wrk.Size() > SIZE_8K)
			{
				return WRAM_16K;
			}
			else switch (c.prgCrc)
			{
				case 0xFB69743AUL: // Aoki Ookami to Shiroki Mejika - Genghis Khan (J)
				case 0xB8747ABFUL: // Best Play Pro Yakyuu Special (J)
				case 0x2225C20FUL: // Genghis Khan (U)
				case 0x29449BA9UL: // Nobunaga no Yabou - Zenkoku Han (J)
				case 0x2B11E0B0UL: // Nobunaga no Yabou - Zenkoku Han (J) alt
				case 0x4642DDA6UL: // Nobunaga's Ambition (U)
				case 0xC6182024UL: // Romance of the Three Kingdoms (U)
				case 0xABBF7217UL: // Sangokushi (J)
				case 0x1028FC27UL: // Sangokushi (J) alt

					return WRAM_16K; // SOROM
			}

			return WRAM_8K;
		}

		Boards::Mmc1::Revision Mapper1::GetRevision(dword crc)
		{
			switch (crc)
			{
				case 0x54430B24UL: // Desert Commander (U)
				case 0xD9F0749FUL: // Kid Icarus (U)
				case 0x3FE272FBUL: // Legend of Zelda (U)
				case 0x70080810UL: // Metroid (U)
				case 0x8CE9C87BUL: // Money Game (J)
				case 0x4642DDA6UL: // Nobunaga's Ambition (U)
				case 0x29449BA9UL: // Nobunaga no Yabou - Zenkoku Han (J)
				case 0x2B11E0B0UL: // Nobunaga no Yabou - Zenkoku Han (J) alt
				case 0x15d53e78UL: // Tatakae!! Rahmen Man - Sakuretsu Choujin 102 Gei (J)
				case 0xEE8C9971UL: // Zelda II: Adventures of Link (U)

					return REV_1A;
			}

			return REV_1B;
		}

		Mapper1::Mapper1(Context& c)
		: Mmc1(c,GetWrkSize(c),GetRevision(c.prgCrc))
		{
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
