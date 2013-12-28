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

#ifndef NST_APPLICATION_CONFIGURATION_H
#define NST_APPLICATION_CONFIGURATION_H

#pragma once

#include "NstCollectionMap.hpp"
#include "NstString.hpp"

namespace Nestopia
{
	namespace Application
	{
		class Configuration : Sealed
		{
		public:

			explicit Configuration(String::Generic);
			~Configuration();

			void Reset(ibool=TRUE);

			enum State
			{
				YES,
				NO,
				ON,
				OFF
			};

		private:

			class ConstValue
			{
			public:

				ibool operator == (State) const;

			private:

				const String::Heap& string;

			public:

				ConstValue(const String::Heap& s)
				: string(s) {}

				operator bool () const 
				{
					return string.Size() != 0;
				}

				ibool operator ! () const
				{
					return string.Empty();
				}

				uint Size() const
				{
					return string.Size();
				}
  
				operator cstring () const 
				{
					return string;
				}

				const String::Heap& GetString() const
				{
					return string;
				}

				operator long () const 
				{ 
					long i; 
					string >> i; 
					return i; 
				}

				operator ulong () const 
				{ 
					ulong i; 
					string >> i; 
					return i; 
				}

				operator schar  () const { return ( schar  ) operator long  (); }
				operator uchar  () const { return ( uchar  ) operator ulong (); }
				operator short  () const { return ( short  ) operator long  (); }
				operator ushort () const { return ( ushort ) operator ulong (); }
				operator int    () const { return ( int    ) operator long  (); }
				operator uint   () const { return ( uint   ) operator ulong (); }

				cstring Default(cstring d) const 
				{ 
					return string.Size() ? string : d;
				}

				char Default(char d) const 
				{
					return string.Size() ? string[0] : d;
				}

				long Default(long d) const
				{
					long i;
					return (string >> i) ? i : d;
				}

				ulong Default(ulong d) const
				{
					ulong i;
					return (string >> i) ? i : d;
				}

				schar  Default( schar  i ) const { return ( schar  ) Default( ( long  ) i ); }
				uchar  Default( uchar  i ) const { return ( uchar  ) Default( ( ulong ) i ); }
				short  Default( short  i ) const { return ( short  ) Default( ( long  ) i ); }
				ushort Default( ushort i ) const { return ( ushort ) Default( ( ulong ) i ); }
				int    Default( int    i ) const { return ( int    ) Default( ( long  ) i ); }
				uint   Default( uint   i ) const { return ( uint   ) Default( ( ulong ) i ); }

				ibool operator != (State state) const 
				{ 
					return !(*this == state); 
				}
			};

			class Value
			{
				String::Heap& string;

			public:

				Value(String::Heap& s)
				: string(s)	{}

				class QuoteProxy
				{
					String::Heap& string;

				public:

					QuoteProxy(String::Heap& s)
					: string(s) {}

					void operator = (const String::Anything&);
				};

				class YesNoProxy
				{
					String::Heap& string;

				public:

					YesNoProxy(String::Heap& s)
					: string(s) {}

					void operator = (ibool);
				};

				class OnOffProxy
				{
					String::Heap& string;

				public:

					OnOffProxy(String::Heap& s)
					: string(s) {}

					void operator = (ibool);
				};

				void operator = (const String::Anything& input)
				{
					string = input;
				}

				Value& operator << (const String::Anything& input)
				{
					string << input;
					return *this;
				}

				uint Size() const
				{
					return string.Size();
				}

				String::Heap& GetString()
				{
					return string;
				}

				QuoteProxy Quote()
				{
					return string;
				}

				YesNoProxy YesNo()
				{
					return string;
				}

				OnOffProxy OnOff()
				{
					return string;
				}
			};

		public:

			Value operator [] (cstring);
			ConstValue operator [] (cstring) const throw();

		private:

			void Parse(cstring,uint,String::Heap* = NULL);

			enum Exception
			{
				ERR_PARSING
			};

			enum
			{
				HINTED_SIZE = 290
			};

			struct Command : String::Heap
			{
				mutable ibool referenced;

				template<typename T> 
				Command(const T& t)
				: String::Heap(t), referenced(FALSE) {}
			};

			typedef Collection::Map<Command,String::Heap> Items;

			Items items;
			String::Heap startupFile;
			ibool save;

			static const String::Heap nullString;

		public:

			void EnableSaving(ibool enable=TRUE)
			{
				save = enable;
			}

			const String::Heap& GetStartupFile() const
			{
				return startupFile;
			}
		};
	}
}

#endif
