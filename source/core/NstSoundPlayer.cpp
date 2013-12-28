////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#include <new>
#include "NstCpu.hpp"
#include "NstSoundPlayer.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			class Player::SampleLoader : public Loader
			{
				Slot* const slots;
				const uint numSlots;

				Result Load(uint slot,const void* input,ulong length,bool stereo,uint bits,ulong rate) throw()
				{
					Result result;
					iword* data;

					if (slot >= numSlots || slots[slot].data)
					{
						return RESULT_ERR_INVALID_PARAM;
					}
					else if (NES_FAILED(result=CanDo( input, length, bits, rate )))
					{
						return result;
					}
					else if (NULL == (data = new (std::nothrow) iword [length]))
					{
						return RESULT_ERR_OUT_OF_MEMORY;
					}

					slots[slot].data = data;
					slots[slot].length = length;
					slots[slot].rate = rate;

					if (bits == 8)
					{
						const byte* NST_RESTRICT src = static_cast<const byte*>(input);
						const byte* const end = src + length;

						if (stereo)
						{
							for (; src != end; src += 2)
							{
								const idword sample = (idword(uint(src[0]) << 8) - 32768) + (idword(uint(src[1]) << 8) - 32768);
								*data++ = NST_CLAMP(sample,Apu::OUTPUT_MIN,Apu::OUTPUT_MAX);
							}
						}
						else
						{
							for (; src != end; src += 1)
							{
								const idword sample = idword(uint(*src) << 8) - 32768;
								*data++ = NST_CLAMP(sample,Apu::OUTPUT_MIN,Apu::OUTPUT_MAX);
							}
						}
					}
					else
					{
						NST_ASSERT( bits == 16 );

						const iword* NST_RESTRICT src = static_cast<const iword*>(input);
						const iword* const end = src + length;

						if (stereo)
						{
							for (; src != end; src += 2)
							{
								const idword sample = src[0] + src[1];
								*data++ = NST_CLAMP(sample,Apu::OUTPUT_MIN,Apu::OUTPUT_MAX);
							}
						}
						else
						{
							for (; src != end; src += 1)
							{
								const idword sample = *src;
								*data++ = NST_CLAMP(sample,Apu::OUTPUT_MIN,Apu::OUTPUT_MAX);
							}
						}
					}

					return RESULT_OK;
				}

			public:

				SampleLoader(Slot* s,uint n)
				: slots(s), numSlots(n) {}
			};

			Player::Slot::Slot()
			: data(NULL) {}

			Player::Slot::~Slot()
			{
				delete [] data;
			}

			Player::Player(Cpu& c,uint n)
			: Pcm(c), slots(new Slot [n]), numSlots(n)
			{
				NST_ASSERT( n );
			}

			Player::~Player()
			{
				delete [] slots;
			}

			Player* Player::Create(Cpu& cpu,Loader::Type type,uint samples)
			{
				if (samples)
				{
					if (Player* const player = new (std::nothrow) Player(cpu,samples))
					{
						{
							SampleLoader loader( player->slots, samples );
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

			void Player::Destroy(Player* player)
			{
				delete player;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif
		}
	}
}
