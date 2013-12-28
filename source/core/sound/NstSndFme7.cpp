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
#include "NstSndFme7.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDFME7::SNDFME7(CPU* const cpu)
: apu(cpu->GetAPU())
{
	apu->HookChannel( PDX_STATIC_CAST(APU::CHANNEL*,&channel)  );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDFME7::~SNDFME7()
{
	apu->ReleaseChannel( PDX_STATIC_CAST(APU::CHANNEL*,&channel) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::Reset()
{
	channel.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::LoadState(PDXFILE& file)       
{
	return channel.LoadState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::SaveState(PDXFILE& file) const
{ 
	return channel.SaveState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::Poke_C000(const UINT data)
{ 
	apu->Update(); 
	channel.WriteReg0( data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::Poke_E000(const UINT data)
{ 
	apu->Update(); 
	channel.WriteReg1( data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::CHANNEL::Reset()
{
	address = 0x00;

	square[0].Reset( pal );
	square[1].Reset( pal );
	square[2].Reset( pal );
	envelope.Reset( pal );
	noise.Reset( pal );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::FME7CHANNEL::Reset(const BOOL pal)
{
	active    = TRUE;
	frequency = NES_APU_TO_FIXED(1UL << 3);
	timer     = 0;
	amp       = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::FME7CHANNEL::LoadState(PDXFILE& file)
{
	if (file.Readable((sizeof(U32) * 2) + sizeof(I32)))
	{
		frequency = file.Read<U32>();
		timer     = file.Read<I32>();
		amp       = file.Read<I32>();

		return PDX_OK;
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::FME7CHANNEL::SaveState(PDXFILE& file) const
{
	file << U32( frequency );
	file << I32( timer     );
	file << I32( amp       );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::CHANNEL::LoadState(PDXFILE& file)       
{
	PDX_TRY( APU::CHANNEL::LoadState(file));

	PDX_TRY( square[0].LoadState(file) );
	PDX_TRY( square[1].LoadState(file) );
	PDX_TRY( square[2].LoadState(file) );
	PDX_TRY( envelope.LoadState(file)  );

	return noise.LoadState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::CHANNEL::SaveState(PDXFILE& file) const
{ 
	PDX_TRY( APU::CHANNEL::SaveState(file));

	PDX_TRY( square[0].SaveState(file) );
	PDX_TRY( square[1].SaveState(file) );
	PDX_TRY( square[2].SaveState(file) );
	PDX_TRY( envelope.SaveState(file)  );

	return noise.SaveState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::CHANNEL::WriteReg0(const UINT data)
{ 
	address = data % 0xE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::CHANNEL::WriteReg1(const UINT data)
{ 
	switch (address)
	{
     	case 0x0: square[0].WriteReg0( data, pal ); return;
		case 0x1: square[0].WriteReg1( data, pal ); return;
		case 0x2: square[1].WriteReg0( data, pal ); return;
		case 0x3: square[1].WriteReg1( data, pal ); return;
		case 0x4: square[2].WriteReg0( data, pal ); return;
		case 0x5: square[2].WriteReg1( data, pal ); return;		
		
		case 0x6: 
			
			noise.WriteReg( data, pal ); 
			return;		
		
		case 0x7: 
			
			square[0].WriteReg2( data >> 0 );
			square[1].WriteReg2( data >> 1 );
			square[2].WriteReg2( data >> 2 );
			return;

		case 0x8: square[0].WriteReg3( data ); return;
		case 0x9: square[1].WriteReg3( data ); return;
		case 0xA: square[2].WriteReg3( data ); return;

		case 0xB: envelope.WriteReg0( data, pal ); return;
		case 0xC: envelope.WriteReg1( data, pal ); return;
		case 0xD: envelope.WriteReg2( data      ); return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG SNDFME7::CHANNEL::Sample()
{
	if (!emulate)
		return 0;
	
	const LONG AnyNoise = noise.Update( pal );
	const LONG EnvVol = envelope.Update( pal );

	LONG sample;

	sample  = square[0].Sample( rate, EnvVol, AnyNoise );
	sample += square[1].Sample( rate, EnvVol, AnyNoise );
	sample += square[2].Sample( rate, EnvVol, AnyNoise );

	return sample;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::SQUARE::Reset(const BOOL pal)
{
	FME7CHANNEL::Reset(pal);

	FreqLo   = 1;
	FreqHi   = 0;
	flags    = 0;
	volume   = 0;
	FlipFlop = 0;
	UseEnv   = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::SQUARE::LoadState(PDXFILE& file)
{
	PDX_TRY(FME7CHANNEL::LoadState(file));

	if (!file.Readable(sizeof(U8) * 9))
		return PDX_FAILURE;

	FreqLo   = file.Read<U8>() << 0;
	FreqHi   = file.Read<U8>() << 8;
	flags    = file.Read<U8>();
	volume   = file.Read<U32>();
	FlipFlop = file.Read<U8>();
	UseEnv   = file.Read<U8>();

	active = frequency && (UseEnv || volume);

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::SQUARE::SaveState(PDXFILE& file) const
{
	PDX_TRY(FME7CHANNEL::SaveState(file));

    file << U8  ( FreqLo >> 0      );
 	file << U8  ( FreqHi >> 8      );  
	file << U8  ( flags            );
	file << U32 ( volume           );
	file << U8  ( FlipFlop ? 1 : 0 );
	file << U8  ( UseEnv ? 1 : 0   );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::SQUARE::WriteReg0(const UINT data,const BOOL pal)
{
	FreqLo = data + 1;
	frequency = NES_APU_TO_FIXED(ULONG(FreqLo + FreqHi) << 3);
	active = frequency && (UseEnv || volume);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::SQUARE::WriteReg1(const UINT data,const BOOL pal)
{
	FreqHi = data << 8;
	frequency = NES_APU_TO_FIXED(ULONG(FreqLo + FreqHi) << 3);
	active = frequency && (UseEnv || volume);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::SQUARE::WriteReg2(const UINT data)
{
	flags = (data & (SOUND_ENVELOPE|SOUND_NOISE)) ^ (SOUND_ENVELOPE|SOUND_NOISE);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::SQUARE::WriteReg3(const UINT data)
{
	if (!(UseEnv = (data & 0x10)))
	{
		volume = NES_APU_OUTPUT(data & 0xF);
		active = frequency && volume;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG SNDFME7::SQUARE::Sample(const ULONG rate,const LONG EnvVol,const LONG AnyNoise)
{
	LONG output = 0;

	if (active)
	{
		for (timer += rate; timer > 0; timer -= frequency)
			FlipFlop ^= 1;

		if (flags)
		{
			if (UseEnv)
				volume = EnvVol;

			if (flags & SOUND_ENVELOPE) output = (FlipFlop ? +volume : -volume);
			if (flags & SOUND_NOISE)    output = (AnyNoise ? +volume : -volume);
		}
	}

	return output >> 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::ENVELOPE::Reset(const BOOL pal)
{
	FME7CHANNEL::Reset(pal);

	FreqLo = 1;
	FreqHi = 0;

	WriteReg2( 0 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::ENVELOPE::LoadState(PDXFILE& file)
{
	PDX_TRY(FME7CHANNEL::LoadState(file));

	if (!file.Readable(sizeof(U8) * 3))
		return PDX_FAILURE;

	FreqLo = file.Read<U8>() << 0;
	FreqHi = file.Read<U8>() << 8;
	
	WriteReg2( file.Read<U8>() );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::ENVELOPE::SaveState(PDXFILE& file) const
{
	PDX_TRY(FME7CHANNEL::SaveState(file));

	file << U8( FreqLo >> 0);
	file << U8( FreqHi >> 8);
	file << U8( TableIndex );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::ENVELOPE::WriteReg0(const UINT data,const BOOL pal)
{
	FreqLo = data + 1;
	frequency = NES_APU_TO_FIXED(ULONG(FreqLo + FreqHi) << 3);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::ENVELOPE::WriteReg1(const UINT data,const BOOL pal)
{
	FreqHi = data << 8;
	frequency = NES_APU_TO_FIXED(ULONG(FreqLo + FreqHi) << 3);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::ENVELOPE::WriteReg2(const UINT data)
{
	static const I8 pulse[] = 
	{
		0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
		0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
		0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
		0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,+00,
	};

	static const I8 PulseHold[] = 
	{
		0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
		0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
		0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
		0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,+02,
		0x1F,+00,
	};

	static const I8 SawTooth[] = 
	{
		0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
		0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
		0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
		0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,-62,
	};

	static const I8 triangle[] = 
	{
		0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
		0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
		0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
		0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,+02,
		0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
		0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
		0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
		0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,-126,
	};

	static const I8 xPulse[] = 
	{
		0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
		0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
		0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
		0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,+00,
	};

	static const I8 xPulseHold[] = 
	{
		0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
		0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
		0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
		0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,+02,
		0x00,+00,
	};

	static const I8 xSawTooth[] = 
	{
		0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
		0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
		0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
		0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,-62,
	};

	static const I8 xTriangle[] = 
	{
		0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
		0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
		0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
		0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,+02,
		0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
		0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
		0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
		0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,-126,
	};

	static const I8* oscilators[16] =
	{
		pulse,	   pulse,	pulse,	   pulse,
		xPulse,	   xPulse,	xPulse,	   xPulse,
		SawTooth,  pulse,	triangle,  PulseHold,
		xSawTooth, xPulse,	xTriangle, xPulseHold,
	};

	TableIndex = data & 0xF;
	oscilator = oscilators[TableIndex];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG SNDFME7::ENVELOPE::Update(const ULONG rate)
{
	if (frequency)
	{
		for (timer += rate; timer > 0; timer -= frequency)
			oscilator += oscilator[1];

		amp = NES_APU_OUTPUT(oscilator[0] & 0x1F);
	}
	else
	{
		amp -= (amp >> 7);
	}

	return amp >> 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::NOISE::Reset(const BOOL pal)
{
	FME7CHANNEL::Reset(pal);
	
	ring = 1;
	amp = NES_APU_OUTPUT(0xFF);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::NOISE::LoadState(PDXFILE& file)
{
	PDX_TRY(FME7CHANNEL::LoadState(file));

	if (!file.Readable(sizeof(U8)))
		return PDX_FAILURE;

	ring = file.Read<U8>();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDFME7::NOISE::SaveState(PDXFILE& file) const
{
	PDX_TRY(FME7CHANNEL::SaveState(file));

	file << U8(ring);

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDFME7::NOISE::WriteReg(const UINT data,const BOOL pal)
{
	frequency = NES_APU_TO_FIXED(ULONG((data & 0x1F) + 1) << 3);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG SNDFME7::NOISE::Update(const ULONG rate)
{
	if (frequency)
	{
		for (timer += rate; timer > 0; timer -= frequency)
		{
			if ((ring + 1) & 0x2) amp = ~amp;
			if ((ring + 0) & 0x1) ring ^= 0x28000UL;

			ring >>= 1;
		}

		return amp;
	}

	return 0;
}

NES_NAMESPACE_END
