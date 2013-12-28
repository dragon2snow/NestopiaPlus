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

#ifndef NST_OBJECT_BACKUP_H
#define NST_OBJECT_BACKUP_H

#pragma once

#include <cstring>
#include "NstMain.hpp"

namespace Nestopia
{
	namespace Object
	{
		class Backup : Sealed
		{
			const uint size;
			void* const data;
			void* const backup;

		public:

			Backup(void* p,uint s)
			: size(s), data(p), backup(std::memcpy( operator new (s), p, s )) {}

			~Backup()
			{ 
				operator delete (backup); 
			}

			void Restore() const
			{
				std::memcpy( data, backup, size );
			}
		};
	}
}

#endif
