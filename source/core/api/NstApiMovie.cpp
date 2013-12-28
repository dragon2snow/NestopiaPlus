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

#include <new>
#include "../NstStream.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiMachine.hpp"
#include "NstApiMovie.hpp"
#include "../NstMovie.hpp"
#include "../NstImage.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Api
	{
		Movie::StateCaller Movie::stateCallback;

		bool Movie::IsPlaying() const
		{
			return emulator.movie && emulator.movie->IsPlaying();
		}

		bool Movie::IsRecording() const
		{
			return emulator.movie && emulator.movie->IsRecording(); 
		}

		bool Movie::IsStopped() const
		{
			return !emulator.movie || emulator.movie->IsStopped();
		}

		Result Movie::Play(std::istream& stream)
		{
			if (emulator.Is( Machine::GAME ))
			{
				try
				{
					if (emulator.movie == NULL)
						emulator.movie = Core::Movie::Create( emulator.cpu, emulator.image->GetPrgCrc() );

					Result result = emulator.movie->Play( &stream );

					if (NES_SUCCEEDED(result) && result != RESULT_NOP && emulator.Is( Machine::ON ))
						result = emulator.Reset( true );

					if (NES_FAILED(result))
						Eject();

					return result;
				}
				catch (Result result)
				{
					Eject();
					return result;
				}
				catch (std::bad_alloc&)
				{
					Eject();
					return RESULT_ERR_OUT_OF_MEMORY;
				}
				catch (...)
				{
					Eject();
					return RESULT_ERR_GENERIC;
				}
			}
			
			return RESULT_ERR_NOT_READY;
		}
	
		Result Movie::Record(std::ostream& stream,How how)
		{
			if (emulator.Is( Machine::GAME ))
			{
				try
				{
					if (emulator.movie == NULL)
						emulator.movie = Core::Movie::Create( emulator.cpu, emulator.image->GetPrgCrc() );

					const Result result = emulator.movie->Record( &stream, how == APPEND );

					if (NES_FAILED(result))
						Eject();

					return result;
				}
				catch (Result result)
				{
					Eject();
					return result;
				}
				catch (std::bad_alloc&)
				{
					Eject();
					return RESULT_ERR_OUT_OF_MEMORY;
				}
				catch (...)
				{
					Eject();
					return RESULT_ERR_GENERIC;
				}
			}
			
			return RESULT_ERR_NOT_READY;
		}
	
		void Movie::Stop()
		{
			if (emulator.movie)
				emulator.movie->Stop();
		}

		void Movie::Eject()
		{
			Core::Movie::Destroy( emulator.movie );
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
