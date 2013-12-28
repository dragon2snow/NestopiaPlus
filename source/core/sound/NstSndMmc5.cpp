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

#include "../../paradox/PdxFile.h"
#include "../NstTypes.h"
#include "../NstMap.h"
#include "../NstCpu.h"
#include "NstSndMmc5.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDMMC5::SNDMMC5(CPU* const c)
: 
cpu (c),
apu (c->GetAPU())
{
	apu->HookChannel( PDX_STATIC_CAST(APU::CHANNEL* const,square+0) );
	apu->HookChannel( PDX_STATIC_CAST(APU::CHANNEL* const,square+1) );
	apu->HookChannel( PDX_STATIC_CAST(APU::CHANNEL* const,&pcm)     );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDMMC5::~SNDMMC5()
{
	apu->ReleaseChannel( PDX_STATIC_CAST(APU::CHANNEL* const,square+0) );
	apu->ReleaseChannel( PDX_STATIC_CAST(APU::CHANNEL* const,square+1) );
	apu->ReleaseChannel( PDX_STATIC_CAST(APU::CHANNEL* const,&pcm)     );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDMMC5::Reset()
{
	cpu->SetPort( 0x5000, this, Peek_Nop,  Poke_5000 );
	cpu->SetPort( 0x5002, this, Peek_Nop,  Poke_5002 );
	cpu->SetPort( 0x5003, this, Peek_Nop,  Poke_5003 );
	cpu->SetPort( 0x5004, this, Peek_Nop,  Poke_5004 );
	cpu->SetPort( 0x5006, this, Peek_Nop,  Poke_5006 );
	cpu->SetPort( 0x5007, this, Peek_Nop,  Poke_5007 );
	cpu->SetPort( 0x5011, this, Peek_Nop,  Poke_5011 );
	cpu->SetPort( 0x5010, this, Peek_Nop,  Poke_5010 );
	cpu->SetPort( 0x5015, this, Peek_Nop,  Poke_5015 );
	cpu->SetPort( 0x5205, this, Peek_5205, Poke_5205 );
	cpu->SetPort( 0x5206, this, Peek_5206, Poke_5206 );

	value[0] = 0;
	value[1] = 0;

	square[0].Reset();
	square[1].Reset();
	pcm.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDMMC5::LoadState(PDXFILE& file)
{
	PDX_TRY(square[0].LoadState( file ));
	PDX_TRY(square[1].LoadState( file ));
	PDX_TRY(pcm.LoadState( file ));
	
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDMMC5::SaveState(PDXFILE& file) const
{
	PDX_TRY(square[0].SaveState( file ));
	PDX_TRY(square[1].SaveState( file ));
	PDX_TRY(pcm.SaveState( file ));
	
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(SNDMMC5,5000) { square[0].WriteReg0( data ); }
NES_POKE(SNDMMC5,5002) { square[0].WriteReg1( data ); }
NES_POKE(SNDMMC5,5003) { square[0].WriteReg2( data ); }
NES_POKE(SNDMMC5,5004) { square[1].WriteReg0( data ); }
NES_POKE(SNDMMC5,5006) { square[1].WriteReg1( data ); }
NES_POKE(SNDMMC5,5007) { square[1].WriteReg2( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(SNDMMC5,5010) { pcm.WriteReg0( data ); }
NES_POKE(SNDMMC5,5011) { pcm.WriteReg1( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(SNDMMC5,5015) 
{ 
	square[0].Toggle( data & SQUARE_0_ENABLE );       
	square[1].Toggle( data & SQUARE_1_ENABLE );       
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(SNDMMC5,5205) { return ((value[0] * value[1]) & 0x00FF) >> 0; }
NES_PEEK(SNDMMC5,5206) { return ((value[0] * value[1]) & 0xFF00) >> 8; }
NES_POKE(SNDMMC5,5205) { value[0] = data; }
NES_POKE(SNDMMC5,5206) { value[1] = data; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(SNDMMC5,Nop)
{
	return cpu->GetCache();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDMMC5::SQUARE::LoadState(PDXFILE& file)
{
	PDX_TRY(CHANNEL::LoadState(file));

	{
		HEADER header;

		if (!file.Read(header))
			return PDX_FAILURE;

		step			= header.step;
		DutyPeriod		= header.DutyPeriod;
		EnvCount		= header.EnvCount;
		EnvRate		    = header.EnvRate;
		EnvDecayRate	= header.EnvDecayRate;
		EnvDecayDisable = header.EnvDecayDisable ? 1 : 0;
		EnvDecayLoop	= header.EnvDecayLoop;
		WaveLengthLow	= header.WaveLengthLow;
		WaveLengthHigh	= header.WaveLengthHigh << 8;
	}

	UpdateVolume();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDMMC5::SQUARE::SaveState(PDXFILE& file) const
{
	PDX_TRY(CHANNEL::SaveState(file));

	HEADER header;

	header.step			   = step;
	header.DutyPeriod	   = DutyPeriod;
	header.EnvCount		   = EnvCount;
	header.EnvRate		   = EnvRate;
	header.EnvDecayRate	   = EnvDecayRate;
	header.EnvDecayDisable = EnvDecayDisable ? 1 : 0;
	header.EnvDecayLoop	   = EnvDecayLoop;
	header.WaveLengthLow   = WaveLengthLow;
	header.WaveLengthHigh  = WaveLengthHigh >> 8;

	file << header;

	return PDX_OK;}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDMMC5::SQUARE::Reset()
{
	APU::CHANNEL::Reset();

	step = 0;
	EnvCount = 0;
	EnvRate = 1;
	DutyPeriod = 2;

	EnvDecayDisable = 0;
	EnvDecayRate = 0;
	EnvDecayLoop = 0;

	WaveLengthLow = 0;
	WaveLengthHigh = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDMMC5::SQUARE::Toggle(const BOOL state)
{
	if (!(enabled = state)) 
	{
		active = 0;
		LengthCounter = 0;
		timer = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDMMC5::SQUARE::WriteReg0(const UINT data)
{
	EnvDecayRate    = data & ENV_DECAY;
	EnvDecayDisable = data & ENV_DECAY_DISABLE;
	EnvDecayLoop    = (data & ENV_DECAY_LOOP_ENABLE) ? 0xF : 0x0;
	EnvRate         = EnvDecayRate + 1;

	UpdateVolume();

	static const UCHAR duties[4] = {2,4,8,12};
	DutyPeriod = duties[data >> DUTY_SHIFT];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDMMC5::SQUARE::WriteReg1(const UINT data)
{
	WaveLengthLow = data;
	frequency     = NES_APU_TO_FIXED(WaveLengthLow + WaveLengthHigh + 1);
	active        = emulate && enabled && LengthCounter && frequency >= NES_APU_TO_FIXED(0x4);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDMMC5::SQUARE::WriteReg2(const UINT data)
{
	step = 0;
	EnvCount = 0xF;

	UpdateVolume();	

	WaveLengthHigh = (data & WAVE_LENGTH_HIGH) << 8;
	LengthCounter  = LengthTable[data >> LENGTH_COUNTER_SHIFT];
	frequency      = NES_APU_TO_FIXED(WaveLengthLow + WaveLengthHigh + 1);
	active         = emulate && enabled && LengthCounter && frequency >= NES_APU_TO_FIXED(0x4);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDMMC5::SQUARE::UpdateVolume()
{
	volume = NES_APU_OUTPUT(EnvDecayDisable ? EnvDecayRate : EnvCount);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDMMC5::SQUARE::UpdateQuarter()
{
	if (!--EnvRate)
	{
		EnvRate = EnvDecayRate + 1;
		EnvCount = EnvCount ? (EnvCount-1) : EnvDecayLoop;
		UpdateVolume();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDMMC5::SQUARE::UpdateWhole()
{
	if (!EnvDecayLoop && !--LengthCounter)
		active = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG SNDMMC5::SQUARE::Sample()
{
	if (active)
	{
		for (timer += rate; timer > 0; timer -= frequency)
		{
			step = (step + 1) & 0xF;

			if (!step) 
			{
				amp = +volume;
			}
			else if (step == DutyPeriod)
			{
				amp = -volume;
			}
		}
	}
	else
	{
		amp -= (amp >> 7);
	}

	return amp;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDMMC5::PCM::WriteReg0(const UINT data)
{
	active = (data & PCM_ENABLE) && emulate;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDMMC5::PCM::WriteReg1(const UINT data)
{
	amp = NES_APU_OUTPUT(data ^ 0x80);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG SNDMMC5::PCM::Sample()
{
	if (!active)
		amp -= (amp >> 7);

	return amp;
}

NES_NAMESPACE_END
