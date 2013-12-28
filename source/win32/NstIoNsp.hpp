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

#ifndef NST_IO_NSP_H
#define NST_IO_NSP_H

#pragma once

#include <vector>
#include "resource/resource.h"
#include "NstString.hpp"
#include "../core/api/NstApiInput.hpp"

namespace Nes
{
	using namespace Api;
}

namespace Nestopia
{
	namespace Io
	{
		namespace Nsp
		{
			struct Context : private Sealed
			{
				Context();
				~Context();

				void Reset();

				enum
				{
					NUM_CONTROLLERS = Nes::Input::NUM_CONTROLLERS,
					NUM_CONTROLLER_PORTS = Nes::Input::NUM_PORTS,
					UNKNOWN = INT_MAX
				};

				struct Cheat
				{
					typedef String::Heap Desc;

					u16 address;
					u8 value;
					u8 compare;
					bool useCompare;
					bool enabled;
					Desc desc;

					ibool operator == (u16 a) const
					{
						return address == a;
					}
				};

				typedef std::vector<Cheat> Cheats;
				typedef String::Path<false> Path;

				Path image;
				Path ips;
				Path save;
				Path state;
				Path movie;
				uint controllers[NUM_CONTROLLER_PORTS];
				uint mode;
				Cheats cheats;
			};

			class File : Sealed
			{
			public:				

				enum Exception
				{
					ERR_EMPTY = IDS_FILE_ERR_EMPTY,
					ERR_SYNTAX = IDS_ERR_SYNTAX
				};

				typedef String::Generic Input;
				typedef String::Heap Output;
				typedef Output Buffer;

				void Load(const Input,Context&) const;
				void Save(Output&,const Context&) const;

			private:

				class Parser;

				static ibool Match (cstring,cstring (&)[2]);
				static ibool ParseFile (cstring,cstring (&)[2][2],Context::Path&);
				static ibool ParsePort (cstring,cstring (&)[2][2],uint&);
				static ibool ParseMode (cstring,cstring (&)[2][2],uint&);
				static ibool ParseGenie (cstring,cstring (&)[2][2],Context::Cheats&,bool=false);
				static ibool ParseCheat (cstring,cstring (&)[2][2],Context::Cheats&);

				static void Skip(cstring&,cstring);

				static cstring const controllerNames[];
			};
		}
	}
}

#endif
