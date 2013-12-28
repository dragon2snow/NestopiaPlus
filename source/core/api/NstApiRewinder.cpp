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

#include <new>
#include "../NstCore.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiMachine.hpp"
#include "NstApiRewinder.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Api
	{
		Rewinder::StateCaller Rewinder::stateCallback;

		Result Rewinder::Enable(bool enable)
		{
			try
			{
				return emulator.tracker.RewinderEnable( enable ? &emulator : NULL );
			}
			catch (Result result)
			{
				return result;
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}
		}

		bool Rewinder::IsEnabled() const
		{
			return emulator.tracker.RewinderIsEnabled();
		}

		bool Rewinder::IsSoundEnabled() const
		{
			return emulator.tracker.RewinderIsSoundEnabled();
		}

		void Rewinder::EnableSound(bool enable)
		{
			emulator.tracker.RewinderEnableSound( enable );
		}

		Rewinder::Direction Rewinder::GetDirection() const
		{
			return emulator.tracker.IsRewinding() ? BACKWARD : FORWARD;
		}

		Result Rewinder::SetDirection(Direction dir)
		{
			if (emulator.Is(Machine::GAME) && emulator.Is(Machine::ON))
			{
				if (dir == BACKWARD)
					return emulator.tracker.RewinderStart();
				else
					return emulator.tracker.RewinderStop();
			}

			return RESULT_ERR_NOT_READY;
		}

		void Rewinder::Reset()
		{
			if (emulator.Is(Machine::GAME) && emulator.Is(Machine::ON))
				emulator.tracker.RewinderReset();
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
