////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
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

#include "NstApplicationInstance.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstDialogVideoFilters.hpp"

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		IDC_VIDEO_FILTER_NTSC_RESOLUTION_SLIDER == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 1 &&
		IDC_VIDEO_FILTER_NTSC_COLORBLEED_SLIDER == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 2 &&
		IDC_VIDEO_FILTER_NTSC_ARTIFACTS_SLIDER	== IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 3 &&
		IDC_VIDEO_FILTER_NTSC_FRINGING_SLIDER   == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 4
	);

	using namespace Window;

	struct VideoFilters::Handlers
	{
		static const MsgHandler::Entry<VideoFilters> messages[];
		static const MsgHandler::Entry<VideoFilters> commands[];
	};

	const MsgHandler::Entry<VideoFilters> VideoFilters::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &VideoFilters::OnInitDialog },
		{ WM_HSCROLL,    &VideoFilters::OnHScroll    },
		{ WM_DESTROY,    &VideoFilters::OnDestroy    }
	};

	const MsgHandler::Entry<VideoFilters> VideoFilters::Handlers::commands[] =
	{
		{ IDC_VIDEO_FILTER_OK,                    &VideoFilters::OnCmdOk         },
		{ IDC_VIDEO_FILTER_DEFAULT,               &VideoFilters::OnCmdDefault    },
		{ IDC_VIDEO_FILTER_CANCEL,                &VideoFilters::OnCmdCancel     },
		{ IDC_VIDEO_FILTER_BILINEAR,              &VideoFilters::OnCmdBilinear   },
		{ IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO,      &VideoFilters::OnCmdNtscFields },
		{ IDC_VIDEO_FILTER_NTSC_FIELDS_ON,        &VideoFilters::OnCmdNtscFields },
		{ IDC_VIDEO_FILTER_NTSC_FIELDS_OFF,       &VideoFilters::OnCmdNtscFields },
		{ IDC_VIDEO_FILTER_NTSC_COMPOSITE,        &VideoFilters::OnCmdNtscCable  },
		{ IDC_VIDEO_FILTER_NTSC_SVIDEO,           &VideoFilters::OnCmdNtscCable  },
		{ IDC_VIDEO_FILTER_NTSC_RGB,              &VideoFilters::OnCmdNtscCable  },
		{ IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI,	  &VideoFilters::OnCmd2xSaI      },
		{ IDC_VIDEO_FILTER_2XSAI_TYPE_SUPER2XSAI, &VideoFilters::OnCmd2xSaI      },
		{ IDC_VIDEO_FILTER_2XSAI_TYPE_SUPEREAGLE, &VideoFilters::OnCmd2xSaI      },
		{ IDC_VIDEO_FILTER_SCALEX_2X,             &VideoFilters::OnCmdScaleX     },
		{ IDC_VIDEO_FILTER_SCALEX_3X,             &VideoFilters::OnCmdScaleX     },
		{ IDC_VIDEO_FILTER_HQX_SCALING_2X,        &VideoFilters::OnCmdHqX        },
		{ IDC_VIDEO_FILTER_HQX_SCALING_3X,        &VideoFilters::OnCmdHqX        }
	};

	VideoFilters::Backup::Backup(const Settings& s,const Nes::Video nes)
	:
	settings   ( s ),
	sharpness  ( nes.GetSharpness() ),
	resolution ( nes.GetColorResolution() ),
	bleed      ( nes.GetColorBleed() ),
	artifacts  ( nes.GetColorArtifacts() ),
	fringing   ( nes.GetColorFringing() ),
	restore    ( true )
	{}

	VideoFilters::VideoFilters(Nes::Video n,uint idd,Settings& s,uint m,bool b)
	: 
	settings      ( s ),
	backup        ( s, n ),
	maxScreenSize ( m ), 
	canDoBilinear ( b ),
	nes           ( n ),
	dialog        ( idd, this,Handlers::messages,Handlers::commands )
	{
		dialog.Open();
	}

	void VideoFilters::RedrawWindow()
	{
		::RedrawWindow( Application::Instance::GetMainWindow(), NULL, NULL, RDW_INVALIDATE );
	}

	VideoFilters::Type VideoFilters::Load(const Configuration& cfg,Settings (&settings)[NUM_TYPES],Nes::Video nes,const uint maxScreenSize,const bool bilinear)
	{
		Type type = TYPE_NONE;

		if (maxScreenSize >= MAX_2X_SIZE)
		{
			const GenericString string( cfg["video filter"] );

			if (string == _T("scanlines"))
			{
				type = TYPE_SCANLINES;
			}
			else if (string == _T("ntsc"))
			{
				if (maxScreenSize >= MAX_NTSC_SIZE)
					type = TYPE_NTSC;
			}
			else if (string == _T("2xsai"))
			{
				type = TYPE_2XSAI;
			}
			else if (string == _T("scalex"))
			{
				type = TYPE_SCALEX;
			}
			else if (string == _T("hqx"))
			{
				type = TYPE_HQX;
			}
		}

		if (bilinear)
		{
			for (uint i=0; i < NUM_TYPES; ++i)
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
					settings[i].attributes[ATR_BILINEAR] = true;
			}
		}

		{
			{
				uint value;

				if (100 >= (value=cfg["video filter scanlines"].Default( 25 ))) 
					settings[TYPE_SCANLINES].attributes[ATR_SCANLINES] = value;

				if (100 >= (value=cfg["video filter ntsc scanlines"].Default( 0 ))) 
					settings[TYPE_NTSC].attributes[ATR_SCANLINES] = value;
			}

			GenericString value;

			settings[TYPE_NTSC].attributes[ATR_FIELDMERGING] = 
			(
	           	(value=cfg["video filter ntsc fieldmerging"]) == _T("yes") ? ATR_FIELDMERGING_ON : 
				(value)                                       == _T("no")  ? ATR_FIELDMERGING_OFF : 
				                                                             ATR_FIELDMERGING_AUTO
			);

			{
				uint value;
				
				if (200 >= (value=cfg["video filter ntsc sharpness"].Default( 100 )))
					nes.SetSharpness( int(value) - 100 );

				if (200 >= (value=cfg["video filter ntsc resolution"].Default( 100 )))
					 nes.SetColorResolution( int(value) - 100 ); 

				if (200 >= (value=cfg["video filter ntsc colorbleed"].Default( 100 )))
					 nes.SetColorBleed( int(value) - 100 ); 

				if (200 >= (value=cfg["video filter ntsc artifacts"].Default( 100 )))
					 nes.SetColorArtifacts( int(value) - 100 ); 

				if (200 >= (value=cfg["video filter ntsc fringing"].Default( 100 )))
					nes.SetColorFringing( int(value) - 100 ); 
			}

			settings[TYPE_NTSC].attributes[ATR_RESCALE_PIC] = 
			(
	         	cfg["video filter ntsc tv aspect"] == Configuration::NO
			);
		
			settings[TYPE_2XSAI].attributes[ATR_TYPE] = 
			(
		     	(value=cfg["video filter 2xsai type"]) == _T("super 2xsai") ? ATR_SUPER2XSAI : 
				(value)                                == _T("super eagle") ? ATR_SUPEREAGLE :
				                                                              ATR_2XSAI
			);
			
			settings[TYPE_SCALEX].attributes[ATR_TYPE] =
			(
       			(value=cfg["video filter scalex scale"]) == _T("3") ? ATR_SCALE3X : ATR_SCALE2X
			);

			settings[TYPE_HQX].attributes[ATR_TYPE] = 
			(
     			(value=cfg["video filter hqx scale"]) == _T("3") ? ATR_HQ3X : ATR_HQ2X
			);
		}

		return type;
	}

	void VideoFilters::Save(Configuration& cfg,const Settings (&settings)[NUM_TYPES],Nes::Video nes,const Type type)
	{
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

			cfg[ "video filter" ] = names[type];
		}

		for (uint i=0; i < NUM_TYPES; ++i)
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

			cfg[lut[i]].YesNo() = settings[i].attributes[ATR_BILINEAR];
		}

		cfg["video filter scanlines"] = (uint) settings[TYPE_SCANLINES].attributes[ATR_SCANLINES];
		cfg["video filter ntsc scanlines"] = (uint) settings[TYPE_NTSC].attributes[ATR_SCANLINES];

		cfg["video filter ntsc fieldmerging"] =
		(
     		settings[TYPE_NTSC].attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_ON  ? _T("yes") :
			settings[TYPE_NTSC].attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_OFF ? _T("no") :
			                                                                           _T("auto")
		);

		cfg[ "video filter ntsc sharpness"  ] = (uint) (nes.GetSharpness()       + 100);
		cfg[ "video filter ntsc resolution" ] = (uint) (nes.GetColorResolution() + 100);
		cfg[ "video filter ntsc colorbleed" ] = (uint) (nes.GetColorBleed()      + 100);
		cfg[ "video filter ntsc artifacts"  ] = (uint) (nes.GetColorArtifacts()  + 100);
		cfg[ "video filter ntsc fringing"   ] = (uint) (nes.GetColorFringing()   + 100);

		cfg["video filter ntsc tv aspect"].YesNo() =
		(
     		!settings[TYPE_NTSC].attributes[ATR_RESCALE_PIC]
		);

		cfg["video filter 2xsai type"] =
		(
       		settings[TYPE_2XSAI].attributes[ATR_TYPE] == ATR_SUPER2XSAI ? _T("super 2xsai")	:
			settings[TYPE_2XSAI].attributes[ATR_TYPE] == ATR_SUPEREAGLE ? _T("super eagle") :
	                										      		  _T("2xsai")
		);

		cfg["video filter scalex scale"] =
		(
     		settings[TYPE_SCALEX].attributes[ATR_TYPE] == ATR_SCALE3X ? _T("3") : _T("2") 
		);

		cfg["video filter hqx scale"] =
		(
     		settings[TYPE_HQX].attributes[ATR_TYPE] == ATR_HQ3X ? _T("3") : _T("2")
		);
	}

	ibool VideoFilters::OnInitDialog(Param&)
	{
		dialog.CheckBox(IDC_VIDEO_FILTER_BILINEAR).Check( settings.attributes[ATR_BILINEAR] );

		if (!canDoBilinear)
			dialog.CheckBox(IDC_VIDEO_FILTER_BILINEAR).Disable();

		uint idc;

		switch (dialog.GetId())
		{
			case IDD_VIDEO_FILTER_NTSC:

				dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO ).Check( settings.attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_AUTO );
				dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_ON   ).Check( settings.attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_ON   );
				dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_OFF  ).Check( settings.attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_OFF  );

				dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_TV_ASPECT ).Check( !settings.attributes[ATR_RESCALE_PIC] );

				for (uint i=IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER; i <= IDC_VIDEO_FILTER_NTSC_FRINGING_SLIDER; ++i)
					dialog.Slider( i ).SetRange( 0, 200 );

				UpdateNtscSliders();

			case IDD_VIDEO_FILTER_SCANLINES:

				dialog.Slider( IDC_VIDEO_FILTER_SCANLINES_SLIDER ).SetRange( 0, 100 );

				UpdateScanlinesSlider();
				break;

			case IDD_VIDEO_FILTER_2XSAI:

				switch (settings.attributes[ATR_TYPE])
				{
					case ATR_SUPER2XSAI: idc = IDC_VIDEO_FILTER_2XSAI_TYPE_SUPER2XSAI; break;
					case ATR_SUPEREAGLE: idc = IDC_VIDEO_FILTER_2XSAI_TYPE_SUPEREAGLE; break;
					default:             idc = IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI;      break;
				}

				dialog.RadioButton(idc).Check();
				break;

			case IDD_VIDEO_FILTER_SCALEX:

				if (settings.attributes[ATR_TYPE] == ATR_SCALE3X)
					idc = IDC_VIDEO_FILTER_SCALEX_3X;
				else
					idc = IDC_VIDEO_FILTER_SCALEX_2X;

				dialog.RadioButton(idc).Check();

				if (maxScreenSize < MAX_3X_SIZE)
				{
					dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_2X).Disable();
					dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_3X).Disable();
				}
				break;

			case IDD_VIDEO_FILTER_HQX:

				if (settings.attributes[ATR_TYPE] == ATR_HQ3X)
					idc = IDC_VIDEO_FILTER_HQX_SCALING_3X;
				else
					idc = IDC_VIDEO_FILTER_HQX_SCALING_2X;

				dialog.RadioButton(idc).Check();

				if (maxScreenSize < MAX_3X_SIZE)
				{
					dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_2X).Disable();
					dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_3X).Disable();
				}
				break;
		}

		return TRUE;
	}

	ibool VideoFilters::OnDestroy(Param&)
	{
		if (backup.restore)
		{
			settings = backup.settings;

			nes.SetSharpness	   ( backup.sharpness  );
			nes.SetColorResolution ( backup.resolution );
			nes.SetColorBleed	   ( backup.bleed      );
			nes.SetColorArtifacts  ( backup.artifacts  );
			nes.SetColorFringing   ( backup.fringing   );
		}

		return TRUE;
	}

	void VideoFilters::UpdateScanlinesSlider() const
	{
		dialog.Slider( IDC_VIDEO_FILTER_SCANLINES_SLIDER ).Position() = (uint) settings.attributes[ATR_SCANLINES];
		dialog.Edit( IDC_VIDEO_FILTER_SCANLINES_VAL ) << (uint) settings.attributes[ATR_SCANLINES];

		RedrawWindow();
	}

	void VideoFilters::UpdateNtscSliders() const
	{
		const u8 values[] =
		{
			100 + nes.GetSharpness(),
			100 + nes.GetColorResolution(),
			100 + nes.GetColorBleed(),
			100 + nes.GetColorArtifacts(),
			100 + nes.GetColorFringing()
		};

		for (uint i=0; i < 5; ++i)
		{
			dialog.Slider( IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER+i  ).Position() = values[i];
			UpdateNtscSlider( values[i], IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL+i );
		}
	}

	void VideoFilters::UpdateNtscSlider(int value,const uint idc) const
	{
		tchar string[7];
		tchar* ptr = string;

		if (value < 0)
		{
			value = -value;
			*ptr++ = '-';
		}

		*ptr++ = '0' + (value / 100);
		*ptr++ = '.';
		*ptr++ = '0' + (value % 100 / 10);

		if (value % 100)
			*ptr++ = '0' + (value % 10);

		*ptr++ = '\0';

		dialog.Edit( idc ).Text() << string;
		
		RedrawWindow();
	}

	ibool VideoFilters::OnHScroll(Param& param)
	{
		int value = param.Slider().Scroll(); 

		if (param.Slider().IsControl( IDC_VIDEO_FILTER_SCANLINES_SLIDER ))
		{
			if (settings.attributes[ATR_SCANLINES] != value)
			{
				settings.attributes[ATR_SCANLINES] = value;
				UpdateScanlinesSlider();
			}
		}
		else if (param.Slider().IsControl( IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER ))
		{
			if (nes.SetSharpness( value-100 ) != Nes::RESULT_NOP)
				UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL );
		}
		else if (param.Slider().IsControl( IDC_VIDEO_FILTER_NTSC_RESOLUTION_SLIDER ))
		{
			if (nes.SetColorResolution( value-100 ) != Nes::RESULT_NOP)
     			UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_RESOLUTION_VAL );
		}
		else if (param.Slider().IsControl( IDC_VIDEO_FILTER_NTSC_COLORBLEED_SLIDER ))
		{
			if (nes.SetColorBleed( value-100 ) != Nes::RESULT_NOP)
     			UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_COLORBLEED_VAL );
		}
		else if (param.Slider().IsControl( IDC_VIDEO_FILTER_NTSC_ARTIFACTS_SLIDER ))
		{
			if (nes.SetColorArtifacts( value-100 ) != Nes::RESULT_NOP)
     			UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_ARTIFACTS_VAL );
		}
		else if (param.Slider().IsControl( IDC_VIDEO_FILTER_NTSC_FRINGING_SLIDER ))
		{
			if (nes.SetColorFringing( value-100 ) != Nes::RESULT_NOP)
     			UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_FRINGING_VAL );
		}

		return TRUE;
	}

	ibool VideoFilters::OnCmdBilinear(Param& param)
	{
		if (param.Button().IsClicked())
		{
			settings.attributes[ATR_BILINEAR] = (bool) dialog.CheckBox( IDC_VIDEO_FILTER_BILINEAR ).IsChecked();
			RedrawWindow();
		}

		return TRUE;
	}

	ibool VideoFilters::OnCmdNtscFields(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint id = param.Button().GetId();

			dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO ).Check( id == IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO );
			dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_ON   ).Check( id == IDC_VIDEO_FILTER_NTSC_FIELDS_ON   );
			dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_OFF  ).Check( id == IDC_VIDEO_FILTER_NTSC_FIELDS_OFF  );
		}

		return TRUE;
	}

	ibool VideoFilters::OnCmdNtscCable(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint id = param.Button().GetId();
			
			nes.SetSharpness	   (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_SHARPNESS_COMP        : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_SHARPNESS_SVIDEO        : Nes::Video::DEFAULT_SHARPNESS_RGB        );
			nes.SetColorResolution (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_RESOLUTION_COMP : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_RESOLUTION_SVIDEO : Nes::Video::DEFAULT_COLOR_RESOLUTION_RGB );
			nes.SetColorBleed	   (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_BLEED_COMP      : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_BLEED_SVIDEO      : Nes::Video::DEFAULT_COLOR_BLEED_RGB      );
			nes.SetColorArtifacts  (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_ARTIFACTS_COMP  : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_ARTIFACTS_SVIDEO  : Nes::Video::DEFAULT_COLOR_ARTIFACTS_RGB  );
			nes.SetColorFringing   (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_FRINGING_COMP   : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_FRINGING_SVIDEO   : Nes::Video::DEFAULT_COLOR_FRINGING_RGB   );

			UpdateNtscSliders();
		}

		return TRUE;
	}

	ibool VideoFilters::OnCmd2xSaI(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint id = param.Button().GetId();

			settings.attributes[ATR_TYPE] = 
			(
		     	id == IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI      ? ATR_2XSAI : 
				id == IDC_VIDEO_FILTER_2XSAI_TYPE_SUPER2XSAI ? ATR_SUPER2XSAI :
				                                               ATR_SUPEREAGLE
			);

			dialog.RadioButton( IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI      ).Check( id == IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI      );
			dialog.RadioButton( IDC_VIDEO_FILTER_2XSAI_TYPE_SUPER2XSAI ).Check( id == IDC_VIDEO_FILTER_2XSAI_TYPE_SUPER2XSAI );
			dialog.RadioButton( IDC_VIDEO_FILTER_2XSAI_TYPE_SUPEREAGLE ).Check( id == IDC_VIDEO_FILTER_2XSAI_TYPE_SUPEREAGLE );

			RedrawWindow();
		}

		return TRUE;
	}

	ibool VideoFilters::OnCmdScaleX(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint id = param.Button().GetId();

			settings.attributes[ATR_TYPE] = (id == IDC_VIDEO_FILTER_SCALEX_2X ? ATR_SCALE2X : ATR_SCALE3X);

			dialog.RadioButton( IDC_VIDEO_FILTER_SCALEX_2X ).Check( id == IDC_VIDEO_FILTER_SCALEX_2X );
			dialog.RadioButton( IDC_VIDEO_FILTER_SCALEX_3X ).Check( id == IDC_VIDEO_FILTER_SCALEX_3X );

			RedrawWindow();
		}

		return TRUE;
	}

	ibool VideoFilters::OnCmdHqX(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint id = param.Button().GetId();

			settings.attributes[ATR_TYPE] = (id == IDC_VIDEO_FILTER_HQX_SCALING_2X ? ATR_HQ2X : ATR_HQ3X);

			dialog.RadioButton( IDC_VIDEO_FILTER_HQX_SCALING_2X ).Check( id == IDC_VIDEO_FILTER_HQX_SCALING_2X );
			dialog.RadioButton( IDC_VIDEO_FILTER_HQX_SCALING_3X ).Check( id == IDC_VIDEO_FILTER_HQX_SCALING_3X );

			RedrawWindow();
		}

		return TRUE;
	}

	ibool VideoFilters::OnCmdDefault(Param& param)
	{
		if (param.Button().IsClicked())
		{
			settings.attributes[ATR_BILINEAR] = false;
			dialog.CheckBox( IDC_VIDEO_FILTER_BILINEAR ).Uncheck();

			switch (uint id = dialog.GetId())
			{
				case IDD_VIDEO_FILTER_NTSC:

					dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO ).Check();
					dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_ON ).Uncheck();
					dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_OFF ).Uncheck();
					
					dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_TV_ASPECT ).Check();
					
					nes.SetSharpness	   ( Nes::Video::DEFAULT_SHARPNESS_COMP        );
					nes.SetColorResolution ( Nes::Video::DEFAULT_COLOR_RESOLUTION_COMP );
					nes.SetColorBleed	   ( Nes::Video::DEFAULT_COLOR_BLEED_COMP      );
					nes.SetColorArtifacts  ( Nes::Video::DEFAULT_COLOR_ARTIFACTS_COMP  );
					nes.SetColorFringing   ( Nes::Video::DEFAULT_COLOR_FRINGING_COMP   );

					UpdateNtscSliders();

				case IDD_VIDEO_FILTER_SCANLINES:

					settings.attributes[ATR_SCANLINES] = (id == IDD_VIDEO_FILTER_SCANLINES ? 25 : 0);
					UpdateScanlinesSlider();
					break;

				case IDD_VIDEO_FILTER_2XSAI:

					settings.attributes[ATR_TYPE] = ATR_2XSAI;
					dialog.RadioButton(IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI).Check();
					break;

				case IDD_VIDEO_FILTER_SCALEX:

					settings.attributes[ATR_TYPE] = ATR_SCALE2X;
					dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_2X).Check();
					dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_3X).Uncheck();
					break;

				case IDD_VIDEO_FILTER_HQX:

					settings.attributes[ATR_TYPE] = ATR_HQ2X;
					dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_2X).Check();
					dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_3X).Uncheck();
					break;
			}

			RedrawWindow();
		}

		return TRUE;
	}

	ibool VideoFilters::OnCmdCancel(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}

	ibool VideoFilters::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			switch (dialog.GetId())
			{
				case IDD_VIDEO_FILTER_NTSC:

					if (dialog.RadioButton(IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO).IsChecked())
					{
						settings.attributes[ATR_FIELDMERGING] = ATR_FIELDMERGING_AUTO;
					}
					else if (dialog.RadioButton(IDC_VIDEO_FILTER_NTSC_FIELDS_ON).IsChecked())
					{
						settings.attributes[ATR_FIELDMERGING] = ATR_FIELDMERGING_ON;
					}
					else
					{
						settings.attributes[ATR_FIELDMERGING] = ATR_FIELDMERGING_OFF;
					}
										
					settings.attributes[ATR_RESCALE_PIC] = (bool) dialog.CheckBox(IDC_VIDEO_FILTER_NTSC_TV_ASPECT).IsUnchecked();
					break;
			}

			backup.restore = false;
			dialog.Close();
		}

		return TRUE;
	}
}
