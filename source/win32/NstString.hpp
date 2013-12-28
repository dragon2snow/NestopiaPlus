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

#ifndef NST_STRING_H
#define NST_STRING_H

#pragma once

#include <cstdlib>
#include <cstring>
#include "NstMain.hpp"

// pretty sure MS is wrong and everyone else is right about this one

#ifdef __INTEL_COMPILER
#define NST_CRAPPY_FRIEND template<typename> 
#elif defined(_MSC_VER)
#define NST_CRAPPY_FRIEND
#else
#define NST_CRAPPY_FRIEND template<typename>
#endif

namespace Nestopia
{
	namespace String
	{			
		int Compare(cstring,cstring) throw();
		int Compare(cstring,cstring,uint) throw();
		int CompareNonNull(cstring,cstring,uint) throw();
		int CompareNonNull(cstring,cstring,uint,uint) throw();

		namespace Private
		{
			uint Length (cstring) throw();
			uint Copy   (char* NST_RESTRICT,cstring NST_RESTRICT) throw();
			void Insert (char* NST_RESTRICT,uint,uint,cstring NST_RESTRICT,uint) throw();
			void Erase  (char*,uint,uint,uint) throw();
			uint Remove (char*,uint,int) throw();

			uint FindFirstOf      (int,cstring,uint) throw();
			uint FindAfterFirstOf (int,cstring,uint) throw();
			uint FindFirstNotOf   (int,cstring,uint) throw();
			uint FindLastOf       (int,cstring,uint) throw();
			uint FindAfterLastOf  (int,cstring,uint) throw();
			uint FindLastNotOf    (int,cstring,uint) throw();

			void MakeLowerCase (char* NST_RESTRICT,uint) throw();
			void MakeUpperCase (char* NST_RESTRICT,uint) throw();

			ibool ToUnsigned (cstring NST_RESTRICT,u32& NST_RESTRICT) throw();	
			char* FromSigned (char (&)[12],i32) throw();
			char* FromUnsigned (char (&)[12],u32) throw();

			uint Trim (char*,uint) throw();
			cstring Find (cstring,cstring,uint) throw();

			uint ExtensionId (cstring NST_RESTRICT) throw();
			ibool FileExist (cstring);
			ibool DirExist (cstring);

			template<typename A,bool X=false> struct Comparer
			{
				template<typename B>
				static ibool IsEqual(const A& a,const B& b)
				{ 
					return a.Size() == b.Size() && Compare( a, b, a.Size() ) == 0; 
				}

				template<>
				static ibool IsEqual(const A& a,const char* const& b)
				{ 
					return Compare( a, b ) == 0; 
				}

				template<>
				static ibool IsEqual(const A& a,char* const& b)
				{ 
					return Compare( a, b ) == 0; 
				}

				template<size_t N>
				static ibool IsEqual(const A& a,const char (&b)[N])
				{ 
					return a.Size() == N-1 && Compare( a, b ) == 0; 
				}

				template<typename B>
				static ibool IsLess(const A& a,const B& b)
				{
					return CompareNonNull( b, a, b.Size() ) > 0; 
				}

				template<>
				static ibool IsLess(const A& a,const char* const& b)
				{
					return Compare( a, b ) < 0; 
				}

				template<>
				static ibool IsLess(const A& a,char* const& b)
				{
					return Compare( a, b ) < 0; 
				}

				template<size_t N>
				static ibool IsLess(const A& a,const char (&b)[N])
				{
					return Compare( a, b ) < 0; 
				}

				template<typename B>
				static ibool IsLessOrEqual(const A& a,const B& b)
				{
					return CompareNonNull( b, a, b.Size() ) > 0; 
				}

				template<>
				static ibool IsLessOrEqual(const A& a,const char* const& b)
				{
					return Compare( a, b ) <= 0; 
				}

				template<>
				static ibool IsLessOrEqual(const A& a,char* const& b)
				{
					return Compare( a, b ) <= 0; 
				}

				template<size_t N>
				static ibool IsLessOrEqual(const A& a,const char (&b)[N])
				{
					return Compare( a, b ) <= 0; 
				}
			};

			template<typename A> struct Comparer<A,true>
			{
				template<typename B>
				static ibool IsEqual(const A& a,const B& b)
				{ 
					return a.Size() == b.Size() && Compare( a, b, a.Size() ) == 0; 
				}

				template<>
				static ibool IsEqual(const A& a,const char* const& b)
				{ 
					return CompareNonNull( a, b, a.Size() ) == 0; 
				}

				template<>
				static ibool IsEqual(const A& a,char* const& b)
				{ 
					return CompareNonNull( a, b, a.Size() ) == 0; 
				}

				template<size_t N>
				static ibool IsEqual(const A& a,const char (&b)[N])
				{ 
					return a.Size() == N-1 && Compare( a, b, N-1 ) == 0; 
				}

				template<typename B>
				static ibool IsLess(const A& a,const B& b)
				{
					return CompareNonNull( a, b, a.Size(), b.Size() ) < 0; 
				}

				template<>
				static ibool IsLess(const A& a,const char* const& b)
				{
					return CompareNonNull( a, b, a.Size() ) < 0; 
				}

				template<>
				static ibool IsLess(const A& a,char* const& b)
				{
					return CompareNonNull( a, b, a.Size() ) < 0; 
				}

				template<size_t N>
				static ibool IsLess(const A& a,const char (&b)[N])
				{
					return CompareNonNull( a, b, a.Size() ) < 0; 
				}

				template<typename B>
				static ibool IsLessOrEqual(const A& a,const B& b)
				{
					return CompareNonNull( a, b, a.Size(), b.Size() ) <= 0; 
				}

				template<>
				static ibool IsLessOrEqual(const A& a,const char* const& b)
				{
					return CompareNonNull( a, b, a.Size() ) <= 0; 
				}

				template<>
				static ibool IsLessOrEqual(const A& a,char* const& b)
				{
					return CompareNonNull( a, b, a.Size() ) <= 0; 
				}

				template<size_t N>
				static ibool IsLessOrEqual(const A& a,const char (&b)[N])
				{
					return CompareNonNull( a, b, a.Size() ) <= 0; 
				}
			};
		}

