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
#include "NstInpCrazyClimber.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			CrazyClimber::CrazyClimber()
			: Device(Api::Input::CRAZYCLIMBER)
			{
				CrazyClimber::Reset();
			}

			void CrazyClimber::Reset()
			{
				shifter = 1;
				stream[RIGHT] = stream[LEFT] = 0x00;
			}

			void CrazyClimber::SaveState(State::Saver& state,const uchar id) const
			{
				state.Begin('C','C',id,'\0').Write8( shifter ^ 1 ).End();
			}

			void CrazyClimber::LoadState(State::Loader& state,const dword id)
			{
				if (id == NES_STATE_CHUNK_ID('C','C','\0','\0'))
					shifter = ~state.Read8() & 0x1;
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void CrazyClimber::BeginFrame(Controllers* i)
			{
				input = i;
				state[RIGHT] = state[LEFT] = 0;
			}

			uint CrazyClimber::Peek(const uint port)
			{
				NST_ASSERT( port <= 1 );
				const uint data = stream[port];
				stream[port] >>= shifter;
				return data & 0x1;
			}

			void CrazyClimber::Poke(const uint data)
			{
				const uint prev = shifter;
				shifter = (data & 0x1) ^ 0x1;

				if (prev < shifter)
				{
					if (input)
					{
						Controllers::CrazyClimber& crazy = input->crazyClimber;
						input = NULL;

						if (Controllers::CrazyClimber::callback( crazy ))
						{
							state[LEFT] = crazy.left;
							state[RIGHT] = crazy.right;

							for (uint i=0; i < 2; ++i)
							{
								if ((state[i] & (BUTTON_LEFT|BUTTON_RIGHT)) == (BUTTON_LEFT|BUTTON_RIGHT))
									state[i] &= (BUTTON_LEFT|BUTTON_RIGHT) ^ 0xFF;

								if ((state[i] & (BUTTON_UP|BUTTON_DOWN)) == (BUTTON_UP|BUTTON_DOWN))
									state[i] &= (BUTTON_UP|BUTTON_DOWN) ^ 0xFF;
							}
						}
					}

					stream[LEFT] = state[LEFT];
					stream[RIGHT] = state[RIGHT];
				}
			}
		}
	}
}
