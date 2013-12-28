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

#ifndef NST_STRING_H
#define NST_STRING_H

#pragma once

#include "NstMain.hpp"
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <windows.h>

namespace Nestopia
{
	namespace String
	{
		class Base
		{
			template<typename T>
			static bool ToUnsigned(const T* NST_RESTRICT,uint,u32&);

			template<typename T> static void AppendSigned(T&,i32);
			template<typename T> static void AppendUnsigned(T&,u32);

			static int Compare(const char* t,int n,const char* u,int m)           
			{
				return ::CompareStringA( LOCALE_INVARIANT, NORM_IGNORECASE, t, n, u, m ) - 2;
			}

			static int Compare(const wchar_t* t,int n,const wchar_t* u,int m)           
			{
				return ::CompareStringW( LOCALE_INVARIANT, NORM_IGNORECASE, t, n, u, m ) - 2;
			}

			static int CompareCase(const char* t,int n,const char* u,int m)           
			{
				return ::CompareStringA( LOCALE_INVARIANT, 0, t, n, u, m ) - 2;
			}

			static int CompareCase(const wchar_t* t,int n,const wchar_t* u,int m)           
			{
				return ::CompareStringW( LOCALE_INVARIANT, 0, t, n, u, m ) - 2;
			}

		protected:

			enum
			{
				MAX_INT_LENGTH = 12
			};

			template<typename T> static T* Remove(T*,T);
			
			static bool Trim(char*);
			static bool Trim(wchar_t*);

			template<typename T> static T* FromUnsigned(T (&)[MAX_INT_LENGTH],u32);
			template<typename T> static T* FromSigned(T (&)[MAX_INT_LENGTH],i32);

			template<typename T> static void AssignSigned(T&,i32);
			template<typename T> static void AssignUnsigned(T&,u32);

			template<typename T,typename U>
			static bool ConvertToUnsigned(const T&,U&);

			static uint Length(const char* str)
			{
				return std::strlen( str );
			}

			static uint Length(const wchar_t* str)
			{
				return std::wcslen( str );
			}

			static void Copy(char* NST_RESTRICT a,const char* NST_RESTRICT b,uint n)
			{
				std::memcpy( a, b, n );
			}

			static void Copy(wchar_t* NST_RESTRICT a,const wchar_t* NST_RESTRICT b,uint n)
			{
				std::wmemcpy( a, b, n );
			}

			static void Copy(wchar_t* NST_RESTRICT a,const char* NST_RESTRICT b,uint n)
			{
				for (uint i=0; i < n; ++i)
					a[i] = b[i];
			}
  
			static void Copy(char* NST_RESTRICT a,const wchar_t* NST_RESTRICT b,uint n)
			{
				for (uint i=0; i < n; ++i)
					a[i] = (char) b[i];
			}

			static void Move(char* a,const char* b,uint n)
			{
				std::memmove( a, b, n );
			}

			static void Move(wchar_t* a,const wchar_t* b,uint n)
			{
				std::wmemmove( a, b, n );
			}

			template<typename T>
			static typename const T::Type* GetPtr(const T& t)
			{
				return t.Ptr();
			}

			template<typename T>
			static const T* GetPtr(T* const& t)
			{
				return t;
			}

			template<typename T>
			static const T* GetPtr(const T* const& t)
			{
				return t;
			}

			template<typename T,size_t N>
			static const T* GetPtr(const T (&t)[N])
			{
				return t;
			}

			template<typename T>
			static uint GetLength(const T& t)
			{
				return t.Length();
			}

			template<typename T>
			static uint GetLength(T* const& t)
			{
				return Length(t);
			}

			template<typename T>
			static uint GetLength(const T* const& t)
			{
				return Length(t);
			}

			template<typename T,size_t N>
			static uint GetLength(const T (&t)[N])
			{
				return Length(t);
			}

			template<typename T,typename U>
			static bool IsEqual(const T& t,const U& u)
			{ 
				return t.Length() == u.Length() && !Compare( t.Ptr(), t.Length(), u.Ptr(), u.Length() );
			}

			template<typename T,typename U>
			static bool IsEqual(const T& t,const U*& u)
			{ 
				return !Compare( t.Ptr(), t.Length(), u, -1 );
			}

			template<typename T,typename U>
			static bool IsEqual(const T& t,const U* const& u)
			{ 
				return !Compare( t.Ptr(), t.Length(), u, -1 );
			}

			template<typename T,typename U,size_t N>
			static bool IsEqual(const T& t,const U (&u)[N])
			{ 
				return !Compare( t.Ptr(), t.Length(), u, -1 );
			}

			template<typename T,typename U>
			static bool IsCaseEqual(const T& t,const U& u)
			{ 
				return t.Length() == u.Length() && !CompareCase( t.Ptr(), t.Length(), u.Ptr(), u.Length() );
			}

			template<typename T,typename U>
			static bool IsCaseEqual(const T& t,const U*& u)
			{ 
				return !CompareCase( t.Ptr(), t.Length(), u, -1 );
			}

			template<typename T,typename U>
			static bool IsCaseEqual(const T& t,const U* const& u)
			{ 
				return !CompareCase( t.Ptr(), t.Length(), u, -1 );
			}

			template<typename T,typename U,size_t N>
			static bool IsCaseEqual(const T& t,const U (&u)[N])
			{ 
				return !CompareCase( t.Ptr(), t.Length(), u, -1 );
			}

			template<typename T,typename U>
			static int Compare(const T& t,const U& u)
			{ 
				return Compare( t.Ptr(), t.Length(), u.Ptr(), u.Length() );
			}

			template<typename T,typename U>
			static int Compare(const T& t,const U*& u)
			{ 
				return Compare( t.Ptr(), t.Length(), u, -1 );
			}

			template<typename T,typename U>
			static int Compare(const T& t,const U* const& u)
			{ 
				return Compare( t.Ptr(), t.Length(), u, -1 );
			}

			template<typename T,typename U,size_t N>
			static int Compare(const T& t,const U (&u)[N])
			{ 
				return Compare( t.Ptr(), t.Length(), u, -1 );
			}

			static bool Wide(const char*,uint)
			{
				return false;
			}

