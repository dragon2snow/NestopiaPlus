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

	GAMEGENIE(CPU* const);
	~GAMEGENIE();

	VOID Destroy();
	VOID Reset();

	PDXRESULT GetContext(IO::GAMEGENIE::CONTEXT&) const;
	PDXRESULT SetContext(const IO::GAMEGENIE::CONTEXT&);

private:

	PDXRESULT SetCode    (const ULONG,const IO::GAMEGENIE::STATE);
	PDXRESULT Pack       (const UINT,const UINT,const UINT,const BOOL,ULONG&) const;
	PDXRESULT Unpack     (const ULONG,UINT&,UINT&,UINT&,BOOL&) const;
	PDXRESULT Encode     (const ULONG,PDXSTRING&) const;
	PDXRESULT Decode     (const CHAR* const,ULONG&) const;
	PDXRESULT EnableCode (const ULONG,const IO::GAMEGENIE::STATE);	
	
	IO::GAMEGENIE::STATE IsCodeEnabled (const ULONG) const;

	class CODE
	{
	public:

		CODE(const BOOL=TRUE);

		VOID operator = (const CODE&);

		PDXRESULT Decode(const CHAR* const,ULONG* const=NULL);
		PDXRESULT Encode(const ULONG,PDXSTRING* const=NULL);

		UINT Peek(const UINT);
		VOID Poke(const UINT,const UINT);

		inline VOID Enable(const BOOL state=TRUE)
		{ enabled = state; }

		inline BOOL IsEnabled() const
		{ return enabled; }

		inline VOID SetPort(const CPU_PORT& p)
		{ port = p; }

		inline const CPU_PORT& GetPort() const
		{ return port; }

		inline UINT Address() const 
		{ return address; }
		
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
		UCHAR  enabled;
	};

	typedef PDXMAP<CODE,UINT> CODES;

	VOID Map(CODE&);

	NES_DECL_PEEK(wizard);
	NES_DECL_POKE(wizard);

	CODES codes;
	CPU* const cpu;
};

NES_NAMESPACE_END

#endif
