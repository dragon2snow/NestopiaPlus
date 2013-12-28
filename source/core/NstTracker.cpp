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
#include "NstCore.hpp"
#include "api/NstApiEmulator.hpp"
#include "api/NstApiMachine.hpp"
#include "NstTrackerMovie.hpp"
#include "NstTrackerRewinder.hpp"
#include "NstImage.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Tracker::Tracker()
		:
		rewinder      (NULL),
		movie         (NULL),
		rewinderSound (false)
		{}

		Tracker::~Tracker()
		{
			delete rewinder;
			delete movie;
		}

		void Tracker::Unload()
		{
			if (rewinder)
				rewinder->Unload();
			else
				MovieEject();
		}

		void Tracker::Reset(bool hard)
		{
			if (movie)
			{
				if (!movie->Reset( hard ))
					MovieEject();
			}
			else
			{
				RewinderReset();
			}
		}

		void Tracker::Flush()
		{
			RewinderReset();
			MovieCut();
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

		Result Tracker::RewinderEnable(Api::Emulator* emulator)
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
					&Api::Emulator::ExecuteFrame,
					&Api::Emulator::LoadState,
					&Api::Emulator::SaveState,
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

		Result Tracker::MoviePlay(Api::Emulator& emulator,StdStream stream,bool mode)
		{
			if (!rewinder && emulator.Is(Api::Machine::GAME))
			{
				Result result;

				try
				{
					if (movie == NULL)
					{
						movie = new Movie
						(
							emulator,
							&Api::Emulator::Reset,
							&Api::Emulator::LoadState,
							&Api::Emulator::SaveState,
							emulator.cpu,
							emulator.Is(Api::Machine::CARTRIDGE) ? emulator.image->GetPrgCrc() : 0
						);
					}

					result = movie->Play( stream, mode );
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
					if (result != RESULT_NOP && emulator.Is(Api::Machine::ON))
						emulator.Reset( true );
				}
				else
				{
					MovieEject();
				}

				return result;
			}

			return RESULT_ERR_NOT_READY;
		}

		Result Tracker::MovieRecord(Api::Emulator& emulator,StdStream stream,bool how,bool mode)
		{
			if (!rewinder && emulator.Is(Api::Machine::GAME))
			{
				Result result;

				try
				{
					if (movie == NULL)
					{
						movie = new Movie
						(
							emulator,
							&Api::Emulator::Reset,
							&Api::Emulator::LoadState,
							&Api::Emulator::SaveState,
							emulator.cpu,
							emulator.image->GetPrgCrc()
						);
					}

					result = movie->Record( stream, how, mode );
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
					MovieEject();

				return result;
			}

			return RESULT_ERR_NOT_READY;
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

		#ifdef NST_PRAGMA_OPTIMIZE
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

		dword Tracker::GetSoundLatency(const Apu& apu) const
		{
			if (rewinder && rewinder->IsSoundRewinding())
				return rewinder->GetSoundLatency();
			else
				return apu.GetLatency();
		}

		Result Tracker::Execute
		(
			Api::Emulator& emulator,
			Video::Output* const video,
			Sound::Output* const sound,
			Input::Controllers* const input
		)
		{
			if (emulator.Is(Api::Machine::ON))
			{
				if (emulator.Is(Api::Machine::GAME))
				{
					if (rewinder)
					{
						return rewinder->Execute( video, sound, input );
					}
					else if (movie)
					{
						if (!movie->BeginFrame( emulator.frame ))
							MovieEject();

						const Result result = emulator.ExecuteFrame( video, sound, input );

						if (NES_FAILED(result) || !movie->EndFrame())
							MovieEject();

						return result;
					}
				}

				return emulator.ExecuteFrame( video, sound, input );
			}
			else
			{
				return RESULT_ERR_NOT_READY;
			}
		}
	}
}
