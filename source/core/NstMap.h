////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
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

#pragma once

#ifndef NST_MAP_H
#define NST_MAP_H

#include "../paradox/PdxLibrary.h"	

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

template<ULONG SIZE> class MAP
{
private:

	enum
	{
		OVERFLOW_SIZE = 8,
		JAM_OPCODE = 0x02
	};

public:

	struct PORT
	{
		PORT()
		: 
		object (NULL),
		reader (NULL),
		writer (NULL)
		{}

		struct OBJECT {};

		typedef VOID (OBJECT::*WRITER)(const UINT,const UINT);
		typedef UINT (OBJECT::*READER)(const UINT);

		inline UINT Peek(const UINT address)
		{ 
			PDX_ASSERT(address < SIZE);
			return (*object.*reader)(address); 
		}

		inline VOID Poke(const UINT address,const UINT data)
		{ 
			PDX_ASSERT(address < SIZE);
			(*object.*writer)(address,data); 
		}

		template<class TYPE>
		inline TYPE* Object() const
		{
			return PDX_CAST(TYPE*,object);
		}

		template<class TYPE>
		inline UINT (TYPE::*Reader() const) (const UINT)
		{
			typedef UINT (TYPE::*READER)(const UINT);
			return PDX_CAST_REF(READER,reader);
		}

		template<class TYPE>
		inline VOID (TYPE::*Writer() const) (const UINT,const UINT)
		{
			typedef VOID (TYPE::*WRITER)(const UINT,const UINT);
			return PDX_CAST_REF(WRITER,reader);
		}

		OBJECT* object;
		WRITER writer;
		READER reader;
	};

	MAP()
	: map(new PORT[SIZE+OVERFLOW_SIZE]) 
	{
		for (ULONG i=0; i < OVERFLOW_SIZE; ++i)
		{
      		map[i + SIZE].object = PDX_CAST( PORT::OBJECT*, this          );
    		map[i + SIZE].reader = PDX_CAST( PORT::READER,  ReadOverflow  ); 
    		map[i + SIZE].writer = PDX_CAST( PORT::WRITER,  WriteOverflow );
		}
	}

	~MAP()
	{
		delete [] map;
	}

	inline UINT operator [] (const UINT address) const
	{
		PDX_ASSERT(address < SIZE);
		return (*map[address].object.*map[address].reader)(address);
	}

	inline UINT Peek(const UINT address) const
	{
		PDX_ASSERT(address < SIZE);
		return (*map[address].object.*map[address].reader)(address);
	}

	inline VOID Poke(const UINT address,const UINT data) const
	{
		PDX_ASSERT(address < SIZE);
		(*map[address].object.*map[address].writer)(address,data);
	}

	template<class TYPE>
	inline TYPE* Object(const UINT address) const
	{
		PDX_ASSERT(address < SIZE);
		return map[address].Object<TYPE>();
	}

	template<class TYPE>
	inline UINT (TYPE::*Reader(const UINT address) const) (const UINT)
	{
		PDX_ASSERT(address < SIZE);
		return map[address].reader<TYPE>();
	}

	template<class TYPE>
	inline VOID (TYPE::*Writer(const UINT address) const) (const UINT,const UINT)
	{
		PDX_ASSERT(address < SIZE);
		return map[address].writer<TYPE>();
	}

	template<class OBJECT,class READER,class WRITER> 
	VOID SetPort(const UINT,OBJECT*,READER,WRITER);

	template<class OBJECT,class READER,class WRITER> 
	VOID SetPort(const UINT,const UINT,OBJECT*,READER,WRITER);

	inline PORT& GetPort(const UINT address)
	{
		PDX_ASSERT(address < SIZE);
		return map[address];
	}

	inline const PORT& GetPort(const UINT address) const
	{
		PDX_ASSERT(address < SIZE);
		return map[address];
	}

private:

	VOID WriteOverflow(const UINT,const UINT) 
	{
		PDX_DEBUG_BREAK_MSG("Memory Map Write overflow");
	}
	
	UINT ReadOverflow(const UINT) 
	{ 
		PDX_DEBUG_BREAK_MSG("Memory Map Read overflow");
		return JAM_OPCODE; 
	}

	PORT* const map;
};

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<ULONG SIZE> template<class OBJECT,class READER,class WRITER> 
VOID MAP<SIZE>::SetPort(const UINT address,OBJECT* object,READER reader,WRITER writer)
{
	PDX_ASSERT(object && reader && writer && address < SIZE);

	map[address].object = PDX_CAST_REF( PORT::OBJECT*,object );
	map[address].reader = PDX_CAST_REF( PORT::READER,reader  ); 
	map[address].writer = PDX_CAST_REF( PORT::WRITER,writer  );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<ULONG SIZE> template<class OBJECT,class READER,class WRITER> 
VOID MAP<SIZE>::SetPort(const UINT first,const UINT last,OBJECT* object,READER reader,WRITER writer)
{
	PDX_ASSERT((first <= last) && (last < SIZE) && object && reader && writer);

	for (ULONG address=first; address <= last; ++address)
	{
		map[address].object = PDX_CAST_REF( PORT::OBJECT*,object );
		map[address].reader = PDX_CAST_REF( PORT::READER,reader  ); 
		map[address].writer = PDX_CAST_REF( PORT::WRITER,writer  );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

typedef MAP<0x0400U>      MAP_1K;
typedef MAP<0x0800U>      MAP_2K;
typedef MAP<0x1000U>      MAP_4K;
typedef MAP<0x2000U>      MAP_8K;
typedef MAP<0x4000U>      MAP_16K;
typedef MAP<0x8000U>      MAP_32K;
typedef MAP<0x00010000UL> MAP_64K;
typedef MAP<0x00020000UL> MAP_128K;
typedef MAP<0x00040000UL> MAP_256K;
typedef MAP<0x00080000UL> MAP_512K;
typedef MAP<0x00100000UL> MAP_1024K;

typedef MAP_1K::PORT	PORT_1K;
typedef MAP_2K::PORT	PORT_2K;
typedef MAP_4K::PORT	PORT_4K;
typedef MAP_8K::PORT	PORT_8K;
typedef MAP_16K::PORT   PORT_16K;
typedef MAP_32K::PORT   PORT_32K;
typedef MAP_64K::PORT   PORT_64K;
typedef MAP_128K::PORT  PORT_128K;
typedef MAP_256K::PORT  PORT_256K;
typedef MAP_512K::PORT  PORT_512K;
typedef MAP_1024K::PORT PORT_1024K;

typedef MAP_64K  CPU_MAP;
typedef MAP_16K  PPU_MAP;
typedef PORT_64K CPU_PORT;
typedef PORT_16K PPU_PORT;

NES_NAMESPACE_END

#endif
