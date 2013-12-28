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

#ifndef NST_SNDN106_H
#define NST_SNDN106_H

#include "../NstApu.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class SNDN106
{
public:

	SNDN106(CPU&);
	~SNDN106();

	VOID Reset();

	VOID Poke_4800(const UINT);
	UINT Peek_4800();
	VOID Poke_F800(const UINT);

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;

private:

	enum
	{
		NUM_CHANNELS = 8,
		ADDRESS_MAX  = b01111111,
		ADDRESS_INC  = b10000000,
		WAVE_LENGTH  = b00011100,
		PHASE_SHIFT  = 18,
		SPEED_SHIFT  = 20
	};

	class CHANNEL : public APU::CHANNEL
	{
	public:

		CHANNEL(SNDN106* const,const UINT);

		VOID Reset();
		VOID Update();

		VOID WriteReg0(const UINT);
		VOID WriteReg2(const UINT);
		VOID WriteReg4(const UINT);
		VOID WriteReg6(const UINT);
		VOID WriteReg7(const UINT);

		PDXRESULT LoadState(PDXFILE&);
		PDXRESULT SaveState(PDXFILE&) const;

	private:

		VOID UpdateContext();

		LONG Sample();

		const UINT channel;
		
		UINT  address;
		UINT  volume;
		ULONG speed;
		ULONG WaveLength;
		ULONG phase;
		ULONG ChannelSpeed;

		SNDN106* const mother;
	};

	friend class CHANNEL;

	APU& apu;

	UINT address;
	UINT AddressIncrease;
	UINT NumChannels;
	
	U8 ExRam[0x80];
	U32 tone[0x100];
	
	CHANNEL* channels[NUM_CHANNELS];
};

NES_NAMESPACE_END

#endif
