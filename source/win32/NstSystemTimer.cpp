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

#pragma comment(lib,"winmm")

#include "NstApplicationException.hpp"
#include "NstSystemTimer.hpp"
#include <Windows.h>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif

#include <MMSystem.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace Nestopia
{
	using System::Timer;

	const Timer::Settings Timer::settings;

	Timer::Settings::Settings()
	: period(0)
	{
		if (!::QueryPerformanceFrequency( reinterpret_cast<LARGE_INTEGER*>(&pfFrequency) ))
			pfFrequency = 0;

		if (::timeBeginPeriod( 1 ) != TIMERR_NOCANDO)
		{
			period = 1;
		}
		else
		{
			TIMECAPS caps;

			if 
			(
				::timeGetDevCaps( &caps, sizeof(caps) ) == TIMERR_NOERROR && 
				caps.wPeriodMin && 
				::timeBeginPeriod( caps.wPeriodMin ) != TIMERR_NOCANDO
			)
				period = caps.wPeriodMin;
		}
	}

	Timer::Settings::~Settings()
	{
		if (period)
			::timeEndPeriod( period );
	}

	Timer::Timer(const Type desired)
	{
		Reset( desired );
	}

	ibool Timer::Reset(const Type desired)
	{
		threshold = THRESHOLD;
		giveup = 0;

		if 
		(
	     	desired == PERFORMANCE && 
			HasPerformanceCounter() &&
			::QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>(&start) )
		)
		{
			type = PERFORMANCE;
		}
		else
		{
			type = MULTIMEDIA;
			start = ::timeGetTime();
		}

		return type == desired;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	Timer::Value Timer::Elapsed() const
	{
		if (type == PERFORMANCE)
		{
			Value time;
			
			if (::QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>(&time) ))
				return time - start;
			else
				return 0;
		}
		else
		{
			return ::timeGetTime() - start;
		}
	}

	void Timer::Wait(Value current,const Value target)
	{
		uint milliSecs;

		{
			const Value duration( target - current );

			if (type == PERFORMANCE)
				milliSecs = (uint) (duration * 1000U / settings.pfFrequency);
			else
				milliSecs = (uint) (duration);
		}

		if (milliSecs > settings.period + threshold)
		{
			::Sleep( milliSecs - threshold );		
			current = Elapsed();

			if (current > target)
			{
				threshold += giveup;
				giveup ^= 1;
			}
		}
	  
		if (type == PERFORMANCE)
		{
			while (current < target && ::QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>(&current) ))
				current -= start;
		}
		else
		{
			while (current < target && (current = ::timeGetTime()) != 0)
				current -= start;
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif
}
