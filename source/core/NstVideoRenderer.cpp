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
#include <cstring>
#include <cfloat>
#include <cmath>
#include <new>
#include "NstCore.hpp"
#include "api/NstApiVideo.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterNone.hpp"
#include "NstVideoFilterScanlines.hpp"
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
			const u8 Renderer::Palette::defaultPalette[64][3] =
			{
				{0x68,0x68,0x68}, {0x00,0x12,0x99}, {0x1A,0x08,0xAA}, {0x51,0x02,0x9A}, 
				{0x7E,0x00,0x69}, {0x8E,0x00,0x1C}, {0x7E,0x03,0x01}, {0x51,0x18,0x00}, 
				{0x2F,0x22,0x02}, {0x01,0x4E,0x00}, {0x00,0x5A,0x00}, {0x00,0x50,0x1C}, 
				{0x00,0x40,0x61}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, 
				{0xB9,0xB9,0xB9}, {0x0C,0x5C,0xD7}, {0x50,0x35,0xF0}, {0x89,0x19,0xE0}, 
				{0xBB,0x0C,0xB3}, {0xCE,0x0C,0x61}, {0xC0,0x2B,0x0E}, {0x95,0x4D,0x01}, 
				{0x8B,0x74,0x00}, {0x1F,0x8B,0x00}, {0x01,0x98,0x0C}, {0x00,0x93,0x4B}, 
				{0x00,0x81,0x9B}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
				{0xFF,0xFF,0xFF}, {0x63,0xB4,0xFF}, {0x9B,0x91,0xFF}, {0xD3,0x77,0xFF}, 
				{0xEF,0x6A,0xFF}, {0xF9,0x68,0xC0}, {0xF9,0x7D,0x6C}, {0xED,0x9B,0x2D}, 
				{0xE7,0xC7,0x36}, {0x7C,0xDA,0x1C}, {0x4B,0xE8,0x47}, {0x35,0xE5,0x91}, 
				{0x3F,0xD9,0xDD}, {0x60,0x60,0x60}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, 
				{0xFF,0xFF,0xFF}, {0xAC,0xE7,0xFF}, {0xD5,0xCD,0xFF}, {0xED,0xBA,0xFF}, 
				{0xF8,0xB0,0xFF}, {0xFE,0xB0,0xEC}, {0xFD,0xC7,0xBF}, {0xF9,0xE1,0xA3}, 
				{0xE8,0xEB,0x7C}, {0xBB,0xF3,0x82}, {0x99,0xF7,0xA2}, {0x8A,0xF5,0xD0}, 
				{0x92,0xF4,0xF1}, {0xBE,0xBE,0xBE}, {0x00,0x00,0x00}, {0x00,0x00,0x00} 
			};

			const double Renderer::Palette::emphasis[8][3] =
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

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif

			Renderer::Palette::Palette()
			: type(PALETTE_INTERNAL), custom(NULL) {}
		
			Renderer::Palette::~Palette()
			{
				delete custom;
			}
		
			void Renderer::Palette::ToHSV(double r,double g,double b,double& h,double& s,double& v)
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
		
			void Renderer::Palette::ToRGB(double h,double s,double v,double& r,double& g,double& b)
			{
				if (s <= +FLT_EPSILON && s >= -FLT_EPSILON)
				{
					r = g = b = v;
				}
				else
				{
					h /= 60.0;
		
					const int i = std::floor( h );
		
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
		
			void Renderer::Palette::ToPAL(const double (&src)[3],u8 (&dst)[3])
			{
				for (uint i=0; i < 3; ++i)
					dst[i] = (src[i] >= 1.0 ? 255 : src[i] <= 0.0 ? 0 : src[i] * 255.0 + 0.5);
			}
	
			Result Renderer::Palette::LoadCustom(const u8 (*colors)[3])
			{
				if (!colors)
					return RESULT_ERR_INVALID_PARAM;
		
				if (!custom && NULL == (custom = new (std::nothrow) Custom))
					return RESULT_ERR_OUT_OF_MEMORY;
		
				std::memcpy( custom->palette, colors, 64*3 );

				return RESULT_OK;
			}

			bool Renderer::Palette::ResetCustom()
			{
				if (custom)
				{
					std::memcpy( custom->palette, defaultPalette, 64*3 );
					return true;
				}

				return false;
			}
		
			Result Renderer::Palette::SetType(PaletteType t)
			{
				if (t == type)
					return RESULT_NOP;
		
				if (t == PALETTE_CUSTOM && !custom)
				{
					if (NULL == (custom = new (std::nothrow) Custom))
						return RESULT_ERR_OUT_OF_MEMORY;
		
					ResetCustom();
				}
		
				type = t;
		
				return RESULT_OK;
			}
		
			void Renderer::Palette::ComputeCustom(const uint brightness,const uint saturation,uint hu)
			{
				NST_ASSERT( type == PALETTE_CUSTOM || type == PALETTE_INTERNAL );

				const double bri = (int(brightness) - 128) / 255.0;
				const double sat = (((int(saturation) - 128) / 255.0) * 2) + 1;
				const int hue = (int(hu) - 128) / 4;
				
				const u8 (*const from)[3] = (type == PALETTE_CUSTOM ? custom->palette : defaultPalette);
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
						h -= hue;
		
						if (h >= 360.0)
						{
							h -= 360.0;
						}
						else if (h < 0.0)
						{
							h += 360.0;
						}
		
						ToRGB( h, s, v, rgb[0], rgb[1], rgb[2] );
		
						rgb[0] *= emphasis[i][0];
						rgb[1] *= emphasis[i][1]; 
						rgb[2] *= emphasis[i][2]; 
		
						ToPAL( rgb, palette[(i * 64) + j] );
					}
				}
			}
		
			void Renderer::Palette::ComputeTV(const uint brightness,const uint saturation,uint hue)
			{
				NST_ASSERT( type == PALETTE_EMULATE );
		
				const double sat = saturation / 256.0;
				const double bri = (int(brightness) - 128) / 256.0;
				hue = HUE_OFFSET - ((int(hue) - 128) / 4);

				for (uint index=0; index < 8; ++index)
				{
					for (uint voltage=0; voltage < 4; ++voltage)
					{
						for (uint phase=0, angle=0; phase < 16; ++phase)
						{
							double s;
							double y = voltage * 0.375;

							switch (phase)
							{
								case 0: 
							
									s = 0.0; 
									y += 0.5;
									break;
							
								case 13: 
							
									angle = 0;
									s = 0.0; 
									y -= 0.5;
									break;
							
								case 14:
								case 15: 
							
									s = 0.0;
									y = 0.0;
									break;
							
								case 1:
							
									angle = hue;
							
								default: 
							
									angle += HUE_ROTATION;
							
									if (angle >= 360)
										angle -= 360;
							
									s = sat;
									break;
							}

							const double h = NST_DEG * angle;
							const double i = s * std::sin( h );
							const double q = s * std::cos( h );
		
							const double rgb[] = 
							{
								bri + ( emphasis[index][0] * (y + 0.956 * i + 0.621 * q) ),
								bri + ( emphasis[index][1] * (y - 0.272 * i - 0.647 * q) ), 
								bri + ( emphasis[index][2] * (y - 1.105 * i + 1.702 * q) ) 
							};
		
							ToPAL( rgb, palette[(index * 64) + (voltage * 16) + phase] );
						}
					}
				}
			}

			void Renderer::Palette::Update(uint brightness,uint saturation,uint hue)
			{
				(*this.*(type == PALETTE_EMULATE ? &Palette::ComputeTV : &Palette::ComputeCustom))( brightness, saturation, hue );
			}

			inline const Renderer::PaletteEntries& Renderer::Palette::Get() const
			{
				return palette;
			}

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
			: bpp(state.bits.count), format(state.bits.mask) {}

			void Renderer::Filter::Transform(const u8 (&src)[PALETTE][3],u32 (&dst)[PALETTE]) const
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
			: 
			update     (UPDATE_PALETTE), 
			brightness (DEFAULT_BRIGHTNESS), 
			saturation (DEFAULT_SATURATION), 
			hue        (DEFAULT_HUE) 
			{}

            Renderer::Renderer()
			: filter(NULL) {}

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
							filter = new (std::nothrow) FilterNone( renderState );

						break;

					case RenderState::FILTER_SCANLINES_BRIGHT:
					case RenderState::FILTER_SCANLINES_DARK:

						if (FilterScanlines::Check( renderState ))
							filter = new (std::nothrow) FilterScanlines( renderState );

						break;

                #ifndef NST_NO_2XSAI

					case RenderState::FILTER_2XSAI:
					case RenderState::FILTER_SUPER_2XSAI:
					case RenderState::FILTER_SUPER_EAGLE:

						if (Filter2xSaI::Check( renderState ))
							filter = new (std::nothrow) Filter2xSaI( renderState );

						break;

                #endif
                #ifndef NST_NO_SCALE2X

					case RenderState::FILTER_SCALE2X:
					case RenderState::FILTER_SCALE3X:

						if (FilterScaleX::Check( renderState ))
							filter = new (std::nothrow) FilterScaleX( renderState );

						break;
                #endif
                #ifndef NST_NO_HQ2X

					case RenderState::FILTER_HQ2X:
					case RenderState::FILTER_HQ3X:

						if (FilterHqX::Check( renderState ))
							filter = new (std::nothrow) FilterHqX( renderState );

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

						if (FilterNtsc<32>::Check( renderState ))
						{
							filter = new (std::nothrow) FilterNtsc<32>( renderState, scanlines, state.brightness, state.saturation, state.hue );
						}
						else if (FilterNtsc<16>::Check( renderState ))
						{
							filter = new (std::nothrow) FilterNtsc<16>( renderState, scanlines, state.brightness, state.saturation, state.hue );
						}
						else if (FilterNtsc<15>::Check( renderState ))
						{
							filter = new (std::nothrow) FilterNtsc<15>( renderState, scanlines, state.brightness, state.saturation, state.hue );
						}
						break;
					}

                #endif
				}
				
				if (filter)
				{
					state.filter = renderState.filter;
					state.width = renderState.width;
					state.height = renderState.height;
					state.mask = renderState.bits.mask;

					if (filter->CanTransform())
						state.update |= State::UPDATE_FILTER;
					else
						state.update &= ~u8(State::UPDATE_FILTER);
  
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

			Result Renderer::SetLevel(u8& type,u8 value)
			{
				if (type == value)				
					return RESULT_NOP;
				
				type = value;
				state.update |= State::UPDATE_PALETTE|State::UPDATE_FILTER;

				return RESULT_OK;
			}

			Result Renderer::SetPaletteType(PaletteType type)
			{
				const Result result = palette.SetType( type );

				if (result == RESULT_OK)
				{
					state.update |= State::UPDATE_PALETTE;

					if (filter && filter->CanTransform())
						state.update |= State::UPDATE_FILTER;
				}

				return result;
			}

			Result Renderer::LoadCustomPalette(const u8 (*colors)[3])
			{
				const Result result = palette.LoadCustom( colors );
				
				if (result == RESULT_OK && palette.GetType() == PALETTE_CUSTOM)
				{
					state.update |= State::UPDATE_PALETTE;

					if (filter && filter->CanTransform())
						state.update |= State::UPDATE_FILTER;
				}

				return result;
			}

			void Renderer::ResetCustomPalette()
			{
				if (palette.ResetCustom() && palette.GetType() == PALETTE_CUSTOM)
				{
					state.update |= State::UPDATE_PALETTE;

					if (filter && filter->CanTransform())
						state.update |= State::UPDATE_FILTER;
				}
			}

			const Renderer::PaletteEntries& Renderer::GetPalette()
			{
				if (state.update & State::UPDATE_PALETTE)
				{
					state.update &= ~u8(State::UPDATE_PALETTE);
					palette.Update( state.brightness, state.saturation, state.hue );
				}

				return palette.Get();
			}

			void Renderer::UpdateFilter()
			{
				if (filter->CanTransform())
				{
					state.update &= ~u8(State::UPDATE_FILTER);

					const PaletteEntries& entries = GetPalette();
					filter->Transform( entries, input.palette );
					Api::Video::Palette::updateCallback( entries );
				}
				else
				{
					RenderState renderState;
					GetState( renderState );

					delete filter;
					filter = NULL;

					SetState( renderState );
				}
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			void Renderer::Blit(Output& output)
			{
				if (filter)
				{
					if (state.update & State::UPDATE_FILTER)
						UpdateFilter();

					if (Output::lockCallback( output ))
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
}
