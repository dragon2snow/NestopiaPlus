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

#include <new>
#include <cmath>
#include <cfloat>
#include "NstCore.hpp"
#include "NstVideoPalette.hpp"
#include <cstdio>

#ifdef __INTEL_COMPILER
#pragma warning( disable : 1572 )
#endif

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{		   
		namespace Video
		{
			const u8 Palette::defaultPalette[64][3] =
			{
				{0x68,0x68,0x68}, {0x00,0x12,0x99}, {0x1A,0x08,0xAA}, {0x51,0x02,0x9A}, 
				{0x7E,0x00,0x69}, {0x8E,0x00,0x1C}, {0x7E,0x03,0x01}, {0x51,0x18,0x00}, 
				{0x1F,0x37,0x00}, {0x01,0x4E,0x00}, {0x00,0x5A,0x00}, {0x00,0x50,0x1C}, 
				{0x00,0x40,0x61}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, 
				{0xB9,0xB9,0xB9}, {0x0C,0x5C,0xD7}, {0x50,0x35,0xF0}, {0x89,0x19,0xE0}, 
				{0xBB,0x0C,0xB3}, {0xCE,0x0C,0x61}, {0xC0,0x2B,0x0E}, {0x95,0x4D,0x01}, 
				{0x61,0x6F,0x00}, {0x1F,0x8B,0x00}, {0x01,0x98,0x0C}, {0x00,0x93,0x4B}, 
				{0x00,0x81,0x9B}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
				{0xFF,0xFF,0xFF}, {0x63,0xB4,0xFF}, {0x9B,0x91,0xFF}, {0xD3,0x77,0xFF}, 
				{0xEF,0x6A,0xFF}, {0xF9,0x68,0xC0}, {0xF9,0x7D,0x6C}, {0xED,0x9B,0x2D}, 
				{0xBD,0xBD,0x16}, {0x7C,0xDA,0x1C}, {0x4B,0xE8,0x47}, {0x35,0xE5,0x91}, 
				{0x3F,0xD9,0xDD}, {0x60,0x60,0x60}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, 
				{0xFF,0xFF,0xFF}, {0xAC,0xE7,0xFF}, {0xD5,0xCD,0xFF}, {0xED,0xBA,0xFF}, 
				{0xF8,0xB0,0xFF}, {0xFE,0xB0,0xEC}, {0xFD,0xBD,0xB5}, {0xF9,0xD2,0x8E}, 
				{0xE8,0xEB,0x7C}, {0xBB,0xF3,0x82}, {0x99,0xF7,0xA2}, {0x8A,0xF5,0xD0}, 
				{0x92,0xF4,0xF1}, {0xBE,0xBE,0xBE}, {0x00,0x00,0x00}, {0x00,0x00,0x00} 
			};

			const double Palette::emphasis[8][3] =
			{
				{1.00,1.00,1.00},
				{1.00,0.80,0.81},
				{0.78,0.94,0.66},
				{0.79,0.77,0.63},
				{0.82,0.83,1.12},
				{0.81,0.71,0.87},
				{0.68,0.79,0.79},
				{0.70,0.70,0.70}
			};
		
			Palette::Palette()
			: 
			update      (true),
			type        (INTERNAL),
			hue         (DEFAULT_HUE),
			brightness  (DEFAULT_BRIGHTNESS),
			saturation  (DEFAULT_SATURATION),
			custom      (NULL)
			{}
		
			Palette::~Palette()
			{
				delete [] custom;
			}
		
			void Palette::ToHSV(double r,double g,double b,double& h,double& s,double& v)
			{
				const double min = NST_MIN( r, NST_MIN( g, b ));
				const double max = NST_MAX( r, NST_MAX( g, b ));
		
				v = max;
						   
				if (max > +FLT_EPSILON || max < -FLT_EPSILON)
				{
					const double delta = max - min;
		
					s = delta / max;
		
					if      (r == max) h = 0.0 + (g - b) / delta;
					else if (g == max) h = 2.0 + (b - r) / delta;
					else			   h = 4.0 + (r - g) / delta;
		
					h *= 60.0;
		
					if (h < 0.0)
						h += 360.0;
				}
				else
				{
					s =  0.0;
					h = -1.0;
				}
			}
		
			void Palette::ToRGB(double h,double s,double v,double& r,double& g,double& b)
			{
				if (s <= +FLT_EPSILON && s >= -FLT_EPSILON)
				{
					r = g = b = v;
				}
				else
				{
					h /= 60.0;
		
					const int i = std::floor(h);
		
					const double f = h - i;
					const double p = v * ( 1.0 - s );
					const double q = v * ( 1.0 - s * f );
					const double t = v * ( 1.0 - s * ( 1.0 - f ) );
		
					switch (i) 
					{
						case 0:  r = v; g = t; b = p; break;
						case 1:  r = q; g = v; b = p; break;
						case 2:  r = p; g = v; b = t; break;
						case 3:  r = p; g = q; b = v; break;
						case 4:  r = t; g = p; b = v; break;
						default: r = v; g = p; b = q; break;
					}
				}
			}
		
			void Palette::ToPAL(const double (&src)[3],u8 (&dst)[3])
			{
				for (uint i=0; i != 3; ++i)
				{
					int final = (int) (src[i] * 255.0 + 0.5);
					dst[i] = (u8) NST_CLAMP( final, 0, 255 );
				}
			}
		
			Result Palette::SetColor(int& dst,const uint src)
			{
				if (src > 255)
					return RESULT_ERR_INVALID_PARAM;
		
				if (int(src) == dst)
					return RESULT_NOP;
		
				dst = src;
				update = true;
		
				return RESULT_OK;
			}
		
			Result Palette::SetCustomColors(Colors colors)
			{
				if (!colors || colors == custom)
					return RESULT_ERR_INVALID_PARAM;
		
				if (!custom)
				{
					try
					{
						custom = new u8 [64][3];
					}
					catch (const std::bad_alloc&)
					{
						return RESULT_ERR_OUT_OF_MEMORY;
					}
				}
		
				update = true;
		
				for (uint i=0; i < 64; ++i)
					for (uint j=0; j < 3; ++j)
						custom[i][j] = colors[i][j];
		
				return RESULT_OK;
			}

			void Palette::ResetCustomColors()
			{
				if (custom)
				{
					for (uint i=0; i < 64; ++i)
						for (uint j=0; j < 3; ++j)
							custom[i][j] = defaultPalette[i][j];
				}
			}
		
			Result Palette::SetType(const Type t)
			{
				if (type != INTERNAL && type != CUSTOM && type != EMULATE)
					return RESULT_ERR_INVALID_PARAM;
		
				if (type == t)
					return RESULT_NOP;
		
				if (t == CUSTOM && !custom)
				{
					try
					{
						custom = new u8 [64][3];
					}
					catch (const std::bad_alloc&)
					{
						return RESULT_ERR_OUT_OF_MEMORY;
					}
		
					ResetCustomColors();
				}
		
				type = t;
				update = true;
		
				return RESULT_OK;
			}
		
			void Palette::ComputeCustom()
			{
				NST_ASSERT( type == CUSTOM || type == INTERNAL );
		
				const double bri = (brightness - 128) / 255.0;
				const double sat = (((saturation - 128) / 255.0) * 2) + 1;
				const int hof = (hue - 128) / 4;
		
				update = false;
		
				Colors from = (type == CUSTOM ? custom : defaultPalette);
				NST_ASSERT( from );
		
				for (uint i=0; i < 8; ++i)
				{
					for (uint j=0; j < 64; ++j)
					{
						double rgb[3] = 
						{
							from[j][0] / 255.0,
							from[j][1] / 255.0,
							from[j][2] / 255.0
						};
		
						double h,s,v;
		
						ToHSV( rgb[0], rgb[1], rgb[2], h, s, v );
		
						s *= sat;
						v += bri;
						h -= hof;
		
						if (h >= 360.0)
							h -= 360.0;
						else if (h < 0.0)
							h += 360.0;
		
						ToRGB( h, s, v, rgb[0], rgb[1], rgb[2] );
		
						rgb[0] *= emphasis[i][0];
						rgb[1] *= emphasis[i][1]; 
						rgb[2] *= emphasis[i][2]; 
		
						ToPAL( rgb, palette[(i * 64) + j] );
					}
				}
			}
		
			void Palette::ComputeTV()
			{
				NST_ASSERT( type == EMULATE );
		
				update = false;
		
				const double sat = NST_MAX((saturation / 255.0) - 0.267,0.0);
				const double bri = (brightness - 128) / 255.0;
				const uint hof = HUE_OFFSET - ((hue - 128) / 4);
		
				static const double luma[3][4] =
				{
					{0.50, 0.75, 1.00, 1.00}, 
					{0.29, 0.45, 0.73, 0.90}, 
					{0.00, 0.24, 0.47, 0.77}
				};
		
				for (uint index=0; index != 8; ++index)
				{
					for (uint voltage=0; voltage != 4; ++voltage)
					{
						for (uint phase=0,angle=0; phase != 16; ++phase)
						{
							double s, y;
		
							switch (phase)
							{
								case 0: 
							
									s = 0.0; 
									y = luma[0][voltage]; 
									break;
							
								case 13: 
							
									angle = 0;
									s = 0.0; 
									y = luma[2][voltage]; 
									break;
							
								case 14:
								case 15: 
							
									s = 0.0;
									y = 0.0; 
									break;
							
								case 1:
							
									angle = hof;
							
								default: 
							
									angle += HUE_ROTATION;
							
									if (angle >= 360)
										angle -= 360;
							
									s = sat; 
									y = luma[1][voltage]; 
									break;
							}
		
							const double h = NST_DEG * angle;
							const double i = s * std::sin( h );
							const double q = s * std::cos( h );
		
							const double rgb[] = 
							{
								bri + ( emphasis[index][0] * (y + 0.956 * i + 0.621 * q) ),
								bri + ( emphasis[index][1] * (y - 0.272 * i - 0.647 * q) ), 
								bri + ( emphasis[index][2] * (y - 1.106 * i + 1.703 * q) ) 
							};
		
							ToPAL( rgb, palette[(index * 64) + (voltage * 16) + phase] );
						}
					}
				}
			}
		}
	}
}

#undef NST_DEG

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
