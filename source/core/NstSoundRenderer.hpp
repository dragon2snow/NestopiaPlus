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

#ifndef NST_SOUND_RENDERER_H
#define NST_SOUND_RENDERER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <cstring>
#include "api/NstApiSound.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			class Buffer
			{
			public:

				Buffer(uint);

				enum
				{
					SIZE = 0x8000U,
					MASK = SIZE-1
				};

				struct Block
				{
					const i16* data;
					uint start;
					uint length;

					Block(uint l)
					: length(l) {}
				};

				void Reset(uint,bool=true);
				void operator >> (Block&);

				template<typename,uint>
				class Renderer;

			private:

				template<typename T>
				class BaseRenderer
				{
				protected:

					T* NST_RESTRICT dst;
					const T* const end;

					BaseRenderer(void* samples,uint length,bool stereo=false)
					:
					dst (static_cast<T*>(samples)),
					end (static_cast<const T*>(samples) + (length << (uint) stereo))
					{}

				public:

					operator bool () const
					{
						return dst != end;
					}
				};

				struct History
				{
					enum
					{
						SIZE = 0x40,
						MASK = SIZE-1
					};

					uint pos;
					i16 buffer[SIZE];

					template<typename T>
					void operator >> (T& sample) const
					{
						sample = buffer[pos & MASK];
					}

					void operator << (Apu::Sample sample)
					{
						buffer[pos++ & MASK] = sample;
					}
				};

				i16 output[SIZE];
				uint pos;
				uint start;

			public:

				History history;

				void operator << (const Apu::Sample sample)
				{
					const uint p = pos;
					pos = (pos + 1) & MASK;
					output[p] = sample;
				}

				uint Latency() const
				{
					return (dword(pos) + SIZE - start) & MASK;
				}
			};

			template<>
			class Buffer::Renderer<i16,0U> : public Buffer::BaseRenderer<i16>
			{
			public:

				Renderer(void* samples,uint length)
				: BaseRenderer<i16>(samples,length) {}

				NST_FORCE_INLINE void operator << (Apu::Sample sample)
				{
					*dst++ = sample;
				}

				NST_FORCE_INLINE bool operator << (const Block& block)
				{
					NST_ASSERT( uint(end - dst) >= block.length );

					if (block.length)
					{
						if (block.start + block.length <= SIZE)
						{
							std::memcpy( dst, block.data + block.start, sizeof(i16) * block.length );
						}
						else
						{
							const uint chunk = SIZE - block.start;
							std::memcpy( dst, block.data + block.start, sizeof(i16) * chunk );
							std::memcpy( dst + chunk, block.data, sizeof(i16) * ((block.start + block.length) - SIZE) );
						}

						dst += block.length;
					}

					return dst != end;
				}
			};

			template<>
			class Buffer::Renderer<i16,1U> : public Buffer::BaseRenderer<i16>
			{
				History& history;

			public:

				Renderer(void* samples,uint length,History& h)
				: BaseRenderer<i16>(samples,length,true), history(h) {}

				NST_FORCE_INLINE void operator << (Apu::Sample sample)
				{
					history >> dst[0];
					history << sample;
					dst[1] = sample;
					dst += 2;
				}

				NST_FORCE_INLINE bool operator << (Block& block)
				{
					NST_ASSERT( uint(end - dst) >= block.length );

					block.length += block.start;

					for (uint i=block.start; i < block.length; ++i)
						(*this) << (Apu::Sample) block.data[i & MASK];

					return dst != end;
				}
			};

			template<>
			class Buffer::Renderer<u8,0U> : public Buffer::BaseRenderer<u8>
			{
			public:

				Renderer(void* samples,uint length)
				: BaseRenderer<u8>(samples,length) {}

				NST_FORCE_INLINE void operator << (Apu::Sample sample)
				{
					*dst++ = dword(sample + 32768L) >> 8;
				}

				NST_FORCE_INLINE bool operator << (Block& block)
				{
					NST_ASSERT( uint(end - dst) >= block.length );

					block.length += block.start;

					for (uint i=block.start; i < block.length; ++i)
						(*this) << (Apu::Sample) block.data[i & MASK];

					return dst != end;
				}
			};

			template<>
			class Buffer::Renderer<u8,1U> : public Buffer::BaseRenderer<u8>
			{
				History& history;

			public:

				Renderer(void* samples,uint length,History& h)
				: BaseRenderer<u8>(samples,length,true), history(h) {}

				NST_FORCE_INLINE void operator << (Apu::Sample sample)
				{
					history >> dst[0];
					sample = dword(sample + 32768L) >> 8;
					history << sample;
					dst[1] = sample;
					dst += 2;
				}

				NST_FORCE_INLINE bool operator << (Block& block)
				{
					NST_ASSERT( uint(end - dst) >= block.length );

					block.length += block.start;

					for (uint i=block.start; i < block.length; ++i)
						(*this) << (Apu::Sample) block.data[i & MASK];

					return dst != end;
				}
			};
		}
	}
}

#endif