		class Generic
		{
		protected:

			cstring string;
			uint size;

			static const char empty = '\0';

			enum Nop {NOP};
			explicit Generic(Nop) {}

		public:

			Generic()
			: string(&empty), size(0) {}

			template<typename T> 
			Generic(const T& t)
			: string(t), size(t.Size()) {}

			template<> 
			Generic(const char* const& c)
			: string(c), size(Private::Length(c)) {}

			template<> 
			Generic(char* const& c)
			: string(c), size(Private::Length(c)) {}

			template<size_t N>
			Generic(const char (&c)[N])
			: string(c), size(N-1) {}

			template<>
			Generic(const char& c)
			: string(&c), size(1) {}

			Generic(cstring c,uint s)
			: string(c), size(s) {}

			template<typename T>
			Generic& operator = (const T& t)
			{
				string = t;
				size = t.Size();
				return *this;
			}

			template<>
			Generic& operator = (const char* const& c)
			{
				string = c;
				size = Private::Length( c );
				return *this;
			}

			template<>
			Generic& operator = (char* const& c)
			{
				string = c;
				size = Private::Length( c );
				return *this;
			}

			template<size_t N>
			Generic& operator = (const char (&c)[N])
			{
				string = c;
				size = N-1;
				return *this;
			}

			template<>
			Generic& operator = (const char& c)
			{
				string = &c;
				size = 1;
				return *this;
			}

			void Reset()
			{
				string = &empty;
				size = 0;
			}

			operator cstring () const
			{
				return string;
			}

			uint Size() const
			{
				return size;
			}

			ibool Empty() const
			{
				return !size;
			}

			int Back() const
			{
				NST_ASSERT( size );
				return string[size-1];
			}

			ibool IsNullTerminated() const
			{
				return string[size] == '\0';
			}

			template<typename T> ibool operator == (const T& t) const { return  Private::Comparer<Generic,true>::IsEqual       ( *this, t ); }
			template<typename T> ibool operator != (const T& t) const { return !Private::Comparer<Generic,true>::IsEqual       ( *this, t ); }
			template<typename T> ibool operator <  (const T& t) const { return  Private::Comparer<Generic,true>::IsLess        ( *this, t ); }
			template<typename T> ibool operator <= (const T& t) const { return  Private::Comparer<Generic,true>::IsLessOrEqual ( *this, t ); }
			template<typename T> ibool operator >  (const T& t) const { return !Private::Comparer<Generic,true>::IsLessOrEqual ( *this, t ); }
			template<typename T> ibool operator >= (const T& t) const { return !Private::Comparer<Generic,true>::IsLess        ( *this, t ); }

			friend ibool operator == (const char* c,const Generic& s) { return s == c; }
			friend ibool operator != (const char* c,const Generic& s) { return s != c; }
			friend ibool operator <  (const char* c,const Generic& s) { return s >= c; }
			friend ibool operator <= (const char* c,const Generic& s) { return s >  c; }
			friend ibool operator >  (const char* c,const Generic& s) { return s <= c; }
			friend ibool operator >= (const char* c,const Generic& s) { return s <  c; }

			friend ibool operator == (char* c,const Generic& s) { return s == c; }
			friend ibool operator != (char* c,const Generic& s) { return s != c; }
			friend ibool operator <  (char* c,const Generic& s) { return s >= c; }
			friend ibool operator <= (char* c,const Generic& s) { return s >  c; }
			friend ibool operator >  (char* c,const Generic& s) { return s <= c; }
			friend ibool operator >= (char* c,const Generic& s) { return s <  c; }

			Generic operator () (uint pos) const
			{
				return Generic( string + pos, size - pos );
			}

			Generic operator () (uint pos,uint length) const
			{
				return Generic( string + pos, length );
			}
		};

		class Anything : public Generic
		{									  
			Anything(const Anything&);
			void operator = (const Anything&);

			char buffer[12];

			void SetValue(i32 number)
			{
				string = Private::FromSigned( buffer, number ); 
				size = buffer + 11 - string; 
			}

			void SetValue(u32 number)
			{
				string = Private::FromUnsigned( buffer, number ); 
				size = buffer + 11 - string; 
			}

		public:

			template<typename T> 
			Anything(const T& t)
			: Generic(t) {}

			Anything( schar  i ) : Generic(NOP) { SetValue( (i32) i ); } 
			Anything( uchar  i ) : Generic(NOP) { SetValue( (u32) i ); } 
			Anything( short  i ) : Generic(NOP) { SetValue( (i32) i ); } 
			Anything( ushort i ) : Generic(NOP) { SetValue( (u32) i ); } 
			Anything( int    i ) : Generic(NOP) { SetValue( (i32) i ); } 
			Anything( uint   i ) : Generic(NOP) { SetValue( (u32) i ); } 
			Anything( long   i ) : Generic(NOP) { SetValue( (i32) i ); } 
			Anything( ulong  i ) : Generic(NOP) { SetValue( (u32) i ); } 

			template<typename T>
			Anything& operator = (const T& t)
			{
				Generic::operator = (t);
				return *this;
			}

			Anything& operator = ( schar  i ) { SetValue( (i32) i ); return *this; }
			Anything& operator = ( uchar  i ) { SetValue( (u32) i ); return *this; }
			Anything& operator = ( short  i ) { SetValue( (i32) i ); return *this; }
			Anything& operator = ( ushort i ) { SetValue( (u32) i ); return *this; }
			Anything& operator = ( int    i ) { SetValue( (i32) i ); return *this; }
			Anything& operator = ( uint   i ) { SetValue( (u32) i ); return *this; }
			Anything& operator = ( long   i ) { SetValue( (i32) i ); return *this; }
			Anything& operator = ( ulong  i ) { SetValue( (u32) i ); return *this; }
		};

		namespace Private
		{
			template<typename> class Subset1;
			template<typename> class Subset2;
			template<uint,bool> class Base;

