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

#include "NstCore.hpp"
#include "api/NstApiVideo.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterTV.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif

			Renderer::FilterTV::Mask::Mask(const RenderState::Bits::Mask& m)
			: r(m.r), g(m.g), b(m.b) {}

			Renderer::FilterTV::FilterTV(const RenderState& state)
			: Filter(state), mask(state.bits.mask)
			{
			}

			bool Renderer::FilterTV::Check(const RenderState& state)
			{
				return
				(
					(state.bits.count == 16 || state.bits.count == 32) &&
					(state.width == WIDTH*2 && state.height == HEIGHT*2)
				);
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			template<typename T>
			NST_FORCE_INLINE void Renderer::FilterTV::BlitType(const Input& input,const Output& output) const
			{
				const u16* NST_RESTRICT src = input.screen;
				T* NST_RESTRICT dst = static_cast<T*>(output.pixels);

				const long pitch = output.pitch;

				for (uint y=0; y < HEIGHT; ++y)
				{
					dword p, q;

					p = input.palette[src[0]];

					for (uint x=0; x < WIDTH-1; ++x)
					{
						dst[x*2+0] = p;

						q = p;
						p = input.palette[src[x+1]];

						dst[x*2+1] =
						(
							((((q & mask.r) + (p & mask.r)) >> 1) & mask.r) | 
							((((q & mask.g) + (p & mask.g)) >> 1) & mask.g) | 
							((((q & mask.b) + (p & mask.b)) >> 1) & mask.b)
						);
					}

					dst[WIDTH*2-2] = p;
					dst[WIDTH*2-1] = p;

					const u16* const NST_RESTRICT next = src + (y < HEIGHT-1 ? WIDTH+1 : 1);
					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);

					q = input.palette[next[0]];

					for (uint x=0; x < WIDTH-2; ++x)
					{
						p = input.palette[src[x]];

						dst[x*2+0] =
						(
							(((((p & mask.r) + (q & mask.r)) * 11) >> 5) & mask.r) | 
							(((((p & mask.g) + (q & mask.g)) * 11) >> 5) & mask.g) | 
							(((((p & mask.b) + (q & mask.b)) * 11) >> 5) & mask.b)
						);

						q = input.palette[next[x+1]];

						dst[x*2+1] =
						(
							(((((p & mask.r) + (q & mask.r)) * 11) >> 5) & mask.r) | 
							(((((p & mask.g) + (q & mask.g)) * 11) >> 5) & mask.g) | 
							(((((p & mask.b) + (q & mask.b)) * 11) >> 5) & mask.b)
						);
					}

					for (uint x=WIDTH-2; x < WIDTH; ++x)
					{
						p = input.palette[src[x]];

						dst[x*2+0] = p =
						(
							(((((p & mask.r) + (q & mask.r)) * 11) >> 5) & mask.r) | 
							(((((p & mask.g) + (q & mask.g)) * 11) >> 5) & mask.g) | 
							(((((p & mask.b) + (q & mask.b)) * 11) >> 5) & mask.b)
						);

						dst[x*2+1] = p;
					}

					src += WIDTH;
					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
				}
			}

			void Renderer::FilterTV::Blit(const Input& input,const Output& output)
			{
				switch (bpp)
				{
					case 32: BlitType< u32 >( input, output ); break;
					case 16: BlitType< u16 >( input, output ); break;
					
					NST_UNREACHABLE
				}
			}

		}
	}
}