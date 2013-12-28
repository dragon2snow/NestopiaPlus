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

#include "NstTypes.h"
#include "NstMap.h"
#include "NstCpu.h"
#include "NstGameGenie.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_PACKED_DATA       0x000000FFUL
#define NES_PACKED_COMPARE    0x0000FF00UL
#define NES_PACKED_ADDRESS    0x7FFF0000UL
#define NES_PACKED_USECOMPARE 0x80000000UL

#define NES_PACKED_SHIFT_DATA        0
#define NES_PACKED_SHIFT_COMPARE     8
#define NES_PACKED_SHIFT_ADDRESS    16
#define NES_PACKED_SHIFT_USECOMPARE 31

#define NES_CODE_TO_KEY(packed)  ( 0x8000U + (((packed) & NES_PACKED_ADDRESS) >> NES_PACKED_SHIFT_ADDRESS) )

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

GAMEGENIE::CODE::CODE(const BOOL state)
: 
enabled    (state), 
address    (0x0000),
data       (0x00),
compare    (0x00),
UseCompare (0x00)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::CODE::operator = (const CODE& code)
{
	data       = code.data;
	compare    = code.compare;
	address    = code.address;
	UseCompare = code.UseCompare;
	enabled    = code.enabled;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIE::GetContext(IO::GAMEGENIE::CONTEXT& context) const
{
	switch (context.op)
	{
    	case IO::GAMEGENIE::ENCODE:   return Encode ( context.packed, context.characters );
		case IO::GAMEGENIE::DECODE:   return Decode ( context.characters.String(), context.packed );
		case IO::GAMEGENIE::PACK:     return Pack   ( context.address, context.value, context.compare, context.UseCompare, context.packed );
		case IO::GAMEGENIE::UNPACK:   return Unpack ( context.packed, context.address, context.value, context.compare, context.UseCompare );
		case IO::GAMEGENIE::GETSTATE: context.state = IsCodeEnabled( context.packed ); return PDX_OK;
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIE::SetContext(const IO::GAMEGENIE::CONTEXT& context)
{
	switch (context.op)
	{
    	case IO::GAMEGENIE::ADD:        return SetCode    ( context.packed, context.state );
		case IO::GAMEGENIE::SETSTATE:   return EnableCode ( context.packed, context.state );
    	case IO::GAMEGENIE::DESTROYALL: Destroy(); return PDX_OK;
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIE::CODE::Decode(const CHAR* const characters,ULONG* const packed)
{
	UINT codes[8];

	switch (DecodeCharacters( characters, codes ))
	{
     	case 0: Decode6( codes ); break;
		case 1: Decode8( codes ); break;
		default: return PDX_FAILURE;
	}

	if (packed)
	{
    	*packed = 
		(
			( ULONG( data    ) << NES_PACKED_SHIFT_DATA	   ) |
            ( ULONG( compare ) << NES_PACKED_SHIFT_COMPARE ) |
            ( ULONG( address ) << NES_PACKED_SHIFT_ADDRESS ) |
    		( UseCompare ? NES_PACKED_USECOMPARE : 0x0UL   )
		);
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIE::CODE::Encode(const ULONG packed,PDXSTRING* const characters)
{
	data       = ( packed & NES_PACKED_DATA       ) >> NES_PACKED_SHIFT_DATA;
	compare    = ( packed & NES_PACKED_COMPARE    ) >> NES_PACKED_SHIFT_COMPARE;
	address    = ( packed & NES_PACKED_ADDRESS    ) >> NES_PACKED_SHIFT_ADDRESS;
	UseCompare = ( packed & NES_PACKED_USECOMPARE ) >> NES_PACKED_SHIFT_USECOMPARE;

	if (characters)
	{
       	UINT codes[8];
		PDXMemZero( codes, 8 );

    	switch (UseCompare)
    	{
        	case 0:  Encode6( codes ); break;
        	default: Encode8( codes ); break;
    	}

    	EncodeCharacters( codes, characters );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

INT GAMEGENIE::CODE::DecodeCharacters(const CHAR* const characters,UINT* const codes) const
{
	PDX_ASSERT( characters && codes );

	UINT length = 6;

	for (UINT i=0; i < length; ++i)
	{
		switch (characters[i])
		{
     		case 'A': case 'a': codes[i] = 0x0; break;
     		case 'P': case 'p': codes[i] = 0x1; break;
     		case 'Z': case 'z': codes[i] = 0x2; break;
    		case 'L': case 'l': codes[i] = 0x3; break;
     		case 'G': case 'g': codes[i] = 0x4; break;
     		case 'I': case 'i': codes[i] = 0x5; break;
     		case 'T': case 't': codes[i] = 0x6; break;
     		case 'Y': case 'y': codes[i] = 0x7; break;
    		case 'E': case 'e': codes[i] = 0x8; break;
       		case 'O': case 'o': codes[i] = 0x9; break;
     		case 'X': case 'x': codes[i] = 0xA; break;
    		case 'U': case 'u': codes[i] = 0xB; break;
     		case 'K': case 'k': codes[i] = 0xC; break;
     		case 'S': case 's': codes[i] = 0xD; break;
     		case 'V': case 'v': codes[i] = 0xE; break;
    		case 'N': case 'n': codes[i] = 0xF; break;
			default: return -1;
		}

		if ((i == 2) && (codes[2] & 0x8))
			length = 8;
	}

	return length == 8 ? 1 : (length == 6 ? 0 : -1);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::CODE::EncodeCharacters(const UINT* const codes,PDXSTRING* const characters) const
{
	PDX_ASSERT( codes && characters );

	const UINT length = (UseCompare ? 8 : 6);
	characters->Buffer().Resize( length + 1 );

	for (UINT i=0; i < length; ++i)
	{
		switch (codes[i])
		{
     		case 0x0: (*characters)[i] = 'A'; continue;
     		case 0x1: (*characters)[i] = 'P'; continue;
     		case 0x2: (*characters)[i] = 'Z'; continue;
    		case 0x3: (*characters)[i] = 'L'; continue;
     		case 0x4: (*characters)[i] = 'G'; continue;
     		case 0x5: (*characters)[i] = 'I'; continue;
     		case 0x6: (*characters)[i] = 'T'; continue;
     		case 0x7: (*characters)[i] = 'Y'; continue;
    		case 0x8: (*characters)[i] = 'E'; continue;
       		case 0x9: (*characters)[i] = 'O'; continue;
     		case 0xA: (*characters)[i] = 'X'; continue;
    		case 0xB: (*characters)[i] = 'U'; continue;
     		case 0xC: (*characters)[i] = 'K'; continue;
     		case 0xD: (*characters)[i] = 'S'; continue;
     		case 0xE: (*characters)[i] = 'V'; continue;
    		case 0xF: (*characters)[i] = 'N'; continue;
		}

		PDX_DEBUG_BREAK_MSG("GAMEGENIE::CODE::EncodeCharacters() internal error!");
	}

	(*characters)[length] = '\0';
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::CODE::DecodeAddress(const UINT* const codes)
{
	address = 
	(
		((codes[4] & 0x1) << 0x0) | 
		((codes[4] & 0x2) << 0x0) |
		((codes[4] & 0x4) << 0x0) |
		((codes[3] & 0x8) << 0x0) |
		((codes[2] & 0x1) << 0x4) |
		((codes[2] & 0x2) << 0x4) |
		((codes[2] & 0x4) << 0x4) |
		((codes[1] & 0x8) << 0x4) |
		((codes[5] & 0x1) << 0x8) |
		((codes[5] & 0x2) << 0x8) |
		((codes[5] & 0x4) << 0x8) |
		((codes[4] & 0x8) << 0x8) |
		((codes[3] & 0x1) << 0xC) |
		((codes[3] & 0x2) << 0xC) |
		((codes[3] & 0x4) << 0xC)
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::CODE::EncodeAddress(UINT* const codes) const
{
	codes[4] |= (address & 0x0001) >> 0x0;
	codes[4] |= (address & 0x0002) >> 0x0;
	codes[4] |= (address & 0x0004) >> 0x0;
	codes[3] |= (address & 0x0008) >> 0x0;
	codes[2] |= (address & 0x0010) >> 0x4;
	codes[2] |= (address & 0x0020) >> 0x4;
	codes[2] |= (address & 0x0040) >> 0x4;
	codes[1] |= (address & 0x0080) >> 0x4;
	codes[5] |= (address & 0x0100) >> 0x8;
	codes[5] |= (address & 0x0200) >> 0x8;
	codes[5] |= (address & 0x0400) >> 0x8;
	codes[4] |= (address & 0x0800) >> 0x8;
	codes[3] |= (address & 0x1000) >> 0xC;
	codes[3] |= (address & 0x2000) >> 0xC;
	codes[3] |= (address & 0x4000) >> 0xC;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::CODE::Decode6(const UINT* const codes)
{
	DecodeAddress( codes );

	data =
	(
		((codes[0] & 0x1) << 0x0) |
		((codes[0] & 0x2) << 0x0) |
		((codes[0] & 0x4) << 0x0) |
		((codes[5] & 0x8) << 0x0) |
		((codes[1] & 0x1) << 0x4) |
		((codes[1] & 0x2) << 0x4) |
		((codes[1] & 0x4) << 0x4) |
		((codes[0] & 0x8) << 0x4) 
	);

	compare = 0x00;
	UseCompare = 0x00;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::CODE::Encode6(UINT* const codes) const
{
	EncodeAddress( codes );

	codes[0] |= (data & 0x01) >> 0x0;
	codes[0] |= (data & 0x02) >> 0x0;
	codes[0] |= (data & 0x04) >> 0x0;
	codes[5] |= (data & 0x08) >> 0x0;
	codes[1] |= (data & 0x10) >> 0x4;
	codes[1] |= (data & 0x20) >> 0x4;
	codes[1] |= (data & 0x40) >> 0x4;
	codes[0] |= (data & 0x80) >> 0x4;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::CODE::Decode8(const UINT* const codes)
{
	DecodeAddress( codes );

	data =
	(
		((codes[0] & 0x1) << 0x0) |
		((codes[0] & 0x2) << 0x0) |
		((codes[0] & 0x4) << 0x0) |
		((codes[7] & 0x8) << 0x0) |
		((codes[1] & 0x1) << 0x4) |
		((codes[1] & 0x2) << 0x4) |
		((codes[1] & 0x4) << 0x4) |
		((codes[0] & 0x8) << 0x4) 
	);

	compare =
	(
		((codes[6] & 0x1) << 0x0) |
		((codes[6] & 0x2) << 0x0) |
		((codes[6] & 0x4) << 0x0) |
		((codes[5] & 0x8) << 0x0) |
		((codes[7] & 0x1) << 0x4) |
		((codes[7] & 0x2) << 0x4) |
		((codes[7] & 0x4) << 0x4) |
		((codes[6] & 0x8) << 0x4)  
	);

	UseCompare = 0x01;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::CODE::Encode8(UINT* const codes) const
{
	EncodeAddress( codes );

	codes[2] |= 0x8;

	codes[0] |= (data & 0x01) >> 0x0;
	codes[0] |= (data & 0x02) >> 0x0;
	codes[0] |= (data & 0x04) >> 0x0;
	codes[7] |= (data & 0x08) >> 0x0;
	codes[1] |= (data & 0x10) >> 0x4;
	codes[1] |= (data & 0x20) >> 0x4;
	codes[1] |= (data & 0x40) >> 0x4;
	codes[0] |= (data & 0x80) >> 0x4; 
	
	codes[6] |= (compare & 0x1) >> 0x0;
	codes[6] |= (compare & 0x2) >> 0x0;
	codes[6] |= (compare & 0x4) >> 0x0;
	codes[5] |= (compare & 0x8) >> 0x0;
	codes[7] |= (compare & 0x1) >> 0x4;
	codes[7] |= (compare & 0x2) >> 0x4;
	codes[7] |= (compare & 0x4) >> 0x4;
	codes[6] |= (compare & 0x8) >> 0x4;  
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIE::Pack(const UINT address,const UINT data,const UINT compare,const BOOL UseCompare,ULONG& packed) const
{
	if ((address < 0x8000U) || (address > 0xFFFFU) || (data > 0xFF) || (compare > 0xFF && UseCompare))
		return PDX_FAILURE;

	packed =
	(
    	( ULONG( data              ) << NES_PACKED_SHIFT_DATA    ) | 
		( ULONG( compare           ) << NES_PACKED_SHIFT_COMPARE ) | 
		( ULONG( address - 0x8000U ) << NES_PACKED_SHIFT_ADDRESS ) |
		( UseCompare ? NES_PACKED_USECOMPARE : 0x0UL )
	);

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIE::Unpack(const ULONG packed,UINT& address,UINT& data,UINT& compare,BOOL& UseCompare) const
{
	data       = (( packed & NES_PACKED_DATA       ) >> NES_PACKED_SHIFT_DATA       );
	compare    = (( packed & NES_PACKED_COMPARE    ) >> NES_PACKED_SHIFT_COMPARE    );
	address    = (( packed & NES_PACKED_ADDRESS    ) >> NES_PACKED_SHIFT_ADDRESS    ) + 0x8000;
	UseCompare = (( packed & NES_PACKED_USECOMPARE ) >> NES_PACKED_SHIFT_USECOMPARE );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

IO::GAMEGENIE::STATE GAMEGENIE::IsCodeEnabled(const ULONG packed) const
{ 
	const CODES::CONSTITERATOR iterator( codes.Find( NES_CODE_TO_KEY(packed) ) );	
	return (iterator != codes.End() && (*iterator).Second().IsEnabled()) ? IO::GAMEGENIE::ENABLE : IO::GAMEGENIE::DISABLE; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIE::Decode(const CHAR* const characters,ULONG& packed) const
{
	return CODE().Decode( characters, &packed );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIE::Encode(const ULONG packed,PDXSTRING& characters) const
{
	return CODE().Encode( packed, &characters );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline UINT GAMEGENIE::CODE::Peek(const UINT address)
{
	PDX_ASSERT( enabled && (0x8000U + this->address) == address );

	if (UseCompare)
	{
		PDX_ASSERT(port.object && port.reader);
		const UINT value = (*port.object.*port.reader)(address);

		if (value != compare)
			return value;
	}

	return data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID GAMEGENIE::CODE::Poke(const UINT address,const UINT value)
{
	PDX_ASSERT( enabled && port.object && port.writer && (0x8000U + this->address) == address );
	(*port.object.*port.writer)(address,value);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

GAMEGENIE::GAMEGENIE(CPU& c)
: cpu(c) {}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

GAMEGENIE::~GAMEGENIE()
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::Destroy()
{
	for (CODES::ITERATOR iterator = codes.Begin(); iterator != codes.End(); ++iterator)
	{
		if (this == cpu.GetPort( (*iterator).First() ).Object<GAMEGENIE>())
		{
			const CPU_PORT& port = (*iterator).Second().GetPort();
			cpu.SetPort( (*iterator).First(), port.object, port.reader, port.writer );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::Reset()
{
	for (CODES::ITERATOR iterator = codes.Begin(); iterator != codes.End(); ++iterator)
		Map( (*iterator).Second() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIE::Map(CODE& code)
{
	const UINT address = 0x8000U + code.Address();

	if (code.IsEnabled())
	{
		const CPU_PORT& port = cpu.GetPort(address);

		if (this != port.Object<GAMEGENIE>())
		{
			code.SetPort( port );
			cpu.SetPort( address, this, Peek_wizard, Poke_wizard );
		}
	}
	else
	{
		if (this == cpu.GetPort(address).Object<GAMEGENIE>())
		{
			const CPU_PORT& port = code.GetPort();
			cpu.SetPort( address, port.object, port.reader, port.writer );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIE::SetCode(const ULONG packed,const IO::GAMEGENIE::STATE state)
{
	CODE code(state == IO::GAMEGENIE::DISABLE ? FALSE : TRUE);

	PDX_TRY(code.Encode( packed ));

	PDX_ASSERT(code.Address() < 0x8000U);

	CODE& target = codes[0x8000U + code.Address()];
	const BOOL enabled = target.IsEnabled();

	target = code;

	if (state == IO::GAMEGENIE::NOCHANGE)
		target.Enable( enabled );

	Map( target );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIE::EnableCode(const ULONG packed,const IO::GAMEGENIE::STATE state)
{
	CODE& code = codes[NES_CODE_TO_KEY(packed)];
	code.Enable( state == IO::GAMEGENIE::DISABLE ? FALSE : TRUE );
	Map( code );
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(GAMEGENIE,wizard)
{
	PDX_ASSERT(address >= 0x8000U);
	return (*codes.Get(address)).Second().Peek(address);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(GAMEGENIE,wizard)
{
	PDX_ASSERT(address >= 0x8000U);
	(*codes.Get(address)).Second().Poke(address,data);
}

NES_NAMESPACE_END
