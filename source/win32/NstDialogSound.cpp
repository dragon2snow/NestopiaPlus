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
#include "NstApplicationConfiguration.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogSound.hpp"
#include "../core/api/NstApiSound.hpp"

namespace Nestopia
{
	using namespace Window;

	NST_COMPILE_ASSERT
	(
		0x1U << Sound::CHANNEL_SQUARE1  == Nes::Sound::CHANNEL_SQUARE1 &&
		0x1U << Sound::CHANNEL_SQUARE2  == Nes::Sound::CHANNEL_SQUARE2 &&
		0x1U << Sound::CHANNEL_TRIANGLE == Nes::Sound::CHANNEL_TRIANGLE &&
		0x1U << Sound::CHANNEL_NOISE    == Nes::Sound::CHANNEL_NOISE &&
		0x1U << Sound::CHANNEL_DPCM     == Nes::Sound::CHANNEL_DPCM &&
		0x1U << Sound::CHANNEL_EXTERNAL == Nes::Sound::CHANNEL_EXTERNAL
	);

	Sound::Settings::Settings()
	:
	enabled  ( FALSE ),
	adapter  ( 0 ),
	volume   ( DEFAULT_VOLUME ),
	rate     ( DEFAULT_RATE ),
	bits     ( DEFAULT_BITS ),
	latency  ( DEFAULT_LATENCY ),
	stereo   ( DEFAULT_MONO ),
	pitch    ( FALSE )
	{}

	const Sound::ChannelLut Sound::channelLut[] =
	{
		{ "sound apu square 1", IDC_SOUND_SQUARE1,  CHANNEL_SQUARE1  },
		{ "sound apu square 2", IDC_SOUND_SQUARE2,  CHANNEL_SQUARE2  },
		{ "sound apu triangle", IDC_SOUND_TRIANGLE, CHANNEL_TRIANGLE },
		{ "sound apu noise",    IDC_SOUND_NOISE,    CHANNEL_NOISE	 },
		{ "sound apu dpcm",     IDC_SOUND_DPCM,     CHANNEL_DPCM	 },
		{ "sound apu external", IDC_SOUND_EXTERNAL, CHANNEL_EXTERNAL }
	};

	struct Sound::Handlers
	{
		static const MsgHandler::Entry<Sound> messages[];
		static const MsgHandler::Entry<Sound> commands[];
	};

