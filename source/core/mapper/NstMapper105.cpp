////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#include <cstdio>
#include <cstring>
#include "../NstMapper.hpp"
#include "../board/NstBrdMmc1.hpp"
#include "NstMapper105.hpp"
#include "../api/NstApiUser.hpp"
			 
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper105::Mapper105(Context& c)
		: 
		Mmc1        (c), 
		dipValue    (0),
		displayTime (true)
		{
			std::strcpy( text, "Time Left: " );
		}
	
		void Mapper105::SubReset(const bool hard)
		{
			irqEnabled = false;
			Mmc1::SubReset( hard );
	
			prg.SwapBank<NES_16K,0x4000U>( 1 );
	
			static const u16 lut[16] =
			{
				5.001 * 60,
				5.316 * 60,
				5.629 * 60,
				5.942 * 60,
				6.254 * 60,
				6.567 * 60,
				6.880 * 60,
				7.193 * 60,
				7.505 * 60,
				7.818 * 60,
				8.131 * 60,
				8.444 * 60,
				8.756 * 60,
				9.070 * 60,
				9.318 * 60,
				9.695 * 60
			};
	
			NST_ASSERT( dipValue < 16 );
	
			time = lut[dipValue];
		}
	
		void Mapper105::SubLoad(State::Loader& state)
		{
			irqEnabled = (regs[1] & IRQ_DISABLE) ^ IRQ_DISABLE;
			frames = 1;
	
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('T','I','M','\0'))
				{
					seconds = state.Read32();
					seconds = NST_CLAMP(seconds,1,time);
				}
	
				state.End();
			}
		}
	
		void Mapper105::SubSave(State::Saver& state) const
		{
			state.Begin('T','I','M','\0').Write32( seconds ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		void Mapper105::UpdatePrg()
		{
			if (regs[1] & 0x8)
			{
				switch (regs[0] & 0xC)
				{
					case 0x0:
					case 0x4:
				
						prg.SwapBank<NES_32K,0x0000U>( (0x8 + (regs[3] & 0x6)) >> 1 );
						break;
				
					case 0x8:
				
						prg.SwapBanks<NES_16K,0x0000U>( 0x8, 0x8 + (regs[3] & 0x7) );
						break;
				
					case 0xC:
				
						prg.SwapBanks<NES_16K,0x0000U>( 0x8 + (regs[3] & 0x7), 0xF );
						break;
				}
			}
			else
			{	
				prg.SwapBank<NES_32K,0x0000U>( (regs[1] & 0x6) >> 1 );
			}
		}
	
		void Mapper105::UpdateRegister0()
		{
			UpdateMirroring();
			UpdatePrg();
		}
	
		void Mapper105::UpdateRegister1()
		{
			const uint state = (regs[1] & IRQ_DISABLE) ^ IRQ_DISABLE;
	
			if (irqEnabled != state)
			{
				irqEnabled = state;
	
				if (state)
				{
					frames = 1;
					seconds = time;
				}
				else
				{
					cpu.ClearIRQ();
				}
			}
	
			UpdatePrg();
		}
	
		void Mapper105::UpdateRegister3()
		{
			UpdatePrg();
		}
	
		cstring Mapper105::GetDipSwitchName(const uint i) const
		{ 
			return (i == 0) ? "Time" : (i == 1) ? "Show Timer" : NULL; 
		}
	
		cstring Mapper105::GetDipSwitchValueName(const uint i,const uint j) const
		{
			if (i == 0)
			{
				switch (j)
				{
					case 0x0: return "5.001";
					case 0x1: return "5.316";
					case 0x2: return "5.629";
					case 0x3: return "5.942";
					case 0x4: return "6.254";
					case 0x5: return "6.567";
					case 0x6: return "6.880";
					case 0x7: return "7.193";
					case 0x8: return "7.505";
					case 0x9: return "7.818";
					case 0xA: return "8.131";
					case 0xB: return "8.444";
					case 0xC: return "8.756";
					case 0xD: return "9.070";
					case 0xE: return "9.318";
					case 0xF: return "9.695";
				}
			}
			else if (i == 1)
			{
				switch (j)
				{
					case 0x0: return "no";
					case 0x1: return "yes";
				}
			}
	
			return NULL;
		}
	
		int Mapper105::GetDipSwitchValue(const uint i) const
		{
			switch (i)
			{
				case 0x0: return dipValue;
				case 0x1: return (displayTime != 0);
			}
	
			return -1;
		}
	
		Result Mapper105::SetDipSwitchValue(const uint i,const uint j)
		{
			if (i == 0 && j < 16)
			{
				dipValue = j;
				return RESULT_OK;
			}
			else if (i == 1 && j < 2)
			{
				displayTime = j;
				return RESULT_OK;
			}
	
			return RESULT_ERR_INVALID_PARAM;
		}
	
		void Mapper105::VSync()
		{
			if (irqEnabled)
			{
				if (!--frames)
				{
					frames = (cpu.GetMode() == MODE_NTSC ? 60 : 50);
	
					if (displayTime)
						std::sprintf( text + TIME_OFFSET, "%u:%u", uint(seconds / 60), uint(seconds % 60) );
	
					if (!--seconds)
					{
						seconds = time;
						cpu.DoIRQ();
					}
				}
	
				if (displayTime)
					Api::User::eventCallback( Api::User::EVENT_DISPLAY_TIMER, text );
			}

			Mmc1::VSync();
		}
	}
}
