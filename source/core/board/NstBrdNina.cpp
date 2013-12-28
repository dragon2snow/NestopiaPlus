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
#include "NstBrdNina.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Nina::Nina(Context& c,Type t)
			:
			Mapper (c,PROM_MAX_256K|CROM_MAX_128K|WRAM_DEFAULT),
			type   (t)
			{}

			void Nina::SubReset(bool)
			{
				for (uint i=0x4100; i < 0x6000; i += 0x200)
					Map( i+0x00, i+0xFF, type == TYPE_003_006_STD ? &Nina::Poke_4100 : &Nina::Poke_4100_Nmt );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Nina::Update(const uint data)
			{
				prg.SwapBank<SIZE_32K,0x0000>( data >> 3 & 0x7 );
				chr.SwapBank<SIZE_8K,0x0000>( (data >> 3 & 0x8) | (data & 0x7) );
			}

			NES_POKE(Nina,4100)
			{
				ppu.Update();
				Update( data );
			}

			NES_POKE(Nina,4100_Nmt)
			{
				ppu.SetMirroring( (data & 0x80) ? Ppu::NMT_VERTICAL : Ppu::NMT_HORIZONTAL );
				Update( data );
			}
		}
	}
}
