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

#ifndef NST_TYPES_H
#error Do not include NstSizedInt.hpp directly!
#endif

#if CHAR_BIT != 8
#error Unsupported plattform!
#endif

namespace Nes
{
	typedef signed char i8;
	typedef unsigned char u8;

	namespace Core
	{
		namespace Sized
		{
			template<int> struct Type {};

			template<> struct Type< 1  > { typedef signed   int   i16; };
			template<> struct Type< 2  > { typedef signed   short i16; };
			template<> struct Type< 3  > { typedef signed   long  i16; };
			template<> struct Type< 4  > { typedef unsigned int   u16; };
			template<> struct Type< 5  > { typedef unsigned short u16; };
			template<> struct Type< 6  > { typedef unsigned long  u16; };
			template<> struct Type< 7  > { typedef signed   int   i32; };
			template<> struct Type< 8  > { typedef signed   short i32; };
			template<> struct Type< 9  > { typedef signed   long  i32; };
			template<> struct Type< 10 > { typedef unsigned int   u32; };
			template<> struct Type< 11 > { typedef unsigned short u32; };
			template<> struct Type< 12 > { typedef unsigned long  u32; };
		}
	}

	typedef Core::Sized::Type<sizeof(int) == 2 ?  1 : sizeof(short) == 2 ?  2 : sizeof(long) == 2 ?  3 : 0>::i16 i16;
	typedef Core::Sized::Type<sizeof(int) == 2 ?  4 : sizeof(short) == 2 ?  5 : sizeof(long) == 2 ?  6 : 0>::u16 u16;
	typedef Core::Sized::Type<sizeof(int) == 4 ?  7 : sizeof(short) == 4 ?  8 : sizeof(long) == 4 ?  9 : 0>::i32 i32;
	typedef Core::Sized::Type<sizeof(int) == 4 ? 10 : sizeof(short) == 4 ? 11 : sizeof(long) == 4 ? 12 : 0>::u32 u32;

	typedef Core::Sized::Type<sizeof(int) >= 4 ? 10 : 12>::u32 udword;
	typedef Core::Sized::Type<sizeof(int) >= 4 ?  7 :  9>::i32 idword;

	typedef udword dword;
	typedef idword iword;

	enum
	{
		b0, b1
	};

	enum
	{
		b00, b01, b10, b11
	};

	enum
	{
		b000, b001, b010, b011, b100, b101, b110, b111
	};

	enum
	{
		b0000, b0001, b0010, b0011, b0100, b0101, b0110, b0111,
		b1000, b1001, b1010, b1011, b1100, b1101, b1110, b1111
	};

	enum
	{
		b00000, b00001, b00010, b00011, b00100, b00101, b00110, b00111,
		b01000, b01001, b01010, b01011, b01100, b01101, b01110, b01111,
		b10000, b10001, b10010, b10011, b10100, b10101, b10110, b10111,
		b11000, b11001, b11010, b11011, b11100, b11101, b11110, b11111
	};

	enum
	{
		b000000, b000001, b000010, b000011, b000100, b000101, b000110, b000111,
		b001000, b001001, b001010, b001011, b001100, b001101, b001110, b001111,
		b010000, b010001, b010010, b010011, b010100, b010101, b010110, b010111,
		b011000, b011001, b011010, b011011, b011100, b011101, b011110, b011111,
		b100000, b100001, b100010, b100011, b100100, b100101, b100110, b100111,
		b101000, b101001, b101010, b101011, b101100, b101101, b101110, b101111,
		b110000, b110001, b110010, b110011, b110100, b110101, b110110, b110111,
		b111000, b111001, b111010, b111011, b111100, b111101, b111110, b111111
	};