			class Dynamic
			{
			protected:

				static char empty[4];

				explicit Dynamic(uint,char* = empty);
				explicit Dynamic(cstring NST_RESTRICT,char* = empty);
				Dynamic(uint,char*,uint);
				Dynamic(cstring NST_RESTRICT,char*,uint);
				Dynamic(const Dynamic& NST_RESTRICT,char*,uint);

				void Destroy(char* = empty,uint=0);
				void Defrag(char* = empty,uint=0);
				void AssignTerminated(cstring NST_RESTRICT,char* = empty);
				void AppendTerminated(cstring NST_RESTRICT,char* = empty);
				void Assign(cstring NST_RESTRICT,uint,char* = empty);
				void Append(cstring NST_RESTRICT,uint,char* = empty);
				void Insert(uint,cstring NST_RESTRICT,uint,char* = empty);
				void Insert(uint,uint,char* = empty);
				void Parse(cstring NST_RESTRICT,uint,char* = empty);

				char* data;
				uint capacity;
				uint size;

			private:

				inline void Reset(uint,char*);

				NST_NO_INLINE void Allocate(uint,char*);
				NST_NO_INLINE void Reallocate(uint,char*);

			protected:

				explicit Dynamic(char* stack=empty,uint minimum=0)
				: data(stack), capacity(minimum), size(0) {}

				void Reserve(uint request,char* stack=empty)
				{
					if (capacity < request)
						Reallocate( request, stack );
				}

			public:

				uint Capacity() const
				{
					return capacity;
				}
			};

			template<uint N,bool B> class Basic
			{
				NST_CRAPPY_FRIEND friend class Subset1;
				NST_CRAPPY_FRIEND friend class Subset2;

				NST_COMPILE_ASSERT( N >= 8 );

				enum
				{
					STACK_SIZE = (N + 7) & ~3U,
					STACK_CAPACITY = N
				};

			protected:

				uint size;

				union
				{
					char data[STACK_SIZE];
					u32 empty;
				};

				Basic()
				: size(0), empty('\0') {}

				explicit Basic(const Basic& NST_RESTRICT basic)
				: size(basic.size)
				{
					NST_ASSERT( basic.size <= STACK_CAPACITY );
					std::memcpy( data, basic.data, (basic.size + 4) & ~3U );
					NST_ASSERT( data[size] == '\0' );
				}

				explicit Basic(cstring NST_RESTRICT string)
				{
					size = Private::Copy( data, string );
					NST_ASSERT( size <= STACK_CAPACITY );
				}

				explicit Basic(uint length)
				: size(length)  
				{
				}

				void operator = (const Basic& NST_RESTRICT basic)
				{
					NST_ASSERT( basic.size <= STACK_CAPACITY );
					size = basic.size;
					std::memcpy( data, basic.data, (basic.size + 4) & ~3U );
					NST_ASSERT( data[size] == '\0' );
				}

				void AssignTerminated(cstring NST_RESTRICT string)
				{
					size = Private::Copy( data, string );
					NST_ASSERT( size <= STACK_CAPACITY );
				}

				void AppendTerminated(cstring NST_RESTRICT string)
				{
					size += Private::Copy( data + size, string );
					NST_ASSERT( size <= STACK_CAPACITY );
				}

				void Insert(uint pos,cstring NST_RESTRICT string,uint length)
				{
					NST_ASSERT( size + length <= STACK_CAPACITY );
					uint old = size; size += length;
					Private::Insert( data, old, pos, string, length );
				}

			public:

				void Assign(cstring NST_RESTRICT string,uint length)
				{
					NST_ASSERT( length <= STACK_CAPACITY );
					std::memcpy( data, string, size=length );
					data[length] = '\0';
				}

				void Append(cstring NST_RESTRICT string,uint length)
				{
					NST_ASSERT( size + length <= STACK_CAPACITY );
					char* offset = data + size; size += length;
					std::memcpy( offset, string, length );
					offset[length] = '\0';
				}

				void Resize(uint length)
				{
					NST_ASSERT( length <= STACK_CAPACITY );
					data[size = length] = '\0';
				}

				void Destroy()
				{
					size = 0;
					empty = '\0';
				}

				uint Capacity() const
				{
					return STACK_CAPACITY;
				}
			};

			template<> class Basic<0U,false> : public Dynamic
			{
				NST_CRAPPY_FRIEND friend class Subset1;
				NST_CRAPPY_FRIEND friend class Subset2;

			protected:

				Basic() {}

				explicit Basic(const Basic& NST_RESTRICT);
				explicit Basic(cstring);
				explicit Basic(uint);

				~Basic()
				{
					if (data != empty)
						delete [] data;
				}

				void operator = (const Basic& NST_RESTRICT);

				void AssignTerminated(cstring);
				void AppendTerminated(cstring);

			public:

				void Append(cstring,uint);
				void Assign(cstring,uint);
				void Reserve(uint);
				void Resize(uint);
				void Defrag();
				void Destroy();
				char* Export();
			};

			template<uint N> class Basic<N,true> : public Dynamic
			{
				NST_CRAPPY_FRIEND friend class Subset1;
				NST_CRAPPY_FRIEND friend class Subset2;

				NST_COMPILE_ASSERT( N >= 8 );

				enum
				{
					STACK_SIZE = (N + 7) & ~3U,
					STACK_CAPACITY = N
				};

				union
				{
					char stack[STACK_SIZE];
					u32 terminate;
				};

			protected:

				Basic()
				: Dynamic(stack,STACK_CAPACITY), terminate('\0') {}

				explicit Basic(const Basic& basic)
				: Dynamic(basic,stack,STACK_CAPACITY) {}

				explicit Basic(cstring string)
				: Dynamic(string,stack,STACK_CAPACITY) {}

				explicit Basic(uint length)
				: Dynamic(length,stack,STACK_CAPACITY) {}

				~Basic()
				{
					if (data != stack)
						delete [] data;
				}

				void operator = (const Basic& basic)
				{
					Dynamic::Assign( basic.data, basic.size, stack );
				}

				void AssignTerminated(cstring string)
				{
					Dynamic::AssignTerminated( string, stack );
				}

				void AppendTerminated(cstring string)
				{
					Dynamic::AppendTerminated( string, stack );
				}

				void Insert(uint pos,cstring string,uint length)
				{
					Dynamic::Insert( pos, string, length, stack );
				}

				void Parse(cstring string,uint length)
				{
					Dynamic::Parse( string, length, stack );
				}

			public:

				void Assign(cstring string,uint length)
				{
					Dynamic::Assign( string, length, stack );
				}

				void Append(cstring string,uint length)
				{
					Dynamic::Append( string, length, stack );
				}

				void Reserve(uint length)
				{
					Dynamic::Reserve( length, stack );
				}

				void Resize(uint length)
				{
					Reserve( length );
					data[size = length] = '\0';
				}

				void Defrag()
				{
					Dynamic::Defrag( stack, STACK_CAPACITY );
				}

				void Destroy()
				{
					terminate = '\0';
					Dynamic::Destroy( stack, STACK_CAPACITY );
				}
			};

