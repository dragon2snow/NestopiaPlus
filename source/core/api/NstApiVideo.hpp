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
			class Palette;

			class Output
			{
				struct Locker;
				struct Unlocker;

			public:

				enum
				{
					WIDTH = 256,
					HEIGHT = 240
				};

				void* pixels;
				long pitch;

				Output()
				: pixels(NULL), pitch(0) {}

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

			void EnableUnlimSprites(bool);
			bool AreUnlimSpritesEnabled() const;
	
			uint GetDefaultBrightness() const;
			uint GetDefaultSaturation() const;
			uint GetDefaultHue() const;
	
			uint GetBrightness() const;
			uint GetSaturation() const;
			uint GetHue() const;
	
			Result SetBrightness(uint);
			Result SetSaturation(uint);
			Result SetHue(uint);
	
			Result Blit(Output&);

			class Palette
			{
				friend class Video;

				struct UpdateCaller;

				Core::Video::Palette& palette;

				Palette(Core::Video::Palette& p)
				: palette(p) {}

			public:

				enum 
				{
					NUM_ENTRIES = 64
				};

				enum Mode
				{
					INTERNAL,
					CUSTOM,
					EMULATED
				};

				typedef const u8 (*Colors)[3];

				Result SetMode(Mode);
				Mode   GetMode() const;
				Mode   GetDefaultMode() const;
				Result SetCustom(Colors);
				Colors GetColors() const;

				typedef void (NST_CALLBACK *UpdateCallback) (UserData,Colors);

				static UpdateCaller updateCallback;
			};

			Palette GetPalette() const;

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
				
				enum
				{
					SCALE_NONE = 1,
					SCALE_TV = 2,
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
					SCALE_SCANLINES_MAX = 2
				};
	
				uchar scale;
				uchar paletteOffset;
	
				enum Filter
				{
					FILTER_NONE,
					FILTER_SCANLINES_BRIGHT,
					FILTER_SCANLINES_DARK,
                #ifndef NST_NO_2XSAI
					FILTER_2XSAI,
					FILTER_SUPER_2XSAI,
					FILTER_SUPER_EAGLE,
                #endif
                #ifndef NST_NO_SCALE2X
					FILTER_SCALE2X,
					FILTER_SCALE3X,
                #endif
                #ifndef NST_NO_HQ2X
					FILTER_HQ2X,
					FILTER_HQ3X,
                #endif
					FILTER_TV
				};

				Filter filter;
			};
	
			Result SetRenderState(const RenderState&);
			Result GetRenderState(RenderState&) const;
		};

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
