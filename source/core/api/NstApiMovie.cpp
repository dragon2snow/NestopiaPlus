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

		bool Movie::IsPlaying(const void* internal) const
		{
			return !internal && emulator.movie && emulator.movie->IsPlaying();
		}

		bool Movie::IsRecording(const void* internal) const
		{
			return !internal && emulator.movie && emulator.movie->IsRecording(); 
		}

		bool Movie::IsStopped(const void* internal) const
		{
			return internal || !emulator.movie || emulator.movie->IsStopped();
		}

		Result Movie::Play(std::istream& stream,CallbackMode mode,bool reset,const void* internal)
		{
			NST_COMPILE_ASSERT( ENABLE_CALLBACK == 1 && DISABLE_CALLBACK == 0 );

			if (!internal && emulator.Is( Machine::GAME ))
			{
				Result result;

				try
				{
					if (emulator.movie == NULL)
					{
						emulator.movie = Core::Movie::Create
						( 
					     	emulator.cpu, 
							emulator.Is(Machine::CARTRIDGE) ? emulator.image->GetPrgCrc() : 0 
						);
					}

					result = emulator.movie->Play( &stream, mode );
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

				if (NES_SUCCEEDED(result))
				{
					if (result != RESULT_NOP && emulator.Is( Machine::ON ))
					{
						if (reset)
							emulator.Reset( true );
						else
							emulator.frame = 0;
					}
				}
				else
				{
					Eject();
				}

				return result;
			}
			
			return RESULT_ERR_NOT_READY;
		}
	
		Result Movie::Record(std::ostream& stream,How how,CallbackMode mode,const void* internal)
		{
			NST_COMPILE_ASSERT( CLEAN == 0 && APPEND == 1 && ENABLE_CALLBACK == 1 && DISABLE_CALLBACK == 0 );

			if (!internal && emulator.Is( Machine::GAME ))
			{
				Result result;

				try
				{
					if (emulator.movie == NULL)
						emulator.movie = Core::Movie::Create( emulator.cpu, emulator.image->GetPrgCrc() );

					result = emulator.movie->Record( &stream, how, mode );
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

				if (NES_FAILED(result))
					Eject();

				return result;
			}
			
			return RESULT_ERR_NOT_READY;
		}
	
		void Movie::Stop(const void* internal)
		{
			if (!internal && emulator.movie)
				emulator.movie->Stop();
		}

		void Movie::Eject(const void* internal)
		{
			if (!internal)
				Core::Movie::Destroy( emulator.movie );
		}

		Result Movie::Play(std::istream& stream,CallbackMode mode)
		{
			return Play( stream, mode, true, emulator.rewinder );
		}

		Result Movie::Record(std::ostream& stream,How how,CallbackMode mode)
		{
			return Record( stream, how, mode, emulator.rewinder );
		}

		void Movie::Stop()
		{
			Stop( emulator.rewinder );
		}

		void Movie::Eject()
		{
			Eject( emulator.rewinder );
		}

		void Movie::Cut()
		{
			if (emulator.movie && !emulator.rewinder)
				emulator.movie->Cut();
		}
	
		bool Movie::IsPlaying() const
		{
			return IsPlaying( emulator.rewinder );
		}

		bool Movie::IsRecording() const
		{
			return IsRecording( emulator.rewinder );
		}

		bool Movie::IsInserted() const
		{
			return !emulator.rewinder && emulator.movie;
		}

		bool Movie::IsStopped() const
		{
			return IsStopped( emulator.rewinder );
		}		
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
