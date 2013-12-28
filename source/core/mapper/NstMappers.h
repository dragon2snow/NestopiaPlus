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

#ifndef NST_MAPPERS_H
#define NST_MAPPERS_H

#include "../../paradox/PdxFile.h"
#include "../NstTypes.h"
#include "../NstMap.h"
#include "../NstCpu.h"
#include "../NstApu.h"
#include "../NstPpu.h"
#include "../NstChip.h"

////////////////////////////////////////////////////////////////////////////////////////
// base mapper
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class MAPPER
{
public:

	class CONTEXT;

	virtual ~MAPPER();

	static MAPPER* New(CONTEXT&);

	virtual VOID Reset(const BOOL);
	
	inline ULONG GetPRomCrc() const { return pRomCrc; }
	inline ULONG GetCRomCrc() const { return cRomCrc; }

	VOID VSync();

	inline CPU& GetCPU() { return cpu; }
	inline PPU& GetPPU() { return ppu; }
	inline APU& GetAPU() { return apu; }

	inline const CPU& GetCPU() const { return cpu; }
	inline const PPU& GetPPU() const { return ppu; }
	inline const APU& GetAPU() const { return apu; }

	virtual PDXRESULT LoadState(PDXFILE& file);
	virtual PDXRESULT SaveState(PDXFILE& file) const;

	virtual UINT NumDipSwitches() const { return 0; }
	virtual VOID GetDipSwitch(const UINT,IO::DIPSWITCH::CONTEXT&) const {}
	virtual VOID SetDipSwitch(const UINT,const IO::DIPSWITCH::CONTEXT&) {}

	static const CHAR* boards[256];

protected:

	MAPPER
	(
       	CONTEXT&,
		VOID* const=NULL,
		const VOID* const=NULL,
		VOID* const=NULL,
		const VOID* const=NULL
	);

	enum IRQSYNCTYPE
	{
		IRQSYNC_NONE,
		IRQSYNC_PPU,
		IRQSYNC_PPU_ALWAYS,
		IRQSYNC_COUNT,
		IRQSYNC_COMBINED
	};

	VOID EnableIrqSync(const IRQSYNCTYPE);
	VOID EnableCartridgeCRam(const BOOL=TRUE,const UINT=n8k);

	virtual VOID EndFrame() {}

	NES_DECL_PEEK( Nop );
	NES_DECL_POKE( Nop ) {}

	NES_DECL_PEEK( 6000 );
	NES_DECL_POKE( 6000 );

	NES_DECL_PEEK( 8000 );
	NES_DECL_PEEK( 9000 );
	NES_DECL_PEEK( A000 );
	NES_DECL_PEEK( B000 );
	NES_DECL_PEEK( C000 );
	NES_DECL_PEEK( D000 );
	NES_DECL_PEEK( E000 );
	NES_DECL_PEEK( F000 );

	NES_DECL_PEEK( cRom_0000 );
	NES_DECL_PEEK( cRom_0400 );
	NES_DECL_PEEK( cRom_0800 );
	NES_DECL_PEEK( cRom_0C00 );
	NES_DECL_PEEK( cRom_1000 );
	NES_DECL_PEEK( cRom_1400 );
	NES_DECL_PEEK( cRom_1800 );
	NES_DECL_PEEK( cRom_1C00 );

	NES_DECL_PEEK( cRam );
	NES_DECL_POKE( cRom );
	NES_DECL_POKE( cRam );
	NES_DECL_PEEK( pRom );

	BOOL IsIrqEnabled() const;
	BOOL IsA13High()    const;
	BOOL IsA13Low()     const;

	BOOL SetIrqEnable (const BOOL);
	VOID TriggerA13   (const UINT);

	typedef CHIP<n32k,4> PROM;
	typedef CHIP<n8k,8>  CROM;
	typedef CHIP<n8k,8>  CRAM;
	typedef CHIP<n8k,8>  VRAM;
	typedef CHIP<n4k,4>  CIRAM;

private:

	const TSIZE StateSize1;
	VOID* const StateChunk1;
	const TSIZE StateSize2;
	VOID* const StateChunk2;

	const UINT wRamInitSize;

public:

	UINT id;
	const UINT pRomCrc;
	const UINT cRomCrc;

protected:

	BOOL IsCRam;
	BOOL HardReset;

	PROM pRom;
	CROM cRom;

	PDXARRAY<U8> wRam;	

	CPU& cpu;
	APU& apu;
	PPU& ppu;

	LONG IrqCount;
	UINT IrqLatch;
	UINT IrqTmp;

	IRQSYNCTYPE IrqSyncType;

private:

	BOOL IrqEnabled;
	LONG IrqCycles;
	UINT IrqA13;

public:

	MIRRORING mirroring;

private:

	VOID ResetLog();

	VOID TriggerA13Lo0();
	VOID TriggerA13Hi0();

	VOID TriggerA13Lo1();
	VOID TriggerA13Hi1();

	NES_DECL_PEEK( cRom_0_A13_0000 );
	NES_DECL_PEEK( cRom_0_A13_0400 );
	NES_DECL_PEEK( cRom_0_A13_0800 );
	NES_DECL_PEEK( cRom_0_A13_0C00 );
	NES_DECL_PEEK( cRom_0_A13_1000 );
	NES_DECL_PEEK( cRom_0_A13_1400 );
	NES_DECL_PEEK( cRom_0_A13_1800 );
	NES_DECL_PEEK( cRom_0_A13_1C00 );
	NES_DECL_PEEK( cRom_1_A13_0000 );
	NES_DECL_PEEK( cRom_1_A13_0400 );
	NES_DECL_PEEK( cRom_1_A13_0800 );
	NES_DECL_PEEK( cRom_1_A13_0C00 );
	NES_DECL_PEEK( cRom_1_A13_1000 );
	NES_DECL_PEEK( cRom_1_A13_1400 );
	NES_DECL_PEEK( cRom_1_A13_1800 );
	NES_DECL_PEEK( cRom_1_A13_1C00 );

	NES_DECL_PEEK( cRam_0_A13_Lo );
	NES_DECL_PEEK( cRam_0_A13_Hi );
	NES_DECL_PEEK( cRam_1_A13_Lo );
	NES_DECL_PEEK( cRam_1_A13_Hi );

	virtual VOID IrqSync () {}
	virtual VOID IrqSync (const UINT) {}

	VOID IrqSyncCheck();

	virtual VOID Reset() {}

	PDXRESULT LoadWRam(PDXFILE&);
	PDXRESULT SaveWRam(PDXFILE&) const;

	PDXRESULT LoadIrqData(PDXFILE&);
	PDXRESULT SaveIrqData(PDXFILE&) const;

	PDXRESULT LoadStateChunks(PDXFILE&);
	PDXRESULT SaveStateChunks(PDXFILE&) const;
};

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID MAPPER::TriggerA13(const UINT address)
{
	if (IrqEnabled)
	{
		const UINT level = address & 0x1000;

		if (level && !IrqA13)
			IrqSync();

		IrqA13 = level;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL MAPPER::SetIrqEnable(const BOOL state)
{
	if (!state)
	{
		cpu.ClearIRQ();
		IrqEnabled = FALSE;
		return FALSE;
	}
	else
	{
		if (!IrqEnabled)
			IrqCycles = cpu.GetCycles<CPU::CYCLE_MASTER>();

		IrqEnabled = TRUE;
		return TRUE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL MAPPER::IsIrqEnabled() const
{
	return IrqEnabled;
}

////////////////////////////////////////////////////////////////////////////////////////
// high is usually HBlank and low in HActive
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL MAPPER::IsA13High() const { return IrqA13 ? TRUE : FALSE; }
inline BOOL MAPPER::IsA13Low()  const { return IrqA13 ? FALSE : TRUE; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

struct MAPPER::CONTEXT
{
	UINT id;

	CPU* cpu;
	PPU* ppu;

	U8* pRom;
	U8* cRom;
	U8* wRam;

	UINT pRomSize;
	UINT cRomSize;
	UINT wRamSize;

	BOOL IsCRam;

	UINT wRamInitSize;

	MIRRORING mirroring;
};

NES_NAMESPACE_END

#endif
