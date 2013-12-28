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

#include <algorithm>
#include "NstIoFile.hpp"
#include "NstIoLog.hpp"
#include "NstWindowUser.hpp"
#include "NstResourceString.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogVideo.hpp"

#ifdef __INTEL_COMPILER
#pragma warning( disable : 279 )
#endif

namespace Nestopia
{
	using namespace Window;

	Video::Settings::Settings()
	:
	filter ( FILTER_NONE ),
	texMem ( TEXMEM_AUTO )
	{
	}

	struct Video::Handlers
	{
		static const MsgHandler::Entry<Video> messages[];
		static const MsgHandler::Entry<Video> commands[];
	};

	const MsgHandler::Entry<Video> Video::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Video::OnInitDialog },
		{ WM_DESTROY,    &Video::OnDestroy    },
		{ WM_HSCROLL,    &Video::OnHScroll    }
	};

	const MsgHandler::Entry<Video> Video::Handlers::commands[] =
	{
		{ IDC_GRAPHICS_DEVICE,            &Video::OnCmdDevice      },
		{ IDC_GRAPHICS_MODE,              &Video::OnCmdMode        },
		{ IDC_GRAPHICS_8_BIT,             &Video::OnCmdBitDepth    },
		{ IDC_GRAPHICS_16_BIT,            &Video::OnCmdBitDepth    },
		{ IDC_GRAPHICS_32_BIT,            &Video::OnCmdBitDepth    },
		{ IDC_GRAPHICS_NESTEXTURE_AUTO,   &Video::OnCmdTexMem      },
		{ IDC_GRAPHICS_NESTEXTURE_VIDMEM, &Video::OnCmdTexMem      },
		{ IDC_GRAPHICS_NESTEXTURE_SYSMEM, &Video::OnCmdTexMem      },
		{ IDC_GRAPHICS_PALETTE_EMULATED,  &Video::OnCmdPalType     },
		{ IDC_GRAPHICS_PALETTE_INTERNAL,  &Video::OnCmdPalType     },
		{ IDC_GRAPHICS_PALETTE_CUSTOM,    &Video::OnCmdPalType     },
		{ IDC_GRAPHICS_COLORS_RESET,      &Video::OnCmdColorsReset },
		{ IDC_GRAPHICS_PALETTE_BROWSE,    &Video::OnCmdPalBrowse   },
		{ IDC_GRAPHICS_PALETTE_CLEAR,     &Video::OnCmdPalClear    },
		{ IDC_GRAPHICS_AUTO_HZ,			  &Video::OnCmdAutoHz      },
		{ IDC_GRAPHICS_DEFAULT,           &Video::OnCmdDefault     },
		{ IDC_GRAPHICS_OK,                &Video::OnCmdOk          }
	};

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
			const String::Heap& type = cfg["video filter"];

		    settings.filter = FILTER_NONE;

			if (type == "bilinear")
			{
				if (settings.adapter->filters & Adapter::FILTER_BILINEAR)
					settings.filter = FILTER_BILINEAR;
			}
			else if (type == "tv soft")
			{
				if 
				(
			    	(settings.adapter->filters & Adapter::FILTER_BILINEAR) &&
					(settings.adapter->maxScreenSize >= NST_MAX(NES_HEIGHT * 2,NES_WIDTH * 2))
				)
					settings.filter = FILTER_TV_SOFT;
			}
			else if (type == "scanlines bright")
			{
				settings.filter = FILTER_SCANLINES_BRIGHT;
			}
			else if (type == "scanlines dark")
			{
				settings.filter = FILTER_SCANLINES_DARK;
			}
			else if (type == "scale3x")
			{
				if (settings.adapter->maxScreenSize >= NST_MAX(NES_HEIGHT * 4,NES_WIDTH * 4))
					settings.filter = FILTER_SCALE3X;
			}
			else if (type == "hq3x")
			{
				if (settings.adapter->maxScreenSize >= NST_MAX(NES_HEIGHT * 4,NES_WIDTH * 4))
					settings.filter = FILTER_HQ3X;
			}
			else if (settings.adapter->maxScreenSize >= NST_MAX(NES_HEIGHT * 2,NES_WIDTH * 2))
			{
				settings.filter =
				(
					type == "tv harsh"    ? FILTER_TV_HARSH :
					type == "2xsai"       ? FILTER_2XSAI :
					type == "super 2xsai" ? FILTER_SUPER_2XSAI :
					type == "super eagle" ? FILTER_SUPER_EAGLE :
					type == "scale2x"     ? FILTER_SCALE2X :
				    type == "hq2x"		  ? FILTER_HQ2X :
				                            FILTER_NONE
				);
			}
		}

		{
			const String::Heap& type = cfg["video texture location"];

			if (type == "vidmem")
			{
				settings.texMem = Settings::TEXMEM_VIDMEM;
			}
			else if (type == "sysmem")
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

			ntsc.left   = cfg[ "video ntsc left"   ].Default( 0   );
			ntsc.top    = cfg[ "video ntsc top"    ].Default( 8   );
			ntsc.right  = cfg[ "video ntsc right"  ].Default( 255 );
			ntsc.bottom = cfg[ "video ntsc bottom" ].Default( 231 );
			pal.left    = cfg[ "video pal left"    ].Default( 0   );
			pal.top     = cfg[ "video pal top"     ].Default( 0   );
			pal.right   = cfg[ "video pal right"   ].Default( 255 );
			pal.bottom  = cfg[ "video pal bottom"  ].Default( 239 );
		}

		ValidateRects();

		{
			uint color;

			if ((color = cfg[ "video color brightness" ].Default( 128U )) <= 255) nes.SetBrightness( color );
			if ((color = cfg[ "video color saturation" ].Default( 128U )) <= 255) nes.SetSaturation( color );
			if ((color = cfg[ "video color hue"        ].Default( 128U )) <= 255) nes.SetHue( color );
		}

		settings.palette = cfg["video palette file"];

		if (settings.palette.Size())
			ImportPalette( Managers::Paths::QUIETLY );

		{
			const String::Heap& type = cfg["video palette"];

			nes.GetPalette().SetMode
			( 
		    	type == "emulated" ?                          Nes::Video::Palette::EMULATED :
				type == "custom" && settings.palette.Size() ? Nes::Video::Palette::CUSTOM :
				                                              Nes::Video::Palette::INTERNAL
			);
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
			static cstring const names[] =
			{
				"none",            
				"bilinear",
				"scanlines bright",
				"scanlines dark",
				"tv soft",
				"tv harsh",
				"2xsai",           
				"super 2xsai",
				"super eagle",     
				"scale2x",         
				"scale3x",
				"hq2x",
				"hq3x"
			};

			cfg[ "video filter" ] = names[settings.filter];
		}

		{
			static cstring const names[] =
			{
				"auto","vidmem","sysmem"
			};

			cfg[ "video texture location" ] = names[settings.texMem];
		}

		{
			String::Generic name;

			switch (nes.GetPalette().GetMode())
			{
				case Nes::Video::Palette::EMULATED: name = "emulated"; break;
				case Nes::Video::Palette::CUSTOM:   name = "custom";   break;
				default:                            name = "internal"; break;
			}

			cfg[ "video palette" ] = name;
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
		cfg[ "video color brightness"       ] = nes.GetBrightness();
		cfg[ "video color saturation"       ] = nes.GetSaturation();
		cfg[ "video color hue"              ] = nes.GetHue();
		cfg[ "video auto display frequency" ].YesNo() = settings.autoHz;
	}

	Video::Modes::const_iterator Video::GetDialogMode() const
	{
		Modes::const_iterator it
		(
	    	settings.adapter->modes.find
     		(
     			Mode(DEFAULT_WIDTH,DEFAULT_HEIGHT,settings.mode->bpp)
			)
		);

		if (it == settings.adapter->modes.end())
			it = settings.adapter->modes.begin();

		return it;
	}

	ibool Video::PutTextureInVideoMemory() const
	{
		if (settings.texMem == Settings::TEXMEM_AUTO)
			return (settings.filter != FILTER_HQ2X && settings.filter != FILTER_HQ3X);
		else 
			return (settings.texMem == Settings::TEXMEM_VIDMEM);
	}

	void Video::ResetDevice()
	{
		settings.adapter = adapters.begin();
		settings.mode = GetDefaultMode();
	}

	void Video::ResetRects()
	{
		settings.rects.ntsc.Set( 0, 8, 256, 232 );
		settings.rects.pal.Set( 0, 0, 256, 240 );
	}

	void Video::ValidateRects()
	{
		Rect& ntsc = settings.rects.ntsc;
		Rect& pal = settings.rects.pal;

		ntsc.left   = NST_CLAMP( ntsc.left,   0,         255 );
		ntsc.top    = NST_CLAMP( ntsc.top,    0,         239 );
		ntsc.right  = NST_CLAMP( ntsc.right,  ntsc.left, 255 ) + 1;
		ntsc.bottom = NST_CLAMP( ntsc.bottom, ntsc.top,  239 ) + 1;
		pal.left    = NST_CLAMP( pal.left,    0,         255 );
		pal.top     = NST_CLAMP( pal.top,     0,         239 );
		pal.right   = NST_CLAMP( pal.right,   pal.left,  255 ) + 1;
		pal.bottom  = NST_CLAMP( pal.bottom,  pal.top,   239 ) + 1;
	}

	void Video::ResetColors()
	{
		nes.SetBrightness ( nes.GetDefaultBrightness() );
		nes.SetSaturation ( nes.GetDefaultSaturation() );
		nes.SetHue        ( nes.GetDefaultHue()        );   
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

			for (Adapters::const_iterator it(adapters.begin()); it != adapters.end(); ++it)
			{
				comboBox.Add( it->name );

				if (settings.adapter == it)
					comboBox[comboBox.Size() - 1].Select();
			}
		}
  
		NST_COMPILE_ASSERT
		(		   
			IDC_GRAPHICS_NTSC_TOP    - IDC_GRAPHICS_NTSC_LEFT == 1 && 
			IDC_GRAPHICS_NTSC_RIGHT  - IDC_GRAPHICS_NTSC_LEFT == 2 &&
			IDC_GRAPHICS_NTSC_BOTTOM - IDC_GRAPHICS_NTSC_LEFT == 3 &&
			IDC_GRAPHICS_PAL_LEFT    - IDC_GRAPHICS_NTSC_LEFT == 4 &&  
			IDC_GRAPHICS_PAL_TOP     - IDC_GRAPHICS_NTSC_LEFT == 5 &&    
			IDC_GRAPHICS_PAL_RIGHT   - IDC_GRAPHICS_NTSC_LEFT == 6 &&  
			IDC_GRAPHICS_PAL_BOTTOM  - IDC_GRAPHICS_NTSC_LEFT == 7 
		);
        
		for (uint i=IDC_GRAPHICS_NTSC_LEFT; i <= IDC_GRAPHICS_PAL_BOTTOM; ++i)
			dialog.Edit( i ).Limit( 3 );

		dialog.Slider( IDC_GRAPHICS_COLORS_BRIGHTNESS ).SetRange( 0, 255 );
		dialog.Slider( IDC_GRAPHICS_COLORS_SATURATION ).SetRange( 0, 255 );
		dialog.Slider( IDC_GRAPHICS_COLORS_HUE        ).SetRange( 0, 255 );

		UpdateColors();
		UpdateTexMem();
		UpdateRects();
		UpdatePalette();
		UpdateDevice();

		dialog.CheckBox( IDC_GRAPHICS_AUTO_HZ ).Check( settings.autoHz );

		const Control::ComboBox comboBox( dialog.ComboBox(IDC_GRAPHICS_EFFECTS) );

		for (uint i=0, size=comboBox.Size(); i < size; ++i)
		{
			if (settings.filter == (Filter) (Control::ComboBox::Value) comboBox[i].Data())
			{
				comboBox[i].Select();
				return TRUE;
			}
		}

		comboBox[0].Select();

		return TRUE;
	}

	ibool Video::OnHScroll(Param& param)
	{
		const uint value = param.Slider().Scroll();

		     if (param.Slider().IsControl( IDC_GRAPHICS_COLORS_BRIGHTNESS )) nes.SetBrightness ( value );
		else if (param.Slider().IsControl( IDC_GRAPHICS_COLORS_SATURATION )) nes.SetSaturation ( value );
		else if (param.Slider().IsControl( IDC_GRAPHICS_COLORS_HUE        )) nes.SetHue        ( value );
		else return TRUE;

		UpdateColors();
		UpdateScreen( param.hWnd );

		return TRUE;
	}

	ibool Video::OnDestroy(Param&)
	{
		settings.filter = (Filter) (Control::ComboBox::Value) dialog.ComboBox( IDC_GRAPHICS_EFFECTS ).Selection().Data();
		return TRUE;
	}

	ibool Video::OnCmdDevice(Param& param)
	{
		if (param.ComboBox().SelectionChanged())
		{
			settings.adapter = adapters.begin();
			
			for (uint i=0, index=dialog.ComboBox( IDC_GRAPHICS_DEVICE ).Selection().GetIndex(); i < index; ++i)
				++settings.adapter;
			
			UpdateDevice();
		}

		return TRUE;
	}

	ibool Video::OnCmdBitDepth(Param& param)
	{
		NST_COMPILE_ASSERT
		(
			IDC_GRAPHICS_16_BIT - IDC_GRAPHICS_8_BIT == 1 &&
			IDC_GRAPHICS_32_BIT - IDC_GRAPHICS_8_BIT == 2
		);

		if (param.Button().IsClicked())
			UpdateBitDepth( 8U << (param.Button().GetId() - IDC_GRAPHICS_8_BIT) );

		return TRUE;
	}

	ibool Video::OnCmdMode(Param& param)
	{
		if (param.ComboBox().SelectionChanged())
		{
			settings.mode = settings.adapter->modes.begin();

			for (uint i=0, index=dialog.ComboBox(IDC_GRAPHICS_MODE).Selection().Data(); i < index; ++i)
				++settings.mode;
		}

		return TRUE;
	}

	ibool Video::OnCmdTexMem(Param& param)
	{
		NST_COMPILE_ASSERT
		(
     		IDC_GRAPHICS_NESTEXTURE_VIDMEM - IDC_GRAPHICS_NESTEXTURE_AUTO == 1 &&
			IDC_GRAPHICS_NESTEXTURE_SYSMEM - IDC_GRAPHICS_NESTEXTURE_AUTO == 2
		);

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

			const Nes::Result result = nes.GetPalette().SetMode
			( 
				cmd == IDC_GRAPHICS_PALETTE_EMULATED ? Nes::Video::Palette::EMULATED :
				cmd == IDC_GRAPHICS_PALETTE_INTERNAL ? Nes::Video::Palette::INTERNAL :
				                                       Nes::Video::Palette::CUSTOM
			);

			if (NES_SUCCEEDED(result) && result != Nes::RESULT_NOP)
			{
				UpdatePalette();
				UpdateScreen( param.hWnd );
			}
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

	ibool Video::OnCmdPalBrowse(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const Managers::Paths::TmpPath file
			(
				paths.BrowseLoad
				( 
					Managers::Paths::File::PALETTE|Managers::Paths::File::ARCHIVE,
					settings.palette
				)
			);

			if (file.Size())
			{
				settings.palette = file;
				
				ImportPalette( Managers::Paths::NOISY );
				UpdatePalette();
			}
		}

		return TRUE;
	}

	ibool Video::OnCmdPalClear(Param& param)
	{
		if (param.Button().IsClicked())
		{
			if (settings.palette.Size())
			{
				settings.palette.Destroy();

				nes.GetPalette().SetMode( Nes::Video::Palette::INTERNAL );

				UpdatePalette();
				UpdateScreen( param.hWnd );
			}
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
			ResetDevice();
			UpdateDevice();

			ResetColors();
			UpdateColors();

			dialog.ComboBox( IDC_GRAPHICS_EFFECTS )[0].Select();

			ResetRects();
			UpdateRects();

			settings.texMem = Settings::TEXMEM_AUTO;
			UpdateTexMem();

			nes.GetPalette().SetMode( Nes::Video::Palette::INTERNAL );
			UpdatePalette();

			settings.autoHz = FALSE;
			dialog.CheckBox( IDC_GRAPHICS_AUTO_HZ ).Uncheck();

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

	void Video::UpdateColors() const
	{
		const uint colors[] =
		{
			nes.GetBrightness(),
			nes.GetSaturation(),
			nes.GetHue()       
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
		dialog.Control( IDC_GRAPHICS_NTSC_LEFT   ).Text() << settings.rects.ntsc.left;
		dialog.Control( IDC_GRAPHICS_NTSC_TOP    ).Text() << settings.rects.ntsc.top;
		dialog.Control( IDC_GRAPHICS_NTSC_RIGHT  ).Text() << settings.rects.ntsc.right - 1;
		dialog.Control( IDC_GRAPHICS_NTSC_BOTTOM ).Text() << settings.rects.ntsc.bottom - 1;
		dialog.Control( IDC_GRAPHICS_PAL_LEFT    ).Text() << settings.rects.pal.left;
		dialog.Control( IDC_GRAPHICS_PAL_TOP     ).Text() << settings.rects.pal.top;
		dialog.Control( IDC_GRAPHICS_PAL_RIGHT   ).Text() << settings.rects.pal.right - 1;
		dialog.Control( IDC_GRAPHICS_PAL_BOTTOM  ).Text() << settings.rects.pal.bottom - 1;
	}

	void Video::UpdatePalette() const
	{
		dialog.Control( IDC_GRAPHICS_PALETTE_CUSTOM ).Enable( settings.palette.Size() );
		dialog.Edit( IDC_GRAPHICS_PALETTE_PATH ) << settings.palette;

		const Nes::Video::Palette::Mode mode = nes.GetPalette().GetMode();

		dialog.RadioButton( IDC_GRAPHICS_PALETTE_EMULATED ).Check( mode == Nes::Video::Palette::EMULATED );
		dialog.RadioButton( IDC_GRAPHICS_PALETTE_INTERNAL ).Check( mode == Nes::Video::Palette::INTERNAL );
		dialog.RadioButton( IDC_GRAPHICS_PALETTE_CUSTOM   ).Check( mode == Nes::Video::Palette::CUSTOM   );
	}

	void Video::ImportPalette(Managers::Paths::Alert alert)
	{
		NST_ASSERT( settings.palette.Size() );

		Managers::Paths::File file;

		enum 
		{
			FILE_TYPES =
			(
		    	Managers::Paths::File::PALETTE |
		    	Managers::Paths::File::ARCHIVE
			)
		};

		if
		(
			!paths.Load( file, FILE_TYPES, settings.palette, alert ) || file.data.Size() < 64 * 3 ||
			NES_FAILED(nes.GetPalette().SetCustom( reinterpret_cast<Nes::Video::Palette::Colors>(file.data.Begin()) ))
		)
		{
			if (alert == Managers::Paths::QUIETLY)
				Io::Log() << "Video: warning, custom palette file: \"" << settings.palette << "\" invalid or not found!\r\n";
			else
				Window::User::Fail( IDS_DIALOG_VIDEO_INVALID_PALETTE );

			settings.palette.Destroy();
			nes.GetPalette().SetMode( Nes::Video::Palette::INTERNAL );
		}
	}

	void Video::UpdateBitDepth(const uint bpp)
	{
		dialog.RadioButton( IDC_GRAPHICS_8_BIT   ).Check( bpp == 8  );
		dialog.RadioButton( IDC_GRAPHICS_16_BIT  ).Check( bpp == 16 );
		dialog.RadioButton( IDC_GRAPHICS_32_BIT  ).Check( bpp == 32 );

		if (settings.mode->bpp != bpp)
		{
			for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it)
			{
				if (it->bpp == bpp && settings.mode->width == it->width && settings.mode->height == it->height)
				{
					settings.mode = it;
					break;
				}
			}

			if (settings.mode->bpp != bpp)
			{
				for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it)
				{
					if (it->bpp == bpp)
					{
						settings.mode = it;
						break;
					}
				}				
			}
		}

		UpdateResolution();		
	}

	void Video::UpdateTexMem() const
	{
		dialog.RadioButton( IDC_GRAPHICS_NESTEXTURE_AUTO   ).Check( settings.texMem == Settings::TEXMEM_AUTO   );
		dialog.RadioButton( IDC_GRAPHICS_NESTEXTURE_VIDMEM ).Check( settings.texMem == Settings::TEXMEM_VIDMEM );
		dialog.RadioButton( IDC_GRAPHICS_NESTEXTURE_SYSMEM ).Check( settings.texMem == Settings::TEXMEM_SYSMEM );
	}

	void Video::UpdateResolution() const
	{
		const Control::ComboBox comboBox( dialog.ComboBox(IDC_GRAPHICS_MODE) );
		comboBox.Clear();

		uint idx=0;

		for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it, ++idx)
		{
			if (settings.mode->bpp == it->bpp)
			{
				comboBox.Add( String::Smart<16>() << it->width << 'x' << it->height ).Data() = idx;

				if (settings.mode->width == it->width && settings.mode->height == it->height)
					comboBox[comboBox.Size() - 1].Select();
			}
		}
	}

	void Video::UpdateDevice()
	{
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

		uint bpp = settings.mode->bpp;

		switch (bpp)
		{
			case 32: bpp = ((available & 32) ? 32 : (available & 16) ? 16 :  8); break;
			case 16: bpp = ((available & 16) ? 16 : (available & 32) ? 32 :  8); break;
			case  8: bpp = ((available &  8) ?  8 : (available & 16) ? 16 : 32); break;
		}

		UpdateBitDepth( bpp );
		UpdateFilters();
		UpdateTexMemEnable();
	}

	void Video::UpdateTexMemEnable() const
	{
		dialog.Slider( IDC_GRAPHICS_NESTEXTURE_AUTO   ).Enable( settings.adapter->videoMemScreen );
		dialog.Slider( IDC_GRAPHICS_NESTEXTURE_VIDMEM ).Enable( settings.adapter->videoMemScreen );
		dialog.Slider( IDC_GRAPHICS_NESTEXTURE_SYSMEM ).Enable( settings.adapter->videoMemScreen );
	}

	void Video::UpdateFilters() const
	{
		const Control::ComboBox comboBox( dialog.ComboBox(IDC_GRAPHICS_EFFECTS) );

		comboBox.Clear();
		comboBox.Add( Resource::String(IDS_VIDEO_FILTER_NONE) ).Data() = (Control::ComboBox::Value) FILTER_NONE;

		if (settings.adapter->filters & Adapter::FILTER_BILINEAR)		
			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_BILINEAR) ).Data() = FILTER_BILINEAR;

		comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SCANLINES_BRIGHT) ).Data() = FILTER_SCANLINES_BRIGHT;
		comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SCANLINES_DARK) ).Data() = FILTER_SCANLINES_DARK;

		if (settings.adapter->maxScreenSize >= NST_MAX(NES_WIDTH*2,NES_HEIGHT*2))
		{
			if (settings.adapter->filters & Adapter::FILTER_BILINEAR)		
				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_TV_SOFT) ).Data() = FILTER_TV_SOFT;

			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_TV_HARSH) ).Data() = FILTER_TV_HARSH;
			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_2XSAI) ).Data() = FILTER_2XSAI;
			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SUPER_2XSAI) ).Data() = FILTER_SUPER_2XSAI;
			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SUPER_EAGLE) ).Data() = FILTER_SUPER_EAGLE;
			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SCALE2X) ).Data() = FILTER_SCALE2X;

			if (settings.adapter->maxScreenSize >= NST_MAX(NES_WIDTH*3,NES_HEIGHT*3))
				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SCALE3X) ).Data() = FILTER_SCALE3X;

			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_HQ2X) ).Data() = FILTER_HQ2X;

			if (settings.adapter->maxScreenSize >= NST_MAX(NES_WIDTH*3,NES_HEIGHT*3))
				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_HQ3X) ).Data() = FILTER_HQ3X;
		}
	}

	void Video::UpdateScreen(HWND const hDlg)
	{
		::RedrawWindow( ::GetParent( hDlg ), NULL, NULL, RDW_INVALIDATE );
	}
}