			static bool Wide(const wchar_t* t,uint n)
			{
				for (uint i=0; i < n; ++i)
				{
					if (t[i] > 0xFF)
						return true;
				}

				return false;
			}

			template<typename T,typename U>
			static void Append(T& t,const U& u)
			{
				t.Append( GetPtr(u), GetLength(u) );
			}

			template<typename T>
			static void Append(T& t,char c)
			{
				const T::Type v = c;
				t.Append( &v, 1 );
			}

			template<typename T>
			static void Append(T& t,wchar_t c)
			{
				t.Append( &c, 1 );
			}

			template<typename T> static void Append(T& t,int    i) { AppendSigned   ( t, i ); }
			template<typename T> static void Append(T& t,uint   i) { AppendUnsigned ( t, i ); }
			template<typename T> static void Append(T& t,schar  i) { AppendSigned   ( t, i ); }
			template<typename T> static void Append(T& t,uchar  i) { AppendUnsigned ( t, i ); }
			template<typename T> static void Append(T& t,short  i) { AppendSigned   ( t, i ); }
			template<typename T> static void Append(T& t,ushort i) { AppendUnsigned ( t, i ); }
			template<typename T> static void Append(T& t,long   i) { AppendSigned   ( t, i ); }
			template<typename T> static void Append(T& t,ulong  i) { AppendUnsigned ( t, i ); }
		};

		template<typename T>
		T* Base::FromUnsigned(T (&buffer)[MAX_INT_LENGTH],u32 number)
		{
			T* it = buffer + MAX_INT_LENGTH-1;

			*it = '\0';

			do 
			{
				*--it = (T) ((number % 10) + '0');
			} 
			while (number /= 10);

			return it;
		}

		template<typename T>
		T* Base::FromSigned(T (&buffer)[MAX_INT_LENGTH],const i32 number)
		{
			uint value = (uint) (number >= 0 ? number : -number);

			T* it = buffer + MAX_INT_LENGTH-1;

			*it = '\0';

			do 
			{
				*--it = (T) ((value % 10) + '0');
			} 
			while (value /= 10);

			if (number < 0)
				*--it = '-';

			return it;
		}

		template<typename T>
		void Base::AssignUnsigned(T& t,const u32 i)
		{
			T::Type buffer[MAX_INT_LENGTH];
			const T::Type* const offset = FromUnsigned( buffer, i );
			t.Assign( offset, buffer + MAX_INT_LENGTH-1 - offset );
		}

		template<typename T>
		void Base::AssignSigned(T& t,const i32 i)
		{
			T::Type buffer[MAX_INT_LENGTH];
			const T::Type* const offset = FromUnsigned( buffer, i );
			t.Assign( offset, buffer + MAX_INT_LENGTH-1 - offset );
		}

		template<typename T>
		void Base::AppendUnsigned(T& t,const u32 i)
		{
			T::Type buffer[MAX_INT_LENGTH];
			const T::Type* const offset = FromUnsigned( buffer, i );
			t.Append( offset, buffer + MAX_INT_LENGTH-1 - offset );
		}

		template<typename T>
		void Base::AppendSigned(T& t,const i32 i)
		{
			T::Type buffer[MAX_INT_LENGTH];
			const T::Type* const offset = FromUnsigned( buffer, i );
			t.Append( offset, buffer + MAX_INT_LENGTH-1 - offset );
		}

		template<typename T>
		bool Base::ToUnsigned(const T* NST_RESTRICT string,const uint length,u32& result)
		{
			result = 0;

			if (length)
			{
				bool neg = false;
				uint base = 10;
				uint maxvalue = 0xFFFFFFFFU / 10U;
				const T* end = string + length;

				switch (*string)
				{
					case '-': neg = true;
					case '+': ++string; break;		
					case '0':
								
						if (++string != end)
						{
							base = 16;
							maxvalue = 0xFFFFFFFFU / 16U;
							const T digit = *string++;

							if (digit == 'x' || digit == 'X')
								break;
							else
								return false;
						}
						else
						{
							return true;
						}
				}

				if (string != end)
				{
					uint value = 0;

					do
					{
						const T digit = *string++;
						uint num;

						if (digit >= '0' && digit <= '9')
						{
							num = digit - '0';
						}
						else if (digit >= 'A' && digit <= 'F' && base == 16)
						{
							num = digit - 'A' + 10;
						}
						else if (digit >= 'a' && digit <= 'f' && base == 16)
						{
							num = digit - 'a' + 10;
						}
						else
						{
							return false;
						}

						if (value < maxvalue || (value == maxvalue && num <= 0xFFFFFFFFU % base)) 
							value = value * base + num;
						else 
							return false;
					}
					while (string != end);

					result = neg ? uint(-int(value)) : value;

					return true;
				}
			}

			return false;
		}

		template<typename T,typename U>
		bool Base::ConvertToUnsigned(const T& string,U& value)
		{
			u32 u; 
			const bool r = ToUnsigned( string.Ptr(), string.Length(), u ); 
			value = (U) u; 
			return r;
		}

		template<typename T>
		T* Base::Remove(T* string,const T c)
		{
			T* p;

			for (p=string; *string; ++string)
			{
				if (*string != c)
					*p++ = *string;
			}

			*p = '\0';

			return p;
		}
  
		template<typename T=tchar,bool I=false>
		class Generic : protected Base
		{
		public:

			typedef T Type;

			enum
			{
				TYPE_SIZE = sizeof(Type)
			};

		protected:
			
			const Type* string;
			uint length;

			static const Type empty = '\0';

			enum DoNothing {NOP};
			Generic(DoNothing) {}

		public:

			Generic()
			: string(&empty), length(0)	{}

			Generic(const Type* t,uint l)
			: string(t), length(l) { NST_ASSERT( t ); }

			template<typename T>
			Generic(const T& t)
			: string(Base::GetPtr(t)), length(Base::GetLength(t)) {}

			template<typename T>
			Generic& operator = (const T& t)
			{
				string = Base::GetPtr(t);
				length = Base::GetLength(t);
				return *this;
			}

			template<typename U>
			bool operator >> (U& t) const
			{
				return Base::ConvertToUnsigned( *this, t );
			}

			uint Length() const
			{
				return length;
			}

			bool Empty() const
			{
				return !length;
			}

