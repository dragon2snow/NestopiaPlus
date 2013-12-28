////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
// Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
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

#ifndef NST_NO_HQ2X

#include "api/NstApiVideo.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterHqX.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif

			Renderer::FilterHqX::Lut::Lut(const bool bpp32,const uint (&left)[3])
			: rgb(bpp32 ? new u32 [0x10000UL] : NULL)
			{
				const uint shifts[3] =
				{
					bpp32 ? 11 : left[0],
					bpp32 ?  5 : left[1],
					bpp32 ?  0 : left[2]
				};

				for (uint i=0; i < 32; ++i)
				{
					for (uint j=0; j < 64; ++j)
					{
						for (uint k=0; k < 32; ++k)
						{
							int r = i << 3;
							int g = j << 2;
							int b = k << 3;

							uint Y = uint(r + g + b) >> 2;
							uint u = 128 + ((r - b) >> 2);
							uint v = 128 + ((-r + 2*g -b) >> 3);

							yuv[(i << shifts[0]) + (j << shifts[1]) + (k << shifts[2])] = (Y << 16) + (u << 8) + v;
						}
					}
				}

				if (bpp32)
				{
					for (dword i=0; i < 0x10000UL; ++i)
						rgb[i] = ((i & 0xF800UL) << 8) | ((i & 0x07E0UL) << 5) | ((i & 0x001FUL) << 3);
				}
			}

			Renderer::FilterHqX::Lut::~Lut()
			{
				delete [] rgb;
			}

			Renderer::FilterHqX::FilterHqX(const RenderState& state)
			: 
			Filter (state),
			lut    (state.bits.count == 32,format.left),
			type   (state.filter)
			{
			}
			
			bool Renderer::FilterHqX::Check(const RenderState& state)
			{
				return
				(
			     	(state.bits.count == 16 && state.bits.mask.b == 0x001FU && ((state.bits.mask.g == 0x07E0U && state.bits.mask.r == 0xF800U) || (state.bits.mask.g == 0x03E0U && state.bits.mask.r == 0x7C00U))) ||
					(state.bits.count == 32 && state.bits.mask.r == 0xFF0000UL && state.bits.mask.g == 0x00FF00UL && state.bits.mask.b == 0x0000FFUL)
				)
				&&
				(
					(state.filter == RenderState::FILTER_HQ2X && state.width == WIDTH*2 && state.height == HEIGHT*2) ||
					(state.filter == RenderState::FILTER_HQ3X && state.width == WIDTH*3 && state.height == HEIGHT*3)
				);
			}

			void Renderer::FilterHqX::Transform(const u8 (&src)[PALETTE][3],u32 (&dst)[PALETTE]) const
			{
				uint rgb[2][3];

				if (bpp == 16)
				{
					rgb[0][0] = format.right[0];
					rgb[0][1] = format.right[1];
					rgb[0][2] = format.right[2];
					rgb[1][0] = format.left[0];
					rgb[1][1] = format.left[1];
					rgb[1][2] = format.left[2];
				}
				else
				{
					rgb[0][0] = 3;
					rgb[0][1] = 2;
					rgb[0][2] = 3;
					rgb[1][0] = 11;
					rgb[1][1] = 5;
					rgb[1][2] = 0;
				}

				for (uint i=0; i < PALETTE; ++i)
				{
					dst[i] = 
					(
						((src[i][0] >> rgb[0][0]) << rgb[1][0]) |
						((src[i][1] >> rgb[0][1]) << rgb[1][1]) |
						((src[i][2] >> rgb[0][2]) << rgb[1][2])
					);
				}
			}
				
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			template<u32 R,u32 G,u32 B>
			dword Renderer::FilterHqX::Interpolate1(dword c1,dword c2)
			{
				return ((((c1 & G)*3 + (c2 & G)) & (G << 2)) + (((c1 & (R|B))*3 + (c2 & (R|B))) & ((R|B) << 2))) >> 2;
			}

			template<>
			inline dword Renderer::FilterHqX::Interpolate1<0xFF0000UL,0x00FF00UL,0x0000FFUL>(dword c1,dword c2)
			{
				return (c1 * 3 + c2) >> 2;
			}
			
			template<u32 R,u32 G,u32 B>
			dword Renderer::FilterHqX::Interpolate2(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*2 + (c2 & G) + (c3 & G)) & (G << 2)) + (((c1 & (R|B))*2 + (c2 & (R|B)) + (c3 & (R|B))) & ((R|B) << 2))) >> 2;
			}

			template<>
			inline dword Renderer::FilterHqX::Interpolate2<0xFF0000UL,0x00FF00UL,0x0000FFUL>(dword c1,dword c2,dword c3)
			{
				return (c1 * 2 + c2 + c3) >> 2;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::FilterHqX::Interpolate3(dword c1,dword c2)
			{
				return ((((c1 & G)*7 + (c2 & G)) & (G << 3)) + (((c1 & (R|B))*7 + (c2 & (R|B))) & ((R|B) << 3))) >> 3;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::FilterHqX::Interpolate4(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*2 + ((c2 & G) + (c3 & G))*7) & (G << 4)) + (((c1 & (R|B))*2 + ((c2 & (R|B)) + (c3 & (R|B)))*7) & ((R|B) << 4))) >> 4;
			}

			inline dword Renderer::FilterHqX::Interpolate5(dword c1,dword c2)
			{
				return (c1 + c2) >> 1;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::FilterHqX::Interpolate6(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*5 + (c2 & G)*2 + (c3 & G)) & (G << 3)) + (((c1 & (R|B))*5 + (c2 & (R|B))*2 + (c3 & (R|B))) & ((R|B) << 3))) >> 3;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::FilterHqX::Interpolate7(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*6 + (c2 & G) + (c3 & G)) & (G << 3)) + (((c1 & (R|B))*6 + (c2 & (R|B)) + (c3 & (R|B))) & ((R|B) << 3))) >> 3;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::FilterHqX::Interpolate9(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*2 + ((c2 & G) + (c3 & G))*3 ) & (G << 3)) + (((c1 & (R|B))*2 + ((c2 & (R|B)) + (c3 & (R|B)))*3 ) & ((R|B) << 3))) >> 3;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::FilterHqX::Interpolate10(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*14 + (c2 & G) + (c3 & G)) & (G << 4)) + (((c1 & (R|B))*14 + (c2 & (R|B)) + (c3 & (R|B))) & ((R|B) << 4))) >> 4;
			}
			
			bool Renderer::FilterHqX::DiffYuv(dword yuv1,dword yuv2)
			{
				return
				(
					(((yuv1 & 0x00FF0000UL) - (yuv2 & 0x00FF0000UL) + 0x300000UL) > 0x600000UL) ||
					(((yuv1 & 0x0000FF00UL) - (yuv2 & 0x0000FF00UL) + 0x000700UL) > 0x000E00UL) ||
					(((yuv1 & 0x000000FFUL) - (yuv2 & 0x000000FFUL) + 0x000006UL) > 0x00000CUL) 
				);
			}

			bool Renderer::FilterHqX::Diff(uint w1,uint w2) const
			{
				return
				(
     				(((lut.yuv[w1] & 0x00FF0000UL) - (lut.yuv[w2] & 0x00FF0000UL) + 0x300000UL) > 0x600000UL) ||
					(((lut.yuv[w1] & 0x0000FF00UL) - (lut.yuv[w2] & 0x0000FF00UL) + 0x000700UL) > 0x000E00UL) ||
					(((lut.yuv[w1] & 0x000000FFUL) - (lut.yuv[w2] & 0x000000FFUL) + 0x000006UL) > 0x00000CUL) 
				);
			}

			template<typename>
			struct Renderer::FilterHqX::Buffer
			{
				uint w[10];
				dword c[10];

				NST_FORCE_INLINE void Convert(const Lut& lut)
				{
					for (uint k=0; k < 9; ++k)
						c[k] = lut.rgb[w[k]];
				}
			};

			template<>
			struct Renderer::FilterHqX::Buffer<u16>
			{
				union
				{
					uint w[10];
					dword c[10];
				};

				void Convert(const Lut&)
				{
				}
			};

			template<typename T,u32 R,u32 G,u32 B>
			NST_FORCE_INLINE void Renderer::FilterHqX::Blit2xRgb(const Input& input,const Output& output) const
			{
				const u8* NST_RESTRICT src = reinterpret_cast<const u8*>(input.screen);
				const long pitch = output.pitch + output.pitch - (WIDTH*2 * sizeof(T));

				T* NST_RESTRICT dst[2] =
				{
					static_cast<T*>(output.pixels) - 2,
					reinterpret_cast<T*>(static_cast<u8*>(output.pixels) + output.pitch) - 2
				};

				//   +----+----+----+
				//   |    |    |    |
				//   | w0 | w1 | w2 |
				//   +----+----+----+
				//   |    |    |    |
				//   | w3 | w4 | w5 |
				//   +----+----+----+
				//   |    |    |    |
				//   | w6 | w7 | w8 |
				//   +----+----+----+

				Buffer<T> b;

				for (uint y=HEIGHT; y; --y)
				{
					const uint lines[2] =
					{
						y < HEIGHT ? WIDTH * sizeof(u16) : 0,
						y > 1      ? WIDTH * sizeof(u16) : 0
					};

					b.w[2] = b.w[1] = input.palette[*reinterpret_cast<const u16*>(src - lines[0])];
					b.w[5] = b.w[4] = input.palette[*reinterpret_cast<const u16*>(src)];
					b.w[8] = b.w[7] = input.palette[*reinterpret_cast<const u16*>(src + lines[1])];

					for (uint x=WIDTH; x; )
					{			
						src += sizeof(u16);
						dst[0] += 2;
						dst[1] += 2;

						b.w[0] = b.w[1];
						b.w[1] = b.w[2];
						b.w[3] = b.w[4];
						b.w[4] = b.w[5];
						b.w[6] = b.w[7];
						b.w[7] = b.w[8];

						if (--x)
						{
							b.w[2] = input.palette[*reinterpret_cast<const u16*>(src - lines[0])];
							b.w[5] = input.palette[*reinterpret_cast<const u16*>(src)];
							b.w[8] = input.palette[*reinterpret_cast<const u16*>(src + lines[1])];
						}

						b.Convert( lut );
  
						const uint yuv5 = lut.yuv[b.w[4]];

						switch 
						(
       						((b.w[4] != b.w[0] && DiffYuv( yuv5, lut.yuv[b.w[0]] )) ? 0x01 : 0x0) |
							((b.w[4] != b.w[1] && DiffYuv( yuv5, lut.yuv[b.w[1]] )) ? 0x02 : 0x0) |
							((b.w[4] != b.w[2] && DiffYuv( yuv5, lut.yuv[b.w[2]] )) ? 0x04 : 0x0) |
							((b.w[4] != b.w[3] && DiffYuv( yuv5, lut.yuv[b.w[3]] )) ? 0x08 : 0x0) |
							((b.w[4] != b.w[5] && DiffYuv( yuv5, lut.yuv[b.w[5]] )) ? 0x10 : 0x0) |
							((b.w[4] != b.w[6] && DiffYuv( yuv5, lut.yuv[b.w[6]] )) ? 0x20 : 0x0) |
							((b.w[4] != b.w[7] && DiffYuv( yuv5, lut.yuv[b.w[7]] )) ? 0x40 : 0x0) |
							((b.w[4] != b.w[8] && DiffYuv( yuv5, lut.yuv[b.w[8]] )) ? 0x80 : 0x0)
						)
						#define PIXEL00_0     dst[0][0] = b.c[4];
						#define PIXEL00_10    dst[0][0] = Interpolate1<R,G,B>( b.c[4], b.c[0] );
						#define PIXEL00_11    dst[0][0] = Interpolate1<R,G,B>( b.c[4], b.c[3] );
						#define PIXEL00_12    dst[0][0] = Interpolate1<R,G,B>( b.c[4], b.c[1] );
						#define PIXEL00_20    dst[0][0] = Interpolate2<R,G,B>( b.c[4], b.c[3], b.c[1] );
						#define PIXEL00_21    dst[0][0] = Interpolate2<R,G,B>( b.c[4], b.c[0], b.c[1] );
						#define PIXEL00_22    dst[0][0] = Interpolate2<R,G,B>( b.c[4], b.c[0], b.c[3] );
						#define PIXEL00_60    dst[0][0] = Interpolate6<R,G,B>( b.c[4], b.c[1], b.c[3] );
                        #define PIXEL00_61    dst[0][0] = Interpolate6<R,G,B>( b.c[4], b.c[3], b.c[1] );
						#define PIXEL00_70    dst[0][0] = Interpolate7<R,G,B>( b.c[4], b.c[3], b.c[1] );
						#define PIXEL00_90    dst[0][0] = Interpolate9<R,G,B>( b.c[4], b.c[3], b.c[1] );
                        #define PIXEL00_100   dst[0][0] = Interpolate10<R,G,B>( b.c[4], b.c[3], b.c[1] );
						#define PIXEL01_0     dst[0][1] = b.c[4];
						#define PIXEL01_10    dst[0][1] = Interpolate1<R,G,B>( b.c[4], b.c[2] );
						#define PIXEL01_11    dst[0][1] = Interpolate1<R,G,B>( b.c[4], b.c[1] );
						#define PIXEL01_12    dst[0][1] = Interpolate1<R,G,B>( b.c[4], b.c[5] );
						#define PIXEL01_20    dst[0][1] = Interpolate2<R,G,B>( b.c[4], b.c[1], b.c[5] );
                        #define PIXEL01_21    dst[0][1] = Interpolate2<R,G,B>( b.c[4], b.c[2], b.c[5] );
						#define PIXEL01_22    dst[0][1] = Interpolate2<R,G,B>( b.c[4], b.c[2], b.c[1] );
						#define PIXEL01_60    dst[0][1] = Interpolate6<R,G,B>( b.c[4], b.c[5], b.c[1] );
                        #define PIXEL01_61    dst[0][1] = Interpolate6<R,G,B>( b.c[4], b.c[1], b.c[5] );
						#define PIXEL01_70    dst[0][1] = Interpolate7<R,G,B>( b.c[4], b.c[1], b.c[5] );
                        #define PIXEL01_90    dst[0][1] = Interpolate9<R,G,B>( b.c[4], b.c[1], b.c[5] );
						#define PIXEL01_100   dst[0][1] = Interpolate10<R,G,B>( b.c[4], b.c[1], b.c[5] );
						#define PIXEL10_0     dst[1][0] = b.c[4];
                        #define PIXEL10_10    dst[1][0] = Interpolate1<R,G,B>( b.c[4], b.c[6] );
						#define PIXEL10_11    dst[1][0] = Interpolate1<R,G,B>( b.c[4], b.c[7] );
						#define PIXEL10_12    dst[1][0] = Interpolate1<R,G,B>( b.c[4], b.c[3] );
                        #define PIXEL10_20    dst[1][0] = Interpolate2<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL10_21    dst[1][0] = Interpolate2<R,G,B>( b.c[4], b.c[6], b.c[3] );
						#define PIXEL10_22    dst[1][0] = Interpolate2<R,G,B>( b.c[4], b.c[6], b.c[7] );
						#define PIXEL10_60    dst[1][0] = Interpolate6<R,G,B>( b.c[4], b.c[3], b.c[7] );
						#define PIXEL10_61    dst[1][0] = Interpolate6<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL10_70    dst[1][0] = Interpolate7<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL10_90    dst[1][0] = Interpolate9<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL10_100   dst[1][0] = Interpolate10<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL11_0     dst[1][1] = b.c[4];
						#define PIXEL11_10    dst[1][1] = Interpolate1<R,G,B>( b.c[4], b.c[8] );
						#define PIXEL11_11    dst[1][1] = Interpolate1<R,G,B>( b.c[4], b.c[5] );
                        #define PIXEL11_12    dst[1][1] = Interpolate1<R,G,B>( b.c[4], b.c[7] );
						#define PIXEL11_20    dst[1][1] = Interpolate2<R,G,B>( b.c[4], b.c[5], b.c[7] );
						#define PIXEL11_21    dst[1][1] = Interpolate2<R,G,B>( b.c[4], b.c[8], b.c[7] );
						#define PIXEL11_22    dst[1][1] = Interpolate2<R,G,B>( b.c[4], b.c[8], b.c[5] );
                        #define PIXEL11_60    dst[1][1] = Interpolate6<R,G,B>( b.c[4], b.c[7], b.c[5] );
						#define PIXEL11_61    dst[1][1] = Interpolate6<R,G,B>( b.c[4], b.c[5], b.c[7] );
						#define PIXEL11_70    dst[1][1] = Interpolate7<R,G,B>( b.c[4], b.c[5], b.c[7] );
						#define PIXEL11_90    dst[1][1] = Interpolate9<R,G,B>( b.c[4], b.c[5], b.c[7] );
                        #define PIXEL11_100   dst[1][1] = Interpolate10<R,G,B>( b.c[4], b.c[5], b.c[7] );
						{
							case 0:
							case 1:
							case 4:
							case 32:
							case 128:
							case 5:
							case 132:
							case 160:
							case 33:
							case 129:
							case 36:
							case 133:
							case 164:
							case 161:
							case 37:
							case 165:
							
								PIXEL00_20
								PIXEL01_20
								PIXEL10_20
								PIXEL11_20
								break;
							
							case 2:
							case 34:
							case 130:
							case 162:
								
								PIXEL00_22
								PIXEL01_21
								PIXEL10_20
								PIXEL11_20
								break;
								
							case 16:
							case 17:
							case 48:
							case 49:
								
								PIXEL00_20
								PIXEL01_22
								PIXEL10_20
								PIXEL11_21
								break;
								
							case 64:
							case 65:
							case 68:
							case 69:
								
								PIXEL00_20
								PIXEL01_20
								PIXEL10_21
								PIXEL11_22
								break;
								
							case 8:
							case 12:
							case 136:
							case 140:
								
								PIXEL00_21
								PIXEL01_20
								PIXEL10_22
								PIXEL11_20
								break;
								
							case 3:
							case 35:
							case 131:
							case 163:
								
								PIXEL00_11
								PIXEL01_21
								PIXEL10_20
								PIXEL11_20
								break;
								
							case 6:
							case 38:
							case 134:
							case 166:
								
								PIXEL00_22
								PIXEL01_12
								PIXEL10_20
								PIXEL11_20
								break;
								
							case 20:
							case 21:
							case 52:
							case 53:
								
								PIXEL00_20
								PIXEL01_11
								PIXEL10_20
								PIXEL11_21
								break;
								
							case 144:
							case 145:
							case 176:
							case 177:
								
								PIXEL00_20
								PIXEL01_22
								PIXEL10_20
								PIXEL11_12
								break;
								
							case 192:
							case 193:
							case 196:
							case 197:
								
								PIXEL00_20
								PIXEL01_20
								PIXEL10_21
								PIXEL11_11
								break;
								
							case 96:
							case 97:
							case 100:
							case 101:
								
								PIXEL00_20
								PIXEL01_20
								PIXEL10_12
								PIXEL11_22
								break;
								
							case 40:
							case 44:
							case 168:
							case 172:
								
								PIXEL00_21
								PIXEL01_20
								PIXEL10_11
								PIXEL11_20
								break;
								
							case 9:
							case 13:
							case 137:
							case 141:
								
								PIXEL00_12
								PIXEL01_20
								PIXEL10_22
								PIXEL11_20
								break;
								
							case 18:
							case 50:
								
								PIXEL00_22
								
								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_10
								else
									PIXEL01_20
								
								PIXEL10_20
								PIXEL11_21
								break;
								
							case 80:
							case 81:
								
								PIXEL00_20
								PIXEL01_22
								PIXEL10_21
								
								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_10
								else
									PIXEL11_20

								break;
								
							case 72:
							case 76:
								
								PIXEL00_21
								PIXEL01_20
								
								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_20

								PIXEL11_22
								break;
								
							case 10:
							case 138:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_10
								else
									PIXEL00_20

								PIXEL01_21
								PIXEL10_22
								PIXEL11_20
								break;
								
							case 66:
								
								PIXEL00_22
								PIXEL01_21
								PIXEL10_21
								PIXEL11_22
								break;
								
							case 24:
								
								PIXEL00_21
								PIXEL01_22
								PIXEL10_22
								PIXEL11_21
								break;
								
							case 7:
							case 39:
							case 135:
								
								PIXEL00_11
								PIXEL01_12
								PIXEL10_20
								PIXEL11_20
								break;
								
							case 148:
							case 149:
							case 180:
								
								PIXEL00_20
								PIXEL01_11
								PIXEL10_20
								PIXEL11_12
								break;
								
							case 224:
							case 228:
							case 225:
								
								PIXEL00_20
								PIXEL01_20
								PIXEL10_12
								PIXEL11_11
								break;
								
							case 41:
							case 169:
							case 45:
								
								PIXEL00_12
								PIXEL01_20
								PIXEL10_11
								PIXEL11_20
								break;
								
							case 22:
							case 54:
								
								PIXEL00_22

								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_0
								else
									PIXEL01_20

								PIXEL10_20
								PIXEL11_21
								break;
								
							case 208:
							case 209:
								
								PIXEL00_20
								PIXEL01_22
								PIXEL10_21

								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20

								break;
								
							case 104:
							case 108:
								
								PIXEL00_21
								PIXEL01_20

								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_20

								PIXEL11_22
								break;
								
							case 11:
							case 139:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20

								PIXEL01_21
								PIXEL10_22
								PIXEL11_20
								break;
								
							case 19:
							case 51:
								
								if (Diff( b.w[1], b.w[5] ))
								{
									PIXEL00_11
									PIXEL01_10
								}
								else
								{
									PIXEL00_60
									PIXEL01_90
								}
								
								PIXEL10_20
								PIXEL11_21
								break;
								
							case 146:
							case 178:
								
								PIXEL00_22
								
								if (Diff( b.w[1], b.w[5] ))
								{
									PIXEL01_10
									PIXEL11_12
								}
								else
								{
									PIXEL01_90
									PIXEL11_61
								}

								PIXEL10_20
								break;
								
							case 84:
							case 85:
								
								PIXEL00_20

								if (Diff( b.w[5], b.w[7] ))
								{
									PIXEL01_11
									PIXEL11_10
								}
								else
								{
									PIXEL01_60
									PIXEL11_90
								}

								PIXEL10_21
								break;
								
							case 112:
							case 113:
								
								PIXEL00_20
								PIXEL01_22

								if (Diff( b.w[5], b.w[7] ))
								{
									PIXEL10_12
									PIXEL11_10
								}
								else
								{
									PIXEL10_61
									PIXEL11_90
								}
								break;
								
							case 200:
							case 204:
								
								PIXEL00_21
								PIXEL01_20

								if (Diff( b.w[7], b.w[3] ))
								{
									PIXEL10_10
									PIXEL11_11
								}
								else
								{
									PIXEL10_90
									PIXEL11_60
								}
								break;
								
							case 73:
							case 77:
								
								if (Diff( b.w[7], b.w[3] ))
								{
									PIXEL00_12
									PIXEL10_10
								}
								else
								{
									PIXEL00_61
									PIXEL10_90
								}
								
								PIXEL01_20
								PIXEL11_22
								break;
								
							case 42:
							case 170:
								
								if (Diff( b.w[3], b.w[1] ))
								{
									PIXEL00_10
									PIXEL10_11
								}
								else
								{
									PIXEL00_90
									PIXEL10_60
								}
								
								PIXEL01_21
								PIXEL11_20
								break;
								
							case 14:
							case 142:
								
								if (Diff( b.w[3], b.w[1] ))
								{
									PIXEL00_10
									PIXEL01_12
								}
								else
								{
									PIXEL00_90
									PIXEL01_61
								}
								
								PIXEL10_22
								PIXEL11_20
								break;
								
							case 67:
								
								PIXEL00_11
								PIXEL01_21
								PIXEL10_21
								PIXEL11_22
								break;
								
							case 70:
								
								PIXEL00_22
								PIXEL01_12
								PIXEL10_21
								PIXEL11_22
								break;
								
							case 28:
								
								PIXEL00_21
								PIXEL01_11
								PIXEL10_22
								PIXEL11_21
								break;
								
							case 152:
								
								PIXEL00_21
								PIXEL01_22
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 194:
								
								PIXEL00_22
								PIXEL01_21
								PIXEL10_21
								PIXEL11_11
								break;
								
							case 98:
								
								PIXEL00_22
								PIXEL01_21
								PIXEL10_12
								PIXEL11_22
								break;
								
							case 56:
								
								PIXEL00_21
								PIXEL01_22
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 25:
								
								PIXEL00_12
								PIXEL01_22
								PIXEL10_22
								PIXEL11_21
								break;
								
							case 26:
							case 31:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20

								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_0
								else
									PIXEL01_20

								PIXEL10_22
								PIXEL11_21
								break;

							case 82:
							case 214:
								
								PIXEL00_22

								if (Diff( b.w[1], b.w[5] ))		
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_21

								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20
									
								break;
								
							case 88:
							case 248:
								
								PIXEL00_21
								PIXEL01_22

								if (Diff( b.w[7], b.w[3] ))	
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_0							
								else								
									PIXEL11_20
								
								break;
								
							case 74:
							case 107:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0									
								else									
									PIXEL00_20
									
								PIXEL01_21

								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_20
										
								PIXEL11_22
								break;
								
							case 27:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0								
								else								
									PIXEL00_20
								
								PIXEL01_10
								PIXEL10_22
								PIXEL11_21
								break;
								
							case 86:
								
								PIXEL00_22

								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_21
								PIXEL11_10
								break;
								
							case 216:
								
								PIXEL00_21
								PIXEL01_22
								PIXEL10_10

								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 106:
								
								PIXEL00_10
								PIXEL01_21

								if (Diff( b.w[7], b.w[3] ))	
									PIXEL10_0								
								else
									PIXEL10_20
								
								PIXEL11_22
								break;
								
							case 30:
								
								PIXEL00_10
								
								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_0
								else
									PIXEL01_20

								PIXEL10_22
								PIXEL11_21
								break;
								
							case 210:
								
								PIXEL00_22
								PIXEL01_10
								PIXEL10_21
								
								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20

								break;
								
							case 120:
								
								PIXEL00_21
								PIXEL01_22

								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_20

								PIXEL11_10
								break;
								
							case 75:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
									
								PIXEL01_21
								PIXEL10_10
								PIXEL11_22
								break;
								
							case 29:
								
								PIXEL00_12
								PIXEL01_11
								PIXEL10_22
								PIXEL11_21
								break;
								
							case 198:
								
								PIXEL00_22
								PIXEL01_12
								PIXEL10_21
								PIXEL11_11
								break;
								
							case 184:
								
								PIXEL00_21
								PIXEL01_22
								PIXEL10_11
								PIXEL11_12
								break;
								
							case 99:
								
								PIXEL00_11
								PIXEL01_21
								PIXEL10_12
								PIXEL11_22
								break;
								
							case 57:
								
								PIXEL00_12
								PIXEL01_22
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 71:
								
								PIXEL00_11
								PIXEL01_12
								PIXEL10_21
								PIXEL11_22
								break;
								
							case 156:
								
								PIXEL00_21
								PIXEL01_11
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 226:
								
								PIXEL00_22
								PIXEL01_21
								PIXEL10_12
								PIXEL11_11
								break;
								
							case 60:
								
								PIXEL00_21
								PIXEL01_11
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 195:
								
								PIXEL00_11
								PIXEL01_21
								PIXEL10_21
								PIXEL11_11
								break;
								
							case 102:
								
								PIXEL00_22
								PIXEL01_12
								PIXEL10_12
								PIXEL11_22
								break;
								
							case 153:
								
								PIXEL00_12
								PIXEL01_22
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 58:
								
								if (Diff( b.w[3], b.w[1] ))	
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 83:
								
								PIXEL00_11

								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								PIXEL10_21
									
								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_10
								else
									PIXEL11_70

								break;
								
							case 92:
								
								PIXEL00_21
								PIXEL01_11

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else							
									PIXEL10_70
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 202:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								PIXEL01_21

								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_70
								
								PIXEL11_11
								break;
								
							case 78:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_10							
								else								
									PIXEL00_70
								
								PIXEL01_12
								
								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_70

								PIXEL11_22
								break;
								
							case 154:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 114:
								
								PIXEL00_22

								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_10
								else
									PIXEL01_70

								PIXEL10_12
								
								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_10
								else
									PIXEL11_70
								
								break;
								
							case 89:
								
								PIXEL00_12
								PIXEL01_22

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 90:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 55:
							case 23:
								
								if (Diff( b.w[1], b.w[5] ))
								{
									PIXEL00_11
									PIXEL01_0
								}
								else
								{
									PIXEL00_60
									PIXEL01_90
								}
								
								PIXEL10_20
								PIXEL11_21
								break;
								
							case 182:
							case 150:
								
								PIXEL00_22
								
								if (Diff( b.w[1], b.w[5] ))
								{
									PIXEL01_0
									PIXEL11_12
								}
								else
								{
									PIXEL01_90
									PIXEL11_61
								}
								
								PIXEL10_20
								break;
								
							case 213:
							case 212:
								
								PIXEL00_20

								if (Diff( b.w[5], b.w[7] ))
								{
									PIXEL01_11
									PIXEL11_0
								}
								else
								{
									PIXEL01_60
									PIXEL11_90
								}
								
								PIXEL10_21
								break;
								
							case 241:
							case 240:
								
								PIXEL00_20
								PIXEL01_22

								if (Diff( b.w[5], b.w[7] ))
								{
									PIXEL10_12
									PIXEL11_0
								}
								else
								{
									PIXEL10_61
									PIXEL11_90
								}
								break;
								
							case 236:
							case 232:
								
								PIXEL00_21
								PIXEL01_20

								if (Diff( b.w[7], b.w[3] ))
								{
									PIXEL10_0
									PIXEL11_11
								}
								else
								{
									PIXEL10_90
									PIXEL11_60
								}
								break;
								
							case 109:
							case 105:
								
								if (Diff( b.w[7], b.w[3] ))
								{
									PIXEL00_12
									PIXEL10_0
								}
								else
								{
									PIXEL00_61
									PIXEL10_90
								}
								
								PIXEL01_20
								PIXEL11_22
								break;
								
							case 171:
							case 43:
								
								if (Diff( b.w[3], b.w[1] ))
								{
									PIXEL00_0
									PIXEL10_11
								}
								else
								{
									PIXEL00_90
									PIXEL10_60
								}
								
								PIXEL01_21
								PIXEL11_20
								break;
								
							case 143:
							case 15:
								
								if (Diff( b.w[3], b.w[1] ))
								{
									PIXEL00_0
									PIXEL01_12
								}
								else
								{
									PIXEL00_90
									PIXEL01_61
								}
								
								PIXEL10_22
								PIXEL11_20
								break;
								
							case 124:
								
								PIXEL00_21
								PIXEL01_11

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								PIXEL11_10
								break;
								
							case 203:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_20
								
								PIXEL01_21
								PIXEL10_10
								PIXEL11_11
								break;
								
							case 62:
								
								PIXEL00_10

								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 211:
								
								PIXEL00_11
								PIXEL01_10
								PIXEL10_21

								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 118:
								
								PIXEL00_22

								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_12
								PIXEL11_10
								break;
								
							case 217:
								
								PIXEL00_12
								PIXEL01_22
								PIXEL10_10

								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 110:
								
								PIXEL00_10
								PIXEL01_12

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								PIXEL11_22
								break;
								
							case 155:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_20
								
								PIXEL01_10
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 188:
								
								PIXEL00_21
								PIXEL01_11
								PIXEL10_11
								PIXEL11_12
								break;
								
							case 185:
								
								PIXEL00_12
								PIXEL01_22
								PIXEL10_11
								PIXEL11_12
								break;
								
							case 61:
								
								PIXEL00_12
								PIXEL01_11
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 157:
								
								PIXEL00_12
								PIXEL01_11
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 103:
								
								PIXEL00_11
								PIXEL01_12
								PIXEL10_12
								PIXEL11_22
								break;
								
							case 227:
								
								PIXEL00_11
								PIXEL01_21
								PIXEL10_12
								PIXEL11_11
								break;
								
							case 230:
								
								PIXEL00_22
								PIXEL01_12
								PIXEL10_12
								PIXEL11_11
								break;
								
							case 199:
								
								PIXEL00_11
								PIXEL01_12
								PIXEL10_21
								PIXEL11_11
								break;
								
							case 220:
								
								PIXEL00_21
								PIXEL01_11

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 158:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 234:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								PIXEL01_21

								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_20								
								
								PIXEL11_11
								break;
								
							case 242:
								
								PIXEL00_22

								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								PIXEL10_12

								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20
								
								break;
								
							case 59:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
								
								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_10
								else
									PIXEL01_70
								
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 121:
								
								PIXEL00_12
								PIXEL01_22

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Diff( b.w[5], b.w[7]))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 87:
								
								PIXEL00_11

								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_21

								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 79:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
									
								PIXEL01_12

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								PIXEL11_22
								break;
								
							case 122:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else							
									PIXEL11_70
								
								break;
								
							case 94:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 218:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 91:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
								
								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_10
								else
									PIXEL01_70
								
								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_70
								
								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_10
								else
									PIXEL11_70
								
								break;
								
							case 229:
								
								PIXEL00_20
								PIXEL01_20
								PIXEL10_12
								PIXEL11_11
								break;
								
							case 167:
								
								PIXEL00_11
								PIXEL01_12
								PIXEL10_20
								PIXEL11_20
								break;
								
							case 173:
								
								PIXEL00_12
								PIXEL01_20
								PIXEL10_11
								PIXEL11_20
								break;
								
							case 181:
								
								PIXEL00_20
								PIXEL01_11
								PIXEL10_20
								PIXEL11_12
								break;
								
							case 186:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_10
								else
									PIXEL00_70
								
								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_10
								else
									PIXEL01_70
								
								PIXEL10_11
								PIXEL11_12
								break;
								
							case 115:
								
								PIXEL00_11

								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								PIXEL10_12

								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_10
								else
									PIXEL11_70

								break;
								
							case 93:
								
								PIXEL00_12
								PIXEL01_11

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 206:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_10
								else
									PIXEL00_70
									
								PIXEL01_12
								
								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_70

								PIXEL11_11
								break;
								
							case 205:
							case 201:
								
								PIXEL00_12
								PIXEL01_20

								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_70
								
								PIXEL11_11
								break;
								
							case 174:
							case 46:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								PIXEL01_12
								PIXEL10_11
								PIXEL11_20
								break;
								
							case 179:
							case 147:
								
								PIXEL00_11

								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_10
								else
									PIXEL01_70
								
								PIXEL10_20
								PIXEL11_12
								break;
								
							case 117:
							case 116:
								
								PIXEL00_20
								PIXEL01_11
								PIXEL10_12

								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_10
								else
									PIXEL11_70
								
								break;
								
							case 189:
								
								PIXEL00_12
								PIXEL01_11
								PIXEL10_11
								PIXEL11_12
								break;
								
							case 231:
								
								PIXEL00_11
								PIXEL01_12
								PIXEL10_12
								PIXEL11_11
								break;
								
							case 126:
								
								PIXEL00_10

								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_0
								else
									PIXEL01_20
								
								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_20
								
								PIXEL11_10
								break;
								
							case 219:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
								
								PIXEL01_10
								PIXEL10_10
								
								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20
								
								break;
								
							case 125:
								
								if (Diff( b.w[7], b.w[3] ))
								{
									PIXEL00_12
									PIXEL10_0
								}
								else
								{
									PIXEL00_61
									PIXEL10_90
								}
								
								PIXEL01_11
								PIXEL11_10
								break;
								
							case 221:
								
								PIXEL00_12
								
								if (Diff( b.w[5], b.w[7] ))
								{
									PIXEL01_11
									PIXEL11_0
								}
								else
								{
									PIXEL01_60
									PIXEL11_90
								}
								
								PIXEL10_10
								break;
								
							case 207:
								
								if (Diff( b.w[3], b.w[1] ))
								{
									PIXEL00_0
									PIXEL01_12
								}
								else
								{
									PIXEL00_90
									PIXEL01_61
								}
								
								PIXEL10_10
								PIXEL11_11
								break;
								
							case 238:
								
								PIXEL00_10
								PIXEL01_12

								if (Diff( b.w[7], b.w[3] ))
								{
									PIXEL10_0
									PIXEL11_11
								}
								else
								{
									PIXEL10_90
									PIXEL11_60
								}
								break;
								
							case 190:
								
								PIXEL00_10

								if (Diff( b.w[1], b.w[5] ))
								{
									PIXEL01_0
									PIXEL11_12
								}
								else
								{
									PIXEL01_90
									PIXEL11_61
								}
								
								PIXEL10_11
								break;
								
							case 187:
								
								if (Diff( b.w[3], b.w[1] ))
								{
									PIXEL00_0
									PIXEL10_11
								}
								else
								{
									PIXEL00_90
									PIXEL10_60
								}
								
								PIXEL01_10
								PIXEL11_12
								break;
								
							case 243:
								
								PIXEL00_11
								PIXEL01_10

								if (Diff( b.w[5], b.w[7] ))
								{
									PIXEL10_12
									PIXEL11_0
								}
								else
								{
									PIXEL10_61
									PIXEL11_90
								}
								break;
								
							case 119:
								
								if (Diff( b.w[1], b.w[5] ))
								{
									PIXEL00_11
									PIXEL01_0
								}
								else
								{
									PIXEL00_60
									PIXEL01_90
								}
								
								PIXEL10_12
								PIXEL11_10
								break;
								
							case 237:
							case 233:
								
								PIXEL00_12
								PIXEL01_20
								
								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_100
								
								PIXEL11_11
								break;
								
							case 175:
							case 47:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_100
								
								PIXEL01_12
								PIXEL10_11
								PIXEL11_20
								break;
								
							case 183:
							case 151:
								
								PIXEL00_11

								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_0
								else
									PIXEL01_100
								
								PIXEL10_20
								PIXEL11_12
								break;
								
							case 245:
							case 244:
								
								PIXEL00_20
								PIXEL01_11
								PIXEL10_12

								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_100
								
								break;
								
							case 250:
								
								PIXEL00_10
								PIXEL01_10

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else
									PIXEL11_20
								
								break;
								
							case 123:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_20
								
								PIXEL01_10
								
								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								PIXEL11_10
								break;
								
							case 95:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_20
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_10
								PIXEL11_10
								break;
								
							case 222:
								
								PIXEL00_10
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_10
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0
								else								
									PIXEL11_20
								
								break;
								
							case 252:
								
								PIXEL00_21
								PIXEL01_11

								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_100
								
								break;
								
							case 249:
								
								PIXEL00_12
								PIXEL01_22

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_100
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 235:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
																
								PIXEL01_21

								if (Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_100
								
								PIXEL11_11
								break;
								
							case 111:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_100
								
								PIXEL01_12
								
								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								PIXEL11_22
								break;
								
							case 63:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_100
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 159:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_20
								
								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_100
								
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 215:
								
								PIXEL00_11

								if (Diff( b.w[1], b.w[5] ))										
									PIXEL01_0
								else
									PIXEL01_100
										
								PIXEL10_21

								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 246:
								
								PIXEL00_22

								if (Diff( b.w[1], b.w[5] ))										
									PIXEL01_0										
								else										
									PIXEL01_20
								
								PIXEL10_12
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_100
								
								break;
								
							case 254:
								
								PIXEL00_10

								if (Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_100
								
								break;
								
							case 253:
								
								PIXEL00_12
								PIXEL01_11

								if (Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_100
								
								if (Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_100
								
								break;
								
							case 251:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_0									
								else									
									PIXEL00_20
								
								PIXEL01_10
								
								if (Diff( b.w[7], b.w[3] ))									
									PIXEL10_0									
								else									
									PIXEL10_100
								
								if (Diff( b.w[5], b.w[7] ))									
									PIXEL11_0									
								else									
									PIXEL11_20
								
								break;
								
							case 239:
								
								if (Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_100
								
								PIXEL01_12
								
								if (Diff( b.w[7], b.w[3] ))									
									PIXEL10_0
								else
									PIXEL10_100

								PIXEL11_11
								break;
								
							case 127:
								
								if (Diff( b.w[3], b.w[1] ))									
									PIXEL00_0									
								else									
									PIXEL00_100
								
								if (Diff( b.w[1], b.w[5] ))									
									PIXEL01_0									
								else									
									PIXEL01_20
								
								if (Diff( b.w[7], b.w[3] ))									
									PIXEL10_0									
								else									
									PIXEL10_20
								
								PIXEL11_10
								break;
								
							case 191:
								
								if (Diff( b.w[3], b.w[1] ))									
									PIXEL00_0									
								else									
									PIXEL00_100
								
								if (Diff( b.w[1], b.w[5] ))									
									PIXEL01_0									
								else									
									PIXEL01_100
								
								PIXEL10_11
								PIXEL11_12
								break;
								
							case 223:
								
								if (Diff( b.w[3], b.w[1] ))									
									PIXEL00_0									
								else									
									PIXEL00_20
								
								if (Diff( b.w[1], b.w[5] ))									
									PIXEL01_0									
								else									
									PIXEL01_100
								
								PIXEL10_10
									
								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20

								break;
								
							case 247:
								
								PIXEL00_11
								
								if (Diff( b.w[1], b.w[5] ))
									PIXEL01_0
								else
									PIXEL01_100

								PIXEL10_12
								
								if (Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_100

								break;
								
							case 255:
								
								if (Diff( b.w[3], b.w[1] ))								
									PIXEL00_0									
								else									
									PIXEL00_100
								
								if (Diff( b.w[1], b.w[5] ))									
									PIXEL01_0									
								else									
									PIXEL01_100
								
								if (Diff( b.w[7], b.w[3] ))									
									PIXEL10_0								
								else									
									PIXEL10_100
								
								if (Diff( b.w[5], b.w[7] ))									
									PIXEL11_0									
								else									
									PIXEL11_100
								
								break;	

							NST_UNREACHABLE

							#undef PIXEL00_0   
							#undef PIXEL00_10  
							#undef PIXEL00_11  
							#undef PIXEL00_12  
							#undef PIXEL00_20  
							#undef PIXEL00_21  
							#undef PIXEL00_22  
							#undef PIXEL00_60  
                            #undef PIXEL00_61  
							#undef PIXEL00_70  
							#undef PIXEL00_90  
                            #undef PIXEL00_100 
							#undef PIXEL01_0   
							#undef PIXEL01_10  
							#undef PIXEL01_11  
							#undef PIXEL01_12  
							#undef PIXEL01_20  
                            #undef PIXEL01_21  
							#undef PIXEL01_22  
							#undef PIXEL01_60  
                            #undef PIXEL01_61  
							#undef PIXEL01_70  
                            #undef PIXEL01_90  
							#undef PIXEL01_100 
							#undef PIXEL10_0   
                            #undef PIXEL10_10  
							#undef PIXEL10_11  
							#undef PIXEL10_12  
                            #undef PIXEL10_20  
							#undef PIXEL10_21  
							#undef PIXEL10_22  
							#undef PIXEL10_60  
							#undef PIXEL10_61  
							#undef PIXEL10_70  
							#undef PIXEL10_90  
							#undef PIXEL10_100 
							#undef PIXEL11_0   
							#undef PIXEL11_10  
							#undef PIXEL11_11  
                            #undef PIXEL11_12  
							#undef PIXEL11_20  
							#undef PIXEL11_21  
							#undef PIXEL11_22  
                            #undef PIXEL11_60  
							#undef PIXEL11_61  
							#undef PIXEL11_70  
							#undef PIXEL11_90  
                            #undef PIXEL11_100 
						}
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[0]) + pitch);
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[1]) + pitch);
				}
			}
				 			
			template<typename T,u32 R,u32 G,u32 B>
			NST_FORCE_INLINE void Renderer::FilterHqX::Blit3xRgb(const Input& input,const Output& output) const
			{
				const u8* NST_RESTRICT src = reinterpret_cast<const u8*>(input.screen);
				const long pitch = (output.pitch * 2) + output.pitch - (WIDTH*3 * sizeof(T));

				T* NST_RESTRICT dst[3] =
				{
					static_cast<T*>(output.pixels) - 3,
					reinterpret_cast<T*>(static_cast<u8*>(output.pixels) + output.pitch) - 3,
					reinterpret_cast<T*>(static_cast<u8*>(output.pixels) + output.pitch * 2) - 3
				};

				//   +----+----+----+
				//   |    |    |    |
				//   | w0 | w1 | w2 |
				//   +----+----+----+
				//   |    |    |    |
				//   | w3 | w4 | w5 |
				//   +----+----+----+
				//   |    |    |    |
				//   | w6 | w7 | w8 |
				//   +----+----+----+

				Buffer<T> b;

				for (uint y=HEIGHT; y; --y)
				{
					const uint lines[2] =
					{
						y < HEIGHT ? WIDTH * sizeof(u16) : 0,
						y > 1      ? WIDTH * sizeof(u16) : 0
					};

					b.w[2] = b.w[1] = input.palette[*reinterpret_cast<const u16*>(src - lines[0])];
					b.w[5] = b.w[4] = input.palette[*reinterpret_cast<const u16*>(src)];
					b.w[8] = b.w[7] = input.palette[*reinterpret_cast<const u16*>(src + lines[1])];

					for (uint x=WIDTH; x; )
					{			
						src += sizeof(u16);
						dst[0] += 3;
						dst[1] += 3;
						dst[2] += 3;

						b.w[0] = b.w[1];
						b.w[1] = b.w[2];
						b.w[3] = b.w[4];
						b.w[4] = b.w[5];
						b.w[6] = b.w[7];
						b.w[7] = b.w[8];

						if (--x)
						{
							b.w[2] = input.palette[*reinterpret_cast<const u16*>(src - lines[0])];
							b.w[5] = input.palette[*reinterpret_cast<const u16*>(src)];
							b.w[8] = input.palette[*reinterpret_cast<const u16*>(src + lines[1])];
						}

						b.Convert( lut );
  
						const uint yuv5 = lut.yuv[b.w[4]];

						switch 
						(
       						((b.w[4] != b.w[0] && DiffYuv( yuv5, lut.yuv[b.w[0]] )) ? 0x01 : 0x0) |
							((b.w[4] != b.w[1] && DiffYuv( yuv5, lut.yuv[b.w[1]] )) ? 0x02 : 0x0) |
							((b.w[4] != b.w[2] && DiffYuv( yuv5, lut.yuv[b.w[2]] )) ? 0x04 : 0x0) |
							((b.w[4] != b.w[3] && DiffYuv( yuv5, lut.yuv[b.w[3]] )) ? 0x08 : 0x0) |
							((b.w[4] != b.w[5] && DiffYuv( yuv5, lut.yuv[b.w[5]] )) ? 0x10 : 0x0) |
							((b.w[4] != b.w[6] && DiffYuv( yuv5, lut.yuv[b.w[6]] )) ? 0x20 : 0x0) |
							((b.w[4] != b.w[7] && DiffYuv( yuv5, lut.yuv[b.w[7]] )) ? 0x40 : 0x0) |
							((b.w[4] != b.w[8] && DiffYuv( yuv5, lut.yuv[b.w[8]] )) ? 0x80 : 0x0)
						)
						#define PIXEL00_1M  dst[0][0] = Interpolate1<R,G,B>( b.c[4], b.c[0] );
                        #define PIXEL00_1U  dst[0][0] = Interpolate1<R,G,B>( b.c[4], b.c[1] );
						#define PIXEL00_1L  dst[0][0] = Interpolate1<R,G,B>( b.c[4], b.c[3] );
						#define PIXEL00_2   dst[0][0] = Interpolate2<R,G,B>( b.c[4], b.c[3], b.c[1] );
                        #define PIXEL00_4   dst[0][0] = Interpolate4<R,G,B>( b.c[4], b.c[3], b.c[1] );
						#define PIXEL00_5   dst[0][0] = Interpolate5( b.c[3], b.c[1] );
						#define PIXEL00_C   dst[0][0] = b.c[4];			
						#define PIXEL01_1   dst[0][1] = Interpolate1<R,G,B>( b.c[4], b.c[1] );
						#define PIXEL01_3   dst[0][1] = Interpolate3<R,G,B>( b.c[4], b.c[1] );
						#define PIXEL01_6   dst[0][1] = Interpolate1<R,G,B>( b.c[1], b.c[4] );
						#define PIXEL01_C   dst[0][1] = b.c[4];			
						#define PIXEL02_1M  dst[0][2] = Interpolate1<R,G,B>( b.c[4], b.c[2] );
						#define PIXEL02_1U  dst[0][2] = Interpolate1<R,G,B>( b.c[4], b.c[1] );
                        #define PIXEL02_1R  dst[0][2] = Interpolate1<R,G,B>( b.c[4], b.c[5] );
						#define PIXEL02_2   dst[0][2] = Interpolate2<R,G,B>( b.c[4], b.c[1], b.c[5] );
						#define PIXEL02_4   dst[0][2] = Interpolate4<R,G,B>( b.c[4], b.c[1], b.c[5] );
						#define PIXEL02_5   dst[0][2] = Interpolate5( b.c[1], b.c[5] );
                        #define PIXEL02_C   dst[0][2] = b.c[4];			
						#define PIXEL10_1   dst[1][0] = Interpolate1<R,G,B>( b.c[4], b.c[3] );
						#define PIXEL10_3   dst[1][0] = Interpolate3<R,G,B>( b.c[4], b.c[3] );
                        #define PIXEL10_6   dst[1][0] = Interpolate1<R,G,B>( b.c[3], b.c[4] );
						#define PIXEL10_C   dst[1][0] = b.c[4];            
						#define PIXEL11     dst[1][1] = b.c[4];			
                        #define PIXEL12_1   dst[1][2] = Interpolate1<R,G,B>( b.c[4], b.c[5] );
						#define PIXEL12_3   dst[1][2] = Interpolate3<R,G,B>( b.c[4], b.c[5] );
						#define PIXEL12_6   dst[1][2] = Interpolate1<R,G,B>( b.c[5], b.c[4] );
						#define PIXEL12_C   dst[1][2] = b.c[4];			
						#define PIXEL20_1M  dst[2][0] = Interpolate1<R,G,B>( b.c[4], b.c[6] );
						#define PIXEL20_1D  dst[2][0] = Interpolate1<R,G,B>( b.c[4], b.c[7] );
						#define PIXEL20_1L  dst[2][0] = Interpolate1<R,G,B>( b.c[4], b.c[3] );
						#define PIXEL20_2   dst[2][0] = Interpolate2<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL20_4   dst[2][0] = Interpolate4<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL20_5   dst[2][0] = Interpolate5( b.c[7], b.c[3]);
                        #define PIXEL20_C   dst[2][0] = b.c[4];			
						#define PIXEL21_1   dst[2][1] = Interpolate1<R,G,B>( b.c[4], b.c[7] );
						#define PIXEL21_3   dst[2][1] = Interpolate3<R,G,B>( b.c[4], b.c[7] );
                        #define PIXEL21_6   dst[2][1] = Interpolate1<R,G,B>( b.c[7], b.c[4] );
						#define PIXEL21_C   dst[2][1] = b.c[4];			
						#define PIXEL22_1M  dst[2][2] = Interpolate1<R,G,B>( b.c[4], b.c[8] );
                        #define PIXEL22_1D  dst[2][2] = Interpolate1<R,G,B>( b.c[4], b.c[7] );
						#define PIXEL22_1R  dst[2][2] = Interpolate1<R,G,B>( b.c[4], b.c[5] );
						#define PIXEL22_2   dst[2][2] = Interpolate2<R,G,B>( b.c[4], b.c[5], b.c[7] );
						#define PIXEL22_4   dst[2][2] = Interpolate4<R,G,B>( b.c[4], b.c[5], b.c[7] );
                        #define PIXEL22_5   dst[2][2] = Interpolate5( b.c[5], b.c[7] );
                        #define PIXEL22_C   dst[2][2] = b.c[4];
						{
							case 0:
							case 1:
							case 4:
							case 32:
							case 128:
							case 5:
							case 132:
							case 160:
							case 33:
							case 129:
							case 36:
							case 133:
							case 164:
							case 161:
							case 37:
							case 165:
							
								PIXEL00_2
								PIXEL01_1
								PIXEL02_2
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_2
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 2:
							case 34:
							case 130:
							case 162:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_2
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 16:
							case 17:
							case 48:
							case 49:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 64:
							case 65:
							case 68:
							case 69:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_2
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 8:
							case 12:
							case 136:
							case 140:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_2
								PIXEL10_C
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 3:
							case 35:
							case 131:
							case 163:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_2
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 6:
							case 38:
							case 134:
							case 166:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_2
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 20:
							case 21:
							case 52:
							case 53:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 144:
							case 145:
							case 176:
							case 177:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 192:
							case 193:
							case 196:
							case 197:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_2
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 96:
							case 97:
							case 100:
							case 101:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_2
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 40:
							case 44:
							case 168:
							case 172:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_2
								PIXEL10_C
								PIXEL11
								PIXEL12_1
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 9:
							case 13:
							case 137:
							case 141:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_2
								PIXEL10_C
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 18:
							case 50:
								
								PIXEL00_1M
						
								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_1M
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
									PIXEL12_3
								}
						
								PIXEL10_1
								PIXEL11
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 80:
							case 81:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL20_1M
								
								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_1M
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 72:
							case 76:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_2
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_1M
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
									PIXEL21_3
								}
								
								PIXEL22_1M
								break;
								
							case 10:
							case 138:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_1M
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
									PIXEL10_3
								}
								
								PIXEL02_1M
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 66:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 24:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 7:
							case 39:
							case 135:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_2
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 148:
							case 149:
							case 180:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 224:
							case 228:
							case 225:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_2
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 41:
							case 169:
							case 45:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_2
								PIXEL10_C
								PIXEL11
								PIXEL12_1
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 22:
							case 54:
								
								PIXEL00_1M

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
									PIXEL12_3
								}
								
								PIXEL10_1
								PIXEL11
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 208:
							case 209:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL20_1M

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_4
								}								
								break;
								
							case 104:
							case 108:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_2
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
									PIXEL21_3
								}
								
								PIXEL22_1M
								break;
								
							case 11:
							case 139:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
									PIXEL10_3
								}
								
								PIXEL02_1M
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 19:
							case 51:
								
								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL00_1L
									PIXEL01_C
									PIXEL02_1M
									PIXEL12_C
								}
								else
								{
									PIXEL00_2
									PIXEL01_6
									PIXEL02_5
									PIXEL12_1
								}
								
								PIXEL10_1
								PIXEL11
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 146:
							case 178:
								
								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_1M
									PIXEL12_C
									PIXEL22_1D
								}
								else
								{
									PIXEL01_1
									PIXEL02_5
									PIXEL12_6
									PIXEL22_2
								}
								
								PIXEL00_1M
								PIXEL10_1
								PIXEL11
								PIXEL20_2
								PIXEL21_1
								break;
								
							case 84:
							case 85:
								
								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL02_1U
									PIXEL12_C
									PIXEL21_C
									PIXEL22_1M
								}
								else
								{
									PIXEL02_2
									PIXEL12_6
									PIXEL21_1
									PIXEL22_5
								}
								
								PIXEL00_2
								PIXEL01_1
								PIXEL10_1
								PIXEL11
								PIXEL20_1M
								break;
								
							case 112:
							case 113:
								
								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL20_1L
									PIXEL21_C
									PIXEL22_1M
								}
								else
								{
									PIXEL12_1
									PIXEL20_2
									PIXEL21_6
									PIXEL22_5
								}
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								break;
								
							case 200:
							case 204:
								
								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_1M
									PIXEL21_C
									PIXEL22_1R
								}
								else
								{
									PIXEL10_1
									PIXEL20_5
									PIXEL21_6
									PIXEL22_2
								}
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_2
								PIXEL11
								PIXEL12_1
								break;
								
							case 73:
							case 77:
								
								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL00_1U
									PIXEL10_C
									PIXEL20_1M
									PIXEL21_C
								}
								else
								{
									PIXEL00_2
									PIXEL10_6
									PIXEL20_5
									PIXEL21_1
								}
								
								PIXEL01_1
								PIXEL02_2
								PIXEL11
								PIXEL12_1
								PIXEL22_1M
								break;
								
							case 42:
							case 170:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_1M
									PIXEL01_C
									PIXEL10_C
									PIXEL20_1D
								}
								else
								{
									PIXEL00_5
									PIXEL01_1
									PIXEL10_6
									PIXEL20_2
								}
								
								PIXEL02_1M
								PIXEL11
								PIXEL12_1
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 14:
							case 142:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_1M
									PIXEL01_C
									PIXEL02_1R
									PIXEL10_C
								}
								else
								{
									PIXEL00_5
									PIXEL01_6
									PIXEL02_2
									PIXEL10_1
								}
								
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 67:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 70:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 28:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 152:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 194:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 98:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 56:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 25:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 26:
							case 31:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL10_3
								}
								
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL02_4
									PIXEL12_3
								}
								
								PIXEL11
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 82:
							case 214:
								
								PIXEL00_1M
								
								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
								}
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1M

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 88:
							case 248:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1M
								PIXEL11

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
								}
								
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL22_4
								}							
								break;
								
							case 74:
							case 107:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
								}
								
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL20_4
									PIXEL21_3
								}
								
								PIXEL22_1M
								break;
								
							case 27:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
									PIXEL10_3
								}
								
								PIXEL02_1M
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 86:
								
								PIXEL00_1M

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
									PIXEL12_3
								}
								
								PIXEL10_1
								PIXEL11
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 216:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL20_1M

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 106:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1M
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
									PIXEL21_3
								}
								
								PIXEL22_1M
								break;
								
							case 30:
								
								PIXEL00_1M

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
									PIXEL12_3
								}
								
								PIXEL10_C
								PIXEL11
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 210:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL20_1M
								
								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 120:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1M
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
									PIXEL21_3
								}
								
								PIXEL22_1M
								break;
								
							case 75:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
									PIXEL10_3
								}
								
								PIXEL02_1M
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 29:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 198:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 184:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 99:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 57:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 71:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 156:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 226:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 60:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 195:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 102:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 153:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 58:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
										
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 83:
								
								PIXEL00_1L
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 92:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								
								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2

								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 202:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 78:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 154:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 114:
								
								PIXEL00_1M
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1L
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 89:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 90:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2

								PIXEL10_C
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 55:
							case 23:
								
								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL00_1L
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL00_2
									PIXEL01_6
									PIXEL02_5
									PIXEL12_1
								}

								PIXEL10_1
								PIXEL11
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 182:
							case 150:
								
								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
									PIXEL22_1D
								}
								else
								{
									PIXEL01_1
									PIXEL02_5
									PIXEL12_6
									PIXEL22_2
								}

								PIXEL00_1M
								PIXEL10_1
								PIXEL11
								PIXEL20_2
								PIXEL21_1
								break;
								
							case 213:
							case 212:
								
								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL02_1U
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL02_2
									PIXEL12_6
									PIXEL21_1
									PIXEL22_5
								}

								PIXEL00_2
								PIXEL01_1
								PIXEL10_1
								PIXEL11
								PIXEL20_1M
								break;
								
							case 241:
							case 240:
								
								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL20_1L
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_1
									PIXEL20_2
									PIXEL21_6
									PIXEL22_5
								}

								PIXEL00_2
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								break;
								
							case 236:
							case 232:
								
								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
									PIXEL22_1R
								}
								else
								{
									PIXEL10_1
									PIXEL20_5
									PIXEL21_6
									PIXEL22_2
								}

								PIXEL00_1M
								PIXEL01_1
								PIXEL02_2
								PIXEL11
								PIXEL12_1
								break;
								
							case 109:
							case 105:
								
								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL00_1U
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL00_2
									PIXEL10_6
									PIXEL20_5
									PIXEL21_1
								}

								PIXEL01_1
								PIXEL02_2
								PIXEL11
								PIXEL12_1
								PIXEL22_1M
								break;
								
							case 171:
							case 43:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
									PIXEL20_1D
								}
								else
								{
									PIXEL00_5
									PIXEL01_1
									PIXEL10_6
									PIXEL20_2
								}

								PIXEL02_1M
								PIXEL11
								PIXEL12_1
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 143:
							case 15:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL02_1R
									PIXEL10_C
								}
								else
								{
									PIXEL00_5
									PIXEL01_6
									PIXEL02_2
									PIXEL10_1
								}

								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 124:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1U
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
									PIXEL21_3
								}

								PIXEL22_1M
								break;
								
							case 203:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
									PIXEL10_3
								}

								PIXEL02_1M
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 62:
								
								PIXEL00_1M

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
									PIXEL12_3
								}

								PIXEL10_C
								PIXEL11
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 211:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL20_1M

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 118:
								
								PIXEL00_1M

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
									PIXEL12_3
								}

								PIXEL10_1
								PIXEL11
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 217:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL20_1M

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 110:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1R
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
									PIXEL21_3
								}

								PIXEL22_1M
								break;
								
							case 155:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
									PIXEL10_3
								}

								PIXEL02_1M
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 188:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 185:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 61:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 157:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 103:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 227:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 230:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 199:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 220:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 158:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
									PIXEL12_3
								}

								PIXEL10_C
								PIXEL11
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 234:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1M
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
									PIXEL21_3
								}

								PIXEL22_1R
								break;
								
							case 242:
								
								PIXEL00_1M
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL10_1
								PIXEL11
								PIXEL20_1L

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 59:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
									PIXEL10_3
								}

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 121:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1M
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
									PIXEL21_3
								}

								if (Diff(b.w[5], b.w[7]))								
									PIXEL22_1M								
								else								
									PIXEL22_2
								
								break;
								
							case 87:
								
								PIXEL00_1L

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
									PIXEL12_3
								}

								PIXEL10_1
								PIXEL11
								PIXEL20_1M
								PIXEL21_C
								
								if (Diff(b.w[5], b.w[7]))								
									PIXEL22_1M								
								else								
									PIXEL22_2
								
								break;
								
							case 79:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
									PIXEL10_3
								}

								PIXEL02_1R
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2								
								
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 122:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))								
									PIXEL02_1M								
								else
									PIXEL02_2
																
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
									PIXEL21_3
								}

								if (Diff(b.w[5], b.w[7]))								
									PIXEL22_1M								
								else								
									PIXEL22_2
								
								break;
								
							case 94:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
									PIXEL12_3
								}
								
								PIXEL10_C
								PIXEL11

								if (Diff(b.w[7], b.w[3]))								
									PIXEL20_1M								
								else
									PIXEL20_2
								
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 218:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2								
								
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL10_C
								PIXEL11

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 91:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
									PIXEL10_3
								}

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))								
									PIXEL20_1M								
								else								
									PIXEL20_2
								
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))								
									PIXEL22_1M								
								else								
									PIXEL22_2
								
								break;
								
							case 229:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_2
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 167:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_2
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 173:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_2
								PIXEL10_C
								PIXEL11
								PIXEL12_1
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 181:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 186:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2								

								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))								
									PIXEL02_1M								
								else								
									PIXEL02_2
								
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 115:
								
								PIXEL00_1L
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))								
									PIXEL02_1M								
								else								
									PIXEL02_2
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1L
								PIXEL21_C
								
								if (Diff(b.w[5], b.w[7]))								
									PIXEL22_1M								
								else								
									PIXEL22_2
								
								break;
								
							case 93:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 206:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 205:
							case 201:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_2
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 174:
							case 46:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_C
								PIXEL11
								PIXEL12_1
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 179:
							case 147:
								
								PIXEL00_1L
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 117:
							case 116:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1L
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 189:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 231:
								
								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_1
								PIXEL11
								PIXEL12_1
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 126:
								
								PIXEL00_1M

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
									PIXEL12_3
								}

								PIXEL11

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
									PIXEL21_3
								}

								PIXEL22_1M
								break;
								
							case 219:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
									PIXEL10_3
								}

								PIXEL02_1M
								PIXEL11
								PIXEL20_1M

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 125:
								
								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL00_1U
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL00_2
									PIXEL10_6
									PIXEL20_5
									PIXEL21_1
								}

								PIXEL01_1
								PIXEL02_1U
								PIXEL11
								PIXEL12_C
								PIXEL22_1M
								break;
								
							case 221:
								
								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL02_1U
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL02_2
									PIXEL12_6
									PIXEL21_1
									PIXEL22_5
								}

								PIXEL00_1U
								PIXEL01_1
								PIXEL10_C
								PIXEL11
								PIXEL20_1M
								break;
								
							case 207:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL02_1R
									PIXEL10_C
								}
								else
								{
									PIXEL00_5
									PIXEL01_6
									PIXEL02_2
									PIXEL10_1
								}

								PIXEL11
								PIXEL12_1
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 238:
								
								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
									PIXEL22_1R
								}
								else
								{
									PIXEL10_1
									PIXEL20_5
									PIXEL21_6
									PIXEL22_2
								}

								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1R
								PIXEL11
								PIXEL12_1
								break;
								
							case 190:
								
								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
									PIXEL22_1D
								}
								else
								{
									PIXEL01_1
									PIXEL02_5
									PIXEL12_6
									PIXEL22_2
								}

								PIXEL00_1M
								PIXEL10_C
								PIXEL11
								PIXEL20_1D
								PIXEL21_1
								break;
								
							case 187:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
									PIXEL20_1D
								}
								else
								{
									PIXEL00_5
									PIXEL01_1
									PIXEL10_6
									PIXEL20_2
								}

								PIXEL02_1M
								PIXEL11
								PIXEL12_C
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 243:
								
								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL20_1L
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_1
									PIXEL20_2
									PIXEL21_6
									PIXEL22_5
								}

								PIXEL00_1L
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_1
								PIXEL11
								break;
								
							case 119:
								
								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL00_1L
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL00_2
									PIXEL01_6
									PIXEL02_5
									PIXEL12_1
								}

								PIXEL10_1
								PIXEL11
								PIXEL20_1L
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 237:
							case 233:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_2
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2
								
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 175:
							case 47:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
									PIXEL00_2

								PIXEL01_C
								PIXEL02_1R
								PIXEL10_C
								PIXEL11
								PIXEL12_1
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_2
								break;
								
							case 183:
							case 151:
								
								PIXEL00_1L
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_C
								else
									PIXEL02_2
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_2
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 245:
							case 244:
								
								PIXEL00_2
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1L
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_C
								else
									PIXEL22_2

								break;
								
							case 250:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1M
								PIXEL11

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
								}

								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL22_4
								}
								break;
								
							case 123:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
								}

								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL20_4
									PIXEL21_3
								}

								PIXEL22_1M
								break;
								
							case 95:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL10_3
								}

								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL02_4
									PIXEL12_3
								}

								PIXEL11
								PIXEL20_1M
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 222:
								
								PIXEL00_1M

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
								}

								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1M

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 252:
								
								PIXEL00_1M
								PIXEL01_1
								PIXEL02_1U
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
								}

								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_C
								else
									PIXEL22_2

								break;
								
							case 249:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1M
								PIXEL10_C
								PIXEL11

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2

								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL22_4
								}
								break;
								
							case 235:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
								}

								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2

								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 111:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
									PIXEL00_2

								PIXEL01_C
								PIXEL02_1R
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL20_4
									PIXEL21_3
								}

								PIXEL22_1M
								break;
								
							case 63:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
									PIXEL00_2

								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL02_4
									PIXEL12_3
								}

								PIXEL10_C
								PIXEL11
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1M
								break;
								
							case 159:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL10_3
								}

								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_C
								else
									PIXEL02_2

								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 215:
								
								PIXEL00_1L
								PIXEL01_C

								if (Diff(b.w[1], b.w[5]))
									PIXEL02_C
								else
									PIXEL02_2

								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1M

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 246:
								
								PIXEL00_1M

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
								}

								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1L
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_C
								else
									PIXEL22_2

								break;
								
							case 254:
								
								PIXEL00_1M

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_4
								}

								PIXEL11

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
								}

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL21_3
									PIXEL22_2
								}
								break;
								
							case 253:
								
								PIXEL00_1U
								PIXEL01_1
								PIXEL02_1U
								PIXEL10_C
								PIXEL11
								PIXEL12_C

								if (Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2

								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_C
								else
									PIXEL22_2
								
								break;
								
							case 251:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
								}
								else
								{
									PIXEL00_4
									PIXEL01_3
								}

								PIXEL02_1M
								PIXEL11

								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_2
									PIXEL21_3
								}

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL12_C
									PIXEL22_C
								}
								else
								{
									PIXEL12_3
									PIXEL22_4
								}
								break;
								
							case 239:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_C
								PIXEL11
								PIXEL12_1
								
								if (Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2

								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 127:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL01_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_2
									PIXEL01_3
									PIXEL10_3
								}

								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL02_4
									PIXEL12_3
								}
								
								PIXEL11
								
								if (Diff(b.w[7], b.w[3]))
								{
									PIXEL20_C
									PIXEL21_C
								}
								else
								{
									PIXEL20_4
									PIXEL21_3
								}
								
								PIXEL22_1M
								break;
								
							case 191:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
									PIXEL00_2
								
								PIXEL01_C
								
								if (Diff(b.w[1], b.w[5]))
									PIXEL02_C
								else
									PIXEL02_2

								PIXEL10_C
								PIXEL11
								PIXEL12_C
								PIXEL20_1D
								PIXEL21_1
								PIXEL22_1D
								break;
								
							case 223:
								
								if (Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL10_3
								}
								
								if (Diff(b.w[1], b.w[5]))
								{
									PIXEL01_C
									PIXEL02_C
									PIXEL12_C
								}
								else
								{
									PIXEL01_3
									PIXEL02_2
									PIXEL12_3
								}
								
								PIXEL11
								PIXEL20_1M

								if (Diff(b.w[5], b.w[7]))
								{
									PIXEL21_C
									PIXEL22_C
								}
								else
								{
									PIXEL21_3
									PIXEL22_4
								}
								break;
								
							case 247:
								
								PIXEL00_1L
								PIXEL01_C
								
								if (Diff(b.w[1], b.w[5]))								
									PIXEL02_C								
								else								
									PIXEL02_2								
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1L
								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))								
									PIXEL22_C								
								else								
									PIXEL22_2
								
								break;
								
							case 255:
								
								if (Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
							     	PIXEL00_2
						
								PIXEL01_C
						
								if (Diff(b.w[1], b.w[5]))           
									PIXEL02_C            
								else            
						     		PIXEL02_2
						
								PIXEL10_C
								PIXEL11
								PIXEL12_C
							
								if (Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2

								PIXEL21_C

								if (Diff(b.w[5], b.w[7]))
									PIXEL22_C
								else
									PIXEL22_2
								break;
							
							NST_UNREACHABLE

							#undef PIXEL00_1M 
                            #undef PIXEL00_1U 
							#undef PIXEL00_1L 
							#undef PIXEL00_2  
                            #undef PIXEL00_4  
							#undef PIXEL00_5  
							#undef PIXEL00_C  
							#undef PIXEL01_1  
							#undef PIXEL01_3  
							#undef PIXEL01_6  
							#undef PIXEL01_C  
							#undef PIXEL02_1M 
							#undef PIXEL02_1U 
                            #undef PIXEL02_1R 
							#undef PIXEL02_2  
							#undef PIXEL02_4  
							#undef PIXEL02_5  
                            #undef PIXEL02_C  
							#undef PIXEL10_1  
							#undef PIXEL10_3  
                            #undef PIXEL10_6  
							#undef PIXEL10_C  
							#undef PIXEL11    
                            #undef PIXEL12_1  
							#undef PIXEL12_3  
							#undef PIXEL12_6  
							#undef PIXEL12_C  
							#undef PIXEL20_1M 
							#undef PIXEL20_1D 
							#undef PIXEL20_1L 
							#undef PIXEL20_2  
							#undef PIXEL20_4  
							#undef PIXEL20_5  
                            #undef PIXEL20_C  
							#undef PIXEL21_1  
							#undef PIXEL21_3  
                            #undef PIXEL21_6  
							#undef PIXEL21_C  
							#undef PIXEL22_1M 
                            #undef PIXEL22_1D 
							#undef PIXEL22_1R 
							#undef PIXEL22_2  
							#undef PIXEL22_4  
                            #undef PIXEL22_5  
                            #undef PIXEL22_C  
						}
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[0]) + pitch);
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[1]) + pitch);
					dst[2] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[2]) + pitch);
				}
			}

			template<typename T>
			NST_FORCE_INLINE void Renderer::FilterHqX::Blit2x(const Input& input,const Output& output) const
			{
				Blit2xRgb<T,0xFF0000UL,0x00FF00UL,0x0000FFUL>( input, output );
			}

			template<>
			NST_FORCE_INLINE void Renderer::FilterHqX::Blit2x<u16>(const Input& input,const Output& output) const
			{
				if (format.left[0] == 11)
					Blit2xRgb<u16,0xF800U,0x07E0U,0x001FU>( input, output );
				else
					Blit2xRgb<u16,0x7C00U,0x03E0U,0x001FU>( input, output );
			}

			template<typename T>
			NST_FORCE_INLINE void Renderer::FilterHqX::Blit3x(const Input& input,const Output& output) const
			{
				Blit3xRgb<T,0xFF0000UL,0x00FF00UL,0x0000FFUL>( input, output );
			}

			template<>
			NST_FORCE_INLINE void Renderer::FilterHqX::Blit3x<u16>(const Input& input,const Output& output) const
			{
				if (format.left[0] == 11)
					Blit3xRgb<u16,0xF800U,0x07E0U,0x001FU>( input, output );
				else
					Blit3xRgb<u16,0x7C00U,0x03E0U,0x001FU>( input, output );
			}

			template<typename T>
			NST_FORCE_INLINE void Renderer::FilterHqX::BlitType(const Input& input,const Output& output) const
			{	
				switch (type)
				{
					case RenderState::FILTER_HQ2X:
				
						Blit2x<T>( input, output );
						break;
				
					case RenderState::FILTER_HQ3X:
				
						Blit3x<T>( input, output );
						break;
				
						NST_UNREACHABLE
				}
			}

			void Renderer::FilterHqX::Blit(const Input& input,const Output& output)
			{
				switch (bpp)
				{
					case 32: BlitType<u32>( input, output ); break;
					case 16: BlitType<u16>( input, output ); break;
				
					NST_UNREACHABLE
				}
			}
		}
	}
}

#endif
