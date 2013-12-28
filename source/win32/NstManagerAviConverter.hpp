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

#ifndef NST_MANAGER_AVICONVERTER_H
#define NST_MANAGER_AVICONVERTER_H

#pragma once

#include "NstObjectHeap.hpp"
#include "NstManagerEmulator.hpp"
#include "../core/api/NstApiMovie.hpp"

namespace Nestopia
{
	namespace Managers
	{
		class AviConverter
		{
		public:

			AviConverter(Emulator&,std::fstream&);
			~AviConverter();

			uint Record(const Path&) const;

		private:

			enum
			{
				MAX_FILE_SIZE = 0x40000000,
				VIDEO_BPP = 16,
				VIDEO_R_MASK = Nes::b11111 << 10,
				VIDEO_G_MASK = Nes::b11111 << 5,
				VIDEO_B_MASK = Nes::b11111 << 0
			};

			Emulator& emulator;
			const ibool on;

			Nes::Video::RenderState renderState;
			Nes::Video::Output::LockCallback nesVideoLockFunc;
			void* nesVideoLockData;
			Nes::Video::Output::UnlockCallback nesVideoUnlockFunc;
			void* nesVideoUnlockData;
			Nes::Sound::Output::LockCallback nesSoundLockFunc;
			void* nesSoundLockData;
			Nes::Sound::Output::UnlockCallback nesSoundUnlockFunc;
			void* nesSoundUnlockData;
			Nes::Movie::StateCallback nesMovieStateFunc;
			void* nesMovieStateData;

			Collection::Buffer saveState;
		};
	}
}

#endif
