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

			Configuration();
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

			class ConstValue : public GenericString
			{
			public:

				ConstValue(const GenericString& g)
				: Generic(g) {}

				bool operator == (State) const;

				bool operator != (State state) const 
				{ 
					return !(*this == state); 
				}

				operator long () const 
				{ 
					long i; 
					*this >> i; 
					return i; 
				}

				operator ulong () const 
				{ 
					ulong i; 
					*this >> i; 
					return i; 
				}

				operator schar  () const { return ( schar  ) operator long  (); }
				operator uchar  () const { return ( uchar  ) operator ulong (); }
				operator short  () const { return ( short  ) operator long  (); }
				operator ushort () const { return ( ushort ) operator ulong (); }
				operator int    () const { return ( int    ) operator long  (); }
				operator uint   () const { return ( uint   ) operator ulong (); }
	  
				tstring Default(tstring d) const 
				{ 
					return Length() ? Ptr() : d;
				}

				tchar Default(tchar d) const 
				{
					return Length() ? Front() : d;
				}

				long Default(long d) const
				{
					long i;
					return (*this >> i) ? i : d;
				}

				ulong Default(ulong d) const
				{
					ulong i;
					return (*this >> i) ? i : d;
				}

				schar  Default( schar  i ) const { return ( schar  ) Default( ( long  ) i ); }
				uchar  Default( uchar  i ) const { return ( uchar  ) Default( ( ulong ) i ); }
				short  Default( short  i ) const { return ( short  ) Default( ( long  ) i ); }
				ushort Default( ushort i ) const { return ( ushort ) Default( ( ulong ) i ); }
				int    Default( int    i ) const { return ( int    ) Default( ( long  ) i ); }
				uint   Default( uint   i ) const { return ( uint   ) Default( ( ulong ) i ); }
			};

			class Value
			{
				HeapString& string;

			public:

				Value(HeapString& s)
				: string(s)	{}

				class QuoteProxy
				{
					HeapString& string;

				public:

					QuoteProxy(HeapString& s)
					: string(s) {}

					void operator = (const GenericString&);
				};

				class YesNoProxy
				{
					HeapString& string;

				public:

					YesNoProxy(HeapString& s)
					: string(s) {}

					void operator = (ibool);
				};

				class OnOffProxy
				{
					HeapString& string;

				public:

					OnOffProxy(HeapString& s)
					: string(s) {}

					void operator = (ibool);
				};

				template<typename T>
				void operator = (const T& t)
				{
					string << t;
				}

				HeapString& GetString()
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

			Value operator [] (const String::Generic<char>);
			const ConstValue operator [] (const String::Generic<char>) const;

		private:

			enum
			{
				UTF16_LE = 0xFEFF,
				UTF16_BE = 0xFFFE
			};

			void Parse(tstring,uint);

			enum Exception
			{
				ERR_PARSING
			};

			enum
			{
				HINTED_SIZE = 360
			};

			struct Command : String::Heap<char>
			{
				mutable ibool referenced;

				template<typename T> 
				Command(const T& t)
				: String::Heap<char>(t), referenced(FALSE) {}
			};

			typedef Collection::Map< Command, HeapString > Items;

			Items items;
			HeapString startupFile;
			ibool save;

		public:

			void EnableSaving(ibool enable=TRUE)
			{
				save = enable;
			}

			const HeapString& GetStartupFile() const
			{
				return startupFile;
			}
		};
	}
}

#endif