			template<typename T> class Subset2
			{
				NST_CRAPPY_FRIEND friend class Base;
				NST_CRAPPY_FRIEND friend struct Comparer;

				friend class Generic;
				friend class Anything;

			protected:

				T& string;
				const uint pos;
				const uint length;

				operator char* ()
				{
					return string.data + pos;
				}

				operator cstring () const
				{
					return string.data + pos;
				}

				void Erase()
				{
					if (length)
					{
						uint size = string.size; string.size -= length;
						Private::Erase( string.data, size, pos, length );
					}
				}

				void Insert(cstring input,uint length)
				{
					string.Insert( pos, input, length );
				}

				void Replace(cstring input,uint length)
				{
					Erase();
					Insert( input, length );
				}

			public:

				Subset2(T& s,uint p,uint l)
				: string(s), pos(p), length(l) {}

				void MakeLowerCase()
				{
					Private::MakeLowerCase( string.data + pos, length );
				}

				void MakeUpperCase()
				{
					Private::MakeLowerCase( string.data + pos, length );
				}

				char& Back()
				{
					NST_ASSERT( length && pos + length <= string.size );
					return string.data[pos+length-1];
				}

				const char& Back() const
				{
					NST_ASSERT( length && pos + length <= string.size );
					return string.data[pos+length-1];
				}

				uint Size() const
				{
					return length;
				}

				ibool Empty() const
				{
					return !length;
				}

				Subset2<T> operator () (uint p,uint l)
				{
					NST_ASSERT( pos + p + l <= string.size );
					return Subset2<T>( string, pos + p, l );
				}

				Subset2<T> operator () (uint p)
				{
					NST_ASSERT( pos + p <= string.size );
					return Subset2<T>( string, pos + p, length - p );
				}

				template<typename U> ibool operator == (const U& u) const { return  Comparer<Subset2,true>::IsEqual       ( *this, u ); }
				template<typename U> ibool operator != (const U& u) const { return !Comparer<Subset2,true>::IsEqual       ( *this, u ); }
				template<typename U> ibool operator <  (const U& u) const { return  Comparer<Subset2,true>::IsLess        ( *this, u ); }
				template<typename U> ibool operator <= (const U& u) const { return  Comparer<Subset2,true>::IsLessOrEqual ( *this, u ); }
				template<typename U> ibool operator >  (const U& u) const { return !Comparer<Subset2,true>::IsLessOrEqual ( *this, u ); }
				template<typename U> ibool operator >= (const U& u) const { return !Comparer<Subset2,true>::IsLess        ( *this, u ); }

				friend ibool operator == (const char* c,const Subset2& s) { return s == c; }
				friend ibool operator != (const char* c,const Subset2& s) { return s != c; }
				friend ibool operator <  (const char* c,const Subset2& s) { return s >= c; }
				friend ibool operator <= (const char* c,const Subset2& s) { return s >  c; }
				friend ibool operator >  (const char* c,const Subset2& s) { return s <= c; }
				friend ibool operator >= (const char* c,const Subset2& s) { return s <  c; }

				friend ibool operator == (char* c,const Subset2& s) { return s == c; }
				friend ibool operator != (char* c,const Subset2& s) { return s != c; }
				friend ibool operator <  (char* c,const Subset2& s) { return s >= c; }
				friend ibool operator <= (char* c,const Subset2& s) { return s >  c; }
				friend ibool operator >  (char* c,const Subset2& s) { return s <= c; }
				friend ibool operator >= (char* c,const Subset2& s) { return s <  c; }
			};

