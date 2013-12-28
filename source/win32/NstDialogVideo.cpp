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

#include <cstring>
#include <algorithm>
#include "NstIoFile.hpp"
#include "NstIoLog.hpp"
#include "NstWindowUser.hpp"
#include "NstResourceString.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstDialogPaletteEditor.hpp"
#include "NstDialogVideoDecoder.hpp"
#include "NstDialogVideo.hpp"

#ifdef __INTEL_COMPILER
#pragma warning( disable : 279 )
#endif

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
       	IDC_GRAPHICS_NTSC_TOP    == IDC_GRAPHICS_NTSC_LEFT + 1 && 
		IDC_GRAPHICS_NTSC_RIGHT  == IDC_GRAPHICS_NTSC_LEFT + 2 &&
		IDC_GRAPHICS_NTSC_BOTTOM == IDC_GRAPHICS_NTSC_LEFT + 3 &&
		IDC_GRAPHICS_PAL_LEFT    == IDC_GRAPHICS_NTSC_LEFT + 4 &&  
		IDC_GRAPHICS_PAL_TOP     == IDC_GRAPHICS_NTSC_LEFT + 5 &&    
		IDC_GRAPHICS_PAL_RIGHT   == IDC_GRAPHICS_NTSC_LEFT + 6 &&  
		IDC_GRAPHICS_PAL_BOTTOM  == IDC_GRAPHICS_NTSC_LEFT + 7
	);

	NST_COMPILE_ASSERT
	(
		IDC_GRAPHICS_16_BIT == IDC_GRAPHICS_8_BIT + 1 &&
		IDC_GRAPHICS_32_BIT == IDC_GRAPHICS_8_BIT + 2
	);

	NST_COMPILE_ASSERT
	(
       	IDC_GRAPHICS_COLORS_SATURATION == IDC_GRAPHICS_COLORS_BRIGHTNESS + 1 &&
     	IDC_GRAPHICS_COLORS_HUE        == IDC_GRAPHICS_COLORS_BRIGHTNESS + 2
	);

	NST_COMPILE_ASSERT
	(
		IDC_GRAPHICS_NESTEXTURE_VIDMEM == IDC_GRAPHICS_NESTEXTURE_AUTO + 1 &&
		IDC_GRAPHICS_NESTEXTURE_SYSMEM == IDC_GRAPHICS_NESTEXTURE_AUTO + 2
	);

	NST_COMPILE_ASSERT
	(
		IDC_GRAPHICS_PALETTE_YUV      == IDC_GRAPHICS_PALETTE_AUTO + 1 &&
		IDC_GRAPHICS_PALETTE_RGB	  == IDC_GRAPHICS_PALETTE_AUTO + 2 &&
		IDC_GRAPHICS_PALETTE_CUSTOM	  == IDC_GRAPHICS_PALETTE_AUTO + 3 &&
		IDC_GRAPHICS_PALETTE_PATH	  == IDC_GRAPHICS_PALETTE_AUTO + 4 &&
		IDC_GRAPHICS_PALETTE_BROWSE	  == IDC_GRAPHICS_PALETTE_AUTO + 5 &&
		IDC_GRAPHICS_PALETTE_CLEAR	  == IDC_GRAPHICS_PALETTE_AUTO + 6 &&
		IDC_GRAPHICS_PALETTE_EDITOR	  == IDC_GRAPHICS_PALETTE_AUTO + 7
	);

	using namespace Window;

	struct Video::Handlers
	{
		static const MsgHandler::Entry<Video> messages[];
		static const MsgHandler::Entry<Video> commands[];
	};

	const MsgHandler::Entry<Video> Video::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Video::OnInitDialog },
		{ WM_HSCROLL,    &Video::OnHScroll    }
	};

	const MsgHandler::Entry<Video> Video::Handlers::commands[] =
	{
		{ IDC_GRAPHICS_DEVICE,            &Video::OnCmdDevice         },
		{ IDC_GRAPHICS_MODE,              &Video::OnCmdMode           },
		{ IDC_GRAPHICS_8_BIT,             &Video::OnCmdBitDepth       },
		{ IDC_GRAPHICS_16_BIT,            &Video::OnCmdBitDepth       },
		{ IDC_GRAPHICS_32_BIT,            &Video::OnCmdBitDepth       },
		{ IDC_GRAPHICS_EFFECTS,           &Video::OnCmdFilter         },
		{ IDC_GRAPHICS_FILTER_SETTINGS,   &Video::OnCmdFilterSettings },
		{ IDC_GRAPHICS_NESTEXTURE_AUTO,   &Video::OnCmdTexMem         },
		{ IDC_GRAPHICS_NESTEXTURE_VIDMEM, &Video::OnCmdTexMem         },
		{ IDC_GRAPHICS_NESTEXTURE_SYSMEM, &Video::OnCmdTexMem         },
		{ IDC_GRAPHICS_PALETTE_AUTO,      &Video::OnCmdPalType        },
		{ IDC_GRAPHICS_PALETTE_YUV,       &Video::OnCmdPalType        },
		{ IDC_GRAPHICS_PALETTE_RGB,       &Video::OnCmdPalType        },
		{ IDC_GRAPHICS_PALETTE_CUSTOM,    &Video::OnCmdPalType        },
		{ IDC_GRAPHICS_COLORS_ADVANCED,   &Video::OnCmdColorsAdvanced },
		{ IDC_GRAPHICS_COLORS_RESET,      &Video::OnCmdColorsReset    },
		{ IDC_GRAPHICS_PALETTE_BROWSE,    &Video::OnCmdPalBrowse      },
		{ IDC_GRAPHICS_PALETTE_CLEAR,     &Video::OnCmdPalClear       },
		{ IDC_GRAPHICS_PALETTE_EDITOR,    &Video::OnCmdPalEditor      },
		{ IDC_GRAPHICS_AUTO_HZ,			  &Video::OnCmdAutoHz         },
		{ IDC_GRAPHICS_DEFAULT,           &Video::OnCmdDefault        },
		{ IDC_GRAPHICS_OK,                &Video::OnCmdOk             }
	};

	Video::Settings::Settings()
	: fullscreenScale(SCREEN_MATCHED) {}

	Video::Filter::Filter()
	: bilinear(false)
	{
		std::memset( attributes, 0, sizeof(attributes) );
	}

	Video::Video
	(
		Managers::Emulator& emulator,
		const Adapters& a,
		const Managers::Paths& p,
		const Configuration& cfg
	)
	: 
	adapters ( a ),
	nes      ( emulator ),
	dialog   ( IDD_VIDEO, this,Handlers::messages, Handlers::commands ),
	paths    ( p )
	{
		settings.adapter = std::find
		(
			adapters.begin(),
			adapters.end(),
			System::Guid( cfg["video device"] )
		);

		if (settings.adapter == adapters.end())
			settings.adapter = adapters.begin();

		settings.mode = settings.adapter->modes.find
   		( 
   			Mode
   			(
     			cfg[ "video fullscreen width"  ],
       			cfg[ "video fullscreen height" ],
       			cfg[ "video fullscreen bpp"    ]
     		)
		);

		if (settings.mode == settings.adapter->modes.end())
			settings.mode = GetDefaultMode();

		{
			const GenericString type( cfg["view size fullscreen"] );

			if (type.Length() > 1)
			{
				if (type == _T("stretched"))
					settings.fullscreenScale = SCREEN_STRETCHED;
			}
			else if (type.Length() && type[0] >= '1' && type[0] <= '9')
			{
				settings.fullscreenScale = type[0] - '1';
			}
		}

		settings.filter = settings.filters + FILTER_NONE;

		if (settings.adapter->maxScreenSize >= NST_MAX(NES_HEIGHT*2,NES_WIDTH*2))
		{
			const GenericString type( cfg["video filter"] );

			if (type == _T("scanlines"))
			{
				settings.filter = settings.filters + FILTER_SCANLINES;
			}
			else if (type == _T("ntsc"))
			{
				if (settings.adapter->maxScreenSize >= NST_MAX(NTSC_HEIGHT,NTSC_WIDTH))
					settings.filter = settings.filters + FILTER_NTSC;
			}
			else if (type == _T("2xsai"))
			{
				settings.filter = settings.filters + FILTER_2XSAI;
			}
			else if (type == _T("scalex"))
			{
				settings.filter = settings.filters + FILTER_SCALEX;
			}
			else if (type == _T("hqx"))
			{
				settings.filter = settings.filters + FILTER_HQX;
			}
		}

		for (uint i=0; i < NUM_FILTERS; ++i)
		{
			static cstring const lut[] =
			{
				"video filter none bilinear",
				"video filter scanlines bilinear",
				"video filter ntsc bilinear",
				"video filter 2xsai bilinear",
				"video filter scalex bilinear",
				"video filter hqx bilinear"
			};

			if (cfg[lut[i]] == Configuration::YES)
				settings.filters[i].bilinear = true;
		}

		{
			GenericString value;

			settings.filters[FILTER_SCANLINES].attributes[0] = (uchar)
			(
	         	(value=cfg["video filter scanlines type"]) == _T("dark") ? Filter::SCANLINES_DARK : 
				                                                           Filter::SCANLINES_BRIGHT
			);
			
			settings.filters[FILTER_NTSC].attributes[0] = (uchar) 
			(
	           	(value=cfg["video filter ntsc scanlines"]) == _T("dark")   ? Filter::SCANLINES_DARK : 
				(value)                                    == _T("bright") ? Filter::SCANLINES_BRIGHT : 
				                                                             Filter::SCANLINES_NONE
			);
		
			settings.filters[FILTER_NTSC].attributes[1] = (uchar) 
			(
	         	cfg["video filter ntsc widescreen"] == Configuration::YES
			);
		
			settings.filters[FILTER_2XSAI].attributes[0] = (uchar) 
			(
		     	(value=cfg["video filter 2xsai type"]) == _T("super 2xsai") ? Filter::TYPE_SUPER2XSAI : 
				(value)                                == _T("super eagle") ? Filter::TYPE_SUPEREAGLE :
				                                                              Filter::TYPE_2XSAI
			);
			
			settings.filters[FILTER_SCALEX].attributes[0] = (uchar) 
			(
       			(value=cfg["video filter scalex scale"]) == _T("3") ? Filter::TYPE_SCALE3X : Filter::TYPE_SCALE2X
			);

			settings.filters[FILTER_HQX].attributes[0] = (uchar) 
			(
     			(value=cfg["video filter hqx scale"]) == _T("3") ? Filter::TYPE_HQ3X : Filter::TYPE_HQ2X
			);
		}

		{
			const GenericString type( cfg["video texture location"] );

			if (type == _T("vidmem"))
			{
				settings.texMem = Settings::TEXMEM_VIDMEM;
			}
			else if (type == _T("sysmem"))
			{
				settings.texMem = Settings::TEXMEM_SYSMEM;
			}
			else
			{
				settings.texMem = Settings::TEXMEM_AUTO;
			}
		}
		
		{
			Rect& ntsc = settings.rects.ntsc;
			Rect& pal = settings.rects.pal;

			ntsc.left   = cfg[ "video ntsc left"   ].Default( 0                  );
			ntsc.top    = cfg[ "video ntsc top"    ].Default( NTSC_CLIP_TOP      );
			ntsc.right  = cfg[ "video ntsc right"  ].Default( NES_WIDTH-1        );
			ntsc.bottom = cfg[ "video ntsc bottom" ].Default( NTSC_CLIP_BOTTOM-1 );
			pal.left    = cfg[ "video pal left"    ].Default( 0                  );
			pal.top     = cfg[ "video pal top"     ].Default( PAL_CLIP_TOP       );
			pal.right   = cfg[ "video pal right"   ].Default( NES_WIDTH-1        );
			pal.bottom  = cfg[ "video pal bottom"  ].Default( PAL_CLIP_BOTTOM-1  );
		}

		ValidateRects();

		{
			uint color;

			if ((color = cfg[ "video color brightness" ].Default( 128U )) <= 255) Nes::Video(nes).SetBrightness( color );
			if ((color = cfg[ "video color saturation" ].Default( 128U )) <= 255) Nes::Video(nes).SetSaturation( color );
			if ((color = cfg[ "video color hue"        ].Default( 128U )) <= 255) Nes::Video(nes).SetHue( color );
		}

		{
			Nes::Video::Decoder decoder( Nes::Video::DECODER_CANONICAL );
			GenericString string;

			for (uint i=0; i < 3; ++i)
			{
				static cstring const axes[3][2] =
				{
					{ "video decoder ry angle", "video decoder ry gain" },
					{ "video decoder rg angle", "video decoder rg gain" },
					{ "video decoder rb angle", "video decoder rb gain" }
				};

				string = cfg[axes[i][0]];

				if (string.Length())
					string >> decoder.axes[i].angle;

				string = cfg[axes[i][1]];

				if (string.Length())
				{
					decoder.axes[i].gain = (float) _tstof( string.Ptr() );
					decoder.axes[i].gain = NST_CLAMP(decoder.axes[i].gain,0.0f,2.0f);
				}
			}

			decoder.boostYellow = (cfg["video decoder yellow boost"] == Configuration::YES);

			Nes::Video(nes).SetDecoder( decoder );
		}

		settings.palette = cfg["video palette file"];
		ImportPalette( settings.palette, Managers::Paths::QUIETLY );

		{
			settings.autoPalette = FALSE;
			const GenericString type( cfg["video palette"] );

			Nes::Video::Palette::Mode mode;

			if (type == _T("yuv"))
			{
				mode = Nes::Video::Palette::MODE_YUV;
			}
			else if (type == _T("rgb"))
			{
				mode = Nes::Video::Palette::MODE_RGB;
			}
			else if (type == _T("custom") && settings.palette.Length())
			{
				mode = Nes::Video::Palette::MODE_CUSTOM;
			}
			else
			{
				settings.autoPalette = TRUE;
				mode = Nes::Video::Palette::MODE_YUV;
			}

			Nes::Video(nes).GetPalette().SetMode( mode );
		}

		settings.autoHz = (cfg["video auto display frequency"] == Configuration::YES);
	}

	Video::~Video()
	{
	}

	void Video::Save(Configuration& cfg) const
	{
		if (settings.adapter != adapters.end())
		{
			cfg[ "video device" ].Quote() = settings.adapter->guid.GetString();

			if (settings.mode != settings.adapter->modes.end())
			{
				cfg[ "video fullscreen width"  ] = settings.mode->width;
				cfg[ "video fullscreen height" ] = settings.mode->height;
				cfg[ "video fullscreen bpp"    ] = settings.mode->bpp;
			}
		}

		{
			HeapString& value = cfg["view size fullscreen"].GetString();

			if (settings.fullscreenScale <= 8)
				value << (settings.fullscreenScale + 1U);
			else
				value << "stretched";
		}

		{
			static tstring const names[] =
			{
				_T( "none"      ),            
				_T( "scanlines" ),
				_T( "ntsc"      ),
				_T( "2xsai"     ),           
				_T( "scalex"    ),         
				_T( "hqx"	    )
			};

			cfg[ "video filter" ] = names[settings.filter - settings.filters];
		}

		for (uint i=0; i < NUM_FILTERS; ++i)
		{
			static cstring const lut[] =
			{
				"video filter none bilinear",
				"video filter scanlines bilinear",
				"video filter ntsc bilinear",
				"video filter 2xsai bilinear",
				"video filter scalex bilinear",
				"video filter hqx bilinear"
			};

			cfg[lut[i]].YesNo() = settings.filters[i].bilinear;
		}

		cfg["video filter scanlines type"] = 
		(
	     	settings.filters[FILTER_SCANLINES].attributes[0] == Filter::SCANLINES_DARK ? _T("dark") : 
			                                                                             _T("bright")
		);

		cfg["video filter ntsc scanlines"] =
		(
     		settings.filters[FILTER_NTSC].attributes[0] == Filter::SCANLINES_DARK   ? _T("dark") :
			settings.filters[FILTER_NTSC].attributes[0] == Filter::SCANLINES_BRIGHT ? _T("bright") :
			                                                                          _T("none")
		);
		
		cfg["video filter ntsc widescreen"].YesNo() =
		(
       		settings.filters[FILTER_NTSC].attributes[1]
		);

		cfg["video filter 2xsai type"] =
		(
     		settings.filters[FILTER_2XSAI].attributes[0] == Filter::TYPE_SUPER2XSAI ? _T("super 2xsai")	:
			settings.filters[FILTER_2XSAI].attributes[0] == Filter::TYPE_SUPEREAGLE ? _T("super eagle") :
																					  _T("2xsai")
		);

		cfg["video filter scalex scale"] =
		(
     		settings.filters[FILTER_SCALEX].attributes[0] == Filter::TYPE_SCALE3X ? _T("3") : _T("2") 
		);

		cfg["video filter hqx scale"] =
		(
     		settings.filters[FILTER_HQX].attributes[0] == Filter::TYPE_HQ3X ? _T("3") : _T("2")
		);

		{
			static tstring const names[] =
			{
				_T("auto"), _T("vidmem"), _T("sysmem")
			};

			cfg[ "video texture location" ] = names[settings.texMem];
		}

		{
			GenericString name;

			if (settings.autoPalette)
			{
				name = _T("auto");
			}
			else switch (Nes::Video(nes).GetPalette().GetMode())
			{
				case Nes::Video::Palette::MODE_YUV: name = _T("yuv");    break;
				case Nes::Video::Palette::MODE_RGB: name = _T("rgb");    break;
				default:                            name = _T("custom"); break;
			}

			cfg[ "video palette" ] = name;
		}

		{
			const Nes::Video::Decoder& decoder = Nes::Video(nes).GetDecoder();

			for (uint i=0; i < 3; ++i)
			{
				static cstring const strings[3][2] =
				{
					{ "video decoder ry angle", "video decoder ry gain" },
					{ "video decoder rg angle", "video decoder rg gain" },
					{ "video decoder rb angle", "video decoder rb gain" }
				};

				cfg[strings[i][0]] = decoder.axes[i].angle;

				tchar buffer[32];
				::_stprintf( buffer, _T("%.3f"), decoder.axes[i].gain );
				cfg[strings[i][1]] = buffer;
			}

			cfg["video decoder yellow boost"].YesNo() = decoder.boostYellow;
		}

		cfg[ "video palette file"           ].Quote() = settings.palette;
		cfg[ "video ntsc left"              ] = settings.rects.ntsc.left;
		cfg[ "video ntsc top"               ] = settings.rects.ntsc.top;
		cfg[ "video ntsc right"             ] = settings.rects.ntsc.right - 1;
		cfg[ "video ntsc bottom"            ] = settings.rects.ntsc.bottom - 1;	
		cfg[ "video pal left"               ] = settings.rects.pal.left;
		cfg[ "video pal top"                ] = settings.rects.pal.top;
		cfg[ "video pal right"              ] = settings.rects.pal.right - 1;
		cfg[ "video pal bottom"             ] = settings.rects.pal.bottom - 1;	
		cfg[ "video color brightness"       ] = Nes::Video(nes).GetBrightness();
		cfg[ "video color saturation"       ] = Nes::Video(nes).GetSaturation();
		cfg[ "video color hue"              ] = Nes::Video(nes).GetHue();
		cfg[ "video auto display frequency" ].YesNo() = settings.autoHz;
	}

	Nes::Video::Palette::Mode Video::GetDesiredPaletteMode() const
	{
		if (Nes::Machine(nes).Is(Nes::Machine::VS))
			return Nes::Video::Palette::MODE_RGB;
		else
			return Nes::Video::Palette::MODE_YUV;
	}

	void Video::UpdatePaletteMode() const
	{
		if (settings.autoPalette && settings.lockedPalette.Empty())
		{
			Nes::Video::Palette::Mode mode;

			if (Nes::Machine(nes).Is(Nes::Machine::VS))
				mode = Nes::Video::Palette::MODE_RGB;
			else
				mode = Nes::Video::Palette::MODE_YUV;

			Nes::Video(nes).GetPalette().SetMode( mode );
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	const Rect Video::GetNesRect(const Nes::Machine::Mode mode) const
	{
		const Rect& rect = (mode == Nes::Machine::NTSC ? settings.rects.ntsc : settings.rects.pal);

		if (settings.filter != settings.filters + FILTER_NTSC)
		{
			return rect;
		}
		else
		{
			Rect tmp( rect );

			if (tmp.right == NES_WIDTH)
				tmp.right = NES_WIDTH-1;

			if (settings.filters[FILTER_NTSC].attributes[1])
			{
				tmp.left   = (long) (tmp.left   * NTSC_WIDTH  / double(NES_WIDTH-1) + 0.5); 
				tmp.top    = (long) (tmp.top    * NTSC_HEIGHT / double(NES_HEIGHT)  + 0.5); 
				tmp.right  = (long) (tmp.right  * NTSC_WIDTH  / double(NES_WIDTH-1) + 0.5); 
				tmp.bottom = (long) (tmp.bottom * NTSC_HEIGHT / double(NES_HEIGHT)  + 0.5); 
			}

			return tmp;
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	void Video::GetRenderState(Nes::Video::RenderState& state,Rect& rect,const Window::Generic window) const
	{
		typedef Nes::Video::RenderState State;

		rect = (Nes::Machine(nes).GetMode() == Nes::Machine::NTSC ? settings.rects.ntsc : settings.rects.pal);

		state.width = NES_WIDTH;
		state.height = NES_HEIGHT;

		uint scale = 1;

		switch (settings.filter - settings.filters)
		{
			case FILTER_SCANLINES:
				
				NST_ASSERT( settings.adapter->maxScreenSize >= NST_MAX(NES_WIDTH*2,NES_HEIGHT*2) );

				// if target screen height is at least twice the size of
				// the original NES rectangle double the NES's too

				if (Rect::Picture(window).Height() >= rect.Height() * 2)
					scale = 2;

				if (settings.filters[FILTER_SCANLINES].attributes[0] == Filter::SCANLINES_BRIGHT)
					state.filter = State::FILTER_SCANLINES_BRIGHT;
				else
					state.filter = State::FILTER_SCANLINES_DARK;
				break;

			case FILTER_NTSC:
			
				if (rect.right == NES_WIDTH)
					--rect.right;

				rect.left   = (long) (rect.left   * NTSC_WIDTH  / double(NES_WIDTH-1) + 0.5); 
				rect.top    = (long) (rect.top    * NTSC_HEIGHT / double(NES_HEIGHT)  + 0.5); 
				rect.right  = (long) (rect.right  * NTSC_WIDTH  / double(NES_WIDTH-1) + 0.5); 
				rect.bottom = (long) (rect.bottom * NTSC_HEIGHT / double(NES_HEIGHT)  + 0.5); 

				state.width = NTSC_WIDTH;
				state.height = NTSC_HEIGHT;

				switch (settings.filters[FILTER_NTSC].attributes[0])
				{
					case Filter::SCANLINES_BRIGHT: state.filter = State::FILTER_NTSC_SCANLINES_BRIGHT; break;
					case Filter::SCANLINES_DARK:   state.filter = State::FILTER_NTSC_SCANLINES_DARK;   break;
					default:                       state.filter = State::FILTER_NTSC;				   break;
				}
				break;

			case FILTER_2XSAI:

				NST_ASSERT( settings.adapter->maxScreenSize >= NST_MAX(NES_WIDTH*2,NES_HEIGHT*2) );

				scale = 2;

				switch (settings.filters[FILTER_2XSAI].attributes[0])
				{
					case Filter::TYPE_SUPER2XSAI: state.filter = State::FILTER_SUPER_2XSAI; break;
					case Filter::TYPE_SUPEREAGLE: state.filter = State::FILTER_SUPER_EAGLE; break;
					default:                      state.filter = State::FILTER_2XSAI;		break;
				}
				break;

			case FILTER_SCALEX:

				NST_ASSERT( settings.adapter->maxScreenSize >= NST_MAX(NES_WIDTH*2,NES_HEIGHT*2) );

				if (settings.filters[FILTER_SCALEX].attributes[0] == Filter::TYPE_SCALE3X && settings.adapter->maxScreenSize >= NST_MAX(NES_WIDTH*3,NES_HEIGHT*3))
				{
					scale = 3;
					state.filter = State::FILTER_SCALE3X;
				}
				else
				{
					scale = 2;
					state.filter = State::FILTER_SCALE2X;
				}
				break;

			case FILTER_HQX:

				NST_ASSERT( settings.adapter->maxScreenSize >= NST_MAX(NES_WIDTH*2,NES_HEIGHT*2) );

				if (settings.filters[FILTER_HQX].attributes[0] == Filter::TYPE_HQ3X && settings.adapter->maxScreenSize >= NST_MAX(NES_WIDTH*3,NES_HEIGHT*3))
				{
					scale = 3;
					state.filter = State::FILTER_HQ3X;
				}
				else
				{
					scale = 2;
					state.filter = State::FILTER_HQ2X;
				}
				break;

			default:

				state.filter = State::FILTER_NONE;
				break;
		}

		state.width = (ushort) (state.width * scale);
		state.height = (ushort) (state.height * scale);
		rect = rect * scale;
	}

	void Video::LoadGamePalette(const Path& path)
	{
		if (path.Length())
		{
			settings.lockedPalette = path;
			settings.lockedMode = Nes::Video(nes).GetPalette().GetMode();
			Nes::Video(nes).GetPalette().SetMode( Nes::Video::Palette::MODE_CUSTOM );
			ImportPalette( settings.lockedPalette, Managers::Paths::QUIETLY );
		}
	}

	void Video::UnloadGamePalette()
	{
		if (settings.lockedPalette.Length())
		{
			settings.lockedPalette.Destroy();
			Nes::Video(nes).GetPalette().SetMode( settings.lockedMode );
			ImportPalette( settings.palette, Managers::Paths::QUIETLY );
		}
	}

	void Video::SavePalette(Path& path) const
	{
		if (Nes::Video(nes).GetPalette().GetMode() == Nes::Video::Palette::MODE_CUSTOM)
		{
			if (settings.lockedPalette.Length())
			{
				path = settings.lockedPalette;
			}
			else if (settings.palette.Length())
			{
				path = settings.palette;
			}
		}
	}

	Video::Modes::const_iterator Video::GetDialogMode() const
	{
		for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it)
		{
			if (it->bpp == settings.mode->bpp && it->width >= DEFAULT_WIDTH && it->height >= DEFAULT_HEIGHT)
				return it;
		}

		return settings.adapter->modes.begin();
	}

	uint Video::GetFullscreenScaleMethod() const
	{
		return settings.filter == settings.filters+FILTER_NTSC ? settings.filters[FILTER_NTSC].attributes[1] ? 2 : 1 : 0;
	}

	void Video::UpdateFullscreenScaleMethod(uint prev)
	{
		if (settings.fullscreenScale != SCREEN_STRETCHED && prev != GetFullscreenScaleMethod())
			settings.fullscreenScale = SCREEN_MATCHED;
	}

	ibool Video::PutTextureInVideoMemory() const
	{
		if (settings.texMem == Settings::TEXMEM_AUTO)
			return (settings.filter != settings.filters + FILTER_HQX);
		else 
			return (settings.texMem == Settings::TEXMEM_VIDMEM);
	}

	void Video::ValidateRects()
	{
		Rect& ntsc = settings.rects.ntsc;
		Rect& pal = settings.rects.pal;

		ntsc.left   = NST_CLAMP( ntsc.left,   0,         NES_WIDTH  -1 );
		ntsc.top    = NST_CLAMP( ntsc.top,    0,         NES_HEIGHT -1 );
		ntsc.right  = NST_CLAMP( ntsc.right,  ntsc.left, NES_WIDTH  -1 ) + 1;
		ntsc.bottom = NST_CLAMP( ntsc.bottom, ntsc.top,  NES_HEIGHT -1 ) + 1;
		pal.left    = NST_CLAMP( pal.left,    0,         NES_WIDTH  -1 );
		pal.top     = NST_CLAMP( pal.top,     0,         NES_HEIGHT -1 );
		pal.right   = NST_CLAMP( pal.right,   pal.left,  NES_WIDTH  -1 ) + 1;
		pal.bottom  = NST_CLAMP( pal.bottom,  pal.top,   NES_HEIGHT -1 ) + 1;
	}

	void Video::ResetColors()
	{
		Nes::Video(nes).SetBrightness ( Nes::Video(nes).GetDefaultBrightness() );
		Nes::Video(nes).SetSaturation ( Nes::Video(nes).GetDefaultSaturation() );
		Nes::Video(nes).SetHue        ( Nes::Video(nes).GetDefaultHue()        );   
	}

	Video::Modes::const_iterator Video::GetDefaultMode() const
	{
		for (uint bpp=16; bpp <= 32; bpp += 16)
		{
			const Modes::const_iterator it( settings.adapter->modes.find(Mode(DEFAULT_WIDTH,DEFAULT_HEIGHT,bpp)) );

			if (it != settings.adapter->modes.end())
				return it;
		}

		return settings.adapter->modes.begin();
	}

	ibool Video::OnInitDialog(Param&)
	{
		{
			const Control::ComboBox comboBox( dialog.ComboBox(IDC_GRAPHICS_DEVICE) );

			for (Adapters::const_iterator it(adapters.begin()), end(adapters.end()); it != end; ++it)
			{
				comboBox.Add( it->name.Ptr() );

				if (settings.adapter == it)
					comboBox[it - adapters.begin()].Select();
			}
		}
          
		for (uint i=IDC_GRAPHICS_NTSC_LEFT; i <= IDC_GRAPHICS_PAL_BOTTOM; ++i)
			dialog.Edit( i ).Limit( 3 );

		for (uint i=IDC_GRAPHICS_COLORS_BRIGHTNESS; i <= IDC_GRAPHICS_COLORS_HUE; ++i)
			dialog.Slider( i ).SetRange( 0, 255 );

		dialog.CheckBox( IDC_GRAPHICS_AUTO_HZ ).Check( settings.autoHz );

		UpdateDevice( *settings.mode );
		UpdateTexMem();
		UpdateRects();
		UpdateColors();
		UpdatePalette();

		return TRUE;
	}

	ibool Video::OnHScroll(Param& param)
	{
		const uint value = param.Slider().Scroll();

		     if (param.Slider().IsControl( IDC_GRAPHICS_COLORS_BRIGHTNESS )) Nes::Video(nes).SetBrightness ( value );
		else if (param.Slider().IsControl( IDC_GRAPHICS_COLORS_SATURATION )) Nes::Video(nes).SetSaturation ( value );
		else if (param.Slider().IsControl( IDC_GRAPHICS_COLORS_HUE        )) Nes::Video(nes).SetHue        ( value );
		else return TRUE;

		UpdateColors();
		UpdateScreen( param.hWnd );

		return TRUE;
	}

	ibool Video::OnCmdDevice(Param& param)
	{
		if (param.ComboBox().SelectionChanged())
		{
			settings.adapter = adapters.begin();
			
			for (uint i=dialog.ComboBox( IDC_GRAPHICS_DEVICE ).Selection().GetIndex(); i; --i)
				++settings.adapter;

			UpdateDevice( *settings.mode );
			UpdateScreen( param.hWnd );
		}

		return TRUE;
	}

	ibool Video::OnCmdBitDepth(Param& param)
	{
		if (param.Button().IsClicked())
			UpdateResolutions( Mode(settings.mode->width,settings.mode->height,8U << (param.Button().GetId() - IDC_GRAPHICS_8_BIT)) );

		return TRUE;
	}

	ibool Video::OnCmdMode(Param& param)
	{
		if (param.ComboBox().SelectionChanged())
		{
			settings.mode = settings.adapter->modes.begin();

			for (uint i=dialog.ComboBox(IDC_GRAPHICS_MODE).Selection().Data(); i; --i)
				++settings.mode;
		}

		return TRUE;
	}
							 
	ibool Video::OnCmdFilter(Param& param)
	{
		if (param.ComboBox().SelectionChanged())
		{
			const uint method = GetFullscreenScaleMethod();
			settings.filter = settings.filters + (FilterType) (Control::ComboBox::Value) dialog.ComboBox( IDC_GRAPHICS_EFFECTS ).Selection().Data();
			UpdateFullscreenScaleMethod( method );

			UpdatePalette();
			UpdateScreen( param.hWnd );
		}

		return TRUE;
	}

	ibool Video::OnCmdFilterSettings(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const MsgHandler::Entry<Video> messages[] =
			{
				{ WM_INITDIALOG, &Video::OnInitFilterDialog }
			};

			const MsgHandler::Entry<Video> commands[] =
			{
				{ IDC_VIDEO_FILTER_OK,      &Video::OnCmdFilterOk      },
				{ IDC_VIDEO_FILTER_DEFAULT, &Video::OnCmdFilterDefault },
				{ IDC_VIDEO_FILTER_CANCEL,  &Video::OnCmdFilterCancel  }
			};

			static const ushort idd[] = 
			{
				IDD_VIDEO_FILTER_NONE,
				IDD_VIDEO_FILTER_SCANLINES,
				IDD_VIDEO_FILTER_NTSC,
				IDD_VIDEO_FILTER_2XSAI,
				IDD_VIDEO_FILTER_SCALEX,
				IDD_VIDEO_FILTER_HQX
			};

			const uint method = GetFullscreenScaleMethod();
			Dialog( idd[settings.filter-settings.filters], this, messages, commands ).Open();
			UpdateFullscreenScaleMethod( method );

			UpdateScreen( param.hWnd );
		}

		return TRUE;
	}

	ibool Video::OnCmdTexMem(Param& param)
	{
		if (param.Button().IsClicked())
		{
			settings.texMem = (Settings::TexMem) (param.Button().GetId() - IDC_GRAPHICS_NESTEXTURE_AUTO);
			UpdateTexMem();
		}

		return TRUE;
	}

	ibool Video::OnCmdPalType(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint cmd = param.Button().GetId();

			settings.autoPalette = (cmd == IDC_GRAPHICS_PALETTE_AUTO);

			Nes::Video(nes).GetPalette().SetMode
			( 
				cmd == IDC_GRAPHICS_PALETTE_YUV      ? Nes::Video::Palette::MODE_YUV :
				cmd == IDC_GRAPHICS_PALETTE_RGB      ? Nes::Video::Palette::MODE_RGB :
				cmd == IDC_GRAPHICS_PALETTE_CUSTOM   ? Nes::Video::Palette::MODE_CUSTOM :
				                                       GetDesiredPaletteMode()
			);

			UpdatePalette();
			UpdateScreen( param.hWnd );
		}

		return TRUE;
	}

	ibool Video::OnCmdColorsReset(Param& param)
	{
		if (param.Button().IsClicked())
		{
			ResetColors();
			UpdateColors();
			UpdateScreen( param.hWnd );
		}

		return TRUE;
	}

	ibool Video::OnCmdColorsAdvanced(Param& param)
	{
		if (param.Button().IsClicked())
		{
			VideoDecoder videoDecoder( nes );
			UpdateScreen( param.hWnd );
		}

		return TRUE;
	}

	ibool Video::OnCmdPalBrowse(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const Path file
			(
				paths.BrowseLoad
				( 
					Managers::Paths::File::PALETTE|Managers::Paths::File::ARCHIVE,
					settings.palette
				)
			);

			if (file.Length())
			{
				settings.palette = file;
				
				ImportPalette( settings.palette, Managers::Paths::NOISY );
				UpdatePalette();				
				UpdateScreen( param.hWnd );
			}
		}

		return TRUE;
	}

	ibool Video::OnCmdPalClear(Param& param)
	{
		if (param.Button().IsClicked())
		{
			if (settings.palette.Length())
			{
				settings.palette.Destroy();

				if (Nes::Video(nes).GetPalette().GetMode() == Nes::Video::Palette::MODE_CUSTOM )
					Nes::Video(nes).GetPalette().SetMode( GetDesiredPaletteMode() );

				UpdatePalette();
				UpdateScreen( param.hWnd );
			}
		}

		return TRUE;
	}

	ibool Video::OnCmdPalEditor(Param& param)
	{
		if (param.Button().IsClicked())
		{
			Nes::Video video( nes );

			if (settings.palette.Empty())
				video.GetPalette().ResetCustom();

			{
				const Path path( PaletteEditor( video, paths, settings.palette ).GetPaletteFile() );

				if (settings.palette.Empty() && path.Length())
					settings.palette = path;
			}

			if (settings.palette.Length())
			{
				ImportPalette( settings.palette, Managers::Paths::QUIETLY );
				UpdatePalette();
			}

			UpdateScreen( param.hWnd );
		}

		return TRUE;
	}

	ibool Video::OnCmdAutoHz(Param& param)
	{
		if (param.Button().IsClicked())
			settings.autoHz = dialog.CheckBox( IDC_GRAPHICS_AUTO_HZ ).IsChecked();

		return TRUE;
	}

	ibool Video::OnCmdDefault(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint method = GetFullscreenScaleMethod();

			settings.adapter = adapters.begin();
			settings.mode = GetDefaultMode();
			settings.filter = settings.filters + FILTER_NONE;

			for (uint i=0; i < NUM_FILTERS; ++i)
				settings.filters[i].bilinear = false;

			settings.filters[ FILTER_SCANLINES ].attributes[0] = Filter::SCANLINES_BRIGHT;
			settings.filters[ FILTER_NTSC      ].attributes[0] = Filter::SCANLINES_NONE;
			settings.filters[ FILTER_NTSC      ].attributes[1] = false;
			settings.filters[ FILTER_2XSAI     ].attributes[0] = Filter::TYPE_2XSAI;
			settings.filters[ FILTER_SCALEX    ].attributes[0] = Filter::TYPE_SCALE2X;
			settings.filters[ FILTER_HQX       ].attributes[0] = Filter::TYPE_HQ2X;

			UpdateDevice( *settings.mode );

			settings.texMem = Settings::TEXMEM_AUTO;
			UpdateTexMem();

			settings.rects.ntsc.Set( 0, NTSC_CLIP_TOP, NES_WIDTH, NTSC_CLIP_BOTTOM );
			settings.rects.pal.Set( 0, PAL_CLIP_TOP, NES_WIDTH, PAL_CLIP_BOTTOM );
			UpdateRects();

			Nes::Video(nes).SetDecoder( Nes::Video::Decoder(Nes::Video::DECODER_CANONICAL) );

			settings.autoHz = FALSE;
			dialog.CheckBox( IDC_GRAPHICS_AUTO_HZ ).Uncheck();

			ResetColors();
			UpdateColors();

			settings.autoPalette = TRUE;

			if (settings.lockedPalette.Empty())
				Nes::Video(nes).GetPalette().SetMode( GetDesiredPaletteMode() );

			UpdateFullscreenScaleMethod( method );

			UpdatePalette();
			UpdateScreen( param.hWnd );
		}

		return TRUE;
	}

	ibool Video::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			dialog.Control( IDC_GRAPHICS_NTSC_LEFT   ).Text() >> settings.rects.ntsc.left;
			dialog.Control( IDC_GRAPHICS_NTSC_TOP    ).Text() >> settings.rects.ntsc.top;
			dialog.Control( IDC_GRAPHICS_NTSC_RIGHT  ).Text() >> settings.rects.ntsc.right;
			dialog.Control( IDC_GRAPHICS_NTSC_BOTTOM ).Text() >> settings.rects.ntsc.bottom;
			dialog.Control( IDC_GRAPHICS_PAL_LEFT    ).Text() >> settings.rects.pal.left;
			dialog.Control( IDC_GRAPHICS_PAL_TOP     ).Text() >> settings.rects.pal.top;
			dialog.Control( IDC_GRAPHICS_PAL_RIGHT   ).Text() >> settings.rects.pal.right;
			dialog.Control( IDC_GRAPHICS_PAL_BOTTOM  ).Text() >> settings.rects.pal.bottom;

			ValidateRects();

			dialog.Close();
		}

		return TRUE;
	}

	ibool Video::OnInitFilterDialog(Param& param)
	{
		Control::CheckBox(param.hWnd,IDC_VIDEO_FILTER_BILINEAR).Check( settings.filter->bilinear );

		if (!(settings.adapter->filters & Adapter::FILTER_BILINEAR))
			Control::CheckBox(param.hWnd,IDC_VIDEO_FILTER_BILINEAR).Disable();

		uint idc;

		switch (settings.filter - settings.filters)
		{
			case FILTER_SCANLINES:

				if (settings.filter->attributes[0] == Filter::SCANLINES_DARK)
					idc = IDC_VIDEO_FILTER_SCANLINES_TYPE_DARK;
				else
					idc = IDC_VIDEO_FILTER_SCANLINES_TYPE_BRIGHT;

				Control::RadioButton(param.hWnd,idc).Check();
				break;

			case FILTER_NTSC:

				Control::CheckBox(param.hWnd,IDC_VIDEO_FILTER_NTSC_WIDESCREEN).Check( settings.filter->attributes[1] );

				switch (settings.filter->attributes[0])
				{
					case Filter::SCANLINES_BRIGHT: idc = IDC_VIDEO_FILTER_NTSC_SCANLINES_BRIGHT; break;
					case Filter::SCANLINES_DARK:   idc = IDC_VIDEO_FILTER_NTSC_SCANLINES_DARK;   break;
					default:                       idc = IDC_VIDEO_FILTER_NTSC_SCANLINES_NONE;   break;
				}

				Control::RadioButton(param.hWnd,idc).Check();
				break;

			case FILTER_2XSAI:

				switch (settings.filter->attributes[0])
				{
					case Filter::TYPE_SUPER2XSAI: idc = IDC_VIDEO_FILTER_2XSAI_TYPE_SUPER2XSAI; break;
					case Filter::TYPE_SUPEREAGLE: idc = IDC_VIDEO_FILTER_2XSAI_TYPE_SUPEREAGLE; break;
					default:                      idc = IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI;      break;
				}

				Control::RadioButton(param.hWnd,idc).Check();
				break;

			case FILTER_SCALEX:

				if (settings.filter->attributes[0] == Filter::TYPE_SCALE3X)
					idc = IDC_VIDEO_FILTER_SCALEX_3X;
				else
					idc = IDC_VIDEO_FILTER_SCALEX_2X;

				Control::RadioButton(param.hWnd,idc).Check();

				if (settings.adapter->maxScreenSize < NST_MAX(NES_HEIGHT*3,NES_WIDTH*3))
				{
					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_SCALEX_2X).Disable();
					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_SCALEX_3X).Disable();
				}
				break;

			case FILTER_HQX:

				if (settings.filter->attributes[0] == Filter::TYPE_HQ3X)
					idc = IDC_VIDEO_FILTER_HQX_SCALING_3X;
				else
					idc = IDC_VIDEO_FILTER_HQX_SCALING_2X;

				Control::RadioButton(param.hWnd,idc).Check();

				if (settings.adapter->maxScreenSize < NST_MAX(NES_HEIGHT*3,NES_WIDTH*3))
				{
					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_HQX_SCALING_2X).Disable();
					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_HQX_SCALING_3X).Disable();
				}
				break;
		}

		return TRUE;
	}

	ibool Video::OnCmdFilterDefault(Param& param)
	{
		if (param.Button().IsClicked())
		{
			Control::CheckBox(param.hWnd,IDC_VIDEO_FILTER_BILINEAR).Uncheck();

			switch (settings.filter-settings.filters)
			{
				case FILTER_SCANLINES:

					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_SCANLINES_TYPE_BRIGHT).Check();
					break;

				case FILTER_NTSC:

					Control::CheckBox(param.hWnd,IDC_VIDEO_FILTER_NTSC_WIDESCREEN).Uncheck();
					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_NTSC_SCANLINES_NONE).Check();
					break;

				case FILTER_2XSAI:

					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI).Check();
					break;

				case FILTER_SCALEX:

					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_SCALEX_2X).Check();
					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_SCALEX_3X).Uncheck();
					break;

				case FILTER_HQX:

					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_HQX_SCALING_2X).Check();
					Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_HQX_SCALING_3X).Uncheck();
					break;
			}
		}

		return TRUE;
	}

	ibool Video::OnCmdFilterCancel(Param& param)
	{
		if (param.Button().IsClicked())
			::EndDialog( param.hWnd, 0 );

		return TRUE;
	}

	ibool Video::OnCmdFilterOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			settings.filter->bilinear = Control::CheckBox(param.hWnd,IDC_VIDEO_FILTER_BILINEAR).IsChecked();
		
			switch (settings.filter-settings.filters)
			{
				case FILTER_SCANLINES:

				    if (Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_SCANLINES_TYPE_DARK).IsChecked())
						settings.filter->attributes[0] = Filter::SCANLINES_DARK;
					else
						settings.filter->attributes[0] = Filter::SCANLINES_BRIGHT;
					break;

				case FILTER_NTSC:

					settings.filter->attributes[1] = (bool) Control::CheckBox(param.hWnd,IDC_VIDEO_FILTER_NTSC_WIDESCREEN).IsChecked();

					if (Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_NTSC_SCANLINES_BRIGHT).IsChecked())
					{
						settings.filter->attributes[0] = Filter::SCANLINES_BRIGHT;
					}
					else if (Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_NTSC_SCANLINES_DARK).IsChecked())
					{
						settings.filter->attributes[0] = Filter::SCANLINES_DARK;
					}
					else
					{
						settings.filter->attributes[0] = Filter::SCANLINES_NONE;
					}
					break;

				case FILTER_2XSAI:

					if (Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_2XSAI_TYPE_SUPER2XSAI).IsChecked())
					{
						settings.filter->attributes[0] = Filter::TYPE_SUPER2XSAI;
					}
					else if (Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_2XSAI_TYPE_SUPEREAGLE).IsChecked())
					{
						settings.filter->attributes[0] = Filter::TYPE_SUPEREAGLE;
					}
					else
					{
						settings.filter->attributes[0] = Filter::TYPE_2XSAI;
					}
					break;

				case FILTER_SCALEX:

					if (Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_SCALEX_3X).IsChecked())
						settings.filter->attributes[0] = Filter::TYPE_SCALE3X;
					else
						settings.filter->attributes[0] = Filter::TYPE_SCALE2X;
					break;

				case FILTER_HQX:

					if (Control::RadioButton(param.hWnd,IDC_VIDEO_FILTER_HQX_SCALING_3X).IsChecked())
						settings.filter->attributes[0] = Filter::TYPE_HQ3X;
					else
						settings.filter->attributes[0] = Filter::TYPE_HQ2X;
					break;
			}

			::EndDialog( param.hWnd, 0 );
		}

		return TRUE;
	}

	void Video::UpdateDevice(Mode mode)
	{
		dialog.Slider( IDC_GRAPHICS_NESTEXTURE_AUTO   ).Enable( settings.adapter->videoMemScreen );
		dialog.Slider( IDC_GRAPHICS_NESTEXTURE_VIDMEM ).Enable( settings.adapter->videoMemScreen );
		dialog.Slider( IDC_GRAPHICS_NESTEXTURE_SYSMEM ).Enable( settings.adapter->videoMemScreen );

		uint available = 0;

		for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it)
		{
			switch (it->bpp)
			{
				case 8:  available |=  8; break;
				case 16: available |= 16; break;
				case 32: available |= 32; break;
			}

			if (available == (8|16|32))
				break;
		}

		NST_ASSERT( available & (8|16|32) );

		dialog.Control( IDC_GRAPHICS_8_BIT  ).Enable( available & 8  );
		dialog.Control( IDC_GRAPHICS_16_BIT ).Enable( available & 16 );
		dialog.Control( IDC_GRAPHICS_32_BIT ).Enable( available & 32 );

		switch (mode.bpp)
		{
			case 32: mode.bpp = ((available & 32) ? 32 : (available & 16) ? 16 :  8); break;
			case 16: mode.bpp = ((available & 16) ? 16 : (available & 32) ? 32 :  8); break;
			case  8: mode.bpp = ((available &  8) ?  8 : (available & 16) ? 16 : 32); break;
		}

		UpdateResolutions( mode );
		UpdateFilters();
	}

	void Video::UpdateResolutions(Mode mode)
	{
		settings.mode = settings.adapter->modes.find( mode );

		if (settings.mode == settings.adapter->modes.end())
		{
			settings.mode = settings.adapter->modes.find( Mode(DEFAULT_WIDTH,DEFAULT_HEIGHT,mode.bpp) );

			if (settings.mode == settings.adapter->modes.end())
				settings.mode = settings.adapter->modes.begin();

			mode = *settings.mode;
		}

		dialog.RadioButton( IDC_GRAPHICS_8_BIT  ).Check( mode.bpp == 8  );
		dialog.RadioButton( IDC_GRAPHICS_16_BIT ).Check( mode.bpp == 16 );
		dialog.RadioButton( IDC_GRAPHICS_32_BIT ).Check( mode.bpp == 32 );

		const Control::ComboBox comboBox( dialog.ComboBox(IDC_GRAPHICS_MODE) );
		comboBox.Clear();

		uint idx=0;
		HeapString string;

		for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it, ++idx)
		{
			if (mode.bpp == it->bpp)
			{
				string.Clear();
				comboBox.Add( (string << it->width << 'x' << it->height).Ptr() ).Data() = idx;

				if (mode.width == it->width && mode.height == it->height)
					comboBox[comboBox.Size() - 1].Select();
			}
		}
	}

	void Video::UpdateFilters()
	{
		const Control::ComboBox comboBox( dialog.ComboBox(IDC_GRAPHICS_EFFECTS) );
		comboBox.Clear();

		comboBox.Add( Resource::String(IDS_VIDEO_FILTER_NONE) ).Data() = FILTER_NONE;

		if (settings.adapter->maxScreenSize >= NST_MAX(NES_WIDTH*2,NES_HEIGHT*2))
		{
			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SCANLINES) ).Data() = FILTER_SCANLINES;

			if (settings.adapter->maxScreenSize >= NST_MAX(NTSC_WIDTH,NTSC_HEIGHT))
				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_NTSC) ).Data() = FILTER_NTSC;

			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_2XSAI) ).Data() = FILTER_2XSAI;
			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SCALEX) ).Data() = FILTER_SCALEX;
			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_HQX) ).Data() = FILTER_HQX;
		}

		for (uint i=0, size=comboBox.Size(); i < size; ++i)
		{
			if (settings.filter-settings.filters == (FilterType) (Control::ComboBox::Value) comboBox[i].Data())
			{
				comboBox[i].Select();
				return;
			}
		}

		settings.filter = settings.filters + FILTER_NONE;
		comboBox[0].Select();
	}

	void Video::UpdateTexMem() const
	{
		dialog.RadioButton( IDC_GRAPHICS_NESTEXTURE_AUTO   ).Check( settings.texMem == Settings::TEXMEM_AUTO   );
		dialog.RadioButton( IDC_GRAPHICS_NESTEXTURE_VIDMEM ).Check( settings.texMem == Settings::TEXMEM_VIDMEM );
		dialog.RadioButton( IDC_GRAPHICS_NESTEXTURE_SYSMEM ).Check( settings.texMem == Settings::TEXMEM_SYSMEM );
	}

	void Video::UpdateColors() const
	{
		const uint colors[] =
		{
			Nes::Video(nes).GetBrightness(),
			Nes::Video(nes).GetSaturation(),
			Nes::Video(nes).GetHue()       
		};

		dialog.Control( IDC_GRAPHICS_COLORS_BRIGHTNESS_VAL ).Text() << colors[0];
		dialog.Control( IDC_GRAPHICS_COLORS_SATURATION_VAL ).Text() << colors[1];
		dialog.Control( IDC_GRAPHICS_COLORS_HUE_VAL        ).Text() << colors[2];

		dialog.Slider( IDC_GRAPHICS_COLORS_BRIGHTNESS ).Position() = colors[0];
		dialog.Slider( IDC_GRAPHICS_COLORS_SATURATION ).Position() = colors[1];
		dialog.Slider( IDC_GRAPHICS_COLORS_HUE        ).Position() = colors[2];
	}

	void Video::UpdateRects() const
	{
		dialog.Control( IDC_GRAPHICS_NTSC_LEFT   ).Text() << (settings.rects.ntsc.left);
		dialog.Control( IDC_GRAPHICS_NTSC_TOP    ).Text() << (settings.rects.ntsc.top);
		dialog.Control( IDC_GRAPHICS_NTSC_RIGHT  ).Text() << (settings.rects.ntsc.right - 1);
		dialog.Control( IDC_GRAPHICS_NTSC_BOTTOM ).Text() << (settings.rects.ntsc.bottom - 1);
		dialog.Control( IDC_GRAPHICS_PAL_LEFT    ).Text() << (settings.rects.pal.left);
		dialog.Control( IDC_GRAPHICS_PAL_TOP     ).Text() << (settings.rects.pal.top);
		dialog.Control( IDC_GRAPHICS_PAL_RIGHT   ).Text() << (settings.rects.pal.right - 1);
		dialog.Control( IDC_GRAPHICS_PAL_BOTTOM  ).Text() << (settings.rects.pal.bottom - 1);
	}

	void Video::UpdatePalette() const
	{
		const ibool enable = 
		(
       		settings.lockedPalette.Empty() &&
	     	settings.filter != settings.filters + FILTER_NTSC
		);

		for (uint i=IDC_GRAPHICS_PALETTE_AUTO; i <= IDC_GRAPHICS_PALETTE_EDITOR; ++i)
			dialog.Control( i ).Enable( enable );

		dialog.Control( IDC_GRAPHICS_PALETTE_CUSTOM ).Enable( enable && settings.palette.Length() );
		dialog.Edit( IDC_GRAPHICS_PALETTE_PATH ) << (settings.lockedPalette.Length() ? settings.lockedPalette.Ptr() : settings.palette.Ptr());

		const Nes::Video::Palette::Mode mode = Nes::Video(nes).GetPalette().GetMode();

		dialog.Control( IDC_GRAPHICS_COLORS_ADVANCED ).Enable( mode == Nes::Video::Palette::MODE_YUV );

		dialog.RadioButton( IDC_GRAPHICS_PALETTE_AUTO     ).Check( settings.autoPalette );
		dialog.RadioButton( IDC_GRAPHICS_PALETTE_YUV      ).Check( !settings.autoPalette && mode == Nes::Video::Palette::MODE_YUV    );
		dialog.RadioButton( IDC_GRAPHICS_PALETTE_RGB      ).Check( !settings.autoPalette && mode == Nes::Video::Palette::MODE_RGB    );
		dialog.RadioButton( IDC_GRAPHICS_PALETTE_CUSTOM   ).Check( !settings.autoPalette && mode == Nes::Video::Palette::MODE_CUSTOM );
	}

	void Video::ImportPalette(Path& palette,Managers::Paths::Alert alert)
	{
		if (palette.Length())
		{
			Managers::Paths::File file;

			if
			(
				!paths.Load( file, Managers::Paths::File::PALETTE|Managers::Paths::File::ARCHIVE, palette, alert ) || file.data.Size() < 64*3 ||
				NES_FAILED(Nes::Video(nes).GetPalette().SetCustom( reinterpret_cast<Nes::Video::Palette::Colors>(file.data.Begin()) ))
			)
			{
				if (alert == Managers::Paths::QUIETLY)
					Io::Log() << "Video: warning, custom palette file: \"" << palette << "\" invalid or not found!\r\n";
				else
					Window::User::Fail( IDS_DIALOG_VIDEO_INVALID_PALETTE );

				palette.Destroy();
				Nes::Video(nes).GetPalette().SetMode( GetDesiredPaletteMode() );
			}
		}
	}

	void Video::UpdateScreen(HWND const hDlg)
	{
		::RedrawWindow( ::GetParent( hDlg ), NULL, NULL, RDW_INVALIDATE );
	}
}
