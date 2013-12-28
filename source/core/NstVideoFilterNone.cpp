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

#include "NstCore.hpp"
#include "api/NstApiVideo.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterNone.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Renderer::FilterNone::FilterNone(const RenderState& state)
			: Filter(state), paletteOffset(state.paletteOffset)
			{
			}

			bool Renderer::FilterNone::Check(const RenderState& state)
			{
				return
				(
					(state.bits.count == 8 || state.bits.count == 16 || state.bits.count == 32) &&
					(state.width == WIDTH && state.height == HEIGHT)
				);
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			template<typename T>
			NST_FORCE_INLINE void Renderer::FilterNone::BlitAligned(const Input& input,const Output& output) const
			{
				T* NST_RESTRICT const dst = static_cast<T*>(output.pixels);

				for (uint i=0; i < PIXELS; ++i)
					dst[i] = input.palette[input.screen[i]];
			}

			template<>
			NST_FORCE_INLINE void Renderer::FilterNone::BlitAligned<u8>(const Input& input,const Output& output) const
			{
				u8* NST_RESTRICT const dst = static_cast<u8*>(output.pixels);

				const uint offset = paletteOffset;

				for (uint i=0; i < PIXELS; ++i)
					dst[i] = offset + input.screen[i];
			}

			template<typename T>
			NST_FORCE_INLINE void Renderer::FilterNone::BlitUnaligned(const Input& input,const Output& output) const
			{
				const u16* NST_RESTRICT src = input.screen;
				T* NST_RESTRICT dst = static_cast<T*>(output.pixels);

				const long pitch = output.pitch;

				for (uint y=0; y < HEIGHT; ++y)
				{
					for (uint x=0; x < WIDTH; ++x)
						dst[x] = input.palette[src[x]];

					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
					src += WIDTH;
				}
			}

			template<>
			NST_FORCE_INLINE void Renderer::FilterNone::BlitUnaligned<u8>(const Input& input,const Output& output) const
			{
				const u16* NST_RESTRICT src = input.screen;
				u8* NST_RESTRICT dst = static_cast<u8*>(output.pixels);

				const long pitch = output.pitch;
				const uint offset = paletteOffset;

				for (uint y=0; y < HEIGHT; ++y)
				{
					for (uint x=0; x < WIDTH; ++x)
						dst[x] = offset + src[x];

					dst += pitch;
					src += WIDTH;
				}
			}

			template<typename T>
			NST_FORCE_INLINE void Renderer::FilterNone::BlitType(const Input& input,const Output& output) const
			{
				if (output.pitch == WIDTH * sizeof(T))
					BlitAligned<T>( input, output );
				else
					BlitUnaligned<T>( input, output );
			}

			void Renderer::FilterNone::Blit(const Input& input,const Output& output,uint)
			{
				switch (bpp)
				{
					case 32: BlitType< u32 >( input, output ); break;
					case 16: BlitType< u16 >( input, output ); break;
					case  8: BlitType< u8  >( input, output ); break;

					NST_UNREACHABLE
				}
			}
		}
	}
}
