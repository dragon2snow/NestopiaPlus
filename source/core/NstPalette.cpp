////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
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

#include "NstTypes.h"
#include "NstPalette.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const U8 PALETTE::DefaultPalette[64][3] =
{
	{0x6D,0x6D,0x6D}, {0x00,0x00,0xB3}, {0x45,0x00,0xBD}, {0x5C,0x00,0xA1}, 
	{0x79,0x00,0x73}, {0x7E,0x00,0x00}, {0x79,0x28,0x00}, {0x61,0x39,0x00},  
	{0x44,0x50,0x00}, {0x00,0x5C,0x00}, {0x00,0x5C,0x00}, {0x00,0x50,0x50},
	{0x00,0x44,0x89}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
	{0xB3,0xB3,0xB3}, {0x00,0x56,0xFF}, {0x50,0x00,0xFF}, {0x7E,0x00,0xF8},
	{0xAD,0x00,0xBE}, {0xBD,0x00,0x56}, {0xBD,0x44,0x00}, {0xA1,0x67,0x00},  
	{0x7E,0x7E,0x00}, {0x00,0x90,0x00}, {0x00,0x95,0x00}, {0x00,0x8A,0x68},
	{0x00,0x79,0xBD}, {0x22,0x22,0x22}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
	{0xFF,0xFF,0xFF}, {0x00,0xA1,0xFF}, {0x84,0x84,0xFF}, {0xAC,0x61,0xFF}, 	
	{0xE7,0x67,0xFF}, {0xFF,0x67,0xBE}, {0xFF,0x79,0x55}, {0xEC,0x9C,0x00}, 
	{0xD0,0xBE,0x00}, {0x90,0xD5,0x00}, {0x00,0xE0,0x00}, {0x00,0xDB,0x84},
	{0x00,0xCF,0xE1}, {0x56,0x56,0x56}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
	{0xFF,0xFF,0xFF}, {0xBD,0xE0,0xFF}, {0xCF,0xCF,0xFF}, {0xE0,0xC4,0xFF}, 
	{0xF8,0xBE,0xFF}, {0xFF,0xBE,0xEC}, {0xFF,0xCF,0xBE}, {0xFF,0xDB,0xA1}, 
	{0xEC,0xE7,0x90}, {0xD5,0xF3,0x95}, {0xBE,0xF8,0xAD}, {0xAD,0xF8,0xD0}, 
	{0xAD,0xF1,0xF3}, {0xB8,0xB8,0xB8}, {0x00,0x00,0x00}, {0x00,0x00,0x00}
};

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const DOUBLE PALETTE::emphasis[8][3] =
{
	{1.000, 1.000, 1.000},
	{1.239, 0.915, 0.743},
	{0.794, 1.086, 0.882},
	{1.019, 0.980, 0.653},
	{0.905, 1.026, 1.277},
	{1.023, 0.908, 0.979},
	{0.741, 0.987, 1.001},
	{0.750, 0.750, 0.750}
};

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PALETTE::PALETTE()
: 
NeedUpdate  (TRUE),
type        (EMULATE),
hue         (DEFAULT_HUE),
brightness  (DEFAULT_BRIGHTNESS),
saturation  (DEFAULT_SATURATION)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PALETTE::SetType(const TYPE t)
{
	if (type != t)
	{
		type = t;
		NeedUpdate = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PALETTE::SetBrightness(const INT level)
{
	if (brightness != level)
	{
		brightness = level;
		NeedUpdate = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PALETTE::SetSaturation(const INT level)
{
	if (saturation != level)
	{
		saturation = level;
		NeedUpdate = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PALETTE::SetHue(const INT level)
{
	if (hue != level)
	{
		hue = level;
		NeedUpdate = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PALETTE::Import(const U8* const input)
{
	if (input)
	{
		if (!NeedUpdate && type == CUSTOM)
			NeedUpdate = TRUE;

		memcpy( custom, input, sizeof(U8) * 64 * 3 );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PALETTE::ToHSV(DOUBLE r,DOUBLE g,DOUBLE b,DOUBLE& h,DOUBLE& s,DOUBLE& v)
{
	const DOUBLE min = PDX_MIN( r, PDX_MIN( g, b ));
	const DOUBLE max = PDX_MAX( r, PDX_MAX( g, b ));

	v = max;

	if (max != 0)
	{
		const DOUBLE delta = max - min;

		s = delta / max;

		     if (r == max) h = 0 + (g - b) / delta;
		else if (g == max) h = 2 + (b - r) / delta;
		else			   h = 4 + (r - g) / delta;

		h *= 60;

		if (h < 0)
			h += 360;
	}
	else
	{
		s =  0;
		h = -1;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PALETTE::ToRGB(DOUBLE h,DOUBLE s,DOUBLE v,DOUBLE& r,DOUBLE& g,DOUBLE& b)
{
	if (s == 0)
	{
		r = g = b = v;
	}
	else
	{
		h /= 60;
		
		const INT i = floor(h);

		const DOUBLE f = h - i;
		const DOUBLE p = v * ( 1 - s );
		const DOUBLE q = v * ( 1 - s * f );
		const DOUBLE t = v * ( 1 - s * ( 1 - f ) );

		switch (i) 
		{
     		case 0:  r = v; g = t; b = p; return;
			case 1:  r = q; g = v; b = p; return;
			case 2:  r = p; g = v; b = t; return;
			case 3:  r = p; g = q; b = v; return;
			case 4:  r = t; g = p; b = v; return;
			default: r = v; g = p; b = q; return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PALETTE::ComputeCustom()
{
	PDX_ASSERT(type == CUSTOM || type == INTERNAL);

	const DOUBLE bri = (brightness - 128) / 255.0;
	const DOUBLE sat = (((saturation - 128) / 255.0) * 2) + 1;
	const INT hof = (hue - 128) / 4;

	NeedUpdate = FALSE;

	const U8 (*const from)[3] = (type == CUSTOM ? custom : DefaultPalette);

	for (UINT i=0; i < 8; ++i)
	{
		for (UINT j=0; j < 64; ++j)
		{
			DOUBLE r = from[j][0] / 255.0;
			DOUBLE g = from[j][1] / 255.0;
			DOUBLE b = from[j][2] / 255.0;

			DOUBLE h,s,v;

			ToHSV(r,g,b,h,s,v);

			s *= sat;
			v += bri;
			h -= hof;

			if (h >= 360)
				h -= 360;
			else if (h < 0)
				h += 360;

			ToRGB(h,s,v,r,g,b);

			r *= emphasis[i][0];
			g *= emphasis[i][1]; 
			b *= emphasis[i][2]; 

			palette[(i * 64) + j][0] = U8(PDX_CLAMP(r * 255,0,255));
			palette[(i * 64) + j][1] = U8(PDX_CLAMP(g * 255,0,255));
			palette[(i * 64) + j][2] = U8(PDX_CLAMP(b * 255,0,255));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PALETTE::ComputeTV()
{
	PDX_ASSERT(type == EMULATE);

	NeedUpdate = FALSE;

	const DOUBLE sat = PDX_MAX((saturation / 255.0) - 0.10,0.0);
	const DOUBLE bri = (brightness - 128) / 255.0;
	const DOUBLE hof = HUE_OFFSET + ((hue - 128) / 4);

	for (UINT index=0; index < 8; ++index)
	{
		for (UINT voltage=0; voltage < 4; ++voltage)
		{
			INT angle = hof;

			for (UINT phase=0; phase < 16; ++phase)
			{
				static const DOUBLE luma[4][4] =
				{
					{0.50, 0.75, 1.00, 1.00},
					{0.29, 0.45, 0.73, 0.90},
					{0.00, 0.24, 0.47, 0.77},
					{0.00, 0.00, 0.00, 0.00}
				};

				DOUBLE Y;
				DOUBLE S;

				switch (phase)
				{
	     			case 0:  S = 0.0; Y = luma[0][voltage]; break;
	       			case 13: S = 0.0; Y = luma[2][voltage]; break;
	       			case 14:
	       			case 15: S = 0.0; Y = luma[3][voltage]; break;
	       			default: S = sat; Y = luma[1][voltage]; break;
				}

				const DOUBLE theta = NES_DEG * angle;

				const DOUBLE R_Y = S * sin(theta);
				const DOUBLE B_Y = S * cos(theta);

				const DOUBLE r = bri + (emphasis[index][0] * (Y + R_Y));
				const DOUBLE g = bri + (emphasis[index][1] * (Y - (27.0 / 53.0) * R_Y + (10.0 / 53.0) * B_Y));
				const DOUBLE b = bri + (emphasis[index][2] * (Y - B_Y));

				palette[(index * 64) + (voltage * 16) + phase][0] = U8(PDX_CLAMP(r * 255,0,255));
				palette[(index * 64) + (voltage * 16) + phase][1] = U8(PDX_CLAMP(g * 255,0,255));
				palette[(index * 64) + (voltage * 16) + phase][2] = U8(PDX_CLAMP(b * 255,0,255));

				angle -= (360 / 12);

				if (angle < 0)
					angle += 360;
			}
		}
	}
}

NES_NAMESPACE_END
