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
#include "NstMapper235.hpp"
		
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper235::SubReset(const bool hard)
		{
			if (hard)
				open = false;

			Map( 0x8000U, 0xFFFFU, &Mapper235::Peek_Prg, &Mapper235::Poke_Prg );
		
			switch (prg.Source().Size())
			{						
				case 0x100000UL: SelectCartridge = &Mapper235::Select1024k; break;
				case 0x200000UL: SelectCartridge = &Mapper235::Select2048k; break;
				case 0x300000UL: SelectCartridge = &Mapper235::Select3072k; break;
				default:         SelectCartridge = NULL; break;
			}
		}
	
		void Mapper235::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('B','U','S','\0'))
					open = state.Read8() & 0x1;
	
				state.End();
			}
		}
	
		void Mapper235::SubSave(State::Saver& state) const
		{
			state.Begin('B','U','S','\0').Write8( open != 0 ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		uint Mapper235::Select1024k(const uint address,uint bank)
		{
			open = address & 0x0300;
			return bank;
		}
	
		uint Mapper235::Select2048k(const uint address,uint bank)
		{
			switch (address & 0x0300)
			{
				case 0x0100:
				case 0x0300: open = true; break;
				case 0x0200: bank = (bank & 0x1F) | 0x20; break;
			}

			return bank;
		}
	
		uint Mapper235::Select3072k(const uint address,uint bank)
		{
			switch (address & 0x0300)
			{
				case 0x0100: open = true; break;
				case 0x0200: bank = (bank & 0x1F) | 0x20; break;
				case 0x0300: bank = (bank & 0x1F) | 0x40; break;
			}

			return bank;
		}
	
		NES_POKE(Mapper235,Prg) 
		{ 
			ppu.SetMirroring
			( 
         		(address & 0x0400U) ? Ppu::NMT_ZERO : 
           		(address & 0x2000U) ? Ppu::NMT_HORIZONTAL : 
                           	       	  Ppu::NMT_VERTICAL 
			);
	
			open = false;
			uint bank = (address >> 3 & 0x60) | (address & 0x1F);
	
			if (SelectCartridge)
				bank = (*this.*SelectCartridge)( address, bank );
	
			if (address & 0x800)
			{
				bank = (bank << 1) | (address >> 12 & 0x1);
				prg.SwapBanks<SIZE_16K,0x0000U>( bank, bank );
			}
			else
			{
				prg.SwapBank<SIZE_32K,0x0000U>( bank );
			}
		}
	
		NES_PEEK(Mapper235,Prg)
		{
			return open == 0 ? prg.Peek(address - 0x8000U) : address >> 8;
		}
	}
}