			const Type& Front() const
			{
				NST_ASSERT( length );
				return string[0];
			}

			const Type& Back() const
			{
				NST_ASSERT( length );
				return string[length-1];
			}

			bool IsNullTerminated() const
			{
				return string[length] == '\0';
			}

			const Type* Ptr() const
			{
				return string;
			}

			const Type& operator [] (uint i) const
			{
				return string[i];
			}

			bool Wide() const
			{
				return Base::Wide( Ptr(), Length() );
			}

			Generic operator () (uint p) const
			{
				NST_ASSERT( p <= length );
				return Generic( string + p, length - p );
			}

			Generic operator () (uint p,uint l) const
			{
				NST_ASSERT( p + l <= length );
				return Generic( string + p, l );
			}

			template<typename T> 
			bool IsEqual(const T& t) const 
			{ 
				return Base::IsCaseEqual( *this, t );
			}

			template<typename U> bool operator == (const U& t) const { return  Base::IsEqual( *this, t );      }
			template<typename U> bool operator != (const U& t) const { return !Base::IsEqual( *this, t );      }
			template<typename U> bool operator <  (const U& t) const { return  Base::Compare( *this, t ) <  0; }
			template<typename U> bool operator <= (const U& t) const { return  Base::Compare( *this, t ) <= 0; }
			template<typename U> bool operator >  (const U& t) const { return  Base::Compare( *this, t ) >  0; }
			template<typename U> bool operator >= (const U& t) const { return  Base::Compare( *this, t ) >= 0; }
		};

		template<typename T>
		class Generic<T,true> : public Generic<T,false>
		{	
			void Copy(const Generic&);

			Type buffer[MAX_INT_LENGTH];

			void SetValue(i32 number)
			{
				string = Base::FromSigned( buffer, number ); 
				length = buffer + MAX_INT_LENGTH-1 - string; 
			}

			void SetValue(u32 number)
			{
				string = Base::FromUnsigned( buffer, number ); 
				length = buffer + MAX_INT_LENGTH-1 - string; 
			}

		public:

			Generic(const Generic& g)
			: Generic<T,false>(NOP)
			{
				Copy( g );
			}

			template<typename U> 
			Generic(const U& t)
			: Generic<T,false>(t) {}

			Generic(Type c)
			: Generic<T,false>(NOP) 
			{
				string = buffer;
				length = 1;
				buffer[0] = c;
				buffer[1] = '\0';
			}

			Generic( schar  i ) : Generic<T,false>(NOP) { SetValue( (i32) i ); } 
			Generic( uchar  i ) : Generic<T,false>(NOP) { SetValue( (u32) i ); } 
			Generic( short  i ) : Generic<T,false>(NOP) { SetValue( (i32) i ); } 
			Generic( ushort i ) : Generic<T,false>(NOP) { SetValue( (u32) i ); } 
			Generic( int    i ) : Generic<T,false>(NOP) { SetValue( (i32) i ); } 
			Generic( uint   i ) : Generic<T,false>(NOP) { SetValue( (u32) i ); } 
			Generic( long   i ) : Generic<T,false>(NOP) { SetValue( (i32) i ); } 
			Generic( ulong  i ) : Generic<T,false>(NOP) { SetValue( (u32) i ); } 

			Generic& operator = (const Generic& g)
			{
				Copy( g );
				return *this;
			}

			template<typename U>
			Generic& operator = (const U& t)
			{
				Generic<T,false>::operator = (t);
				return *this;
			}

			Generic& operator = (Type c)
			{
				string = buffer;
				length = 1;
				buffer[0] = c;
				buffer[1] = '\0';
				return *this;
			}

			Generic& operator = ( schar  i ) { SetValue( (i32) i ); return *this; }
			Generic& operator = ( uchar  i ) { SetValue( (u32) i ); return *this; }
			Generic& operator = ( short  i ) { SetValue( (i32) i ); return *this; }
			Generic& operator = ( ushort i ) { SetValue( (u32) i ); return *this; }
			Generic& operator = ( int    i ) { SetValue( (i32) i ); return *this; }
			Generic& operator = ( uint   i ) { SetValue( (u32) i ); return *this; }
			Generic& operator = ( long   i ) { SetValue( (i32) i ); return *this; }
			Generic& operator = ( ulong  i ) { SetValue( (u32) i ); return *this; }
		};

		template<typename T=tchar>
		class Num : public Generic<T,true>
		{
		public:

			Num(uint v)
			: Generic(v) {}

			Num(int v)
			: Generic(v) {}

			Num& operator = (uint v)
			{
				Generic::operator = (v);
				return *this;
			}

			Num& operator = (int v)
			{
				Generic::operator = (v);
				return *this;
			}
		};

		template<typename T>
		void Generic<T,true>::Copy(const Generic& g)
		{
			length = g.length;

			if (g.string < g.buffer || g.string >= g.buffer + MAX_INT_LENGTH)
			{
				string = g.string;
			}
			else
			{
				string = buffer + (g.string - g.buffer);
				Base::Copy( buffer, g.buffer, MAX_INT_LENGTH );
			}
		}

		template<typename T>
		class Sub : protected Base
		{
		public:

			typedef typename T::Type Type;

			enum
			{
				TYPE_SIZE = typename T::TYPE_SIZE
			};

			template<typename U> 
			void Assign(const U* NST_RESTRICT,uint);

		protected:

			T& string;
			uint offset;
			uint length;

		public:

			Sub(T& t,uint o)
			: string(t), offset(o), length(t.Length() - offset) 
			{
				NST_ASSERT( offset + length <= string.Length() );
			}

			Sub(T& t,uint o,uint l)
			: string(t), offset(o), length(l) 
			{
				NST_ASSERT( offset + length <= string.Length() );
			}

			template<typename U>
			Sub& operator = (const U& t)
			{
				Assign( Base::GetPtr(t), Base::GetLength(t) );
				return *this;
			}
  
			Sub& operator = (Type c)
			{
				Assign( &c, 1 ); 
				return *this; 
			}

