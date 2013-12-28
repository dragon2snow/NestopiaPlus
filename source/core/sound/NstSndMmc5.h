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

#ifndef NST_SNDMMC5_H
#define NST_SNDMMC5_H

#include "../NstApu.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// MMC5
////////////////////////////////////////////////////////////////////////////////////////

class SNDMMC5
{
public:

	SNDMMC5(CPU* const);
	~SNDMMC5();

	VOID Reset();

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;

private:

	NES_DECL_POKE( 5000 );
	NES_DECL_POKE( 5002 );
	NES_DECL_POKE( 5003 );
	NES_DECL_POKE( 5004 );
	NES_DECL_POKE( 5006 );
	NES_DECL_POKE( 5007 );
	NES_DECL_POKE( 5011 );
	NES_DECL_POKE( 5010 );
	NES_DECL_POKE( 5015 );
	NES_DECL_PEEK( 5205 );
	NES_DECL_POKE( 5205 );
	NES_DECL_PEEK( 5206 );
	NES_DECL_POKE( 5206 );
	NES_DECL_PEEK( Nop  );

	class SQUARE : public APU::CHANNEL
	{
	public:

		VOID Reset();		
		VOID WriteReg0(const UINT);
		VOID WriteReg1(const UINT);
		VOID WriteReg2(const UINT);

		VOID UpdateQuarter();
		VOID UpdateWhole();

		VOID Toggle(const BOOL);

		PDXRESULT LoadState(PDXFILE&);
		PDXRESULT SaveState(PDXFILE&) const;

	private:

		enum
		{
			ENV_DECAY             = 0x0F,
			ENV_DECAY_DISABLE     = 0x10,
			ENV_DECAY_LOOP_ENABLE = 0x20,
			WAVE_LENGTH_HIGH      = 0x7,
			LENGTH_COUNTER_SHIFT  = 3,
			DUTY_SHIFT            = 6
		};

		LONG Sample();
		VOID UpdateVolume();

		LONG volume;
		UINT step;
		UINT DutyPeriod;
		UINT EnvCount;
		UINT EnvRate;
		UINT EnvDecayRate;
		UINT EnvDecayDisable;
		UINT EnvDecayLoop;
		UINT WaveLengthLow;
		UINT WaveLengthHigh;

       #pragma pack(push,1)

		struct HEADER
		{
			U8  WaveLengthLow;
			U8  step             : 4;
			U8  DutyPeriod       : 4;
			U8  EnvCount         : 4;
			U8  EnvDecayRate     : 4;
			U8  EnvRate          : 5;
			U8  EnvDecayLoop     : 4;
			U8  EnvDecayDisable  : 1;
			U8  WaveLengthHigh   : 3;
		};

       #pragma pack(pop)
	};

	class PCM : public APU::CHANNEL
	{
	public:

		VOID WriteReg0(const UINT);
		VOID WriteReg1(const UINT);

		PDXRESULT LoadState(PDXFILE& file)
		{ return APU::CHANNEL::LoadState( file ); }

		PDXRESULT SaveState(PDXFILE& file) const
		{ return APU::CHANNEL::SaveState( file ); }

	private:

		enum
		{
			PCM_ENABLE = 0x1
		};

		LONG Sample();
	};

	enum
	{
		SQUARE_0_ENABLE = 0x1,
		SQUARE_1_ENABLE = 0x2
	};

	CPU* const cpu;
	APU* const apu;
	
	UINT value[2];

	SQUARE square[2];
	PCM pcm;
};

NES_NAMESPACE_END

#endif
