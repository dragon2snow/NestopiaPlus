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

#ifndef NST_BOARDS_TAITOTC_H
#define NST_BOARDS_TAITOTC_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE TaitoTc : public Mapper
			{
			protected:
	
				enum Type
				{
					TYPE_TC190_TC0350,
					TYPE_TC190V
				};
	
				TaitoTc(Context&,Type);
				~TaitoTc();
	
			private:
	
				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void VSync();
	
				NES_DECL_POKE( 8000 )
				NES_DECL_POKE( C000 ) 
				NES_DECL_POKE( C001 ) 
				NES_DECL_POKE( C002 ) 
				NES_DECL_POKE( C003 ) 
				NES_DECL_POKE( E000 ) 
	
				Mmc3::Irq* const irq;
			};
		}
	}
}

#endif
