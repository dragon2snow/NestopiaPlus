////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
// Copyright (C) 2006 Shay Green
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
				const uint sat,
				int hue,
				const Api::Video::Decoder& decoder
			)
			{
				brightness = colorRange*2 - ((brightness - 128) / (256 / (colorRange-1)));

				for (int i=0, n=colorRange*5; i < n; ++i)
					toInt[i] = NST_CLAMP(i-brightness,0,colorRange-1);

				const double saturation = sat / 128.0;
				hue = (hue - 128) / 4 + 33;

				double rgb[3][2];

				for (uint i=0; i < 3; ++i)
				{
					rgb[i][0] = std::sin( decoder.axes[i].angle * NST_DEG ) * decoder.axes[i].gain * 2;
					rgb[i][1] = std::cos( decoder.axes[i].angle * NST_DEG ) * decoder.axes[i].gain * 2;
				}

				for (uint phase=0; phase < PHASE_COUNT; ++phase)
				{
					{
						const double angle = (int(phase * 8 + 1) * -15 + hue) * NST_DEG;
						const double v = std::sin( angle ) * saturation;
						const double u = std::cos( angle ) * saturation;

						for (uint i=0; i < 3; ++i)
						{
							rgbPhases[phase][i*2+0] = (rgb[i][0] * u - rgb[i][1] * v) * FIXED_MUL;
							rgbPhases[phase][i*2+1] = (rgb[i][0] * v + rgb[i][1] * u) * FIXED_MUL;
						}
					}

					for (uint level=0; level < 4; ++level)
					{
						for (uint index=0; index < 16; ++index)
						{
							double y=0.0, i=0.0, q=0.0;

							if (index-1U < 12)
							{
								static const double chroma[4] = 
								{ 
									0.26, 0.33, 0.34, 0.14
								};

								const double angle = ((int(phase * 4 + index) - 3) * (360/12)) * NST_DEG;

								i = std::sin( angle ) * chroma[level]; 
								q = std::cos( angle ) * chroma[level]; 
							}

							if (index < 14)
							{
								static const double luma[3][4] = 
								{
									{ 0.39, 0.67, 1.00, 1.00 },
									{ 0.14, 0.34, 0.66, 0.86 },
									{-0.12, 0.00, 0.31, 0.72 }
								};

								y = luma[index == 0 ? 0 : index < 13 ? 1 : 2][level];
							}

							i32 (&dst)[4] = nesPhases[phase][(level * 16) + index];

							dst[0] = (y + i) * (colorRange * FIXED_MUL);
							dst[1] = (y + q) * (colorRange * FIXED_MUL);
							dst[2] = (i - y) * (colorRange * FIXED_MUL);
							dst[3] = (q - y) * (colorRange * FIXED_MUL);
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
