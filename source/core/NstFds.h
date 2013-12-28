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

#ifndef NST_FDS_H
#define NST_FDS_H

class PDXFILE;

#include "sound/NstSndFds.h"

NES_NAMESPACE_BEGIN

class PPU;
class CPU;

////////////////////////////////////////////////////////////////////////////////////////
// Famicom Disk System
////////////////////////////////////////////////////////////////////////////////////////

class FDS
{
public:

	FDS(CPU&,PPU&);
	~FDS();

	PDXRESULT Load(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&);
	PDXRESULT LoadState(PDXFILE&);

	VOID Unload();
	VOID Reset(const BOOL);
	VOID VSync();

	PDXRESULT GetContext(IO::FDS::CONTEXT&) const;
	PDXRESULT SetContext(const IO::FDS::CONTEXT&);
	
	static VOID SetWriteProtection(const BOOL=TRUE);
	static BOOL IsWriteProtected();

	static PDXRESULT SetBIOS(PDXFILE&);
	static const U8* GetBIOS();

private:

	VOID LogReset();
	VOID WriteDiskFile();
	VOID IrqSync();

	enum
	{
		CTRL_MOTOR                = 0x01,
		CTRL_TRANSFER_RESET       = 0x02,
		CTRL_READ_MODE            = 0x04,
		CTRL_MIRRORING_HORIZONTAL = 0x08,
		CTRL_CRC                  = 0x10,
		CTRL_DRIVE_READY          = 0x40,
		CTRL_DISK_IRQ_ENABLED     = 0x80,
	};

	enum
	{
		IRQ_ALWAYS     = 0x1,
		IRQ_ENABLE     = 0x2,
		IRQ_WAIT_SHORT = 150,
		IRQ_WAIT_LONG  = 200
	};

	enum
	{
		IO_STATUS_IRQ_1 = 0x1,
		IO_STATUS_IRQ_2 = 0x2
	};

	enum
	{
		DISK_EJECTED           = 0xFFF,
		DISK_IO_ENABLED        = 0x01,
		BATTERY_CHARGED        = 0x80,
		STATUS_DRIVE_NOT_READY = 0x02
	};

	NES_DECL_PEEK( Nop  );
	NES_DECL_POKE( Nop  );
	NES_DECL_PEEK( wRam );
	NES_DECL_POKE( wRam );
	NES_DECL_PEEK( bRom );
	NES_DECL_POKE( 4020 );
	NES_DECL_POKE( 4021 );
	NES_DECL_POKE( 4022 );
	NES_DECL_POKE( 4023 );
	NES_DECL_POKE( 4024 );
	NES_DECL_POKE( 4025 );
	NES_DECL_PEEK( 4030 );
	NES_DECL_PEEK( 4031 );
	NES_DECL_PEEK( 4032 );
	NES_DECL_PEEK( 4033 );

	typedef PDXARRAY<PDXARRAY<U8,65500U> > DISKS;

	UINT offset;
	UINT last;
	UINT ctrl;
	UINT pos;
	UINT DiskEnabled;
	UINT WriteSkip;
	UINT InsertWait;

	PDXWORD IrqLatch;
	BOOL    IrqOnce;
	LONG    IrqWait;
	LONG    IrqCycles;

	CPU& cpu;
	PPU& ppu;

	DISKS disks;

	U8 wRam[n32k];

	SNDFDS sound;

	PDXSTRING ImageFileName;

	static BOOL WriteProtect;
	static BOOL BiosLoaded;
	static U8 bRom[n8k];

   #pragma pack(push,1)

	struct HEADER
	{
		U32 magic;
		U8 NumDisksAndSides;
		U8 reserved[11];
	};

   #pragma pack(pop)

	HEADER header;
};

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID FDS::SetWriteProtection(const BOOL state)
{ 
	WriteProtect = state; 
}

inline BOOL FDS::IsWriteProtected()
{ 
	return WriteProtect; 
}

NES_NAMESPACE_END

#endif
