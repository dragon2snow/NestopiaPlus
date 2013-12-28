////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
// Copyright (C) 2006 Shay Green
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
#include <cstring>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4293 )
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class BaseFilterNtscLut
			{
			public:

				enum
				{
					PHASE_COUNT = 3,
					PALETTE_SIZE = 64,
					FIXED_MUL = 0x400,
					FIXED_SHIFT = 10
				};

			protected:

				static void Build
				(
			     	i32 (&)[PHASE_COUNT][PALETTE_SIZE][4],
					i32 (&)[PHASE_COUNT][6],
					u8* NST_RESTRICT,
					int,
					int,
					uint,
					int,
					const Api::Video::Decoder&
				);
			};

			template<uint BITS>
			class Renderer::FilterNtsc : public Renderer::Filter
			{					
				enum
				{
					BPP = BITS,
					R_SHIFT = (BPP == 32 ? 16 : BPP == 16 ? 11 : 10),
					G_SHIFT = (BPP == 32 ?  8 : BPP == 16 ?  6 :  5),
					B_SHIFT = 0,
                #ifdef NST_BIG_ENDIAN
					R_SHIFT_0 = R_SHIFT + 16,
					G_SHIFT_0 = G_SHIFT + 16,
					B_SHIFT_0 = B_SHIFT + 16,
					R_SHIFT_16 = R_SHIFT,
					G_SHIFT_16 = G_SHIFT,
					B_SHIFT_16 = B_SHIFT,
                #else
					R_SHIFT_0 = R_SHIFT,
					G_SHIFT_0 = G_SHIFT,
					B_SHIFT_0 = B_SHIFT,
					R_SHIFT_16 = R_SHIFT + 16,
					G_SHIFT_16 = G_SHIFT + 16,
					B_SHIFT_16 = B_SHIFT + 16,
                #endif
					R_MASK = (BPP == 32 ? 0xFF0000 : BPP == 16 ? 0xF800 : 0x7C00),
					G_MASK = (BPP == 32 ? 0x00FF00 : BPP == 16 ? 0x07E0 : 0x03E0),
					B_MASK = (BPP == 32 ? 0x0000FF : BPP == 16 ? 0x001F : 0x001F),
					NTSC_WIDTH = 680,
					NTSC_HEIGHT = 480,
					PALETTE_SIZE = BaseFilterNtscLut::PALETTE_SIZE,
					COMPOSITE_PADDING = 16,
					COMPOSITE_SIZE = 736 * 2 + COMPOSITE_PADDING * 2,
					FIXED_MUL = BaseFilterNtscLut::FIXED_MUL,
					FIXED_SHIFT = BaseFilterNtscLut::FIXED_SHIFT,
					K2 = -292, /* -0.2857818688 * FIXED_MUL=0x400 */
					K0 = 146   /* (0.5 / 3.499666147) * FIXED_MUL=0x400 */
				};

				struct Lut : BaseFilterNtscLut
				{
					enum
					{
						COLOR_RANGE = (BPP == 32 ? 256 : 32)
					};

					i32 nesPhases[PHASE_COUNT][PALETTE_SIZE][4];
					i32 rgbPhases[PHASE_COUNT][6];
					u8 toInt[COLOR_RANGE * 5];

					Lut(uint brightness,uint saturation,uint hue,const Api::Video::Decoder& decoder)
					{
						BaseFilterNtscLut::Build
						( 
							nesPhases, 
							rgbPhases, 
							toInt, 
							COLOR_RANGE, 
							brightness, 
							saturation, 
							hue,
							decoder
						);
					}
				};

				struct Scanlines
				{
					enum
					{
						MASK_0 = (BPP == 32 ? 0x00FEFEFE : BPP == 16 ? 0xF7DEF7DE : 0x7BDE7BDE),
						MASK_1 = (BPP == 32 ? 0x00FCFCFC : BPP == 16 ? 0xE79CE79C : 0x739C739C)
					};

					const dword mask;
					const uint shifter;

					Scanlines(uint s)
					: mask(s == 1 ? MASK_0 : s == 2 ? MASK_1 : ~dword(0)), shifter(s) {}
				};

				i32 composite[COMPOSITE_SIZE];
				const Lut lut;
				const Scanlines scanlines;

				void Blit(const Input&,const Output&);

				bool CanTransform() const
				{
					return false;
				}

			public:

				FilterNtsc(const RenderState&,uint,uint,uint,uint,const Api::Video::Decoder&);

				static bool Check(const RenderState&);
			};

			template<uint BITS>
			Renderer::FilterNtsc<BITS>::FilterNtsc(const RenderState& state,uint lines,uint brightness,uint saturation,uint hue,const Api::Video::Decoder& decoder)
			: 
			Filter    ( state ), 
			lut       ( brightness, saturation, hue, decoder ),
			scanlines ( lines )
			{
				std::memset( composite, 0, sizeof(composite) );
			}

			template<uint BITS>
			bool Renderer::FilterNtsc<BITS>::Check(const RenderState& state)
			{
				return
				(
					( state.bits.count  == BPP         ) &&
					( state.bits.mask.r == R_MASK      ) &&
					( state.bits.mask.g == G_MASK      ) &&
					( state.bits.mask.b == B_MASK      ) &&
					( state.width       == NTSC_WIDTH  ) &&
					( state.height      == NTSC_HEIGHT )
				);
			}

			template<uint BITS>
			void Renderer::FilterNtsc<BITS>::Blit(const Input& input,const Output& output)
			{
				static uint basePhase = 0;

				const u16* NST_RESTRICT src = input.screen;
				u32* NST_RESTRICT dst = static_cast<u32*>(output.pixels);

				const long pitch = output.pitch - NTSC_WIDTH * (BPP == 32 ? sizeof(u32) : sizeof(u16));

				for (uint i=0, phase=(basePhase^=1); i < HEIGHT; ++i)
				{
					{
						const i32 (&quads)[PALETTE_SIZE][4] = lut.nesPhases[phase];
						
						i32* NST_RESTRICT out = composite + COMPOSITE_PADDING;
						
						for (uint j=0; j < NTSC_WIDTH/8; ++j)
						{
							const i32* NST_RESTRICT quad = quads[src[0] & (PALETTE_SIZE-1)];
							out[0] = quad[0];
							out[2] = quad[1];
							i32 next = quad[2];
							quad = quads[src[1] & (PALETTE_SIZE-1)];
							out[4] = (next + next + quad[2]) / 3;
							out[6] = quad[3];
							out[8] = quad[0];							
							next = quad[1];
							quad = quads[src[2] & (PALETTE_SIZE-1)];
							out[10] = (next + quad[1] + quad[1]) / 3;
							out[12] = quad[2];
							out[14] = quad[3];
							src += 3;
							out += 16;
						}

						src += WIDTH - NTSC_WIDTH/8*3;
					}
			
					{
						for (uint j=0; j <= 2; j += 2)
						{
							i32* NST_RESTRICT p = composite + COMPOSITE_PADDING + j;

							i32 p0 = p[4];
							i32 p1 = p[0];
							i32 p2 = 0;
							i32 p3 = p2;

							for (uint k=0; k < NTSC_WIDTH/8; ++k)
							{
								p1 = ((((p1 + p0) * K0) + (p3 * K2)) >> FIXED_SHIFT) + p2;
								p0 = ((((p0 + (p3=p[8])) * K0) + (p2 * K2)) >> FIXED_SHIFT) + p1;
								p3 = ((((p3 + (p2=p[12])) * K0) + (p1 * K2)) >> FIXED_SHIFT) + p0;
								p[13] = p2 = ((((p2 + (p[1]=p1, p1=p[16])) * K0) + ((p[5]=p0) * K2)) >> FIXED_SHIFT) + (p[9]=p3);
								p0 = p[20];
								p += 16;
							}
						} 
					}

					const i32 (&toRgb)[6] = lut.rgbPhases[phase];
					const i32* NST_RESTRICT in = composite + COMPOSITE_PADDING;

					u32 buffer[BPP == 32 ? NTSC_WIDTH : NTSC_WIDTH/2];
					u32* NST_RESTRICT cache = buffer;

					for (uint j=0; j < NTSC_WIDTH/4; ++j)
					{
						if (BPP == 32)
						{
							dst[0] = cache[0] =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[0] * in[1]) >> FIXED_SHIFT) + (in[0] - in[1]) + ((toRgb[1] * (in[-1] + in[3])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << R_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[2] * in[1]) >> FIXED_SHIFT) + (in[0] - in[1]) + ((toRgb[3] * (in[-1] + in[3])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << G_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[4] * in[1]) >> FIXED_SHIFT) + (in[0] - in[1]) + ((toRgb[5] * (in[-1] + in[3])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << B_SHIFT )
							);

							dst[1] = cache[1] =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[0] * (in[1] + in[5])) >> (1+FIXED_SHIFT)) + (in[2] - in[3]) + ((toRgb[1] * in[3]) >> FIXED_SHIFT)) >> FIXED_SHIFT)] << R_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[2] * (in[1] + in[5])) >> (1+FIXED_SHIFT)) + (in[2] - in[3]) + ((toRgb[3] * in[3]) >> FIXED_SHIFT)) >> FIXED_SHIFT)] << G_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[4] * (in[1] + in[5])) >> (1+FIXED_SHIFT)) + (in[2] - in[3]) + ((toRgb[5] * in[3]) >> FIXED_SHIFT)) >> FIXED_SHIFT)] << B_SHIFT )
							);

							dst[2] = cache[2] =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[0] * in[5]) >> FIXED_SHIFT) + (in[5] - in[4]) + ((toRgb[1] * (in[3] + in[7])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << R_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[2] * in[5]) >> FIXED_SHIFT) + (in[5] - in[4]) + ((toRgb[3] * (in[3] + in[7])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << G_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[4] * in[5]) >> FIXED_SHIFT) + (in[5] - in[4]) + ((toRgb[5] * (in[3] + in[7])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << B_SHIFT )
							);

							dst[3] = cache[3] =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[0] * (in[5] + in[9])) >> (1+FIXED_SHIFT)) + (in[7] - in[6]) + ((toRgb[1] * in[7]) >> FIXED_SHIFT)) >> FIXED_SHIFT)] << R_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[2] * (in[5] + in[9])) >> (1+FIXED_SHIFT)) + (in[7] - in[6]) + ((toRgb[3] * in[7]) >> FIXED_SHIFT)) >> FIXED_SHIFT)] << G_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[4] * (in[5] + in[9])) >> (1+FIXED_SHIFT)) + (in[7] - in[6]) + ((toRgb[5] * in[7]) >> FIXED_SHIFT)) >> FIXED_SHIFT)] << B_SHIFT )
							);

							dst += 4;
							cache += 4;
						}
						else
						{
							dst[0] = cache[0] =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[0] * in[1]) >> FIXED_SHIFT) + (in[0] - in[1]) + ((toRgb[1] * (in[-1] + in[3])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << R_SHIFT_0  ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[2] * in[1]) >> FIXED_SHIFT) + (in[0] - in[1]) + ((toRgb[3] * (in[-1] + in[3])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << G_SHIFT_0  ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[4] * in[1]) >> FIXED_SHIFT) + (in[0] - in[1]) + ((toRgb[5] * (in[-1] + in[3])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << B_SHIFT_0  ) |								
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[0] * (in[1] + in[5])) >> (1+FIXED_SHIFT)) + (in[2] - in[3]) + ((toRgb[1] * in[3]) >> FIXED_SHIFT)) >> FIXED_SHIFT)]  << R_SHIFT_16 ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[2] * (in[1] + in[5])) >> (1+FIXED_SHIFT)) + (in[2] - in[3]) + ((toRgb[3] * in[3]) >> FIXED_SHIFT)) >> FIXED_SHIFT)]  << G_SHIFT_16 ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[4] * (in[1] + in[5])) >> (1+FIXED_SHIFT)) + (in[2] - in[3]) + ((toRgb[5] * in[3]) >> FIXED_SHIFT)) >> FIXED_SHIFT)]  << B_SHIFT_16 )
							);

							dst[1] = cache[1] =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[0] * in[5]) >> FIXED_SHIFT) + (in[5] - in[4]) + ((toRgb[1] * (in[3] + in[7])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << R_SHIFT_0  ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[2] * in[5]) >> FIXED_SHIFT) + (in[5] - in[4]) + ((toRgb[3] * (in[3] + in[7])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << G_SHIFT_0  ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[4] * in[5]) >> FIXED_SHIFT) + (in[5] - in[4]) + ((toRgb[5] * (in[3] + in[7])) >> (1+FIXED_SHIFT))) >> FIXED_SHIFT)] << B_SHIFT_0  ) |								
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[0] * (in[5] + in[9])) >> (1+FIXED_SHIFT)) + (in[7] - in[6]) + ((toRgb[1] * in[7]) >> FIXED_SHIFT)) >> FIXED_SHIFT)] << R_SHIFT_16 ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[2] * (in[5] + in[9])) >> (1+FIXED_SHIFT)) + (in[7] - in[6]) + ((toRgb[3] * in[7]) >> FIXED_SHIFT)) >> FIXED_SHIFT)] << G_SHIFT_16 ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + ((((toRgb[4] * (in[5] + in[9])) >> (1+FIXED_SHIFT)) + (in[7] - in[6]) + ((toRgb[5] * in[7]) >> FIXED_SHIFT)) >> FIXED_SHIFT)] << B_SHIFT_16 )
							);

							dst += 2;
							cache += 2;
						}

						in += 8;
					}
			
					dst = reinterpret_cast<u32*>(reinterpret_cast<u8*>(dst) + pitch);
					cache = buffer;

					const Scanlines scanline( scanlines );

					for (uint j=0; j < NST_COUNT(buffer); ++j)
						*dst++ = (*cache++ & scanline.mask) >> scanline.shifter;

					dst = reinterpret_cast<u32*>(reinterpret_cast<u8*>(dst) + pitch);
					phase = (phase + 1) < 3 ? (phase + 1) : 0;
				}
			}
     	}
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
