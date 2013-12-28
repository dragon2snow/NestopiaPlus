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
#include "NstSndFds.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDFDS::SNDFDS(CPU& c)
: 
cpu (c),
apu (c.GetAPU())
{
	apu.HookChannel( PDX_STATIC_CAST(APU::CHANNEL*,&channel) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDFDS::~SNDFDS()
{
	apu.ReleaseChannel( PDX_STATIC_CAST(APU::CHANNEL*,&channel) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFDS::Reset()
{
	cpu.SetPort( 0x4040, 0x407F, this, Peek_4040, Poke_4040 );
	cpu.SetPort( 0x4080,         this, Peek_bad,  Poke_4080 );
	cpu.SetPort( 0x4082,         this, Peek_bad,  Poke_4082 );
	cpu.SetPort( 0x4083,         this, Peek_bad,  Poke_4083 );
	cpu.SetPort( 0x4084,         this, Peek_bad,  Poke_4084 );
	cpu.SetPort( 0x4085,         this, Peek_bad,  Poke_4085 );
	cpu.SetPort( 0x4086,         this, Peek_bad,  Poke_4086 );
	cpu.SetPort( 0x4087,         this, Peek_bad,  Poke_4087 );
	cpu.SetPort( 0x4088,         this, Peek_bad,  Poke_4088 );
	cpu.SetPort( 0x4089,         this, Peek_bad,  Poke_4089 );
	cpu.SetPort( 0x408A,         this, Peek_bad,  Poke_408A );
	cpu.SetPort( 0x4090,         this, Peek_4090, Poke_bad  );
	cpu.SetPort( 0x4092,         this, Peek_4092, Poke_bad  );

	channel.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFDS::LoadState(PDXFILE& file)       { return channel.LoadState( file ); }
PDXRESULT SNDFDS::SaveState(PDXFILE& file) const { return channel.SaveState( file ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(SNDFDS,bad) {}
NES_PEEK(SNDFDS,bad) { return cpu.GetCache(); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(SNDFDS,4040) 
{ 
	apu.Update(); 
	return channel.ReadWave( address - 0x4040 ) | (cpu.GetCache() & 0xC0); 
}

NES_POKE(SNDFDS,4040) 
{ 
	apu.Update(); 
	channel.WriteWave( address - 0x4040, data ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(SNDFDS,4080) { apu.Update(); channel.WriteReg0( data ); }
NES_POKE(SNDFDS,4082) { apu.Update(); channel.WriteReg2( data ); }
NES_POKE(SNDFDS,4083) { apu.Update(); channel.WriteReg3( data ); }
NES_POKE(SNDFDS,4084) { apu.Update(); channel.WriteReg4( data ); }
NES_POKE(SNDFDS,4085) { apu.Update(); channel.WriteReg5( data ); }
NES_POKE(SNDFDS,4086) { apu.Update(); channel.WriteReg6( data ); }
NES_POKE(SNDFDS,4087) { apu.Update(); channel.WriteReg7( data ); }
NES_POKE(SNDFDS,4088) { apu.Update(); channel.WriteReg8( data ); }
NES_POKE(SNDFDS,4089) { apu.Update(); channel.WriteReg9( data ); }
NES_POKE(SNDFDS,408A) { apu.Update(); channel.WriteRegA( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(SNDFDS,4090) 
{ 
	apu.Update(); 
	return channel.ReadWaveAmp() | (cpu.GetCache() & 0xC0); 
}

NES_PEEK(SNDFDS,4092) 
{ 
	apu.Update(); 
	return channel.ReadModAmp() | (cpu.GetCache() & 0xC0); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFDS::CHANNEL::Reset()
{
	APU::CHANNEL::Reset();

	WaveAmp         = 0;
	ModAmp          = 0;
	EnvCount		= 0;
	EnvLoad			= 0;
	FadeEnable		= 1;
	VolumeImmediate	= 0;
	VolumeFadeIn	= 0;
	VolumeFadeSpeed	= 0;
	VolumeFadeCount = 0;
	EffectImmediate = 0;
	EffectFadeIn    = 0;
	EffectFadeSpeed	= 0;
	EffectFadeCount	= 0;
	InFrequency.d   = 0;
	LfoSpeed.d		= 0;
	LfoSpeedReset	= 0;
	WaveShifter1	= 0;
	WaveShifter2	= 0;
	WaveAdder		= 0;
	WaveLatch		= 0;
	WavePos			= 0;
	WaveCount		= 0;
	sweep			= 0;
	reverb			= 0;
	FlipFlop		= 1;

	memset( mWave, 0x00, sizeof(U8) * 0x20 );
	memset( cWave, 0x00, sizeof(U8) * 0x40 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFDS::CHANNEL::LoadState(PDXFILE& file)
{
	return file.Read( PDX_CAST(CHAR*,&WaveAmp), PDX_CAST(CHAR*,cWave+0x40) ) ? PDX_OK : PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFDS::CHANNEL::SaveState(PDXFILE& file) const
{
	file.Write( PDX_CAST(const CHAR*,&WaveAmp), PDX_CAST(const CHAR*,cWave+0x40) );
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteReg0(const UINT data) 
{ 
	VolumeFadeSpeed = data & VOLUME_FADE_SPEED;
	VolumeFadeIn = data & VOLUME_FADE_IN;

	if (VolumeImmediate = (data & VOLUME_IMMEDIATE)) 
		WaveAmp = data & VOLUME_AMP; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteReg2(const UINT data) 
{
	InFrequency.b.l = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteReg3(const UINT data) 
{ 
	FadeEnable = !(data & FADE_DISABLE);
	InFrequency.b.h = data & FREQUENCY_HIGH;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteReg4(const UINT data) 
{ 
	EffectFadeSpeed = data & EFFECT_FADE_SPEED;
	EffectFadeIn = data & EFFECT_FADE_IN;

	if (EffectImmediate = (data & EFFECT_IMMEDIATE)) 
		ModAmp = data & EFFECT_AMP; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteReg5(const UINT data) 
{ 
	sweep = data & EFFECT_SWEEP; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteReg6(const UINT data) 
{
	LfoSpeed.b.l = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteReg7(const UINT data)
{
	LfoSpeed.b.h = data & LFO_SPEED_HIGH;
	LfoSpeedReset = data & LFO_SPEED_RESET;
	WaveLatch = 0;
	sweep = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteReg8(const UINT data)
{
	mWave[sweep++] = data & LFO_MOD;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteReg9(const UINT data) 
{ 
	reverb = (data & EFFECT_REVERB) + 2;
	active = !(data & SOUND_DISABLE); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteRegA(const UINT data) 
{ 
	EnvLoad = data * 2;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDFDS::CHANNEL::WriteWave(const UINT pos,const INT data)
{
	if (!active)
		cWave[pos] = data & WAVE_DATA;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline UINT SNDFDS::CHANNEL::ReadWave(const UINT pos) const
{
	return cWave[pos];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline UINT SNDFDS::CHANNEL::ReadWaveAmp() const 
{ 
	return WaveAmp; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline UINT SNDFDS::CHANNEL::ReadModAmp()  const 
{ 
	return ModAmp;  
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFDS::CHANNEL::UpdateEnvelope()
{
	EnvCount += EnvLoad;

	if (FadeEnable)
	{
		if (!VolumeImmediate && VolumeFadeCount-- <= 0)
		{
			VolumeFadeCount = VolumeFadeSpeed;

			if (VolumeFadeIn) { if (WaveAmp < 0x3F) ++WaveAmp; }
			else              { if (WaveAmp > 0x00) --WaveAmp; }
		}

		if (!EffectImmediate && EffectFadeCount-- <= 0)
		{
			EffectFadeCount = EffectFadeSpeed;

			if (EffectFadeIn) { if (ModAmp < 0x3F) ++ModAmp; }
			else              { if (ModAmp > 0x00) --ModAmp; }
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG SNDFDS::CHANNEL::Sample()
{
	if (active)
	{
		ULONG i = 0;
		LONG  a = 0;

		const ULONG cycle = NES_APU_TO_FIXED(1);

		for (timer += rate; timer > 0; timer -= cycle, ++i)
		{
			if (FlipFlop ^= 1)
			{
				if (!WaveCount++)
				{
					WaveShifter1 = InFrequency.d;
					WaveLatch += LfoSpeed.d;
	
					if (LfoSpeedReset)
					{		
						WaveShifter2 = 0x80;
					}
					else  
					{ 
						const UINT t = mWave[(WaveLatch >> 13) & 0x1F];
						WaveShifter2 = (t & 0x3) * (ModAmp & 0x1F);
						WaveShifter2 = (t & 0x4) ? 0x80 - WaveShifter2 : 0x80 + WaveShifter2;
					}
				}
				else
				{
					WaveShifter1 <<= 1;  
					WaveShifter2 >>= 1;
				}
	
				WaveAdder = (WavePos + WaveShifter1) & 0x1FFFFFFUL;
	
				if (WaveShifter2 & 0x1)
					WavePos = WaveAdder;
										 
				if (--EnvCount <= 0)
					UpdateEnvelope();
			}

			a += (cWave[WavePos >> 19] * PDX_MIN(WaveAmp,0x20)) * 4 / reverb;
		}

		amp = (a / i) << 1;
	}
	else
	{
		amp -= (amp >> 7);
	}

	return amp;
}

NES_NAMESPACE_END
