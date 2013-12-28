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

#ifndef NST_GAMEGENIE_H
#define NST_GAMEGENIE_H

#include "../paradox/PdxMap.h"

NES_NAMESPACE_BEGIN

class CPU;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class GAMEGENIE
{
public:

	GAMEGENIE(CPU&);
	~GAMEGENIE();

	VOID Reset();
	VOID ClearCodes();

	static PDXRESULT Encode (const ULONG,PDXSTRING&);
	static PDXRESULT Decode (const CHAR* const,ULONG&);
	static PDXRESULT Pack   (const UINT,const UINT,const UINT,const BOOL,ULONG&);
	static PDXRESULT Unpack (const ULONG,UINT&,UINT&,UINT&,BOOL&);

	PDXRESULT AddCode    (const ULONG);
	PDXRESULT DeleteCode (const ULONG);

	TSIZE NumCodes() const;
	
	ULONG GetCode(const TSIZE) const;	

private:

	class CODE
	{
	public:

		CODE();

		VOID operator = (const CODE&);

		PDXRESULT Decode(const CHAR* const,ULONG* const=NULL);
		PDXRESULT Encode(const ULONG,PDXSTRING* const=NULL);

		UINT Peek(const UINT);
		VOID Poke(const UINT,const UINT);

		inline VOID SetPort(const CPU_PORT& p)
		{ port = p; }

		inline const CPU_PORT& GetPort() const
		{ return port; }

		inline UINT Address() const 
		{ return address; }

		ULONG Packed() const;
		
	private:

		INT  DecodeCharacters(const CHAR* const,UINT* const) const;
		VOID EncodeCharacters(const UINT* const,PDXSTRING* const) const;

		VOID Decode6(const UINT* const);
		VOID Encode6(UINT* const) const;

		VOID Decode8(const UINT* const);
		VOID Encode8(UINT* const) const;

		VOID DecodeAddress(const UINT* const);
		VOID EncodeAddress(UINT* const) const;

		CPU_PORT port;
		
		UCHAR  data;
		UCHAR  compare;
		USHORT address;
		UCHAR  UseCompare;
	};

	typedef PDXMAP<CODE,UINT> CODES;

	VOID Map(CODE&,const BOOL);

	NES_DECL_PEEK(wizard);
	NES_DECL_POKE(wizard);

	CODES codes;
	CPU& cpu;
};

NES_NAMESPACE_END

#endif