			Sub& operator = (int    i) { Base::AssignSigned   ( *this, i ); return *this; }
			Sub& operator = (uint   i) { Base::AssignUnsigned ( *this, i ); return *this; }
			Sub& operator = (schar  i) { Base::AssignSigned   ( *this, i ); return *this; }
			Sub& operator = (uchar  i) { Base::AssignUnsigned ( *this, i ); return *this; }
			Sub& operator = (short  i) { Base::AssignSigned   ( *this, i ); return *this; }
			Sub& operator = (ushort i) { Base::AssignUnsigned ( *this, i ); return *this; }
			Sub& operator = (long   i) { Base::AssignSigned   ( *this, i ); return *this; }
			Sub& operator = (ulong  i) { Base::AssignUnsigned ( *this, i ); return *this; }

			template<typename U>
			bool operator >> (U& t) const
			{
				return Base::ConvertToUnsigned( *this, t );
			}

			void Clear()
			{
				if (const uint n = length)
				{
					length = 0;
					string.Erase( offset, n );
				}
			}

			uint Length() const
			{
				return length;
			}

			bool Empty() const
			{
				return !length;
			}

			Type* Ptr()
			{
				return string.Ptr() + offset;
			}

			const Type* Ptr() const
			{
				return string.Ptr() + offset;
			}

			Type& operator [] (uint i)
			{
				NST_ASSERT( i < length );
				return string[offset+i];
			}

			const Type& operator [] (uint i) const
			{
				NST_ASSERT( i < length );
				return string[offset+i];
			}

			Type& Front()
			{
				NST_ASSERT( length );
				return string[offset];
			}

			const Type& Front() const
			{
				NST_ASSERT( length );
				return string[offset];
			}

			Type& Back()
			{
				NST_ASSERT( length );
				return string[offset+length-1];
			}

			const Type& Back() const
			{
				NST_ASSERT( length );
				return string[offset+length-1];
			}

			bool Wide() const
			{
				return Base::Wide( Ptr(), Length() );
			}

			Generic<Type> operator () (uint p) const
			{
				NST_ASSERT( p <= length );
				return Generic<Type>( string.Ptr() + offset + p, length - p );
			}

			Generic<Type> operator () (uint p,uint l) const
			{
				NST_ASSERT( p + l <= length );
				return Generic<Type>( string.Ptr() + offset + p, l );
			}

			template<typename T> 
			bool IsEqual(const T& t) const 
			{ 
				return Base::IsCaseEqual( *this, t );
			}

			template<typename U> bool operator == (const U& t) const { return  Base::IsEqual( *this, t );      }
			template<typename U> bool operator != (const U& t) const { return !Base::IsEqual( *this, t );      }
			template<typename U> bool operator <  (const U& t) const { return  Base::Compare( *this, t ) <  0; }
			template<typename U> bool operator <= (const U& t) const { return  Base::Compare( *this, t ) <= 0; }
			template<typename U> bool operator >  (const U& t) const { return  Base::Compare( *this, t ) >  0; }
			template<typename U> bool operator >= (const U& t) const { return  Base::Compare( *this, t ) >= 0; }
		};

		template<typename T> template<typename U>
		void Sub<T>::Assign(const U* NST_RESTRICT const t,const uint n)
		{
			const uint l = length;
			length = n;
			string.Erase( offset, l );
			string.Insert( offset, t, n );
		}

		template<uint N,typename T=tchar>
		class Stack : Base
		{
		public:

			typedef T Type;
			typedef Sub< Stack<N,Type> > SubString;

			enum
			{
				TYPE_SIZE = sizeof(Type),
				MAX_LENGTH = N
			};
			
			template<typename U> void Assign(const U* NST_RESTRICT,uint);
			template<typename U> void Append(const U* NST_RESTRICT,uint);
			template<typename U> void Insert(const uint,const U* NST_RESTRICT,uint);
			void Erase(uint,uint);

		private:

			template<typename U> void Assign(const U* NST_RESTRICT);
			template<typename U> void Append(const U* NST_RESTRICT);

			enum
			{
				STACK_LENGTH = (N + 4U) & ~3U
			};

			uint length;

			union
			{
				Type string[STACK_LENGTH];
				u32 empty;
			};

		public:

			Stack()
			: length(0), empty(0) {}

			Stack(const Stack& t)
			{
				Assign( t.Ptr(), t.Length() );
			}

			template<typename U>
			Stack(const U* u,uint n)
			{
				Assign( u, n );
			}

			template<typename U>
			Stack(const U& t)
			{
				Assign( t.Ptr(), t.Length() );
			}

			template<typename U>
			Stack(const U* const& t)
			{
				Assign( t );
			}

			template<typename U>
			Stack(U* const& t)
			{
				Assign( t );
			}

			template<typename U,size_t N>
			Stack(const U (&t)[N])
			{
				Assign( t );
			}

			Stack& operator = (const Stack& t)
			{
				Assign( t.Ptr(), t.Length() );
				return *this;
			}

			template<typename U>
			Stack& operator = (const U& t)
			{
				Assign( t.Ptr(), t.Length() );
				return *this;
			}

			template<typename U>
			Stack& operator = (const U* const& t)
			{
				Assign( t );
				return *this;
			}

			template<typename U>
			Stack& operator = (U* const& t)
			{
				Assign( t );
				return *this;
			}

			template<typename U,size_t N>
			Stack& operator = (const U (&t)[N])
			{
				Assign( t );
				return *this;
			}

			template<typename U>
			Stack& operator << (const U& t)
			{
				Base::Append( *this, t );
				return *this;
			}

			template<typename U>
			bool operator >> (U& t) const
			{
				return Base::ConvertToUnsigned( *this, t );
			}

			uint Length() const
			{
				return length;
			}

			bool Empty() const
			{
				return !length;
			}

			Type* Ptr()
			{
				return string;
			}

			const Type* Ptr() const
			{
				return string;
			}

			Type& operator [] (uint i)
			{
				NST_ASSERT( i < MAX_LENGTH );
				return string[i];
			}

			const Type& operator [] (uint i) const
			{
				NST_ASSERT( i < MAX_LENGTH );
				return string[i];
			}

			Type& At(uint i)
			{
				NST_ASSERT( i < MAX_LENGTH );
				return string[i];
			}

			const Type& At(uint i) const
			{
				NST_ASSERT( i < MAX_LENGTH );
				return string[i];
			}

