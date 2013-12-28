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

#ifndef NST_VSSYSTEM_H
#define NST_VSSYSTEM_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class PPU;
class CPU;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class VSSYSTEM
{
private:

	struct DIPSWITCH
	{
		DIPSWITCH();		

		DIPSWITCH(const DIPSWITCH&);

		DIPSWITCH
		(
	     	const CHAR* const,
			const UINT,
			const UINT,
			const CHAR* const, const UINT,
			const CHAR* const, const UINT,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL,
			const CHAR* const=NULL, const UINT=NULL
		);

		DIPSWITCH& operator = (const DIPSWITCH&);

		typedef PDXARRAY< PDXPAIR<PDXSTRING,UINT> > SETTINGS;

		UINT index;
		UINT mask;
		PDXSTRING name;
		SETTINGS settings;
	};

	typedef PDXARRAY<DIPSWITCH> DIPSWITCHES;

public:

	class CONTEXT
	{
	public:

		CONTEXT()
		: 
		cpu               (NULL),
		ppu               (NULL),
		ColorMapIndex     (UINT_MAX),
		SwapPads          (FALSE),
		CopyProtectionPPU (FALSE),
		SecurityPPU       (0)
		{
			for (UINT i=0; i < 8; ++i)
				ButtonMap[i] = i;
		}

		CPU* cpu;
		PPU* ppu;
		UINT ColorMapIndex;
		BOOL SwapPads;
		BOOL CopyProtectionPPU;
		UINT SecurityPPU;
		UINT ButtonMap[8];

	private:

		friend class VSSYSTEM;

		DIPSWITCHES DipSwitches;
	};

	VSSYSTEM(CONTEXT&);

	PDX_NO_INLINE VOID Reset(const BOOL);
	PDX_NO_INLINE VOID SetContext(IO::INPUT* const);

	static VSSYSTEM* New(CPU&,PPU&,const ULONG);

	inline UINT NumDipSwitches() const
	{ return DipSwitches.Size(); }

	VOID GetDipSwitch(const UINT,IO::DIPSWITCH::CONTEXT&) const;
	VOID SetDipSwitch(const UINT,const IO::DIPSWITCH::CONTEXT&);

protected:

	enum
	{
		DIPSWITCH_4016_MASK	 = b00000011,
		DIPSWITCH_4016_SHIFT = 3,
		DIPSWITCH_4017_MASK  = b11111100,
		DIPSWITCH_4017_SHIFT = 0,
		COIN_1               = IO::INPUT::VS::COIN_1,
		COIN_2               = IO::INPUT::VS::COIN_2,
		COIN                 = COIN_1|COIN_2,
		STATUS_4016_MASK     = (DIPSWITCH_4016_MASK << DIPSWITCH_4016_SHIFT) | COIN,
		STATUS_4017_MASK     = (DIPSWITCH_4017_MASK << DIPSWITCH_4017_SHIFT)
	};

	VOID Poke(const UINT,const UINT) {}

	NES_DECL_PEEK( 2002 );
	NES_DECL_POKE( 2002 );
	NES_DECL_PEEK( 2006 );
	NES_DECL_POKE( 2006 );
	NES_DECL_PEEK( 2007 );
	NES_DECL_POKE( 2007 );
	NES_DECL_PEEK( 4016 );
	NES_DECL_POKE( 4016 );
	NES_DECL_PEEK( 4017 );
	NES_DECL_POKE( 4017 );
	NES_DECL_PEEK( 4020 );
	NES_DECL_POKE( 4020 );
	
	CPU& cpu;
	PPU& ppu;

	const U8* const ColorMap;

	CPU_PORT p2007;
	CPU_PORT p4016;
	CPU_PORT p4017;

	UINT flags[2];
	UINT coin;

private:

	VOID UpdateDipSwitches(const UINT,const UINT);

	virtual VOID Reset() {}

	static VOID DipSkip(VSSYSTEM::CONTEXT&);

	DIPSWITCHES DipSwitches;

	enum
	{
		A      = IO::INPUT::PAD::INDEX_A,
		B      = IO::INPUT::PAD::INDEX_B,
		SELECT = IO::INPUT::PAD::INDEX_SELECT,
		START  = IO::INPUT::PAD::INDEX_START, 
		UP     = IO::INPUT::PAD::INDEX_UP,
		DOWN   = IO::INPUT::PAD::INDEX_DOWN,
		LEFT   = IO::INPUT::PAD::INDEX_LEFT,
		RIGHT  = IO::INPUT::PAD::INDEX_RIGHT
	};

	BOOL RemapButtons;
	UINT FirstPad;
	UINT ButtonMap[8];

	BOOL CopyProtectionPPU;
	UINT SecurityPPU;
	CPU_PORT p2002;

	static const U8 ColorMaps[4][64];
};

NES_NAMESPACE_END

#endif
