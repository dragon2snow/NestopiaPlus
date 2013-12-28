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

#ifndef NST_VIDEO_RENDERER_H
#define NST_VIDEO_RENDERER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "api/NstApiVideo.hpp"
#include "NstVideoScreen.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Renderer
			{
				typedef Api::Video::RenderState RenderState;
				typedef Api::Video::Decoder Decoder;
				typedef Screen Input;

			public:

				Renderer();
				~Renderer();

				enum PaletteType
				{
					PALETTE_YUV,
					PALETTE_PC10,
					PALETTE_VS1,
					PALETTE_VS2,
					PALETTE_VS3,
					PALETTE_VS4,
					PALETTE_CUSTOM
				};

				enum
				{
					WIDTH = Input::WIDTH,
					HEIGHT = Input::HEIGHT,
					PIXELS = Input::PIXELS,
					PALETTE = Input::PALETTE,
					DEFAULT_PALETTE = PALETTE_YUV
				};

				Result SetState(const RenderState&);
				Result GetState(RenderState&) const;
				Result SetHue(int);
				void Blit(Output&,Input&,uint=1);

				void SetMode(Mode);
				Result SetDecoder(const Decoder&);

				Result SetPaletteType(PaletteType);
				Result LoadCustomPalette(const u8 (*)[3],bool);
				void   ResetCustomPalette();

				void EnableFieldMerging(bool);

				typedef u8 PaletteEntries[PALETTE][3];

				const PaletteEntries& GetPalette();

			private:

				void UpdateFilter(Input&);

				class Palette
				{
				public:

					Palette();
					~Palette();

					Result SetType(PaletteType);
					Result LoadCustom(const u8 (*)[3],bool);
					uint   SaveCustom(u8 (*)[3],bool) const;
					bool   ResetCustom();
					void   Update(int,int,int,int);
					Result SetDecoder(const Decoder&);

					inline const PaletteEntries& Get() const;

				private:

					struct Custom
					{
						inline Custom();
						inline ~Custom();

						bool EnableEmphasis(bool);

						u8 palette[64][3];
						u8 (*emphasis)[64][3];
					};

					void Generate(int,int,int,int);
					void Build(int,int,int,int);

					static void ToPAL(const double (&)[3],u8 (&)[3]);
					static void ToHSV(double,double,double,double&,double&,double&);
					static void ToRGB(double,double,double,double&,double&,double&);

					PaletteType type;
					Custom* custom;
					Decoder decoder;
					u8 palette[64*8][3];

					static const u8 pc10Palette[64][3];
					static const u8 vsPalette[4][64][3];

				public:

					PaletteType GetType() const
					{
						return type;
					}

					const Decoder& GetDecoder() const
					{
						return decoder;
					}

					bool HasCustomEmphasis() const
					{
						return custom && custom->emphasis;
					}
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

				protected:

					template<uint BITS>
					struct OutPixel
					{
						typedef u16 Type;
					};

				public:

					Filter(const RenderState&);
					virtual ~Filter() {}

					virtual void Blit(const Input&,const Output&,uint) = 0;
					virtual void Transform(const u8 (&)[PALETTE][3],u32 (&)[PALETTE]) const;

					const uint bpp;
					const Format format;
				};

				struct State
				{
					State();

					enum
					{
						UPDATE_PALETTE = 0x1,
						UPDATE_FILTER = 0x2,
						UPDATE_NTSC = 0x4,
						FIELD_MERGING_USER = 0x1,
						FIELD_MERGING_PAL = 0x2
					};

					RenderState::Filter filter;
					u16 width;
					u16 height;
					u8 update;
					i8 brightness;
					i8 saturation;
					i8 hue;
					i8 contrast;
					i8 sharpness;
					i8 resolution;
					i8 bleed;
					i8 artifacts;
					i8 fringing;
					u8 scanlines;
					u8 fieldMerging;
					RenderState::Bits::Mask mask;
				};

				Result SetLevel(i8&,int,uint=State::UPDATE_PALETTE|State::UPDATE_FILTER);

				Filter* filter;
				State state;
				Palette palette;
				Decoder decoder;

			public:

				Result SetBrightness(int brightness)
				{
					return SetLevel( state.brightness, brightness );
				}

				Result SetSaturation(int saturation)
				{
					return SetLevel( state.saturation, saturation );
				}

				Result SetContrast(int contrast)
				{
					return SetLevel( state.contrast, contrast );
				}

				Result SetSharpness(int sharpness)
				{
					return SetLevel( state.sharpness, sharpness, State::UPDATE_NTSC );
				}

				Result SetColorResolution(int resolution)
				{
					return SetLevel( state.resolution, resolution, State::UPDATE_NTSC );
				}

				Result SetColorBleed(int bleed)
				{
					return SetLevel( state.bleed, bleed, State::UPDATE_NTSC );
				}

				Result SetColorArtifacts(int artifacts)
				{
					return SetLevel( state.artifacts, artifacts, State::UPDATE_NTSC );
				}

				Result SetColorFringing(int fringing)
				{
					return SetLevel( state.fringing, fringing, State::UPDATE_NTSC );
				}

				int GetBrightness() const
				{
					return state.brightness;
				}

				int GetSaturation() const
				{
					return state.saturation;
				}

				int GetContrast() const
				{
					return state.contrast;
				}

				int GetSharpness() const
				{
					return state.sharpness;
				}

				int GetColorResolution() const
				{
					return state.resolution;
				}

				int GetColorBleed() const
				{
					return state.bleed;
				}

				int GetColorArtifacts() const
				{
					return state.artifacts;
				}

				int GetColorFringing() const
				{
					return state.fringing;
				}

				int GetHue() const
				{
					return state.hue;
				}

				bool IsFieldMergingEnabled() const
				{
					return state.fieldMerging & State::FIELD_MERGING_USER;
				}

				PaletteType GetPaletteType() const
				{
					return palette.GetType();
				}

				bool HasCustomPaletteEmphasis() const
				{
					return palette.HasCustomEmphasis();
				}

				uint SaveCustomPalette(u8 (*colors)[3],bool emphasis) const
				{
					return palette.SaveCustom( colors, emphasis );
				}

				const Decoder& GetDecoder() const
				{
					return palette.GetDecoder();
				}

				bool IsReady() const
				{
					return filter != NULL;
				}
			};

			template<>
			struct Renderer::Filter::OutPixel<32U>
			{
				typedef u32 Type;
			};
		}
	}
}

#endif
