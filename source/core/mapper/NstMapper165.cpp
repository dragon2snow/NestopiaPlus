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
#include "../board/NstBrdMmc2.hpp"
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper165.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper165::Mapper165(Context& c)
		: Mmc3(c,BRD_GENERIC,PROM_MAX_512K|CROM_MAX_256K|CRAM_4K) {}

		void Mapper165::SubReset(bool)
		{
			selector[1] = 0, selector[0] = 0;

			chr.SetAccessor( 0, this, &Mapper165::Access_Chr_0000 );
			chr.SetAccessor( 1, this, &Mapper165::Access_Chr_1000 );

			Mmc3::SubReset( true );
		}

		void Mapper165::SubSave(State::Saver& state) const
		{
			state.Begin('L','T','C','\0').Write8( selector[0] | selector[1] ).End();
		}

		void Mapper165::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('L','T','C','\0'))
				{
					const uint data = state.Read8();
					selector[0] = data << 0 & 0x1;
					selector[1] = data << 1 & 0x6;
				}

				state.End();
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper165::SwapChrLo() const
		{
			chr.Source( banks.chr[selector[0]] == 0 ).SwapBank<SIZE_4K,0x0000U>( banks.chr[selector[0]] >> 1 );
		}

		void Mapper165::SwapChrHi() const
		{
			chr.Source( banks.chr[selector[1]] == 0 ).SwapBank<SIZE_4K,0x1000U>( banks.chr[selector[1]] >> 2 );
		}

		void Mapper165::UpdateChr() const
		{
			ppu.Update();
			SwapChrLo();
			SwapChrHi();
		}

		NES_ACCESSOR(Mapper165,Chr_0000)
		{
			const uint data = chr.Peek( address );

			switch (address & 0xFF8)
			{
				case 0xFD0: selector[0] = 0; break;
				case 0xFE8: selector[0] = 1; break;
				default: return data;
			}

			SwapChrLo();

			return data;
		}

		NES_ACCESSOR(Mapper165,Chr_1000)
		{
			const uint data = chr.Peek( address );

			switch (address & 0xFF8)
			{
				case 0xFD0: selector[1] = 2; break;
				case 0xFE8: selector[1] = 4; break;
				default: return data;
			}

			SwapChrHi();

			return data;
		}
	}
}