	enum
	{
		b0000000, b0000001, b0000010, b0000011, b0000100, b0000101, b0000110, b0000111,
		b0001000, b0001001, b0001010, b0001011, b0001100, b0001101, b0001110, b0001111,
		b0010000, b0010001, b0010010, b0010011, b0010100, b0010101, b0010110, b0010111,
		b0011000, b0011001, b0011010, b0011011, b0011100, b0011101, b0011110, b0011111,
		b0100000, b0100001, b0100010, b0100011, b0100100, b0100101, b0100110, b0100111,
		b0101000, b0101001, b0101010, b0101011, b0101100, b0101101, b0101110, b0101111,
		b0110000, b0110001, b0110010, b0110011, b0110100, b0110101, b0110110, b0110111,
		b0111000, b0111001, b0111010, b0111011, b0111100, b0111101, b0111110, b0111111,
		b1000000, b1000001, b1000010, b1000011, b1000100, b1000101, b1000110, b1000111,
		b1001000, b1001001, b1001010, b1001011, b1001100, b1001101, b1001110, b1001111,
		b1010000, b1010001, b1010010, b1010011, b1010100, b1010101, b1010110, b1010111,
		b1011000, b1011001, b1011010, b1011011, b1011100, b1011101, b1011110, b1011111,
		b1100000, b1100001, b1100010, b1100011, b1100100, b1100101, b1100110, b1100111,
		b1101000, b1101001, b1101010, b1101011, b1101100, b1101101, b1101110, b1101111,
		b1110000, b1110001, b1110010, b1110011, b1110100, b1110101, b1110110, b1110111,
		b1111000, b1111001, b1111010, b1111011, b1111100, b1111101, b1111110, b1111111
	};

	enum
	{
		b00000000, b00000001, b00000010, b00000011, b00000100, b00000101, b00000110, b00000111,
		b00001000, b00001001, b00001010, b00001011, b00001100, b00001101, b00001110, b00001111,
		b00010000, b00010001, b00010010, b00010011, b00010100, b00010101, b00010110, b00010111,
		b00011000, b00011001, b00011010, b00011011, b00011100, b00011101, b00011110, b00011111,
		b00100000, b00100001, b00100010, b00100011, b00100100, b00100101, b00100110, b00100111,
		b00101000, b00101001, b00101010, b00101011, b00101100, b00101101, b00101110, b00101111,
		b00110000, b00110001, b00110010, b00110011, b00110100, b00110101, b00110110, b00110111,
		b00111000, b00111001, b00111010, b00111011, b00111100, b00111101, b00111110, b00111111,
		b01000000, b01000001, b01000010, b01000011, b01000100, b01000101, b01000110, b01000111,
		b01001000, b01001001, b01001010, b01001011, b01001100, b01001101, b01001110, b01001111,
		b01010000, b01010001, b01010010, b01010011, b01010100, b01010101, b01010110, b01010111,
		b01011000, b01011001, b01011010, b01011011, b01011100, b01011101, b01011110, b01011111,
		b01100000, b01100001, b01100010, b01100011, b01100100, b01100101, b01100110, b01100111,
		b01101000, b01101001, b01101010, b01101011, b01101100, b01101101, b01101110, b01101111,
		b01110000, b01110001, b01110010, b01110011, b01110100, b01110101, b01110110, b01110111,
		b01111000, b01111001, b01111010, b01111011, b01111100, b01111101, b01111110, b01111111,
		b10000000, b10000001, b10000010, b10000011, b10000100, b10000101, b10000110, b10000111,
		b10001000, b10001001, b10001010, b10001011, b10001100, b10001101, b10001110, b10001111,
		b10010000, b10010001, b10010010, b10010011, b10010100, b10010101, b10010110, b10010111,
		b10011000, b10011001, b10011010, b10011011, b10011100, b10011101, b10011110, b10011111,
		b10100000, b10100001, b10100010, b10100011, b10100100, b10100101, b10100110, b10100111,
		b10101000, b10101001, b10101010, b10101011, b10101100, b10101101, b10101110, b10101111,
		b10110000, b10110001, b10110010, b10110011, b10110100, b10110101, b10110110, b10110111,
		b10111000, b10111001, b10111010, b10111011, b10111100, b10111101, b10111110, b10111111,
		b11000000, b11000001, b11000010, b11000011, b11000100, b11000101, b11000110, b11000111,
		b11001000, b11001001, b11001010, b11001011, b11001100, b11001101, b11001110, b11001111,
		b11010000, b11010001, b11010010, b11010011, b11010100, b11010101, b11010110, b11010111,
		b11011000, b11011001, b11011010, b11011011, b11011100, b11011101, b11011110, b11011111,
		b11100000, b11100001, b11100010, b11100011, b11100100, b11100101, b11100110, b11100111,
		b11101000, b11101001, b11101010, b11101011, b11101100, b11101101, b11101110, b11101111,
		b11110000, b11110001, b11110010, b11110011, b11110100, b11110101, b11110110, b11110111,
		b11111000, b11111001, b11111010, b11111011, b11111100, b11111101, b11111110, b11111111
	};

#if defined(ULLONG_MAX)

