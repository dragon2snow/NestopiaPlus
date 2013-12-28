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

#include <cstdlib>
#include "NstCore.hpp"
#include "api/NstApiVideo.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterNone.hpp"
#include "NstVideoFilterScanlines.hpp"
#include "NstVideoFilterTV.hpp"
#ifndef NST_NO_2XSAI
#include "NstVideoFilter2xSaI.hpp"
#endif
#ifndef NST_NO_SCALE2X
#include "NstVideoFilterScaleX.hpp"
#endif
#ifndef NST_NO_HQ2X
#include "NstVideoFilterHqX.hpp"
#endif
#ifndef NST_NO_NTSCVIDEO
#include "NstVideoFilterNtsc.hpp"
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif

			Renderer::Filter::Format::Format(const RenderState::Bits::Mask& m)
			{
				const dword mask[3] = {m.r,m.g,m.b};

				for (uint i=0; i < 3; ++i)
				{
					left[i] = 0; 

					if (mask[i])
					{
						while (!(mask[i] & (0x1UL << left[i])))
							++left[i];
					}

					for 
					(
						right[i] = 0; 
     					right[i] < 8 && (mask[i] & (0x1UL << (left[i] + right[i]))); 
     					right[i] += 1
					);

					right[i] = 8 - right[i];
				}
			}

			Renderer::Filter::Filter(const RenderState& state)
			: bpp(state.bits.count), format(state.bits.mask)
			{
			}

			void Renderer::Filter::Transform(const u8 (*NST_RESTRICT src)[3],u32 (&dst)[PALETTE]) const
			{
				NST_ASSERT( src );

				if (bpp >= 16)
				{
					for (uint i=0; i < PALETTE; ++i)
					{
						dst[i] = 
						(
							((src[i][0] >> format.right[0]) << format.left[0]) |
							((src[i][1] >> format.right[1]) << format.left[1]) |
							((src[i][2] >> format.right[2]) << format.left[2])
						);
					}
				}
			}

			Renderer::State::State()
			: brightness(128), saturation(128), hue(128) {}

            Renderer::Renderer()
			: filter(NULL), palette(NULL)
			{
			}

			Renderer::~Renderer()
			{
				delete filter;
			}

			Result Renderer::SetState(const RenderState& renderState)
			{
				if (filter)
				{
					if 
					(
				    	state.filter == renderState.filter &&
						state.width == renderState.width &&
						state.height == renderState.height &&
						filter->bpp == renderState.bits.count &&						
						state.mask.r == renderState.bits.mask.r &&
						state.mask.g == renderState.bits.mask.g &&
						state.mask.b == renderState.bits.mask.b &&
						(filter->bpp != 8 || static_cast<const FilterNone*>(filter)->paletteOffset == renderState.paletteOffset)
					)
						return RESULT_NOP;

					delete filter;
					filter = NULL;
				}

				switch (renderState.filter)
				{
     				case RenderState::FILTER_NONE:

						if (FilterNone::Check( renderState ))
							filter = new FilterNone( renderState );

						break;

					case RenderState::FILTER_SCANLINES_BRIGHT:
					case RenderState::FILTER_SCANLINES_DARK:

						if (FilterScanlines::Check( renderState ))
							filter = new FilterScanlines( renderState );

						break;

                #ifndef NST_NO_2XSAI

					case RenderState::FILTER_2XSAI:
					case RenderState::FILTER_SUPER_2XSAI:
					case RenderState::FILTER_SUPER_EAGLE:

						if (Filter2xSaI::Check( renderState ))
							filter = new Filter2xSaI( renderState );

						break;

                #endif
                #ifndef NST_NO_SCALE2X

					case RenderState::FILTER_SCALE2X:
					case RenderState::FILTER_SCALE3X:

						if (FilterScaleX::Check( renderState ))
							filter = new FilterScaleX( renderState );

						break;
                #endif
                #ifndef NST_NO_HQ2X

					case RenderState::FILTER_HQ2X:
					case RenderState::FILTER_HQ3X:

						if (FilterHqX::Check( renderState ))
							filter = new FilterHqX( renderState );

						break;

                #endif
                #ifndef NST_NO_NTSCVIDEO

					case RenderState::FILTER_NTSC:
					case RenderState::FILTER_NTSC_SCANLINES_BRIGHT:
					case RenderState::FILTER_NTSC_SCANLINES_DARK:
					{
						const uint scanlines = 
						(
					    	renderState.filter == RenderState::FILTER_NTSC_SCANLINES_BRIGHT ? 1 : 
							renderState.filter == RenderState::FILTER_NTSC_SCANLINES_DARK ? 2 : 0
						);

						const double colors[3] =
						{
							(int( state.hue        ) - 128) / 256.0,
							(int( state.brightness ) - 128) / 256.0,
							(int( state.saturation ) - 128) / 256.0
						};

						if (FilterNtsc<32>::Check( renderState ))
						{
							filter = new FilterNtsc<32>( renderState, scanlines, colors[0], colors[1], colors[2] );
						}
						else if (FilterNtsc<16>::Check( renderState ))
						{
							filter = new FilterNtsc<16>( renderState, scanlines, colors[0], colors[1], colors[2] );
						}
						else if (FilterNtsc<15>::Check( renderState ))
						{
							filter = new FilterNtsc<15>( renderState, scanlines, colors[0], colors[1], colors[2] );
						}
						break;
					}

                #endif

					case RenderState::FILTER_TV:

						if (FilterTV::Check( renderState ))
							filter = new FilterTV( renderState );

						break;
				}
				
				if (filter)
				{
					state.filter = renderState.filter;
					state.width = renderState.width;
					state.height = renderState.height;
					state.mask = renderState.bits.mask;

					filter->Transform( palette, input.palette );

					return RESULT_OK;
				}
				else
				{
					return RESULT_ERR_UNSUPPORTED;
				}
			}
				
			Result Renderer::GetState(RenderState& output) const
			{
				if (filter)
				{
					output.filter = state.filter;
					output.width = state.width;
					output.height = state.height;
					output.bits.count = filter->bpp;

					if (output.bits.count == 8)
					{
						output.bits.mask.b = output.bits.mask.g = output.bits.mask.r = 0;
						output.paletteOffset = static_cast<const FilterNone*>(filter)->paletteOffset;
					}
					else
					{
						output.bits.mask = state.mask;
						output.paletteOffset = 0;
					}
		
					return RESULT_OK;
				}
		
				return RESULT_ERR_NOT_READY;
			}

			void Renderer::UpdateColors()
			{
            #ifndef NST_NO_NTSCVIDEO
				if 
				(
			     	filter && 
					(
				     	state.filter == RenderState::FILTER_NTSC || 
						state.filter == RenderState::FILTER_NTSC_SCANLINES_BRIGHT ||
						state.filter == RenderState::FILTER_NTSC_SCANLINES_DARK
					)
				)
				{
					RenderState renderState;
					GetState( renderState );

					delete filter;
					filter = NULL;

					SetState( renderState );
				}
            #endif
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			void Renderer::Blit(Output& output) const
			{
				if (filter && Output::lockCallback( output ))
				{
					NST_ASSERT( output.pixels && output.pitch );

					if (ulong(std::labs( output.pitch )) >= filter->bpp * (WIDTH / 8U))
						filter->Blit( input, output );

					Output::unlockCallback( output );
				}
			}	
		}
	}
}
