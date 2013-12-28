////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

////////////////////////////////////////////////////////////////////////////////////////
//
// Class functions: Blit2xSaI(), BlitSuper2xSaI() and BlitSuperEagle() contains
// derivate work of "2xSaI : The advanced 2x Scale and Interpolation engine"
// originally written by Derek Liauw Kie Fa.
//
// Copyright notice:
// --------------------------------------------------------------------
// The following (piece of) code, (part of) the 2xSaI engine,          
// copyright (c) 2001 by Derek Liauw Kie Fa.                           
// Non-Commercial use of the engine is allowed and is encouraged,      
// provided that appropriate credit be given and that this copyright   
// notice will not be removed under any circumstance.                  
// You may freely modify this code, but I request                      
// that any improvements to the engine be submitted to me, so          
// that I can implement these improvements in newer versions of        
// the engine.                                                         
// If you need more information, have any comments or suggestions,     
// you can e-mail me. My e-mail: DerekL666@yahoo.com                   
// --------------------------------------------------------------------
//
// 2xSaI homepage: http://elektron.its.tudelft.nl/~dalikifa/
//
// Changes: - indexed palette conversions
//			- bug fix, out of bounds memory reading removed
//
// 2005-04-12 / Martin Freij
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// Class functions: BlitScale2x() and BlitScale3x() contains derivate work of
// "Scale2x" originally written by Andrea Mazzoleni under the GPL licence.
// 
// Copyright notice:
// --------------------------------------------------------------------
// Copyright (C) 2001, 2002, 2003, 2004 Andrea Mazzoleni
// --------------------------------------------------------------------
//
// Scale2x homepage: http://scale2x.sourceforge.net/
//
// Changes:	- indexed palette conversions
//            optimizations
//
// 2005-04-13 / Martin Freij
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// Class functions: BlitHq2x() and BlitHq3x() contains derivate work of
// "hq2x" and "hq3x" originally written by Maxim Stepin under the LGPL licence.
// 
// Copyright notice:
// --------------------------------------------------------------------
// Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
// --------------------------------------------------------------------
//
// hq2x/hq3x homepage: http://www.hiend3d.com
//
// Changes:	- indexed palette conversions
//          - added 555 and 565 pixel format support
//          - optimizations
//
// 2005-09-18 / Martin Freij
//
////////////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include "NstCore.hpp"
#include "api/NstApiVideo.hpp"
#include "NstVideoRenderer.hpp"