			template<typename T> class Subset1
			{
			protected:

				T& string;
				const uint pos;

				void Erase()
				{
					string.data[string.size = pos] = '\0';
				}

				void Insert(cstring input,uint length)
				{
					string.Insert( pos, input, length );
				}

				void Replace(cstring input,uint length)
				{
					Erase();
					string.Append( input, length );
				}

			public:

				Subset1(T& s,uint p)
				: string(s), pos(p) {}

				void MakeLowerCase()
				{
					Private::MakeLowerCase( string.data + pos, string.size - pos );
				}

				void MakeUpperCase()
				{
					Private::MakeLowerCase( string.data + pos, string.size - pos );
				}

				uint Size() const
				{
					return string.size - pos;
				}

				ibool Empty() const
				{
					return pos == string.size;
				}

				operator char* ()
				{
					return string.data + pos;
				}

				operator cstring () const
				{
					return string.data + pos;
				}

				char& Back()
				{
					NST_ASSERT( string.size && pos < string.size );
					return string.data[string.size-1];
				}

				const char& Back() const
				{
					NST_ASSERT( string.size && pos < string.size );
					return string.data[string.size-1];
				}

				Subset2<T> operator () (uint p,uint l)
				{
					NST_ASSERT( pos + p + l <= string.size );
					return Subset2<T>( string, pos + p, l );
				}

				Subset1 operator () (uint p)
				{
					NST_ASSERT( pos + p <= string.size );
					return Subset1( string, pos + p );
				}

				template<typename U> ibool operator == (const U& u) const { return  Comparer<Subset1>::IsEqual       ( *this, u ); }
				template<typename U> ibool operator != (const U& u) const { return !Comparer<Subset1>::IsEqual       ( *this, u ); }
				template<typename U> ibool operator <  (const U& u) const { return  Comparer<Subset1>::IsLess        ( *this, u ); }
				template<typename U> ibool operator <= (const U& u) const { return  Comparer<Subset1>::IsLessOrEqual ( *this, u ); }
				template<typename U> ibool operator >  (const U& u) const { return !Comparer<Subset1>::IsLessOrEqual ( *this, u ); }
				template<typename U> ibool operator >= (const U& u) const { return !Comparer<Subset1>::IsLess        ( *this, u ); }

				friend ibool operator == (const char* c,const Subset1& s) { return s == c; }
				friend ibool operator != (const char* c,const Subset1& s) { return s != c; }
				friend ibool operator <  (const char* c,const Subset1& s) { return s >= c; }
				friend ibool operator <= (const char* c,const Subset1& s) { return s >  c; }
				friend ibool operator >  (const char* c,const Subset1& s) { return s <= c; }
				friend ibool operator >= (const char* c,const Subset1& s) { return s <  c; }

				friend ibool operator == (char* c,const Subset1& s) { return s == c; }
				friend ibool operator != (char* c,const Subset1& s) { return s != c; }
				friend ibool operator <  (char* c,const Subset1& s) { return s >= c; }
				friend ibool operator <= (char* c,const Subset1& s) { return s >  c; }
				friend ibool operator >  (char* c,const Subset1& s) { return s <= c; }
				friend ibool operator >= (char* c,const Subset1& s) { return s <  c; }
			};