			Type& Front()
			{
				NST_ASSERT( length );
				return string[0];
			}

			const Type& Front() const
			{
				NST_ASSERT( length );
				return string[0];
			}

			Type& Back()
			{
				NST_ASSERT( length );
				return string[length-1];
			}

			const Type& Back() const
			{
				NST_ASSERT( length );
				return string[length-1];
			}

			void Clear()
			{
				length = 0;
				empty = 0;
			}

			bool Wide() const
			{
				return Base::Wide( Ptr(), Length() );
			}

			uint Validate()
			{
				return length = Base::Length(string);
			}

			void Resize(uint n)
			{
				NST_ASSERT( n <= MAX_LENGTH );
				string[length=n] = '\0';
			}

			void ShrinkTo(uint n)
			{
				Resize( n );
			}
  
			void Remove(const Type c)
			{
				length = Base::Remove( string, c ) - string;
			}

			void Trim()
			{
				if (Base::Trim( string ))
					Validate();
			}

			Generic<Type> operator () (uint p) const
			{
				NST_ASSERT( p <= length );
				return Generic<Type>( string + p, length - p );
			}

			Generic<Type> operator () (uint p,uint l) const
			{
				NST_ASSERT( p + l <= length );
				return Generic<Type>( string + p, l );
			}

			SubString operator () (uint p)
			{
				NST_ASSERT( p <= length );
				return SubString( *this, p, length - p );
			}

			SubString operator () (uint p,uint l)
			{
				NST_ASSERT( p + l <= length );
				return SubString( *this, p, l );
			}

			template<typename T> 
			bool IsEqual(const T& t) const 
			{ 
				return Base::IsCaseEqual( *this, t );
			}

			template<typename U> bool operator == (const U& t) const { return  Base::IsEqual( *this, t );      }
			template<typename U> bool operator != (const U& t) const { return !Base::IsEqual( *this, t );      }
			template<typename U> bool operator <  (const U& t) const { return  Base::Compare( *this, t ) <  0; }
			template<typename U> bool operator <= (const U& t) const { return  Base::Compare( *this, t ) <= 0; }
			template<typename U> bool operator >  (const U& t) const { return  Base::Compare( *this, t ) >  0; }
			template<typename U> bool operator >= (const U& t) const { return  Base::Compare( *this, t ) >= 0; }
		};

		template<uint N,typename T> template<typename U>
		void Stack<N,T>::Assign(const U* NST_RESTRICT const t)
		{
			NST_ASSERT( Base::Length(t) <= MAX_LENGTH );

			uint i = ~0U;
			Type c;

			do 
			{
				++i;
				string[i] = c = t[i];
			} 
			while (c);

			length = i;
		}

		template<uint N,typename T> template<typename U>
		void Stack<N,T>::Assign(const U* NST_RESTRICT const t,const uint n)
		{
			NST_ASSERT( t && n <= MAX_LENGTH );

			length = n;
			Base::Copy( string, t, n );
			string[n] = '\0';
		}

		template<uint N,typename T> template<typename U>
		void Stack<N,T>::Append(const U* NST_RESTRICT const t)
		{
			NST_ASSERT( length + Base::Length(t) <= MAX_LENGTH );

			uint i = length - 1U;
			Type c;

			do 
			{
				++i;
				string[i] = c = t[i];
			} 
			while (c);

			length = i;
		}

		template<uint N,typename T> template<typename U>
		void Stack<N,T>::Append(const U* NST_RESTRICT const t,const uint n)
		{
			NST_ASSERT( t && length + n <= MAX_LENGTH );

			Type* const offset = string + length; 
			length += n;
			Base::Copy( offset, t, n );
			offset[n] = '\0';
		}

		template<uint N,typename T>
		void Stack<N,T>::Erase(const uint p,const uint n)
		{
			NST_ASSERT( p + n <= length );
			
			const uint l = length;
			length -= n;
			Base::Move( string + p, string + p + n, (l + 1) - (p + n) );
		}

		template<uint N,typename T> template<typename U>
		void Stack<N,T>::Insert(const uint pos,const U* NST_RESTRICT const t,const uint n)
		{
			NST_ASSERT( length + n <= MAX_LENGTH );

			if (n)
			{
				const uint p = length;
				length += n;
				Base::Move( string + pos + n, string + pos, p + 1 - pos );
				Base::Copy( string + pos, t, n );
			}
		}

		template<typename T=tchar>
		class Heap : Base
		{
		public:

			typedef T Type;
			typedef Sub< Heap<Type> > SubString;

			enum
			{
				TYPE_SIZE = sizeof(Type)
			};

			template<typename U> void Assign(const U* NST_RESTRICT,uint);
			template<typename U> void Append(const U* NST_RESTRICT,uint);
			template<typename U> void Insert(uint,const U* NST_RESTRICT,uint);

			void Reserve(uint);
			void Resize(uint);
			void Defrag();
			void Destroy();
			void Erase(uint,uint);

		private:

			template<typename U> void Init(const U* NST_RESTRICT,uint);
			template<typename U> void Assign(const U* NST_RESTRICT);
			template<typename U> void Append(const U* NST_RESTRICT);

			Type* string;
			uint length;
			uint capacity;

			static Type empty[4];

			void Allocate(const uint size)
			{
				NST_ASSERT( size && size+1 );
				string = (Type*) std::malloc( (size+1) * TYPE_SIZE );
			}

			void Reallocate(const uint size)
			{
				NST_ASSERT( size && size+1 );
				string = (Type*) std::realloc( string, (size+1) * TYPE_SIZE );
			}

			void Free()
			{
				Type* tmp = string;
				string = empty;
				std::free( tmp );
			}
							   
			template<typename U>
			void Copy(const U* NST_RESTRICT const t,const uint n)
			{
				NST_ASSERT( t );
				Base::Copy( string, t, n );
				string[n] = '\0';
			}

		public:

			Heap()
			: string(empty), length(0), capacity(0) 
			{
				NST_ASSERT( !*empty );
			}

			Heap(const Heap& t)
			{
				Init( t.Ptr(), t.Length() );
			}

			template<typename U>
			Heap(const U& t)
			{
				Init( Base::GetPtr(t), Base::GetLength(t) );
			}

