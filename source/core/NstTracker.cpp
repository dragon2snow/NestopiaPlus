////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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
#include "NstMachine.hpp"
#include "NstTrackerMovie.hpp"
#include "NstTrackerRewinder.hpp"
#include "NstImage.hpp"
#include "api/NstApiMachine.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Tracker::Tracker()
		:
		frame         (0),
		rewinderSound (false),
		rewinder      (NULL),
		movie         (NULL)
		{}

		Tracker::~Tracker()
		{
			delete rewinder;
			delete movie;
		}

		void Tracker::Unload()
		{
			frame = 0;

			if (rewinder)
				rewinder->Unload();
			else
				MovieEject();
		}

		void Tracker::Reset(bool hard)
		{
			frame = 0;

			if (!movie)
			{
				RewinderReset();
			}
			else if (!movie->Reset( hard ))
			{
				MovieEject();
			}
		}

		void Tracker::Flush()
		{
			RewinderReset();
			MovieCut();
		}

		Result Tracker::Flush(Result lastResult)
		{
			NST_VERIFY( NES_SUCCEEDED(lastResult) );

			if (NES_SUCCEEDED(lastResult) && lastResult != RESULT_NOP)
				Flush();

			return lastResult;
		}

		void Tracker::RewinderReset() const
		{
			if (rewinder)
				rewinder->Reset();
		}

		void Tracker::RewinderEnableSound(bool enable)
		{
			rewinderSound = enable;

			if (rewinder)
				rewinder->EnableSound( enable );
		}

		Result Tracker::RewinderEnable(Machine* emulator)
		{
			if (bool(rewinder) == bool(emulator))
				return RESULT_NOP;

			if (movie)
				return RESULT_ERR_NOT_READY;

			if (emulator)
			{
				rewinder = new Rewinder
				(
					*emulator,
					&Machine::ExecuteFrame,
					&Machine::LoadState,
					&Machine::SaveState,
					emulator->cpu,
					emulator->ppu,
					rewinderSound
				);
			}
			else
			{
				delete rewinder;
				rewinder = NULL;
			}

			return RESULT_OK;
		}

		Result Tracker::MoviePlay(Machine& emulator,StdStream stream,bool mode)
		{
			if (rewinder || !emulator.Is(Api::Machine::GAME))
				return RESULT_ERR_NOT_READY;

			Result result;

			try
			{
				if (movie == NULL)
				{
					movie = new Movie
					(
						emulator,
						&Machine::Reset,
						&Machine::LoadState,
						&Machine::SaveState,
						emulator.cpu,
						emulator.Is(Api::Machine::CARTRIDGE) ? emulator.image->GetPrgCrc() : 0
					);
				}

				if (movie->Play( stream, mode ))
				{
					if (emulator.Is(Api::Machine::ON))
						emulator.Reset( true );

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

			MovieEject();

			return result;
		}

		Result Tracker::MovieRecord(Machine& emulator,StdStream stream,bool how,bool mode)
		{
			if (rewinder || !emulator.Is(Api::Machine::GAME))
				return RESULT_ERR_NOT_READY;

			Result result;

			try
			{
				if (movie == NULL)
				{
					movie = new Movie
					(
						emulator,
						&Machine::Reset,
						&Machine::LoadState,
						&Machine::SaveState,
						emulator.cpu,
						emulator.image->GetPrgCrc()
					);
				}

				return movie->Record( stream, how, mode ) ? RESULT_OK : RESULT_NOP;
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

			MovieEject();

			return result;
		}

		void Tracker::MovieStop()
		{
			if (movie)
				movie->Stop();
		}

		void Tracker::MovieCut()
		{
			if (movie)
				movie->Cut();
		}

		void Tracker::MovieEject()
		{
			delete movie;
			movie = NULL;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Result Tracker::RewinderStart() const
		{
			return rewinder ? rewinder->Start() : RESULT_ERR_NOT_READY;
		}

		Result Tracker::RewinderStop() const
		{
			return rewinder ? rewinder->Stop() : RESULT_NOP;
		}

		bool Tracker::IsRewinding() const
		{
			return rewinder && rewinder->IsRewinding();
		}

		bool Tracker::MovieIsPlaying() const
		{
			return movie && movie->IsPlaying();
		}

		bool Tracker::MovieIsRecording() const
		{
			return movie && movie->IsRecording();
		}

		bool Tracker::MovieIsStopped() const
		{
			return !movie || movie->IsStopped();
		}

		bool Tracker::IsLocked() const
		{
			return IsRewinding() || MovieIsPlaying();
		}

		Result Tracker::Execute
		(
			Machine& emulator,
			Video::Output* const video,
			Sound::Output* const sound,
			Input::Controllers* const input
		)
		{
			if (emulator.Is(Api::Machine::ON))
			{
				++frame;

				if (emulator.Is(Api::Machine::GAME))
				{
					if (rewinder)
					{
						rewinder->Execute( video, sound, input );
						return RESULT_OK;
					}
					else if (movie)
					{
						if (!movie->BeginFrame( emulator.frame ))
							MovieEject();

						emulator.ExecuteFrame( video, sound, input );

						if (!movie->EndFrame())
							MovieEject();

						return RESULT_OK;
					}
				}

				emulator.ExecuteFrame( video, sound, input );
				return RESULT_OK;
			}
			else
			{
				return RESULT_ERR_NOT_READY;
			}
		}
	}
}
