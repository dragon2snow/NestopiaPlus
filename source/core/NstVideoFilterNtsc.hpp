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

#ifndef NST_VIDEO_FILTER_NTSC_H
#define NST_VIDEO_FILTER_NTSC_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <cmath>
#include "../nes_ntsc/nes_ntsc.h"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
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

				bool CanTransform() const
				{
					return false;
				}

				struct Lut : nes_ntsc_emph_t
				{
					const uint noFieldMerging;

					Lut(int brightness,int saturation,int hue,int contrast,int sharpness,bool mergeFields,const Api::Video::Decoder& decoder)
					: noFieldMerging(mergeFields ? 0U : ~0U)
					{
						const float matrix[6] =
						{
							std::sin( decoder.axes[0].angle * NST_DEG ) * decoder.axes[0].gain * 2,
							std::cos( decoder.axes[0].angle * NST_DEG ) * decoder.axes[0].gain * 2,
							std::sin( decoder.axes[1].angle * NST_DEG ) * decoder.axes[1].gain * 2,
							std::cos( decoder.axes[1].angle * NST_DEG ) * decoder.axes[1].gain * 2,
							std::sin( decoder.axes[2].angle * NST_DEG ) * decoder.axes[2].gain * 2,
							std::cos( decoder.axes[2].angle * NST_DEG ) * decoder.axes[2].gain * 2
						};

						nes_ntsc_setup_t setup;

						setup.hue = 33.0f/360.f + (hue - 128) / 768.f;
						setup.saturation = (saturation - 128) / 128.f;
						setup.contrast = (contrast - 128) / 128.f;
						setup.brightness = (brightness - 128) / 128.f;
						setup.sharpness = (sharpness - 128) / 128.f;
						setup.hue_warping = 0.0f;
						setup.merge_fields = mergeFields;
						setup.decoder_matrix = matrix;
						setup.palette_out = NULL;
					
						::nes_ntsc_init_emph( this, &setup );
					}
				};

				const Lut lut;
				const uint scanlines;

			public:

				FilterNtsc(const RenderState&,uint,uint,uint,uint,uint,bool,const Api::Video::Decoder&);

				static bool Check(const RenderState&);
			};

			template<uint BITS>
			Renderer::FilterNtsc<BITS>::FilterNtsc
			(
		       	const RenderState& state,
				uint brightness,
				uint saturation,
				uint hue,
				uint contrast,
				uint sharpness,
				bool fieldMerging,
				const Api::Video::Decoder& decoder
			)
			: 
			Filter    ( state ),
			lut       ( brightness, saturation, hue, contrast, sharpness, fieldMerging, decoder ),
			scanlines ( (100-state.scanlines) * (BPP == 32 ? 256 : 32) / 100 )
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
			void Renderer::FilterNtsc<BITS>::Blit(const Input& input,const Output& output,uint phase)
			{
				NST_ASSERT( phase < 3 );

				typedef typename OutPixel<BPP>::Type Pixel;

				Pixel buffer[NTSC_WIDTH];

				const u16* NST_RESTRICT src = input.screen;
				Pixel* NST_RESTRICT dst = static_cast<Pixel*>(output.pixels);
				const long pad = output.pitch - NTSC_WIDTH * sizeof(Pixel);

				phase &= lut.noFieldMerging;

				for (uint y=0; y < HEIGHT; ++y)
				{
					NES_NTSC_BEGIN_ROW( &lut, phase, *src++ );
					
					Pixel* NST_RESTRICT cache = buffer;

					for (uint x=0; x < NTSC_WIDTH/7-1; ++x)
					{
						NES_NTSC_COLOR_IN( 0, src[0] );
						NES_NTSC_RGB_OUT( 0, dst[0]=cache[0], BPP );
						NES_NTSC_RGB_OUT( 1, dst[1]=cache[1], BPP );

						NES_NTSC_COLOR_IN( 1, src[1] );
						NES_NTSC_RGB_OUT( 2, dst[2]=cache[2], BPP );
						NES_NTSC_RGB_OUT( 3, dst[3]=cache[3], BPP );

						NES_NTSC_COLOR_IN( 2, src[2] );
						NES_NTSC_RGB_OUT( 4, dst[4]=cache[4], BPP );
						NES_NTSC_RGB_OUT( 5, dst[5]=cache[5], BPP );
						NES_NTSC_RGB_OUT( 6, dst[6]=cache[6], BPP );

						src += 3;
						dst += 7;
						cache += 7;
					}

					NES_NTSC_COLOR_IN( 0, 0xF );
					NES_NTSC_RGB_OUT( 0, dst[0]=cache[0], BPP );
					NES_NTSC_RGB_OUT( 1, dst[1]=cache[1], BPP );

					NES_NTSC_COLOR_IN( 1, 0xF );
					NES_NTSC_RGB_OUT( 2, dst[2]=cache[2], BPP );
					NES_NTSC_RGB_OUT( 3, dst[3]=cache[3], BPP );

					NES_NTSC_COLOR_IN( 2, 0xF );
					NES_NTSC_RGB_OUT( 4, dst[4]=cache[4], BPP );
					NES_NTSC_RGB_OUT( 5, dst[5]=cache[5], BPP );
					NES_NTSC_RGB_OUT( 6, dst[6]=cache[6], BPP );

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

#endif

