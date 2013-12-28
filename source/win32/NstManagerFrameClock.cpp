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

#include "NstObjectHeap.hpp"
#include "NstIoScreen.hpp"
#include "NstIoLog.hpp"
#include "NstResourceString.hpp"
#include "NstManagerEmulator.hpp"
#include "NstWindowMenu.hpp"
#include "NstDialogFrameClock.hpp"
#include "NstManagerFrameClock.hpp"
#include "../core/api/NstApiRewinder.hpp"

namespace Nestopia
{
	using namespace Managers;
	using System::Timer;

	FrameClock::FrameClock(Window::Menu& m,Emulator& e,const Configuration& cfg)
	: 
	emulator ( e ),
	menu     ( m ),
	dialog   ( new Window::FrameClock(cfg) )
	{
		Io::Log() << "Timer: performance counter ";

		if (Timer::HasPerformanceCounter())
			Io::Log() << "present (" << uint(timer.GetFrequency()) << " hz)\r\n";
		else
			Io::Log() << "not present\r\n";

		m.Commands().Add( IDM_OPTIONS_TIMING, this, &FrameClock::OnMenuOptionsTiming );
		emulator.Events().Add( this, &FrameClock::OnEmuEvent );

		UpdateSettings();
	}

	FrameClock::~FrameClock()
	{
		emulator.Events().Remove( this );
	}

	void FrameClock::OnMenuOptionsTiming(uint)
	{
		dialog->Open();
		UpdateSettings();
	}

	void FrameClock::Save(Configuration& cfg) const
	{
		dialog->Save( cfg );
	}

	void FrameClock::UpdateSettings()
	{	
		UpdateRewinderState();

		settings.autoFrameSkip = dialog->UseAutoFrameSkip();
		settings.maxFrameSkips = dialog->GetMaxFrameSkips();

		emulator.ResetSpeed
		( 
	     	dialog->UseDefaultSpeed() ? Emulator::DEFAULT_SPEED : dialog->GetSpeed(), 
			dialog->UseVSync(),
			dialog->UseTrippleBuffering()
		);
				   
		ResetTimer();
	}

	void FrameClock::UpdateRewinderState(ibool force) const
	{
		if (NES_SUCCEEDED(Nes::Rewinder(emulator).Enable( force && dialog->UseRewinder() )))
			Nes::Rewinder(emulator).EnableSound( !dialog->NoRewindSound() );
	}

	void FrameClock::ResetTimer()
	{
		timer.Reset( dialog->UsePerformanceCounter() ? Timer::PERFORMANCE : Timer::MULTIMEDIA );
		time.counter = 0;
		time.frameSkips = 0;
	}

	void FrameClock::OnEmuEvent(Emulator::Event event)
	{
		typedef Nes::Rewinder Rewinder;

		switch (event)
		{
			case Emulator::EVENT_SPEEDING_ON:

				settings.autoFrameSkip = (dialog->UseAutoFrameSkip() || dialog->GetAltSpeed() > MAX_SPEED_NO_FRAMESKIP);
				emulator.SetSpeed( dialog->GetAltSpeed() );
				break;

			case Emulator::EVENT_SPEEDING_OFF:
			
				if (dialog->UseDefaultRewindSpeed() || Rewinder(emulator).GetDirection() == Rewinder::FORWARD)
				{
					settings.autoFrameSkip = dialog->UseAutoFrameSkip();
					emulator.SetSpeed( Emulator::DEFAULT_SPEED );
				}
				else
				{
					settings.autoFrameSkip = (dialog->UseAutoFrameSkip() || dialog->GetRewindSpeed() > MAX_SPEED_NO_FRAMESKIP);
					emulator.SetSpeed( dialog->GetRewindSpeed() );
				}
				break;
			
			case Emulator::EVENT_REWINDING_ON:

				if (dialog->UseRewinder())
				{
					if (NES_FAILED(Rewinder(emulator).SetDirection( Rewinder::BACKWARD )))
						Io::Screen() << Resource::String( IDS_EMU_ERR_CANT_REWIND );
				}
				break;

			case Emulator::EVENT_REWINDING_OFF:
			
				Rewinder(emulator).SetDirection( Rewinder::FORWARD );
				break;
 
			case Emulator::EVENT_SPEED:

				settings.refreshRate = emulator.GetSpeed();
				ResetTimer();
				break;

			case Emulator::EVENT_REWINDING_START:

				if (!dialog->UseDefaultRewindSpeed() && !emulator.IsSpeeding())
				{
					settings.autoFrameSkip = (dialog->UseAutoFrameSkip() || dialog->GetRewindSpeed() > MAX_SPEED_NO_FRAMESKIP);
					emulator.SetSpeed( dialog->GetRewindSpeed() );
				}

				ResetTimer();
				break;

			case Emulator::EVENT_REWINDING_STOP:

				if (!emulator.IsSpeeding())
				{
					settings.autoFrameSkip = dialog->UseAutoFrameSkip();
					emulator.SetSpeed( Emulator::DEFAULT_SPEED );
				}

				ResetTimer();
				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				UpdateRewinderState( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				menu[IDM_OPTIONS_TIMING].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				break;
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	void FrameClock::Synchronize(const ibool wait,const uint skip)
	{
		Timer::Value current( timer.Elapsed() );

		time.counter += timer.GetFrequency();
		const Timer::Value next( time.counter / settings.refreshRate );

		if (current > next)
		{
			current *= settings.refreshRate;

			if (skip & settings.autoFrameSkip)
			{
				time.frameSkips = (uint) ((current - time.counter) / timer.GetFrequency()) + 1;

				if (time.frameSkips > settings.maxFrameSkips)
					time.frameSkips = settings.maxFrameSkips;

				time.counter += timer.GetFrequency() * time.frameSkips;

				if (time.counter < current)
					time.counter = current;
			}
			else
			{
				time.counter = current;
			}
		}
		else if (wait)
		{
			timer.Wait( current, next );
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif
}
