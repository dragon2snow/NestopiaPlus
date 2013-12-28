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

#include "NstInpDevice.hpp"
#include "NstInpKeyboard.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
	
			Keyboard::Keyboard() 
			: Device(Api::Input::KEYBOARD)
			{ 
				Reset();
			}
	
			void Keyboard::Reset()
			{
				scan = 0;
				mode = 0;
			}
	
			void Keyboard::SaveState(State::Saver& state,const uchar id) const
			{
				state.Begin('K','B',id,'\0').Write8( mode | (scan << 1) ).End();
			}

			void Keyboard::LoadState(State::Loader& state,const dword id)
			{
				if (id == NES_STATE_CHUNK_ID('K','B','\0','\0'))
				{
					 const uint data = state.Read8();

					 mode = data & 0x1;
					 scan = (data >> 1) & 0xF;

					 if (scan > 9)
						 scan = 0;
				}
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
	
			void Keyboard::Poke(const uint data)
			{
				if (data & COMMAND_KEY)
				{
					const uint out = data & COMMAND_SCAN;
	
					if (mode && !out && ++scan > 9)
						scan = 0;
	
					mode = out >> 1;
	
					if (data & COMMAND_RESET)
						scan = 0;
				}
			}
	
			uint Keyboard::Peek(uint port)
			{
				if (port == 0)
				{
					return 0x00;
				}
				else if (input && Controllers::Keyboard::callback( input->keyboard, scan, mode ))
				{
					return ~input->keyboard.parts[scan] & 0x1E;
				}
				else
				{
					return 0x1E;
				}
			}
		}
	}
}
