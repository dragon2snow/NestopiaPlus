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

#ifndef NST_VIDEO_RENDERER_H
#define NST_VIDEO_RENDERER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Renderer
			{
			public:
				
				Result SetState(const Api::Video::RenderState&);
				Result GetState(Api::Video::RenderState&) const;
				void Blit(Output&) const;
		
			private:
		
				enum
				{
					WIDTH = 256,
					HEIGHT = 240,
					WIDTH_1 = WIDTH,
					WIDTH_2 = WIDTH * 2,
					WIDTH_3 = WIDTH * 3,
					HEIGHT_1 = HEIGHT,
					HEIGHT_2 = HEIGHT * 2,
					HEIGHT_3 = HEIGHT * 3,
					PIXELS = ulong(WIDTH) * HEIGHT,
					PALETTE_ENTRIES = 64 * 8
				};
		
				inline dword Blend(dword,dword) const;
				inline dword Blend(dword,dword,dword,dword) const;

				template<typename T> void BlitFilter(const Output&) const;
		
				template<typename T> void BlitAligned     (T* NST_RESTRICT) const;
				template<typename T> void BlitUnaligned   (const Output&) const;
				template<typename T> void BlitScanlines1x (const Output&) const;
				template<typename T> void BlitScanlines2x (const Output&) const;
				template<typename T> void BlitTV          (const Output&) const;

            #ifndef NST_NO_SCALE2X

				template<typename T> void BlitScale2x (const Output&) const;
				template<typename T> void BlitScale3x (const Output&) const;

				template<typename T,int PREV,int NEXT> NST_FORCE_INLINE T* Scale2xBorder (T* NST_RESTRICT,const u16* NST_RESTRICT) const;
				template<typename T,int PREV,int NEXT> NST_FORCE_INLINE T* Scale3xBorder (T* NST_RESTRICT,const u16* NST_RESTRICT) const;
				template<typename T>                   NST_FORCE_INLINE T* Scale3xCenter (T* NST_RESTRICT,const u16* NST_RESTRICT) const;
				template<typename T,int PREV,int NEXT> NST_FORCE_INLINE T* Scale2xLine   (T* NST_RESTRICT,const u16* NST_RESTRICT,long) const;
				template<typename T,int PREV,int NEXT> NST_FORCE_INLINE T* Scale3xLine   (T* NST_RESTRICT,const u16* NST_RESTRICT,long) const;

            #endif
            #ifndef NST_NO_2XSAI

				template<typename T> void Blit2xSaI       (const Output&) const;
				template<typename T> void BlitSuper2xSaI  (const Output&) const;
				template<typename T> void BlitSuperEagle  (const Output&) const;

            #endif
            #ifndef NST_NO_HQ2X

				struct Hq2x
				{
					Hq2x();
					~Hq2x();

					void Initialize(uint,const uint (&)[3]);
					void Uninitialize();

					template<typename>
					struct Buffer;

					template<u32 R,u32 G,u32 B> static dword Interpolate1(dword,dword);
					template<u32 R,u32 G,u32 B> static dword Interpolate2(dword,dword,dword);
					template<u32 R,u32 G,u32 B> static dword Interpolate3(dword,dword);
					template<u32 R,u32 G,u32 B> static dword Interpolate4(dword,dword,dword);
					                            static inline dword Interpolate5(dword,dword);
					template<u32 R,u32 G,u32 B> static dword Interpolate6(dword,dword,dword);
					template<u32 R,u32 G,u32 B> static dword Interpolate7(dword,dword,dword);
					template<u32 R,u32 G,u32 B> static dword Interpolate9(dword,dword,dword);
					template<u32 R,u32 G,u32 B> static dword Interpolate10(dword,dword,dword);

					static bool IsSupported(uint,dword,dword,dword);

					static bool Diff(uint,uint);
					static bool DiffYuv(dword,dword);

					u32* yuv;
					u32* rgb;
				};

				template<typename T> void BlitHq2x(const Output&) const;
				template<typename T> void BlitHq3x(const Output&) const;

				template<typename T,u32 R,u32 G,u32 B> void BlitHq2xRgb(const Output&) const;
				template<typename T,u32 R,u32 G,u32 B> void BlitHq3xRgb(const Output&) const;

				static Hq2x hq2x;

            #endif

				void UpdatePalette();
				void BlitPaletteIndexed(const Output&) const;

				struct State
				{
					u8 bpp;
					u8 filter;
					u8 scale;
					u8 paletteOffset;
				};

				struct Format
				{
					dword mask[3];
					dword lsb[2];
					dword left[3];
					dword right[3];
				};
				 
				u16 screen[WIDTH * HEIGHT];
				u32 palette[PALETTE_ENTRIES];
				Format format;
				State state;
				const u8 (*srcPalette)[3];
		
			public:
		
				Renderer()
				: srcPalette(NULL)
				{
					state.bpp = 0;
				}

				void SetPalette(const u8 (*palette)[3])
				{
					NST_ASSERT( palette );		
					srcPalette = palette;
					UpdatePalette();
				}
		
				typedef u16 (&Screen)[WIDTH * HEIGHT];
		
				Screen GetScreen()
				{
					return screen;
				}

				bool IsReady() const
				{
					return state.bpp != 0;
				}
			};
		}
	}
}

#endif
