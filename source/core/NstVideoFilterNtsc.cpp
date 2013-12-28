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

#include "NstAssert.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterNtsc.hpp"
#include "NstFpuPrecision.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			void Renderer::FilterNtsc::Blit(const Input& input,const Output& output,uint phase)
			{
				(*this.*path)( input, output, phase );
			}

			template<typename Pixel,uint BITS>
			void Renderer::FilterNtsc::BlitType(const Input& input,const Output& output,uint phase) const
			{
				enum
				{
					BPP = BITS,
					R_MASK = BPP == 32 ? 0xFF0000 : BPP == 16 ? 0xF800 : 0x7C00,
					G_MASK = BPP == 32 ? 0x00FF00 : BPP == 16 ? 0x07E0 : 0x03E0,
					B_MASK = BPP == 32 ? 0x0000FF : BPP == 16 ? 0x001F : 0x001F,
					RB_MASK = R_MASK|B_MASK,
					S_SHIFT = BPP == 32 ? 8 : 5
				};

				NST_ASSERT( phase < 3 );

				Pixel buffer[NTSC_WIDTH+1];

				const Input::Pixel* NST_RESTRICT src = input.pixels;
				Pixel* NST_RESTRICT dst = static_cast<Pixel*>(output.pixels);
				const long pad = output.pitch - NTSC_WIDTH * sizeof(Pixel);

				phase &= lut.noFieldMerging;

				for (uint y=HEIGHT; y; --y)
				{
					NES_NTSC_BEGIN_ROW( &lut, phase, lut.black, lut.black, *src++ );

					Pixel* NST_RESTRICT cache = buffer;

					for (const Input::Pixel* const end=src+(NTSC_WIDTH/7*3-3); src != end; src += 3, dst += 7, cache += 7)
					{
						NES_NTSC_COLOR_IN( 0, src[0] );
						NES_NTSC_RGB_OUT( 0, cache[0], BPP ); dst[0] = cache[0];
						NES_NTSC_RGB_OUT( 1, cache[1], BPP ); dst[1] = cache[1];

						NES_NTSC_COLOR_IN( 1, src[1] );
						NES_NTSC_RGB_OUT( 2, cache[2], BPP ); dst[2] = cache[2];
						NES_NTSC_RGB_OUT( 3, cache[3], BPP ); dst[3] = cache[3];

						NES_NTSC_COLOR_IN( 2, src[2] );
						NES_NTSC_RGB_OUT( 4, cache[4], BPP ); dst[4] = cache[4];
						NES_NTSC_RGB_OUT( 5, cache[5], BPP ); dst[5] = cache[5];
						NES_NTSC_RGB_OUT( 6, cache[6], BPP ); dst[6] = cache[6];
					}

					NES_NTSC_COLOR_IN( 0, lut.black );
					NES_NTSC_RGB_OUT( 0, cache[0], BPP ); dst[0] = cache[0];
					NES_NTSC_RGB_OUT( 1, cache[1], BPP ); dst[1] = cache[1];

					NES_NTSC_COLOR_IN( 1, lut.black );
					NES_NTSC_RGB_OUT( 2, cache[2], BPP ); dst[2] = cache[2];
					NES_NTSC_RGB_OUT( 3, cache[3], BPP ); dst[3] = cache[3];

					NES_NTSC_COLOR_IN( 2, lut.black );
					NES_NTSC_RGB_OUT( 4, cache[4], BPP ); dst[4] = cache[4];
					NES_NTSC_RGB_OUT( 5, cache[5], BPP ); dst[5] = cache[5];
					NES_NTSC_RGB_OUT( 6, cache[6], BPP ); dst[6] = cache[6];

					dst = reinterpret_cast<Pixel*>(reinterpret_cast<byte*>(dst) + pad + 7 * sizeof(Pixel));
					cache = buffer;

					for (dword reg=buffer[0], x=NTSC_WIDTH, scale=scanlines; x; --x)
					{
						const dword prefetched = *(++cache);
						*dst++ = (scale * (reg & dword(G_MASK)) >> S_SHIFT & G_MASK) | (scale * (reg & dword(RB_MASK)) >> S_SHIFT & RB_MASK);
						reg = prefetched;
					}

					dst = reinterpret_cast<Pixel*>(reinterpret_cast<byte*>(dst) + pad);
					phase = (phase + 1) % 3;
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			inline uint Renderer::FilterNtsc::Lut::GetBlack(const byte (&p)[PALETTE][3])
			{
				uint index = DEF_BLACK;

				for (uint i=0, intensity = 0xFF * 100; i < 64; ++i)
				{
					const uint v = p[i][0] * 30 + p[i][1] * 59 + p[i][2] * 11;

					if (intensity > v)
					{
						intensity = v;
						index = i;
					}
				}

				return index;
			}

			Renderer::FilterNtsc::Lut::Lut
			(
				const byte (&palette)[PALETTE][3],
				const schar sharpness,
				const schar resolution,
				const schar bleed,
				const schar artifacts,
				const schar fringing,
				const bool fieldMerging
			)
			:
			noFieldMerging (fieldMerging ? 0U : ~0U),
			black          (GetBlack(palette))
			{
				FpuPrecision precision;

				nes_ntsc_setup_t setup;

				setup.hue = 0;
				setup.saturation = 0;
				setup.contrast = 0;
				setup.brightness = 0;
				setup.sharpness = sharpness / 100.0;
				setup.gamma = 0;
				setup.resolution = resolution / 100.0;
				setup.artifacts = artifacts / 100.0;
				setup.fringing = fringing / 100.0;
				setup.bleed = bleed / 100.0;
				setup.merge_fields = fieldMerging;
				setup.decoder_matrix = NULL;
				setup.palette_out = NULL;
				setup.palette = *palette;
				setup.base_palette = NULL;

				::nes_ntsc_init( this, &setup );
			}

			bool Renderer::FilterNtsc::Check(const RenderState& state)
			{
				return (state.width == NTSC_WIDTH && state.height == NTSC_HEIGHT && state.scanlines <= 100) &&
				(
					(state.bits.count == 16 && state.bits.mask.b == 0x001F && ((state.bits.mask.g == 0x07E0 && state.bits.mask.r == 0xF800) || (state.bits.mask.g == 0x03E0 && state.bits.mask.r == 0x7C00))) ||
					(state.bits.count == 32 && state.bits.mask.r == 0xFF0000 && state.bits.mask.g == 0x00FF00 && state.bits.mask.b == 0x0000FF)
				);
			}

			Renderer::FilterNtsc::Path Renderer::FilterNtsc::GetPath(const RenderState& state)
			{
				if (state.bits.count == 32)
				{
					return &FilterNtsc::BlitType<dword,32>;
				}
				else if (state.bits.mask.g == 0x07E0)
				{
					return &FilterNtsc::BlitType<word,16>;
				}
				else
				{
					return &FilterNtsc::BlitType<word,15>;
				}
			}

			Renderer::FilterNtsc::FilterNtsc
			(
				const RenderState& state,
				const byte (&palette)[PALETTE][3],
				schar sharpness,
				schar resolution,
				schar bleed,
				schar artifacts,
				schar fringing,
				bool fieldMerging
			)
			:
			Filter    (state),
			path      (GetPath(state)),
			lut       (palette,sharpness,resolution,bleed,artifacts,fringing,fieldMerging),
			scanlines ((100-state.scanlines) * (state.bits.count == 32 ? 256 : 32) / 100)
			{
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif
		}
	}
}