			template<typename U>
			Heap(const U* t,uint n)
			{
				Init( t, n );
			}

			~Heap()
			{
				if (capacity)
					std::free( string );
			}

			Heap& operator = (const Heap& t)
			{
				Assign( t.Ptr(), t.Length() );
				return *this;
			}

			template<typename U>
			Heap& operator = (const U& t)
			{
				Assign( Base::GetPtr(t), Base::GetLength(t) );
				return *this;
			}

			template<typename U>
			Heap& operator << (const U& t)
			{
				Base::Append( *this, t );
				return *this;
			}

			template<typename U>
			bool operator >> (U& t) const
			{
				return Base::ConvertToUnsigned( *this, t );
			}

			template<typename U>
			void Insert(const uint pos,const U& t)
			{
				Insert( pos, Base::GetPtr(t), Base::GetLength(t) );
			}

			void ShrinkTo(uint n)
			{
				NST_ASSERT( capacity >= n );
				string[length = n] = '\0';
			}

			void Clear()
			{
				string[length = 0] = '\0';
			}

			uint Validate()
			{
				return length = Base::Length(string);
			}

			void Remove(const Type c)
			{
				length = Base::Remove( string, c ) - string;
			}

			void Trim()
			{
				if (Base::Trim( string ))
					Validate();
			}

			uint Length() const
			{
				return length;
			}

			uint Capacity() const
			{
				return capacity;
			}

			bool Empty() const
			{
				return !length;
			}

			Type* Ptr()
			{
				return string;
			}

			const Type* Ptr() const
			{
				return string;
			}

			Type& operator [] (uint i)
			{
				return string[i];
			}

			const Type& operator [] (uint i) const
			{
				return string[i];
			}

			Type& At(uint i)
			{
				return string[i];
			}

			const Type& At(uint i) const
			{
				return string[i];
			}

			Type& Front()
			{
				NST_ASSERT( length );
				return string[0];
			}

			const Type& Front() const
			{
				NST_ASSERT( length );
				return string[0];
			}

			Type& Back()
			{
				NST_ASSERT( length );
				return string[length-1];
			}

			const Type& Back() const
			{
				NST_ASSERT( length );
				return string[length-1];
			}

			bool Wide() const
			{
				return Base::Wide( Ptr(), Length() );
			}

			Generic<Type> operator () (uint p) const
			{
				NST_ASSERT( p <= length );
				return Generic<Type>( string + p, length - p );
			}

			Generic<Type> operator () (uint p,uint l) const
			{
				NST_ASSERT( p + l <= length );
				return Generic<Type>( string + p, l );
			}

			SubString operator () (uint p)
			{
				NST_ASSERT( p <= length );
				return SubString( *this, p, length - p );
			}

			SubString operator () (uint p,uint l)
			{
				NST_ASSERT( p + l <= length );
				return SubString( *this, p, l );
			}

			uint FindFirstOf(const Type c,const uint length)
			{
				for (uint i=0; i < length; ++i)
					if (string[i] == c)
						return i;

				return length;
			}

			uint FindFirstOf(const Type c)
			{
				return FindFirstOf( c, length );
			}

			uint FindFirstNotOf(const Type c,const uint length)
			{
				for (uint i=0; i < length; ++i)
					if (string[i] != c)
						return i;

				return length;
			}

			uint FindLastOf(const Type c,const uint length)
			{
				for (uint i=length; i; )
					if (string[--i] == c)
						return i;

				return length;
			}

			uint FindLastNotOf(const Type c,const uint length)
			{
				for (uint i=length; i; )
					if (string[--i] != c)
						return i;

				return length;
			}

			SubString FirstOf(Type c) 
			{ 
				uint first = FindFirstOf(c,length);
				return SubString( *this, first, length - first ); 
			}

			template<typename U> bool operator == (const U& t) const { return  Base::IsEqual( *this, t );      }
			template<typename U> bool operator != (const U& t) const { return !Base::IsEqual( *this, t );      }
			template<typename U> bool operator <  (const U& t) const { return  Base::Compare( *this, t ) <  0; }
			template<typename U> bool operator <= (const U& t) const { return  Base::Compare( *this, t ) <= 0; }
			template<typename U> bool operator >  (const U& t) const { return  Base::Compare( *this, t ) >  0; }
			template<typename U> bool operator >= (const U& t) const { return  Base::Compare( *this, t ) >= 0; }
		};

		template<typename T>
		typename Heap<T>::Type typename Heap<T>::empty[4] = {'\0','\0','\0','\0'};

		template<typename T> template<typename U>
		void Heap<T>::Init(const U* NST_RESTRICT const t,const uint n)
		{
			NST_ASSERT( t && !*empty );

			if (0 != (capacity = length = n))
			{
				Allocate( n );
				Copy( t, n );
			}
			else
			{
				string = empty;
			}
		}

		template<typename T>
		void Heap<T>::Reserve(const uint n)
		{
			if (capacity < n)
			{
				uint prev = capacity;
				capacity = n;

				if (prev)
				{
					Reallocate( n );
				}
				else
				{
					Allocate( n );
					string[0] = '\0';
				}
			}
		}

		template<typename T>
		void Heap<T>::Resize(uint n)
		{
			Reserve( n );
			string[length = n] = '\0';
		}

		template<typename T>
		void Heap<T>::Destroy()
		{
			if (capacity)
			{
				length = capacity = 0;
				Free();
			}
		}

		template<typename T> template<typename U>
		void Heap<T>::Assign(const U* NST_RESTRICT t,uint n)
		{		
			Resize( n );
			Copy( t, n );
		}

		template<typename T> template<typename U>
		void Heap<T>::Assign(const U* NST_RESTRICT t)
		{
			Assign( t, Base::Length(t) );
		}

		template<typename T> template<typename U>
		void Heap<T>::Append(const U* NST_RESTRICT t,uint n)
		{	
			if (capacity < length + n)
			{
				uint prev = capacity;
				capacity = capacity * 2 + n;

				if (prev)
					Reallocate( capacity );
				else
					Allocate( capacity );
			}

			Type* const offset = string + length;
			length += n;
			Base::Copy( offset, t, n );
			offset[n] = '\0';
		}