			template<uint N,bool B> class Base : public Basic<N,B>
			{
				typedef Basic<N,B> Simple;

			public:

				class Sub1 : public Subset1<Simple>
				{
					friend class Base<N,B>;

					Sub1(Simple& s,uint p)
					: Subset1<Simple>(s,p) {}

				public:

					Sub1& operator << (const Anything& input)
					{
						Insert( input, input.Size() );
						return *this;
					}

					Sub1& operator = (const Anything& input)
					{
						Replace( input, input.Size() );
						return *this;
					}

					using Subset1<Simple>::Erase;
				};

				class Sub2 : public Subset2<Simple>
				{
					friend class Base<N,B>;

					Sub2(Simple& s,uint p,uint l)
					: Subset2<Simple>(s,p,l) {}

				public:

					Sub2& operator = (const Anything& input)
					{
						Replace( input, input.Size() );
						return *this;
					}

					using Subset2<Simple>::Erase;
				};

			protected:

				Base() {}			

				Base(const Base& base)
				: Simple( base )
				{
				}

				template<typename T> 
				Base(const T& NST_RESTRICT t)
				: Simple( t.Size() )
				{
					std::memcpy( data, static_cast<cstring>(t), size );
					data[size] = '\0';
				}

				template<> 
				Base(const char* const& c)
				: Simple( c )
				{
				}

				template<> 
				Base(char* const& c)
				: Simple( c )
				{
				}

				template<size_t N> 
				Base(const char (&c)[N])
				: Simple( N-1 )
				{
					std::memcpy( data, c, N );
					NST_ASSERT( c[N-1] == '\0' );
				}

				Base(char c)
				: Simple( 1 )
				{
					data[0] = c;
					data[1]	= '\0';
				}				   

				Base(cstring c,uint n)
				: Simple( n )
				{
					std::memcpy( data, c, n );
					data[n] = '\0';
				}

				template<typename T>
				void Assign(const T& t)
				{
					Simple::Assign( t, t.Size() );
				}

				template<>
				void Assign(const char* const& c)
				{
					Simple::AssignTerminated( c );
				}

				template<>
				void Assign(char* const& c)
				{
					Simple::AssignTerminated( c );
				}

				template<size_t N>
				void Assign(const char (&c)[N])
				{
					Simple::Assign( c, N-1 );
				}

				void Assign(char c)
				{
					Simple::Assign( &c, 1 );
				}

				template<typename T>
				void Append(const T& t)
				{
					Simple::Append( t, t.Size() );
				}

				template<>
				void Append(const char* const& c)
				{
					Simple::AppendTerminated( c );
				}

				template<>
				void Append(char* const& c)
				{
					Simple::AppendTerminated( c );
				}

				template<size_t N>
				void Append(const char (&c)[N])
				{
					Simple::Append( c, N-1 );
				}

				void Append(char c)
				{
					Simple::Append( &c, 1 );
				}

				void Append(int i)
				{
					char buffer[12];
					cstring string = Private::FromSigned( buffer, i );
					Append( string, buffer + 11 - string );
				}

				void Append(uint i)
				{
					char buffer[12];
					cstring string = Private::FromUnsigned( buffer, i );
					Append( string, buffer + 11 - string );
				}

				void Append(schar  i) { Append( ( int  ) i ); }
				void Append(uchar  i) { Append( ( uint ) i ); }
				void Append(short  i) { Append( ( int  ) i ); }
				void Append(ushort i) { Append( ( uint ) i ); }
				void Append(long   i) { Append( ( int  ) i ); }
				void Append(ulong  i) { Append( ( uint ) i ); }

				ibool ToInteger( schar&  i ) const { u32 u; ibool r = Private::ToUnsigned( data, u ); i = ( schar  ) u; return r; }
				ibool ToInteger( uchar&  i ) const { u32 u; ibool r = Private::ToUnsigned( data, u ); i = ( uchar  ) u; return r; }
				ibool ToInteger( short&  i ) const { u32 u; ibool r = Private::ToUnsigned( data, u ); i = ( short  ) u; return r; }
				ibool ToInteger( ushort& i ) const { u32 u; ibool r = Private::ToUnsigned( data, u ); i = ( ushort ) u; return r; }
				ibool ToInteger( int&    i ) const { u32 u; ibool r = Private::ToUnsigned( data, u ); i = ( int    ) u; return r; }
				ibool ToInteger( uint&   i ) const { u32 u; ibool r = Private::ToUnsigned( data, u ); i = ( uint   ) u; return r; }
				ibool ToInteger( long&   i ) const { u32 u; ibool r = Private::ToUnsigned( data, u ); i = ( long   ) u; return r; }
				ibool ToInteger( ulong&  i ) const { u32 u; ibool r = Private::ToUnsigned( data, u ); i = ( ulong  ) u; return r; }

			public:

				using Simple::Assign;
				using Simple::Append;

				template<size_t N>
				ibool ExactEqual(const char (&string)[N]) const
				{
					return size == N-1 && std::memcmp( data, string, N-1 ) == 0;
				}

				void ShrinkTo(uint count)
				{
					NST_ASSERT( Capacity() >= count );
					data[size = count] = '\0';
				}

				void Shrink(uint length=1)
				{
					NST_ASSERT( size >= length );
					data[size -= length] = '\0';
				}

				void Grow(uint length=1)
				{
					Resize( size + length );
				}

				void Remove(const int c)
				{
					size = Private::Remove( data, size, c );
				}

				void Trim()
				{
					size = Private::Trim( data, size );
				}

				uint Size() const													 
				{
					return size;
				}

				ibool Empty() const
				{
					return !size;
				}

				void Clear()
				{
					data[size = 0] = '\0';
				}

				uint Validate()
				{
					return size = Length(data);
				}

				char& Front()
				{
					NST_ASSERT( size );
					return *data;
				}

				const char& Front() const
				{
					NST_ASSERT( size );
					return *data;
				}

				char& Back()
				{
					NST_ASSERT( size );
					return data[size - 1];
				}

				const char& Back() const
				{
					NST_ASSERT( size );
					return data[size - 1];
				}

				char* Begin()
				{
					return data;
				}

				const char* Begin() const
				{
					return data;
				}

				char* End()
				{
					return data + size;
				}

				const char* End() const
				{
					return data + size;
				}

				operator char* ()
				{
					return data;
				}

				operator cstring () const
				{
					return data;
				}

				template<typename T> ibool operator == (const T& t) const { return  Comparer<Base>::IsEqual       ( *this, t ); }
				template<typename T> ibool operator != (const T& t) const { return !Comparer<Base>::IsEqual       ( *this, t ); }
				template<typename T> ibool operator <  (const T& t) const { return  Comparer<Base>::IsLess        ( *this, t ); }
				template<typename T> ibool operator <= (const T& t) const { return  Comparer<Base>::IsLessOrEqual ( *this, t ); }
				template<typename T> ibool operator >  (const T& t) const { return !Comparer<Base>::IsLessOrEqual ( *this, t ); }
				template<typename T> ibool operator >= (const T& t) const { return !Comparer<Base>::IsLess        ( *this, t ); }

				friend ibool operator == (const char* c,const Base& b) { return b == c; }
				friend ibool operator != (const char* c,const Base& b) { return b != c; }
				friend ibool operator <  (const char* c,const Base& b) { return b >= c; }
				friend ibool operator <= (const char* c,const Base& b) { return b >  c; }
				friend ibool operator >  (const char* c,const Base& b) { return b <= c; }
				friend ibool operator >= (const char* c,const Base& b) { return b <  c; }

				friend ibool operator == (char* c,const Base& b) { return b == c; }
				friend ibool operator != (char* c,const Base& b) { return b != c; }
				friend ibool operator <  (char* c,const Base& b) { return b >= c; }
				friend ibool operator <= (char* c,const Base& b) { return b >  c; }
				friend ibool operator >  (char* c,const Base& b) { return b <= c; }
				friend ibool operator >= (char* c,const Base& b) { return b <  c; }

				Sub1 operator () (uint pos)
				{
					return Sub1( *this, pos );
				}

				const Generic operator () (uint pos) const
				{
					return Generic( data + pos, size - pos );
				}

				Sub2 operator () (uint pos,uint length)
				{
					return Sub2( *this, pos, length );
				}

				const Generic operator () (uint pos,uint length) const
				{
					return Generic( data + pos, length );
				}

				Sub2 Find(const Generic needle)
				{
					cstring offset = Private::Find( data, needle, needle.Size() );

					return Sub2
					( 
						*this,
						offset ? offset - data : size, 
						offset ? length : 0 
					);
				}

				const Generic Find(const Generic needle) const
				{
					cstring offset = Private::Find( data, needle, needle.Size() );

					return Generic
					( 
						offset ? offset : data + size, 
						offset ? needle.Size() : 0 
					);
				}

				ibool Has(const Generic needle) const
				{
					return Private::Find( data, needle, needle.Size() ) != NULL;
				}

				uint FindFirstOf      (int c) const { return Private::FindFirstOf      ( c, data, size ); }
				uint FindFirstNotOf   (int c) const { return Private::FindFirstNotOf   ( c, data, size ); }
				uint FindLastOf       (int c) const { return Private::FindLastOf       ( c, data, size ); }
				uint FindLastNotOf    (int c) const { return Private::FindLastNotOf    ( c, data, size ); }
				uint FindAfterFirstOf (int c) const { return Private::FindAfterFirstOf ( c, data, size ); }
				uint FindAfterLastOf  (int c) const { return Private::FindAfterLastOf  ( c, data, size ); }

				Sub1 FirstOf      (int c) { return Sub1( *this, FindFirstOf      (c) ); }
				Sub1 AfterFirstOf (int c) { return Sub1( *this, FindAfterFirstOf (c) ); }
				Sub1 FirstNotOf   (int c) { return Sub1( *this, FindFirstNotOf   (c) ); }
				Sub1 AfterLastOf  (int c) { return Sub1( *this, FindAfterLastOf  (c) ); }
				Sub1 LastOf       (int c) { return Sub1( *this, FindLastOf       (c) ); }
				Sub1 LastNotOf    (int c) { return Sub1( *this, FindLastNotOf    (c) ); }

				const Generic FirstOf      (int c) const { uint p = FindFirstOf      (c); return Generic( data + p, size - p ); }
				const Generic AfterFirstOf (int c) const { uint p = FindAfterFirstOf (c); return Generic( data + p, size - p ); }
				const Generic FirstNotOf   (int c) const { uint p = FindFirstNotOf   (c); return Generic( data + p, size - p ); }
				const Generic LastOf       (int c) const { uint p = FindLastOf       (c); return Generic( data + p, size - p ); }
				const Generic AfterLastOf  (int c) const { uint p = FindAfterLastOf  (c); return Generic( data + p, size - p ); }
				const Generic LastNotOf    (int c) const { uint p = FindLastNotOf    (c); return Generic( data + p, size - p ); }

				void MakeLowerCase()
				{
					Private::MakeLowerCase( data, size );
				}

				void MakeUpperCase()
				{
					Private::MakeLowerCase( data, size );
				}
			};
		}

