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

#ifndef NST_SNDFDS_H
#define NST_SNDFDS_H

#include "../NstApu.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class SNDFDS
{
public:

	SNDFDS(CPU&);
	~SNDFDS();

	VOID Reset();

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;

private:

	NES_DECL_PEEK(bad);
	NES_DECL_POKE(bad);

	NES_DECL_PEEK(4040);
	NES_DECL_POKE(4040);
	NES_DECL_POKE(4080);
	NES_DECL_POKE(4082);
	NES_DECL_POKE(4083);
	NES_DECL_POKE(4084);
	NES_DECL_POKE(4085);
	NES_DECL_POKE(4086);
	NES_DECL_POKE(4087);
	NES_DECL_POKE(4088);
	NES_DECL_POKE(4089);
	NES_DECL_POKE(408A);
	NES_DECL_PEEK(4090);
	NES_DECL_PEEK(4092);

	class CHANNEL : public APU::CHANNEL
	{
	public:

		VOID Reset();

		VOID WriteReg0(const UINT);
		VOID WriteReg2(const UINT);
		VOID WriteReg3(const UINT);
		VOID WriteReg4(const UINT);
		VOID WriteReg5(const UINT);
		VOID WriteReg6(const UINT);
		VOID WriteReg7(const UINT);
		VOID WriteReg8(const UINT);
		VOID WriteReg9(const UINT);
		VOID WriteRegA(const UINT);
		
		UINT ReadWaveAmp() const;
		UINT ReadModAmp()  const;

		VOID WriteWave (const UINT,const INT);
		UINT ReadWave  (const UINT) const;

		PDXRESULT LoadState(PDXFILE&);
		PDXRESULT SaveState(PDXFILE&) const;

	private:

		VOID UpdateEnvelope();
		LONG Sample();

		enum
		{
			VOLUME_FADE_SPEED = 0x3F,
			VOLUME_FADE_IN    = 0x40,
			VOLUME_IMMEDIATE  = 0x80,
			VOLUME_AMP        = 0x3F,
			EFFECT_FADE_SPEED = 0x3F,
			EFFECT_FADE_IN    = 0x40,
			EFFECT_IMMEDIATE  = 0x80,
			EFFECT_AMP        = 0x3F,
			EFFECT_REVERB     = 0x03,
			EFFECT_SWEEP      = 0x1F,
			FADE_DISABLE      = 0x40,
			LFO_MOD           = 0x07,
			LFO_SPEED_HIGH    = 0x03,
			LFO_SPEED_RESET   = 0x80,
			WAVE_DATA         = 0x3F,
			FREQUENCY_HIGH    = 0x0F,
			SOUND_DISABLE     = 0x80
		};

		UINT    WaveAmp;
		UINT    ModAmp;
		INT     EnvCount;
		UINT    EnvLoad;
		UINT    FadeEnable;
		UINT    VolumeImmediate;
		UINT    VolumeFadeIn;
		UINT    VolumeFadeSpeed;
		INT     VolumeFadeCount;
		UINT    EffectImmediate;
		UINT    EffectFadeIn;
		UINT    EffectFadeSpeed;
		INT     EffectFadeCount;
		PDXWORD InFrequency;
		PDXWORD LfoSpeed;
		BOOL    LfoSpeedReset;
		UINT    reverb;
		UINT    FlipFlop;
		ULONG   WaveShifter1;
		UINT    WaveShifter2;
		ULONG   WaveAdder;
		ULONG   WaveLatch;
		ULONG   WavePos;
		UINT    WaveCount : 3;
		UINT    sweep : 5;
		U8      mWave[0x20];
		U8      cWave[0x40];
	};

	CPU& cpu;
	APU& apu;

	CHANNEL channel;
};

NES_NAMESPACE_END

#endif