#ifdef __INTEL_COMPILER
#pragma warning( disable : 279 )
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
          #ifndef NST_NO_HQ2X

			Renderer::Hq2x::Hq2x()
			: yuv(NULL), rgb(NULL) {}

			Renderer::Hq2x::~Hq2x()
			{
				Uninitialize();
			}

			void Renderer::Hq2x::Initialize(const uint bpp,const uint (&shifts)[3])
			{
				if (yuv == NULL)
					yuv = new u32 [0x10000UL];

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

				if (bpp == 32)
				{
					NST_ASSERT( shifts[0] == 11 && shifts[1] == 5 && shifts[2] == 0 );

					if (rgb == NULL)
						rgb = new u32 [0x10000UL];

					for (dword i=0; i < 0x10000UL; ++i)
						rgb[i] = ((i & 0xF800UL) << 8) | ((i & 0x07E0UL) << 5) | ((i & 0x001FUL) << 3);
				}
				else if (rgb)
				{
					delete [] rgb;
					rgb = NULL;
				}
			}

			void Renderer::Hq2x::Uninitialize()
			{
				delete [] yuv;
				yuv = NULL;
				delete [] rgb;
				rgb = NULL;
			}

			Renderer::Hq2x Renderer::hq2x;

          #endif

            #define NES_PIXELS (WIDTH * HEIGHT)

			Result Renderer::SetState(const Api::Video::RenderState& input)
			{
				switch (input.bits.count)
				{
					case 8:

						if (input.scale == 1 && input.filter == Api::Video::RenderState::FILTER_NONE)
						{
							state.bpp = 8;
							state.paletteOffset = input.paletteOffset;
							return RESULT_OK;
						}
						break;

					case 16:
					case 32:
					
						state.scale = 0;

						switch (input.filter)
						{
							case Api::Video::RenderState::FILTER_NONE:

								if (input.scale == 1)
									state.scale = 1;

								break;
						
							case Api::Video::RenderState::FILTER_TV: 
                        #ifndef NST_NO_HQ2X
							case Api::Video::RenderState::FILTER_HQ2X:

								if (!Hq2x::IsSupported( input.bits.count, input.bits.mask.r, input.bits.mask.g, input.bits.mask.b ))
									break;
                        #endif
                        #ifndef NST_NO_SCALE2X 
							case Api::Video::RenderState::FILTER_SCALE2X:
                        #endif
                        #ifndef NST_NO_2XSAI
							case Api::Video::RenderState::FILTER_2XSAI: 
							case Api::Video::RenderState::FILTER_SUPER_2XSAI: 
							case Api::Video::RenderState::FILTER_SUPER_EAGLE: 
                        #endif								
								if (input.scale == 2)
									state.scale = 2;

								break;
									
                        #if !defined(NST_NO_HQ2X) || !defined(NST_NO_SCALE2X)
                         #ifndef NST_NO_HQ2X
							case Api::Video::RenderState::FILTER_HQ3X:

								if (!Hq2x::IsSupported( input.bits.count, input.bits.mask.r, input.bits.mask.g, input.bits.mask.b ))
									break;

                         #endif
                         #ifndef NST_NO_SCALE2X 
							case Api::Video::RenderState::FILTER_SCALE3X:
                         #endif
								if (input.scale == 3)
									state.scale = 3;

								break;
                        #endif

							case Api::Video::RenderState::FILTER_SCANLINES_BRIGHT:
							case Api::Video::RenderState::FILTER_SCANLINES_DARK:

								if (input.scale == 1 || input.scale == 2)
									state.scale = input.scale;

								break;
						}

						if (state.scale)
						{
							state.bpp = input.bits.count;
							state.filter = input.filter;

							format.mask[0] = input.bits.mask.r;
							format.mask[1] = input.bits.mask.g;
							format.mask[2] = input.bits.mask.b;

							for (uint i=0; i < 3; ++i)
							{
								format.left[i] = 0; 

								if (format.mask[i])
								{
									while (!(format.mask[i] & (0x1UL << format.left[i])))
										++format.left[i];
								}

								for 
								(
					 				format.right[i] = 0; 
				     				format.right[i] < 8 && (format.mask[i] & (0x1UL << (format.left[i] + format.right[i]))); 
				    				format.right[i] += 1
								);

								format.right[i] = 8 - format.right[i];
							}

							format.lsb[0] = ~((1UL << format.left[0]) | (1UL << format.left[1]) | (1UL << format.left[2]));
							format.lsb[1] = ~((3UL << format.left[0]) | (3UL << format.left[1]) | (3UL << format.left[2])); 

							UpdatePalette();

							return RESULT_OK;
						}
				}

				state.bpp = 0;

				return RESULT_ERR_UNSUPPORTED;
			}
				
			Result Renderer::GetState(Api::Video::RenderState& output) const
			{
				if (state.bpp)
				{
					output.bits.count = state.bpp;

					if (state.bpp == 8)
					{
						output.bits.mask.r = 0;
						output.bits.mask.g = 0;
						output.bits.mask.b = 0;
						output.paletteOffset = state.paletteOffset;
						output.filter = Api::Video::RenderState::FILTER_NONE;
						output.scale = 1;
					}
					else
					{
						output.bits.mask.r = format.mask[0];
						output.bits.mask.g = format.mask[1];
						output.bits.mask.b = format.mask[2];
						output.paletteOffset = 0;
						output.filter = (Api::Video::RenderState::Filter) state.filter;
						output.scale = state.scale;
					}
		
					return RESULT_OK;
				}
		
				return RESULT_ERR_NOT_READY;
			}
		
			void Renderer::UpdatePalette()
			{
				NST_ASSERT( srcPalette );
		
				if (state.bpp >= 16)
				{
                    #ifndef NST_NO_HQ2X					
					if (state.filter == Api::Video::RenderState::FILTER_HQ2X || state.filter == Api::Video::RenderState::FILTER_HQ3X)
					{
						uint rgb[2][3];

						if (state.bpp == 16)
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

						for (uint i=0; i < PALETTE_ENTRIES; ++i)
						{
							palette[i] = 
							(
								((srcPalette[i][0] >> rgb[0][0]) << rgb[1][0]) |
								((srcPalette[i][1] >> rgb[0][1]) << rgb[1][1]) |
								((srcPalette[i][2] >> rgb[0][2]) << rgb[1][2])
							);
						}

						hq2x.Initialize( state.bpp, rgb[1] );
						return;
					}
                    #endif

					for (uint i=0; i < PALETTE_ENTRIES; ++i)
					{
						palette[i] = 
						(
      						((srcPalette[i][0] >> format.right[0]) << format.left[0]) |
     						((srcPalette[i][1] >> format.right[1]) << format.left[1]) |
     						((srcPalette[i][2] >> format.right[2]) << format.left[2])
						);
					}
				}

                #ifndef NST_NO_HQ2X					
				hq2x.Uninitialize();
                #endif
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			void Renderer::Blit(Output& video) const
			{
				if (state.bpp && Output::lockCallback( video ))
				{
					NST_ASSERT( video.pixels && video.pitch );

					if (ulong(std::labs( video.pitch )) >= state.bpp * (WIDTH / 8U))
					{
						if (state.bpp == 16)
						{
							BlitFilter<u16>( video );
						}
						else if (state.bpp == 32)
						{
							BlitFilter<u32>( video );
						}
						else
						{
							BlitPaletteIndexed( video );
						}
					}

					Output::unlockCallback( video );
				}
			}
		
			template<typename T>
			void Renderer::BlitFilter(const Output& video) const
			{
				switch (state.filter)
				{
					case Api::Video::RenderState::FILTER_NONE:
				
						if (video.pitch == WIDTH * sizeof(T))
							BlitAligned( static_cast<T*>(video.pixels) );
						else
							BlitUnaligned<T>( video );
						break;
				
					case Api::Video::RenderState::FILTER_SCANLINES_BRIGHT: 
					case Api::Video::RenderState::FILTER_SCANLINES_DARK: 
				
						if (state.scale == 2)
							BlitScanlines2x<T>( video );
						else
							BlitScanlines1x<T>( video );
						break;
				
					case Api::Video::RenderState::FILTER_TV: 
				
						BlitTV<T>( video );
						break;

                #ifndef NST_NO_2XSAI

					case Api::Video::RenderState::FILTER_2XSAI:

						Blit2xSaI<T>( video );
						break;

					case Api::Video::RenderState::FILTER_SUPER_2XSAI:

						BlitSuper2xSaI<T>( video );
						break;

					case Api::Video::RenderState::FILTER_SUPER_EAGLE:

						BlitSuperEagle<T>( video );
						break;
				
                #endif
                #ifndef NST_NO_SCALE2X

					case Api::Video::RenderState::FILTER_SCALE2X:
				
						BlitScale2x<T>( video );
						break;
				
					case Api::Video::RenderState::FILTER_SCALE3X:
				
						BlitScale3x<T>( video );
						break;

                #endif
                #ifndef NST_NO_HQ2X

					case Api::Video::RenderState::FILTER_HQ2X:

						BlitHq2x<T>( video );
						break;

					case Api::Video::RenderState::FILTER_HQ3X:

						BlitHq3x<T>( video );
						break;

                #endif

					NST_UNREACHABLE
				}
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("w", on)
            #endif
		
			void Renderer::BlitPaletteIndexed(const Output& video) const
			{
				NST_ASSERT( state.scale == 1 );
		
				u8* NST_RESTRICT dst = static_cast<u8*>(video.pixels);
				
				if (video.pitch == WIDTH)
				{
					if (const uint offset = state.paletteOffset)
					{
						for (uint i=0; i < NES_PIXELS; ++i)
							dst[i] = offset + screen[i];
					}
					else
					{
						for (uint i=0; i < NES_PIXELS; ++i)
							dst[i] = screen[i];
					}
				}
				else
				{
					const u16* NST_RESTRICT src = screen;
					const long pitch = video.pitch;

					if (const uint offset = state.paletteOffset)
					{
						for (uint y=0; y < HEIGHT; ++y)
						{
							for (uint x=0; x < WIDTH; ++x)
								dst[x] = offset + src[x];
		
							dst += pitch;
							src += WIDTH;
						}
					}
					else
					{
						for (uint y=0; y < HEIGHT; ++y)
						{
							for (uint x=0; x < WIDTH; ++x)
								dst[x] = src[x];
		
							dst += pitch;
							src += WIDTH;
						}
					}
				}
			}
		
			template<typename T>
			void Renderer::BlitAligned(T* NST_RESTRICT dst) const
			{
				NST_ASSERT( state.scale == 1 );
		
				for (uint i=0; i < NES_PIXELS; ++i)
					dst[i] = palette[screen[i]];
			}
		
			template<typename T>
			void Renderer::BlitUnaligned(const Output& video) const
			{
				NST_ASSERT( state.scale == 1 );
		
				const u16* NST_RESTRICT src = screen;
				T* NST_RESTRICT dst = static_cast<T*>(video.pixels);
		
				const long pitch = video.pitch - long(sizeof(T) * WIDTH);
		
				for (uint y=0; y < HEIGHT; ++y)
				{
					for (uint x=0; x < WIDTH; ++x)
						dst[x] = palette[src[x]];

					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
					src += WIDTH;
				}
			}

			template<typename T> 
			void Renderer::BlitScanlines2x(const Output& video) const
			{
				NST_ASSERT( state.scale == 2 );

				const u16* NST_RESTRICT src = screen;
				T* NST_RESTRICT dst = static_cast<T*>(video.pixels);
				
				const uint darken = (state.filter == Api::Video::RenderState::FILTER_SCANLINES_DARK) + 1;		
				const dword zeroLow = format.lsb[darken-1];
				const long pitch = video.pitch;

				for (uint y=0; y < HEIGHT; ++y)
				{
					register dword p;

					for (uint x=0; x < WIDTH; ++x)
					{
						dst[x*2+0] = p = palette[src[x]];
						dst[x*2+1] = p;
					}

					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);

					for (uint x=0; x < WIDTH; ++x)
					{
						dst[x*2+0] = p = (palette[src[x]] & zeroLow) >> darken;
						dst[x*2+1] = p;
					}

					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
					src += WIDTH;
				}
			}

			template<typename T> 
			void Renderer::BlitScanlines1x(const Output& video) const
			{
				NST_ASSERT( state.scale == 1 );

				const u16* NST_RESTRICT src = screen;
				T* NST_RESTRICT dst = static_cast<T*>(video.pixels);

				const uint darken = (state.filter == Api::Video::RenderState::FILTER_SCANLINES_DARK) + 1;		
				const dword zeroLow = format.lsb[darken-1];
				const long pitch = video.pitch;

				for (uint y=0; y < HEIGHT; y += 2)
				{
					for (uint x=0; x < WIDTH; ++x)
						dst[x] = palette[src[x]];

					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
					src += WIDTH;

					for (uint x=0; x < WIDTH; ++x)
						dst[x] = (palette[src[x]] & zeroLow) >> darken;

					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
					src += WIDTH;
				}
			}

			template<typename T>
			void Renderer::BlitTV(const Output& video) const
			{
				NST_ASSERT( state.scale == 2 );

				const u16* NST_RESTRICT src = screen;
				T* NST_RESTRICT dst = static_cast<T*>(video.pixels);

				const long pitch = video.pitch;

				for (uint y=0; y < HEIGHT; ++y)
				{
					dword p, q;

					p = palette[src[0]];

					for (uint x=0; x < WIDTH-1; ++x)
					{
						dst[x*2+0] = p;

						q = p;
						p = palette[src[x+1]];

						dst[x*2+1] =
						(
							((((q & format.mask[0]) + (p & format.mask[0])) >> 1) & format.mask[0]) | 
							((((q & format.mask[1]) + (p & format.mask[1])) >> 1) & format.mask[1]) | 
							((((q & format.mask[2]) + (p & format.mask[2])) >> 1) & format.mask[2])
						);
					}

					dst[WIDTH*2-2] = p;
					dst[WIDTH*2-1] = p;

					const u16* const NST_RESTRICT next = src + (y < HEIGHT-1 ? WIDTH+1 : 1);
					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
					
					q = palette[next[0]];

					for (uint x=0; x < WIDTH-2; ++x)
					{
						p = palette[src[x]];

						dst[x*2+0] =
						(
							(((((p & format.mask[0]) + (q & format.mask[0])) * 11) >> 5) & format.mask[0]) | 
							(((((p & format.mask[1]) + (q & format.mask[1])) * 11) >> 5) & format.mask[1]) | 
							(((((p & format.mask[2]) + (q & format.mask[2])) * 11) >> 5) & format.mask[2])
						);

						q = palette[next[x+1]];
		
						dst[x*2+1] =
						(
							(((((p & format.mask[0]) + (q & format.mask[0])) * 11) >> 5) & format.mask[0]) | 
							(((((p & format.mask[1]) + (q & format.mask[1])) * 11) >> 5) & format.mask[1]) | 
							(((((p & format.mask[2]) + (q & format.mask[2])) * 11) >> 5) & format.mask[2])
						);
					}

					for (uint x=WIDTH-2; x < WIDTH; ++x)
					{
						p = palette[src[x]];
					
						dst[x*2+0] = p =
						(
							(((((p & format.mask[0]) + (q & format.mask[0])) * 11) >> 5) & format.mask[0]) | 
							(((((p & format.mask[1]) + (q & format.mask[1])) * 11) >> 5) & format.mask[1]) | 
							(((((p & format.mask[2]) + (q & format.mask[2])) * 11) >> 5) & format.mask[2])
						);
					
						dst[x*2+1] = p;
					}

					src += WIDTH;
					dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst) + pitch);
				}
			}

        #ifndef NST_NO_SCALE2X

			template<typename T,int PREV,int NEXT>
			NST_FORCE_INLINE T* Renderer::Scale2xBorder(T* NST_RESTRICT dst,const u16* NST_RESTRICT src) const
			{
				{
					dword p[4] =
					{
						palette[src[ PREV ]],
						palette[src[ 0    ]],
						palette[src[ 1    ]],
						palette[src[ NEXT ]]
					};

					if (p[0] != p[3] && p[2] != p[1] && p[2] == p[0])
						p[1] = p[0];

					dst[0] = p[1];
					dst[1] = p[1];
				}

				src += 1;
				dst += 2;

				for (uint x=0; x < WIDTH-2; ++x)
				{
					const dword p[4] =
					{
						palette[src[  PREV ]],
						palette[src[ -1    ]],
						palette[src[  0    ]],
						palette[src[  1    ]]
					};

					if (p[0] != p[1] && p[1] != p[3]) 
					{
						dst[0] = p[1] == p[0] ? p[0] : p[2];
						dst[1] = p[3] == p[0] ? p[0] : p[2];
					} 
					else 
					{
						dst[0] = p[2];
						dst[1] = p[2];
					}

					src += 1;
					dst += 2;
				}

				const dword p[4] = 
				{
					palette[src[  PREV ]],
					palette[src[ -1    ]],
					palette[src[  0    ]],
					palette[src[  NEXT ]]
				};

				if (p[0] != p[3] && p[1] != p[2]) 
				{
					dst[0] = p[1] == p[0] ? p[0] : p[2];
					dst[1] = p[2] == p[0] ? p[0] : p[2];
				} 
				else 
				{
					dst[0] = p[2];
					dst[1] = p[2];
				}

				return dst + 2;
			}

			template<typename T,int PREV,int NEXT>
			NST_FORCE_INLINE T* Renderer::Scale3xBorder(T* NST_RESTRICT dst,const u16* NST_RESTRICT src) const
			{
				{
					const dword p = palette[src[0]];

					dst[0] = p;
					dst[1] = p;

					const dword q = palette[src[PREV]];

					dst[2] = (q != palette[src[1]] && q != palette[src[NEXT]]) ? q : p;
				}

				src += 1;
				dst += 3;

				for (uint x=0; x < WIDTH-2; ++x)
				{
					const dword p[5] =
					{
						palette[src[  PREV ]],
						palette[src[ -1    ]],
						palette[src[  0    ]],
						palette[src[  1    ]],
						palette[src[  NEXT ]]
					};

					dst[0] = (p[1] == p[0] && p[4] != p[0] && p[3] != p[0]) ? p[0] : p[2];
					dst[1] = p[2];
					dst[2] = (p[3] == p[0] && p[4] != p[0] && p[1] != p[0]) ? p[0] : p[2];

					src += 1;
					dst += 3;
				}

				const dword p[2] =
				{
					palette[src[ PREV ]],
					palette[src[ 0    ]]
				};

				dst[0] = p[p[0] != palette[src[-1]] || p[0] == palette[src[NEXT]]];
				dst[1] = p[1];
				dst[2] = p[1];

				return dst + 3;
			}

			template<typename T>
			NST_FORCE_INLINE T* Renderer::Scale3xCenter(T* NST_RESTRICT dst,const u16* NST_RESTRICT src) const
			{
				for (uint x=0; x < WIDTH; ++x)
				{
					const dword p = palette[*src++];
					
					dst[0] = p;
					dst[1] = p;
					dst[2] = p;

					dst += 3;
				}

				return dst;
			}

			template<typename T,int PREV,int NEXT>
			NST_FORCE_INLINE T* Renderer::Scale2xLine(T* dst,const u16* const src,const long pad) const
			{
				dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(Scale2xBorder<T,PREV,NEXT>( dst, src )) + pad);
				dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(Scale2xBorder<T,NEXT,PREV>( dst, src )) + pad);

				return dst;
 			}

			template<typename T,int PREV,int NEXT>
			NST_FORCE_INLINE T* Renderer::Scale3xLine(T* dst,const u16* const src,const long pad) const
			{
				dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(Scale3xBorder<T,PREV,NEXT>( dst, src )) + pad);
				dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(Scale3xCenter<T>( dst, src )) + pad);
				dst = reinterpret_cast<T*>(reinterpret_cast<u8*>(Scale3xBorder<T,NEXT,PREV>( dst, src )) + pad);
				
				return dst;
 			}

			template<typename T>
			void Renderer::BlitScale2x(const Output& video) const
			{
				NST_ASSERT( state.scale == 2 );

				const u16* src = screen;
				T* dst = static_cast<T*>(video.pixels);
				const long pad = video.pitch - long(sizeof(T) * WIDTH_2);

				dst = Scale2xLine<T,-0,+WIDTH>( dst, src, pad );

				for (uint y=0; y < HEIGHT-2; ++y)
					dst = Scale2xLine<T,-WIDTH,+WIDTH>( dst, src += WIDTH, pad );

				Scale2xLine<T,-WIDTH,+0>( dst, src + WIDTH, pad );
			}
		
			template<typename T>
			void Renderer::BlitScale3x(const Output& video) const
			{
				NST_ASSERT( state.scale == 3 );

				const u16* src = screen;
				T* dst = static_cast<T*>(video.pixels);
				const long pad = video.pitch - long(sizeof(T) * WIDTH_3);

				dst = Scale3xLine<T,-0,+WIDTH>( dst, src, pad );

				for (uint y=0; y < HEIGHT-2; ++y)
					dst = Scale3xLine<T,-WIDTH,+WIDTH>( dst, src += WIDTH, pad );

				Scale3xLine<T,-WIDTH,+0>( dst, src + WIDTH, pad );
			}
		
        #endif
        #ifndef NST_NO_2XSAI

			inline dword Renderer::Blend(dword a,dword b) const
			{
				return (a != b) ? ((a & format.lsb[0]) >> 1) + ((b & format.lsb[0]) >> 1) + (a & b & ~format.lsb[0]) : a;
			}

			inline dword Renderer::Blend(dword a,dword b,dword c,dword d) const
			{
				return
				(
					(((a & format.lsb[1]) >> 2) + ((b & format.lsb[1]) >> 2) + ((c & format.lsb[1]) >> 2) + ((d & format.lsb[1]) >> 2)) +
					((((a & ~format.lsb[1]) + (b & ~format.lsb[1]) + (c & ~format.lsb[1]) + (d & ~format.lsb[1])) >> 2) & ~format.lsb[1])
				);
			}

			template<typename T>
			void Renderer::Blit2xSaI(const Output& video) const
			{
				NST_ASSERT( state.scale == 2 );

				const u16* NST_RESTRICT src = screen;
				const long pitch = video.pitch;

				T* NST_RESTRICT dst[2] = 
				{
					static_cast<T*>(video.pixels), 
					reinterpret_cast<T*>(reinterpret_cast<u8*>(video.pixels) + pitch)
				};

				dword a,b,c,d,e=0,f=0,g,h,i=0,j=0,k,l,m,n,o;

				for (uint y=0; y < HEIGHT; ++y)
				{
					for (uint x=0; x < WIDTH; ++x, ++src, dst[0] += 2, dst[1] += 2)
					{
						if (y)
						{
							i = x > 0 ?       palette[src[ -WIDTH-1 ]] : 0;
							e =               palette[src[ -WIDTH   ]];
							f = x < WIDTH-1 ? palette[src[ -WIDTH+1 ]] : 0;
							j = x < WIDTH-2 ? palette[src[ -WIDTH+2 ]] : 0;
						}

						g = x > 0 ?       palette[src[ -1 ]] : 0;
						a =               palette[src[  0 ]];
						b = x < WIDTH-1 ? palette[src[  1 ]] : 0;
						k = x < WIDTH-2 ? palette[src[  2 ]] : 0;
						
						if (y < HEIGHT-1)
						{
							h = x > 0 ?       palette[src[ WIDTH-1 ]] : 0;
							c =               palette[src[ WIDTH   ]];
							d = x < WIDTH-1 ? palette[src[ WIDTH+1 ]] : 0;
							l = x < WIDTH-2 ? palette[src[ WIDTH+2 ]] : 0;
                            
							if (y < HEIGHT-2)
							{
								m = x > 0 ?       palette[src[ WIDTH_2-1 ]] : 0;
								n =               palette[src[ WIDTH_2   ]];
								o = x < WIDTH-1 ? palette[src[ WIDTH_2+1 ]] : 0;
							}
							else
							{
								m = n = o = 0;
							}
						}
						else
						{
							h = c = d = l = m = n = o = 0;
						}

						dword q[3];

						if (a == d && b != c)
						{
							if ((a == e && b == l) || (a == c && a == f && b != e && b == j))
							{
								q[0] = a;
							}
							else
							{
								q[0] = Blend( a, b );
							}
					
							if ((a == g && c == o) || (a == b && a == h && g != c && c == m))
							{
								q[1] = a;
							}
							else
							{
								q[1] = Blend( a, c );
							}
							
							q[2] = a;
						}
						else if (b == c && a != d)
						{
							if ((b == f && a == h) || (b == e && b == d && a != f && a == i))
							{
								q[0] = b;
							}
							else
							{
								q[0] = Blend( a, b );
							}

							if ((c == h && a == f) || (c == g && c == d && a != h && a == i))
							{
								q[1] = c;
							}
							else
							{
								q[1] = Blend( a, c );
							}
							
							q[2] = b;
						}
						else if (a == d && b == c)
						{
							if (a == b)
							{
								q[0] = a;
								q[1] = a;
								q[2] = a;
							}
							else
							{
								q[1] = Blend( a, c );
								q[0] = Blend( a, b );
								
								const int result = 
								(
							     	(a == g && a == e ? -1 : b == g && b == e ? +1 : 0) +
									(b == k && b == f ? -1 : a == k && a == f ? +1 : 0) +
									(b == h && b == n ? -1 : a == h && a == n ? +1 : 0) +
									(a == l && a == o ? -1 : b == l && b == o ? +1 : 0)
								);

								if (result > 0)
								{
									q[2] = a;
								}
								else if (result < 0)
								{
									q[2] = b;
								}
								else
								{
									q[2] = Blend( a, b, c, d );
								}
							}
						}
						else
						{
							q[2] = Blend( a, b, c, d );

							if (a == c && a == f && b != e && b == j)
							{
								q[0] = a;
							}
							else if (b == e && b == d && a != f && a == i)
							{
								q[0] = b;
							}
							else
							{
								q[0] = Blend( a, b );
							}

							if (a == b && a == h && g != c && c == m)
							{
								q[1] = a;
							}
							else if (c == g && c == d && a != h && a == i)
							{
								q[1] = c;
							}
							else
							{
								q[1] = Blend( a, c );
							}
						}

						dst[0][0] = a;
						dst[0][1] = q[0];
						dst[1][0] = q[1];
						dst[1][1] = q[2];
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[1]) + (pitch - long(sizeof(T) * WIDTH_2)));
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[0]) + pitch);
				}
			}

			template<typename T>
			void Renderer::BlitSuper2xSaI(const Output& video) const
			{
				NST_ASSERT( state.scale == 2 );

				const u16* NST_RESTRICT src = screen;
				const long pitch = video.pitch;

				T* NST_RESTRICT dst[2] = 
				{
					static_cast<T*>(video.pixels), 
					reinterpret_cast<T*>(reinterpret_cast<u8*>(video.pixels) + pitch)
				};

				dword a,b,c,d,e,f,g,h,i,j,k=0,l=0,m=0,n=0,o,p;

				for (uint y=0; y < HEIGHT; ++y)
				{
					for (uint x=0; x < WIDTH; ++x, ++src, dst[0] += 2, dst[1] += 2)
					{
						if (y)
						{
							k = x > 0 ?       palette[src[ -WIDTH-1 ]] : 0;
							l =               palette[src[ -WIDTH   ]];
							m = x < WIDTH-1 ? palette[src[ -WIDTH+1 ]] : 0;
							n = x < WIDTH-2 ? palette[src[ -WIDTH+2 ]] : 0;
						}

						d = x > 0 ?       palette[src[ -1 ]] : 0;
						e =               palette[src[  0 ]];
						f = x < WIDTH-1 ? palette[src[  1 ]] : 0;
						p = x < WIDTH-2 ? palette[src[  2 ]] : 0;

						if (y < HEIGHT-1)
						{
							a = x > 0 ?       palette[src[ WIDTH-1 ]] : 0;
							b =               palette[src[ WIDTH   ]];
							c = x < WIDTH-1 ? palette[src[ WIDTH+1 ]] : 0;
							o = x < WIDTH-2 ? palette[src[ WIDTH+2 ]] : 0;
                            
							if (y < HEIGHT-2)
							{
								g = x > 0 ?       palette[src[ WIDTH_2-1 ]] : 0;
								h =               palette[src[ WIDTH_2   ]];
								i = x < WIDTH-1 ? palette[src[ WIDTH_2+1 ]] : 0;
								j = x < WIDTH-2 ? palette[src[ WIDTH_2+2 ]] : 0;
							}
							else
							{
								g = h = i = j = 0;
							}
						}
						else
						{
							a = b = c = o = g = h = i = j = 0;
						}

						dword q[4];

						if (b == f && e != c)
						{
							q[3] = q[1] = b;
						}
						else if (e == c && b != f)
						{
							q[3] = q[1] = e;
						}
						else if (e == c && b == f && e != f)
						{
							const int result = 
							(
							 	(f == a && f == h ? -1 : e == a && e == h ? +1 : 0) +
								(f == d && f == l ? -1 : e == d && e == l ? +1 : 0) +
								(f == i && f == o ? -1 : e == i && e == o ? +1 : 0) +
								(f == m && f == p ? -1 : e == m && e == p ? +1 : 0)
							);

							if (result > 0)
							{
								q[3] = q[1] = f;
							}
							else if (result < 0)
							{
								q[3] = q[1] = e;
							}
							else
							{
								q[3] = q[1] = Blend( e, f );
							}
						}
						else
						{
							if (f == c && c == h && b != i && c != g)
							{
								q[3] = Blend( c, c, c, b );
							}
							else if (e == b && b == i && h != c && b != j)
							{
								q[3] = Blend( b, b, b, c );
							}
							else
							{
								q[3] = Blend( b, c );
							}

							if (f == c && f == l && e != m && f != k)
							{
								q[1] = Blend( f, f, f, e );
							}
							else if (e == b && e == m && l != f && e != n)
							{
								q[1] = Blend( f, e, e, e );
							}
							else
							{
								q[1] = Blend( e, f );
							}
						}

						if (e == c && b != f && d == e && e != i)
						{
							q[2] = Blend( b, e );
						}
						else if (e == a && f == e && d != b && e != g)
						{
							q[2] = Blend( b, e );
						}
						else
						{
							q[2] = b;
						}

						if (b == f && e != c && a == b && b != m)
						{
							q[0] = Blend( b, e );
						}
						else if (d == b && c == b && a != e && b != k)
						{
							q[0] = Blend( b, e );
						}
						else
						{
							q[0] = e;
						}

						dst[0][0] = q[0];
						dst[0][1] = q[1];
						dst[1][0] = q[2];
						dst[1][1] = q[3];
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[1]) + (pitch - long(sizeof(T) * WIDTH_2)));
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[0]) + pitch);
				}
			}
				    
			template<typename T>
			void Renderer::BlitSuperEagle(const Output& video) const
			{
				NST_ASSERT( state.scale == 2 );

				const u16* NST_RESTRICT src = screen;
				const long pitch = video.pitch;

				T* NST_RESTRICT dst[2] = 
				{
					static_cast<T*>(video.pixels), 
					reinterpret_cast<T*>(reinterpret_cast<u8*>(video.pixels) + pitch)
				};

				dword a,b,c,d,e,f,g,h,i=0,j=0,k,l;

				for (uint y=0; y < HEIGHT; ++y)
				{
					for (uint x=0; x < WIDTH; ++x, ++src, dst[0] += 2, dst[1] += 2)
					{
						if (y)
						{
							i = palette[src[ -WIDTH   ]];
							j = palette[src[ -WIDTH+1 ]];
						}

						d = x > 0 ?       palette[src[ -1 ]] : 0;
						e =               palette[src[  0 ]];
						f = x < WIDTH-1 ? palette[src[  1 ]] : 0;
						l = x < WIDTH-2 ? palette[src[  2 ]] : 0;

						if (y < HEIGHT-1)
						{
							a = x > 0 ?       palette[src[ WIDTH-1 ]] : 0;
							b =               palette[src[ WIDTH   ]];
							c = x < WIDTH-1 ? palette[src[ WIDTH+1 ]] : 0;
							k = x < WIDTH-2 ? palette[src[ WIDTH+2 ]] : 0;
                            
							if (y < HEIGHT-2)
							{
								g =               palette[src[ WIDTH_2   ]];
					     		h = x < WIDTH-1 ? palette[src[ WIDTH_2+1 ]] : 0;
							}
							else
							{
								g = h = 0;
							}
						}
						else
						{
							a = b = c = k = g = h = 0;
						}

						dword q[4];

						if (b == f && e != c)
						{
							q[1] = q[2] = b;

							if ((a == b && f == l) || (b == g && f == j))
							{
								q[0] = Blend( b, e    );
								q[0] = Blend( b, q[0] );
								q[3] = Blend( b, c    );
								q[3] = Blend( b, q[3] );
							}
							else
							{
								q[0] = Blend( e, f );
								q[3] = Blend( b, c );
							}
						}
						else if (e == c && b != f)
						{
							q[3] = q[0] = e;

							if ((i == e && c == h) || (d == e && c == k))
							{
								q[1] = Blend( e, f    );
								q[1] = Blend( e, q[1] );
								q[2] = Blend( e, b    );
								q[2] = Blend( e, q[2] );
							}
							else
							{
								q[1] = Blend( e, f );
								q[2] = Blend( b, c );
							}
						}
						else if (e == c && b == f && e != f)
						{
							const int result = 
							(
							 	(f == a && f == g ? -1 : e == a && e == g ? +1 : 0) +
								(f == d && f == i ? -1 : e == d && e == i ? +1 : 0) +
								(f == h && f == k ? -1 : e == h && e == k ? +1 : 0) +
								(f == j && f == l ? -1 : e == j && e == l ? +1 : 0)
							);

							if (result > 0)
							{
								q[1] = q[2] = b;
								q[0] = q[3] = Blend( e, f) ;
							}
							else if (result < 0)
							{
								q[3] = q[0] = e;
								q[1] = q[2] = Blend( e, f );
							}
							else
							{
								q[3] = q[0] = e;
								q[1] = q[2] = b;
							}
						}
						else
						{
							if (b == e || c == f)
							{
								q[0] = e;
								q[2] = b;
								q[1] = f;
								q[3] = c;

							}
							else
							{
								q[1] = q[0] = Blend( e, f );
								q[0] = Blend( e, q[0] );
								q[1] = Blend( f, q[1] );

								q[2] = q[3] = Blend( b, c );
								q[2] = Blend( b, q[2] );
								q[3] = Blend( c, q[3] );
							}
						}

						dst[0][0] = q[0];
						dst[0][1] = q[1];
						dst[1][0] = q[2];
						dst[1][1] = q[3];
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[1]) + (pitch - long(sizeof(T) * WIDTH_2)));
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[0]) + pitch);
				}
			}

        #endif
        #ifndef NST_NO_HQ2X

			bool Renderer::Hq2x::IsSupported(uint bpp,dword r,dword g,dword b)
			{
				return
				(
				    (bpp == 16 && b == 0x001FU && ((g == 0x07E0U && r == 0xF800U) || (g == 0x03E0U && r == 0x7C00U))) ||
					(bpp == 32 && r == 0xFF0000UL && g == 0x00FF00UL && b == 0x0000FFUL)
				);
			}

			template<typename T>
			void Renderer::BlitHq2x(const Output& video) const
			{
				BlitHq2xRgb<T,0xFF0000UL,0x00FF00UL,0x0000FFUL>( video );
			}

			template<>
			void Renderer::BlitHq2x<u16>(const Output& video) const
			{
				if (format.mask[1] == 0x07E0U)
					BlitHq2xRgb<u16,0xF800U,0x07E0U,0x001FU>( video );
				else
					BlitHq2xRgb<u16,0x7C00U,0x03E0U,0x001FU>( video );
			}

			template<typename T>
			void Renderer::BlitHq3x(const Output& video) const
			{
				BlitHq3xRgb<T,0xFF0000UL,0x00FF00UL,0x0000FFUL>( video );
			}

			template<>
			void Renderer::BlitHq3x<u16>(const Output& video) const
			{
				if (format.mask[1] == 0x07E0U)
					BlitHq3xRgb<u16,0xF800U,0x07E0U,0x001FU>( video );
				else
					BlitHq3xRgb<u16,0x7C00U,0x03E0U,0x001FU>( video );
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::Hq2x::Interpolate1(dword c1,dword c2)
			{
				return ((((c1 & G)*3 + (c2 & G)) & (G << 2)) + (((c1 & (R|B))*3 + (c2 & (R|B))) & ((R|B) << 2))) >> 2;
			}

			template<>
			inline dword Renderer::Hq2x::Interpolate1<0xFF0000UL,0x00FF00UL,0x0000FFUL>(dword c1,dword c2)
			{
				return (c1 * 3 + c2) >> 2;
			}
			
			template<u32 R,u32 G,u32 B>
			dword Renderer::Hq2x::Interpolate2(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*2 + (c2 & G) + (c3 & G)) & (G << 2)) + (((c1 & (R|B))*2 + (c2 & (R|B)) + (c3 & (R|B))) & ((R|B) << 2))) >> 2;
			}

			template<>
			inline dword Renderer::Hq2x::Interpolate2<0xFF0000UL,0x00FF00UL,0x0000FFUL>(dword c1,dword c2,dword c3)
			{
				return (c1 * 2 + c2 + c3) >> 2;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::Hq2x::Interpolate3(dword c1,dword c2)
			{
				return ((((c1 & G)*7 + (c2 & G)) & (G << 3)) + (((c1 & (R|B))*7 + (c2 & (R|B))) & ((R|B) << 3))) >> 3;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::Hq2x::Interpolate4(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*2 + ((c2 & G) + (c3 & G))*7) & (G << 4)) + (((c1 & (R|B))*2 + ((c2 & (R|B)) + (c3 & (R|B)))*7) & ((R|B) << 4))) >> 4;
			}

			inline dword Renderer::Hq2x::Interpolate5(dword c1,dword c2)
			{
				return (c1 + c2) >> 1;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::Hq2x::Interpolate6(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*5 + (c2 & G)*2 + (c3 & G)) & (G << 3)) + (((c1 & (R|B))*5 + (c2 & (R|B))*2 + (c3 & (R|B))) & ((R|B) << 3))) >> 3;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::Hq2x::Interpolate7(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*6 + (c2 & G) + (c3 & G)) & (G << 3)) + (((c1 & (R|B))*6 + (c2 & (R|B)) + (c3 & (R|B))) & ((R|B) << 3))) >> 3;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::Hq2x::Interpolate9(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*2 + ((c2 & G) + (c3 & G))*3 ) & (G << 3)) + (((c1 & (R|B))*2 + ((c2 & (R|B)) + (c3 & (R|B)))*3 ) & ((R|B) << 3))) >> 3;
			}

			template<u32 R,u32 G,u32 B>
			dword Renderer::Hq2x::Interpolate10(dword c1,dword c2,dword c3)
			{
				return ((((c1 & G)*14 + (c2 & G) + (c3 & G)) & (G << 4)) + (((c1 & (R|B))*14 + (c2 & (R|B)) + (c3 & (R|B))) & ((R|B) << 4))) >> 4;
			}
			
			bool Renderer::Hq2x::DiffYuv(dword yuv1,dword yuv2)
			{
				return
				(
					(((yuv1 & 0x00FF0000UL) - (yuv2 & 0x00FF0000UL) + 0x300000UL) > 0x600000UL) ||
					(((yuv1 & 0x0000FF00UL) - (yuv2 & 0x0000FF00UL) + 0x000700UL) > 0x000E00UL) ||
					(((yuv1 & 0x000000FFUL) - (yuv2 & 0x000000FFUL) + 0x000006UL) > 0x00000CUL) 
				);
			}

			bool Renderer::Hq2x::Diff(uint w1,uint w2)
			{
				const dword colors[] =
				{
					Renderer::hq2x.yuv[w1],
					Renderer::hq2x.yuv[w2]
				};

				return
				(
     				(((colors[0] & 0x00FF0000UL) - (colors[1] & 0x00FF0000UL) + 0x300000UL) > 0x600000UL) ||
					(((colors[0] & 0x0000FF00UL) - (colors[1] & 0x0000FF00UL) + 0x000700UL) > 0x000E00UL) ||
					(((colors[0] & 0x000000FFUL) - (colors[1] & 0x000000FFUL) + 0x000006UL) > 0x00000CUL) 
				);
			}

			template<typename>
			struct Renderer::Hq2x::Buffer
			{
				uint w[10];
				dword c[10];

				NST_FORCE_INLINE void Convert()
				{
					for (uint k=0; k < 9; ++k)
						c[k] = Renderer::hq2x.rgb[w[k]];
				}
			};

			template<>
			struct Renderer::Hq2x::Buffer<u16>
			{
				union
				{
					uint w[10];
					dword c[10];
				};

				void Convert()
				{
				}
			};

			template<typename T,u32 R,u32 G,u32 B>
			void Renderer::BlitHq2xRgb(const Output& video) const
			{
				NST_ASSERT( state.scale == 2 );

				const u8* NST_RESTRICT src = reinterpret_cast<const u8*>(screen);
				const long pitch = video.pitch + video.pitch - (WIDTH_2 * sizeof(T));

				T* NST_RESTRICT dst[2] =
				{
					static_cast<T*>(video.pixels) - 2,
					reinterpret_cast<T*>(static_cast<u8*>(video.pixels) + video.pitch) - 2
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

				Hq2x::Buffer<T> b;

				for (uint y=HEIGHT; y; --y)
				{
					const uint lines[2] =
					{
						y < HEIGHT ? WIDTH * sizeof(u16) : 0,
						y > 1      ? WIDTH * sizeof(u16) : 0
					};

					b.w[2] = b.w[1] = palette[*reinterpret_cast<const u16*>(src - lines[0])];
					b.w[5] = b.w[4] = palette[*reinterpret_cast<const u16*>(src)];
					b.w[8] = b.w[7] = palette[*reinterpret_cast<const u16*>(src + lines[1])];

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
							b.w[2] = palette[*reinterpret_cast<const u16*>(src - lines[0])];
							b.w[5] = palette[*reinterpret_cast<const u16*>(src)];
							b.w[8] = palette[*reinterpret_cast<const u16*>(src + lines[1])];
						}

						b.Convert();
  
						const uint yuv5 = hq2x.yuv[b.w[4]];

						switch 
						(
       						((b.w[4] != b.w[0] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[0]] )) ? 0x01 : 0x0) |
							((b.w[4] != b.w[1] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[1]] )) ? 0x02 : 0x0) |
							((b.w[4] != b.w[2] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[2]] )) ? 0x04 : 0x0) |
							((b.w[4] != b.w[3] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[3]] )) ? 0x08 : 0x0) |
							((b.w[4] != b.w[5] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[5]] )) ? 0x10 : 0x0) |
							((b.w[4] != b.w[6] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[6]] )) ? 0x20 : 0x0) |
							((b.w[4] != b.w[7] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[7]] )) ? 0x40 : 0x0) |
							((b.w[4] != b.w[8] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[8]] )) ? 0x80 : 0x0)
						)
						#define PIXEL00_0     dst[0][0] = b.c[4];
						#define PIXEL00_10    dst[0][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[0] );
						#define PIXEL00_11    dst[0][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[3] );
						#define PIXEL00_12    dst[0][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[1] );
						#define PIXEL00_20    dst[0][0] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[3], b.c[1] );
						#define PIXEL00_21    dst[0][0] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[0], b.c[1] );
						#define PIXEL00_22    dst[0][0] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[0], b.c[3] );
						#define PIXEL00_60    dst[0][0] = Hq2x::Interpolate6<R,G,B>( b.c[4], b.c[1], b.c[3] );
                        #define PIXEL00_61    dst[0][0] = Hq2x::Interpolate6<R,G,B>( b.c[4], b.c[3], b.c[1] );
						#define PIXEL00_70    dst[0][0] = Hq2x::Interpolate7<R,G,B>( b.c[4], b.c[3], b.c[1] );
						#define PIXEL00_90    dst[0][0] = Hq2x::Interpolate9<R,G,B>( b.c[4], b.c[3], b.c[1] );
                        #define PIXEL00_100   dst[0][0] = Hq2x::Interpolate10<R,G,B>( b.c[4], b.c[3], b.c[1] );
						#define PIXEL01_0     dst[0][1] = b.c[4];
						#define PIXEL01_10    dst[0][1] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[2] );
						#define PIXEL01_11    dst[0][1] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[1] );
						#define PIXEL01_12    dst[0][1] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[5] );
						#define PIXEL01_20    dst[0][1] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[1], b.c[5] );
                        #define PIXEL01_21    dst[0][1] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[2], b.c[5] );
						#define PIXEL01_22    dst[0][1] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[2], b.c[1] );
						#define PIXEL01_60    dst[0][1] = Hq2x::Interpolate6<R,G,B>( b.c[4], b.c[5], b.c[1] );
                        #define PIXEL01_61    dst[0][1] = Hq2x::Interpolate6<R,G,B>( b.c[4], b.c[1], b.c[5] );
						#define PIXEL01_70    dst[0][1] = Hq2x::Interpolate7<R,G,B>( b.c[4], b.c[1], b.c[5] );
                        #define PIXEL01_90    dst[0][1] = Hq2x::Interpolate9<R,G,B>( b.c[4], b.c[1], b.c[5] );
						#define PIXEL01_100   dst[0][1] = Hq2x::Interpolate10<R,G,B>( b.c[4], b.c[1], b.c[5] );
						#define PIXEL10_0     dst[1][0] = b.c[4];
                        #define PIXEL10_10    dst[1][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[6] );
						#define PIXEL10_11    dst[1][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[7] );
						#define PIXEL10_12    dst[1][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[3] );
                        #define PIXEL10_20    dst[1][0] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL10_21    dst[1][0] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[6], b.c[3] );
						#define PIXEL10_22    dst[1][0] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[6], b.c[7] );
						#define PIXEL10_60    dst[1][0] = Hq2x::Interpolate6<R,G,B>( b.c[4], b.c[3], b.c[7] );
						#define PIXEL10_61    dst[1][0] = Hq2x::Interpolate6<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL10_70    dst[1][0] = Hq2x::Interpolate7<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL10_90    dst[1][0] = Hq2x::Interpolate9<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL10_100   dst[1][0] = Hq2x::Interpolate10<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL11_0     dst[1][1] = b.c[4];
						#define PIXEL11_10    dst[1][1] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[8] );
						#define PIXEL11_11    dst[1][1] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[5] );
                        #define PIXEL11_12    dst[1][1] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[7] );
						#define PIXEL11_20    dst[1][1] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[5], b.c[7] );
						#define PIXEL11_21    dst[1][1] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[8], b.c[7] );
						#define PIXEL11_22    dst[1][1] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[8], b.c[5] );
                        #define PIXEL11_60    dst[1][1] = Hq2x::Interpolate6<R,G,B>( b.c[4], b.c[7], b.c[5] );
						#define PIXEL11_61    dst[1][1] = Hq2x::Interpolate6<R,G,B>( b.c[4], b.c[5], b.c[7] );
						#define PIXEL11_70    dst[1][1] = Hq2x::Interpolate7<R,G,B>( b.c[4], b.c[5], b.c[7] );
						#define PIXEL11_90    dst[1][1] = Hq2x::Interpolate9<R,G,B>( b.c[4], b.c[5], b.c[7] );
                        #define PIXEL11_100   dst[1][1] = Hq2x::Interpolate10<R,G,B>( b.c[4], b.c[5], b.c[7] );
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
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
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
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_10
								else
									PIXEL11_20

								break;
								
							case 72:
							case 76:
								
								PIXEL00_21
								PIXEL01_20
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_20

								PIXEL11_22
								break;
								
							case 10:
							case 138:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
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

								if (Hq2x::Diff( b.w[1], b.w[5] ))
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20

								break;
								
							case 104:
							case 108:
								
								PIXEL00_21
								PIXEL01_20

								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_20

								PIXEL11_22
								break;
								
							case 11:
							case 139:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20

								PIXEL01_21
								PIXEL10_22
								PIXEL11_20
								break;
								
							case 19:
							case 51:
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
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
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))
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

								if (Hq2x::Diff( b.w[7], b.w[3] ))
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
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))
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
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
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
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
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
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20

								if (Hq2x::Diff( b.w[1], b.w[5] ))
									PIXEL01_0
								else
									PIXEL01_20

								PIXEL10_22
								PIXEL11_21
								break;

							case 82:
							case 214:
								
								PIXEL00_22

								if (Hq2x::Diff( b.w[1], b.w[5] ))		
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_21

								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20
									
								break;
								
							case 88:
							case 248:
								
								PIXEL00_21
								PIXEL01_22

								if (Hq2x::Diff( b.w[7], b.w[3] ))	
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_0							
								else								
									PIXEL11_20
								
								break;
								
							case 74:
							case 107:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_0									
								else									
									PIXEL00_20
									
								PIXEL01_21

								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_20
										
								PIXEL11_22
								break;
								
							case 27:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_0								
								else								
									PIXEL00_20
								
								PIXEL01_10
								PIXEL10_22
								PIXEL11_21
								break;
								
							case 86:
								
								PIXEL00_22

								if (Hq2x::Diff( b.w[1], b.w[5] ))
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 106:
								
								PIXEL00_10
								PIXEL01_21

								if (Hq2x::Diff( b.w[7], b.w[3] ))	
									PIXEL10_0								
								else
									PIXEL10_20
								
								PIXEL11_22
								break;
								
							case 30:
								
								PIXEL00_10
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
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
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20

								break;
								
							case 120:
								
								PIXEL00_21
								PIXEL01_22

								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_20

								PIXEL11_10
								break;
								
							case 75:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
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
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))	
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 83:
								
								PIXEL00_11

								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								PIXEL10_21
									
								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_10
								else
									PIXEL11_70

								break;
								
							case 92:
								
								PIXEL00_21
								PIXEL01_11

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else							
									PIXEL10_70
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 202:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								PIXEL01_21

								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_70
								
								PIXEL11_11
								break;
								
							case 78:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_10							
								else								
									PIXEL00_70
								
								PIXEL01_12
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_70

								PIXEL11_22
								break;
								
							case 154:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 114:
								
								PIXEL00_22

								if (Hq2x::Diff( b.w[1], b.w[5] ))
									PIXEL01_10
								else
									PIXEL01_70

								PIXEL10_12
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_10
								else
									PIXEL11_70
								
								break;
								
							case 89:
								
								PIXEL00_12
								PIXEL01_22

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 90:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 55:
							case 23:
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
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
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))
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

								if (Hq2x::Diff( b.w[7], b.w[3] ))
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
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))
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
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
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
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
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

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								PIXEL11_10
								break;
								
							case 203:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_20
								
								PIXEL01_21
								PIXEL10_10
								PIXEL11_11
								break;
								
							case 62:
								
								PIXEL00_10

								if (Hq2x::Diff( b.w[1], b.w[5] ))								
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 118:
								
								PIXEL00_22

								if (Hq2x::Diff( b.w[1], b.w[5] ))								
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 110:
								
								PIXEL00_10
								PIXEL01_12

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								PIXEL11_22
								break;
								
							case 155:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
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

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 158:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 234:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								PIXEL01_21

								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_20								
								
								PIXEL11_11
								break;
								
							case 242:
								
								PIXEL00_22

								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								PIXEL10_12

								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20
								
								break;
								
							case 59:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
									PIXEL01_10
								else
									PIXEL01_70
								
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 121:
								
								PIXEL00_12
								PIXEL01_22

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Hq2x::Diff( b.w[5], b.w[7]))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 87:
								
								PIXEL00_11

								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_21

								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 79:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
									
								PIXEL01_12

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								PIXEL11_22
								break;
								
							case 122:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else							
									PIXEL11_70
								
								break;
								
							case 94:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 218:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_10								
								else								
									PIXEL00_70
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 91:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
									PIXEL01_10
								else
									PIXEL01_70
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_70
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))
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
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_10
								else
									PIXEL00_70
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
									PIXEL01_10
								else
									PIXEL01_70
								
								PIXEL10_11
								PIXEL11_12
								break;
								
							case 115:
								
								PIXEL00_11

								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_10								
								else								
									PIXEL01_70
								
								PIXEL10_12

								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_10
								else
									PIXEL11_70

								break;
								
							case 93:
								
								PIXEL00_12
								PIXEL01_11

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_10								
								else								
									PIXEL10_70
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_10								
								else								
									PIXEL11_70
								
								break;
								
							case 206:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_10
								else
									PIXEL00_70
									
								PIXEL01_12
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_70

								PIXEL11_11
								break;
								
							case 205:
							case 201:
								
								PIXEL00_12
								PIXEL01_20

								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_10
								else
									PIXEL10_70
								
								PIXEL11_11
								break;
								
							case 174:
							case 46:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
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

								if (Hq2x::Diff( b.w[1], b.w[5] ))
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))
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

								if (Hq2x::Diff( b.w[1], b.w[5] ))
									PIXEL01_0
								else
									PIXEL01_20
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_20
								
								PIXEL11_10
								break;
								
							case 219:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
								
								PIXEL01_10
								PIXEL10_10
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20
								
								break;
								
							case 125:
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))
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
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))
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
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
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

								if (Hq2x::Diff( b.w[7], b.w[3] ))
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

								if (Hq2x::Diff( b.w[1], b.w[5] ))
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
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))
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
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
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
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_100
								
								PIXEL11_11
								break;
								
							case 175:
							case 47:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
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

								if (Hq2x::Diff( b.w[1], b.w[5] ))
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

								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_100
								
								break;
								
							case 250:
								
								PIXEL00_10
								PIXEL01_10

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else
									PIXEL11_20
								
								break;
								
							case 123:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_20
								
								PIXEL01_10
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								PIXEL11_10
								break;
								
							case 95:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_20
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_10
								PIXEL11_10
								break;
								
							case 222:
								
								PIXEL00_10
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_10
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0
								else								
									PIXEL11_20
								
								break;
								
							case 252:
								
								PIXEL00_21
								PIXEL01_11

								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_100
								
								break;
								
							case 249:
								
								PIXEL00_12
								PIXEL01_22

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_100
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 235:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_20
																
								PIXEL01_21

								if (Hq2x::Diff( b.w[7], b.w[3] ))
									PIXEL10_0
								else
									PIXEL10_100
								
								PIXEL11_11
								break;
								
							case 111:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_100
								
								PIXEL01_12
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								PIXEL11_22
								break;
								
							case 63:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_100
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								PIXEL10_11
								PIXEL11_21
								break;
								
							case 159:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_0								
								else								
									PIXEL00_20
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_100
								
								PIXEL10_22
								PIXEL11_12
								break;
								
							case 215:
								
								PIXEL00_11

								if (Hq2x::Diff( b.w[1], b.w[5] ))										
									PIXEL01_0
								else
									PIXEL01_100
										
								PIXEL10_21

								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_20
								
								break;
								
							case 246:
								
								PIXEL00_22

								if (Hq2x::Diff( b.w[1], b.w[5] ))										
									PIXEL01_0										
								else										
									PIXEL01_20
								
								PIXEL10_12
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_100
								
								break;
								
							case 254:
								
								PIXEL00_10

								if (Hq2x::Diff( b.w[1], b.w[5] ))								
									PIXEL01_0								
								else								
									PIXEL01_20
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_20
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_100
								
								break;
								
							case 253:
								
								PIXEL00_12
								PIXEL01_11

								if (Hq2x::Diff( b.w[7], b.w[3] ))								
									PIXEL10_0								
								else								
									PIXEL10_100
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))								
									PIXEL11_0								
								else								
									PIXEL11_100
								
								break;
								
							case 251:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_0									
								else									
									PIXEL00_20
								
								PIXEL01_10
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))									
									PIXEL10_0									
								else									
									PIXEL10_100
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))									
									PIXEL11_0									
								else									
									PIXEL11_20
								
								break;
								
							case 239:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))
									PIXEL00_0
								else
									PIXEL00_100
								
								PIXEL01_12
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))									
									PIXEL10_0
								else
									PIXEL10_100

								PIXEL11_11
								break;
								
							case 127:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))									
									PIXEL00_0									
								else									
									PIXEL00_100
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))									
									PIXEL01_0									
								else									
									PIXEL01_20
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))									
									PIXEL10_0									
								else									
									PIXEL10_20
								
								PIXEL11_10
								break;
								
							case 191:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))									
									PIXEL00_0									
								else									
									PIXEL00_100
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))									
									PIXEL01_0									
								else									
									PIXEL01_100
								
								PIXEL10_11
								PIXEL11_12
								break;
								
							case 223:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))									
									PIXEL00_0									
								else									
									PIXEL00_20
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))									
									PIXEL01_0									
								else									
									PIXEL01_100
								
								PIXEL10_10
									
								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_20

								break;
								
							case 247:
								
								PIXEL00_11
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))
									PIXEL01_0
								else
									PIXEL01_100

								PIXEL10_12
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))
									PIXEL11_0
								else
									PIXEL11_100

								break;
								
							case 255:
								
								if (Hq2x::Diff( b.w[3], b.w[1] ))								
									PIXEL00_0									
								else									
									PIXEL00_100
								
								if (Hq2x::Diff( b.w[1], b.w[5] ))									
									PIXEL01_0									
								else									
									PIXEL01_100
								
								if (Hq2x::Diff( b.w[7], b.w[3] ))									
									PIXEL10_0								
								else									
									PIXEL10_100
								
								if (Hq2x::Diff( b.w[5], b.w[7] ))									
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
			void Renderer::BlitHq3xRgb(const Output& video) const
			{
				NST_ASSERT( state.scale == 3 );

				const u8* NST_RESTRICT src = reinterpret_cast<const u8*>(screen);
				const long pitch = (video.pitch * 2) + video.pitch - (WIDTH_3 * sizeof(T));

				T* NST_RESTRICT dst[3] =
				{
					static_cast<T*>(video.pixels) - 3,
					reinterpret_cast<T*>(static_cast<u8*>(video.pixels) + video.pitch) - 3,
					reinterpret_cast<T*>(static_cast<u8*>(video.pixels) + video.pitch * 2) - 3
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

				Hq2x::Buffer<T> b;

				for (uint y=HEIGHT; y; --y)
				{
					const uint lines[2] =
					{
						y < HEIGHT ? WIDTH * sizeof(u16) : 0,
						y > 1      ? WIDTH * sizeof(u16) : 0
					};

					b.w[2] = b.w[1] = palette[*reinterpret_cast<const u16*>(src - lines[0])];
					b.w[5] = b.w[4] = palette[*reinterpret_cast<const u16*>(src)];
					b.w[8] = b.w[7] = palette[*reinterpret_cast<const u16*>(src + lines[1])];

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
							b.w[2] = palette[*reinterpret_cast<const u16*>(src - lines[0])];
							b.w[5] = palette[*reinterpret_cast<const u16*>(src)];
							b.w[8] = palette[*reinterpret_cast<const u16*>(src + lines[1])];
						}

						b.Convert();
  
						const uint yuv5 = hq2x.yuv[b.w[4]];

						switch 
						(
       						((b.w[4] != b.w[0] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[0]] )) ? 0x01 : 0x0) |
							((b.w[4] != b.w[1] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[1]] )) ? 0x02 : 0x0) |
							((b.w[4] != b.w[2] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[2]] )) ? 0x04 : 0x0) |
							((b.w[4] != b.w[3] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[3]] )) ? 0x08 : 0x0) |
							((b.w[4] != b.w[5] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[5]] )) ? 0x10 : 0x0) |
							((b.w[4] != b.w[6] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[6]] )) ? 0x20 : 0x0) |
							((b.w[4] != b.w[7] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[7]] )) ? 0x40 : 0x0) |
							((b.w[4] != b.w[8] && Hq2x::DiffYuv( yuv5, hq2x.yuv[b.w[8]] )) ? 0x80 : 0x0)
						)
						#define PIXEL00_1M  dst[0][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[0] );
                        #define PIXEL00_1U  dst[0][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[1] );
						#define PIXEL00_1L  dst[0][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[3] );
						#define PIXEL00_2   dst[0][0] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[3], b.c[1] );
                        #define PIXEL00_4   dst[0][0] = Hq2x::Interpolate4<R,G,B>( b.c[4], b.c[3], b.c[1] );
						#define PIXEL00_5   dst[0][0] = Hq2x::Interpolate5( b.c[3], b.c[1] );
						#define PIXEL00_C   dst[0][0] = b.c[4];			
						#define PIXEL01_1   dst[0][1] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[1] );
						#define PIXEL01_3   dst[0][1] = Hq2x::Interpolate3<R,G,B>( b.c[4], b.c[1] );
						#define PIXEL01_6   dst[0][1] = Hq2x::Interpolate1<R,G,B>( b.c[1], b.c[4] );
						#define PIXEL01_C   dst[0][1] = b.c[4];			
						#define PIXEL02_1M  dst[0][2] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[2] );
						#define PIXEL02_1U  dst[0][2] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[1] );
                        #define PIXEL02_1R  dst[0][2] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[5] );
						#define PIXEL02_2   dst[0][2] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[1], b.c[5] );
						#define PIXEL02_4   dst[0][2] = Hq2x::Interpolate4<R,G,B>( b.c[4], b.c[1], b.c[5] );
						#define PIXEL02_5   dst[0][2] = Hq2x::Interpolate5( b.c[1], b.c[5] );
                        #define PIXEL02_C   dst[0][2] = b.c[4];			
						#define PIXEL10_1   dst[1][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[3] );
						#define PIXEL10_3   dst[1][0] = Hq2x::Interpolate3<R,G,B>( b.c[4], b.c[3] );
                        #define PIXEL10_6   dst[1][0] = Hq2x::Interpolate1<R,G,B>( b.c[3], b.c[4] );
						#define PIXEL10_C   dst[1][0] = b.c[4];            
						#define PIXEL11     dst[1][1] = b.c[4];			
                        #define PIXEL12_1   dst[1][2] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[5] );
						#define PIXEL12_3   dst[1][2] = Hq2x::Interpolate3<R,G,B>( b.c[4], b.c[5] );
						#define PIXEL12_6   dst[1][2] = Hq2x::Interpolate1<R,G,B>( b.c[5], b.c[4] );
						#define PIXEL12_C   dst[1][2] = b.c[4];			
						#define PIXEL20_1M  dst[2][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[6] );
						#define PIXEL20_1D  dst[2][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[7] );
						#define PIXEL20_1L  dst[2][0] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[3] );
						#define PIXEL20_2   dst[2][0] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL20_4   dst[2][0] = Hq2x::Interpolate4<R,G,B>( b.c[4], b.c[7], b.c[3] );
						#define PIXEL20_5   dst[2][0] = Hq2x::Interpolate5( b.c[7], b.c[3]);
                        #define PIXEL20_C   dst[2][0] = b.c[4];			
						#define PIXEL21_1   dst[2][1] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[7] );
						#define PIXEL21_3   dst[2][1] = Hq2x::Interpolate3<R,G,B>( b.c[4], b.c[7] );
                        #define PIXEL21_6   dst[2][1] = Hq2x::Interpolate1<R,G,B>( b.c[7], b.c[4] );
						#define PIXEL21_C   dst[2][1] = b.c[4];			
						#define PIXEL22_1M  dst[2][2] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[8] );
                        #define PIXEL22_1D  dst[2][2] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[7] );
						#define PIXEL22_1R  dst[2][2] = Hq2x::Interpolate1<R,G,B>( b.c[4], b.c[5] );
						#define PIXEL22_2   dst[2][2] = Hq2x::Interpolate2<R,G,B>( b.c[4], b.c[5], b.c[7] );
						#define PIXEL22_4   dst[2][2] = Hq2x::Interpolate4<R,G,B>( b.c[4], b.c[5], b.c[7] );
                        #define PIXEL22_5   dst[2][2] = Hq2x::Interpolate5( b.c[5], b.c[7] );
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
						
								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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
								
								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1M
								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2

								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 202:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1M
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 78:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 154:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1L
								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 90:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C

								if (Hq2x::Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2

								PIXEL10_C
								PIXEL11
								PIXEL12_C

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 55:
							case 23:
								
								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2

								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2

								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1M
								PIXEL11
								PIXEL12_1

								if (Hq2x::Diff(b.w[7], b.w[3]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL10_1
								PIXEL11
								PIXEL20_1L

								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))								
									PIXEL22_1M								
								else								
									PIXEL22_2
								
								break;
								
							case 87:
								
								PIXEL00_1L

								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[5], b.w[7]))								
									PIXEL22_1M								
								else								
									PIXEL22_2
								
								break;
								
							case 79:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2								
								
								PIXEL21_C
								PIXEL22_1M
								break;
								
							case 122:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C

								if (Hq2x::Diff(b.w[1], b.w[5]))								
									PIXEL02_1M								
								else
									PIXEL02_2
																
								PIXEL11
								PIXEL12_C

								if (Hq2x::Diff(b.w[7], b.w[3]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))								
									PIXEL22_1M								
								else								
									PIXEL22_2
								
								break;
								
							case 94:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))								
									PIXEL20_1M								
								else
									PIXEL20_2
								
								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 218:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2								
								
								PIXEL01_C

								if (Hq2x::Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL10_C
								PIXEL11

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2

								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
									PIXEL02_1M
								else
									PIXEL02_2
								
								PIXEL11
								PIXEL12_C

								if (Hq2x::Diff(b.w[7], b.w[3]))								
									PIXEL20_1M								
								else								
									PIXEL20_2
								
								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))								
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2								

								PIXEL01_C

								if (Hq2x::Diff(b.w[1], b.w[5]))								
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

								if (Hq2x::Diff(b.w[1], b.w[5]))								
									PIXEL02_1M								
								else								
									PIXEL02_2
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1L
								PIXEL21_C
								
								if (Hq2x::Diff(b.w[5], b.w[7]))								
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))
									PIXEL22_1M
								else
									PIXEL22_2
								
								break;
								
							case 206:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_1M
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Hq2x::Diff(b.w[7], b.w[3]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_1M
								else
									PIXEL20_2
								
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 174:
							case 46:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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
								
								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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
								
								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2
								
								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 175:
							case 47:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
									PIXEL22_C
								else
									PIXEL22_2

								break;
								
							case 250:
								
								PIXEL00_1M
								PIXEL01_C
								PIXEL02_1M
								PIXEL11

								if (Hq2x::Diff(b.w[7], b.w[3]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2

								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2

								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 111:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
									PIXEL00_2

								PIXEL01_C
								PIXEL02_1R
								PIXEL10_C
								PIXEL11
								PIXEL12_1

								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
									PIXEL00_2

								PIXEL01_C

								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
									PIXEL02_C
								else
									PIXEL02_2

								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1M

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
									PIXEL22_C
								else
									PIXEL22_2

								break;
								
							case 254:
								
								PIXEL00_1M

								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
								{
									PIXEL10_C
									PIXEL20_C
								}
								else
								{
									PIXEL10_3
									PIXEL20_4
								}

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2

								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))
									PIXEL22_C
								else
									PIXEL22_2
								
								break;
								
							case 251:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[7], b.w[3]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
									PIXEL00_2
								
								PIXEL01_C
								PIXEL02_1R
								PIXEL10_C
								PIXEL11
								PIXEL12_1
								
								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2

								PIXEL21_C
								PIXEL22_1R
								break;
								
							case 127:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
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

								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[7], b.w[3]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
									PIXEL00_2
								
								PIXEL01_C
								
								if (Hq2x::Diff(b.w[1], b.w[5]))
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
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
								{
									PIXEL00_C
									PIXEL10_C
								}
								else
								{
									PIXEL00_4
									PIXEL10_3
								}
								
								if (Hq2x::Diff(b.w[1], b.w[5]))
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

								if (Hq2x::Diff(b.w[5], b.w[7]))
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
								
								if (Hq2x::Diff(b.w[1], b.w[5]))								
									PIXEL02_C								
								else								
									PIXEL02_2								
								
								PIXEL10_1
								PIXEL11
								PIXEL12_C
								PIXEL20_1L
								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))								
									PIXEL22_C								
								else								
									PIXEL22_2
								
								break;
								
							case 255:
								
								if (Hq2x::Diff(b.w[3], b.w[1]))
									PIXEL00_C
								else
							     	PIXEL00_2
						
								PIXEL01_C
						
								if (Hq2x::Diff(b.w[1], b.w[5]))           
									PIXEL02_C            
								else            
						     		PIXEL02_2
						
								PIXEL10_C
								PIXEL11
								PIXEL12_C
							
								if (Hq2x::Diff(b.w[7], b.w[3]))
									PIXEL20_C
								else
									PIXEL20_2

								PIXEL21_C

								if (Hq2x::Diff(b.w[5], b.w[7]))
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

        #endif

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		}
	}
}
