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

#include "NstMappers.h"
#include "NstMapper000.h"
#include "NstMapper001.h"
#include "NstMapper002.h"
#include "NstMapper003.h"
#include "NstMapper004.h"
#include "NstMapper005.h"
#include "NstMapper006.h"
#include "NstMapper007.h"
#include "NstMapper008.h"
#include "NstMapper009.h"
#include "NstMapper010.h"
#include "NstMapper011.h"
#include "NstMapper013.h"
#include "NstMapper015.h"
#include "NstMapper016.h"
#include "NstMapper017.h"
#include "NstMapper018.h"
#include "NstMapper019.h"
#include "NstMapper021.h"
#include "NstMapper022.h"
#include "NstMapper023.h"
#include "NstMapper024.h"
#include "NstMapper025.h"
#include "NstMapper026.h"
#include "NstMapper032.h"
#include "NstMapper033.h"
#include "NstMapper034.h"
#include "NstMapper040.h"
#include "NstMapper041.h"
#include "NstMapper042.h"
#include "NstMapper043.h"
#include "NstMapper044.h"
#include "NstMapper045.h"
#include "NstMapper046.h"
#include "NstMapper047.h"
#include "NstMapper048.h"
#include "NstMapper049.h"
#include "NstMapper050.h"
#include "NstMapper051.h"
#include "NstMapper052.h"
#include "NstMapper057.h"
#include "NstMapper058.h"
#include "NstMapper060.h"
#include "NstMapper061.h"
#include "NstMapper062.h"
#include "NstMapper064.h"
#include "NstMapper065.h"
#include "NstMapper066.h"
#include "NstMapper067.h"
#include "NstMapper068.h"
#include "NstMapper069.h"
#include "NstMapper070.h"
#include "NstMapper071.h"
#include "NstMapper072.h"
#include "NstMapper073.h"
#include "NstMapper074.h"
#include "NstMapper075.h"
#include "NstMapper076.h"
#include "NstMapper077.h"
#include "NstMapper078.h"
#include "NstMapper079.h"
#include "NstMapper080.h"
#include "NstMapper082.h"
#include "NstMapper083.h"
#include "NstMapper085.h"
#include "NstMapper086.h"
#include "NstMapper087.h"
#include "NstMapper088.h"
#include "NstMapper089.h"
#include "NstMapper090.h"
#include "NstMapper091.h"
#include "NstMapper092.h"
#include "NstMapper093.h"
#include "NstMapper094.h"
#include "NstMapper095.h"
#include "NstMapper096.h"
#include "NstMapper097.h"
#include "NstMapper099.h"
#include "NstMapper100.h"
#include "NstMapper101.h"
#include "NstMapper105.h"
#include "NstMapper107.h"
#include "NstMapper112.h"
#include "NstMapper113.h"
#include "NstMapper114.h"
#include "NstMapper115.h"
#include "NstMapper117.h"
#include "NstMapper118.h"
#include "NstMapper119.h"
#include "NstMapper122.h"
#include "NstMapper133.h"
#include "NstMapper134.h"
#include "NstMapper135.h"
#include "NstMapper140.h"
#include "NstMapper144.h"
#include "NstMapper151.h"
#include "NstMapper152.h"
#include "NstMapper153.h"
#include "NstMapper154.h"
#include "NstMapper155.h"
#include "NstMapper156.h"
#include "NstMapper157.h"
#include "NstMapper160.h"
#include "NstMapper180.h"
#include "NstMapper181.h"
#include "NstMapper182.h"
#include "NstMapper183.h"
#include "NstMapper184.h"
#include "NstMapper185.h"
#include "NstMapper187.h"
#include "NstMapper188.h"
#include "NstMapper189.h"
#include "NstMapper198.h"
#include "NstMapper222.h"
#include "NstMapper225.h"
#include "NstMapper226.h"
#include "NstMapper227.h"
#include "NstMapper228.h"
#include "NstMapper229.h"
#include "NstMapper230.h"
#include "NstMapper231.h"
#include "NstMapper232.h"
#include "NstMapper233.h"
#include "NstMapper234.h"
#include "NstMapper235.h"
#include "NstMapper236.h"
#include "NstMapper237.h"
#include "NstMapper240.h"
#include "NstMapper241.h"
#include "NstMapper242.h"
#include "NstMapper243.h"
#include "NstMapper244.h"
#include "NstMapper245.h"
#include "NstMapper246.h"
#include "NstMapper248.h"
#include "NstMapper249.h"
#include "NstMapper250.h"
#include "NstMapper255.h"
#include "../../paradox/PdxCrc32.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_CASE_(x) case x: mapper = new MAPPER##x (context); break

