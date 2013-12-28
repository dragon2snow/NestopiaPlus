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
#include "NstManagerEmulator.hpp"
#include "NstWindowParam.hpp"
#include "NstDialogVideoFilters.hpp"

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_FILTER_NTSC_RESOLUTION_SLIDER == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 1 &&
			IDC_VIDEO_FILTER_NTSC_COLORBLEED_SLIDER == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 2 &&
			IDC_VIDEO_FILTER_NTSC_ARTIFACTS_SLIDER  == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 3 &&
			IDC_VIDEO_FILTER_NTSC_FRINGING_SLIDER   == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 4 &&

			IDC_VIDEO_FILTER_NTSC_RESOLUTION_VAL == IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL + 1 &&
			IDC_VIDEO_FILTER_NTSC_COLORBLEED_VAL == IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL + 2 &&
			IDC_VIDEO_FILTER_NTSC_ARTIFACTS_VAL  == IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL + 3 &&
			IDC_VIDEO_FILTER_NTSC_FRINGING_VAL   == IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL + 4 &&

			IDC_VIDEO_FILTER_NTSC_RESOLUTION_TEXT == IDC_VIDEO_FILTER_NTSC_SHARPNESS_TEXT + 1 &&
			IDC_VIDEO_FILTER_NTSC_COLORBLEED_TEXT == IDC_VIDEO_FILTER_NTSC_SHARPNESS_TEXT + 2 &&
			IDC_VIDEO_FILTER_NTSC_ARTIFACTS_TEXT  == IDC_VIDEO_FILTER_NTSC_SHARPNESS_TEXT + 3 &&
			IDC_VIDEO_FILTER_NTSC_FRINGING_TEXT   == IDC_VIDEO_FILTER_NTSC_SHARPNESS_TEXT + 4 &&

			IDC_VIDEO_FILTER_NTSC_SVIDEO == IDC_VIDEO_FILTER_NTSC_COMPOSITE + 1 &&
			IDC_VIDEO_FILTER_NTSC_RGB    == IDC_VIDEO_FILTER_NTSC_COMPOSITE + 2
		);

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
			{ IDC_VIDEO_FILTER_BILINEAR,              &VideoFilters::OnCmdBilinear   },
			{ IDC_VIDEO_FILTER_NTSC_AUTO_TUNING,      &VideoFilters::OnCmdNtscTuning },
			{ IDC_VIDEO_FILTER_NTSC_COMPOSITE,        &VideoFilters::OnCmdNtscCable  },
			{ IDC_VIDEO_FILTER_NTSC_SVIDEO,           &VideoFilters::OnCmdNtscCable  },
			{ IDC_VIDEO_FILTER_NTSC_RGB,              &VideoFilters::OnCmdNtscCable  },
			{ IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI,      &VideoFilters::OnCmd2xSaI      },
			{ IDC_VIDEO_FILTER_2XSAI_TYPE_SUPER2XSAI, &VideoFilters::OnCmd2xSaI      },
			{ IDC_VIDEO_FILTER_2XSAI_TYPE_SUPEREAGLE, &VideoFilters::OnCmd2xSaI      },
			{ IDC_VIDEO_FILTER_SCALEX_AUTO,           &VideoFilters::OnCmdScaleX     },
			{ IDC_VIDEO_FILTER_SCALEX_2X,             &VideoFilters::OnCmdScaleX     },
			{ IDC_VIDEO_FILTER_SCALEX_3X,             &VideoFilters::OnCmdScaleX     },
			{ IDC_VIDEO_FILTER_HQX_SCALING_AUTO,      &VideoFilters::OnCmdHqX        },
			{ IDC_VIDEO_FILTER_HQX_SCALING_2X,        &VideoFilters::OnCmdHqX        },
			{ IDC_VIDEO_FILTER_HQX_SCALING_3X,        &VideoFilters::OnCmdHqX        },
			{ IDC_VIDEO_FILTER_HQX_SCALING_4X,        &VideoFilters::OnCmdHqX        },
			{ IDC_VIDEO_FILTER_DEFAULT,               &VideoFilters::OnCmdDefault    },
			{ IDOK,                                   &VideoFilters::OnCmdOk         }
		};

		void VideoFilters::Settings::Reset(Type type)
		{
			std::memset( attributes, 0, sizeof(attributes) );

			if (type == TYPE_SCANLINES)
				attributes[ATR_SCANLINES] = 25;
		}

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

		VideoFilters::VideoFilters
		(
			Nes::Video n,
			uint idd,Settings& s,
			uint m,
			bool b,
			Nes::Video::Palette::Mode p
		)
		:
		settings      ( s ),
		backup        ( s, n ),
		maxScreenSize ( m ),
		canDoBilinear ( b ),
		paletteMode   ( p ),
		nes           ( n ),
		dialog        ( idd, this,Handlers::messages,Handlers::commands )
		{
		}

		VideoFilters::Type VideoFilters::Load
		(
			const Configuration& cfg,
			Settings (&settings)[NUM_TYPES],
			Nes::Video nes,
			const uint maxScreenSize,
			const bool bilinear,
			const Nes::Video::Palette::Mode paletteMode
		)
		{
			for (uint i=0; i < NUM_TYPES; ++i)
				settings[i].Reset( (Type) i );

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

				settings[TYPE_NTSC].attributes[ATR_NO_AUTO_TUNING] =
				(
					cfg["video filter ntsc auto tuning"] == Configuration::NO
				);

				settings[TYPE_2XSAI].attributes[ATR_TYPE] =
				(
					(value=cfg["video filter 2xsai type"]) == _T("super 2xsai") ? ATR_SUPER2XSAI :
					(value)                                == _T("super eagle") ? ATR_SUPEREAGLE :
                                                                                  ATR_2XSAI
				);

				settings[TYPE_SCALEX].attributes[ATR_TYPE] =
				(
					(value=cfg["video filter scalex scale"]) == _T("3") ? ATR_SCALE3X :
					(value)                                  == _T("2") ? ATR_SCALE2X :
                                                                          ATR_SCALEAX
				);

				settings[TYPE_HQX].attributes[ATR_TYPE] =
				(
					(value=cfg["video filter hqx scale"]) == _T("4") ? ATR_HQ4X :
					(value)                               == _T("3") ? ATR_HQ3X :
					(value)                               == _T("2") ? ATR_HQ2X :
                                                                       ATR_HQAX
				);

				UpdateAutoModes( settings, nes, paletteMode );
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
					_T( "hqx"       )
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

			cfg["video filter ntsc auto tuning"].YesNo() =
			(
				!settings[TYPE_NTSC].attributes[ATR_NO_AUTO_TUNING]
			);

			cfg["video filter ntsc tv aspect"].YesNo() =
			(
				!settings[TYPE_NTSC].attributes[ATR_RESCALE_PIC]
			);

			cfg["video filter 2xsai type"] =
			(
				settings[TYPE_2XSAI].attributes[ATR_TYPE] == ATR_SUPER2XSAI ? _T("super 2xsai") :
				settings[TYPE_2XSAI].attributes[ATR_TYPE] == ATR_SUPEREAGLE ? _T("super eagle") :
                                                                              _T("2xsai")
			);

			cfg["video filter scalex scale"] =
			(
				settings[TYPE_SCALEX].attributes[ATR_TYPE] == ATR_SCALE3X ? _T("3") :
				settings[TYPE_SCALEX].attributes[ATR_TYPE] == ATR_SCALE2X ? _T("2") :
																			_T("auto")
			);

			cfg["video filter hqx scale"] =
			(
				settings[TYPE_HQX].attributes[ATR_TYPE] == ATR_HQ4X ? _T("4") :
				settings[TYPE_HQX].attributes[ATR_TYPE] == ATR_HQ3X ? _T("3") :
				settings[TYPE_HQX].attributes[ATR_TYPE] == ATR_HQ2X ? _T("2") :
                                                                      _T("auto")
			);
		}

		void VideoFilters::ResetAutoModes(Nes::Video nes,const Nes::Video::Palette::Mode mode)
		{
			nes.SetSharpness       ( mode == Nes::Video::Palette::MODE_YUV ? Nes::Video::DEFAULT_SHARPNESS_COMP        : Nes::Video::DEFAULT_SHARPNESS_RGB        );
			nes.SetColorResolution ( mode == Nes::Video::Palette::MODE_YUV ? Nes::Video::DEFAULT_COLOR_RESOLUTION_COMP : Nes::Video::DEFAULT_COLOR_RESOLUTION_RGB );
			nes.SetColorBleed      ( mode == Nes::Video::Palette::MODE_YUV ? Nes::Video::DEFAULT_COLOR_BLEED_COMP      : Nes::Video::DEFAULT_COLOR_BLEED_RGB      );
			nes.SetColorArtifacts  ( mode == Nes::Video::Palette::MODE_YUV ? Nes::Video::DEFAULT_COLOR_ARTIFACTS_COMP  : Nes::Video::DEFAULT_COLOR_ARTIFACTS_RGB  );
			nes.SetColorFringing   ( mode == Nes::Video::Palette::MODE_YUV ? Nes::Video::DEFAULT_COLOR_FRINGING_COMP   : Nes::Video::DEFAULT_COLOR_FRINGING_RGB   );
		}

		void VideoFilters::UpdateAutoModes(const Settings (&settings)[NUM_TYPES],Nes::Video nes,Nes::Video::Palette::Mode mode)
		{
			if (!settings[TYPE_NTSC].attributes[ATR_NO_AUTO_TUNING])
				ResetAutoModes( nes, mode );
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

					dialog.RadioButton
					(
						settings.attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_ON  ? IDC_VIDEO_FILTER_NTSC_FIELDS_ON :
						settings.attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_OFF ? IDC_VIDEO_FILTER_NTSC_FIELDS_OFF :
																						IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO
					).Check();

					dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_AUTO_TUNING ).Check( !settings.attributes[ATR_NO_AUTO_TUNING] );
					dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_TV_ASPECT ).Check( !settings.attributes[ATR_RESCALE_PIC] );

					for (uint i=IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER; i <= IDC_VIDEO_FILTER_NTSC_FRINGING_SLIDER; ++i)
						dialog.Slider( i ).SetRange( 0, 200 );

					UpdateNtscSliders();
					UpdateNtscTuning();

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

					switch (settings.attributes[ATR_TYPE])
					{
						case ATR_SCALE3X: idc = IDC_VIDEO_FILTER_SCALEX_3X;   break;
						case ATR_SCALE2X: idc = IDC_VIDEO_FILTER_SCALEX_2X;   break;
						default:          idc = IDC_VIDEO_FILTER_SCALEX_AUTO; break;
					}

					dialog.RadioButton(idc).Check();

					if (maxScreenSize < MAX_3X_SIZE)
					{
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_2X).Disable();
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_3X).Disable();
					}
					break;

				case IDD_VIDEO_FILTER_HQX:

					switch (settings.attributes[ATR_TYPE])
					{
						case ATR_HQ4X: idc = IDC_VIDEO_FILTER_HQX_SCALING_4X;   break;
						case ATR_HQ3X: idc = IDC_VIDEO_FILTER_HQX_SCALING_3X;   break;
						case ATR_HQ2X: idc = IDC_VIDEO_FILTER_HQX_SCALING_2X;   break;
						default:       idc = IDC_VIDEO_FILTER_HQX_SCALING_AUTO; break;
					}

					dialog.RadioButton(idc).Check();

					if (maxScreenSize < MAX_3X_SIZE)
					{
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_2X).Disable();
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_3X).Disable();
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_4X).Disable();
					}
					else if (maxScreenSize < MAX_4X_SIZE)
					{
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_4X).Disable();
					}
					break;
			}

			return true;
		}

		ibool VideoFilters::OnDestroy(Param&)
		{
			if (backup.restore)
			{
				settings = backup.settings;

				nes.SetSharpness       ( backup.sharpness  );
				nes.SetColorResolution ( backup.resolution );
				nes.SetColorBleed      ( backup.bleed      );
				nes.SetColorArtifacts  ( backup.artifacts  );
				nes.SetColorFringing   ( backup.fringing   );
			}

			return true;
		}

		void VideoFilters::UpdateScanlinesSlider() const
		{
			dialog.Slider( IDC_VIDEO_FILTER_SCANLINES_SLIDER ).Position() = (uint) settings.attributes[ATR_SCANLINES];
			dialog.Edit( IDC_VIDEO_FILTER_SCANLINES_VAL ) << (uint) settings.attributes[ATR_SCANLINES];

			Application::Instance::GetMainWindow().Redraw();
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
			dialog.Edit( idc ).Text() << RealString( value / 100.0, 2 ).Ptr();
			Application::Instance::GetMainWindow().Redraw();
		}

		void VideoFilters::UpdateNtscTuning() const
		{
			const ibool enabled = settings.attributes[ATR_NO_AUTO_TUNING];

			for (uint i=0; i < 5; ++i)
			{
				dialog.Control( IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER+i ).Enable( enabled );
				dialog.Control( IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL+i    ).Enable( enabled );
				dialog.Control( IDC_VIDEO_FILTER_NTSC_SHARPNESS_TEXT+i   ).Enable( enabled );
			}

			for (uint i=0; i < 3; ++i)
				dialog.Control( IDC_VIDEO_FILTER_NTSC_COMPOSITE+i ).Enable( enabled );
		}

		ibool VideoFilters::OnHScroll(Param& param)
		{
			const int value = param.Slider().Scroll();

			switch (param.Slider().GetId())
			{
				case IDC_VIDEO_FILTER_SCANLINES_SLIDER:

					if (settings.attributes[ATR_SCANLINES] != value)
					{
						settings.attributes[ATR_SCANLINES] = value;
						UpdateScanlinesSlider();
					}
					break;

				case IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER:

					if (nes.SetSharpness( value-100 ) != Nes::RESULT_NOP)
						UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL );

					break;

				case IDC_VIDEO_FILTER_NTSC_RESOLUTION_SLIDER:

					if (nes.SetColorResolution( value-100 ) != Nes::RESULT_NOP)
						UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_RESOLUTION_VAL );

					break;

				case IDC_VIDEO_FILTER_NTSC_COLORBLEED_SLIDER:

					if (nes.SetColorBleed( value-100 ) != Nes::RESULT_NOP)
						UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_COLORBLEED_VAL );

					break;

				case IDC_VIDEO_FILTER_NTSC_ARTIFACTS_SLIDER:

					if (nes.SetColorArtifacts( value-100 ) != Nes::RESULT_NOP)
						UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_ARTIFACTS_VAL );

					break;

				case IDC_VIDEO_FILTER_NTSC_FRINGING_SLIDER:

					if (nes.SetColorFringing( value-100 ) != Nes::RESULT_NOP)
						UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_FRINGING_VAL );

					break;
			}

			return true;
		}

		ibool VideoFilters::OnCmdBilinear(Param& param)
		{
			if (param.Button().Clicked())
			{
				settings.attributes[ATR_BILINEAR] = (bool) dialog.CheckBox( IDC_VIDEO_FILTER_BILINEAR ).Checked();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdNtscTuning(Param& param)
		{
			if (param.Button().Clicked())
			{
				settings.attributes[ATR_NO_AUTO_TUNING] = dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_AUTO_TUNING ).Unchecked();

				ResetAutoModes( nes, paletteMode );
				UpdateNtscTuning();
				UpdateNtscSliders();
			}

			return true;
		}

		ibool VideoFilters::OnCmdNtscCable(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint id = param.Button().GetId();

				nes.SetSharpness       (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_SHARPNESS_COMP        : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_SHARPNESS_SVIDEO        : Nes::Video::DEFAULT_SHARPNESS_RGB        );
				nes.SetColorResolution (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_RESOLUTION_COMP : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_RESOLUTION_SVIDEO : Nes::Video::DEFAULT_COLOR_RESOLUTION_RGB );
				nes.SetColorBleed      (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_BLEED_COMP      : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_BLEED_SVIDEO      : Nes::Video::DEFAULT_COLOR_BLEED_RGB      );
				nes.SetColorArtifacts  (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_ARTIFACTS_COMP  : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_ARTIFACTS_SVIDEO  : Nes::Video::DEFAULT_COLOR_ARTIFACTS_RGB  );
				nes.SetColorFringing   (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_FRINGING_COMP   : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_FRINGING_SVIDEO   : Nes::Video::DEFAULT_COLOR_FRINGING_RGB   );

				UpdateNtscSliders();
			}

			return true;
		}

		ibool VideoFilters::OnCmd2xSaI(Param& param)
		{
			if (param.Button().Clicked())
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

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdScaleX(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint id = param.Button().GetId();

				settings.attributes[ATR_TYPE] =
				(
					id == IDC_VIDEO_FILTER_SCALEX_3X ? ATR_SCALE3X :
					id == IDC_VIDEO_FILTER_SCALEX_2X ? ATR_SCALE2X :
                                                       ATR_SCALEAX
				);

				dialog.RadioButton( IDC_VIDEO_FILTER_SCALEX_AUTO ).Check( id == IDC_VIDEO_FILTER_SCALEX_AUTO );
				dialog.RadioButton( IDC_VIDEO_FILTER_SCALEX_2X   ).Check( id == IDC_VIDEO_FILTER_SCALEX_2X   );
				dialog.RadioButton( IDC_VIDEO_FILTER_SCALEX_3X   ).Check( id == IDC_VIDEO_FILTER_SCALEX_3X   );

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdHqX(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint id = param.Button().GetId();

				settings.attributes[ATR_TYPE] =
				(
					id == IDC_VIDEO_FILTER_HQX_SCALING_2X ? ATR_HQ2X :
					id == IDC_VIDEO_FILTER_HQX_SCALING_3X ? ATR_HQ3X :
					id == IDC_VIDEO_FILTER_HQX_SCALING_4X ? ATR_HQ4X :
															ATR_HQAX
				);

				dialog.RadioButton( IDC_VIDEO_FILTER_HQX_SCALING_AUTO ).Check( id == IDC_VIDEO_FILTER_HQX_SCALING_AUTO );
				dialog.RadioButton( IDC_VIDEO_FILTER_HQX_SCALING_2X   ).Check( id == IDC_VIDEO_FILTER_HQX_SCALING_2X   );
				dialog.RadioButton( IDC_VIDEO_FILTER_HQX_SCALING_3X   ).Check( id == IDC_VIDEO_FILTER_HQX_SCALING_3X   );
				dialog.RadioButton( IDC_VIDEO_FILTER_HQX_SCALING_4X   ).Check( id == IDC_VIDEO_FILTER_HQX_SCALING_4X   );

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdDefault(Param& param)
		{
			if (param.Button().Clicked())
			{
				settings.attributes[ATR_BILINEAR] = false;
				dialog.CheckBox( IDC_VIDEO_FILTER_BILINEAR ).Uncheck();

				switch (uint id = dialog.GetId())
				{
					case IDD_VIDEO_FILTER_NTSC:

						dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO ).Check();
						dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_ON ).Uncheck();
						dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_OFF ).Uncheck();

						dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_AUTO_TUNING ).Check();
						dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_TV_ASPECT ).Check();

						settings.attributes[ATR_NO_AUTO_TUNING] = false;

						ResetAutoModes( nes, paletteMode );
						UpdateNtscSliders();
						UpdateNtscTuning();

					case IDD_VIDEO_FILTER_SCANLINES:

						settings.attributes[ATR_SCANLINES] = (id == IDD_VIDEO_FILTER_SCANLINES ? 25 : 0);
						UpdateScanlinesSlider();
						break;

					case IDD_VIDEO_FILTER_2XSAI:

						settings.attributes[ATR_TYPE] = ATR_2XSAI;
						dialog.RadioButton(IDC_VIDEO_FILTER_2XSAI_TYPE_2XSAI).Check();
						break;

					case IDD_VIDEO_FILTER_SCALEX:

						settings.attributes[ATR_TYPE] = ATR_SCALEAX;
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_AUTO).Check();
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_2X).Uncheck();
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_3X).Uncheck();
						break;

					case IDD_VIDEO_FILTER_HQX:

						settings.attributes[ATR_TYPE] = ATR_HQAX;
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_AUTO).Check();
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_2X).Uncheck();
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_3X).Uncheck();
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_4X).Uncheck();
						break;
				}

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				switch (dialog.GetId())
				{
					case IDD_VIDEO_FILTER_NTSC:

						if (dialog.RadioButton(IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO).Checked())
						{
							settings.attributes[ATR_FIELDMERGING] = ATR_FIELDMERGING_AUTO;
						}
						else if (dialog.RadioButton(IDC_VIDEO_FILTER_NTSC_FIELDS_ON).Checked())
						{
							settings.attributes[ATR_FIELDMERGING] = ATR_FIELDMERGING_ON;
						}
						else
						{
							settings.attributes[ATR_FIELDMERGING] = ATR_FIELDMERGING_OFF;
						}

						settings.attributes[ATR_RESCALE_PIC] = (bool) dialog.CheckBox(IDC_VIDEO_FILTER_NTSC_TV_ASPECT).Unchecked();
						break;
				}

				backup.restore = false;
				dialog.Close();
			}

			return true;
		}
	}
}
