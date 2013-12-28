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

#ifndef NST_SYSTEM_REGISTRY_H
#define NST_SYSTEM_REGISTRY_H

#pragma once

#include "NstString.hpp"

namespace Nestopia
{
	namespace System
	{
		class Registry : Sealed
		{
		public:

			enum
			{
				MAX_STACK = 8
			};

			static void UpdateAssociations();

		private:

			enum
			{
				ASSOCIATION_CHANGED = 0x2
			};

			class Key
			{
			public:

				ibool operator << (String::Generic) const;
				ibool operator >> (String::Heap&) const;

				void Delete() const;
				void Delete(String::Generic) const;

			private:

				struct AutoKey;

				const uint count;
				cstring stack[MAX_STACK];

			public:

				Key(const Key& NST_RESTRICT parent,cstring name)
				: count(parent.count + 1)
				{
					NST_ASSERT( count <= MAX_STACK );
					std::memcpy( stack, parent.stack, parent.count * sizeof(char*) );
					stack[parent.count] = name;
				}

				Key(const Key& NST_RESTRICT parent)
				: count(parent.count)
				{
					std::memcpy( stack, parent.stack, parent.count * sizeof(char*) );
				}

				Key(cstring name) 
				: count(1)
				{
					stack[0] = name;
				}

				Key operator [] (cstring name) const
				{
					return Key( *this, name );
				}
			};

		public:

			Key operator [] (cstring name) const
			{
				return name;
			}
		};
	}
}

#endif
