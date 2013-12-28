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
#include "NstInpPad.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			uint Pad::mic;

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif		

			Pad::Pad(uint i)
			: Device(Type(uint(Api::Input::PAD1) + i))
			{
				NST_ASSERT( i < 4 );

				NST_COMPILE_ASSERT
				( 
					( Api::Input::PAD2 - Api::Input::PAD1 ) == 1 &&
					( Api::Input::PAD3 - Api::Input::PAD1 ) == 2 &&
					( Api::Input::PAD4 - Api::Input::PAD1 ) == 3
				);

				Pad::Reset();
			}
		
			void Pad::Reset()
			{
				strobe = 0;
				stream = 0xFF;
				state = 0;
			}

			void Pad::SaveState(State::Saver& state,const uchar id) const
			{
				const u8 data[2] = 
				{
					strobe, stream ^ 0xFF
				};

				state.Begin('P','D',id,'\0').Write( data ).End();
			}

			void Pad::LoadState(State::Loader& state,const dword id)
			{
				if (id == NES_STATE_CHUNK_ID('P','D','\0','\0'))
				{
					const State::Loader::Data<2> data( state );

					strobe = data[0] & 0x1;
					stream = data[1] ^ 0xFF;
				}
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			void Pad::BeginFrame(Controllers* i)
			{
				input = i;
				state = 0;
				mic = 0;
			}

			void Pad::Poll()
			{
				NST_ASSERT( input );
				
				Controllers::Pad& pad = input->pad[type - Api::Input::PAD1];
				input = NULL;

				if (Controllers::Pad::callback( pad, type - Api::Input::PAD1 ))
				{
					mic |= pad.mic;
					uint buttons = pad.buttons;

					enum
					{
						UP    = Controllers::Pad::UP,
						RIGHT = Controllers::Pad::RIGHT,
						DOWN  = Controllers::Pad::DOWN,
						LEFT  =	Controllers::Pad::LEFT
					};

					if (!pad.allowSimulAxes)
					{
						if ((buttons & (UP|DOWN)) == (UP|DOWN))
							buttons &= (UP|DOWN) ^ 0xFF;

						if ((buttons & (LEFT|RIGHT)) == (LEFT|RIGHT))
							buttons &= (LEFT|RIGHT) ^ 0xFF;
					}

					state = buttons;
				}
			}

			uint Pad::Peek(uint port)
			{
				if (strobe == 0)
				{
					const uint data = stream;
					stream >>= 1;
					
					return (~data & 0x1) | (mic & (~port << 2));
				}
				else
				{
				//	NST_DEBUG_MSG("Pad::Peek() input stuck!");

					if (input)
						Poll();

					return state & 0x1;
				}
			}
		
			void Pad::Poke(const uint data)
			{
				const uint prev = strobe;
				strobe = data & 0x1;

				if (prev > strobe)
				{
					if (input)
						Poll();

					stream = state ^ 0xFF;
				}
			}
		}
	}
}
