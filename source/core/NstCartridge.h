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

#ifndef NST_CARTRIDGE_H
#define NST_CARTRIDGE_H

class PDXFILE;

NES_NAMESPACE_BEGIN

class MAPPER;
class CPU;
class PPU;

////////////////////////////////////////////////////////////////////////////////////////
// Cartridge class
////////////////////////////////////////////////////////////////////////////////////////

class CARTRIDGE
{
public:

	CARTRIDGE();
	~CARTRIDGE();

	PDX_NO_INLINE PDXRESULT Load(PDXFILE&,const PDXSTRING* const,CPU* const,PPU* const,const IO::GENERAL::CONTEXT&);
	PDX_NO_INLINE PDXRESULT Unload();
	PDX_NO_INLINE PDXRESULT Reset(const BOOL);

	inline MAPPER* Mapper()
	{ return mapper; }

	inline const MAPPER* Mapper() const
	{ return mapper; }

	inline const IO::CARTRIDGE::INFO& GetInfo() const
	{ return info; }

private:

	friend class INES;
	friend class UNIF;

	VOID DetectVS();
	VOID DetectMirroring();
	VOID DetectBattery();
	VOID DetectControllers();
	VOID LoadBatteryRam();
	VOID SaveBatteryRam() const;

	MAPPER* mapper;

	PDXARRAY<U8> pRom;
	PDXARRAY<U8> cRom;
	PDXARRAY<U8> wRam;

	IO::CARTRIDGE::INFO info;

	PDXSTRING SaveName;
	IO::GENERAL::CONTEXT context;
};

NES_NAMESPACE_END

#endif
