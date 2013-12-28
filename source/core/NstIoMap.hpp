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

#ifndef NST_IO_MAP_H
#define NST_IO_MAP_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstIoPort.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Io
		{
			template<dword SIZE> class Map
			{
			protected:

				enum
				{
					OVERFLOW_SIZE = 0x100,
					JAM_OPCODE = 0x02
				};

				Io::Port ports[SIZE + OVERFLOW_SIZE];

			public:

				typedef Io::Port& Port;

				class Ports
				{
					friend class Map;

					Io::Port* const begin;
					const Io::Port* const end;

					Ports(Io::Port* b,const Io::Port* e)
					: begin(b), end(e) {}

				public:

					template<typename A,typename B,typename C>
					void Set(A a,B b,C c) const
					{
						for (Io::Port* port=begin; port != end; ++port)
							port->Set( a, b, c );
					}

					template<typename A,typename B>
					void Set(A a,B b) const
					{
						for (Io::Port* port=begin; port != end; ++port)
							port->Set( a, b );
					}

					template<typename A>
					void Set(A a) const
					{
						for (Io::Port* port=begin; port != end; ++port)
							port->Set( a );
					}
				};

				template<typename A,typename B,typename C> 
				Map(A a,B b,C c)
				{
					for (dword i=SIZE; i < SIZE + OVERFLOW_SIZE; ++i)
						ports[i].Set( a, b, c );
				}

				Io::Port& operator [] (Address address)
				{
					NST_ASSERT( address < SIZE + OVERFLOW_SIZE );
					return ports[address];
				}

				const Io::Port& operator [] (Address address) const
				{
					NST_ASSERT( address < SIZE + OVERFLOW_SIZE );
					return ports[address];
				}

				Port operator () (Address address)
				{
					NST_ASSERT( address < SIZE + OVERFLOW_SIZE );
					return ports[address];
				}

				Ports operator () (Address first,Address last)
				{
					NST_ASSERT( first <= last && last < SIZE );
					return Ports( ports + first, ports + last + 1 );
				}

				void Poke(Address address,Data data)
				{
					NST_ASSERT( address < SIZE + OVERFLOW_SIZE );
					ports[address].Poke( address, data );
				}

				Data Peek(Address address) const
				{
					NST_ASSERT( address < SIZE + OVERFLOW_SIZE );
					return ports[address].Peek( address );
				}
			};
		}
	}
}

#endif
