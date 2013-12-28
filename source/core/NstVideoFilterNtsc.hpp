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

#ifndef NST_VIDEO_FILTER_NTSC_H
#define NST_VIDEO_FILTER_NTSC_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <cmath>
#include "NstFpuPrecision.hpp"
#include "../snes_ntsc/snes_ntsc.h"

#define SNES_NTSC_IN_FORMAT( n ) (ntsc_rgb_t*) (ktable + n * (snes_ntsc_entry_size / 2 * sizeof (ntsc_rgb_t)))

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4127 )
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			struct FilterNtscState
			{
				const Api::Video::RenderState& renderState;
				const i8 sharpness;
				const i8 resolution;
				const i8 bleed;
				const i8 artifacts;
				const i8 fringing;
				const bool fieldMerging;

				FilterNtscState
				(
					const Api::Video::RenderState& r,
					i8 s,
					i8 e,
					i8 b,
					i8 a,
					i8 f,
					bool m
				)
				:
				renderState  (r),
				sharpness    (s),
				resolution   (e),
				bleed        (b),
				artifacts    (a),
				fringing     (f),
				fieldMerging (m)
				{}
			};

			template<uint BITS>
			class Renderer::FilterNtsc : public Renderer::Filter
			{
				enum
				{
					BPP = BITS,
					NTSC_WIDTH = 602,
					NTSC_HEIGHT = HEIGHT * 2,
					R_MASK = BPP == 32 ? 0xFF0000 : BPP == 16 ? 0xF800 : 0x7C00,
					G_MASK = BPP == 32 ? 0x00FF00 : BPP == 16 ? 0x07E0 : 0x03E0,
					B_MASK = BPP == 32 ? 0x0000FF : BPP == 16 ? 0x001F : 0x001F,
					RB_MASK = R_MASK|B_MASK,
					S_SHIFT = BPP == 32 ? 8 : 5
				};

				void Blit(const Input&,const Output&,uint);
				void Transform(const u8 (&)[PALETTE][3],u32 (&)[PALETTE]) const;

				struct Lut : snes_ntsc_t
				{
					const uint noFieldMerging;

					Lut(const FilterNtscState& state)
					: noFieldMerging(state.fieldMerging ? 0U : ~0U)
					{
						FpuPrecision precision;

						snes_ntsc_setup_t setup;

						setup.hue = 0.0;
						setup.saturation = 0.0;
						setup.contrast = 0.0;
						setup.brightness = 0.0;
						setup.sharpness = state.sharpness / 100.0;
						setup.gamma = 0.0;
						setup.resolution = state.resolution / 100.0;
						setup.artifacts = state.artifacts / 100.0;
						setup.fringing = state.fringing / 100.0;
						setup.bleed = state.bleed / 100.0;
						setup.hue_warping = 0.0;
						setup.merge_fields = state.fieldMerging;
						setup.decoder_matrix = NULL;
						setup.bsnes_colortbl = NULL;

						::snes_ntsc_init( this, &setup );
					}
				};

				const Lut lut;
				const uint scanlines;

			public:

				FilterNtsc(const FilterNtscState&);

				static bool Check(const RenderState&);
			};

			template<uint BITS>
			Renderer::FilterNtsc<BITS>::FilterNtsc(const FilterNtscState& state)
			:
			Filter    ( state.renderState ),
			lut       ( state ),
			scanlines ( (100-state.renderState.scanlines) * (BPP == 32 ? 256 : 32) / 100 )
			{
			}

			template<uint BITS>
			bool Renderer::FilterNtsc<BITS>::Check(const RenderState& state)
			{
				return
				(
					( state.bits.count  == (BPP == 32 ? 32 : 16) ) &&
					( state.bits.mask.r == R_MASK                ) &&
					( state.bits.mask.g == G_MASK                ) &&
					( state.bits.mask.b == B_MASK                ) &&
					( state.width       == NTSC_WIDTH            ) &&
					( state.height      == NTSC_HEIGHT           ) &&
					( state.scanlines   <= 100                   )
				);
			}

			template<uint BITS>
			void Renderer::FilterNtsc<BITS>::Transform(const u8 (&src)[PALETTE][3],u32 (&dst)[PALETTE]) const
			{
				for (uint i=0; i < PALETTE; ++i)
				{
					dst[i] =
					(
						((src[i][0] >> 4) << 10) |
						((src[i][1] >> 3) <<  5) |
						((src[i][2] >> 4) <<  1)
					);
				}
			}

			template<uint BITS>
			void Renderer::FilterNtsc<BITS>::Blit(const Input& input,const Output& output,uint phase)
			{
				NST_ASSERT( phase < 3 );

				typedef typename OutPixel<BPP>::Type Pixel;

				Pixel buffer[NTSC_WIDTH];

				const u16* NST_RESTRICT src = input.pixels;
				Pixel* NST_RESTRICT dst = static_cast<Pixel*>(output.pixels);
				const long pad = output.pitch - NTSC_WIDTH * sizeof(Pixel);

				phase &= lut.noFieldMerging;

				for (uint y=0; y < HEIGHT; ++y)
				{
					SNES_NTSC_LORES_ROW( &lut, phase, 0, 0, input.palette[*src++] );

					Pixel* NST_RESTRICT cache = buffer;

					for (uint x=0; x < NTSC_WIDTH/7-1; ++x)
					{
						SNES_NTSC_PIXEL_IN( 0, input.palette[src[0]] );
						SNES_NTSC_LORES_OUT( 0, dst[0]=cache[0], BPP );
						SNES_NTSC_LORES_OUT( 1, dst[1]=cache[1], BPP );

						SNES_NTSC_PIXEL_IN( 1, input.palette[src[1]] );
						SNES_NTSC_LORES_OUT( 2, dst[2]=cache[2], BPP );
						SNES_NTSC_LORES_OUT( 3, dst[3]=cache[3], BPP );

						SNES_NTSC_PIXEL_IN( 2, input.palette[src[2]] );
						SNES_NTSC_LORES_OUT( 4, dst[4]=cache[4], BPP );
						SNES_NTSC_LORES_OUT( 5, dst[5]=cache[5], BPP );
						SNES_NTSC_LORES_OUT( 6, dst[6]=cache[6], BPP );

						src += 3;
						dst += 7;
						cache += 7;
					}

					SNES_NTSC_PIXEL_IN( 0, 0 );
					SNES_NTSC_LORES_OUT( 0, dst[0]=cache[0], BPP );
					SNES_NTSC_LORES_OUT( 1, dst[1]=cache[1], BPP );

					SNES_NTSC_PIXEL_IN( 1, 0 );
					SNES_NTSC_LORES_OUT( 2, dst[2]=cache[2], BPP );
					SNES_NTSC_LORES_OUT( 3, dst[3]=cache[3], BPP );

					SNES_NTSC_PIXEL_IN( 2, 0 );
					SNES_NTSC_LORES_OUT( 4, dst[4]=cache[4], BPP );
					SNES_NTSC_LORES_OUT( 5, dst[5]=cache[5], BPP );
					SNES_NTSC_LORES_OUT( 6, dst[6]=cache[6], BPP );

					dst = reinterpret_cast<Pixel*>(reinterpret_cast<u8*>(dst) + pad + 7 * sizeof(Pixel));
					cache = buffer;

					for (uint x=0, scale=scanlines; x < NTSC_WIDTH; ++x, ++cache)
						*dst++ = (scale * (*cache & G_MASK) >> S_SHIFT & G_MASK) | (scale * (*cache & RB_MASK) >> S_SHIFT & RB_MASK);

					dst = reinterpret_cast<Pixel*>(reinterpret_cast<u8*>(dst) + pad);
					phase = (phase + 1) % 3;
				}
			}
		}
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
