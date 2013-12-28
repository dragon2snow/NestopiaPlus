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

#include "../NstCore.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiMachine.hpp"
#include "NstApiNsf.hpp"
#include "../NstNsf.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Api
	{
		cstring Nsf::GetName() const
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetName();
	
			return "";
		}
	
		cstring Nsf::GetArtist() const
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetArtist();
	
			return "";
		}
	
		cstring Nsf::GetMaker() const
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetMaker();
	
			return "";
		}
	
		uint Nsf::GetChips() const
		{
			if (emulator.Is(Machine::SOUND))
			{
				NST_COMPILE_ASSERT
				(
					CHIP_VRC6  == Core::Nsf::CHIP_VRC6  &&
					CHIP_VRC7  == Core::Nsf::CHIP_VRC7  &&
					CHIP_FDS   == Core::Nsf::CHIP_FDS   &&
					CHIP_MMC5  == Core::Nsf::CHIP_MMC5  &&
					CHIP_N106  == Core::Nsf::CHIP_N106  &&
					CHIP_FME07 == Core::Nsf::CHIP_FME07
				);
	
				return (Chip) static_cast<const Core::Nsf*>(emulator.image)->GetChips();
			}
	
			return 0;
		}
	
		Nsf::TuneMode Nsf::GetMode() const
		{
			NST_COMPILE_ASSERT
			( 
				TUNE_MODE_NTSC == Core::Nsf::TUNE_MODE_NTSC &&
				TUNE_MODE_PAL  == Core::Nsf::TUNE_MODE_PAL &&
				TUNE_MODE_BOTH == Core::Nsf::TUNE_MODE_BOTH
			);
	
			if (emulator.Is(Machine::SOUND))
				return (TuneMode) static_cast<const Core::Nsf*>(emulator.image)->GetTuneMode();
	
			return TUNE_MODE_NTSC;
		}
	
		uint Nsf::GetNumSongs() const
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->NumSongs();
	
			return 0;
		}
	
		int Nsf::GetCurrentSong() const
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->CurrentSong(); 
	
			return NO_SONG;
		}
	
		int Nsf::GetStartingSong() const
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->StartingSong(); 
	
			return NO_SONG;
		}
	
		uint Nsf::GetInitAddress() const
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetInitAddress(); 
	
			return 0x0000;
		}
	
		uint Nsf::GetLoadAddress() const
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetLoadAddress(); 
	
			return 0x0000;
		}
	
		uint Nsf::GetPlayAddress() const
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetPlayAddress(); 
	
			return 0x0000;
		}
	
		Result Nsf::SelectSong(uint song)
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<Core::Nsf*>(emulator.image)->SelectSong( song );
	
			return RESULT_ERR_NOT_READY;
		}
	
		Result Nsf::PlaySong()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<Core::Nsf*>(emulator.image)->PlaySong();
	
			return RESULT_ERR_NOT_READY;
		}
	
		Result Nsf::StopSong()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<Core::Nsf*>(emulator.image)->StopSong();
	
			return RESULT_ERR_NOT_READY;
		}
	
		Result Nsf::SelectNextSong()
		{
			if (emulator.Is(Machine::SOUND))
			{
				return static_cast<Core::Nsf*>(emulator.image)->SelectSong
				( 
					static_cast<const Core::Nsf*>(emulator.image)->CurrentSong() + 1U 
				);
			}
	
			return RESULT_ERR_NOT_READY;
		}
	
		Result Nsf::SelectPrevSong()
		{
			if (emulator.Is(Machine::SOUND))
			{
				return static_cast<Core::Nsf*>(emulator.image)->SelectSong
				( 
					static_cast<const Core::Nsf*>(emulator.image)->CurrentSong() - 1U 
				);
			}
	
			return RESULT_ERR_NOT_READY;
		}
	
		bool Nsf::IsPlaying() const
		{
			return emulator.Is(Machine::SOUND) && static_cast<Core::Nsf*>(emulator.image)->IsPlaying();
		}
	
		bool Nsf::UsesBankSwitching() const
		{
			return emulator.Is(Machine::SOUND) && static_cast<Core::Nsf*>(emulator.image)->UsesBankSwitching();
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