MAPPER* MAPPER::New(CONTEXT& context)
{
	MAPPER* mapper = NULL;

	switch (context.id)
	{
		NES_CASE_(   0 );
		NES_CASE_(   1 );
		NES_CASE_(   2 );
		NES_CASE_(   3 );
		NES_CASE_(   4 );
		NES_CASE_(   5 );
		NES_CASE_(   6 );
		NES_CASE_(   7 );
		NES_CASE_(   8 );
		NES_CASE_(   9 );
		NES_CASE_(  10 );
		NES_CASE_(  11 );
		NES_CASE_(  13 );
		NES_CASE_(  15 );
		NES_CASE_(  16 );
		NES_CASE_(  17 );
		NES_CASE_(  18 );
		NES_CASE_(  19 );
		NES_CASE_(  21 );
		NES_CASE_(  22 );
		NES_CASE_(  23 );
		NES_CASE_(  24 );
		NES_CASE_(  25 );
		NES_CASE_(  26 );
		NES_CASE_(  32 );
		NES_CASE_(  33 );
		NES_CASE_(  34 );
		NES_CASE_(  40 );
		NES_CASE_(  41 );
		NES_CASE_(  42 );
		NES_CASE_(  43 );
		NES_CASE_(  44 );
		NES_CASE_(  45 );
		NES_CASE_(  46 );
		NES_CASE_(  47 );
		NES_CASE_(  48 );
		NES_CASE_(  49 );
		NES_CASE_(  50 );
		NES_CASE_(  51 );
		NES_CASE_(  52 );
		NES_CASE_(  57 );
		NES_CASE_(  58 );
		NES_CASE_(  60 );
		NES_CASE_(  61 );
		NES_CASE_(  62 );
		NES_CASE_(  64 );
		NES_CASE_(  65 );
		NES_CASE_(  66 );
		NES_CASE_(  67 );
		NES_CASE_(  68 );
		NES_CASE_(  69 );
		NES_CASE_(  70 );
		NES_CASE_(  71 );
		NES_CASE_(  72 );
		NES_CASE_(  73 );
		NES_CASE_(  74 );
		NES_CASE_(  75 );
		NES_CASE_(  76 );
		NES_CASE_(  77 );
		NES_CASE_(  78 );
		NES_CASE_(  79 );
		NES_CASE_(  80 );
		NES_CASE_(  82 );
		NES_CASE_(  83 );
		NES_CASE_(  85 );
		NES_CASE_(  86 );
		NES_CASE_(  87 );
		NES_CASE_(  88 );
		NES_CASE_(  89 );
		NES_CASE_(  90 );
		NES_CASE_(  91 );
		NES_CASE_(  92 );
		NES_CASE_(  93 );
		NES_CASE_(  94 );
		NES_CASE_(  95 );
		NES_CASE_(  96 );
		NES_CASE_(  97 );
		NES_CASE_(  99 );
		NES_CASE_( 100 );
		NES_CASE_( 101 );
		NES_CASE_( 105 );
		NES_CASE_( 107 );
		NES_CASE_( 112 );
		NES_CASE_( 113 );
		NES_CASE_( 114 );
		NES_CASE_( 115 );
		NES_CASE_( 117 );
		NES_CASE_( 118 );
		NES_CASE_( 119 );
		NES_CASE_( 122 );
		NES_CASE_( 133 );
		NES_CASE_( 134 );
		NES_CASE_( 135 );
		NES_CASE_( 140 );
		NES_CASE_( 144 );
		NES_CASE_( 151 );
		NES_CASE_( 152 );
		NES_CASE_( 153 );
		NES_CASE_( 154 );
		NES_CASE_( 155 );
		NES_CASE_( 156 );
		NES_CASE_( 157 );
		NES_CASE_( 160 );
		NES_CASE_( 180 );
		NES_CASE_( 181 );
		NES_CASE_( 182 );
		NES_CASE_( 183 );
		NES_CASE_( 184 );
		NES_CASE_( 185 );
		NES_CASE_( 187 );
		NES_CASE_( 188 );
		NES_CASE_( 189 );
		NES_CASE_( 198 );
		NES_CASE_( 222 );
		NES_CASE_( 225 );
		NES_CASE_( 226 );
		NES_CASE_( 227 );
		NES_CASE_( 228 );
		NES_CASE_( 229 );
		NES_CASE_( 230 );
		NES_CASE_( 231 );
		NES_CASE_( 232 );
		NES_CASE_( 233 );
		NES_CASE_( 234 );
		NES_CASE_( 235 );
		NES_CASE_( 236 );
		NES_CASE_( 237 );
		NES_CASE_( 240 );
		NES_CASE_( 241 );
		NES_CASE_( 242 );
		NES_CASE_( 243 );
		NES_CASE_( 244 );
		NES_CASE_( 245 );
		NES_CASE_( 246 );
		NES_CASE_( 248 );
		NES_CASE_( 249 );
		NES_CASE_( 250 );
		NES_CASE_( 255 );
	}

	return mapper;
}

