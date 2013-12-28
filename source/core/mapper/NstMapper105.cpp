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

		Mapper105::CartSwitches::CartSwitches()
		: time(0), showTime(true) {}

		Mapper105::Mapper105(Context& c)
		: Mmc1(c,BRD_GENERIC_WRAM,REV_B)
		{
			std::strcpy( text, "Time Left: " );
		}

		uint Mapper105::CartSwitches::NumDips() const
		{
			return 2;
		}

		uint Mapper105::CartSwitches::NumValues(uint dip) const
		{
			NST_ASSERT( dip < 2 );
			return (dip == 0) ? 16 : 2;
		}

		cstring Mapper105::CartSwitches::GetDipName(uint dip) const
		{
			NST_ASSERT( dip < 2 );
			return (dip == 0) ? "Time" : "Show Time";
		}

		cstring Mapper105::CartSwitches::GetValueName(uint dip,uint value) const
		{
			NST_ASSERT( dip < 2 );

			if (dip == 0)
			{
				NST_ASSERT( value < 16 );

				switch (value)
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

					NST_UNREACHABLE
				}
			}

			NST_ASSERT( value < 2 );
			return (value == 0) ? "no" : "yes";
		}

		uint Mapper105::CartSwitches::GetValue(uint dip) const
		{
			NST_ASSERT( dip < 2 );
			return (dip == 0) ? time : showTime;
		}

		bool Mapper105::CartSwitches::SetValue(uint dip,uint value)
		{
			NST_ASSERT( dip < 2 );

			if (dip == 0)
			{
				NST_ASSERT( value < 16 );

				if (time != value)
				{
					time = value;
					return true;
				}
			}
			else
			{
				NST_ASSERT( value < 2 );

				if (showTime != value)
				{
					showTime = value;
					return true;
				}
			}

			return false;
		}

		uint Mapper105::CartSwitches::GetTime() const
		{
			static const u16 lut[16] =
			{
				300, // 5.001 * 60
				318, // 5.316 * 60
				337, // 5.629 * 60
				356, // 5.942 * 60
				375, // 6.254 * 60
				394, // 6.567 * 60
				412, // 6.880 * 60
				431, // 7.193 * 60
				450, // 7.505 * 60
				469, // 7.818 * 60
				487, // 8.131 * 60
				506, // 8.444 * 60
				525, // 8.756 * 60
				544, // 9.070 * 60
				559, // 9.318 * 60
				581  // 9.695 * 60
			};

			NST_ASSERT( time < 16 );

			return lut[time];
		}

		inline bool Mapper105::CartSwitches::ShowTime() const
		{
			return showTime;
		}

		Mapper105::Device Mapper105::QueryDevice(DeviceType type)
		{
			if (type == DEVICE_DIP_SWITCHES)
				return &cartSwitches;
			else
				return Mapper::QueryDevice( type );
		}

		void Mapper105::SubReset(const bool hard)
		{
			irqEnabled = false;
			time = cartSwitches.GetTime();

			Mmc1::SubReset( hard );

			prg.SwapBank<SIZE_16K,0x4000U>( 1 );
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

		void Mapper105::UpdateRegisters(const uint index)
		{
			NST_ASSERT( index < 4 );

			if (index != 2)
			{
				if (regs[1] & 0x8)
				{
					switch (regs[0] & 0xC)
					{
						case 0x0:
						case 0x4:

							prg.SwapBank<SIZE_32K,0x0000U>( 0x4 | (regs[3] >> 1 & 0x3) );
							break;

						case 0x8:

							prg.SwapBanks<SIZE_16K,0x0000U>( 0x8, 0x8 | (regs[3] & 0x7) );
							break;

						case 0xC:

							prg.SwapBanks<SIZE_16K,0x0000U>( 0x8 | (regs[3] & 0x7), 0xF );
							break;
					}
				}
				else
				{
					prg.SwapBank<SIZE_32K,0x0000U>( regs[1] >> 1 & 0x3 );
				}

				UpdateWrk();

				if (index == 0)
				{
					UpdateNmt();
				}
				else
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
				}
			}
		}

		void Mapper105::VSync()
		{
			if (irqEnabled)
			{
				if (!--frames)
				{
					frames = (cpu.GetMode() == MODE_NTSC ? 60 : 50);

					if (cartSwitches.ShowTime())
						std::sprintf( text + TIME_OFFSET, "%u:%u", uint(seconds / 60), uint(seconds % 60) );

					if (!--seconds)
					{
						seconds = time;
						cpu.DoIRQ();
					}
				}

				if (cartSwitches.ShowTime())
					Api::User::eventCallback( Api::User::EVENT_DISPLAY_TIMER, text );
			}

			Mmc1::VSync();
		}
	}
}
