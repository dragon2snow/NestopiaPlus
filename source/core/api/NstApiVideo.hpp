////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 304 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
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

				Output(void* v=0,long p=0)
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

			template<typename T>
			Video(T& e)
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

			Result EnableUnlimSprites(bool) throw();
			bool AreUnlimSpritesEnabled() const throw();

			int GetBrightness() const throw();
			int GetSaturation() const throw();
			int GetContrast() const throw();
			int GetSharpness() const throw();
			int GetColorResolution() const throw();
			int GetColorBleed() const throw();
			int GetColorArtifacts() const throw();
			int GetColorFringing() const throw();
			int GetHue() const throw();

			Result SetBrightness(int) throw();
			Result SetSaturation(int) throw();
			Result SetContrast(int) throw();
			Result SetSharpness(int) throw();
			Result SetColorResolution(int) throw();
			Result SetColorBleed(int) throw();
			Result SetColorArtifacts(int) throw();
			Result SetColorFringing(int) throw();
			Result SetHue(int) throw();

			void EnableFieldMerging(bool) throw();
			bool IsFieldMergingEnabled() const throw();

			Result Blit(Output&) throw();

			enum DecoderPreset
			{
				DECODER_CANONICAL,
				DECODER_CONSUMER,
				DECODER_ALTERNATIVE
			};

			struct Decoder
			{
				Decoder(DecoderPreset=DECODER_CANONICAL) throw();

				bool operator == (const Decoder&) const throw();
				bool operator != (const Decoder&) const throw();

				enum
				{
					AXIS_RY,
					AXIS_GY,
					AXIS_BY,
					NUM_AXES
				};

				struct
				{
					float gain;
					uint angle;
				}   axes[NUM_AXES];

				uint boostYellow;
			};

			Result SetDecoder(const Decoder&) throw();
			const Decoder& GetDecoder() const throw();

			class Palette
			{
				Core::Machine& emulator;

			public:

				Palette(Core::Machine& e)
				: emulator(e) {}

				enum
				{
					NUM_ENTRIES = 64,
					NUM_ENTRIES_EXT = 512
				};

				enum CustomType
				{
					STD_PALETTE = NUM_ENTRIES,
					EXT_PALETTE = NUM_ENTRIES_EXT
				};

				enum Mode
				{
					MODE_YUV,
					MODE_RGB,
					MODE_CUSTOM
				};

				typedef const uchar (*Colors)[3];

				Mode       GetMode() const throw();
				Mode       GetDefaultMode() const throw();
				Result     SetCustom(Colors,CustomType=STD_PALETTE) throw();
				uint       GetCustom(uchar (*)[3],CustomType) const throw();
				void       ResetCustom() throw();
				CustomType GetCustomType() const throw();
				Colors     GetColors() const throw();
				Result     SetMode(Mode) throw();
			};

			Palette GetPalette()
			{
				return emulator;
			}

			struct RenderState
			{
				RenderState() throw();

				struct Bits
				{
					struct Mask
					{
						ulong r,g,b;
					};

					Mask mask;
					uint count;
				};

				Bits bits;
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
					FILTER_NONE,
					FILTER_NTSC
				#ifndef NST_NO_SCALEX
					,FILTER_SCALE2X
					,FILTER_SCALE3X
				#endif
				#ifndef NST_NO_HQ2X
					,FILTER_HQ2X
					,FILTER_HQ3X
					,FILTER_HQ4X
				#endif
				};

				enum Scale
				{
					SCALE_NONE_SCANLINES = 2,
				#ifndef NST_NO_SCALEX
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

			Result SetRenderState(const RenderState&) throw();
			Result GetRenderState(RenderState&) const throw();
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