	typedef unsigned long long u64;
	#define NST_NATIVE_U64

#elif defined(_MSC_VER)

	typedef unsigned __int64 u64;
	#define NST_NATIVE_U64

#elif defined(NST_U64_DEFINED)

	typedef ::u64 u64;

#else

	class u64
	{
		void Multiply(u64);
		static void Divide(u64&,const u64,bool);

		u32 lo;
		u32 hi;

	public:

		u64() {}

		template<typename V>
		u64(const V& v)
		: lo(v), hi(0) {}

		u64(u32 msdw,u32 lsdw=0)
		: lo(lsdw), hi(msdw) {}

		u64(const u64& v)
		: lo(v.lo), hi(v.hi) {}

		template<typename V>
		u64& operator = (const V& v)
		{
			lo = v;
			hi = 0;
			return *this;
		}

		u64& operator = (const u64& v)
		{
			lo = v.lo;
			hi = v.hi;
			return *this;
		}

		template<typename V>
		u64& operator += (const V& v)
		{
			u32 t = lo;
			lo += v;
			hi += (t > lo);
			return *this;
		}

		u64& operator += (const u64& v)
		{
			u32 t = lo;
			lo += v.lo;
			hi += (t > lo) + v.hi;
			return *this;
		}

		template<typename V>
		u64& operator -= (const V& v)
		{
			u32 t = lo;
			lo -= v;
			hi -= (t < lo);
			return *this;
		}

		u64& operator -= (const u64& v)
		{
			u32 t = lo;
			lo -= v.lo;
			hi -= (t < lo) + v.hi;
			return *this;
		}

		u64 operator ++ (int)
		{
			u64 t;
			t.lo = lo++;
			t.hi = hi;
			hi += t.lo > lo;
			return t;
		}

		u64& operator ++ ()
		{
			u32 t = lo++;
			hi += t > lo;
			return *this;
		}

		u64 operator -- (int)
		{
			u64 t;
			t.lo = lo--;
			t.hi = hi;
			hi -= t.lo < lo;
			return t;
		}

		u64& operator -- ()
		{
			u32 t = lo--;
			hi += t > lo;
			return *this;
		}

		template<typename V>
		u64& operator *= (const V& v)
		{
			if (!(((lo | v) & 0xFFFF0000UL) | hi))
				lo *= v;
			else
				Multiply( u64(v) );

			return *this;
		}

		u64& operator *= (const u64& v)
		{
			Multiply( v );
			return *this;
		}

		template<typename V>
		u64& operator /= (const V& v)
		{
			if (!hi)
				lo /= v;
			else
				Divide( *this, u64(v), false );

			return *this;
		}

		u64& operator /= (const u64& v)
		{
			if (hi | v.hi)
				Divide( *this, v, false );
			else
				lo /= v.lo;

			return *this;
		}

		template<typename V>
		u64& operator %= (const V& v)
		{
			if (!hi)
				lo %= v;
			else
				Divide( *this, u64(v), true );

			return *this;
		}

		u64& operator %= (const u64& v)
		{
			Divide( *this, v, true );
			return *this;
		}

