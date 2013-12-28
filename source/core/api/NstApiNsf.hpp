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

#ifndef NST_API_NSF_H
#define NST_API_NSF_H

#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		class Nsf : public Base
		{
		public:

			template<typename T>
			Nsf(T& e)
			: Base(e) {}

			enum
			{
				NO_SONG = -1
			};

			enum TuneMode
			{
				TUNE_MODE_NTSC,
				TUNE_MODE_PAL,
				TUNE_MODE_BOTH
			};

			enum
			{
				CHIP_VRC6 = 0x01,
				CHIP_VRC7 = 0x02,
				CHIP_FDS  = 0x04,
				CHIP_MMC5 = 0x08,
				CHIP_N106 = 0x10,
				CHIP_S5B  = 0x20,
				CHIP_ALL  = 0x3F
			};

			const char* GetName         () const throw();
			const char* GetArtist       () const throw();
			const char* GetMaker        () const throw();
			TuneMode    GetMode         () const throw();
			uint        GetInitAddress  () const throw();
			uint        GetLoadAddress  () const throw();
			uint        GetPlayAddress  () const throw();
			uint        GetNumSongs     () const throw();
			int         GetCurrentSong  () const throw();
			int         GetStartingSong () const throw();

			uint GetChips() const throw();
			bool IsPlaying() const throw();
			bool UsesBankSwitching() const throw();

			Result SelectSong(uint) throw();
			Result SelectNextSong() throw();
			Result SelectPrevSong() throw();
			Result PlaySong() throw();
			Result StopSong() throw();
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
