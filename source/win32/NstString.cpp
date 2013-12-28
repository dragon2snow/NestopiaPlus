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

#include "NstIoFile.hpp"
#include <Windows.h>

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("t", on)
#endif

namespace Nestopia
{
	namespace String
	{
		static const uchar lowerCases[256] =
		{
			0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
			0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
			0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
			0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
			0x40,

			'a','b','c','d','e','f','g','h','i','j','k','l','m',
			'n','o','p','q','r','s','t','u','v','w','x','y','z',

			0x5B,0x5C,0x5D,0x5E,0x5F,
			0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
			0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
			0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
			0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
			0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
			0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
			0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
			0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
			0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
			0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
		};

		static const uchar upperCases[256] =
		{
			0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
			0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
			0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
			0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
			0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
			0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
			0x60,

			'A','B','C','D','E','F','G','H','I','J','K','L','M',
			'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',

			0x7B,0x7C,0x7D,0x7E,0x7F,
			0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
			0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
			0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
			0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
			0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
			0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
			0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
			0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
		};

		int Compare(cstring a,cstring b) throw()
		{
			NST_ASSERT( a && b );

			int c, d;

			do 
			{
				c = lowerCases[ (uchar) (*a++) ];
				d = lowerCases[ (uchar) (*b++) ];
			} 
			while (c && c == d);

			return c - d;
		}

		int Compare(cstring a,cstring b,uint l) throw()
		{
			NST_ASSERT( a && b );

			if (l)
			{
				int c, d;

				do 
				{
					c = lowerCases[ (uchar) (*a++) ];
					d = lowerCases[ (uchar) (*b++) ];
				} 
				while (--l && c && c == d);

				return c - d;
			}

			return 0;
		}

		int CompareNonNull(cstring a,cstring b,uint l) throw()
		{
			NST_ASSERT( a && b );

			int c, d;

			do 
			{
				c = l-- ? lowerCases[ (uchar) (*a++) ] : 0;
				d =       lowerCases[ (uchar) (*b++) ];
			} 
			while (c && c == d);

			return c - d;
		}

		int CompareNonNull(cstring a,cstring b,uint al,uint bl) throw()
		{
			NST_ASSERT( a && b );

			int c, d;

			do
			{
				c = al-- ? lowerCases[ (uchar) (*a++) ] : 0;
				d = bl-- ? lowerCases[ (uchar) (*b++) ] : 0;
			} 
			while (c && c == d);

			return c - d;
		}
	}

	using namespace String;

	uint Private::Length(cstring const string) throw()
	{
		NST_COMPILE_ASSERT( sizeof(u32) == sizeof(char) * 4 );

		const char* offset = string - 1;

		do 
		{
			offset++;

			if (!(reinterpret_cast<UINT_PTR>(offset) & 0x3))
			{
				uint values;

				do 
				{
					values = *reinterpret_cast<const u32*>(offset);
					offset += sizeof(u32);
				} 
				while (!((values - 0x01010101U) & ~values & 0x80808080U));

				offset -= sizeof(u32);
			}
		} 
		while (*offset);

		return offset - string;
	}

	uint Private::Copy(char* NST_RESTRICT dst,cstring NST_RESTRICT src) throw()
	{
		cstring const pos = dst + 1;
		while ((*dst++ = *src++) != 0);
		return dst - pos;
	}

	void Private::Insert
	(
		char* NST_RESTRICT dst,
		const uint size,
		const uint pos,
		cstring const NST_RESTRICT src,
		const uint length
	)   throw()
	{
		dst += pos;
		std::memmove( dst + length, dst, size + 1 - pos );

		if (src)
			std::memcpy( dst, src, length );
	}

	void Private::Erase(char* const string,const uint size,const uint pos,const uint length) throw()
	{
		std::memmove( string + pos, string + pos + length, (size + 1) - (pos + length) );
	}

	uint Private::Remove(char* const string,uint size,const int c) throw()
	{
		NST_ASSERT( c != '\0' );

		if (size)
		{
			char* it = string;
			cstring next = string;
			cstring const end = string + size;

			do
			{
				if (*next != c)
					*it++ = *next;
			}
			while (next++ != end);

			size = it - 1 - string;
		}

		return size;
	}

