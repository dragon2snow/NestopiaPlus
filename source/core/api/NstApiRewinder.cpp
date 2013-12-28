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

#include <new>
#include "../NstCore.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiMachine.hpp"
#include "NstApiMovie.hpp"
#include "NstApiRewinder.hpp"
#include "../NstRewinder.hpp"

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
			Result result;

			try
			{
				if (enable && !emulator.rewinder)
				{
					if (emulator.movie)
						return RESULT_ERR_NOT_READY;
					
					emulator.rewinder = new Core::Rewinder
					( 
						emulator.ppu,
						emulator.cpu.GetApu(),
						emulator.rewinderSound,
						emulator, 
						emulator, 
						&Api::Emulator::ExecuteFrame 
					);									

					return RESULT_OK;
				}
				else if (!enable && emulator.rewinder)
				{
					delete emulator.rewinder;
					emulator.rewinder = NULL;
					
					return RESULT_OK;
				}
				else
				{
					return RESULT_NOP;
				}
			}
			catch (Result r)
			{
				result = r;
			}
			catch (const std::bad_alloc&)
			{
				result = RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				result = RESULT_ERR_GENERIC;
			}

			Destroy();

			return result;
		}
	
		bool Rewinder::IsEnabled() const
		{
			return emulator.rewinder && emulator.rewinder->IsEnabled();
		}

		bool Rewinder::IsSoundEnabled() const
		{
			return emulator.rewinderSound;
		}

		void Rewinder::EnableSound(bool state)
		{
			emulator.rewinderSound = state;

			if (emulator.rewinder)
				emulator.rewinder->EnableSound( state );
		}

		Rewinder::Direction Rewinder::GetDirection() const
		{
			NST_COMPILE_ASSERT( FORWARD == 0 && BACKWARD == 1 );
			return (Direction) (emulator.rewinder && emulator.rewinder->IsRewinding());
		}

		Result Rewinder::SetDirection(Direction dir)
		{
			if (emulator.rewinder && emulator.Is(Machine::GAME) && emulator.Is(Machine::ON))
			{
				return (dir == BACKWARD) ? emulator.rewinder->Start() : emulator.rewinder->Stop();
			}
			else
			{
				return RESULT_ERR_NOT_READY;
			}
		}

		void Rewinder::Reset()
		{
			if (emulator.rewinder)
				emulator.rewinder->Reset();
		}

		void Rewinder::Destroy()
		{
			delete emulator.rewinder;
			emulator.rewinder = NULL;
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
