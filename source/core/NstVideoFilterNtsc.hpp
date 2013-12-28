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
					NTSC_WIDTH = 640,
					NTSC_HEIGHT = 480,
					PALETTE_SIZE = 64,
					COMPOSITE_PADDING = 16,
					COMPOSITE_SIZE = 736 * 2 + COMPOSITE_PADDING * 2
				};

				struct Lut
				{
					Lut(double,double,double,double);

					enum
					{
						COLOR_RANGE = (BPP == 32 ? 256 : 32),
						COLOR_MAX = COLOR_RANGE-1,
						PHASE_COUNT = 3
					};

					float nesPhases[PHASE_COUNT][PALETTE_SIZE][4];
					float rgbPhases[PHASE_COUNT][6];
					u8 toInt[COLOR_RANGE * 5];
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

				float composite[COMPOSITE_SIZE];
				const Lut lut;
				const Scanlines scanlines;

			public:

				FilterNtsc(const RenderState&,uint,double=0.0,double=0.0,double=0.0,double=0.0);

				void Blit(const Input&,const Output&);

				static bool Check(const RenderState&);
			};

			template<uint BITS>
			Renderer::FilterNtsc<BITS>::FilterNtsc
			(
		    	const RenderState& state,
				uint lines,
				double hue,
				double brightness,
				double saturation,
				double contrast
			)
			: 
			Filter    ( state ), 
			lut       ( hue, brightness, saturation, contrast ),
			scanlines ( lines )
			{
				for (uint i=0; i < COMPOSITE_SIZE; ++i)
					composite[i] = 0.f;
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
			Renderer::FilterNtsc<BITS>::Lut::Lut(double hue,double brightness,double contrast,double saturation)
			{
				const int bias = brightness * COLOR_MAX;	
				contrast += 1.0;

				for (int i=0; i < NST_COUNT(toInt); ++i)
				{
					const int y = (int) ((i - COLOR_RANGE*2) * contrast) + bias;
					toInt[i] = NST_CLAMP(y,0,COLOR_MAX);
				} 

				if (saturation > 0.0)
					saturation *= 0.5;
					
				saturation += 1.0;

				for (uint phase=0; phase < PHASE_COUNT; ++phase)
				{
					{
						const double angle = NST_PI / -12 * int(phase * 8 + 1) + NST_PI * hue;
						const double s = std::sin( angle ) * saturation;
						const double c = std::cos( angle ) * saturation;

						for (uint i=0; i < 6; i += 2)
						{
							static const double rgb[6] = {0.956,0.621,-0.272,-0.647,-1.105,1.702};

							rgbPhases[phase][i+0] = rgb[i+0] * c - rgb[i+1] * s;
							rgbPhases[phase][i+1] = rgb[i+0] * s + rgb[i+1] * c;
						}
					}
					
					for (uint c=0; c < PALETTE_SIZE; ++c)
					{
						const uint ph = c & 0xF;
						double y = (c >> 4) * 0.375;

						if (ph == 0)
						{
							y += 0.5;
						}
						else if (ph >= 0xD)
						{
							y -= 0.5;

							if (ph > 0xD)
								y = 0.0;
						}

						const double gain = COLOR_RANGE - 0.000001;
					
						y = y * gain + 1.0 / 0x10000;
						
						const double angle = NST_PI / 6.0 * (int(phase) * 4 + int(ph) - 3);
						const double factor = (ph-1U < 0xCU) ? 0.5 * gain : 0.0;
						const double i = std::sin( angle ) * factor;
						const double q = std::cos( angle ) * factor;

						const double cmin = -0.2 * gain;
						const double cmax =  1.2 * gain;

						nesPhases[phase][c][0] =  NST_CLAMP(y + i,cmin,cmax);
						nesPhases[phase][c][1] =  NST_CLAMP(y + q,cmin,cmax);
						nesPhases[phase][c][2] = -NST_CLAMP(y - i,cmin,cmax);
						nesPhases[phase][c][3] = -NST_CLAMP(y - q,cmin,cmax);
					}	
				}
			}

			template<uint BITS>
			void Renderer::FilterNtsc<BITS>::Blit(const Input& input,const Output& output)
			{
				static uint basePhase = 0;

				const u16* NST_RESTRICT src = input.screen + (WIDTH-NTSC_WIDTH/8*3) / 2;
				
				u32* NST_RESTRICT dst[2] = 
				{
					static_cast<u32*>(output.pixels),
					reinterpret_cast<u32*>(static_cast<u8*>(output.pixels) + output.pitch)
				};
				
				const long pitch = output.pitch + output.pitch - NTSC_WIDTH * (BPP == 32 ? sizeof(u32) : sizeof(u16));

				for (uint i=0, phase=(basePhase^=1); i < HEIGHT; ++i)
				{
					{
						const float (&quads)[PALETTE_SIZE][4] = lut.nesPhases[phase^=1];
						float* NST_RESTRICT out = composite + COMPOSITE_PADDING;
												
						for (uint j=0; j < NTSC_WIDTH/8; ++j)
						{
							const float* NST_RESTRICT quad = quads[src[0] & (PALETTE_SIZE-1)];
							float t0 = quad[0];
							float t1 = quad[1];
							float next = quad[2];
							next += next;
							quad = quads[src[1] & (PALETTE_SIZE-1)];
							out[0] = t0;
							out[2] = t1;							
							t0 = quad[0];
							t1 = quad[3];
							next += quad[2];
							out[4] = next * (1.f/3.f);							
							next = quad[1];
							quad = quads[src[2] & (PALETTE_SIZE-1)];
							out[6] = t1;
							out[8] = t0;							
							t0 = quad[2];
							t1 = quad[3];
							next += quad[1];
							next += quad[1];
							out[10] = next * (1.f/3.f);
							out[12] = t0;
							out[14] = t1;
							src += 3;
							out += 16;
						}

						src += WIDTH - NTSC_WIDTH/8*3;
					}
			
					{
						const float k0 = float(0.5 / 3.499666147);
						const float k2 = -0.2857818688f;

						for (uint j=0; j <= 2; j += 2)
						{
							float* NST_RESTRICT p = composite + COMPOSITE_PADDING + j;

							float p0 = p[4];
							float p1 = p[0];
							float p2 = 0.f;
							float p3 = p2;

							for (uint k=0; k < NTSC_WIDTH/8; ++k)
							{
								p1 = ((p1 + p0) * k0) + (p3 * k2) + p2 + 0.000001f;
								p0 = ((p0 + (p3=p[8])) * k0) + (p2 * k2) + p1;
								p3 = ((p3 + (p2=p[12])) * k0) + (p1 * k2) + p0;
								p[13] = p2 = ((p2 + (p[1]=p1, p1=p[16])) * k0) + ((p[5]=p0) * k2) + (p[9]=p3);
								p0 = p[20];
								p += 16;
							}
						} 
					}

					const float (&toRgb)[6] = lut.rgbPhases[phase];
					const float* NST_RESTRICT in = composite + COMPOSITE_PADDING;

					for (uint j=0; j < NTSC_WIDTH/4; ++j)
					{
						if (BPP == 32)
						{
							u32 pixel;

							dst[0][0] = pixel =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[0] * in[1] + (in[0] - in[1]) + toRgb[1] * ((in[-1] + in[3]) * 0.5f))] << R_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[2] * in[1] + (in[0] - in[1]) + toRgb[3] * ((in[-1] + in[3]) * 0.5f))] << G_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[4] * in[1] + (in[0] - in[1]) + toRgb[5] * ((in[-1] + in[3]) * 0.5f))] << B_SHIFT )
							);

							dst[1][0] = (pixel & scanlines.mask) >> scanlines.shifter;

							dst[0][1] = pixel =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[0] * ((in[1] + in[5]) * 0.5f) + (in[2] - in[3]) + toRgb[1] * in[3])] << R_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[2] * ((in[1] + in[5]) * 0.5f) + (in[2] - in[3]) + toRgb[3] * in[3])] << G_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[4] * ((in[1] + in[5]) * 0.5f) + (in[2] - in[3]) + toRgb[5] * in[3])] << B_SHIFT )
							);

							dst[1][1] = (pixel & scanlines.mask) >> scanlines.shifter;

							dst[0][2] = pixel =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[0] * in[5] + (in[5] - in[4]) + toRgb[1] * ((in[3] + in[7]) * 0.5f))] << R_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[2] * in[5] + (in[5] - in[4]) + toRgb[3] * ((in[3] + in[7]) * 0.5f))] << G_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[4] * in[5] + (in[5] - in[4]) + toRgb[5] * ((in[3] + in[7]) * 0.5f))] << B_SHIFT )
							);

							dst[1][2] = (pixel & scanlines.mask) >> scanlines.shifter;

							dst[0][3] = pixel =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[0] * ((in[5] + in[9]) * 0.5f) + (in[7] - in[6]) + toRgb[1] * in[7])] << R_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[2] * ((in[5] + in[9]) * 0.5f) + (in[7] - in[6]) + toRgb[3] * in[7])] << G_SHIFT ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[4] * ((in[5] + in[9]) * 0.5f) + (in[7] - in[6]) + toRgb[5] * in[7])] << B_SHIFT )
							);

							dst[1][3] = (pixel & scanlines.mask) >> scanlines.shifter;

							dst[0] += 4;
							dst[1] += 4;
						}
						else
						{
							u32 pixel;

							dst[0][0] = pixel =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[0] * in[1] + (in[0] - in[1]) + toRgb[1] * ((in[-1] + in[3]) * 0.5f))] << R_SHIFT_0  ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[2] * in[1] + (in[0] - in[1]) + toRgb[3] * ((in[-1] + in[3]) * 0.5f))] << G_SHIFT_0  ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[4] * in[1] + (in[0] - in[1]) + toRgb[5] * ((in[-1] + in[3]) * 0.5f))] << B_SHIFT_0  ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[0] * ((in[1] + in[5]) * 0.5f) + (in[2] - in[3]) + toRgb[1] * in[3])]  << R_SHIFT_16 ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[2] * ((in[1] + in[5]) * 0.5f) + (in[2] - in[3]) + toRgb[3] * in[3])]  << G_SHIFT_16 ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[4] * ((in[1] + in[5]) * 0.5f) + (in[2] - in[3]) + toRgb[5] * in[3])]  << B_SHIFT_16 )
							);

							dst[1][0] = (pixel & scanlines.mask) >> scanlines.shifter;

							dst[0][1] = pixel =
							(
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[0] * in[5] + (in[5] - in[4]) + toRgb[1] * ((in[3] + in[7]) * 0.5f))] << R_SHIFT_0  ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[2] * in[5] + (in[5] - in[4]) + toRgb[3] * ((in[3] + in[7]) * 0.5f))] << G_SHIFT_0  ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[4] * in[5] + (in[5] - in[4]) + toRgb[5] * ((in[3] + in[7]) * 0.5f))] << B_SHIFT_0  ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[0] * ((in[5] + in[9]) * 0.5f) + (in[7] - in[6]) + toRgb[1] * in[7])] << R_SHIFT_16 ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[2] * ((in[5] + in[9]) * 0.5f) + (in[7] - in[6]) + toRgb[3] * in[7])] << G_SHIFT_16 ) |
								( lut.toInt[Lut::COLOR_RANGE*2 + int(toRgb[4] * ((in[5] + in[9]) * 0.5f) + (in[7] - in[6]) + toRgb[5] * in[7])] << B_SHIFT_16 )
							);

							dst[1][1] = (pixel & scanlines.mask) >> scanlines.shifter;

							dst[0] += 2;
							dst[1] += 2;
						}

						in += 8;
					}
			
					dst[0] = reinterpret_cast<u32*>(reinterpret_cast<u8*>(dst[0]) + pitch);
					dst[1] = reinterpret_cast<u32*>(reinterpret_cast<u8*>(dst[1]) + pitch);
				}
			}
     	}
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
