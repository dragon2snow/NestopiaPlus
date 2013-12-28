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

#ifndef NST_RESOURCE_VERSION_H
#define NST_RESOURCE_VERSION_H

#pragma once

#include "NstObjectPod.hpp"
#include "NstString.hpp"
#include <Windows.h>

namespace Nestopia
{
	namespace Resource
	{
		class Version : Sealed
		{
		public:

			explicit Version(cstring=NULL);

			enum 
			{
				MAX_SIZE = 8
			};

		private:				 

			ibool InitInfo(cstring);
			void InitString(uint,uint);

			Object::Pod<VS_FIXEDFILEINFO> info;
			Nestopia::String::Stack<MAX_SIZE> string;

		public:

			const VS_FIXEDFILEINFO& GetInfo() const
			{
				return info;
			}

			operator cstring () const
			{
				return string; 
			}

			uint Size() const
			{
				return string.Size();
			}
		};
	}
}

#endif
