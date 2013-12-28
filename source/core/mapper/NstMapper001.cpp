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

		Mapper1::Board Mapper1::GetBoard(const Context& c)
		{
			if (c.attribute & ATR_BOARD)
			{
				switch (c.attribute & ATR_BOARD)
				{
					case ATR_SAROM:    return BRD_SAROM;
					case ATR_SBROM:    return BRD_SBROM;
					case ATR_SCROM:    return BRD_SCROM;
					case ATR_SC1ROM:   return BRD_SC1ROM;
					case ATR_SEROM:    return BRD_SEROM;
					case ATR_SFROM:    return BRD_SFROM;
					case ATR_SF1ROM:   return BRD_SF1ROM;
					case ATR_SFEOROM:  return BRD_SFEOROM;
					case ATR_SFEXPROM: return BRD_SFEXPROM;
					case ATR_SGROM:    return BRD_SGROM;
					case ATR_SHROM:    return BRD_SHROM;
					case ATR_SH1ROM:   return BRD_SH1ROM;
					case ATR_SJROM:    return BRD_SJROM;
					case ATR_SKROM:    return BRD_SKROM;
					case ATR_SLROM:    return BRD_SLROM;
					case ATR_SL1ROM:   return BRD_SL1ROM;
					case ATR_SL3ROM:   return BRD_SL3ROM;
					case ATR_SLRROM:   return BRD_SLRROM;
					case ATR_SNROM:    return BRD_SNROM;
					case ATR_SOROM:    return BRD_SOROM;
					case ATR_SUROM:    return BRD_SUROM;
					case ATR_SXROM:    return BRD_SXROM;
				}
			}

			if (c.wrk.Size() >= SIZE_32K)
				return BRD_SXROM;

			if (c.wrk.Size() >= SIZE_16K)
				return BRD_SOROM;

			return BRD_GENERIC;
		}

		Mapper1::Mapper1(Context& c)
		: Mmc1(c,GetBoard(c),(c.attribute & ATR_MMC1A) ? REV_A : REV_B)
		{
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
