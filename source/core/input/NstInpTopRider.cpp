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

#include "NstInpDevice.hpp"
#include "NstInpTopRider.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
	
			TopRider::TopRider()
			: Device(Api::Input::TOPRIDER)
			{
				TopRider::Reset();
			}
	
			void TopRider::Reset()
			{
				pos = accel = brake = buttons = 0;
				stream[1] = stream[0] = 0;
			}
	
			void TopRider::SaveState(State::Saver& state,const uchar id) const
			{
				state.Begin('T','R',id,'\0').End();
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			void TopRider::BeginFrame(Controllers* const input)
			{ 
				if (input && Controllers::TopRider::callback( input->topRider ))
				{
					uint data = input->topRider.buttons;
	
					if ((data & STEERING) == STEERING)
						data &= STEERING ^ 0xFF;
	
					     if ( !(data & STEERING) ) pos += (pos > 0 ? -1 : pos < 0 ? +1 : 0);
					else if ( data & STEER_LEFT  ) pos -= (pos > -MAX_STEER);
					else if ( data & STEER_RIGHT ) pos += (pos < +MAX_STEER);
	
					if (data & BRAKE) brake += (brake < MAX_BRAKE);
					else              brake -= (brake > 0);
	
					if (data & ACCEL) accel += (accel < MAX_ACCEL);
					else              accel -= (accel > 0);
	
					buttons &= (0x80|0x40);
	
					if (data & SHIFT_GEAR)
					{
						if (!(buttons & 0x40))
						{
							buttons ^= 0x80;
							buttons |= 0x40;
						}
					}
					else
					{
						buttons &= 0x40 ^ 0xFF;
					}
	
					buttons |=
					(
						(( data & REAR   ) >> 5) |
						(( data & SELECT ) << 3) |
						(( data & START  ) << 1)
					);
				}
				else
				{
					pos = accel = brake = buttons = 0;
				}
			}
	
			void TopRider::Poke(uint data)
			{
				if (data & 0x1)
				{
					data = 0;
	
					if (pos > 0)
					{
						     if (pos > DEADZONE_MAX) data = (0x20 | 0x080);
						else if (pos > DEADZONE_MID) data = (0x20 | 0x000);
						else if (pos > DEADZONE_MIN) data = (0x00 | 0x080);
					}
					else
					{
						     if (pos < -DEADZONE_MAX) data = (0x40 | 0x100);
						else if (pos < -DEADZONE_MID) data = (0x40 | 0x000);
						else if (pos < -DEADZONE_MIN) data = (0x00 | 0x100);
					}
	
					stream[0] = data | ((buttons & 0x01) << (4+7))	| ((buttons & 0x80) << (4-1));
					data = 0;
	
					if (accel > 8 || brake < 8)
					{
						     if (accel > DEADZONE_MAX) data = 0x008;
						else if (accel > DEADZONE_MID) data = 0x080;
						else if (accel > DEADZONE_MIN) data = 0x100;
					}
					else
					{
						stream[0] |= 0x200;
				
						     if (brake > DEADZONE_MAX) data = 0x10;
						else if (brake > DEADZONE_MID) data = 0x20;
						else if (brake > DEADZONE_MIN) data = 0x40;
					}
	
					stream[1] = data | ((buttons & 0x30) << (3+2));
				}
			}
	
			uint TopRider::Peek(uint port)
			{
				if (port)
				{
					port = (stream[0] & 0x10) | (stream[1] & 0x08);
					stream[0] >>= 1, stream[1] >>= 1;
				}
	
				return port;
			}
		}
	}
}