		template<uint N>
		class Stack : public Private::Base<N,false>
		{
			typedef Private::Base<N,false> Base;

		public:

			Stack() {}

			Stack(const Stack& s)
			: Base(static_cast<const Base&>(s)) {}

			template<typename T> 
			Stack(const T& t)
			: Base(t) {}

			Stack(cstring c,uint l)
			: Base(c,l) {}

			template<typename T>
			Stack& operator = (const T& t)
			{
				Assign( t );
				return *this;
			}

			template<typename T>
			Stack& operator << (const T& t)
			{
				Append( t );
				return *this;
			}

			template<typename T>
			ibool operator >> (T& t) const
			{
				return ToInteger( t );
			}
		};

		class Heap : public Private::Base<0,false>
		{
			typedef Private::Base<0,false> Base;

		public:

			Heap() {}

			Heap(const Heap& h)
			: Base(static_cast<const Base&>(h)) {}

			template<typename T> 
			Heap(const T& t)
			: Base(t) {}

			Heap(cstring c,uint l)
			: Base(c,l) {}

			template<typename T>
			Heap& operator = (const T& t)
			{
				Assign( t );
				return *this;
			}

			template<typename T>
			Heap& operator << (const T& t)
			{
				Append( t );
				return *this;
			}

			template<typename T>
			ibool operator >> (T& t) const
			{
				return ToInteger( t );
			}
		};

		template<uint N>
		class Smart : public Private::Base<N,true>
		{
			typedef Private::Base<N,true> Base;

		public:

			Smart() {}

			Smart(const Smart& s)
			: Base(static_cast<const Base&>(s)) {}

			template<typename T> 
			Smart(const T& t)
			: Base(t) {}

			Smart(cstring c,uint l)
			: Base(c,l) {}

			template<typename T>
			Smart& operator = (const T& t)
			{
				Assign( t );
				return *this;
			}

			template<typename T>
			Smart& operator << (const T& t)
			{
				Append( t );
				return *this;
			}

			template<typename T>
			ibool operator >> (T& t) const
			{
				return ToInteger( t );
			}
		};

		template<uint N=0>
		class Stream : protected Private::Base<N,N ? true : false>
		{
			typedef Private::Base<N,N ? true : false> Base;

		public:

			Stream() {}

			Stream(const Generic& string)
			{
				Parse( string, string.Size() );
			}

			Stream(cstring string,uint length)
			{
				Parse( string, length );
			}

			const Generic operator () (cstring begin,cstring end) const
			{ 
				return Generic( begin, end - begin ); 
			}

			using Base::Size;

			operator cstring () const
			{
				return data;
			}
		};

		template<bool B=false>
		class Path : public Private::Base<B ? _MAX_PATH : 0, B ? true : false>
		{
			typedef Private::Base<B ? _MAX_PATH : 0,B ? true : false> Base;

		public:

			Path(Generic,Generic,Generic);

			void Set(Generic,Generic,Generic);

		private:

			uint FindDirectory() const;
			uint FindExtension() const;
			uint FindArchive() const;
			void FindFileInArchive(uint (&)[2]) const;

		public:

			Path() {}

			Path(const Path& p)
			: Base(static_cast<const Base&>(p)) {}

			template<typename T> 
			Path(const T& t)
			: Base(t) {}

			Path(cstring c,uint l)
			: Base(c,l) {}

			template<typename T>
			Path& operator = (const T& t)
			{
				Assign( t );
				return *this;
			}

			template<typename T>
			Path& operator << (const T& t)
			{
				Append( t );
				return *this;
			}

		private:

			class DirectorySub : public Private::Subset2<Path>
			{		
			public:

				DirectorySub(Path& path,uint length)
				: Private::Subset2<Path>( path, 0, length ) {}

				void operator = (Generic);
				void operator += (Generic);
				void operator -= (uint);

				void Validate();

				void Clear()
				{
					Erase();
				}
			};

			class FileSub : public Private::Subset1<Path>
			{
			public:

				FileSub(Path& path,uint pos)
				: Private::Subset1<Path>( path, pos ) {}

				void operator = (const Generic& input)
				{
					Replace( input, input.Size() );
				}

				void Clear()
				{
					Erase();
				}
			};

			class ExtensionSub : public Private::Subset1<Path>
			{
				void Set(cstring NST_RESTRICT,uint);

			public:

				ExtensionSub(Path& path,uint pos)
				: Private::Subset1<Path>( path, pos ) {}

				void operator = (const Generic& input)
				{
					Set( input, input.Size() );
				}

				uint Id() const
				{
					return string.size >= 3 ? Private::ExtensionId( string.data + pos ) : 0;
				}

				void Clear();
			};

			class ConstExtensionSub : public Generic
			{
			public:

				ConstExtensionSub(cstring string,uint length)
				: Generic( string, length ) {}

				uint Id() const
				{
					return size >= 3 ? Private::ExtensionId( string ) : 0;
				}
			};

			class ArchiveFileSub : public Generic
			{
			public:

				ArchiveFileSub(cstring string,uint length)
				: Generic( string, length ) {}

				Generic File() const
				{
					for (uint i=size; i; )
					{
						switch (string[--i])
						{
				    		case '\\':
				    		case '/': return Generic( string + i + 1, size - (i + 1) );
						}
					}

					return Generic( string, size );
				}
			};

