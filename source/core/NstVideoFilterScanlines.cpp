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

#include "NstCore.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterScanlines.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			void Renderer::FilterScanlines::Blit(const Input& input,const Output& output,uint phase)
			{
				(*this.*path)( input, output, phase );
			}

			template<typename T>
			void Renderer::FilterScanlines::Blit2x(const Input& input,const Output& output,uint) const
			{
				const Input::Pixel* NST_RESTRICT src = input.pixels;
				T* NST_RESTRICT dst = static_cast<T*>(output.pixels);

				const long pad = output.pitch - WIDTH*2 * sizeof(T);

				for (uint prefetched=src[0], y=HEIGHT, x=WIDTH; y; --y)
				{
					do
					{
						const dword reg = input.palette[prefetched];
						prefetched = *(++src);
						dst[0] = reg;
						dst[1] = reg;
						dst += 2;
					}
					while (--x);

					src -= WIDTH;
					prefetched = src[0];

					dst = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst) + pad);

					x = WIDTH;

					const uint s = scanlines, h = rgbShift;
					const dword g = gMask, rb = rbMask;

					do
					{
						const dword reg = (s * (input.palette[prefetched] & g) >> h & g) | (s * (input.palette[prefetched] & rb) >> h & rb);
						prefetched = *(++src);
						dst[0] = reg;
						dst[1] = reg;
						dst += 2;
					}
					while (--x);

					dst = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst) + pad);
					x   = WIDTH;
				}
			}

			template<typename T>
			void Renderer::FilterScanlines::Blit1x(const Input& input,const Output& output,uint) const
			{
				const Input::Pixel* NST_RESTRICT src = input.pixels;
				T* NST_RESTRICT dst = static_cast<T*>(output.pixels);

				const long pad = output.pitch - WIDTH * sizeof(T);

				for (uint prefetched=src[0], y=HEIGHT/2, x=WIDTH; y; --y)
				{
					do
					{
						const dword reg = input.palette[prefetched];
						prefetched = *(++src);
						*dst++ = reg;
					}
					while (--x);

					dst = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst) + pad);
					x = WIDTH;

					const uint s = scanlines, h = rgbShift;
					const dword g = gMask, rb = rbMask;

					do
					{
						const dword reg = (s * (input.palette[prefetched] & g) >> h & g) | (s * (input.palette[prefetched] & rb) >> h & rb);
						prefetched = *(++src);
						*dst++ = reg;
					}
					while (--x);

					dst = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst) + pad);
					x = WIDTH;
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Renderer::FilterScanlines::Path Renderer::FilterScanlines::GetPath(const RenderState& state)
			{
				if (state.width != WIDTH)
				{
					if (state.bits.count == 32)
						return &FilterScanlines::Blit2x<dword>;
					else
						return &FilterScanlines::Blit2x<word>;
				}
				else
				{
					if (state.bits.count == 32)
						return &FilterScanlines::Blit1x<dword>;
					else
						return &FilterScanlines::Blit1x<word>;
				}
			}

			Renderer::FilterScanlines::FilterScanlines(const RenderState& state)
			:
			Filter    ( state ),
			path      ( GetPath(state) ),
			scanlines ( (100-state.scanlines) * (state.bits.count == 32 ? 256 : 32) / 100 ),
			gMask     ( state.bits.mask.g ),
			rbMask    ( state.bits.mask.r|state.bits.mask.b ),
			rgbShift  ( state.bits.count == 32 ? 8 : 5 )
			{
				NST_COMPILE_ASSERT( Video::Screen::PIXELS_PADDING >= 1 );
			}

			bool Renderer::FilterScanlines::Check(const RenderState& state)
			{
				return
				(
					(state.bits.count == 16 || state.bits.count == 32) &&
					(state.width == WIDTH || state.width == WIDTH*2) &&
					(state.height == HEIGHT || state.height == HEIGHT*2) &&
					(state.scanlines <= 100)
				);
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif
		}
	}
}