	uint Private::FindFirstOf(const int character,cstring const string,const uint length) throw()
	{
		for (uint i=0; i < length; ++i)
			if (string[i] == character)
				return i;

		return length;
	}

	uint Private::FindAfterFirstOf(const int character,cstring const string,const uint length) throw()
	{
		for (uint i=0; i < length; ++i)
			if (string[i] == character)
				return i + 1;

		return length;
	}

	uint Private::FindFirstNotOf(const int character,cstring const string,const uint length) throw()
	{
		for (uint i=0; i < length; ++i)
			if (string[i] != character)
				return i;

		return length;
	}

	uint Private::FindLastOf(const int character,cstring const string,const uint length) throw()
	{
		for (uint i=length; i; )
			if (string[--i] == character)
				return i;

		return length;
	}

	uint Private::FindAfterLastOf(const int character,cstring const string,const uint length) throw()
	{
		for (uint i=length; i; )
			if (string[--i] == character)
				return i + 1;

		return length;
	}

	uint Private::FindLastNotOf(const int character,cstring const string,const uint length) throw()
	{
		for (uint i=length; i; )
			if (string[--i] != character)
				return i;

		return length;
	}

	namespace
	{
		static inline cstring FindFirst(cstring NST_RESTRICT haystack,const uchar first)
		{
			do
			{
				if (lowerCases[uchar(*haystack)] == first)
					return haystack;
			}
			while (*haystack++);

			return NULL;
		}
	}

	cstring Private::Find(cstring haystack,cstring const needle,const uint length) throw()
	{
		for 
		(
			const uchar first = lowerCases[uchar(*needle)]; 
    		(haystack = FindFirst( haystack, first )) != NULL; 
    		++haystack
		)
		{
			if (Compare( haystack, needle, length ) == 0)
				return haystack;
		}

		return NULL;
	}

	void Private::MakeLowerCase(char* const NST_RESTRICT string,uint length) throw()
	{
		while (length--)
			string[length] = lowerCases[ (uchar) string[length] ];
	}

	void Private::MakeUpperCase(char* const NST_RESTRICT string,uint length) throw()
	{
		while (length--)
			string[length] = upperCases[ (uchar) string[length] ];
	}

	char* Private::FromSigned(char (&buffer)[12],const i32 number) throw()
	{
		uint value = (uint) (number >= 0 ? number : -number);

		char* it = buffer + 11;

		*it = '\0';

		do 
		{
			*--it = (char) ((value % 10) + '0');
		} 
		while (value /= 10);

		if (number < 0)
			*--it = '-';

		return it;
	}

	char* Private::FromUnsigned(char (&buffer)[12],u32 number) throw()
	{
		char* it = buffer + 11;

		*it = '\0';

		do 
		{
			*--it = (char) ((number % 10) + '0');
		} 
		while (number /= 10);

		return it;
	}

	ibool Private::ToUnsigned(cstring NST_RESTRICT string,u32& NST_RESTRICT result) throw()
	{
		result = 0;

		ibool neg = FALSE;
		uint base = 10;
		uint maxvalue = 0xFFFFFFFFU / 10U;

		switch (*string)
		{
			case '-': neg = TRUE;
			case '+': ++string; break;		
			case '0':
		
				base = 16;
				maxvalue = 0xFFFFFFFFU / 16U;
		
				switch (*(++string)++)
				{
					case 'x':
					case 'X': break;
					case '\0': return TRUE;
					default: return FALSE;
				}
				break;
		}

		if (uint digit = *string++)
		{
			uint value = 0;

			do
			{
				if (digit >= '0' && digit <= '9')
				{
					digit -= '0';
				}
				else
				{
					digit = String::upperCases[digit];

					if (digit >= 'A' && digit <= 'F' && base == 16)
					{
						digit = digit - 'A' + 10;
					}
					else
					{
						return FALSE;
					}
				}

				if (value < maxvalue || (value == maxvalue && digit <= 0xFFFFFFFFU % base)) 
					value = value * base + digit;
				else 
					return FALSE;
			}
			while ((digit = *string++) != 0);

			result = neg ? uint(-int(value)) : value;
			return TRUE;
		}

		return FALSE;
	}