		public:

			DirectorySub Directory()
			{
				return DirectorySub( *this, FindDirectory() );
			}

			const Generic Directory() const
			{
				return Generic( data, FindDirectory() );
			}

			FileSub File()
			{
				return FileSub( *this, FindDirectory() );
			}

			const Generic File() const
			{
				uint p = FindDirectory();
				return Generic( data + p, size - p );
			}

			ExtensionSub Extension()
			{
				return ExtensionSub( *this, FindExtension() );
			}

			ConstExtensionSub Extension() const
			{
				uint p = FindExtension();
				return ConstExtensionSub( data + p, size - p );
			}

			Generic Archive() const
			{
				return Generic( data, FindArchive() );
			}

			ArchiveFileSub FileInArchive() const
			{
				uint range[2];
				FindFileInArchive( range );
				return ArchiveFileSub( data + range[0], range[1] );
			}

			ArchiveFileSub Target() const
			{
				ArchiveFileSub archive( FileInArchive() );

				if (archive.Size())
					return archive;
				
				return ArchiveFileSub( data, size );
			}

			ibool FileExist() const
			{
				return Private::FileExist( data );
			}

			ibool DirExist() const
			{
				return Private::DirExist( data );
			}
		};

		template<bool B>
		Path<B>::Path(const Generic dir,const Generic file,const Generic ext)
		{
			Set( dir, file, ext );
		}

		template<bool B>
		void Path<B>::Set(const Generic dir,const Generic file,const Generic ext)
		{
			Clear();
			Reserve( dir.Size() + file.Size() + ext.Size() );

			Directory() = dir;
			File() = file;

			if (ext.Size())
				Extension() = ext;
		}

		template<bool B>
		uint Path<B>::FindDirectory() const
		{
			for (uint i=size; i; )
			{
				switch (data[--i])
				{
		     		case '\\':
		    		case '/': return i + 1;
					case '>': while (i && data[--i] != '<'); break;
				}
			}

			return 0;
		}

		template<bool B>
		uint Path<B>::FindExtension() const
		{
			for (uint i=size; i; )
			{
				switch (data[--i])
				{
			    	case '.': return i + 1;
		    		case '\\':
		     		case '/': return size;
					case '>': while (i && data[--i] != '<'); break;
				}
			}

			return size;
		}

		template<bool B>
		uint Path<B>::FindArchive() const
		{
			ibool paired = FALSE;

			for (uint i=size; i; )
			{
				switch (data[--i])
				{
    				case '>': 
						
						paired = TRUE; 
						break;

					case '<': 
						
						if (paired)
						{
							while (i && data[i-1] == ' ')
								--i;

							return i;
						}
						return 0;
				}
			}

			return 0;
		}

		template<bool B>
		void Path<B>::FindFileInArchive(uint (&ranges)[2]) const
		{
			uint end = 0;
			ranges[0] = 0, ranges[1] = 0;

			for (uint i=size; i; )
			{
				switch (data[--i])
				{
    				case '>': 
						
						while (i && data[i-1] == ' ')
							--i;

						end = i; 
						break;

					case '<': 
						
						if (end)
						{
							while (data[++i] == ' ');
							
							ranges[0] = i;
							ranges[1] = end - i;
						}
						return;
				}
			}
		}

		template<bool B>
		void Path<B>::DirectorySub::operator = (const Generic input)
		{
			NST_ASSERT( pos == 0 );

			Clear();

			if (input.Size())
			{
				const ibool putSlash = input.Back() != '\\' && input.Back() != '/';

				Insert( NULL, input.Size() + putSlash );
				std::memcpy( string.data, input, input.Size() );

				if (putSlash)
					string.data[input.Size()] = '\\';
			}
		}

		template<bool B>
		void Path<B>::DirectorySub::operator += (const Generic input)
		{
			NST_ASSERT( pos == 0 );

			if (input.Size())
			{
				const ibool putSlash = input.Back() != '\\' && input.Back() != '/';

				string.Insert( length, NULL, input.Size() + putSlash );
				std::memcpy( string.data + length, input, input.Size() );

				if (putSlash)
					string.data[length + input.Size()] = '\\';
			}
		}

		template<bool B>
		void Path<B>::DirectorySub::Validate()
		{
			if (length < string.size)
				string << '\\';
		}

		template<bool B>
		void Path<B>::DirectorySub::operator -= (uint subs)
		{
			if (subs && length && (string.data[length-1] == '\\' || string.data[length-1] == '/'))
			{
				uint i = length - 1;
				uint offset = ~0U;

				do
				{
					if (string.data[--i] == '\\' || string.data[i] == '/')
					{
						offset = i;

						if (!--subs)
							break;
					}
				}
				while (i);

				if (offset != ~0U)
				{
					uint size = string.size; string.size -= length - offset - 1;
					Private::Erase( string.data, size, offset, length - offset - 1 );
				}
			}
		}

		template<bool B>
		void Path<B>::ExtensionSub::Clear()
		{
			if (pos != string.size)
				string.data[string.size = (pos-1)] = '\0';
		}

		template<bool B>
		void Path<B>::ExtensionSub::Set(cstring NST_RESTRICT input,uint length)
		{
			if (*input == '.')
			{
				++input;
				--length;
			}

			const uint offset = (pos != string.size ? pos-1 : pos);
			string.Resize( offset + bool(length) + length );

			if (length)
			{
				string.data[offset] = '.';
				std::memcpy( string.data + offset + 1, input, length );
			}
		}

		class Hex
		{
		public:

			explicit Hex(u32,bool=false) throw();
			explicit Hex(u16,bool=false) throw();
			explicit Hex(u8,bool=false) throw();

		private:

			char buffer[12];
			cstring const string;
			const uint length;

			void Convert(uint,uint) throw();

		public:

			operator cstring () const
			{
				return string;
			}

			uint Size() const
			{
				return length;
			}
		};
	}
}

#undef NST_CRAPPY_FRIEND

#endif
