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

				enum PaletteType
				{
					PALETTE_INTERNAL,
					PALETTE_CUSTOM,
					PALETTE_EMULATE
				};

				enum
				{
					WIDTH = 256,
					HEIGHT = 240,
					PIXELS = ulong(WIDTH) * HEIGHT,
					PALETTE = 64 * 8,
					DEFAULT_HUE = 128,
					DEFAULT_BRIGHTNESS = 128,
					DEFAULT_SATURATION = 128,
					DEFAULT_PALETTE = PALETTE_INTERNAL
				};

				Result SetState(const RenderState&);
				Result GetState(RenderState&) const;
				void Blit(Output&);

				Result SetPaletteType(PaletteType);
				Result LoadCustomPalette(const u8 (*)[3]);
				void   ResetCustomPalette();

				typedef u8 PaletteEntries[PALETTE][3];
				typedef u16 Screen[PIXELS];

				const PaletteEntries& GetPalette();

			private:

				void UpdateFilter();
				Result SetLevel(u8&,u8);

				class Palette
				{
				public:

					Palette();
					~Palette();

					Result SetType(PaletteType);
					Result LoadCustom(const u8 (*)[3]);
					bool   ResetCustom();
					void   Update(uint,uint,uint);

					inline const PaletteEntries& Get() const;

				private:

					enum
					{
						HUE_OFFSET = 255,
						HUE_ROTATION = 360 / 12
					};

					struct Custom
					{
						u8 palette[64][3];
					};

					void ComputeTV(uint,uint,uint);
					void ComputeCustom(uint,uint,uint);

					static void ToPAL(const double (&)[3],u8 (&)[3]);
					static void ToHSV(double,double,double,double&,double&,double&);
					static void ToRGB(double,double,double,double&,double&,double&);

					PaletteType type;						
					Custom* custom;
					u8 palette[64*8][3];

					static const double emphasis[8][3];
					static const u8 defaultPalette[64][3];

				public:

					PaletteType GetType() const
					{
						return type;
					}
				};

				struct Input
				{
					u16 screen[PIXELS];
					u32 palette[PALETTE];
				};

				class FilterNone;
				class FilterScanlines;

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
					virtual void Transform(const u8 (&)[PALETTE][3],u32 (&)[PALETTE]) const;
					virtual bool CanTransform() const { return true; }

					const uint bpp;
					const Format format;
				};

				struct State
				{
					State();

					enum
					{
						UPDATE_PALETTE = 0x1,
						UPDATE_FILTER = 0x2
					};

					RenderState::Filter filter;
					u16 width;
					u16 height;
					u8 update;
					u8 brightness;
					u8 saturation;
					u8 hue;
					RenderState::Bits::Mask mask;
				};

				Filter* filter;
				State state;
				Input input;
				Palette palette;

			public:

				Result SetBrightness(u8 brightness)
				{
					return SetLevel( state.brightness, brightness );
				}

				Result SetSaturation(u8 saturation)
				{
					return SetLevel( state.saturation, saturation );
				}

				Result SetHue(u8 hue)
				{
					return SetLevel( state.hue, hue );
				}

				uint GetBrightness() const
				{
					return state.brightness;
				}

				uint GetSaturation() const
				{
					return state.saturation;
				}

				uint GetHue() const
				{
					return state.hue;
				}

				PaletteType GetPaletteType() const
				{
					return palette.GetType();
				}
  
				Screen& GetScreen()
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