	const MsgHandler::Entry<Sound> Sound::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Sound::OnInitDialog },
		{ WM_DESTROY,    &Sound::OnDestroy    }
	};

	const MsgHandler::Entry<Sound> Sound::Handlers::commands[] =
	{
		{ IDC_SOUND_ENABLE,  &Sound::OnCmdEnable  },
		{ IDC_SOUND_DEFAULT, &Sound::OnCmdDefault },
		{ IDC_SOUND_8_BIT,   &Sound::OnCmdBits    },
		{ IDC_SOUND_16_BIT,  &Sound::OnCmdBits    },
		{ IDC_SOUND_MONO,    &Sound::OnCmdOutput  },
		{ IDC_SOUND_STEREO,  &Sound::OnCmdOutput  },
		{ IDC_SOUND_OK,      &Sound::OnCmdOk      }
	};

	Sound::Sound(const Adapters& a,const Managers::Paths& paths,const Configuration& cfg)
	: 
	adapters ( a ),
	dialog   ( IDD_SOUND, this, Handlers::messages, Handlers::commands ),
	recorder ( paths )
	{
		if (adapters.size() && cfg[ "sound enabled" ] != Configuration::NO)
			settings.enabled = TRUE;

		{
			const Adapters::const_iterator it
			(
				std::find
				(
       				adapters.begin(),
     				adapters.end(),
     				System::Guid( cfg["sound device"] )
				)
			);

			if (it != adapters.end())
				settings.adapter = it - adapters.begin();
			else
				settings.adapter = GetDefaultAdapter();
		}

		switch (uint rate = cfg["sound sample rate"])
		{
			case 11025U:
			case 22050U:
			case 44100U:
			case 48000U:
			case 88200U:
			case 96000U:
	
				settings.rate = rate;
		    	break;
		}

		switch (uint bits = cfg["sound sample bits"])
		{
			case 8:
			case 16:
	
				settings.bits = bits;
				break;
		}

		settings.pitch = (cfg["sound adjust pitch"] == Configuration::YES);
		
		const String::Heap& string = cfg["sound output"];
		settings.stereo = (string == "stereo");

		for (uint i=0; i < NST_COUNT(channelLut); ++i)
			settings.channels[channelLut[i].index] = (cfg[channelLut[i].cfg] != Configuration::NO);

		settings.latency = cfg[ "sound buffers" ].Default( (uint) DEFAULT_LATENCY);

		if (settings.latency > LATENCY_MAX)
			settings.latency = DEFAULT_LATENCY;

		settings.volume = cfg[ "sound volume" ].Default( (uint) DEFAULT_VOLUME );

		if (settings.volume > VOLUME_MAX)
			settings.volume = DEFAULT_VOLUME;
	}

	Sound::~Sound()
	{
	}

	void Sound::Save(Configuration& cfg) const
	{
		{
			System::Guid guid;

			if (adapters.size() > settings.adapter)
				guid = adapters[settings.adapter].guid;

			cfg[ "sound device" ].Quote() = guid.GetString();
		}

		cfg[ "sound enabled"      ].YesNo() = settings.enabled;
		cfg[ "sound sample rate"  ] = settings.rate;
		cfg[ "sound sample bits"  ] = settings.bits;
		cfg[ "sound buffers"      ] = settings.latency;
		cfg[ "sound volume"       ] = settings.volume;
		cfg[ "sound output"       ] = (settings.stereo ? "stereo" : "mono");
		cfg[ "sound adjust pitch" ].YesNo() = settings.pitch;

		for (uint i=0; i < NST_COUNT(channelLut); ++i)
			cfg[channelLut[i].cfg].OnOff() = settings.channels[channelLut[i].index];
	}

	uint Sound::GetDefaultAdapter() const
	{
		const Adapters::const_iterator it( std::find( adapters.begin(), adapters.end(), System::Guid() ) );
		return it != adapters.end() ? it - adapters.begin() : 0;
	}

	ibool Sound::OnInitDialog(Param&)
	{
		if (adapters.empty())
		{
			dialog.Control( IDC_SOUND_DEFAULT ).Disable();
		}
		else
		{
			const Control::ComboBox comboBox( dialog.ComboBox(IDC_SOUND_DEVICE) );

			for (Adapters::const_iterator it(adapters.begin()); it != adapters.end(); ++it)
				comboBox.Add( it->name );

			comboBox[settings.adapter].Select();
		}

		{
			static const char rates[][6] =
			{
				"11025","22050","44100","48000","88200","96000"
			};

			uint index;

			switch (settings.rate)
			{
				case 11025U: index = 0; break;
				case 22050U: index = 1; break;
				case 48000U: index = 3; break;
				case 88200U: index = 4; break;
				case 96000U: index = 5; break;
				default:	 index = 2; break;
			}

			const Control::ComboBox comboBox( dialog.ComboBox(IDC_SOUND_SAMPLE_RATE) );

			comboBox.Add( rates, NST_COUNT(rates) );
			comboBox[index].Select();
		}

		dialog.RadioButton( (settings.bits == 8 ? IDC_SOUND_8_BIT : IDC_SOUND_16_BIT) ).Check();
		dialog.RadioButton( (settings.stereo ? IDC_SOUND_STEREO : IDC_SOUND_MONO) ).Check();

		{
			const Control::Slider control( dialog.Slider(IDC_SOUND_VOLUME) );
			control.SetRange( 0, VOLUME_MAX );
			control.Position() = VOLUME_MAX - settings.volume;
		}

		{
			const Control::Slider control( dialog.Slider(IDC_SOUND_LATENCY) );
			control.SetRange( 1, LATENCY_MAX );
			control.Position() = settings.latency;
		}

		for (uint i=0; i < NST_COUNT(channelLut); ++i)
			dialog.CheckBox( channelLut[i].ctrl ).Check( settings.channels[channelLut[i].index] );

		dialog.CheckBox( IDC_SOUND_ADJUST_PITCH ).Check( settings.pitch );
		dialog.CheckBox( IDC_SOUND_ENABLE ).Check( settings.enabled );

		if (!settings.enabled)
			Enable( FALSE );

		return TRUE;
	}

	ibool Sound::OnDestroy(Param&)
	{	
		if (adapters.size())
		{
			static const uint rates[] = {11025,22050,44100,48000,88200,96000};

			settings.adapter = dialog.ComboBox( IDC_SOUND_DEVICE ).Selection().GetIndex();
			settings.volume = VOLUME_MAX - dialog.Slider( IDC_SOUND_VOLUME ).Position();
			settings.rate = rates[dialog.ComboBox( IDC_SOUND_SAMPLE_RATE ).Selection().GetIndex()];
			settings.bits = dialog.RadioButton( IDC_SOUND_8_BIT ).IsChecked() ? 8 : 16;
			settings.latency = dialog.Slider( IDC_SOUND_LATENCY ).Position();	
			settings.enabled = dialog.CheckBox( IDC_SOUND_ENABLE ).IsChecked();
			settings.stereo = dialog.RadioButton( IDC_SOUND_STEREO ).IsChecked();
			settings.pitch = dialog.CheckBox( IDC_SOUND_ADJUST_PITCH ).IsChecked();

			for (uint i=0; i < NST_COUNT(channelLut); ++i)
				settings.channels[channelLut[i].index] = dialog.CheckBox( channelLut[i].ctrl ).IsChecked();
		}

		return TRUE;
	}

	ibool Sound::OnCmdEnable(Param& param)
	{
		if (param.Button().IsClicked())
			Enable( dialog.CheckBox( IDC_SOUND_ENABLE ).IsChecked() );

		return TRUE;
	}

	ibool Sound::OnCmdBits(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint cmd = param.Button().GetId();
			dialog.RadioButton( IDC_SOUND_16_BIT ).Check( cmd == IDC_SOUND_16_BIT );
			dialog.RadioButton( IDC_SOUND_8_BIT ).Check( cmd == IDC_SOUND_8_BIT );
		}

		return TRUE;
	}

	ibool Sound::OnCmdOutput(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint cmd = param.Button().GetId();
			dialog.RadioButton( IDC_SOUND_MONO ).Check( cmd == IDC_SOUND_MONO );
			dialog.RadioButton( IDC_SOUND_STEREO ).Check( cmd == IDC_SOUND_STEREO );
		}

		return TRUE;
	}

	ibool Sound::OnCmdDefault(Param& param)
	{
		NST_VERIFY( adapters.size() );

		if (param.Button().IsClicked())
		{
			Enable( TRUE );

			dialog.ComboBox( IDC_SOUND_DEVICE )[GetDefaultAdapter()].Select();
			dialog.ComboBox( IDC_SOUND_SAMPLE_RATE )[2].Select();	

			dialog.RadioButton( IDC_SOUND_16_BIT ).Check();
			dialog.RadioButton( IDC_SOUND_8_BIT ).Uncheck();

			dialog.RadioButton( IDC_SOUND_MONO ).Check();
			dialog.RadioButton( IDC_SOUND_STEREO ).Uncheck();

			dialog.CheckBox( IDC_SOUND_ADJUST_PITCH ).Uncheck();

			dialog.Slider( IDC_SOUND_VOLUME  ).Position() = VOLUME_MAX - DEFAULT_VOLUME;
			dialog.Slider( IDC_SOUND_LATENCY ).Position() = DEFAULT_LATENCY;

			for (uint i=0; i < NST_COUNT(channelLut); ++i)
				dialog.CheckBox( channelLut[i].ctrl ).Check();
		}

		return TRUE;
	}

	ibool Sound::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}

	void Sound::Enable(const ibool state)
	{
		dialog.Control( IDC_SOUND_DEVICE       ).Enable( state );
		dialog.Control( IDC_SOUND_SAMPLE_RATE  ).Enable( state );
		dialog.Control( IDC_SOUND_8_BIT        ).Enable( state );
		dialog.Control( IDC_SOUND_16_BIT       ).Enable( state );
		dialog.Control( IDC_SOUND_VOLUME       ).Enable( state );
		dialog.Control( IDC_SOUND_LATENCY      ).Enable( state );
		dialog.Control( IDC_SOUND_DEFAULT      ).Enable( state );
		dialog.Control( IDC_SOUND_MONO         ).Enable( state );
		dialog.Control( IDC_SOUND_STEREO       ).Enable( state );
		dialog.Control( IDC_SOUND_ADJUST_PITCH ).Enable( state );

		for (uint i=0; i < NST_COUNT(channelLut); ++i)
			dialog.Control( channelLut[i].ctrl ).Enable( state );
	}

	struct Sound::Recorder::Handlers
	{
		static const MsgHandler::Entry<Recorder> messages[];
		static const MsgHandler::Entry<Recorder> commands[];
	};

	const MsgHandler::Entry<Sound::Recorder> Sound::Recorder::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Recorder::OnInitDialog }
	};

	const MsgHandler::Entry<Sound::Recorder> Sound::Recorder::Handlers::commands[] =
	{
		{ IDC_SOUND_CAPTURE_CLEAR,  &Recorder::OnCmdClear  },
		{ IDC_SOUND_CAPTURE_BROWSE, &Recorder::OnCmdBrowse },
		{ IDC_SOUND_CAPTURE_OK,     &Recorder::OnCmdOk     },
		{ IDC_SOUND_CAPTURE_CANCEL, &Recorder::OnCmdCancel }
	};

	Sound::Recorder::Recorder(const Managers::Paths& p)
	: dialog(IDD_SOUND_RECORDER,this,Handlers::messages,Handlers::commands), paths(p) {}

	ibool Sound::Recorder::OnInitDialog(Param&)
	{
		dialog.Edit( IDC_SOUND_CAPTURE_FILE ) << waveFile;
		dialog.Edit( IDC_SOUND_CAPTURE_FILE ).Limit( _MAX_PATH );

		return TRUE;
	}

	ibool Sound::Recorder::OnCmdClear(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Edit(IDC_SOUND_CAPTURE_FILE).Clear();

		return TRUE;
	}

	ibool Sound::Recorder::OnCmdBrowse(Param& param)
	{
		if (param.Button().IsClicked())	
			dialog.Edit(IDC_SOUND_CAPTURE_FILE).Try() << paths.BrowseSave( Managers::Paths::File::WAVE, waveFile );

		return TRUE;
	}

	ibool Sound::Recorder::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			dialog.Edit(IDC_SOUND_CAPTURE_FILE) >> waveFile;
			dialog.Close();
		}

		return TRUE;
	}

	ibool Sound::Recorder::OnCmdCancel(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}
}
