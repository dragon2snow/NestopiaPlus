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

		bool Mapper4::IsMmc6(dword crc)
		{
			switch (crc)
			{
				case 0xBEB88304UL: // Startropics (U)
				case 0xAC74DD5CUL: // Startropics (E)
				case 0x80FB117EUL: // Startropics 2 - Zoda's Revenge (U)

					return true;
			}

			return false;
		}

		Mapper4::Revision Mapper4::GetRevision(dword crc)
		{
			switch (crc)
			{
				case 0xAF5AD5AFUL: // 8 Eyes (J)
				case 0xCB475567UL: // 8 Eyes (U)
				case 0xDE182C88UL: // Bad Dudes (U)
				case 0x4076E7A6UL: // Batman (E)
				case 0x5A3E41D8UL: // Batman (U)
				case 0x5CEB1256UL: // Indiana Jones & Temple of Doom (U)
				case 0x4CDCAF6BUL: // Legacy of the Wizard (U)
				case 0x15BE6E58UL: // Mappy-Land (J)
				case 0x6D9BA342UL: // Mappy-Land (U)
				case 0x99344ACDUL: // Rampage (U)
				case 0x78784CBFUL: // River City Ransom (U)
				case 0x2706B3A1UL: // Robocop (E)
				case 0xC382120CUL: // Robocop (J)
				case 0x468DB9A0UL: // Robocop (U)
				case 0x066F5A83UL: // Rock'n Ball (U)
				case 0x08AF16A0UL: // Super Mario Bros 2 (E)
				case 0x07854B3FUL: // Super Mario Bros 2 (U)
				case 0xEBB9DF3DUL: // Super Spike V'Ball (E)
				case 0x1E598A14UL: // Super Spike V'Ball (U)
				case 0x163E86C0UL: // To the Earth (U)

					return REV_A;
			}

			return REV_B_C;
		}

		Mapper4::Mapper4(Context& c)
		:
		Mmc3 (c,WRAM_8K,GetRevision(c.prgCrc)),
		mmc6 (IsMmc6(c.prgCrc))
		{}

		void Mapper4::SubReset(const bool hard)
		{
			Mmc3::SubReset( hard );

			if (mmc6.indeed)
			{
				mmc6.wRam = 0;

				Map( 0x6000U, 0x6FFFU, NOP_PEEK_POKE );
				Map( 0x7000U, 0x7FFFU, &Mapper4::Peek_Mmc6_wRam, &Mapper4::Poke_Mmc6_wRam );

				for (uint i=0xA001U; i < 0xC000U; i += 0x2)
					Map( i, &Mapper4::Poke_Mmc6_A001 );
			}
		}

		void Mapper4::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0') && mmc6.indeed)
					mmc6.wRam = state.Read8();

				state.End();
			}
		}

		void Mapper4::SubSave(State::Saver& state) const
		{
			if (mmc6.indeed)
				state.Begin('R','E','G','\0').Write8( mmc6.wRam ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		inline ibool Mapper4::Mmc6::IsWRamEnabled() const
		{
			return wRam & (Mmc6::WRAM_LO_BANK_ENABLED|Mmc6::WRAM_HI_BANK_ENABLED);
		}

		inline ibool Mapper4::Mmc6::IsWRamReadable(uint address) const
		{
			return (wRam >> ((address & 0x200) >> 8)) & 0x20;
		}

		inline ibool Mapper4::Mmc6::IsWRamWritable(uint address) const
		{
			return ((wRam >> ((address & 0x200) >> 8)) & 0x30) == 0x30;
		}

		NES_POKE(Mapper4,Mmc6_wRam)
		{
			NST_VERIFY( mmc6.IsWRamWritable( address ) );

			if (mmc6.IsWRamWritable( address ))
			{
				address &= 0x3FFU;

				// mirror data just for 8K save file compatibility

				wrk[0][0x1C00 + address] =
				wrk[0][0x1800 + address] =
				wrk[0][0x1400 + address] =
				wrk[0][0x1000 + address] =
				wrk[0][0x0C00 + address] =
				wrk[0][0x0800 + address] =
				wrk[0][0x0400 + address] =
				wrk[0][0x0000 + address] = data;
			}
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
