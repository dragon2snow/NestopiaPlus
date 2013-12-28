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

#include <cstring>
#include <new>
#include "NstCore.hpp"
#include "NstCpu.hpp"
#include "NstSoundPlayer.hpp"
	   
namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif

			class Player::SampleLoader : public Loader
			{		
				Slots& slots;

				Result Load(uint slot,const void* input,dword length,bool stereo,uint bits,dword rate)
				{
					Result result;
					i16* data;

					if (slot >= slots.size() || slots[slot].data)
					{
						return RESULT_ERR_INVALID_PARAM;
					}
					else if (NES_FAILED(result=CanDo( input, length, bits, rate )))
					{
						return result;
					}
					else if (NULL == (data = new (std::nothrow) i16 [length]))
					{
						return RESULT_ERR_OUT_OF_MEMORY;
					}

					slots[slot].data = data;
					slots[slot].length = length;
					slots[slot].rate = rate;

					if (bits == 8)
					{
						const u8* NST_RESTRICT src = static_cast<const u8*>(input);

						if (stereo)
						{
							while (length--)
							{
								*data++ = (src[0] << 8) - 32768L + (src[1] << 8) - 32768L;
								src += 2;
							}
						}
						else
						{
							while (length--)
								*data++ = (*src++ << 8) - 32768L;
						}
					}
					else
					{
						NST_ASSERT( bits == 16 );

						const i16* NST_RESTRICT src = static_cast<const i16*>(input);

						if (stereo)
						{
							while (length--)
							{
								*data++ = src[0] + src[1];
								src += 2;
							}
						}
						else
						{
							while (length--)
								*data++ = *src++;
						}
					}

					return RESULT_OK;
				}

			public:

				SampleLoader(Slots& s)
				: slots(s) {}
			};

			Player::Player(Cpu& c,uint n)
			: Pcm(c), slots(n) 
			{
				NST_ASSERT( n );
			}

			Player* Player::Create(Cpu& cpu,Loader::Type type,uint samples)
			{
				if (samples)
				{
					if (Player* const player = new (std::nothrow) Player(cpu,samples))
					{
						{
							SampleLoader loader( player->slots );
							Loader::loadCallback( type, loader );
						}

						while (samples--)
						{
							if (player->slots[samples].data)
								return player;
						}

						delete player;
					}
				}

				return NULL;
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		}
	}
}