	uint Private::Trim(char* const string,const uint length) throw()
	{
		uint last = FindLastNotOf( ' ', string, length );

		if (last != length)
		{
			if (uint first = FindFirstNotOf( ' ', string, ++last ))
			{
				last -= first;
				std::memmove( string, string + first, last );
			}
		}
		else
		{
			last = 0;
		}

		string[last] = '\0';

		return last;
	}

	uint Private::ExtensionId(cstring const NST_RESTRICT string) throw()
	{
		uint id = 0;

		for (uint i=0; i < 4 && string[i] && string[i] != '>' && string[i] != ' '; ++i)
			id |= lowerCases[ (uchar) string[i] ] << (8 * i);

		return id;
	}

	ibool Private::FileExist(cstring const string)
	{
		return Io::File::FileExist( string );
	}

	ibool Private::DirExist(cstring const string)
	{
		return Io::File::DirExist( string );
	}

	using namespace Private;

	char Dynamic::empty[4] = {};

	Dynamic::Dynamic(const uint length,char* const stack)
	: 
	data     (length ? new char [(length + 7) & ~3U] : stack),
	capacity (length),
	size     (length)
	{}

	Dynamic::Dynamic(const uint length,char* const stack,const uint minimum)
	: 
	data     (length <= minimum ? stack : new char [(length + 7) & ~3U]),
	capacity (NST_MAX(length,minimum)),
	size     (length)
	{}

	Dynamic::Dynamic(const Dynamic& NST_RESTRICT string,char* const stack,const uint minimum)
	: 
	data     (string.size <= minimum ? stack : new char [(string.size + 7) & ~3U]),
	capacity (NST_MAX(string.size,minimum)),
	size     (string.size)
	{
		std::memcpy( data, string.data, (string.size + 4) & ~3U );
	}

	Dynamic::Dynamic(cstring const NST_RESTRICT string,char* const stack)
	: size(Length(string))
	{
		data = stack;

		if ((capacity = size) != 0)
			std::memcpy( data = new char [(size + 7) & ~3U], string, size + 1 );
	}

	Dynamic::Dynamic(cstring const NST_RESTRICT string,char* const stack,const uint minimum)
	: size(Length(string))
	{
		if (size < minimum)
		{
			capacity = minimum;
			data = stack;
		}
		else
		{
			capacity = size;
			data = new char [(size + 7) & ~3U];
		}

		std::memcpy( data, string, size + 1 );
	}

	void Dynamic::Destroy(char* const stack,const uint minimum)
	{
		size = 0;

		if (data != stack)
		{
			char* const tmp = data;
			data = stack;
			capacity = minimum;
			delete [] tmp;
		}
	}

	NST_NO_INLINE void Dynamic::Allocate(const uint length,char* const stack)
	{
		NST_ASSERT( length );

		if (data != stack)
		{
			char* const tmp = data;
			data = stack;
			delete [] tmp;
		}

		capacity = length;
		data = new char [(length + 7) & ~3U];
	}

	NST_NO_INLINE void Dynamic::Reallocate(const uint length,char* const stack)
	{
		NST_ASSERT( length );

		capacity = NST_MAX(capacity * 2,length);

		char* const next = new char [(capacity + 7) & ~3U];
		char* const prev = data;
		data = next;

		std::memcpy( next, prev, (size + 4) & ~3U );

		if (prev != stack)
			delete [] prev;
	}

	void Dynamic::Defrag(char* const stack,const uint minimum)
	{
		if (capacity > size && data != stack)
		{
			char* const prev = data;
			capacity = NST_MAX(size,minimum);
			data = size <= minimum ? stack : new char [(size + 7) & ~3U];
			std::memcpy( data, prev, (size + 4) & ~3U );
			delete [] prev;
		}
	}

	inline void Dynamic::Reset(const uint request,char* const stack)
	{
		if (capacity < request)
			Allocate( request, stack );
	}

	void Dynamic::Assign(cstring const NST_RESTRICT string,const uint length,char* const stack)
	{
		Reset( length, stack );
		std::memcpy( data, string, (size=length) );
		data[length] = '\0';
	}

	void Dynamic::AssignTerminated(cstring const NST_RESTRICT string,char* const stack)
	{
		Assign( string, Length(string), stack );
	}

