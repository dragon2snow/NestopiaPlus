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

#ifndef NST_BOARDS_JALECO_H
#define NST_BOARDS_JALECO_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			class Player;
		}

		namespace Boards
		{
			class Jaleco : public Mapper
			{
			protected:

				enum Type
				{
					TYPE_0,
					TYPE_1,
					TYPE_2
				};

				Jaleco(Context&,Type);
				~Jaleco();

			private:

				static Sound::Player* DetectSound(Type,uint,Apu&);

				enum
				{
					ATR_TYPE_0_SAMPLES_MPT    = 1,
					ATR_TYPE_1_SAMPLES_MPY88K = 1,
					ATR_TYPE_2_SAMPLES_MPY    = 1
				};

				void SubReset(bool);

				NES_DECL_POKE( 6000 );
				NES_DECL_POKE( 7000 );
				NES_DECL_POKE( 8000 );

				Sound::Player* const sound;
				const uint prgOffset;
				const Type type;
			};
		}
	}
}

#endif
