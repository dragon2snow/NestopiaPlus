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
#include "NstVideoFilterScanlines.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif

			Renderer::FilterScanlines::FilterScanlines(const RenderState& state)
			: 
			Filter  (state),
			scale   (state.width != WIDTH),
			darken  (state.filter == Api::Video::RenderState::FILTER_SCANLINES_DARK ? 2 : 1),
			zeroLow (~(((darken==2 ? 3UL : 1UL) << format.left[0]) | ((darken==2 ? 3UL : 1UL) << format.left[1]) | ((darken==2 ? 3UL : 1UL) << format.left[2])))
			{
			}

			bool Renderer::FilterScanlines::Check(const RenderState& state)
			{
				return
				(
					(state.bits.count == 16 || state.bits.count == 32) &&
					(state.width == WIDTH || state.width == WIDTH*2) &&
					(state.height == HEIGHT || state.height == HEIGHT*2)
				);
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			template<typename T> 
			NST_FORCE_INLINE void Renderer::FilterScanlines::Blit2x(const Input& input,const Output& output) const
			{
				const u16* NST_RESTRICT src = input.screen;
				T* NST_RESTRICT dst = static_cast<T*>(output.pixels);

				const long pitch = output.pitch;

				for (uint y=0; y < HEIGHT; ++y)
				{
					register dword p;

					for (uint x=0; x < WIDTH; ++x)
					{
						dst[x*2+0] = p = input.palette[src[x]];
						dst[x*2+1] = p;
					}

					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);

					for (uint x=0; x < WIDTH; ++x)
					{
						dst[x*2+0] = p = (input.palette[src[x]] & zeroLow) >> darken;
						dst[x*2+1] = p;
					}

					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
					src += WIDTH;
				}
			}

			template<typename T> 
			NST_FORCE_INLINE void Renderer::FilterScanlines::Blit1x(const Input& input,const Output& output) const
			{
				const u16* NST_RESTRICT src = input.screen;
				T* NST_RESTRICT dst = static_cast<T*>(output.pixels);

				const long pitch = output.pitch;

				for (uint y=0; y < HEIGHT; y += 2)
				{
					for (uint x=0; x < WIDTH; ++x)
						dst[x] = input.palette[src[x]];

					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
					src += WIDTH;

					for (uint x=0; x < WIDTH; ++x)
						dst[x] = (input.palette[src[x]] & zeroLow) >> darken;

					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
					src += WIDTH;
				}
			}

			template<typename T>
			NST_FORCE_INLINE void Renderer::FilterScanlines::BlitType(const Input& input,const Output& output) const
			{
				if (scale)
					Blit2x<T>( input, output );
				else
					Blit1x<T>( input, output );
			}

			void Renderer::FilterScanlines::Blit(const Input& input,const Output& output)
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
