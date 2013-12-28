////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#include "input/NstInpDevice.hpp"
#include "NstIoAdapter.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
	
			AdapterTwo::AdapterTwo(Device* a,Device* b)
			{
				devices[0] = a;
				devices[1] = b;
			}
	
			Device* AdapterTwo::Connect(uint port,Device* device)
			{
				NST_ASSERT( port < 2 );
	
				Device* old = devices[port];
				devices[port] = device;
	
				return old;
			}
	
			void AdapterTwo::Initialize(dword crc)
			{
				for (uint i=0; i < 2; ++i)
					devices[i]->Initialize( crc );
			}
	
			void AdapterTwo::Reset()
			{
				for (uint i=0; i < 2; ++i)
					devices[i]->Reset();
			}
	
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
	
			uint AdapterTwo::NumPorts() const
			{
				return 2;
			}
	
			Device* AdapterTwo::GetDevice(uint port) const
			{
				NST_ASSERT( port < 2 );
				return devices[port];
			}
	
			void AdapterTwo::BeginFrame(Controllers* input)
			{
				devices[0]->BeginFrame( input );
				devices[1]->BeginFrame( input );
			}
					
			void AdapterTwo::Poke(uint data)
			{
				devices[0]->Poke( data );
				devices[1]->Poke( data );
			}
	
			uint AdapterTwo::Peek(uint line)
			{
				NST_ASSERT( line < 2 );
				return devices[line]->Peek( line );
			}
	
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
	
			AdapterFour::AdapterFour(Device* a,Device* b,Device* c,Device* d)
			: fourscore(true), increaser(1)
			{
				count[1] = count[0] = 0;

				devices[0] = a;
				devices[1] = b;
				devices[2] = c;
				devices[3] = d;
			}
	
			Device* AdapterFour::Connect(uint port,Device* device)
			{
				NST_ASSERT( port < 4 );
	
				Device* old = devices[port];
				devices[port] = device;
	
				return old;
			}
	
			void AdapterFour::Initialize(dword crc)
			{
				switch (crc)
				{
					case 0x85C5B6B4UL: // Nekketsu Kakutou Densetsu
					case 0x3B7B3BE1UL: // -||- (T)
					case 0x9771A019UL: // -||- (T)
					case 0xAAFED9B4UL: // -||-
					case 0x7BD7B849UL: // Nekketsu Koukou - Dodgeball Bu
					case 0xDEDDD5E5UL: // Kunio Kun no Nekketsu Soccer League
					case 0x6EF0C08EUL: // -||- (T)
					case 0x6375C11EUL: // -||- (T)
					case 0xF985AC97UL: // -||- (T)
					case 0xBA11692DUL: // -||- (T)
					case 0x4FB460CDUL: // Nekketsu! Street Basket - Ganbare Dunk Heroes
					case 0x457BC688UL: // -||- (T)
					case 0x5D4A01A9UL: // -||- (T)

						fourscore = false;
						break;

					default:

						fourscore = true;
						break;
				}

				for (uint i=0; i < 4; ++i)
					devices[i]->Initialize( crc );
			}
	
			void AdapterFour::Reset()
			{
				increaser = 1;
				count[1] = count[0] = 0;
	
				for (uint i=0; i < 4; ++i)
					devices[i]->Reset();
			}
	
			void AdapterFour::SaveState(State::Saver& state,const dword id) const
			{
				if (fourscore)
				{
					const u8 data[3] =
					{
						increaser ^ 1, count[0], count[1]
					};

					state.Begin( id ).Write( data ).End();
				}
			}

			void AdapterFour::LoadState(State::Loader& state)
			{
				if (fourscore)
				{
					const State::Loader::Data<3> data( state );

					increaser = (data[0] & 0x1) ^ 1;
					count[0] = data[1] <= 20 ? data[1] : 0;
					count[1] = data[2] <= 20 ? data[2] : 0;
				}
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
	
			uint AdapterFour::NumPorts() const
			{
				return 4;
			}
	
			Device* AdapterFour::GetDevice(uint port) const
			{
				NST_ASSERT( port < 4 );
				return devices[port];
			}
	
			void AdapterFour::BeginFrame(Controllers* input)
			{
				for (uint i=0; i < 4; ++i)
					devices[i]->BeginFrame( input );
			}
	
			void AdapterFour::Poke(const uint data)
			{
				if (fourscore)
				{
					increaser = (data & 0x1) ^ 1;

					if (!increaser)
						count[1] = count[0] = 0;
				}
	
				for (uint i=0; i < 4; ++i)
					devices[i]->Poke( data );
			}
	
			uint AdapterFour::Peek(const uint line)
			{
				NST_ASSERT( line < 2 );
	
				if (fourscore)
				{
					const uint index = count[line];

					if (index < 20)
					{
						count[line] += increaser;

						if (index < 16)
						{
							return devices[line + (index < 8 ? 0 : 2)]->Peek( line );
						}
						else if (index >= 18)
						{
							return (index - 18) ^ line;
						}
					}

					return 0;
				}
				else 
				{
					return 
					(
						((devices[line+0]->Peek( line ) & 0x1) << 0) | 
						((devices[line+2]->Peek( line ) & 0x1) << 1)
					);
				}
			}
		}
	}
}
