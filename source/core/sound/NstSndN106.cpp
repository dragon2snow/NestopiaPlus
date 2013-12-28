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
#include "NstSndN106.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDN106::SNDN106(CPU& c)
: apu(c.GetAPU())
{
	for (UINT i=0; i < NUM_CHANNELS; ++i)
	{
		channels[i] = new CHANNEL(this,i);
		apu.HookChannel( PDX_STATIC_CAST(APU::CHANNEL*,channels[i]) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDN106::~SNDN106()
{
	for (UINT i=0; i < NUM_CHANNELS; ++i)
	{
		apu.ReleaseChannel( PDX_STATIC_CAST(APU::CHANNEL*,channels[i]) );
		delete channels[i];
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDN106::Reset()
{
	address = 0x00;
	AddressIncrease = 1;
	NumChannels = NUM_CHANNELS;

	PDXMemZero( ExRam, 0x80 );
	PDXMemZero( tone, 0x100 );

	for (UINT i=0; i < NUM_CHANNELS; ++i)
		channels[i]->Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDN106::LoadState(PDXFILE& file)       
{
	if (!file.Readable(sizeof(U8) * (0x80 + 0x100 + 0x4)))
		return PDX_FAILURE;

	file.Read( ExRam, ExRam + 0x80 );
	file.Read( tone,  tone + 0x100 );

	address         = file.Read<U16>();
	AddressIncrease = file.Read<U8>();
	NumChannels     = file.Read<U8>();

	for (UINT i=0; i < NUM_CHANNELS; ++i)
		PDX_TRY(channels[i]->LoadState(file));

	return PDX_OK; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDN106::SaveState(PDXFILE& file) const
{ 
	file.Write( ExRam, ExRam + 0x80 );
	file.Write( tone,  tone + 0x100 );
	
	file << U16( address         );
	file <<  U8( AddressIncrease );
	file <<  U8( NumChannels     );

	for (UINT i=0; i < NUM_CHANNELS; ++i)
		PDX_TRY(channels[i]->SaveState(file));

	return PDX_OK; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT SNDN106::Peek_4800()
{
	apu.Update(); 

	const UINT data = ExRam[address];
	address = (address + AddressIncrease) & ADDRESS_MAX;
	
	return data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDN106::Poke_4800(const UINT data)
{ 
	apu.Update(); 

	ExRam[address] = data;
	tone[(address << 1) + 0] = ((data & 0xF) << 2) - 0x20;
	tone[(address << 1) + 1] = ((data >>  4) << 2) - 0x20;

	if (address >= 0x40)
	{
		const UINT index = (address - 0x40) >> 3;

		switch (address & 0x7)
		{
       		case 0x0: channels[index]->WriteReg0( data ); break;
			case 0x2: channels[index]->WriteReg2( data ); break;
			case 0x4: channels[index]->WriteReg4( data ); break;
			case 0x6: channels[index]->WriteReg6( data ); break;
			case 0x7: channels[index]->WriteReg7( data ); 
				
				if (index == 7)
					NumChannels = 1 + ((data >> 4) & 0x7);

				break;
		}

		for (UINT i=0; i < NUM_CHANNELS; ++i)
			channels[i]->Update();
	}

	address = (address + AddressIncrease) & ADDRESS_MAX;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDN106::Poke_F800(const UINT data)
{ 
	apu.Update(); 
	address = data & ADDRESS_MAX;
	AddressIncrease = (data & ADDRESS_INC) ? 1 : 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDN106::CHANNEL::CHANNEL(SNDN106* const m,const UINT c)
: mother(m), channel(c) {}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDN106::CHANNEL::Reset()
{
	APU::CHANNEL::Reset();
	
	address    = 0x00;
	volume     = 0;
	WaveLength = 0x10UL << PHASE_SHIFT;
	phase      = 0;
	speed      = 0;

	UpdateContext();
	Update();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDN106::CHANNEL::UpdateContext()
{
	IO::SFX::CONTEXT context;
	mother->apu.GetContext( context );
	rate = ULONG((NES_MASTER_CLOCK_HZ_REAL_NTSC / (45.0L * context.SampleRate)) * (1UL << SPEED_SHIFT));
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDN106::CHANNEL::LoadState(PDXFILE& file)
{
	PDX_TRY(APU::CHANNEL::LoadState(file));

	if (!file.Readable(sizeof(U8) + sizeof(U16) + (sizeof(U32) * 4)))
		return PDX_FAILURE;

	address      = file.Read<U8>();
	volume       = file.Read<U16>();
	speed        = file.Read<U32>();
	WaveLength   = file.Read<U32>();
	phase        = file.Read<U32>();
	ChannelSpeed = file.Read<U32>();

	Update();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDN106::CHANNEL::SaveState(PDXFILE& file) const
{
	PDX_TRY(APU::CHANNEL::SaveState(file));

	file <<	 U8( address      );
	file << U16( volume       );
	file << U32( speed        );
	file << U32( WaveLength   );
	file << U32( phase        );
	file << U32( ChannelSpeed );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDN106::CHANNEL::WriteReg0(const UINT data) 
{ 
	frequency = (frequency & 0xFFFFFF00UL) | data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDN106::CHANNEL::WriteReg2(const UINT data) 
{
	frequency = (frequency & 0xFFFF00FFUL) | (data << 8);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDN106::CHANNEL::WriteReg4(const UINT data) 
{ 
	speed = (frequency & 0x0000FFFFUL) | ((data & 0x3) << 16);
	
	const ULONG length = (0x20 - (data & WAVE_LENGTH)) << PHASE_SHIFT;

	if (WaveLength != length)
	{
		WaveLength = length;
		phase = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDN106::CHANNEL::WriteReg6(const UINT data) 
{
	address = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDN106::CHANNEL::WriteReg7(const UINT data)
{
	volume = (data & 0xF) << 4;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDN106::CHANNEL::Update()
{
	active = volume && speed && (channel >= (NUM_CHANNELS - mother->NumChannels)) && emulate;
	ChannelSpeed = mother->NumChannels << SPEED_SHIFT;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG SNDN106::CHANNEL::Sample()
{
	if (active)
	{
		for (timer += rate; timer >= ChannelSpeed; timer -= ChannelSpeed)
			phase += speed;

		while (phase >= WaveLength)
			phase -= WaveLength;

		amp = mother->tone[((phase >> PHASE_SHIFT) + address) & 0xFF] * volume;
	}
	else
	{
		amp -= (amp >> 7);
	}

	return amp;
}

NES_NAMESPACE_END
