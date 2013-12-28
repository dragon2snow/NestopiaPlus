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

#include "NstApplicationConfiguration.hpp"
#include "NstDialogVideoFilters.hpp"

namespace Nestopia
{
	using namespace Window;

	struct VideoFilters::Handlers
	{
		static const MsgHandler::Entry<VideoFilters> messages[];
		static const MsgHandler::Entry<VideoFilters> commands[];
	};

	const MsgHandler::Entry<VideoFilters> VideoFilters::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &VideoFilters::OnInitDialog },
		{ WM_HSCROLL,    &VideoFilters::OnHScroll    }
	};

	const MsgHandler::Entry<VideoFilters> VideoFilters::Handlers::commands[] =
	{
		{ IDC_VIDEO_FILTER_OK,                    &VideoFilters::OnCmdOk         },
		{ IDC_VIDEO_FILTER_DEFAULT,               &VideoFilters::OnCmdDefault    },
		{ IDC_VIDEO_FILTER_CANCEL,                &VideoFilters::OnCmdCancel     },
		{ IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO,      &VideoFilters::OnCmdNtscFields },
		{ IDC_VIDEO_FILTER_NTSC_FIELDS_ON,        &VideoFilters::OnCmdNtscFields },
		{ IDC_VIDEO_FILTER_NTSC_FIELDS_OFF,       &VideoFilters::OnCmdNtscFields }
	};

	VideoFilters::VideoFilters(uint idd,Settings& s,uint m,bool b)
	: 
	settings      (s), 
	maxScreenSize (m), 
	canDoBilinear (b), 
	dialog        (idd,this,Handlers::messages,Handlers::commands)
	{
		dialog.Open();
	}

	VideoFilters::Type VideoFilters::Load(const Configuration& cfg,Settings (&settings)[NUM_TYPES],const uint maxScreenSize,const bool bilinear)
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

				if (ATR_SCANLINES_MAX >= (value = cfg["video filter scanlines"].Default( 25 ))) 
					settings[TYPE_SCANLINES].attributes[ATR_SCANLINES] = (i8) value;

				if (ATR_SCANLINES_MAX >= (value = cfg["video filter ntsc scanlines"].Default( 0 ))) 
					settings[TYPE_NTSC].attributes[ATR_SCANLINES] = (i8) value;
			}

			GenericString value;

			settings[TYPE_NTSC].attributes[ATR_FIELDMERGING] = (i8) 
			(
	           	(value=cfg["video filter ntsc fieldmerging"]) == _T("yes") ? ATR_FIELDMERGING_ON : 
				(value)                                       == _T("no")  ? ATR_FIELDMERGING_OFF : 
				                                                             ATR_FIELDMERGING_AUTO
			);

			{
				int value;

				if (ATR_CONTRAST_MAX-ATR_CONTRAST_MIN >= (value = cfg["video filter ntsc contrast"].Default( (ATR_CONTRAST_MAX-ATR_CONTRAST_MIN)/2 ))) 
					settings[TYPE_NTSC].attributes[ATR_CONTRAST] = (i8) (value + ATR_CONTRAST_MIN);

				if (ATR_SHARPNESS_MAX-ATR_SHARPNESS_MIN >= (value = cfg["video filter ntsc sharpness"].Default( (ATR_SHARPNESS_MAX-ATR_SHARPNESS_MIN)/2 ))) 
					settings[TYPE_NTSC].attributes[ATR_SHARPNESS] = (i8) (value + ATR_SHARPNESS_MIN);
			}

			settings[TYPE_NTSC].attributes[ATR_NO_WIDESCREEN] = (i8) 
			(
	         	cfg["video filter ntsc widescreen"] == Configuration::NO
			);
		
			settings[TYPE_2XSAI].attributes[ATR_TYPE] = (i8) 
			(
		     	(value=cfg["video filter 2xsai type"]) == _T("super 2xsai") ? ATR_SUPER2XSAI : 
				(value)                                == _T("super eagle") ? ATR_SUPEREAGLE :
				                                                              ATR_2XSAI
			);
			
			settings[TYPE_SCALEX].attributes[ATR_TYPE] = (i8) 
			(
       			(value=cfg["video filter scalex scale"]) == _T("3") ? ATR_SCALE3X : ATR_SCALE2X
			);

			settings[TYPE_HQX].attributes[ATR_TYPE] = (i8) 
			(
     			(value=cfg["video filter hqx scale"]) == _T("3") ? ATR_HQ3X : ATR_HQ2X
			);
		}

		return type;
	}

	void VideoFilters::Save(Configuration& cfg,const Settings (&settings)[NUM_TYPES],const Type type)
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

		cfg[ "video filter ntsc contrast" ] = (uint) (settings[TYPE_NTSC].attributes[ATR_CONTRAST] - ATR_CONTRAST_MIN);
		cfg[ "video filter ntsc sharpness" ] = (uint) (settings[TYPE_NTSC].attributes[ATR_SHARPNESS] - ATR_SHARPNESS_MIN);

		cfg["video filter ntsc widescreen"].YesNo() =
		(
     		!settings[TYPE_NTSC].attributes[ATR_NO_WIDESCREEN]
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

				dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_WIDESCREEN ).Check( !settings.attributes[ATR_NO_WIDESCREEN] );

				dialog.Slider( IDC_VIDEO_FILTER_NTSC_CONTRAST ).SetRange( 0, ATR_CONTRAST_MAX-ATR_CONTRAST_MIN );
				dialog.Slider( IDC_VIDEO_FILTER_NTSC_SHARPNESS ).SetRange( 0, ATR_SHARPNESS_MAX-ATR_SHARPNESS_MIN );

				dialog.Slider( IDC_VIDEO_FILTER_NTSC_CONTRAST ).Position() = (uint) (settings.attributes[ATR_CONTRAST] - ATR_CONTRAST_MIN);
				dialog.Slider( IDC_VIDEO_FILTER_NTSC_SHARPNESS ).Position() = (uint) (settings.attributes[ATR_SHARPNESS] - ATR_SHARPNESS_MIN);

			case IDD_VIDEO_FILTER_SCANLINES:

				dialog.Slider( IDC_VIDEO_FILTER_SCANLINES_SLIDER ).SetRange( 0, ATR_SCANLINES_MAX );
				dialog.Slider( IDC_VIDEO_FILTER_SCANLINES_SLIDER ).Position() = (uint) settings.attributes[ATR_SCANLINES];
				dialog.Edit( IDC_VIDEO_FILTER_SCANLINES_VAL ) << (uint) settings.attributes[ATR_SCANLINES];
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

	ibool VideoFilters::OnHScroll(Param& param)
	{
		if (param.Slider().IsControl( IDC_VIDEO_FILTER_SCANLINES_SLIDER ))
			dialog.Edit( IDC_VIDEO_FILTER_SCANLINES_VAL ) << (uint) param.Slider().Scroll();

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

	ibool VideoFilters::OnCmdDefault(Param& param)
	{
		if (param.Button().IsClicked())
		{
			dialog.CheckBox(IDC_VIDEO_FILTER_BILINEAR).Uncheck();

			switch (uint id = dialog.GetId())
			{
				case IDD_VIDEO_FILTER_NTSC:

					dialog.RadioButton(IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO).Check();
					dialog.RadioButton(IDC_VIDEO_FILTER_NTSC_FIELDS_ON).Uncheck();
					dialog.RadioButton(IDC_VIDEO_FILTER_NTSC_FIELDS_OFF).Uncheck();
					dialog.Slider(IDC_VIDEO_FILTER_NTSC_CONTRAST).Position() = (ATR_CONTRAST_MAX-ATR_CONTRAST_MIN) / 2U;
					dialog.Slider(IDC_VIDEO_FILTER_NTSC_SHARPNESS).Position() = (ATR_SHARPNESS_MAX-ATR_SHARPNESS_MIN) / 2U;
					dialog.CheckBox(IDC_VIDEO_FILTER_NTSC_WIDESCREEN).Check();

				case IDD_VIDEO_FILTER_SCANLINES:

					id = (id == IDD_VIDEO_FILTER_SCANLINES ? 25 : 0);
					dialog.Slider( IDC_VIDEO_FILTER_SCANLINES_SLIDER ).Position() = id;
					dialog.Edit( IDC_VIDEO_FILTER_SCANLINES_VAL ) << id;
					break;

				case IDD_VIDEO_FILTER_2XSAI:

					dialog.RadioButton(IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI).Check();
					break;

				case IDD_VIDEO_FILTER_SCALEX:

					dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_2X).Check();
					dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_3X).Uncheck();
					break;

				case IDD_VIDEO_FILTER_HQX:

					dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_2X).Check();
					dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_3X).Uncheck();
					break;
			}
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
			settings.attributes[ATR_BILINEAR] = (bool) dialog.CheckBox(IDC_VIDEO_FILTER_BILINEAR).IsChecked();

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

					settings.attributes[ATR_CONTRAST] = (i8) (ATR_CONTRAST_MIN + (int) dialog.Slider(IDC_VIDEO_FILTER_NTSC_CONTRAST).Position());
					settings.attributes[ATR_SHARPNESS] = (i8) (ATR_SHARPNESS_MIN + (int) dialog.Slider(IDC_VIDEO_FILTER_NTSC_SHARPNESS).Position());					
					settings.attributes[ATR_NO_WIDESCREEN] = (bool) dialog.CheckBox(IDC_VIDEO_FILTER_NTSC_WIDESCREEN).IsUnchecked();

				case IDD_VIDEO_FILTER_SCANLINES:

					settings.attributes[ATR_SCANLINES] = (i8) dialog.Slider(IDC_VIDEO_FILTER_SCANLINES_SLIDER).Position();
					break;

				case IDD_VIDEO_FILTER_2XSAI:

					if (dialog.RadioButton(IDC_VIDEO_FILTER_2XSAI_TYPE_SUPER2XSAI).IsChecked())
					{
						settings.attributes[ATR_TYPE] = ATR_SUPER2XSAI;
					}
					else if (dialog.RadioButton(IDC_VIDEO_FILTER_2XSAI_TYPE_SUPEREAGLE).IsChecked())
					{
						settings.attributes[ATR_TYPE] = ATR_SUPEREAGLE;
					}
					else
					{
						settings.attributes[ATR_TYPE] = TYPE_2XSAI;
					}
					break;

				case IDD_VIDEO_FILTER_SCALEX:

					if (dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_3X).IsChecked())
						settings.attributes[ATR_TYPE] = ATR_SCALE3X;
					else
						settings.attributes[ATR_TYPE] = ATR_SCALE2X;
					break;

				case IDD_VIDEO_FILTER_HQX:

					if (dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_3X).IsChecked())
						settings.attributes[ATR_TYPE] = ATR_HQ3X;
					else
						settings.attributes[ATR_TYPE] = ATR_HQ2X;
					break;
			}

			dialog.Close();
		}

		return TRUE;
	}
}
