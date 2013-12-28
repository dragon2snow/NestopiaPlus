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
			const u8 Renderer::Palette::rgbPalette[64][3] =
			{
				{0x78,0x78,0x78}, {0x00,0x38,0x98}, {0x00,0x00,0xD8}, {0x78,0x58,0xD8},
				{0x98,0x00,0x78}, {0xB8,0x00,0x78}, {0xB8,0x38,0x00}, {0x98,0x58,0x00},
				{0x78,0x58,0x00}, {0x38,0x58,0x00}, {0x00,0x78,0x38}, {0x00,0x98,0x00},
				{0x00,0x58,0x58}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
				{0xB8,0xB8,0xB8}, {0x00,0x78,0xD8}, {0x00,0x58,0xFF}, {0x98,0x00,0xFF},
				{0xB8,0x00,0xFF}, {0xFF,0x00,0x98}, {0xFF,0x00,0x00}, {0xD8,0x78,0x00},
				{0x98,0x78,0x00}, {0x38,0x98,0x00}, {0x00,0x98,0x00}, {0x00,0xB8,0x78},
				{0x00,0x98,0x98}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
				{0xFF,0xFF,0xFF}, {0x78,0xB8,0xFF}, {0x98,0x98,0xFF}, {0xD8,0x78,0xFF},
				{0xFF,0x00,0xFF}, {0xFF,0x78,0xFF}, {0xFF,0x98,0x00}, {0xFF,0xB8,0x00},
				{0xD8,0xD8,0x00}, {0x78,0xD8,0x00}, {0x00,0xFF,0x00}, {0x58,0xFF,0xD8},
				{0x00,0xFF,0xFF}, {0x4F,0x4F,0x4F}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, 
				{0xFF,0xFF,0xFF}, {0xB8,0xD8,0xFF}, {0xD8,0xB8,0xFF}, {0xFF,0xB8,0xFF},
				{0xFF,0x98,0xFF}, {0xFF,0xB8,0xB8}, {0xFF,0xD8,0x98}, {0xFF,0xFF,0x58},
				{0xFF,0xFF,0x78}, {0xB8,0xFF,0x58}, {0x98,0xFF,0x78}, {0x58,0xFF,0xD8},
				{0x98,0xD8,0xFF}, {0xA4,0xA4,0xA4}, {0x00,0x00,0x00}, {0x00,0x00,0x00}
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
			: type(PALETTE_YUV), custom(NULL) 
			{
				SetDecoder( Api::Video::DECODER_CANONICAL );
			}
		
			Renderer::Palette::~Palette()
			{
				delete custom;
			}
		
			Result Renderer::Palette::SetDecoder(const Decoder& d)
			{
				if (decoder == d)
					return RESULT_NOP;

				for (uint i=0; i < 3; ++i)
				{
					if (d.axes[i].angle >= 360 || d.axes[i].gain > 2.0)
						return RESULT_ERR_INVALID_PARAM;
				}

				decoder = d;
				return RESULT_OK;
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
					std::memcpy( custom->palette, rgbPalette, 64*3 );
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
		
			void Renderer::Palette::Build(const int b,const int s,int hue)
			{
				NST_ASSERT( type == PALETTE_CUSTOM || type == PALETTE_RGB );

				const double brightness = (b - 128) / 256.0;
				const double saturation = s / 128.0;
				hue = (hue - 128) / 4;
				
				const u8 (*const from)[3] = (type == PALETTE_CUSTOM ? custom->palette : rgbPalette);
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
		
						s *= saturation;
						v += brightness;
						h += hue;
		
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
		
			void Renderer::Palette::Generate(const int b,const int s,int hue)
			{
				NST_ASSERT( type == PALETTE_YUV );
		
				const double saturation = s / 128.0;
				const double brightness = (b - 128) / 256.0;
				hue = HUE_OFFSET + (hue - 128) / 4;
					
				for (uint index=0; index < 8; ++index)
				{
					for (uint level=0; level < 4; ++level)
					{
						for (uint phase=0; phase < 16; ++phase)
						{
							double y=0.0, u=0.0, v=0.0;

							if (phase-1U < 12)
							{
								static const double chroma[4] = 
								{ 
									0.26, 0.33, 0.34, 0.14
								};

								const double angle = ((int(phase) - 3) * (360/12) + hue) * NST_DEG;

								v = std::sin( angle ) * chroma[level]; 
								u = std::cos( angle ) * chroma[level]; 

								if (decoder.boostYellow)
								{								 
									const double yellowness = v - u; 

									if (yellowness > 0.0) 
									{ 
										v = v + yellowness * (level / 4.0); 
										u = u - yellowness * (level / 4.0); 
									} 
								}

								v *= saturation; 
								u *= saturation; 
							}

							if (phase < 14)
							{
								static const double luma[3][4] = 
								{
									{ 0.39, 0.67, 1.00, 1.00 },
									{ 0.14, 0.34, 0.66, 0.86 },
									{-0.12, 0.00, 0.31, 0.72 }
								};

								y = luma[phase == 0 ? 0 : phase < 13 ? 1 : 2][level];
							}

							const double rgb[3] =
							{
								(y + std::sin( decoder.axes[0].angle * NST_DEG ) * decoder.axes[0].gain * 2 * v + std::cos( decoder.axes[0].angle * NST_DEG ) * decoder.axes[0].gain * 2 * u) * emphasis[index][0] + brightness, 
								(y + std::sin( decoder.axes[1].angle * NST_DEG ) * decoder.axes[1].gain * 2 * v + std::cos( decoder.axes[1].angle * NST_DEG ) * decoder.axes[1].gain * 2 * u) * emphasis[index][1] + brightness, 
								(y + std::sin( decoder.axes[2].angle * NST_DEG ) * decoder.axes[2].gain * 2 * v + std::cos( decoder.axes[2].angle * NST_DEG ) * decoder.axes[2].gain * 2 * u) * emphasis[index][2] + brightness
							};

							ToPAL( rgb, palette[(index * 64) + (level * 16) + phase] );
						}
					}
				}
			}

			void Renderer::Palette::Update(uint brightness,uint saturation,uint hue)
			{
				(*this.*(type == PALETTE_YUV ? &Palette::Generate : &Palette::Build))( brightness, saturation, hue );
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
							filter = new (std::nothrow) FilterNtsc<32>( renderState, scanlines, state.brightness, state.saturation, state.hue, palette.GetDecoder() );
						}
						else if (FilterNtsc<16>::Check( renderState ))
						{
							filter = new (std::nothrow) FilterNtsc<16>( renderState, scanlines, state.brightness, state.saturation, state.hue, palette.GetDecoder() );
						}
						else if (FilterNtsc<15>::Check( renderState ))
						{
							filter = new (std::nothrow) FilterNtsc<15>( renderState, scanlines, state.brightness, state.saturation, state.hue, palette.GetDecoder() );
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

			Result Renderer::SetDecoder(const Decoder& decoder)
			{
				const Result result = palette.SetDecoder( decoder );

				if (result == RESULT_OK && palette.GetType() == PALETTE_YUV)
					state.update |= State::UPDATE_PALETTE|State::UPDATE_FILTER;

				return result;
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
