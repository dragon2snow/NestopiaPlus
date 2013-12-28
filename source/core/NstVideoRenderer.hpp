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
				typedef Api::Video::RenderState RenderState;

			public:
		
				Renderer();
				~Renderer();

				enum
				{
					WIDTH = 256,
					HEIGHT = 240,
					PIXELS = ulong(WIDTH) * HEIGHT,
					PALETTE = 64 * 8
				};

				typedef u16 (&Screen)[PIXELS];

				Result SetState(const RenderState&);
				Result GetState(RenderState&) const;
				void Blit(Output&) const;
		
			private:
					
				void UpdateColors();

				struct Input
				{
					u16 screen[PIXELS];
					u32 palette[PALETTE];
				};

				class FilterNone;
				class FilterScanlines;
				class FilterTV;

                #ifndef NST_NO_2XSAI
				class Filter2xSaI;
                #endif

                #ifndef NST_NO_SCALE2X
				class FilterScaleX;
                #endif

                #ifndef NST_NO_HQ2X
				class FilterHqX;
                #endif

                #ifndef NST_NO_NTSCVIDEO
				template<uint BITS> class FilterNtsc;
                #endif

				class NST_NO_VTABLE Filter
				{
					struct Format
					{
						Format(const RenderState::Bits::Mask&);

						dword left[3];
						dword right[3];
					};

				public:

					Filter(const RenderState&);
					virtual ~Filter() {}

					virtual void Blit(const Input&,const Output&) = 0;
					virtual void Transform(const u8 (*NST_RESTRICT)[3],u32 (&)[PALETTE]) const;

					const uint bpp;
					const Format format;
				};

				struct State
				{
					State();

					RenderState::Filter filter;
					ushort width;
					ushort height;
					ushort brightness;
					uchar saturation;
					uchar hue;
					RenderState::Bits::Mask mask;
				};

				Filter* filter;
				const u8 (*palette)[3];
				State state;
				Input input;

			public:
		
				void SetPalette(const u8 (*p)[3])
				{
					NST_ASSERT( p );		
					palette = p;

					if (filter)
						filter->Transform( palette, input.palette );
				}
				
				void SetBrightness(uchar brightness)
				{
					if (state.brightness != brightness)
					{
						state.brightness = brightness;
						UpdateColors();
					}
				}

				void SetSaturation(uchar saturation)
				{
					if (state.saturation != saturation)
					{
						state.saturation = saturation;
						UpdateColors();
					}
				}

				void SetHue(uchar hue)
				{
					if (state.hue != hue)
					{
						state.hue = hue;
						UpdateColors();
					}
				}

				Screen GetScreen()
				{
					return input.screen;
				}

				bool IsReady() const
				{
					return filter != NULL;
				}
			};
		}
	}
}

#endif
