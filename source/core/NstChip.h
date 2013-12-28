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

#ifndef NST_CHIP_H
#define NST_CHIP_H

#include "../paradox/PdxFile.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<UINT A,UINT B> class CHIP
{
public:

	enum
	{				   
		ADDRESS_SPACE = A,
		NUM_PAGES     = B,
		BANK_SIZE     = ADDRESS_SPACE / NUM_PAGES
	};

	CHIP(const TSIZE=0);
	CHIP(U8* const,const TSIZE);

	~CHIP() 
	{
		if (internal)
			delete [] ram;
	}

	VOID Destroy();

	inline U8& operator[](const UINT address)
	{ 
		return pages[address / BANK_SIZE][address % BANK_SIZE]; 
	}

	inline const U8& operator[](const UINT address) const
	{ 
		return pages[address / BANK_SIZE][address % BANK_SIZE]; 
	}

	inline UINT Peek(const UINT address)
	{ 
		return pages[address / BANK_SIZE][address % BANK_SIZE]; 
	}

	inline VOID Poke(const UINT address,const UINT data)
	{ 
		pages[address / BANK_SIZE][address % BANK_SIZE] = data; 
	}

	inline U8* Ram()
	{ return ram; }

	inline const U8* Ram() const
	{ return ram; }

	inline U8* Ram(const UINT offset)
	{ return ram + (offset & mask); }

	inline const U8* Ram(const UINT offset) const
	{ return ram + (offset & mask); }

	VOID Fill(const UINT start,const UINT end,const U8 value)
	{
		PDX_ASSERT((start <= end && end <= size) && ((ram && size) || (!ram && !size)));
		PDX::Fill( ram + start, ram + end, value);
	}

	VOID Fill(const U8 value)
	{
		PDX_ASSERT((ram && size) || (!ram && !size));
		PDX::Fill( ram, ram + size, value );
	}

	VOID Clear()
	{
		PDX_ASSERT((ram && size) || (!ram && !size));
		PDXMemZero( ram, size );
	}

	VOID ReAssign(U8* const r,const TSIZE s)
	{
		PDX_ASSERT((r && s) || (!r && !s));

		if (internal)
		{
			internal = FALSE;
			delete [] ram;
		}

		ram = b;
		size = s;
		SetMask();
	}

	VOID ReAssign(const TSIZE s)
	{
		if (internal)
			delete [] ram;

		internal = TRUE;
		ram = new U8[size=s];
		SetMask();
	}

	inline TSIZE Size() const
	{
		return size;
	}

	inline U8* GetBank(const UINT index)
	{
		return pages[index];
	}

	inline const U8* GetBank(const UINT index) const
	{
		return pages[index];
	}

	inline U8& operator () (const UINT index,const UINT offset)
	{
		return pages[index][offset];
	}

	inline const U8& operator () (const UINT index,const UINT offset) const
	{
		return pages[index][offset];
	}

	inline UINT Mask() const
	{ return mask; }

	template<UINT SIZE> 
	inline UINT NumBanks() const
	{
		return size / SIZE;
	}

	template<ULONG SWAP_SIZE,ULONG SWAP_ADDRESS> 
	inline VOID SwapBanks(const UINT bank)
	{
		U8* const offset = ram + ((bank * SWAP_SIZE) & mask);
		SetBanks<SWAP_SIZE,SWAP_ADDRESS>(offset);
	}

	template<ULONG SWAP_SIZE> 
	inline VOID SwapBanks(const UINT address,const UINT bank)
	{
		U8* const offset = ram + ((bank * SWAP_SIZE) & mask);
		SetBanks<SWAP_SIZE>(address,offset);
	}

	template<ULONG SWAP_SIZE> 
	inline VOID SetBanks(const UINT address,U8* const chunk)
	{
		PDX_ASSERT(ADDRESS_SPACE >= (address + SWAP_SIZE));
		PDX_COMPILE_ASSERT(SWAP_SIZE >= BANK_SIZE);
		PDX_COMPILE_ASSERT(SWAP_SIZE % BANK_SIZE == 0);

		const UINT offset = address / BANK_SIZE;

		for (UINT i=0; i < (SWAP_SIZE / BANK_SIZE); ++i)
			pages[offset+i] = chunk + (i * BANK_SIZE);
	}

	template<ULONG SWAP_SIZE,ULONG SWAP_ADDRESS> 
	inline VOID SetBanks(U8* const chunk)
	{
		PDX_COMPILE_ASSERT(ADDRESS_SPACE >= (SWAP_ADDRESS + SWAP_SIZE));
		PDX_COMPILE_ASSERT(SWAP_SIZE >= BANK_SIZE);
		PDX_COMPILE_ASSERT(SWAP_SIZE % BANK_SIZE == 0);

		enum
		{
			OFFSET = SWAP_ADDRESS / BANK_SIZE,
			LENGTH = SWAP_SIZE / BANK_SIZE
		};

		for (UINT i=0; i < LENGTH; ++i)
			pages[OFFSET+i] = chunk + (i * BANK_SIZE);
	}

	PDXRESULT SaveState(PDXFILE&,const BOOL=TRUE) const;
	PDXRESULT LoadState(PDXFILE&,const BOOL=TRUE);

private:

	VOID SetMask();

	PDX_COMPILE_ASSERT(NUM_PAGES <= 32);

	U8* pages[NUM_PAGES];
	U8* ram;
	UINT size;
	UINT mask;
	BOOL internal;
};

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<UINT A,UINT B>
CHIP<A,B>::CHIP(U8* const chunk,const TSIZE RamSize)
: 
ram      (RamSize ? chunk : NULL), 
size     (RamSize), 
internal (FALSE)
{
	for (UINT i=0; i < NUM_PAGES; ++i)
		pages[i] = ram ? (ram + ((i * BANK_SIZE) % size)) : NULL;

	SetMask();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<UINT A,UINT B>
CHIP<A,B>::CHIP(const TSIZE RamSize)
: 
ram      (RamSize ? new U8[RamSize] : NULL), 
size     (RamSize), 
internal (TRUE)
{
	PDXMemZero( ram, RamSize );

	for (UINT i=0; i < NUM_PAGES; ++i)
		pages[i] = ram ? (ram + ((i * BANK_SIZE) % size)) : NULL;

	SetMask();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<UINT A,UINT B>
VOID CHIP<A,B>::Destroy()
{
	for (UINT i=0; i < NUM_PAGES; ++i)
		pages[i] = NULL;

	if (internal)
	{
		internal = FALSE;
		delete [] ram;
	}

	ram = NULL;
	size = 0;
	mask = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<UINT A,UINT B>
PDXRESULT CHIP<A,B>::SaveState(PDXFILE& file,const BOOL SaveRam) const
{
	if (!ram)
		return PDX_OK;

	if (SaveRam)
	{
		U8 present = 0;

		for (TSIZE i=0; i < size; ++i)
		{
			if (ram[i])
			{
				present = 1;
				break;
			}
		}

		file << present;

		if (present)
			file.Write( ram, ram + size );
	}

	U32 flags = 0;

	for (UINT i=0; i < NUM_PAGES; ++i)
	{
		if (pages[i] >= ram && pages[i] <= ram + size) 
			flags |= (1UL << i);
	}

	file << flags;

	for (UINT i=0; i < NUM_PAGES; ++i)
	{
		if (flags & (1UL << i))
			file << U32( pages[i] - ram );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<UINT A,UINT B>
PDXRESULT CHIP<A,B>::LoadState(PDXFILE& file,const BOOL LoadRam)
{
	if (!ram)
		return PDX_OK;

	if (LoadRam)
	{
		U8 present;

		if (!file.Read(present))
			return PDX_FAILURE;

		if (present)
		{
			if (!file.Read( ram, ram + size ))
				return PDX_FAILURE;
		}
		else
		{
			PDXMemZero( ram, size );
		}
	}

	U32 flags;
	
	if (!file.Read(flags))
		return PDX_FAILURE;

	if (flags)
	{
		for (UINT i=0; i < NUM_PAGES; ++i)
		{
			if (flags & (1UL << i)) 
			{
				U32 offset;

				if (!file.Read(offset))
					return PDX_FAILURE;

				if (offset >= size)
					return PDX_FAILURE;

				pages[i] = ram + offset;
			}
		}
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<UINT A,UINT B>
VOID CHIP<A,B>::SetMask()
{
	for (mask=1; mask < size; mask += mask);
	--mask;
}

NES_NAMESPACE_END

#endif
