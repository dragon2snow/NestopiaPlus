////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
// Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
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

#include "NstCore.hpp"

#ifndef NST_NO_NTSCVIDEO

#include "api/NstApiVideo.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterNtsc.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			void BaseFilterNtscLut::Build
			(
		     	i32 (&nesPhases)[PHASE_COUNT][PALETTE_SIZE][4],
				i32 (&rgbPhases)[PHASE_COUNT][6],
				u8* const NST_RESTRICT toInt,
				const int colorRange,
				int brightness,
				const uint iSaturation,
				const int iHue
			)
			{
				brightness = colorRange*2 - ((brightness - 128) / (256 / (colorRange-1)));

				for (int i=0, n=colorRange*5; i < n; ++i)
					toInt[i] = NST_CLAMP(i-brightness,0,colorRange-1);

				double hue;

				if (iHue != 128)
					hue = (iHue - 128) / 512.0 * NST_PI;
				else
					hue = 0.0;

				const double saturation = iSaturation / 128.0;

				for (uint phase=0; phase < PHASE_COUNT; ++phase)
				{
					{
						const double angle = NST_PI / -12 * int(phase * 8 + 1) + hue;
						const double s = std::sin( angle ) * saturation;
						const double c = std::cos( angle ) * saturation;

						for (uint i=0; i < 6; i += 2)
						{
							static const double rgb[6] = {0.956,0.621,-0.272,-0.647,-1.105,1.702};

							rgbPhases[phase][i+0] = (rgb[i+0] * c - rgb[i+1] * s) * FIXED_MUL;
							rgbPhases[phase][i+1] = (rgb[i+0] * s + rgb[i+1] * c) * FIXED_MUL;
						}
					}

					for (uint c=0; c < PALETTE_SIZE; ++c)
					{
						const uint ph = c & 0xF;

						double y = (c >> 4) * 0.375;

						if (ph == 0)
						{
							y += 0.5;
						}
						else if (ph == 0xD)
						{
							y -= 0.5;
						}
						else if (ph > 0xD)
						{
							y = 0.0;
						}

						double i=0.0, q=0.0;

						if (ph-1U < 0xCU)
						{
							const double angle = NST_PI / 6 * (int(phase * 4 + ph) - 3);

							i = std::sin( angle ) / 2;
							q = std::cos( angle ) / 2;
						}

						const double v[] =
						{
							y + i,
							y + q,
							y - i,
							y - q
						};

						for (uint j=0; j < 4; ++j)
						{
							if (v[j] >= 1.2)
							{
								nesPhases[phase][c][j] = 1.2 * (colorRange * FIXED_MUL);
							}
							else if (v[j] <= -0.2)
							{
								nesPhases[phase][c][j] = -0.2 * (colorRange * FIXED_MUL);
							}
							else
							{
								nesPhases[phase][c][j] = v[j] * (colorRange * FIXED_MUL);
							}

							if (j >= 2)
								nesPhases[phase][c][j] = -nesPhases[phase][c][j];
						}
					}	
				}
			}
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif

#endif
