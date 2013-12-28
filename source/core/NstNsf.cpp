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

#include "../paradox/PdxFile.h"
#include "NstTypes.h"
#include "NstCpu.h"
#include "Nstfds.h"
#include "sound/NstSndVrc6.h"
#include "sound/NstSndN106.h"
#include "sound/NstSndFme7.h"
#include "sound/NstSndMmc5.h"
#include "NstNsf.h"

NES_NAMESPACE_BEGIN

#define NES_TIMEOUT_INSTRUCTIONS NES_NTSC_TO_CPU( NES_CPU_MCC_FRAME_NTSC )

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NSF::NSF(CPU& c)
: 
cpu     (c),
apu     (c.GetAPU()),
chip    (NULL),
playing (FALSE),
song    (0) 
{
	context.chip = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NSF::~NSF()
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::Destroy()
{
	wRam.Destroy();

	switch (context.chip)
	{
     	case CHIP_VRCVI: delete vrc6; break;
		case CHIP_FDS:	 delete fds;  break;
		case CHIP_N106:  delete n106; break;
		case CHIP_FME7:	 delete fme7; break;
		case CHIP_MMC5:  delete mmc5; break;
	}

	chip = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSF::Load(PDXFILE& file)
{
	Destroy();

	if (!file.Read( context ))
		return MsgError("Not a valid nsf file!");

	if (*PDX_CAST(const U32*,context.signature) != 0x4D53454EUL)
		return MsgError("Invalid file type or file is corrupt!");

	// just in case
	context.LoadAddress |= 0x8000;
	context.InitAddress |= 0x8000;
	context.PlayAddress |= 0x8000;

	BankSwitched = 
	(
		PDX_CAST(const U32*,context.banks)[0] ||
		PDX_CAST(const U32*,context.banks)[1]
	);

	switch (context.chip)
	{
     	case CHIP_VRCVI:  InitVrc6(); break;		
		case CHIP_FDS:	  InitFds();  break;
		case CHIP_MMC5:	  InitMmc5(); break;
		case CHIP_N106:   InitN106(); break;
		case CHIP_FME7:   InitFme7(); break;
		case CHIP_VRCVII: return MsgError("The VRCVII chip is not supported!");
	}

	if (wRam.IsEmpty())
		wRam.Resize( n8k );

	const TSIZE pRomSize = (file.Size() - sizeof(CONTEXT)) * sizeof(U8);

	if (!pRomSize)
		return MsgError("NSF file is corrupt!");

	const UINT offset = context.LoadAddress & 0xFFF;
	const UINT size = pRomSize + offset;
	
	UINT pow2 = 1;

	while (pow2 < size)
		pow2 += pow2;

	pRom.ReAssign( pow2 );
	pRom.Fill( 0, offset, JAM );
	
	if (!file.Read( pRom.Ram() + offset, pRom.Ram() + size ))
		return MsgError("NSF file is corrupt!");

	cpu.SetupFrame( NES_CPU_MCC_FRAME_NTSC );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::InitVrc6() 
{
	PDX_ASSERT( !vrc6 );
	vrc6 = new SNDVRC6(cpu);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::InitFds()	 
{
	PDX_ASSERT( !fds );
	fds  = new SNDFDS(cpu);
	wRam.Resize( n8k + n32k );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::InitN106() 
{
	PDX_ASSERT( !n106 );
	n106 = new SNDN106(cpu);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::InitFme7() 
{
	PDX_ASSERT( !fme7 );
	fme7 = new SNDFME7(cpu);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::InitMmc5() 
{
	PDX_ASSERT( !mmc5 );
	mmc5 = new SNDMMC5(cpu);
	wRam.Resize( n8k + n1k );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::Reset()
{
	cpu.SetPort( 0x2000, 0x3FFF, this, Peek_bad, Poke_bad );
	cpu.SetPort( 0x4014,         this, Peek_bad, Poke_bad );
	cpu.SetPort( 0x4016,         this, Peek_bad, Poke_bad );
	cpu.SetPort( 0x4017,         this, Peek_bad, Poke_bad );

	cpu.SetPort( ROUTINE + 0, this, Peek_Routine_0, Poke_bad );
	cpu.SetPort( ROUTINE + 1, this, Peek_Routine_1, Poke_bad );
	cpu.SetPort( ROUTINE + 2, this, Peek_Routine_2, Poke_bad );
	cpu.SetPort( ROUTINE + 3, this, Peek_Routine_3, Poke_bad );

	if (BankSwitched)
	{
		cpu.SetPort( 0x5FF8, this, Peek_bad, Poke_5FF8 );
		cpu.SetPort( 0x5FF9, this, Peek_bad, Poke_5FF9 );
		cpu.SetPort( 0x5FFA, this, Peek_bad, Poke_5FFA );
		cpu.SetPort( 0x5FFB, this, Peek_bad, Poke_5FFB );
		cpu.SetPort( 0x5FFC, this, Peek_bad, Poke_5FFC );
		cpu.SetPort( 0x5FFD, this, Peek_bad, Poke_5FFD );
		cpu.SetPort( 0x5FFE, this, Peek_bad, Poke_5FFE );
		cpu.SetPort( 0x5FFF, this, Peek_bad, Poke_5FFF );
	}

	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_wRam, Poke_wRam );
	cpu.SetPort( 0x8000, 0xFFFF, this, Peek_pRom, Poke_bad  );

	switch (context.chip)
	{
       	case CHIP_VRCVI: ResetVrc6(); break;		
       	case CHIP_FDS:	 ResetFds();  break;
     	case CHIP_N106:  ResetN106(); break;
     	case CHIP_FME7:  ResetFme7(); break;
		case CHIP_MMC5:  ResetMmc5(); break;
	}

	song = context.StartSong - 1;
	
	LoadSong();
	playing = TRUE;

	ResetLog();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::ResetLog()
{
	PDXSTRING log("NSF: ");

	LogOutput( log << "reset" );

	log.Resize( 5 ); 
	LogOutput( log << "version " << context.version );
	
	log.Resize( 5 ); LogOutput( log <<	"name: "      << context.info.name      );
	log.Resize( 5 ); LogOutput( log <<	"artist: "    << context.info.artist    );
	log.Resize( 5 ); LogOutput( log <<	"copyright: " << context.info.copyright );
	
	log.Resize( 5 ); 
	LogOutput( log << "start song " << context.StartSong << " of " << context.NumSongs );

	log.Resize( 5 );

	if (context.mode.pal)
	{
		if (context.mode.ntsc) log << "PAL/NTSC";
		else				   log << "PAL only";
	}
	else
	{
		log << "NTSC only";
	}

	LogOutput( log );

	log.Resize( 5 );

	switch (context.chip)
	{
       	case CHIP_VRCVI: log << "Konami VRC6 sound chip present";         break;		
		case CHIP_FDS:	 log << "Famicom Disk System sound chip present"; break;
		case CHIP_N106:  log << "Namco N106 sound chip present";          break;
		case CHIP_FME7:  log << "Sunsoft FME-07 sound chip present";      break;
		case CHIP_MMC5:  log << "MMC5 sound chip present";                break;
	}

	if (log.Length() > 5)
	{
		LogOutput( log );
		log.Resize( 5 );
	}

	log << (pRom.Size() / 1024);
	log << (BankSwitched ? "k bankswitchable PRG-ROM present" : "k non-bankswitchable PRG-ROM present");
	LogOutput( log );

	log.Resize( 5 );
	log << (wRam.Size() / 1024);
	log << (context.chip == CHIP_FDS ? "k PRG-RAM present" : "k WRAM present");

	if (context.chip != CHIP_MMC5 && context.chip != CHIP_FDS)
		log << " for compatibility";

	LogOutput( log );

	log.Resize( 5 ); log << "Load Address - "; log.Append( context.LoadAddress, PDXSTRING::HEX ); LogOutput( log );
	log.Resize( 5 ); log << "Init Address - "; log.Append( context.InitAddress, PDXSTRING::HEX ); LogOutput( log );
	log.Resize( 5 ); log << "Play Address - "; log.Append( context.PlayAddress, PDXSTRING::HEX ); LogOutput( log );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::ResetVrc6()
{
	PDX_ASSERT( vrc6 );

	cpu.SetPort( 0x9000, this, Peek_pRom, Poke_vrc6_9000 );
	cpu.SetPort( 0x9001, this, Peek_pRom, Poke_vrc6_9001 );
	cpu.SetPort( 0x9002, this, Peek_pRom, Poke_vrc6_9002 );
	cpu.SetPort( 0xA000, this, Peek_pRom, Poke_vrc6_A000 );
	cpu.SetPort( 0xA001, this, Peek_pRom, Poke_vrc6_A001 );
	cpu.SetPort( 0xA002, this, Peek_pRom, Poke_vrc6_A002 );
	cpu.SetPort( 0xB000, this, Peek_pRom, Poke_vrc6_B000 );
	cpu.SetPort( 0xB001, this, Peek_pRom, Poke_vrc6_B001 );
	cpu.SetPort( 0xB002, this, Peek_pRom, Poke_vrc6_B002 );

	vrc6->Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::ResetFds()
{
	PDX_ASSERT( fds );

	cpu.SetPort( 0x5FF6, this, Peek_bad, Poke_fds_5FF6 );
	cpu.SetPort( 0x5FF7, this, Peek_bad, Poke_fds_5FF7 );
	cpu.SetPort( 0x5FF8, this, Peek_bad, Poke_fds_5FF8 );
	cpu.SetPort( 0x5FF9, this, Peek_bad, Poke_fds_5FF9 );
	cpu.SetPort( 0x5FFA, this, Peek_bad, Poke_fds_5FFA );
	cpu.SetPort( 0x5FFB, this, Peek_bad, Poke_fds_5FFB );
	cpu.SetPort( 0x5FFC, this, Peek_bad, Poke_fds_5FFC );
	cpu.SetPort( 0x5FFD, this, Peek_bad, Poke_fds_5FFD );
	cpu.SetPort( 0x5FFE, this, Peek_bad, Poke_fds_5FFE );
	cpu.SetPort( 0x5FFF, this, Peek_bad, Poke_fds_5FFF );

	cpu.SetPort( 0x6000, 0xDFFF, this, Peek_wRam, Poke_wRam );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_wRam, Poke_bad  );

	fds->Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::ResetN106()
{
	PDX_ASSERT( n106 );

	cpu.SetPort( 0x4800, 0x4FFF, this, Peek_n106_4800, Poke_n106_4800 );
	cpu.SetPort( 0xF800, 0xFFFF, this, Peek_pRom,      Poke_n106_F800 );

	n106->Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::ResetFme7()
{
	PDX_ASSERT( fme7 );

	cpu.SetPort( 0xC000, 0xDFFF, this, Peek_pRom, Poke_fme7_C000 );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_pRom, Poke_fme7_E000 );

	fme7->Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::ResetMmc5()
{
	PDX_ASSERT( mmc5 );

	cpu.SetPort( 0x5C00, 0x5FF5, this, Peek_ExRam, Poke_ExRam );

	mmc5->Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSF::GetContext(IO::NSF::CONTEXT& out) const
{
	out.song      = song;
	out.NumSongs  = context.NumSongs;
	out.name      = context.info.name;
	out.artist    = context.info.artist;
	out.copyright = context.info.copyright;
	out.pal       = context.mode.pal;

	switch (context.chip)
	{
     	case CHIP_VRCVI:  out.chip = "VRC6";           return PDX_OK;
		case CHIP_VRCVII: out.chip = "VRC7";           return PDX_OK;
		case CHIP_FDS:	  out.chip = "FDS";			   return PDX_OK;
		case CHIP_MMC5:	  out.chip = "MMC5";           return PDX_OK;
		case CHIP_N106:   out.chip = "Namco 106";      return PDX_OK;
		case CHIP_FME7:   out.chip = "Sunsoft FME-07"; return PDX_OK;
	}					  

	out.chip.Clear();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NSF::SetContext(const IO::NSF::CONTEXT& out)
{
	switch (out.op)
	{
     	case IO::NSF::PLAY: Play(); return PDX_OK;
		case IO::NSF::STOP: Stop(); return PDX_OK;
		case IO::NSF::NEXT: Next(); return PDX_OK;
		case IO::NSF::PREV: Prev(); return PDX_OK;
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::LoadSong()
{
	wRam.Fill( 0x00 );

	cpu.ClearRAM();

	if (BankSwitched)
	{
		pRom.SwapBanks<n4k,0x0000>( context.banks[0] );
		pRom.SwapBanks<n4k,0x1000>( context.banks[1] );
		pRom.SwapBanks<n4k,0x2000>( context.banks[2] );
		pRom.SwapBanks<n4k,0x3000>( context.banks[3] );
		pRom.SwapBanks<n4k,0x4000>( context.banks[4] );
		pRom.SwapBanks<n4k,0x5000>( context.banks[5] );
		pRom.SwapBanks<n4k,0x6000>( context.banks[6] );
		pRom.SwapBanks<n4k,0x7000>( context.banks[7] );
	}
	else
	{
		const UINT offset = context.LoadAddress & 0x7000;
	
		for (UINT i=0x0000; i < offset; i += 0x1000)
			pRom.SwapBanks<n4k>( i, 0 );
	
		for (UINT i=offset; i < 0x8000; i += 0x1000)
			pRom.SwapBanks<n4k>( i, ((i-offset) & pRom.Mask()) >> 12 );
	}

	if (context.chip == CHIP_FDS)
		LoadFds();

	cpu.ResetCycles();
	apu.Reset();

	cpu.Poke( 0x4000, 0x00 );
	cpu.Poke( 0x4001, 0x00 );
	cpu.Poke( 0x4002, 0x00 );
	cpu.Poke( 0x4003, 0x00 );
	cpu.Poke( 0x4004, 0x00 );
	cpu.Poke( 0x4005, 0x00 );
	cpu.Poke( 0x4006, 0x00 );
	cpu.Poke( 0x4007, 0x00 );
	cpu.Poke( 0x4008, 0x00 );
	cpu.Poke( 0x400A, 0x00 );
	cpu.Poke( 0x400B, 0x00 );
	cpu.Poke( 0x400C, 0x10 );
	cpu.Poke( 0x400E, 0x00 );
	cpu.Poke( 0x400F, 0x00 );
	cpu.Poke( 0x4010, 0x10 );
	cpu.Poke( 0x4011, 0x00 );
	cpu.Poke( 0x4012, 0x00 );
	cpu.Poke( 0x4013, 0x00 );
	cpu.Poke( 0x4015, 0x0F );

	cpu.Poke_4017( 0x40 );

	SetUpRoutine( context.InitAddress, song, context.mode.pal );
	ExecuteRoutine();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::LoadFds()
{
	memcpy( wRam.At( 0x0000 ), pRom.GetBank( 6 ), sizeof(U8) * n4k );
	memcpy( wRam.At( 0x1000 ), pRom.GetBank( 7 ), sizeof(U8) * n4k );
	memcpy( wRam.At( 0x2000 ), pRom.GetBank( 0 ), sizeof(U8) * n4k );
	memcpy( wRam.At( 0x3000 ), pRom.GetBank( 1 ), sizeof(U8) * n4k );
	memcpy( wRam.At( 0x4000 ), pRom.GetBank( 2 ), sizeof(U8) * n4k );
	memcpy( wRam.At( 0x5000 ), pRom.GetBank( 3 ), sizeof(U8) * n4k );
	memcpy( wRam.At( 0x6000 ), pRom.GetBank( 4 ), sizeof(U8) * n4k );
	memcpy( wRam.At( 0x7000 ), pRom.GetBank( 5 ), sizeof(U8) * n4k );
	memcpy( wRam.At( 0x8000 ), pRom.GetBank( 6 ), sizeof(U8) * n4k );
	memcpy( wRam.At( 0x9000 ), pRom.GetBank( 7 ), sizeof(U8) * n4k );

	cpu.Poke( 0x4089, 0x80 );
	cpu.Poke( 0x408A, 0xE8 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::UnloadSong()
{
	cpu.SetPC( ROUTINE_4 );
	apu.ClearBuffers();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::Execute()
{
	if (playing)
	{
		SetUpRoutine( context.PlayAddress, 0, 0 );
		cpu.Execute();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::SetUpRoutine(const UINT address,const UINT a,const UINT x)
{
	RoutineAddress.d = address;

	cpu.SetPC ( ROUTINE );
	cpu.SetA  ( a       );
	cpu.SetX  ( x       );
	cpu.SetY  ( 0x00    );
	cpu.SetSP ( 0xFF    );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::ExecuteRoutine()
{
	cpu.ResetCycles();

	RoutineDone = FALSE;

	for (ULONG timeout=0; !RoutineDone && timeout < NES_TIMEOUT_INSTRUCTIONS; ++timeout)
		cpu.Step();

	cpu.ResetCycles();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::Stop()
{
	if (playing)
	{
		playing = FALSE;
		UnloadSong();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::Play()
{
	if (!playing)
	{
		playing = TRUE;
		LoadSong();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::Next()
{
	if (song+1 < context.NumSongs)
	{
		++song;
		UnloadSong();

		if (playing) 
			LoadSong();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NSF::Prev()
{
	if (song > 0)
	{
		--song;
		UnloadSong();

		if (playing) 
			LoadSong();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// player routine
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(NSF,Routine_0) { return JSR;                }
NES_PEEK(NSF,Routine_1) { return RoutineAddress.b.l; }
NES_PEEK(NSF,Routine_2) { return RoutineAddress.b.h; }
NES_PEEK(NSF,Routine_3) { cpu.SetPC( cpu.GetPC() - 1 ); RoutineDone = TRUE; return NOP; }

////////////////////////////////////////////////////////////////////////////////////////
// general
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(NSF,5FF8) { apu.Update(); pRom.SwapBanks<n4k,0x0000>( data ); }
NES_POKE(NSF,5FF9) { apu.Update(); pRom.SwapBanks<n4k,0x1000>( data ); }
NES_POKE(NSF,5FFA) { apu.Update(); pRom.SwapBanks<n4k,0x2000>( data ); }
NES_POKE(NSF,5FFB) { apu.Update(); pRom.SwapBanks<n4k,0x3000>( data ); }
NES_POKE(NSF,5FFC) { apu.Update(); pRom.SwapBanks<n4k,0x4000>( data ); }
NES_POKE(NSF,5FFD) { apu.Update(); pRom.SwapBanks<n4k,0x5000>( data ); }
NES_POKE(NSF,5FFE) { apu.Update(); pRom.SwapBanks<n4k,0x6000>( data ); }
NES_POKE(NSF,5FFF) { apu.Update(); pRom.SwapBanks<n4k,0x7000>( data ); }

////////////////////////////////////////////////////////////////////////////////////////
// Famicom Disk System
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(NSF,fds_5FF6) { apu.Update(); memcpy( wRam.At( 0x0000 ), pRom.Ram( data << 12 ), sizeof(U8) * n4k ); } 
NES_POKE(NSF,fds_5FF7) { apu.Update(); memcpy( wRam.At( 0x1000 ), pRom.Ram( data << 12 ), sizeof(U8) * n4k ); }
NES_POKE(NSF,fds_5FF8) { apu.Update(); memcpy( wRam.At( 0x2000 ), pRom.Ram( data << 12 ), sizeof(U8) * n4k ); }
NES_POKE(NSF,fds_5FF9) { apu.Update(); memcpy( wRam.At( 0x3000 ), pRom.Ram( data << 12 ), sizeof(U8) * n4k ); }
NES_POKE(NSF,fds_5FFA) { apu.Update(); memcpy( wRam.At( 0x4000 ), pRom.Ram( data << 12 ), sizeof(U8) * n4k ); }
NES_POKE(NSF,fds_5FFB) { apu.Update(); memcpy( wRam.At( 0x5000 ), pRom.Ram( data << 12 ), sizeof(U8) * n4k ); }
NES_POKE(NSF,fds_5FFC) { apu.Update(); memcpy( wRam.At( 0x6000 ), pRom.Ram( data << 12 ), sizeof(U8) * n4k ); }
NES_POKE(NSF,fds_5FFD) { apu.Update(); memcpy( wRam.At( 0x7000 ), pRom.Ram( data << 12 ), sizeof(U8) * n4k ); }
NES_POKE(NSF,fds_5FFE) { apu.Update(); memcpy( wRam.At( 0x8000 ), pRom.Ram( data << 12 ), sizeof(U8) * n4k ); }
NES_POKE(NSF,fds_5FFF) { apu.Update(); memcpy( wRam.At( 0x9000 ), pRom.Ram( data << 12 ), sizeof(U8) * n4k ); }

////////////////////////////////////////////////////////////////////////////////////////
// Konami VRC6
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(NSF,vrc6_9000) { vrc6->WriteSquare1Reg0  ( data ); }
NES_POKE(NSF,vrc6_9001) { vrc6->WriteSquare1Reg1  ( data ); }
NES_POKE(NSF,vrc6_9002) { vrc6->WriteSquare1Reg2  ( data ); }
NES_POKE(NSF,vrc6_A000) { vrc6->WriteSquare2Reg0  ( data ); }
NES_POKE(NSF,vrc6_A001) { vrc6->WriteSquare2Reg1  ( data ); }
NES_POKE(NSF,vrc6_A002) { vrc6->WriteSquare2Reg2  ( data ); }
NES_POKE(NSF,vrc6_B000) { vrc6->WriteSawToothReg0 ( data ); }
NES_POKE(NSF,vrc6_B001) { vrc6->WriteSawToothReg1 ( data ); }
NES_POKE(NSF,vrc6_B002) { vrc6->WriteSawToothReg2 ( data ); }

////////////////////////////////////////////////////////////////////////////////////////
// Sunsoft FME-07
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(NSF,fme7_C000) { fme7->Poke_C000( data ); }
NES_POKE(NSF,fme7_E000) { fme7->Poke_E000( data ); }

////////////////////////////////////////////////////////////////////////////////////////
// Namcot 106
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(NSF,n106_4800) { return n106->Peek_4800(); }
NES_POKE(NSF,n106_4800) { n106->Poke_4800( data );  }
NES_POKE(NSF,n106_F800) { n106->Poke_F800( data );  }

////////////////////////////////////////////////////////////////////////////////////////
// MMC5
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(NSF,ExRam)	{ return wRam[address - (0x5C00 - 0x2000)]; }
NES_POKE(NSF,ExRam)	{ wRam[address - (0x5C00 - 0x2000)] = data; }

////////////////////////////////////////////////////////////////////////////////////////
// code/data
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(NSF,pRom) { return pRom[address - 0x8000]; }
NES_PEEK(NSF,wRam) { return wRam[address - 0x6000]; }
NES_POKE(NSF,wRam) { wRam[address - 0x6000] = data; }
NES_PEEK(NSF,bad)  { return cpu.GetCache();        }
NES_POKE(NSF,bad)  {                                }

NES_NAMESPACE_END
