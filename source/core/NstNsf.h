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

#ifndef NST_NSF_H
#define NST_NSF_H

#include "NstChip.h"

NES_NAMESPACE_BEGIN

class CPU;
class SNDVRC6;
class SNDFME7;
class SNDN106;
class SNDMMC5;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class NSF
{
public:

	NSF(CPU&);
	~NSF();

	VOID Reset();
	VOID Execute();
	VOID Stop();
	VOID Play();
	VOID Next();
	VOID Prev();

	PDXRESULT Load(PDXFILE&);
	PDXRESULT GetContext(IO::NSF::CONTEXT&) const;
	PDXRESULT SetContext(const IO::NSF::CONTEXT&);

	VOID SetMode(const MODE mode)
	{ cpu.SetFrameCycles( mode == MODE_PAL ? NES_CPU_MCC_FRAME_PAL : NES_CPU_MCC_FRAME_NTSC); }

	inline BOOL IsPAL() const
	{ return context.mode.pal; }

private:

	VOID ResetLog();
	VOID Destroy();

	VOID InitVrc6();
	VOID InitFds();
	VOID InitN106();
	VOID InitFme7();
	VOID InitMmc5();

	VOID ResetVrc6();
	VOID ResetFds();
	VOID ResetN106();
	VOID ResetFme7();
	VOID ResetMmc5();

	VOID LoadFds();
	VOID LoadSong();
	VOID UnloadSong();
	VOID SetUpRoutine(const UINT,const UINT,const UINT);
	VOID ExecuteRoutine();

	enum 
	{
		ROUTINE   = 0x3FFC,
		ROUTINE_1 = 0x3FFC,
		ROUTINE_2 = 0x3FFD,
		ROUTINE_3 = 0x3FFE,
		ROUTINE_4 = 0x3FFF,
		JAM		  = 0x02,
		JSR       = 0x20,
		NOP       = 0x1A
	};

	NES_DECL_PEEK( Routine_0 );
	NES_DECL_PEEK( Routine_1 );
	NES_DECL_PEEK( Routine_2 );
	NES_DECL_PEEK( Routine_3 );

	NES_DECL_PEEK( pRom );
	NES_DECL_PEEK( wRam );
	NES_DECL_POKE( wRam );

	NES_DECL_POKE( 5FF8 );
	NES_DECL_POKE( 5FF9 );
	NES_DECL_POKE( 5FFA );
	NES_DECL_POKE( 5FFB );
	NES_DECL_POKE( 5FFC );
	NES_DECL_POKE( 5FFD );
	NES_DECL_POKE( 5FFE );
	NES_DECL_POKE( 5FFF );

	NES_DECL_POKE( fds_5FF6 );
	NES_DECL_POKE( fds_5FF7 );
	NES_DECL_POKE( fds_5FF8 );
	NES_DECL_POKE( fds_5FF9 );
	NES_DECL_POKE( fds_5FFA );
	NES_DECL_POKE( fds_5FFB );
	NES_DECL_POKE( fds_5FFC );
	NES_DECL_POKE( fds_5FFD );
	NES_DECL_POKE( fds_5FFE );
	NES_DECL_POKE( fds_5FFF );

	NES_DECL_POKE( vrc6_9000 );
	NES_DECL_POKE( vrc6_9001 );
	NES_DECL_POKE( vrc6_9002 );
	NES_DECL_POKE( vrc6_A000 );
	NES_DECL_POKE( vrc6_A001 );
	NES_DECL_POKE( vrc6_A002 );
	NES_DECL_POKE( vrc6_B000 );
	NES_DECL_POKE( vrc6_B001 );
	NES_DECL_POKE( vrc6_B002 );

	NES_DECL_POKE( fme7_C000 );
	NES_DECL_POKE( fme7_E000 );

	NES_DECL_POKE( n106_4800 );
	NES_DECL_PEEK( n106_4800 );
	NES_DECL_POKE( n106_F800 );

	NES_DECL_PEEK( ExRam );
	NES_DECL_POKE( ExRam );

	NES_DECL_PEEK( bad );
	NES_DECL_POKE( bad );

	enum
	{
		CHIP_VRCVI  = b00000001,
		CHIP_VRCVII = b00000010,
		CHIP_FDS    = b00000100,
		CHIP_MMC5   = b00001000,
		CHIP_N106   = b00010000,
		CHIP_FME7   = b00100000
	};

   #pragma pack(push,1)

	struct CONTEXT
	{
		struct MODE
		{
			UCHAR ntsc   : 1;
			UCHAR pal    : 1;
            UCHAR        : 6;
		};

		struct INFO
		{
			CHAR name[32];
			CHAR artist[32];
			CHAR copyright[32];
		};

		CHAR signature[5];
		U8   version;
		U8   NumSongs;
		U8   StartSong;
		U16  LoadAddress;
		U16  InitAddress;
		U16  PlayAddress;
		INFO info;
		U16  SpeedNTSC;
		U8   banks[8];
		U16  SpeedPAL;
		MODE mode;
		U8   chip;
		U8   reserved[4];
	};

   #pragma pack(pop)

	CPU& cpu;
	APU& apu;

	typedef CHIP<n32k,8> PROM;
	typedef PDXARRAY<U8> WRAM;

	BOOL playing;
	UINT SongAddress;

	UINT song;
	BOOL BankSwitched;
	
	PDXWORD RoutineAddress;
	BOOL RoutineDone;

	CONTEXT context;

	union
	{
		SNDVRC6* vrc6;
		SNDFDS*  fds;
		SNDFME7* fme7;
		SNDN106* n106;
		SNDMMC5* mmc5;
		VOID*    chip;
	};

	PROM pRom;
	WRAM wRam;
};

NES_NAMESPACE_END

#endif
