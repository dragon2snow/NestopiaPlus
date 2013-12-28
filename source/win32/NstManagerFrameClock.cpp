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

#include "NstObjectHeap.hpp"
#include "NstIoLog.hpp"
#include "NstManagerEmulator.hpp"
#include "NstWindowMenu.hpp"
#include "NstDialogFrameClock.hpp"
#include "NstManagerFrameClock.hpp"

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
		
		settings.refreshRate = emulator.GetSpeed();
		
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
		settings.vsync = dialog->UseVSync();
		settings.autoFrameSkip = dialog->UseAutoFrameSkip();
		settings.maxFrameSkips = dialog->GetMaxFrameSkips();

		ResetTimer();

		emulator.SetSpeed( dialog->UseDefaultSpeed() ? Emulator::DEFAULT_SPEED : dialog->GetSpeed(), settings.vsync );
		emulator.SetAltSpeed( dialog->GetAltSpeed() );
	}

	void FrameClock::ResetTimer()
	{
		timer.Reset( dialog->UsePerformanceCounter() ? Timer::PERFORMANCE : Timer::MULTIMEDIA );
		time.counter = 0;
		time.out = timer.GetFrequency() * TIME_OUT / 1000;
		time.frameSkips = 0;
	}

	void FrameClock::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
     		case Emulator::EVENT_SPEED:
     		case Emulator::EVENT_ALT_SPEED:

				if (event == Emulator::EVENT_ALT_SPEED && emulator.IsSpeedAlternating())
				{
					settings.refreshRate = emulator.GetAltSpeed();
					settings.autoFrameSkip = TRUE;
				}
				else
				{
					settings.refreshRate = emulator.GetSpeed();
					settings.autoFrameSkip = dialog->UseAutoFrameSkip();
				}

				ResetTimer();
				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				menu[IDM_OPTIONS_TIMING].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				break;
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("at", on)
    #endif

	void FrameClock::Synchronize(const ibool wait,const uint skip)
	{
		const Timer::Value current( timer.Elapsed() );

		time.counter += timer.GetFrequency();
		Timer::Value next( time.counter / settings.refreshRate );

		if (current > next)
		{
			next = current - next;

			if (next >= time.out)
			{
				time.counter = current * settings.refreshRate;
				time.frameSkips = 0;
			}
			else if (skip & settings.autoFrameSkip)
			{
				time.frameSkips = (uint) (next * settings.refreshRate / timer.GetFrequency());

				if (time.frameSkips > settings.maxFrameSkips)
					time.frameSkips = settings.maxFrameSkips;

				time.counter += timer.GetFrequency() * time.frameSkips;
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
