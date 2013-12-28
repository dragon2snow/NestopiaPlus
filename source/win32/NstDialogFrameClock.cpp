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

#include "NstSystemTimer.hpp"
#include "NstWindowParam.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstDialogFrameClock.hpp"

namespace Nestopia
{
	using namespace Window;

	struct FrameClock::Handlers
	{
		static const MsgHandler::Entry<FrameClock> messages[];
		static const MsgHandler::Entry<FrameClock> commands[];
	};

	const MsgHandler::Entry<FrameClock> FrameClock::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &FrameClock::OnInitDialog },
		{ WM_DESTROY,    &FrameClock::OnDestroy    },
		{ WM_HSCROLL,    &FrameClock::OnHScroll    }
	};

	const MsgHandler::Entry<FrameClock> FrameClock::Handlers::commands[] =
	{
		{ IDC_TIMING_SYNC_REFRESH,           &FrameClock::OnCmdRefresh      },
		{ IDC_TIMING_AUTO_FRAME_SKIP,        &FrameClock::OnCmdRefresh      },
		{ IDC_TIMING_REWINDER,               &FrameClock::OnCmdRewinder     },
		{ IDC_TIMING_DEFAULT_SPEED,          &FrameClock::OnCmdDefaultSpeed },
		{ IDC_TIMING_REWINDER_DEFAULT_SPEED, &FrameClock::OnCmdRewinder     },
		{ IDC_TIMING_DEFAULT,                &FrameClock::OnCmdDefault      },
		{ IDC_TIMING_OK,                     &FrameClock::OnCmdOk           }
	};

	FrameClock::FrameClock(const Configuration& cfg)
	: dialog(IDD_TIMING,this,Handlers::messages,Handlers::commands)
	{
		settings.autoFrameSkip         = ( cfg[ "timer auto frame skip"      ] == Configuration::YES );
		settings.vsync                 = ( cfg[ "timer vsync"                ] != Configuration::NO  );
		settings.tripleBuffering       = ( cfg[ "timer triple buffering"     ] == Configuration::YES );
		settings.rewinder              = ( cfg[ "timer rewinder"             ] == Configuration::YES );
		settings.useDefaultSpeed       = ( cfg[ "timer default speed"        ] != Configuration::NO  );
		settings.useDefaultRewindSpeed = ( cfg[ "timer default rewind speed" ] != Configuration::NO  );
		settings.noRewindSound         = ( cfg[ "timer no rewind sound"      ] == Configuration::YES );
		settings.pfCounter             = ( cfg[ "timer performance counter"  ] != Configuration::NO  );

		settings.speed = cfg[ "timer speed" ];
		settings.altSpeed = cfg[ "timer alternative speed" ];
		settings.rewindSpeed = cfg[ "timer rewind speed" ];
		settings.maxFrameSkips = cfg[ "timer max frame skips" ];

		if (!System::Timer::HasPerformanceCounter())
			settings.pfCounter = FALSE;

		settings.maxFrameSkips =
		(
			settings.maxFrameSkips ? NST_CLAMP(settings.maxFrameSkips,MIN_FRAME_SKIPS,MAX_FRAME_SKIPS) : 
	    	                         DEFAULT_FRAME_SKIPS
		);

		settings.speed =
		(
			settings.speed ? NST_CLAMP(settings.speed,MIN_SPEED,MAX_SPEED) : 
	                     	 DEFAULT_SPEED
		);

		settings.altSpeed =
		(
    		settings.altSpeed ? NST_CLAMP(settings.altSpeed,MIN_SPEED,MAX_SPEED) : 
    		                    DEFAULT_ALT_SPEED
		);

		settings.rewindSpeed =
		(
    		settings.rewindSpeed ? NST_CLAMP(settings.rewindSpeed,MIN_SPEED,MAX_SPEED) : 
    		                       DEFAULT_REWIND_SPEED
		);
	}

	void FrameClock::Save(Configuration& cfg) const
	{
		cfg[ "timer speed"                ] = (uint) settings.speed;
		cfg[ "timer alternative speed"    ] = (uint) settings.altSpeed;
		cfg[ "timer rewind speed"         ] = (uint) settings.rewindSpeed;
		cfg[ "timer max frame skips"      ] = (uint) settings.maxFrameSkips;
		cfg[ "timer auto frame skip"      ].YesNo() = settings.autoFrameSkip;
		cfg[ "timer vsync"                ].YesNo() = settings.vsync;
		cfg[ "timer triple buffering"     ].YesNo() = settings.tripleBuffering;
		cfg[ "timer rewinder"             ].YesNo() = settings.rewinder;
		cfg[ "timer default speed"        ].YesNo() = settings.useDefaultSpeed;
		cfg[ "timer default rewind speed" ].YesNo() = settings.useDefaultRewindSpeed;
		cfg[ "timer no rewind sound"	  ].YesNo() = settings.noRewindSound;
		cfg[ "timer performance counter"  ].YesNo() = settings.pfCounter;		
	}

	void FrameClock::UpdateRewinderEnable() const
	{
		const ibool enabled = dialog.CheckBox( IDC_TIMING_REWINDER ).IsChecked();
		const ibool defaultSpeed = dialog.CheckBox( IDC_TIMING_REWINDER_DEFAULT_SPEED ).IsChecked();

		dialog.Control( IDC_TIMING_REWINDER_DEFAULT_SPEED ).Enable( enabled );
		dialog.Control( IDC_TIMING_REWINDER_NOSOUND       ).Enable( enabled );
		dialog.Control( IDC_TIMING_REWINDER_SPEED         ).Enable( enabled && !defaultSpeed );
		dialog.Control( IDC_TIMING_REWINDER_SPEED_NUM     ).Enable( enabled && !defaultSpeed );
	}

	ibool FrameClock::OnInitDialog(Param&)
	{
		dialog.Slider      ( IDC_TIMING_SPEED                  ).SetRange     ( MIN_SPEED, MAX_SPEED                   );
		dialog.Slider      ( IDC_TIMING_SPEED                  ).Position() = ( settings.speed                         );	
		dialog.Slider      ( IDC_TIMING_ALT_SPEED              ).SetRange     ( MIN_SPEED, MAX_SPEED                   );
		dialog.Slider      ( IDC_TIMING_ALT_SPEED              ).Position() = ( settings.altSpeed                      );	
		dialog.Slider      ( IDC_TIMING_REWINDER_SPEED         ).SetRange     ( MIN_SPEED, MAX_SPEED                   );
		dialog.Slider      ( IDC_TIMING_REWINDER_SPEED         ).Position() = ( settings.rewindSpeed                   );
		dialog.Slider      ( IDC_TIMING_FRAME_SKIPS            ).SetRange     ( MIN_FRAME_SKIPS, MAX_FRAME_SKIPS       );
		dialog.Slider      ( IDC_TIMING_FRAME_SKIPS            ).Position() = ( settings.maxFrameSkips                 );
		dialog.RadioButton ( IDC_TIMING_SYNC_REFRESH           ).Check        ( !settings.autoFrameSkip                );
		dialog.RadioButton ( IDC_TIMING_AUTO_FRAME_SKIP        ).Check        ( settings.autoFrameSkip                 );
		dialog.CheckBox    ( IDC_TIMING_VSYNC                  ).Check        ( settings.vsync                         );
		dialog.CheckBox    ( IDC_TIMING_TRIPLE_BUFFERING       ).Check        ( settings.tripleBuffering               );
		dialog.CheckBox    ( IDC_TIMING_REWINDER               ).Check        ( settings.rewinder                      );
		dialog.CheckBox    ( IDC_TIMING_DEFAULT_SPEED          ).Check        ( settings.useDefaultSpeed               );
		dialog.CheckBox    ( IDC_TIMING_REWINDER_DEFAULT_SPEED ).Check        ( settings.useDefaultRewindSpeed         );
		dialog.CheckBox    ( IDC_TIMING_REWINDER_NOSOUND       ).Check        ( settings.noRewindSound                 );
		dialog.CheckBox    ( IDC_TIMING_PFC                    ).Check        ( settings.pfCounter                     );
		dialog.Control     ( IDC_TIMING_PFC                    ).Enable       ( System::Timer::HasPerformanceCounter() );
		dialog.Control     ( IDC_TIMING_FRAME_SKIPS            ).Enable       ( settings.autoFrameSkip                 );
		dialog.Control     ( IDC_TIMING_FRAME_SKIPS_TEXT       ).Enable       ( settings.autoFrameSkip                 );
		dialog.Control     ( IDC_TIMING_FRAME_SKIPS_NUM        ).Enable       ( settings.autoFrameSkip                 );
		dialog.Control     ( IDC_TIMING_SPEED                  ).Enable       ( !settings.useDefaultSpeed              );
		dialog.Control     ( IDC_TIMING_SPEED_NUM              ).Enable       ( !settings.useDefaultSpeed              );

		dialog.Edit( IDC_TIMING_SPEED_NUM          ) << settings.speed;
		dialog.Edit( IDC_TIMING_ALT_SPEED_NUM      ) << settings.altSpeed;
		dialog.Edit( IDC_TIMING_REWINDER_SPEED_NUM ) << settings.rewindSpeed;
		dialog.Edit( IDC_TIMING_FRAME_SKIPS_NUM    ) << settings.maxFrameSkips;	

		UpdateRewinderEnable();

		return TRUE;
	}

	ibool FrameClock::OnDestroy(Param&)
	{
		settings.autoFrameSkip         = dialog.RadioButton ( IDC_TIMING_AUTO_FRAME_SKIP        ).IsChecked();
		settings.vsync                 = dialog.CheckBox    ( IDC_TIMING_VSYNC                  ).IsChecked();
		settings.tripleBuffering       = dialog.CheckBox    ( IDC_TIMING_TRIPLE_BUFFERING       ).IsChecked();	
		settings.rewinder              = dialog.CheckBox    ( IDC_TIMING_REWINDER               ).IsChecked();
		settings.useDefaultSpeed       = dialog.CheckBox    ( IDC_TIMING_DEFAULT_SPEED          ).IsChecked();
		settings.useDefaultRewindSpeed = dialog.CheckBox    ( IDC_TIMING_REWINDER_DEFAULT_SPEED ).IsChecked();
		settings.noRewindSound         = dialog.CheckBox    ( IDC_TIMING_REWINDER_NOSOUND       ).IsChecked();
		settings.pfCounter             = dialog.CheckBox    ( IDC_TIMING_PFC                    ).IsChecked();	

		return TRUE;
	}

	ibool FrameClock::OnHScroll(Param& param)
	{
		if (param.Slider().IsControl( IDC_TIMING_SPEED ))
		{
			const uchar speed = param.Slider().Scroll();

			if (settings.speed != speed)
			{
				settings.speed = speed;
				dialog.Edit( IDC_TIMING_SPEED_NUM ) << speed;
			}
		}
		else if (param.Slider().IsControl( IDC_TIMING_ALT_SPEED ))
		{
			const uchar altSpeed = param.Slider().Scroll();

			if (settings.altSpeed != altSpeed)
			{
				settings.altSpeed = altSpeed;
				dialog.Edit( IDC_TIMING_ALT_SPEED_NUM ) << altSpeed;
			}
		}		
		else if (param.Slider().IsControl( IDC_TIMING_REWINDER_SPEED ))
		{
			const uchar rewindSpeed = param.Slider().Scroll();

			if (settings.rewindSpeed != rewindSpeed)
			{
				settings.rewindSpeed = rewindSpeed;
				dialog.Edit( IDC_TIMING_REWINDER_SPEED_NUM ) << rewindSpeed;
			}
		}		
		else if (param.Slider().IsControl( IDC_TIMING_FRAME_SKIPS ))
		{
			const uchar maxFrameSkips = param.Slider().Scroll();

			if (settings.maxFrameSkips != maxFrameSkips)
			{
				settings.maxFrameSkips = maxFrameSkips;
				dialog.Edit( IDC_TIMING_FRAME_SKIPS_NUM ) << maxFrameSkips;
			}
		}

		return TRUE;
	}

	ibool FrameClock::OnCmdRefresh(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const ibool autoFrameSkip = (param.Button().GetId() == IDC_TIMING_AUTO_FRAME_SKIP);

			dialog.Control( IDC_TIMING_FRAME_SKIPS      ).Enable( autoFrameSkip );
			dialog.Control( IDC_TIMING_FRAME_SKIPS_TEXT ).Enable( autoFrameSkip );
			dialog.Control( IDC_TIMING_FRAME_SKIPS_NUM  ).Enable( autoFrameSkip );
		}

		return TRUE;
	}

	ibool FrameClock::OnCmdDefaultSpeed(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const ibool speed = dialog.CheckBox( IDC_TIMING_DEFAULT_SPEED ).IsUnchecked();

			dialog.Control( IDC_TIMING_SPEED     ).Enable( speed );
			dialog.Control( IDC_TIMING_SPEED_NUM ).Enable( speed );
		}

		return TRUE;
	}

	ibool FrameClock::OnCmdRewinder(Param& param)
	{
		if (param.Button().IsClicked())
			UpdateRewinderEnable();

		return TRUE;
	}

	ibool FrameClock::OnCmdDefault(Param& param)
	{
		if (param.Button().IsClicked())
		{
			settings.speed = DEFAULT_SPEED;
			settings.maxFrameSkips = DEFAULT_FRAME_SKIPS;
			settings.altSpeed = DEFAULT_ALT_SPEED;
			settings.rewindSpeed = DEFAULT_REWIND_SPEED;
															
			dialog.Slider      ( IDC_TIMING_SPEED                  ).Position() = DEFAULT_SPEED;	
			dialog.Slider      ( IDC_TIMING_ALT_SPEED              ).Position() = DEFAULT_ALT_SPEED;
			dialog.Slider      ( IDC_TIMING_REWINDER_SPEED         ).Position() = DEFAULT_REWIND_SPEED;
			dialog.Slider      ( IDC_TIMING_FRAME_SKIPS            ).Position() = DEFAULT_FRAME_SKIPS;
			dialog.RadioButton ( IDC_TIMING_SYNC_REFRESH           ).Check  ( TRUE  );
			dialog.RadioButton ( IDC_TIMING_AUTO_FRAME_SKIP        ).Check  ( FALSE );
			dialog.CheckBox    ( IDC_TIMING_VSYNC                  ).Check  ( TRUE  );
			dialog.CheckBox    ( IDC_TIMING_TRIPLE_BUFFERING       ).Check  ( FALSE );
			dialog.CheckBox    ( IDC_TIMING_DEFAULT_SPEED          ).Check  ( TRUE  );
			dialog.CheckBox    ( IDC_TIMING_REWINDER               ).Check  ( FALSE );
			dialog.CheckBox    ( IDC_TIMING_REWINDER_DEFAULT_SPEED ).Check  ( TRUE  );
			dialog.CheckBox    ( IDC_TIMING_REWINDER_NOSOUND       ).Check  ( FALSE );
			dialog.CheckBox    ( IDC_TIMING_PFC                    ).Check  ( System::Timer::HasPerformanceCounter() );
			dialog.Control     ( IDC_TIMING_PFC                    ).Enable ( System::Timer::HasPerformanceCounter() );
			dialog.Control     ( IDC_TIMING_FRAME_SKIPS            ).Enable ( FALSE );
			dialog.Control     ( IDC_TIMING_FRAME_SKIPS_TEXT       ).Enable ( FALSE );
			dialog.Control     ( IDC_TIMING_FRAME_SKIPS_NUM        ).Enable ( FALSE );
			dialog.Control     ( IDC_TIMING_SPEED                  ).Enable ( FALSE );
			dialog.Control     ( IDC_TIMING_SPEED_NUM              ).Enable ( FALSE );
			dialog.Edit        ( IDC_TIMING_SPEED_NUM              ) << (uint) DEFAULT_SPEED;
			dialog.Edit        ( IDC_TIMING_ALT_SPEED_NUM          ) << (uint) DEFAULT_ALT_SPEED;
			dialog.Edit        ( IDC_TIMING_REWINDER_SPEED_NUM     ) << (uint) DEFAULT_REWIND_SPEED;
			dialog.Edit        ( IDC_TIMING_FRAME_SKIPS_NUM        ) << (uint) DEFAULT_FRAME_SKIPS;

			UpdateRewinderEnable();
		}

		return TRUE;
	}

	ibool FrameClock::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}
}