#undef NES_CASE_

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER::MAPPER(CONTEXT& context,VOID* const begin1,const VOID* const end1,VOID* const begin2,const VOID* const end2)
:
pRom         (context.pRom,context.pRomSize),
cRom         (context.cRom,context.cRomSize),
cpu          (*context.cpu),
apu          (context.cpu->GetAPU()),
ppu          (*context.ppu),
mirroring    (context.mirroring),
id           (context.id),
wRamInitSize (context.wRamInitSize),
IsCRam       (context.IsCRam),
HardReset    (TRUE),
pRomCrc	     (PDXCRC32::Compute(context.pRom,context.pRomSize)),
cRomCrc	     (PDXCRC32::Compute(context.cRom,context.cRomSize)),
StateChunk1  (begin1),
StateSize1   (PDX_CAST(const CHAR*,end1) - PDX_CAST(const CHAR*,begin1)),
StateChunk2  (begin2),
StateSize2   (PDX_CAST(const CHAR*,end2) - PDX_CAST(const CHAR*,begin2))
{
	wRam.Hook( context.wRam, context.wRam + context.wRamSize );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER::~MAPPER()
{
	cpu.RemoveEvent(this);
	cpu.RemoveEvent(PPU::Update);
	wRam.UnHook();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER::Reset(const BOOL hard)
{
	if (IsCRam)
	{
		IsCRam = FALSE;
		cRom.Destroy();
	}

	IrqSyncType = IRQSYNC_NONE;
	HardReset = hard;

	pRom.SwapBanks<n16k,0x0000>(0);
	pRom.SwapBanks<n16k,0x4000>(pRom.NumBanks<n16k>() - 1);

	cpu.SetPort( 0x4020, 0x5FFF, this, Peek_Nop,  Poke_Nop  );
	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_6000, Poke_6000 );
	cpu.SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_Nop  );
	cpu.SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_Nop  );
	cpu.SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_Nop  );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_Nop  );

	if (cRom.Size())
	{
		cRom.SwapBanks<n8k,0x0000>(0);

		ppu.SetPort( 0x0000, 0x03FF, this, Peek_cRom_0000, Poke_cRom );
		ppu.SetPort( 0x0400, 0x07FF, this, Peek_cRom_0400, Poke_cRom );
		ppu.SetPort( 0x0800, 0x0BFF, this, Peek_cRom_0800, Poke_cRom );
		ppu.SetPort( 0x0C00, 0x0FFF, this, Peek_cRom_0C00, Poke_cRom );
		ppu.SetPort( 0x1000, 0x13FF, this, Peek_cRom_1000, Poke_cRom );
		ppu.SetPort( 0x1400, 0x17FF, this, Peek_cRom_1400, Poke_cRom );
		ppu.SetPort( 0x1800, 0x1BFF, this, Peek_cRom_1800, Poke_cRom );
		ppu.SetPort( 0x1C00, 0x1FFF, this, Peek_cRom_1C00, Poke_cRom );
	}

	ppu.SetMirroring( mirroring );

	cpu.ClearIRQ();

	IrqEnabled = 0;
	IrqCount   = 0;
	IrqLatch   = 0;
	IrqTmp     = 0;
	IrqCycles  = 0;	
	IrqA13     = 0x0000;

	Reset();
	ResetLog();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER::ResetLog()
{
	PDXSTRING log;

	log << "MAPPER " << id <<  ": ";
	const TSIZE length = log.Length();

	log << "reset";
	LogOutput( log );

	if (strlen( boards[id] ))
	{
		log.Resize( length );
		LogOutput( log << "board name: " << boards[id] );
	}

	log.Resize( length );
	LogOutput( log << (pRom.Size() / 1024) << "k PRG-ROM present" );

	log.Resize( length );

	if (cRom.Size())
	{
		log << (cRom.Size() / 1024) << (IsCRam ? "k CHR-RAM present" : "k CHR-ROM present");
	}
	else
	{
		log << "no CHR-ROM present";
	}

	LogOutput( log );

	log.Resize( length );
	log << (wRam.Size() / 1024) << "k WRAM present";

	if (wRam.Size() != wRamInitSize)
		log << " for compatibility";

	LogOutput( log );

	log.Resize( length );
	log << "defaulting to ";

	switch (mirroring)
	{
        case MIRROR_HORIZONTAL: log << "horizontal";  break;
        case MIRROR_VERTICAL:   log << "vertical";    break;
        case MIRROR_FOURSCREEN: log << "four-screen"; break;
        case MIRROR_ZERO:       log << "zero bank";   break;
        case MIRROR_ONE:        log << "first bank";  break;
        case MIRROR_TWO:        log << "second bank"; break;
        case MIRROR_THREE:      log << "third bank";  break;
     	default:                log << "unknown";     break;
	}

	log << " PPU name-table mirroring";
	LogOutput( log );

	log.Resize( length );

	switch (IrqSyncType)
	{
        case IRQSYNC_PPU:	
        case IRQSYNC_PPU_ALWAYS: log << "PPU synchronized IRQ counter present"; break;
        case IRQSYNC_COUNT:   	 log << "IRQ counter present";                  break;
		case IRQSYNC_COMBINED: 	 log << "CPU and PPU IRQ counter present";      break;
		default:          	     log << "no IRQ counter present";               break;
	}

	LogOutput( log );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER::VSync()
{
	if (IrqEnabled)
		IrqCycles -= cpu.GetFrameCycles<CPU::CYCLE_MASTER>();

	EndFrame();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER::EnableCartridgeCRam(const BOOL state,const UINT size)
{ 
	if ((IsCRam = state) && !cRom.Size())
	{
		cRom.ReAssign( size );
		cRom.SwapBanks<n8k,0x0000>(0);

		if (IrqSyncType == IRQSYNC_PPU || IrqSyncType == IRQSYNC_PPU_ALWAYS || IrqSyncType == IRQSYNC_COMBINED)
		{
			EnableIrqSync( IrqSyncType );
		}
		else
		{
			ppu.SetPort( 0x0000, 0x03FF, this, Peek_cRom_0000, Poke_cRom );
			ppu.SetPort( 0x0400, 0x07FF, this, Peek_cRom_0400, Poke_cRom );
			ppu.SetPort( 0x0800, 0x0BFF, this, Peek_cRom_0800, Poke_cRom );
			ppu.SetPort( 0x0C00, 0x0FFF, this, Peek_cRom_0C00, Poke_cRom );
			ppu.SetPort( 0x1000, 0x13FF, this, Peek_cRom_1000, Poke_cRom );
			ppu.SetPort( 0x1400, 0x17FF, this, Peek_cRom_1400, Poke_cRom );
			ppu.SetPort( 0x1800, 0x1BFF, this, Peek_cRom_1800, Poke_cRom );
			ppu.SetPort( 0x1C00, 0x1FFF, this, Peek_cRom_1C00, Poke_cRom );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER::EnableIrqSync(const IRQSYNCTYPE type)
{
	cpu.RemoveEvent(this);
	cpu.RemoveEvent(PPU::Update);

	switch (IrqSyncType = type)
	{
     	case IRQSYNC_COMBINED:
    	case IRQSYNC_PPU:

			cpu.SetEvent( &ppu, PPU::Update );

			ppu.SetPort( 0x0000, 0x03FF, this, cRom.Size() ? Peek_cRom_0_A13_0000 : Peek_cRam_0_A13_Lo, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x0400, 0x07FF, this, cRom.Size() ? Peek_cRom_0_A13_0400 : Peek_cRam_0_A13_Lo, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x0800, 0x0BFF, this, cRom.Size() ? Peek_cRom_0_A13_0800 : Peek_cRam_0_A13_Lo, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x0C00, 0x0FFF, this, cRom.Size() ? Peek_cRom_0_A13_0C00 : Peek_cRam_0_A13_Lo, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x1000, 0x13FF, this, cRom.Size() ? Peek_cRom_0_A13_1000 : Peek_cRam_0_A13_Hi, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x1400, 0x17FF, this, cRom.Size() ? Peek_cRom_0_A13_1400 : Peek_cRam_0_A13_Hi, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x1800, 0x1BFF, this, cRom.Size() ? Peek_cRom_0_A13_1800 : Peek_cRam_0_A13_Hi, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x1C00, 0x1FFF, this, cRom.Size() ? Peek_cRom_0_A13_1C00 : Peek_cRam_0_A13_Hi, cRom.Size() ? Poke_cRom : Poke_cRam );

			if (IrqSyncType == IRQSYNC_COMBINED)
				cpu.SetEvent( this, IrqSyncCheck );

			return;

    	case IRQSYNC_PPU_ALWAYS:

			cpu.SetEvent( &ppu, PPU::Update );

			ppu.SetPort( 0x0000, 0x03FF, this, cRom.Size() ? Peek_cRom_1_A13_0000 : Peek_cRam_1_A13_Lo, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x0400, 0x07FF, this, cRom.Size() ? Peek_cRom_1_A13_0400 : Peek_cRam_1_A13_Lo, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x0800, 0x0BFF, this, cRom.Size() ? Peek_cRom_1_A13_0800 : Peek_cRam_1_A13_Lo, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x0C00, 0x0FFF, this, cRom.Size() ? Peek_cRom_1_A13_0C00 : Peek_cRam_1_A13_Lo, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x1000, 0x13FF, this, cRom.Size() ? Peek_cRom_1_A13_1000 : Peek_cRam_1_A13_Hi, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x1400, 0x17FF, this, cRom.Size() ? Peek_cRom_1_A13_1400 : Peek_cRam_1_A13_Hi, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x1800, 0x1BFF, this, cRom.Size() ? Peek_cRom_1_A13_1800 : Peek_cRam_1_A13_Hi, cRom.Size() ? Poke_cRom : Poke_cRam );
			ppu.SetPort( 0x1C00, 0x1FFF, this, cRom.Size() ? Peek_cRom_1_A13_1C00 : Peek_cRam_1_A13_Hi, cRom.Size() ? Poke_cRom : Poke_cRam );
			return;

		case IRQSYNC_COUNT:

			cpu.SetEvent( this, IrqSyncCheck );
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER::IrqSyncCheck()
{
	if (IrqEnabled)
	{
		const ULONG CpuCycles = cpu.GetCycles<CPU::CYCLE_MASTER>();
		IrqSync( (CpuCycles - IrqCycles) / (cpu.IsPAL() ? NES_CPU_PAL_FIXED : NES_CPU_NTSC_FIXED) );
		IrqCycles = CpuCycles;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID MAPPER::TriggerA13Lo0()
{
	IrqA13 = 0x0000;
}

inline VOID MAPPER::TriggerA13Hi0()
{
	if (!IrqA13)
	{
		IrqA13 = 0x1000;

		if (IrqEnabled)
			IrqSync();
	}
}

inline VOID MAPPER::TriggerA13Lo1()
{
	if (IrqA13)
	{
		IrqA13 = 0x0000;
		IrqSync();
	}
}

inline VOID MAPPER::TriggerA13Hi1()
{
	if (!IrqA13)
	{
		IrqA13 = 0x1000;
		IrqSync();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER,8000) { return pRom( 0, address & 0x1FFF ); }
NES_PEEK(MAPPER,9000) { return pRom( 0, address & 0x1FFF ); }
NES_PEEK(MAPPER,A000) { return pRom( 1, address & 0x1FFF ); }
NES_PEEK(MAPPER,B000) { return pRom( 1, address & 0x1FFF ); }
NES_PEEK(MAPPER,C000) { return pRom( 2, address & 0x1FFF ); }
NES_PEEK(MAPPER,D000) { return pRom( 2, address & 0x1FFF ); }
NES_PEEK(MAPPER,E000) { return pRom( 3, address & 0x1FFF ); }
NES_PEEK(MAPPER,F000) { return pRom( 3, address & 0x1FFF ); }
NES_PEEK(MAPPER,pRom) { return pRom[ address - 0x8000 ]; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER,cRom_0000) { return cRom( 0, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_0400) { return cRom( 1, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_0800) { return cRom( 2, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_0C00) { return cRom( 3, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1000) { return cRom( 4, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1400) { return cRom( 5, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1800) { return cRom( 6, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1C00) { return cRom( 7, address & 0x3FF ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER,cRam) 
{ 
	return ppu.Peek_cRam( address & 0x1FFF ); 
}

NES_POKE(MAPPER,cRam) 
{ 
	ppu.Poke_cRam( address & 0x1FFF, data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER,cRom_0_A13_0000) { TriggerA13Lo0(); return cRom( 0, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_0_A13_0400) { TriggerA13Lo0(); return cRom( 1, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_0_A13_0800) { TriggerA13Lo0(); return cRom( 2, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_0_A13_0C00) { TriggerA13Lo0(); return cRom( 3, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_0_A13_1000) { TriggerA13Hi0(); return cRom( 4, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_0_A13_1400) { TriggerA13Hi0(); return cRom( 5, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_0_A13_1800) { TriggerA13Hi0(); return cRom( 6, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_0_A13_1C00) { TriggerA13Hi0(); return cRom( 7, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1_A13_0000) { TriggerA13Lo1(); return cRom( 0, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1_A13_0400) { TriggerA13Lo1(); return cRom( 1, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1_A13_0800) { TriggerA13Lo1(); return cRom( 2, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1_A13_0C00) { TriggerA13Lo1(); return cRom( 3, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1_A13_1000) { TriggerA13Hi1(); return cRom( 4, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1_A13_1400) { TriggerA13Hi1(); return cRom( 5, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1_A13_1800) { TriggerA13Hi1(); return cRom( 6, address & 0x3FF ); }
NES_PEEK(MAPPER,cRom_1_A13_1C00) { TriggerA13Hi1(); return cRom( 7, address & 0x3FF ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER,cRam_0_A13_Lo) { TriggerA13Lo0(); return ppu.Peek_cRam( address & 0x1FFF ); }
NES_PEEK(MAPPER,cRam_0_A13_Hi) { TriggerA13Hi0(); return ppu.Peek_cRam( address & 0x1FFF ); }
NES_PEEK(MAPPER,cRam_1_A13_Lo) { TriggerA13Lo1(); return ppu.Peek_cRam( address & 0x1FFF ); }
NES_PEEK(MAPPER,cRam_1_A13_Hi) { TriggerA13Hi1(); return ppu.Peek_cRam( address & 0x1FFF ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER,cRom)
{
	if (IsCRam)
		cRom[address & 0x1FFF] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER,6000) { wRam[address - 0x6000] = data; }
NES_PEEK(MAPPER,6000) { return wRam[address - 0x6000]; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER,Nop)
{
	return cpu.GetCache();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER::LoadState(PDXFILE& file)
{
	PDX_TRY(LoadWRam( file ));
	PDX_TRY(pRom.LoadState( file, FALSE ));

	if (cRom.Size())
		PDX_TRY(cRom.LoadState( file, IsCRam ));

	PDX_TRY(LoadIrqData( file ));
	PDX_TRY(LoadStateChunks( file ));

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER::SaveState(PDXFILE& file) const
{
	PDX_TRY(SaveWRam( file ));
	PDX_TRY(pRom.SaveState( file, FALSE ));

	if (cRom.Size())
		PDX_TRY(cRom.SaveState( file, IsCRam ));

	PDX_TRY(SaveIrqData( file ));
	PDX_TRY(SaveStateChunks( file ));

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER::LoadWRam(PDXFILE& file)
{
	U32 size;

	if (!file.Read(size) || size != wRam.Size())
		return PDX_FAILURE;

	U8 present;

	if (!file.Read(present) || (present & 0xF0))
		return PDX_FAILURE;

	if ((present & 0x1) && !file.Read( wRam.At(0x0000), wRam.At(0x1000) )) return PDX_FAILURE;
	if ((present & 0x2) && !file.Read( wRam.At(0x1000), wRam.At(0x2000) )) return PDX_FAILURE;
	if ((present & 0x4) && !file.Read( wRam.At(0x2000), wRam.End()      )) return PDX_FAILURE;

	if (!(present & 0x1)) wRam.Fill( wRam.At(0x0000), wRam.At(0x1000), 0x60 );
	if (!(present & 0x2)) wRam.Fill( wRam.At(0x1000), wRam.At(0x2000), 0x70 );
	if (!(present & 0x4)) wRam.Fill( wRam.At(0x2000), wRam.End(),      0x00 );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER::SaveWRam(PDXFILE& file) const
{
	U8 present = 0;

	for (UINT i=0x0000; i < 0x1000; ++i)
	{
		if (wRam[i] != 0x60)
		{
			present |= 0x1;
			break;
		}
	}

	for (UINT i=0x1000; i < 0x2000; ++i)
	{
		if (wRam[i] != 0x70)
		{
			present |= 0x2;
			break;
		}
	}

	for (UINT i=0x2000; i < wRam.Size(); ++i)
	{
		if (wRam[i])
		{
			present |= 0x4;
			break;
		}
	}

	file << U32(wRam.Size());
	file << present;

	if (present & 0x1) file.Write( wRam.At(0x0000), wRam.At(0x1000) );
	if (present & 0x2) file.Write( wRam.At(0x1000), wRam.At(0x2000) );
	if (present & 0x4) file.Write( wRam.At(0x2000), wRam.End()      );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER::LoadIrqData(PDXFILE& file)
{
	U8 present;

	if (!file.Read(present) || present > 1)
		return PDX_FAILURE;

	if (present)
	{
		if (!file.Readable(sizeof(U8) * 12))
			return PDX_FAILURE;

		IrqEnabled = file.Read<U8>  ();
		IrqCount   = file.Read<I32> ();
		IrqLatch   = file.Read<U8>  ();   
		IrqTmp     = file.Read<U8>  ();     
		IrqCycles  = file.Read<I32> ();
		IrqA13     = file.Read<U8>  () << 8;
	}			

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER::SaveIrqData(PDXFILE& file) const
{
	const U8 IrqData = (IrqEnabled || IrqCount || IrqLatch || IrqTmp || IrqCycles || IrqA13) ? 1 : 0;

	file << IrqData;

	if (IrqData)
	{
		file <<  U8( IrqEnabled ? 1 : 0 );
		file << I32( IrqCount           );
		file <<  U8( IrqLatch           );
		file <<  U8( IrqTmp             );
		file << I32( IrqCycles          );
		file <<  U8( IrqA13 >> 8        );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER::LoadStateChunks(PDXFILE& file)
{
	U32 size;

	if (!file.Read(size))
		return PDX_FAILURE;

	if (StateChunk1 && StateSize1)
	{
		if (size != StateSize1)
			return PDX_FAILURE;

		const BOOL yep = file.Read
		(
			PDX_CAST(CHAR*,StateChunk1),
			PDX_CAST(const CHAR*,StateChunk1) + StateSize1
		);

		if (!yep)
			return PDX_FAILURE;
	}
	else
	{
		if (size)
			return PDX_FAILURE;
	}

	if (!file.Read(size))
		return PDX_FAILURE;

	if (StateChunk2 && StateSize2)
	{
		if (size != StateSize2)
			return PDX_FAILURE;

		const BOOL yep = file.Read
		(
			PDX_CAST(CHAR*,StateChunk2),
			PDX_CAST(const CHAR*,StateChunk2) + StateSize2
		);

		if (!yep)
			return PDX_FAILURE;
	}
	else
	{
		if (size)
			return PDX_FAILURE;
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER::SaveStateChunks(PDXFILE& file) const
{
	if (StateChunk1 && StateSize1)
	{
		file.Write( U32(StateSize1) );

		file.Write
		(
			PDX_CAST(const CHAR*,StateChunk1),
			PDX_CAST(const CHAR*,StateChunk1) + StateSize1
		);
	}
	else
	{
		file.Write( U32(0) );
	}

	if (StateChunk2 && StateSize2)
	{
		file.Write( U32(StateSize2) );

		file.Write
		(
			PDX_CAST(const CHAR*,StateChunk2),
			PDX_CAST(const CHAR*,StateChunk2) + StateSize2
		);
	}
	else
	{
		file.Write( U32(0) );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const CHAR* MAPPER::boards[256] =
{
	"NROM",				               // 000
	"MMC1",				               // 001
	"UNROM",			               // 002
	"CNROM",			               // 003
	"MMC3/MMC6",			           // 004
	"MMC5",				               // 005
	"FFE F4xxx",		               // 006
	"AUROM",			               // 007
	"FFE F3xxx",		               // 008
	"MMC2",				               // 009
	"MMC4",				               // 010
	"COLOR DREAMS",		               // 011
	"",					               // 012
	"CPROM",			               // 013
	"",					               // 014
	"CONTRA 100-IN-1",	               // 015
	"BANDAI",			               // 016
	"FFE F8xxx",		               // 017
	"JALECO SS8806",	               // 018
	"NAMCOT 106",		               // 019
	"",					               // 020
	"KONAMI VRC4 2A",	               // 021
	"KONAMI VRC4 1B",	               // 022
	"KONAMI VRC2B",		               // 023
	"KONAMI VRC6",  	               // 024
	"KONAMI VRC4 Y",	               // 025
	"KONAMI VRC6 A0-A1",               // 026
	"",					               // 027
	"",					               // 028
	"",					               // 029
	"",					               // 030
	"",					               // 031
	"IREM G-101",		               // 032
	"TAITO TC0190/TC0350",             // 033
	"NINA-1 / BNROM",  	               // 034
	"",					               // 035
	"",					               // 036
	"",					               // 037
	"",					               // 038
	"",					               // 039
	"BOOTLEG (Smb2j)",	               // 040
	"CALTRON 6-IN-1",	               // 041
	"BOOTLEG (Mario Baby)",            // 042
	"LF36 (Smb2j)",                    // 043
	"SUPER HiK 7-IN-1 (MMC3)",	       // 044
	"SUPER (1000000/13)-IN-1 (MMC3)",  // 045
	"GAME-STATION / RUMBLE-STATION",   // 046
	"NES-QJ (MMC3)",	               // 047
	"TAITO TC190V",		               // 048
	"SUPER HiK 4-IN-1 (MMC3)",         // 049
	"SMB2j rev.A", 	                   // 050
	"Ball Games 11-IN-1",              // 051
	"Mario Party 7-IN-1 (MMC3)",       // 052
	"",					               // 053
	"",					               // 054
	"",					               // 055
	"",					               // 056
	"GAME STAR GK-54",	               // 057
	"STUDY & GAME 32-IN-1",            // 058
	"BMC-T3H53",		               // 059
	"RESET-TRIGGERED 4-IN-1",          // 060
	"20-IN-1",			               // 061
	"700-IN-1",			               // 062
	"HELLO KITTY 255-IN-1",            // 063
	"TENGEN RAMBO-1 ",	               // 064
	"IREM H-3001",		               // 065
	"GNROM",                           // 066
	"SUNSOFT #3",		               // 067
	"SUNSOFT #4",		               // 068
	"SUNSOFT FME-7",	               // 069
	"",             	               // 070
	"CAMERICA", 	                   // 071
	"JALECO",			               // 072
	"KONAMI VRC3",		               // 073
	"Taiwanese MMC3",                  // 074
	"JALECO SS8805 / KONAMI VRC1",     // 075
	"NAMCO 109",		               // 076
	"IREM",				               // 077
	"IREM 74HC161/32",                 // 078
	"NINA-06 / NINA-03",	           // 079
	"TAITO X-005",			           // 080
	"",					               // 081
	"TAITO",			               // 082
	"BOOTLEG (Dragon Ball Party)",     // 083
	"",					               // 084
	"KONAMI VRC7",		               // 085
	"JALECO",			               // 086
	"KONAMI 74HC161/32",               // 087
	"NAMCO 118",		               // 088
	"SUNSOFT",			               // 089
	"PC-JY",     		               // 090
	"PC-HK-SF3",					   // 091
	"JALECO (early)",	               // 092
	"SUNSOFT 74HC161/32",		       // 093
	"CAPCOM 74HC161/32",               // 094
	"DRAGON BUSTER (MMC3)",            // 095
	"BANDAI 74HC161/32",			   // 096
	"IREM 74HC161/32",		           // 097
	"",					               // 098
	"VS SYSTEM 8KB CHR SWITCH",        // 099	
	"NESTICLE MMC3",	               // 100	
	"JALECO 74HC161/32",		       // 101	
	"",					               // 102	
	"",					               // 103	
	"",					               // 104	
	"NES-EVENT",		               // 105	
	"",					               // 106	
	"",					               // 107	
	"",					               // 108	
	"",					               // 109	
	"",					               // 110	
	"",					               // 111	
	"ASDER",			               // 112	
	"MB-91",			               // 113	
	"",					               // 114	
	"",					               // 115	
	"",					               // 116	
	"",					               // 117	
	"TLSROM / TKSROM (MMC3)",	       // 118	
	"TQROM (MMC3)",                    // 119	
	"",					               // 120	
	"",					               // 121	
	"SUNSOFT 74HC161/32",              // 122	
	"",					               // 123	
	"",					               // 124	
	"",					               // 125	
	"",					               // 126	
	"",					               // 127	
	"",					               // 128	
	"",					               // 129	
	"",					               // 130	
	"",					               // 131	
	"",					               // 132	
	"",					               // 133	
	"",					               // 134	
	"",					               // 135	
	"",					               // 136	
	"",					               // 137	
	"",					               // 138	
	"",					               // 139	
	"JALECO",			               // 140	
	"SACHEN",			               // 141	
	"KS 202 BOOTLEG (Smb2j)",          // 142	
	"SACHEN",			               // 143	
	"AGCI",	    		               // 144	
	"",					               // 145	
	"",					               // 146	
	"",					               // 147	
	"",					               // 148	
	"",					               // 149	
	"",					               // 150	
	"VS SYSTEM (KONAMI)",              // 151	
	"",					               // 152	
	"BANDAI",			               // 153	
	"NAMCO",			               // 154	
	"MMC1 (without WRAM)",	           // 155	
	"",					               // 156	
	"",					               // 157	
	"",					               // 158	
	"",					               // 159	
	"",					               // 160	
	"",					               // 161	
	"",					               // 162	
	"",					               // 163	
	"",					               // 164	
	"",					               // 165	
	"",					               // 166	
	"",					               // 167	
	"",					               // 168	
	"",					               // 169	
	"",	       			               // 170	
	"",					               // 171	
	"",					               // 172	
	"",					               // 173	
	"",					               // 174	
	"",					               // 175	
	"",					               // 176	
	"",					               // 177	
	"",					               // 178	
	"",					               // 179	
	"NICHIBUTSU",	                   // 180	
	"",					               // 181	
	"SUPER DONKEY KONG",               // 182	
	"",					               // 183	
	"SUNSOFT 74HC161/32",	           // 184	
	"CHR-ROM Disable Protect",		   // 185	
	"",					               // 186	
	"",					               // 187	
	"BANDAI KARAOKE STUDIO",           // 188	
	"SF2 YOKO VERSION",                // 189	
	"",					               // 190	
	"",					               // 191	
	"",					               // 192	
	"",					               // 193	
	"",					               // 194	
	"",					               // 195	
	"",					               // 196	
	"",					               // 197	
	"",					               // 198	
	"",					               // 199	
	"",					               // 200	
	"",					               // 201	
	"",					               // 202	
	"",					               // 203	
	"",					               // 204	
	"",					               // 205	
	"",					               // 206	
	"",					               // 207	
	"",					               // 208	
	"",					               // 209	
	"",					               // 210	
	"",					               // 211
	"",					               // 212
	"",					               // 213
	"",					               // 214
	"",					               // 215
	"",					               // 216
	"",					               // 217
	"",					               // 218
	"",					               // 219
	"",					               // 220
	"",					               // 221
	"",					               // 222
	"",					               // 223
	"",					               // 224
	"BOOTLEG (58-IN-1 & 110-IN-1/52)", // 225
	"BOOTLEG (76-IN-1)",               // 226
	"BOOTLEG (1200-IN-1)",             // 227
	"ACTION 52",		               // 228
	"BOOTLEG (31-IN-1)",               // 229
	"",					               // 230
	"20-IN-1",			               // 231
	"BIC-48",			               // 232
	"",					               // 233
	"MAXI-15",			               // 234
	"BOOTLEG (Golden Game 150-IN-1)",  // 235
	"",					               // 236
	"",					               // 237
	"",					               // 238
	"",					               // 239
	"",					               // 240
	"",					               // 241
	"",					               // 242
	"SACHEN 74LS374N",			       // 243
	"",					               // 244
	"",					               // 245
	"",					               // 246
	"",					               // 247
	"",					               // 248
	"WAIXING",			               // 249
	"TIME DIVER AVENGER (MMC3)",       // 250
	"",					               // 251
	"",					               // 252
	"",					               // 253
	"",					               // 254
	"110-IN-1"			               // 255
};

NES_NAMESPACE_END
