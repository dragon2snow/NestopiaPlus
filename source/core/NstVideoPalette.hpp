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

#ifndef NST_VIDEO_PALETTE_H
#define NST_VIDEO_PALETTE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Palette
			{
			public:
		
				enum Type
				{
					INTERNAL,
					CUSTOM,
					EMULATE
				};
		
				enum
				{
					DEFAULT_HUE = 128,
					DEFAULT_BRIGHTNESS = 128,
					DEFAULT_SATURATION = 128,
					DEFAULT_PALETTE_TYPE = INTERNAL
				};
		
				typedef const u8 (*Colors)[3];
		
				Palette();
				~Palette();
		
				Result SetType(Type);
				Result SetCustomColors(Colors);
				void   ResetCustomColors();
		
			private:
		
				enum
				{
					HUE_OFFSET = 255,
					HUE_ROTATION = 360 / 12
				};
		
				void ComputeTV();
				void ComputeCustom();
		
				Result SetColor(int&,uint);
		
				static void ToPAL(const double (&)[3],u8 (&)[3]);
				static void ToHSV(double,double,double,double&,double&,double&);
				static void ToRGB(double,double,double,double&,double&,double&);
		
				ibool update;
				Type type;
		
				int hue;
				int brightness;
				int saturation;
		
				u8 palette[64*8][3];
				u8 (*custom)[3];
		
				static const double emphasis[8][3];
				static const u8 defaultPalette[64][3];
		
			public:
		
				uint GetBrightness() const
				{
					return brightness;
				}
		
				uint GetSaturation() const
				{
					return saturation;
				}
		
				uint GetHue() const
				{
					return hue;
				}
		
				Result SetBrightness(uint value)
				{
					return SetColor( brightness, value );
				}
		
				Result SetSaturation(uint value)
				{
					return SetColor( saturation, value );
				}
		
				Result SetHue(uint value)
				{
					return SetColor( hue, value );
				}
		
				Type GetType() const
				{
					return type;
				}
		
				ibool NeedUpdate() const
				{
					return update;
				}
		
				Colors GetColors()
				{
					if (update)
					{
						if (type == EMULATE) ComputeTV();
						else                 ComputeCustom();
					}
		
					return palette;
				}
		
				Colors GetColorsNoChange() const
				{
					return palette;
				}
			};
		}
	}
}

#endif