		template<typename T> template<typename U>
		void Heap<T>::Append(const U* NST_RESTRICT t)
		{
			Append( t, Base::Length(t) );
		}

		template<typename T> template<typename U>
		void Heap<T>::Insert(const uint pos,const U* NST_RESTRICT const t,const uint n)
		{
			if (n)
			{
				if (capacity >= length + n)
				{
					Base::Move( string + pos + n, string + pos, length + 1 - pos );
				}
				else
				{
					capacity = NST_MAX(capacity * 2,length + n);

					Type* const next = (Type*) std::malloc( (capacity+1) * TYPE_SIZE );

					Base::Copy( next, string, pos );
					Base::Copy( next + pos + n, string + pos, length + 1 - pos );

					if (string != empty)
						std::free( string );

					string = next;
				}

				length += n;
				Base::Copy( string + pos, t, n );
			}
		}

		template<typename T>
		void Heap<T>::Erase(const uint p,const uint n)
		{
			NST_ASSERT( p + n <= length );

			const uint l = length;
			length -= n;
			Base::Move( string + p, string + p + n, (l + 1) - (p + n) );
		}
  
		template<typename T>
		void Heap<T>::Defrag()
		{
			if (capacity != length)
			{
				capacity = length;

				if (length)
					Reallocate( length );
				else
					Free();
			}
		}

		template<typename T=tchar>
		class Path : public Heap<T>
		{
		public:

			Path() {}

			Path(Generic<Type>,Generic<Type>,Generic<Type> = Generic<Type>());

			Path(const Path& p)
			: Heap(static_cast<const Heap&>(p)) {}

			template<typename U> 
			Path(const U& t)
			: Heap(t) {}

			Path& operator = (const Path& t)
			{
				Heap::operator = (static_cast<const Heap&>(t));
				return *this;
			}

			template<typename U>
			Path& operator = (const U& t)
			{
				Heap::operator = (t);
				return *this;
			}

			template<typename U>
			Path& operator << (const U& t)
			{
				Heap::operator << (t);
				return *this;
			}

			void Set(Generic<Type>,Generic<Type>,Generic<Type> = Generic<Type>());
			void CheckSlash();

			static const Path Compact(Generic<Type>,uint);

		private:

			static ibool Compact(T*,const T*,uint);

			uint FindDirectory() const;
			void FindExtension(uint (&)[2]) const;
			uint FindArchive() const;
			void FindFileInArchive(uint (&)[2]) const;
			static uint ExtensionId(const Type* NST_RESTRICT,uint);

			class DirectorySub : public SubString
			{		
			public:

				DirectorySub(Path& path,uint length)
				: SubString( path, 0, length ) {}

				void operator  = (Generic<Type>);
				void operator += (Generic<Type>);
				void operator -= (uint);
			};

			class ExtensionSub : public SubString
			{
			public:

				ExtensionSub(Path& path,uint pos,uint length)
				: SubString( path, pos, length ) {}

				void Assign(const Type* NST_RESTRICT,uint);
				void Clear();

				template<typename U>
				void operator = (const U& t)
				{
					Assign( Base::GetPtr(t), Base::GetLength(t) );
				}

				uint Id() const
				{
					return ExtensionId( Ptr(), Length() );
				}
			};

			class ConstExtensionSub : public Generic<Type>
			{
			public:

				ConstExtensionSub(const Type* string,uint length)
				: Generic( string, length ) {}

				uint Id() const
				{
					return ExtensionId( Ptr(), Length() );
				}
			};

			class ArchiveFileSub : public Generic<Type>
			{
			public:

				ArchiveFileSub(const Type* string,uint length)
				: Generic( string, length ) {}

				const Generic File() const
				{
					for (uint i=length; i; )
					{
						switch (string[--i])
						{
							case '\\':
							case '/': return Generic( string + i + 1, length - (i + 1) );
						}
					}

					return *this;
				}
			};

		public:
  
			DirectorySub Directory()
			{
				return DirectorySub( *this, FindDirectory() );
			}

			const Generic<Type> Directory() const
			{
				return Generic<Type>( Ptr(), FindDirectory() );
			}
  
			SubString File()
			{
				return SubString( *this, FindDirectory() );
			}
  
			const Generic<Type> File() const
			{
				uint p = FindDirectory();
				return Generic<Type>( Ptr() + p, Length() - p );
			}
  
			ExtensionSub Extension()
			{
				uint p[2];
				FindExtension( p );
				return ExtensionSub( *this, p[0], p[1] );
			}

			const ConstExtensionSub Extension() const
			{
				uint p[2];
				FindExtension( p );
				return ConstExtensionSub( Ptr() + p[0], p[1] );
			}
  
			const Generic<Type> Archive() const
			{
				return Generic<Type>( Ptr(), FindArchive() );
			}

			ArchiveFileSub FileInArchive() const
			{
				uint p[2];
				FindFileInArchive( p );
				return ArchiveFileSub( Ptr() + p[0], p[1] );
			}
  
			ArchiveFileSub Target() const
			{
				const ArchiveFileSub archive( FileInArchive() );

				if (archive.Length())
					return archive;

				return ArchiveFileSub( Ptr(), Length() );
			}
		};

		template<typename T>
		Path<T>::Path(const Generic<Type> dir,const Generic<Type> file,const Generic<Type> ext)
		{
			Set( dir, file, ext );
		}

		template<typename T>
		void Path<T>::Set(const Generic<Type> dir,const Generic<Type> file,const Generic<Type> ext)
		{
			Clear();
			Reserve( dir.Length() + file.Length() + ext.Length() );

			Directory() = dir;
			File() = file;

			if (ext.Length())
				Extension() = ext;
		}
	  
	    template<typename T>
	    void Path<T>::CheckSlash()
	    {
			if (Length() && Back() != '\\' && Back() != '/')
				operator << ('\\');
	    }

		template<typename T>
		uint Path<T>::FindDirectory() const
		{
			for (uint i=Length(); i; )
			{
				switch (At(--i))
				{
					case '\\':
					case '/': return i + 1;
					case '>': while (i && At(--i) != '<'); break;
				}
			}

			return 0;
		}