		template<typename V> u64 operator + (const V& v) const { return u64(*this) += v; }
		template<typename V> u64 operator - (const V& v) const { return u64(*this) -= v; }
		template<typename V> u64 operator * (const V& v) const { return u64(*this) *= v; }
		template<typename V> u64 operator / (const V& v) const { return u64(*this) /= v; }
		template<typename V> u64 operator % (const V& v) const { return u64(*this) %= v; }

		template<typename V> u64& operator |= (const V& v) { lo |= v;         return *this; }
		template<typename V> u64& operator &= (const V& v) { lo &= v; hi = 0; return *this; }
		template<typename V> u64& operator ^= (const V& v) { lo ^= v;         return *this; }

		u64& operator |= (const u64& v) { lo |= v.lo; hi |= v.hi; return *this; }
		u64& operator &= (const u64& v) { lo &= v.lo; hi &= v.hi; return *this; }
		u64& operator ^= (const u64& v) { lo ^= v.lo; hi ^= v.hi; return *this; }

		template<typename V> u64 operator | (const V& v) const { return u64( hi, lo | v ); }
		template<typename V> u64 operator & (const V& v) const { return u64(     lo & v ); }
		template<typename V> u64 operator ^ (const V& v) const { return u64( hi, lo ^ v ); }

		u64 operator | (const u64& v) const { return u64( hi | v.hi, lo | v.lo ); }
		u64 operator & (const u64& v) const { return u64( hi & v.hi, lo & v.lo ); }
		u64 operator ^ (const u64& v) const { return u64( hi ^ v.hi, lo ^ v.lo ); }

		u64& operator >>= (uint v)
		{
			if (v)
			{
				if (v < 32)
				{
					u32 t = hi << (32-v);
					hi = (hi >> v);
					lo = (lo >> v) | t;
				}
				else
				{
					lo = hi >> (v-32);
					hi = 0;
				}
			}
			return *this;
		}

		u64& operator <<= (uint v)
		{
			if (v)
			{
				if (v < 32)
				{
					u32 t = lo >> (32-v);
					lo = (lo << v);
					hi = (hi << v) | t;
				}
				else
				{
					hi = lo << (v-32);
					lo = 0;
				}
			}
			return *this;
		}

		u64 operator >> (uint v) const { return u64(*this) >>= v; }
		u64 operator << (uint v) const { return u64(*this) <<= v; }

		u64 operator ~() const
		{
			return u64( ~hi, ~lo );
		}

		template<typename V>
		bool operator == (const V& v) const
		{
			return !((lo - v) | hi);
		}

		bool operator == (const u64& v) const
		{
			return !((lo - v.lo) | (hi - v.hi));
		}

		template<typename V>
		bool operator < (const V& v) const
		{
			return (lo < v && !hi);
		}

		bool operator < (const u64& v) const
		{
			return (hi < v.hi) || (lo < v.lo && hi == v.hi);
		}

		template<typename V>
		bool operator <= (const V& v) const
		{
			return (lo <= v && !hi);
		}

		bool operator <= (const u64& v) const
		{
			return (hi < v.hi) || (hi == v.hi ? (lo <= v.lo) : false);
		}

		template<typename V>
		bool operator != (const V& v) const
		{
			return !(*this == v);
		}

		template<typename V>
		bool operator > (const V& v) const
		{
			return !(*this <= v);
		}

		template<typename V>
		bool operator >= (const V& v) const
		{
			return !(*this < v);
		}

		bool operator !() const
		{
			return !(lo|hi);
		}

		operator bool() const
		{
			return (lo|hi);
		}

		operator int    () const { return lo; }
		operator uint   () const { return lo; }
		operator char   () const { return lo; }
		operator schar  () const { return lo; }
		operator uchar  () const { return lo; }
		operator short  () const { return lo; }
		operator ushort () const { return lo; }
		operator long   () const { return lo; }
		operator ulong  () const { return lo; }
	};

#endif

	typedef u64 qword;
}