	void Dynamic::Append(cstring const NST_RESTRICT string,const uint length,char* const stack)
	{
		Reserve( size + length, stack );
		char* const offset = data + size; size += length;
		std::memcpy( offset, string, length );
		offset[length] = '\0';
	}

	void Dynamic::AppendTerminated(cstring const NST_RESTRICT string,char* const stack)
	{
		Append( string, Length(string), stack );
	}

	void Dynamic::Insert(const uint pos,cstring const NST_RESTRICT string,const uint length,char* const stack)
	{
		if (length)
		{
			if (capacity >= size + length)
			{
				std::memmove( data + pos + length, data + pos, size + 1 - pos );
			}
			else
			{
				capacity = NST_MAX(capacity * 2,size + length);

				char* const next = new char [(capacity + 7) & ~3U];

				std::memcpy( next, data, (pos + 4) & ~3U );
				std::memcpy( next + pos + length, data + pos, (size + 4 - pos) & ~3U );

				if (data != stack)
					delete [] data;

				data = next;
			}

			size += length;

			if (string)
				std::memcpy( data + pos, string, length );
		}
	}

	void Dynamic::Parse(cstring NST_RESTRICT src,const uint length,char* const stack)
	{
		if (length)
		{
			Reserve( size + (length * 2), stack );

			char* dst = data + size;
			cstring const end = src + length;

			do
			{
				uint ch = (uint) *src++;

				if (ch > 31)
				{
					*dst++ = (char) ch;
				}
				else if (ch == '\n')
				{
					*dst++ = '\r';
					*dst++ = '\n';
				}
				else if (ch == '\t')
				{
					*dst++ = ' ';
				}
			}
			while (src != end);

			*dst = '\0';
			size = dst - data;
		}
	}

	Basic<0U,false>::Basic(const Basic& NST_RESTRICT basic)
	: Dynamic(basic.size) 
	{
		std::memcpy( data, basic.data, (basic.size + 4) & ~3U );
		NST_ASSERT( data[size] == '\0' );
	}

	Basic<0U,false>::Basic(cstring const string)
	: Dynamic(string) {}

	Basic<0U,false>::Basic(const uint length)
	: Dynamic(length) {}

	void Basic<0U,false>::operator = (const Basic& NST_RESTRICT basic)
	{
		Dynamic::Assign( basic.data, basic.size );
	}

	void Basic<0U,false>::AssignTerminated(cstring const string)
	{
		Dynamic::AssignTerminated( string );
	}

	void Basic<0U,false>::AppendTerminated(cstring const string)
	{
		Dynamic::AppendTerminated( string );
	}

	void Basic<0U,false>::Append(cstring const string,const uint length)
	{
		Dynamic::Append( string, length );
	}

	void Basic<0U,false>::Assign(cstring const string,const uint length)
	{
		Dynamic::Assign( string, length );
	}

	void Basic<0U,false>::Reserve(uint const length)
	{
		Dynamic::Reserve( length );
	}

	void Basic<0U,false>::Resize(const uint length)
	{
		Reserve( length );
		data[size = length] = '\0';
	}

	void Basic<0U,false>::Defrag()
	{
		Dynamic::Defrag();
	}

	void Basic<0U,false>::Destroy()
	{
		Dynamic::Destroy();
	}

	char* Basic<0U,false>::Export()
	{
		char* tmp = data;
		data = empty;
		capacity = 0;
		size = 0;
		return tmp;
	}

	Hex::Hex(const u32 i,const bool n0x) throw() 
	: 
	string(n0x ? buffer + 2 : buffer), 
	length(n0x ? 8 : 2+8) 
	{ 
		Convert( i, 2+8 ); 
	}

	Hex::Hex(const u16 i,const bool n0x) throw() 
	: 
	string(n0x ? buffer + 2 : buffer), 
	length(n0x ? 4 : 2+4) 
	{ 
		Convert( i, 2+4 ); 
	}

	Hex::Hex(const u8 i,const bool n0x) throw() 
	: 
	string(n0x ? buffer + 2 : buffer), 
	length(n0x ? 2 : 2+2) 
	{ 
		Convert( i, 2+2 ); 
	}

	void Hex::Convert(uint number,const uint length) throw()
	{
		char* it = buffer + length;

		*it = '\0';

		static const char lut[] = 
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
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