		template<typename T>
		void Path<T>::FindExtension(uint (&p)[2]) const
		{
			p[1] = Length();

			for (uint i=Length(); i; )
			{
				switch (At(--i))
				{
					case '.': 
						
						p[0] = i + 1;
						p[1] -= p[0];
						return;

					case '\\':
					case '/': 

						i = 0;
						break;

					case '>': 
						
						while (i && At(--i) != '<');

						if (i && At(i-1) == ' ')
							--i;

						p[1] = i;
						break;
				}
			}

			p[0] = p[1];
			p[1] = 0;
		}

		template<typename T>
		uint Path<T>::FindArchive() const
		{
			for (uint i=Length(); i; )
			{
				if (At(--i) == '>')
				{
					while (i)
					{
						if (At(--i) == '<')
						{
							if (i && At(i-1) == ' ')
								--i;

							return i;
						}
					}					
					break;
				}
			}
				
			return 0;
		}

		template<typename T>
		void Path<T>::FindFileInArchive(uint (&p)[2]) const
		{
			for (uint i=Length(); i; )
			{
				if (At(--i) == '>')
				{
					p[1] = i;

					while (i)
					{
						if (At(--i) == '<')
						{
							p[0] = i + 1;
							p[1] -= p[0];

							return;
						}
					}					
					break;
				}
			}

			p[0] = Length();
			p[1] = 0;
		}

		template<typename T>
		uint Path<T>::ExtensionId(const Type* NST_RESTRICT const string,uint length)
		{
			uint id = 0;

			if (length >= 3)
			{
				if (length > 4)
					length = 4;

				for (uint i=0; i < length; ++i)
				{
					const Type c = string[i];
					
					uint v;

					if (c >= 'a' && c <= 'z')
					{
						v = c;
					}
					else if (c >= 'A' && c <= 'Z')
					{
						v = c - 'A' + 'a';
					}
					else if (c >= '0' && c <= '9')
					{
						v = c;
					}
					else
					{
						return 0;
					}

					id |= v << (8 * i);
				}
			}

			return id;
		}

		template<typename T>
		void Path<T>::DirectorySub::operator = (const Generic<Type> t)
		{
			Clear();

			if (t.Length())
			{
				string.Insert( 0, t.Ptr(), t.Length() );
				length = t.Length();

				if (t.Back() != '\\' && t.Back() != '/')
				{
					const Type c = '\\';
					string.Insert( length++, &c, 1 );
				}
			}
		}

		template<typename T>
		void Path<T>::DirectorySub::operator += (const Generic<Type> t)
		{
			if (t.Length())
			{
				string.Insert( length, t.Ptr(), t.Length() );
				length += t.Length();

				if (t.Back() != '\\' && t.Back() != '/')
				{
					const Type c = '\\';
					string.Insert( length++, &c, 1 );
				}
			}
		}

		template<typename T>
		void Path<T>::DirectorySub::operator -= (uint subs)
		{
			if (subs && length && (Back() == '\\' || Back() == '/'))
			{
				uint i = length - 1;
				uint p = ~0U;

				do
				{
					if (string.At(--i) == '\\' || string.At(i) == '/')
					{
						p = i;

						if (!--subs)
							break;
					}
				}
				while (i);

				if (p != ~0U)
				{
					const uint n = length;
					length = ++p;
					string.Erase( p, n - p );
				}
			}
		}

		template<typename T>
		void Path<T>::ExtensionSub::Clear()
		{
			const uint dot = (offset && string[offset-1] == '.');
			const uint n = length;
			length = 0;
			string.Erase( offset -= dot, n + dot );
		}

		template<typename T>
		void Path<T>::ExtensionSub::Assign(const Type* NST_RESTRICT t,uint n)
		{
			Clear();

			if (n)
			{
				if (*t == '.')
				{
					++t;
					--n;
				}

				if (n)
				{
					length = n;
					const Type c = '.';
					string.Insert( offset++, &c, 1 );
					string.Insert( offset, t, n );
				}
			}
		}

		template<typename T>
		const Path<T> Path<T>::Compact(const Generic<Type> t,const uint maxLength)
		{
			Path path;

			if (t.Length())
			{
				path.Reserve( maxLength );

				if (Compact( path.Ptr(), Path<>(t).Ptr(), maxLength ))
					path.Validate();
				else
					path.Clear();
			}

			return path;
		}

		template<typename T=tchar>
		class Hex
		{
		public:

			typedef T Type;

			enum
			{
				TYPE_SIZE = sizeof(Type)
			};

			explicit Hex(u32,bool=false);
			explicit Hex(u16,bool=false);
			explicit Hex(u8,bool=false);

		private:

			Type buffer[12];
			const Type* const string;
			const uint length;

			void Convert(uint,uint);

		public:

			const Type* Ptr() const
			{
				return string;
			}

			uint Length() const
			{
				return length;
			}
		};

		template<typename T>
		void Hex<T>::Convert(uint number,const uint length)
		{
			Type* it = buffer + length;

			*it = '\0';

			static const Type lut[] = 
			{
				'0','1','2','3','4','5','6','7',
				'8','9','A','B','C','D','E','F'
			};

			do 
			{
				*--it = lut[number % 16];
				number /= 16;
			} 
			while (it != buffer + 2);

			*--it = 'x';
			*--it = '0';
		}

		template<typename T>
		Hex<T>::Hex(const u32 i,const bool n0x)
		: 
		string(n0x ? buffer + 2 : buffer), 
		length(n0x ? 8 : 2+8) 
		{ 
			Convert( i, 2+8 ); 
		}

		template<typename T>
		Hex<T>::Hex(const u16 i,const bool n0x)
		: 
		string(n0x ? buffer + 2 : buffer), 
		length(n0x ? 4 : 2+4) 
		{ 
			Convert( i, 2+4 ); 
		}

		template<typename T>
		Hex<T>::Hex(const u8 i,const bool n0x)
		: 
		string(n0x ? buffer + 2 : buffer), 
		length(n0x ? 2 : 2+2) 
		{ 
			Convert( i, 2+2 ); 
		}
	}

	typedef String::Heap<tchar>         HeapString;
	typedef String::Generic<tchar>      GenericString;
	typedef String::Generic<tchar,true> ValueString;
	typedef String::Hex<tchar>          HexString;
	typedef String::Path<tchar>         Path;
}

#endif
