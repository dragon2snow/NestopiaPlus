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

#ifndef NST_API_VIDEO_H
#define NST_API_VIDEO_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstApi.hpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Renderer;

			class Output
			{
				struct Locker;
				struct Unlocker;

			public:

				enum
				{
					WIDTH = 256,
					HEIGHT = 240,
					NTSC_WIDTH = 602,
					NTSC_HEIGHT = 480
				};

				void* pixels;
				long pitch;

				Output(void* v=NULL,long p=0)
				: pixels(v), pitch(p) {}

				typedef bool (NST_CALLBACK *LockCallback) (void*,Output&);
				typedef void (NST_CALLBACK *UnlockCallback) (void*,Output&);

				static Locker lockCallback;
				static Unlocker unlockCallback;
			};

			struct Output::Locker : UserCallback<Output::LockCallback>
			{
				bool operator () (Output& output) const
				{
					return (!function || function( userdata, output )) && output.pixels && output.pitch;
				}
			};

			struct Output::Unlocker : UserCallback<Output::UnlockCallback>
			{
				void operator () (Output& output) const
				{
					if (function)
						function( userdata, output );
				}
			};
		}
	}

	namespace Api
	{
		class Video : public Base
		{
		public:

			Video(Emulator& e)
			: Base(e) {}

			typedef Core::Video::Output Output;

			enum
			{
				MIN_BRIGHTNESS                  = -100,
				DEFAULT_BRIGHTNESS              =    0,
				MAX_BRIGHTNESS                  = +100,
				MIN_SATURATION                  = -100,
				DEFAULT_SATURATION              =    0,
				MAX_SATURATION                  = +100,
				MIN_CONTRAST                    = -100,
				DEFAULT_CONTRAST                =    0,
				MAX_CONTRAST                    = +100,
				MIN_SHARPNESS                   = -100,
				DEFAULT_SHARPNESS_COMP          =    0,
				DEFAULT_SHARPNESS_SVIDEO        =   20,
				DEFAULT_SHARPNESS_RGB           =   20,
				MAX_SHARPNESS                   = +100,
				MIN_COLOR_RESOLUTION            = -100,
				DEFAULT_COLOR_RESOLUTION_COMP   =    0,
				DEFAULT_COLOR_RESOLUTION_SVIDEO =   20,
				DEFAULT_COLOR_RESOLUTION_RGB    =   70,
				MAX_COLOR_RESOLUTION            = +100,
				MIN_COLOR_BLEED                 = -100,
				DEFAULT_COLOR_BLEED_COMP        =    0,
				DEFAULT_COLOR_BLEED_SVIDEO      =    0,
				DEFAULT_COLOR_BLEED_RGB         = -100,
				MAX_COLOR_BLEED                 = +100,
				MIN_COLOR_ARTIFACTS             = -100,
				DEFAULT_COLOR_ARTIFACTS_COMP    =    0,
				DEFAULT_COLOR_ARTIFACTS_SVIDEO  = -100,
				DEFAULT_COLOR_ARTIFACTS_RGB     = -100,
				MAX_COLOR_ARTIFACTS             = +100,
				MIN_COLOR_FRINGING              = -100,
				DEFAULT_COLOR_FRINGING_COMP     =    0,
				DEFAULT_COLOR_FRINGING_SVIDEO   = -100,
				DEFAULT_COLOR_FRINGING_RGB      = -100,
				MAX_COLOR_FRINGING              = +100,
				MIN_HUE                         =  -45,
				DEFAULT_HUE                     =    0,
				MAX_HUE                         =  +45
			};

			void EnableUnlimSprites(bool);
			bool AreUnlimSpritesEnabled() const;

			int GetBrightness() const;
			int GetSaturation() const;
			int GetContrast() const;
			int GetSharpness() const;
			int GetColorResolution() const;
			int GetColorBleed() const;
			int GetColorArtifacts() const;
			int GetColorFringing() const;
			int GetHue() const;

			Result SetBrightness(int);
			Result SetSaturation(int);
			Result SetContrast(int);
			Result SetSharpness(int);
			Result SetColorResolution(int);
			Result SetColorBleed(int);
			Result SetColorArtifacts(int);
			Result SetColorFringing(int);
			Result SetHue(int);

			void EnableFieldMerging(bool);
			bool IsFieldMergingEnabled() const;

			Result Blit(Output&);

			enum DecoderPreset
			{
				DECODER_CANONICAL,
				DECODER_CONSUMER,
				DECODER_ALTERNATIVE
			};

			struct Decoder
			{
				Decoder() {}
				Decoder(DecoderPreset);

				bool operator == (const Decoder&) const;

				enum
				{
					AXIS_RY,
					AXIS_GY,
					AXIS_BY,
					NUM_AXES
				};

				struct Axis
				{
					uint angle;
					float gain;
				};

				Axis axes[NUM_AXES];
				bool boostYellow;
			};

			Result SetDecoder(const Decoder&);
			const Decoder& GetDecoder() const;

			class Palette;
			friend class Palette;

			class Palette
			{
				friend class Video;

				struct UpdateCaller;

				Video& video;

				Palette(Video& v)
				: video(v) {}

			public:

				enum
				{
					NUM_ENTRIES = 64
				};

				enum Mode
				{
					MODE_YUV,
					MODE_RGB,
					MODE_CUSTOM
				};

				typedef const u8 (*Colors)[3];

				inline Mode   GetMode() const;
				inline Mode   GetDefaultMode() const;
				inline Result SetCustom(Colors);
				inline void   ResetCustom();
				inline Colors GetColors() const;
				inline Result SetMode(Mode);

				typedef void (NST_CALLBACK *UpdateCallback) (UserData,Colors);

				static UpdateCaller updateCallback;
			};

		private:

			Palette::Mode GetPaletteMode() const;
			Palette::Mode GetDefaultPaletteMode() const;
			Result SetCustomPalette(Palette::Colors);
			void ResetCustomPalette();
			Palette::Colors GetPaletteColors() const;
			Result SetPaletteMode(Palette::Mode);

		public:

			Palette GetPalette()
			{
				return *this;
			}

			struct RenderState
			{
				struct Bits
				{
					struct Mask
					{
						ulong r,g,b;
					};

					Mask mask;
					uchar count;
				};

				Bits bits;
				uchar paletteOffset;
				ushort width;
				ushort height;

				enum Scanlines
				{
					SCANLINES_NONE = 0,
					SCANLINES_MAX = 100
				};

				uint scanlines;

				enum Filter
				{
					FILTER_NONE
				#ifndef NST_NO_NTSCVIDEO
					,FILTER_NTSC
				#endif
				#ifndef NST_NO_2XSAI
					,FILTER_2XSAI
					,FILTER_SUPER_2XSAI
					,FILTER_SUPER_EAGLE
				#endif
				#ifndef NST_NO_SCALE2X
					,FILTER_SCALE2X
					,FILTER_SCALE3X
				#endif
				#ifndef NST_NO_HQ2X
					,FILTER_HQ2X
					,FILTER_HQ3X
				#endif
				};

				enum Scale
				{
					SCALE_NONE_SCANLINES = 2,
				#ifndef NST_NO_2XSAI
					SCALE_2XSAI = 2,
					SCALE_SUPER_2XSAI = 2,
					SCALE_SUPER_EAGLE = 2,
				#endif
				#ifndef NST_NO_SCALE2X
					SCALE_SCALE2X = 2,
					SCALE_SCALE3X = 3,
				#endif
				#ifndef NST_NO_HQ2X
					SCALE_HQ2X = 2,
					SCALE_HQ3X = 3,
				#endif
					SCALE_NONE = 1
				};

				Filter filter;
			};

			Result SetRenderState(const RenderState&);
			Result GetRenderState(RenderState&) const;
		};

		inline Video::Palette::Mode Video::Palette::GetMode() const
		{
			return video.GetPaletteMode();
		}

		inline Video::Palette::Mode Video::Palette::GetDefaultMode() const
		{
			return video.GetDefaultPaletteMode();
		}

		inline Result Video::Palette::SetCustom(Colors colors)
		{
			return video.SetCustomPalette( colors );
		}

		inline void Video::Palette::ResetCustom()
		{
			video.ResetCustomPalette();
		}

		inline Video::Palette::Colors Video::Palette::GetColors() const
		{
			return video.GetPaletteColors();
		}

		inline Result Video::Palette::SetMode(Mode mode)
		{
			return video.SetPaletteMode( mode );
		}

		struct Video::Palette::UpdateCaller : Core::UserCallback<Video::Palette::UpdateCallback>
		{
			void operator () (Colors colors) const
			{
				if (function)
					function( userdata, colors );
			}
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
