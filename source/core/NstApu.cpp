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
#include "NstApu.h"
#include "NstCpu.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// nice macros
////////////////////////////////////////////////////////////////////////////////////////

#define NES_APU_BUFFER_SIZE    0x10000UL
#define NES_APU_BUFFER_MASK    (NES_APU_BUFFER_SIZE-1)
#define NES_APU_BUFFER_TO_8(x) (((x) >> 8) ^ 0x80)

#define NES_TRIANGLE_MIN_FREQUENCY NES_APU_TO_FIXED(0x4UL)
#define NES_SQUARE_MIN_FREQUENCY   NES_APU_TO_FIXED(0x8UL)

////////////////////////////////////////////////////////////////////////////////////////
// length counter conversion table
////////////////////////////////////////////////////////////////////////////////////////

const UCHAR APU::CHANNEL::LengthTable[32] = 
{
	0x05, 0x7F, 0x0A, 0x01, 0x14, 0x02, 0x28, 0x03, 
	0x50, 0x04, 0x1E, 0x05, 0x07, 0x06, 0x0D, 0x07, 
	0x06, 0x08, 0x0C, 0x09, 0x18, 0x0A, 0x30, 0x0B, 
	0x60, 0x0C, 0x24, 0x0D, 0x08, 0x0E, 0x10, 0x0F
};

////////////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////////////

APU::CYCLES::CYCLES()
{
	Reset( 44100, FALSE );
}

////////////////////////////////////////////////////////////////////////////////////////
// Reset
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::CYCLES::Reset(const UINT TargetRate,const BOOL pal)
{
	elapsed = 0;
	RateCounter = 0;
	PhaseIndex = 0;

	if (pal)
	{
		rate         = ULONG(ceil(DOUBLE(NES_MASTER_CLOCK_HZ_PAL) / DOUBLE(TargetRate)));
		FrameInit    =-NES_CPU_PAL_FIXED * RESET_CYCLES;
		FrameCounter = NES_CPU_PAL_FIXED * RESET_CYCLES;
		QuarterFrame = NES_MASTER_CC_QUARTER_FRAME_PAL;
	}
	else
	{
		rate         = ULONG(ceil(DOUBLE(NES_MASTER_CLOCK_HZ_NTSC) / DOUBLE(TargetRate)));
		FrameInit    =-NES_CPU_NTSC_FIXED * RESET_CYCLES;
		FrameCounter = NES_CPU_NTSC_FIXED * RESET_CYCLES;
		QuarterFrame = NES_MASTER_CC_QUARTER_FRAME_NTSC;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// Sound Buffer Constructor
////////////////////////////////////////////////////////////////////////////////////////

APU::BUFFER::BUFFER() 
: output(new I16[NES_APU_BUFFER_SIZE])
{ 
	Reset(16); 
}

////////////////////////////////////////////////////////////////////////////////////////
// Sound Buffer Destructor
////////////////////////////////////////////////////////////////////////////////////////

APU::BUFFER::~BUFFER()
{ 
	delete [] output; 
}

////////////////////////////////////////////////////////////////////////////////////////
// Reset Sound Buffer
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::BUFFER::Reset(const UINT bits,const BOOL discard)
{
	pos = start = 0;
	Bit16 = (bits == 16);

	if (discard)
		PDXMemZero( output, NES_APU_BUFFER_SIZE );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

TSIZE APU::BUFFER::GetLatency() const
{
	if (pos >= start)
		return pos - start;

	return (NES_APU_BUFFER_SIZE - start) + pos;
}

////////////////////////////////////////////////////////////////////////////////////////
// Write a sample into the buffer
////////////////////////////////////////////////////////////////////////////////////////

inline VOID APU::BUFFER::Write(const LONG sample)
{
	output[pos] = PDX_CLAMP(sample,I16_MIN,I16_MAX);
	pos = (pos + 1) & NES_APU_BUFFER_MASK;
}

////////////////////////////////////////////////////////////////////////////////////////
// Flush the sound buffer
////////////////////////////////////////////////////////////////////////////////////////

TSIZE APU::BUFFER::Flush(IO::SFX* const PDX_RESTRICT stream)
{
	PDX_ASSERT(stream && stream->samples && stream->length <= NES_APU_BUFFER_SIZE);

	LONG available = pos - start;

	if (available < 0)
		available = pos + (NES_APU_BUFFER_SIZE - start);

	const ULONG length = PDX_MIN(available,stream->length);
	const TSIZE NewStart = start + length;

	if (Bit16)
	{
		I16* const PDX_RESTRICT samples = PDX_CAST(I16*,stream->samples);

		if (NewStart > NES_APU_BUFFER_SIZE)
		{
			const TSIZE chunk = NES_APU_BUFFER_SIZE - start;
			memcpy( samples, output + start, sizeof(I16) * chunk );
			memcpy( samples + chunk, output, sizeof(I16) * (NewStart - NES_APU_BUFFER_SIZE) );
		}
		else
		{
			memcpy( samples, output + start, sizeof(I16) * length );
		}
	}
	else
	{
		Flush8( stream, NewStart );
	}

	start = NewStart & NES_APU_BUFFER_MASK;

	return length;
}

////////////////////////////////////////////////////////////////////////////////////////
// Flush the sound buffer (8bit version)
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::BUFFER::Flush8(IO::SFX* const stream,const TSIZE NewStart)
{
	U8* PDX_RESTRICT overhere = PDX_CAST(U8*,stream->samples);
	const I16* const PDX_RESTRICT from = output + start;

	if (NewStart > NES_APU_BUFFER_SIZE)
	{
		TSIZE chunk = NES_APU_BUFFER_SIZE - start;

		for (TSIZE i=0; i < chunk; ++i)
			overhere[i] = NES_APU_BUFFER_TO_8(from[i]);

		overhere += chunk;
		chunk = NewStart - NES_APU_BUFFER_SIZE;

		for (TSIZE i=0; i < chunk; ++i)
			overhere[i] = NES_APU_BUFFER_TO_8(output[i]);
	}
	else
	{
		for (TSIZE i=0; i < stream->length; ++i)
			overhere[i] = NES_APU_BUFFER_TO_8(from[i]);
	}
}


////////////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////////////

APU::APU(CPU& c) 
:
square1        (0),
square2        (1),
noise          (dmc),
triangle       (dmc),
dmc            (c),
cpu            (c),
pal            (FALSE),
SynchronizePtr (SynchronizeOFF)
{}

////////////////////////////////////////////////////////////////////////////////////////
// Switch between PAL/NTSC
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SetMode(const MODE mode)
{
	const BOOL IsPAL = (mode == MODE_PAL);

	if (bool(pal) != bool(IsPAL))
	{
		pal = IsPAL;

		cycles.Reset( emulation.SampleRate, pal );
		buffer.Reset( emulation.SampleBits );

		square1.SetContext  ( cycles.rate, pal, emulation.square1  );
		square2.SetContext  ( cycles.rate, pal, emulation.square2  );
		noise.SetContext    ( cycles.rate, pal, emulation.noise    );
		triangle.SetContext ( cycles.rate, pal, emulation.triangle );
		dmc.SetContext      ( cycles.rate, pal, emulation.dpcm     );

		for (UINT i=0; i < ExtChannels.Size(); ++i)
			ExtChannels[i]->SetContext( cycles.rate, pal, emulation.external );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// Reset the apu
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::Reset()
{
	LogOutput("APU: reset");

	cpu.SetPort( 0x4000, this, Peek_4xxx, Poke_4000 );
	cpu.SetPort( 0x4001, this, Peek_4xxx, Poke_4001 );
	cpu.SetPort( 0x4002, this, Peek_4xxx, Poke_4002 );
	cpu.SetPort( 0x4003, this, Peek_4xxx, Poke_4003 );
	cpu.SetPort( 0x4004, this, Peek_4xxx, Poke_4004 );
	cpu.SetPort( 0x4005, this, Peek_4xxx, Poke_4005 );
	cpu.SetPort( 0x4006, this, Peek_4xxx, Poke_4006 );
	cpu.SetPort( 0x4007, this, Peek_4xxx, Poke_4007 );
	cpu.SetPort( 0x4008, this, Peek_4xxx, Poke_4008 );
	cpu.SetPort( 0x400A, this, Peek_4xxx, Poke_400A );
	cpu.SetPort( 0x400B, this, Peek_4xxx, Poke_400B );
	cpu.SetPort( 0x400C, this, Peek_4xxx, Poke_400C );
	cpu.SetPort( 0x400E, this, Peek_4xxx, Poke_400E );
	cpu.SetPort( 0x400F, this, Peek_4xxx, Poke_400F );
	cpu.SetPort( 0x4010, this, Peek_4xxx, Poke_4010 );
	cpu.SetPort( 0x4011, this, Peek_4xxx, Poke_4011 );
	cpu.SetPort( 0x4012, this, Peek_4xxx, Poke_4012 );
	cpu.SetPort( 0x4013, this, Peek_4xxx, Poke_4013 );

	cycles.Reset( emulation.SampleRate, pal );
	buffer.Reset( emulation.SampleBits );

	square1.SetContext  ( cycles.rate, pal, emulation.square1  ); square1.Reset  ();
	square2.SetContext  ( cycles.rate, pal, emulation.square2  ); square2.Reset  ();
	triangle.SetContext ( cycles.rate, pal, emulation.triangle ); triangle.Reset ();
	noise.SetContext    ( cycles.rate, pal, emulation.noise    ); noise.Reset    ();
	dmc.SetContext      ( cycles.rate, pal, emulation.dpcm     ); dmc.Reset      ();

	for (UINT i=0; i < ExtChannels.Size(); ++i)
	{
		ExtChannels[i]->SetContext( cycles.rate, pal, emulation.external );
		ExtChannels[i]->Reset();
	}

	stream = NULL;
	SynchronizePtr = SynchronizeOFF;
}

////////////////////////////////////////////////////////////////////////////////////////
// Connect an external sound chip channel
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::HookChannel(CHANNEL* const channel)
{
	ExtChannels.InsertBack(channel);
	ExtChannels.Back()->SetContext( cycles.rate, pal, emulation.external );
}

////////////////////////////////////////////////////////////////////////////////////////
// unconnect an external sound chip channel
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::ReleaseChannel(const CHANNEL* const channel)
{
	EXTCHANNELS::ITERATOR ditch = PDX::Find(ExtChannels.Begin(),ExtChannels.End(),channel);
	PDX_ASSERT(ditch);
	ExtChannels.Erase(ditch);
}

////////////////////////////////////////////////////////////////////////////////////////
// output a sample from all channels
////////////////////////////////////////////////////////////////////////////////////////

inline LONG APU::Sample()
{
	LONG sample;

	sample  = square1.Sample();
	sample += square2.Sample();
	sample += triangle.Sample();
	sample += noise.Sample();
	sample += dmc.Sample();

	return sample;
}

inline LONG APU::SampleAll()
{
	LONG sample;

	sample  = square1.Sample();
	sample += square2.Sample();
	sample += triangle.Sample();
	sample += noise.Sample();
	sample += dmc.Sample();

	for (INT i=ExtChannels.Size()-1; i >= 0; --i)
		sample += ExtChannels[i]->Sample();

	return sample;
}

////////////////////////////////////////////////////////////////////////////////////////
// synchronize the APU with the CPU
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::Synchronize()
{
	const ULONG count = cpu.GetCycles<CPU::CYCLE_MASTER>();
	const ULONG frame = cpu.GetFrameCycles<CPU::CYCLE_MASTER>();
	(*this.*SynchronizePtr)(PDX_MIN(count,frame));
}

////////////////////////////////////////////////////////////////////////////////////////
// synchronize the APU with the input cycle count
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SynchronizeON(const ULONG current)
{
	const LONG count = current - cycles.elapsed;
	cycles.elapsed = current;

	for 
	(
     	cycles.RateCounter += count; 
     	cycles.RateCounter > 0; 
		cycles.RateCounter -= cycles.rate, 
		cycles.FrameCounter -= cycles.rate
	)
	{
		if (cycles.FrameCounter <= 0)
			UpdatePhase();

		buffer.Write( Sample() );
	}
}

VOID APU::SynchronizeAllON(const ULONG current)
{
	const LONG count = current - cycles.elapsed;
	cycles.elapsed = current;

	for 
	(
     	cycles.RateCounter += count; 
     	cycles.RateCounter > 0; 
		cycles.RateCounter -= cycles.rate, 
		cycles.FrameCounter -= cycles.rate
	)
	{
		if (cycles.FrameCounter <= 0)
			UpdatePhase();

		buffer.Write( SampleAll() );
	}
}

VOID APU::SynchronizeOFF(const ULONG current)
{
	const LONG count = current - cycles.elapsed;
	cycles.elapsed = current;

	for 
	(
     	cycles.RateCounter += count; 
    	cycles.RateCounter > 0; 
		cycles.RateCounter -= cycles.rate, 
		cycles.FrameCounter -= cycles.rate
	)
	{
		if (cycles.FrameCounter <= 0)
			UpdatePhase();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// Clock the chip(s)
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::UpdatePhase()
{
	cycles.FrameCounter += cycles.QuarterFrame;

	if (cycles.PhaseIndex == 4)
	{
		cycles.PhaseIndex = 0;
		return;
	}

	const BOOL ch0 = square1.IsActive();
	const BOOL ch1 = square2.IsActive();
	const BOOL ch2 = triangle.IsActive();
	const BOOL ch3 = noise.IsActive();

	if (ch0) square1.UpdateQuarter();
	if (ch1) square2.UpdateQuarter();
	if (ch2) triangle.UpdateQuarter();
	if (ch3) noise.UpdateQuarter();

	for (UINT i=0; i < ExtChannels.Size(); ++i)
	{
		if (ExtChannels[i]->IsActive())
			ExtChannels[i]->UpdateQuarter();
	}

	if (cycles.PhaseIndex & 0x1)
	{
		if (ch0) square1.UpdateHalf();
		if (ch1) square2.UpdateHalf();

		for (UINT i=0; i < ExtChannels.Size(); ++i)
		{
			if (ExtChannels[i]->IsActive())
				ExtChannels[i]->UpdateHalf();
		}
	}

	if (cycles.PhaseIndex++ == 3)
	{
		if (ch0) square1.UpdateWhole();
		if (ch1) square2.UpdateWhole();
		if (ch2) triangle.UpdateWhole();
		if (ch3) noise.UpdateWhole();

		for (UINT i=0; i < ExtChannels.Size(); ++i)
		{
			if (ExtChannels[i]->IsActive())
				ExtChannels[i]->UpdateWhole();
		}

		if (!(cpu.GetStatus() & CPU::STATUS_EXT_IRQ))
			cycles.PhaseIndex = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// I/O
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(APU,4000) { Synchronize(); square1.WriteReg0  (data); }
NES_POKE(APU,4001) { Synchronize(); square1.WriteReg1  (data); }
NES_POKE(APU,4002) { Synchronize(); square1.WriteReg2  (data); }
NES_POKE(APU,4003) { Synchronize(); square1.WriteReg3  (data); }
NES_POKE(APU,4004) { Synchronize(); square2.WriteReg0  (data); }
NES_POKE(APU,4005) { Synchronize(); square2.WriteReg1  (data); }
NES_POKE(APU,4006) { Synchronize(); square2.WriteReg2  (data); }
NES_POKE(APU,4007) { Synchronize(); square2.WriteReg3  (data); }
NES_POKE(APU,4008) { Synchronize(); triangle.WriteReg0 (data); }
NES_POKE(APU,400A) { Synchronize(); triangle.WriteReg2 (data); }
NES_POKE(APU,400B) { Synchronize(); triangle.WriteReg3 (data); }
NES_POKE(APU,400C) { Synchronize(); noise.WriteReg0    (data); }
NES_POKE(APU,400E) { Synchronize(); noise.WriteReg2    (data); }
NES_POKE(APU,400F) { Synchronize(); noise.WriteReg3    (data); }
NES_POKE(APU,4010) { Synchronize(); dmc.WriteReg0      (data); }
NES_POKE(APU,4012) { Synchronize(); dmc.WriteReg2      (data); }
NES_POKE(APU,4013) { Synchronize(); dmc.WriteReg3      (data); }
NES_POKE(APU,4011) { Synchronize(); dmc.WriteReg1      (data); }

////////////////////////////////////////////////////////////////////////////////////////
// channel enable/disable
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::Poke_4015(const UINT data) 
{
	Synchronize();

	square1.Toggle  ( data & 0x01 );
	square2.Toggle  ( data & 0x02 );
	triangle.Toggle ( data & 0x04 );
	noise.Toggle    ( data & 0x08 );
	dmc.Toggle      ( data & 0x10 );
}

////////////////////////////////////////////////////////////////////////////////////////
// Return the channel states
////////////////////////////////////////////////////////////////////////////////////////

UINT APU::Peek_4015() 
{ 
	Synchronize();

	UINT data = 0x00;

	if ( square1.IsActive()  ) data  = 0x01;
	if ( square2.IsActive()  ) data |= 0x02;
	if ( triangle.IsActive() ) data |= 0x04;
	if ( noise.IsActive()    ) data |= 0x08;
	if ( dmc.IsEnabled()     ) data |= 0x10;

	return data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::Poke_4017(const UINT data)
{
	Synchronize();

	cycles.PhaseIndex = 0;

	if (data & CPU::STATUS_EXT_IRQ)
		UpdatePhase();

	cycles.FrameInit = -cpu.GetCycles<CPU::CYCLE_MASTER>();
	cycles.FrameCounter = cycles.QuarterFrame;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(APU,4xxx)
{
	return cpu.GetCache();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::GetContext(IO::SFX::CONTEXT& context) const
{
	memcpy( &context, &emulation, sizeof(context) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SetContext(const IO::SFX::CONTEXT& context)
{
	PDX_ASSERT(emulation.SampleRate && (emulation.SampleBits == 16 || emulation.SampleBits == 8));

	memcpy( &emulation, &context, sizeof(emulation) );

	if (emulation.enabled)
	{
		emulation.enabled = 
		(
     		emulation.square1  ||
    		emulation.square2  ||
     		emulation.triangle ||
       		emulation.noise    ||
     		emulation.dpcm     ||
     		emulation.external
		);
	}
	else
	{
		emulation.square1  = FALSE;
		emulation.square2  = FALSE;
		emulation.triangle = FALSE;
		emulation.noise    = FALSE;
		emulation.dpcm     = FALSE;
		emulation.external = FALSE;
	}

	cycles.Reset( emulation.SampleRate, pal );
	buffer.Reset( emulation.SampleBits );

	square1.SetContext  ( cycles.rate, pal, emulation.square1  );
	square2.SetContext  ( cycles.rate, pal, emulation.square2  );
	triangle.SetContext ( cycles.rate, pal, emulation.triangle );
	noise.SetContext    ( cycles.rate, pal, emulation.noise    );
	dmc.SetContext      ( cycles.rate, pal, emulation.dpcm     );

	for (UINT i=0; i < ExtChannels.Size(); ++i)
		ExtChannels[i]->SetContext( cycles.rate, pal, emulation.external );

	SynchronizePtr = 
	(
     	(stream && emulation.enabled) ? 
		(ExtChannels.Size() ? SynchronizeAllON : SynchronizeON) : 
     	(SynchronizeOFF)
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::UpdateBuffer(const TSIZE offset)
{
	PDX_ASSERT(stream && stream->length && offset < stream->length);

	if (buffer.Is16Bit())
	{
		I16* PDX_RESTRICT samples = PDX_CAST(I16*,stream->samples) + offset;
		const I16* const end = PDX_CAST(I16*,stream->samples) + stream->length;

		do
		{
			const LONG sample = SampleAll();
			*samples = PDX_CLAMP(sample,I16_MIN,I16_MAX);
		}
		while (++samples != end);
	}
	else
	{
		U8* PDX_RESTRICT samples = PDX_CAST(U8*,stream->samples) + offset;
		const U8* const end = PDX_CAST(U8*,stream->samples) + stream->length;

		do
		{
			LONG sample = SampleAll();
			sample = PDX_CLAMP(sample,I16_MIN,I16_MAX);
			*samples = NES_APU_BUFFER_TO_8(sample);
		}
		while (++samples != end);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::BeginFrame(IO::SFX* const s)
{
	stream = s;

	SynchronizePtr = 
	(
     	(s && emulation.enabled) ? 
		(ExtChannels.Size() ? SynchronizeAllON : SynchronizeON) : 
     	(SynchronizeOFF)
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::EndFrame()
{
	const ULONG frame = cpu.GetFrameCycles<CPU::CYCLE_MASTER>();
	(*this.*SynchronizePtr)(frame);

	cycles.elapsed = 0;
	cycles.FrameInit = (cycles.FrameInit + frame) % cycles.QuarterFrame;
	cycles.FrameCounter = cycles.QuarterFrame - cycles.FrameInit;

	if (stream && emulation.enabled && PDX_SUCCEEDED(stream->Lock()))
	{
		const TSIZE written = buffer.Flush( stream );

		if (written < stream->length)
			UpdateBuffer( written );

		stream->Unlock();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::ClearBuffers()
{
	square1  .ClearAmp();
	square2  .ClearAmp();
	triangle .ClearAmp();
	noise    .ClearAmp();
	dmc      .ClearAmp();

	for (UINT i=0; i < ExtChannels.Size(); ++i)
		ExtChannels[i]->ClearAmp();

	buffer.Reset( emulation.SampleBits, FALSE );
}

////////////////////////////////////////////////////////////////////////////////////////
// channel constructor
////////////////////////////////////////////////////////////////////////////////////////

APU::CHANNEL::CHANNEL()
: pal(0), emulate(TRUE) {}

////////////////////////////////////////////////////////////////////////////////////////
// channel reset
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::CHANNEL::Reset()
{
	active        = 0;
	enabled       = 0;
	LengthCounter = 0;
	frequency     = NES_APU_TO_FIXED(1);
	timer         = 0;
	amp           = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::CHANNEL::SetContext(const ULONG r,const BOOL p,const BOOL e)
{
	rate = r;
	pal = (p ? 1 : 0);

	if (!(emulate = e))
		active = FALSE;

	UpdateContext();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::CHANNEL::LoadState(PDXFILE& file)
{
	HEADER header;
	
	if (!file.Read(header))
		return PDX_FAILURE;

	active		  = header.active;
	enabled	      = header.enabled;
	LengthCounter = header.LengthCounter;
	frequency	  = header.frequency;
	timer		  = header.timer;
	amp	  	      = header.amp;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::CHANNEL::SaveState(PDXFILE& file) const
{
	HEADER header;

	header.active		 = active ? 1 : 0;
	header.enabled	     = enabled ? 1 : 0;
	header.LengthCounter = LengthCounter;
	header.frequency	 = frequency;
	header.timer		 = timer;
	header.amp	  	     = amp;

	file << header;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// square channel #1/2 reset
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SQUARE::Reset()
{
	CHANNEL::Reset();

	step = 0;
	EnvCount = 0;
	EnvRate = 1;
	SweepRate = 1;
	DutyPeriod = 2;

	EnvDecayDisable = 0;
	EnvDecayRate = 0;
	EnvDecayLoop = 0;

	SweepEnabled = FALSE;
	SweepUpdateRate = 0;
	SweepShift = 0;
	SweepDecrease = 0;
	SweepCarry = NES_APU_TO_FIXED(0x400); 

	WaveLengthLow = 0;
	WaveLengthHigh = 0;

	volume = 0;
	ValidFrequency = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SQUARE::Toggle(const BOOL state)
{
	enabled = state; 

	if (!state) 
	{
		active = 0;
		LengthCounter = 0;
		timer = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID APU::SQUARE::UpdateVolume()
{
	volume = NES_APU_OUTPUT(EnvDecayDisable ? EnvDecayRate : EnvCount);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL APU::SQUARE::IsValidFrequency() const
{
	return frequency >= NES_SQUARE_MIN_FREQUENCY && (SweepDecrease || frequency <= SweepCarry);
}

////////////////////////////////////////////////////////////////////////////////////////
// square channel #1/2 register #1 write
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SQUARE::WriteReg0(const UINT data)
{
	REG0 reg;
	reg.latch = data;

	EnvDecayRate    = reg.EnvDecayRate;
	EnvDecayDisable = reg.EnvDecayDisable;
	EnvDecayLoop    = reg.EnvDecayLoop ? 0xF : 0x0;
	EnvRate         = EnvDecayRate + 1;

	UpdateVolume();

	static const UCHAR duties[4] = {2,4,8,12};
	DutyPeriod = duties[reg.DutyCycleType];
}

////////////////////////////////////////////////////////////////////////////////////////
// square channel #1/2 register #2 write
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SQUARE::WriteReg1(const UINT data)
{
	static const USHORT SweepCarries[8] =
	{
		0x400, 0x556, 0x667, 0x71D, 
		0x788, 0x7C2, 0x7E1, 0x7F1
	};

	REG1 reg;
	reg.latch = data;
	
	SweepShift      = reg.SweepShift;
	SweepDecrease   = reg.SweepDecrease;
	SweepUpdateRate	= reg.SweepUpdateRate + 1;
	SweepRate       = SweepUpdateRate;
	SweepEnabled    = reg.SweepEnabled;
	SweepCarry      = NES_APU_TO_FIXED(SweepCarries[reg.SweepShift]);

	ValidFrequency = IsValidFrequency();
	active = LengthCounter && ValidFrequency && enabled && emulate;
}

////////////////////////////////////////////////////////////////////////////////////////
// square channel #1/2 register #3 write
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SQUARE::WriteReg2(const UINT data)
{
	WaveLengthLow = data;
	frequency = NES_APU_TO_FIXED(WaveLengthLow + WaveLengthHigh + 1);

	ValidFrequency = IsValidFrequency();
	active = LengthCounter && ValidFrequency && enabled && emulate;
}

////////////////////////////////////////////////////////////////////////////////////////
// square channel #1/2 register #4 write
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SQUARE::WriteReg3(const UINT data)
{
	step = 0;
	EnvCount = 0xF;

	UpdateVolume();	

	REG3 reg;
	reg.latch = data;

	LengthCounter  = LengthTable[reg.LengthCount];
	WaveLengthHigh = reg.WaveLengthHigh << 8;
	frequency      = NES_APU_TO_FIXED(WaveLengthLow + WaveLengthHigh + 1);

	ValidFrequency = IsValidFrequency();
	active = enabled && ValidFrequency && emulate;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SQUARE::UpdateQuarter()
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

VOID APU::SQUARE::UpdateHalf()
{
	if (SweepEnabled && ValidFrequency && !--SweepRate)
	{
		SweepRate = SweepUpdateRate;

		if (SweepShift)
		{
			const UINT f = NES_APU_FROM_FIXED(frequency);
			const UINT shifted = f >> SweepShift;

			if (SweepDecrease)
			{
				frequency = NES_APU_TO_FIXED(f - (shifted + complement));

				if (frequency < NES_SQUARE_MIN_FREQUENCY)
					ValidFrequency = active = 0;
			}
			else
			{
				frequency = NES_APU_TO_FIXED(f + shifted);

				if (frequency > SweepCarry)
					ValidFrequency = active = 0;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::SQUARE::UpdateWhole()
{
	if (!EnvDecayLoop && !--LengthCounter)
		active = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// sample
////////////////////////////////////////////////////////////////////////////////////////

LONG APU::SQUARE::Sample()
{
	if (active)
	{
		LONG weight = PDX_MIN(rate,timer);
		LONG sum = (step < DutyPeriod) ? +weight : -weight;

		for (timer -= rate; timer < 0; )
		{
			weight = frequency;

			if ((timer += frequency) > 0)
				weight -= timer;

			step = (step + 1) & 0xF;
			sum += (step < DutyPeriod) ? +weight : -weight;
		}

		amp = LONG(floor(FLOAT(volume * sum) / FLOAT(rate) + 0.5f));
	}
	else
	{
		amp -= (amp >> 6);
	}

	return amp;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::SQUARE::LoadState(PDXFILE& file)
{
	PDX_TRY(CHANNEL::LoadState(file));

	{
		HEADER header;
		
		if (!file.Read(header))
			return PDX_FAILURE;

		step			 = header.step;
		DutyPeriod		 = header.DutyPeriod;
		EnvCount		 = header.EnvCount;
		EnvRate		     = header.EnvRate;
		EnvDecayRate	 = header.EnvDecayRate;
		EnvDecayDisable  = header.EnvDecayDisable ? 1 : 0;
		EnvDecayLoop	 = header.EnvDecayLoop;
		SweepRate		 = header.SweepRate;
		SweepUpdateRate  = header.SweepUpdateRate;
		SweepShift		 = header.SweepShift;
		SweepDecrease	 = header.SweepDecrease ? 1 : 0;
		SweepEnabled	 = header.SweepEnabled ? 1 : 0;
		WaveLengthLow	 = header.WaveLengthLow;
		WaveLengthHigh	 = header.WaveLengthHigh << 8;
		SweepCarry       = header.SweepCarry;
	}

	ValidFrequency = IsValidFrequency();

	UpdateVolume();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::SQUARE::SaveState(PDXFILE& file) const
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
	header.SweepRate	   = SweepRate;
	header.SweepUpdateRate = SweepUpdateRate;
	header.SweepShift	   = SweepShift;
	header.SweepDecrease   = SweepDecrease;
	header.SweepEnabled	   = SweepEnabled ? 1 : 0;
	header.WaveLengthLow   = WaveLengthLow;
	header.WaveLengthHigh  = WaveLengthHigh >> 8;
	header.SweepCarry      = SweepCarry;

	file << header;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// triangle	channel reset
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::TRIANGLE::Reset()
{
	CHANNEL::Reset();

	step = 0;
	counting = FALSE;
	ChangeMode = FALSE;
	
	LinearCounter = 0;
	LinearCounterStart = 0;
	LinearCounterLoad = 0;
	
	WaveLengthLow = 0;
	WaveLengthHigh = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::TRIANGLE::Toggle(const BOOL state)
{
	enabled = state; 

	if (!state) 
	{
		active = 0;
		LengthCounter = 0;
		LinearCounter = 0;
		timer = 0;
		counting = FALSE;
		ChangeMode = FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// triangle	channel register writes
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::TRIANGLE::WriteReg0(const UINT data) 
{ 
	REG0 reg;
	reg.latch = data;

	LinearCounterStart = reg.LinearCounterStart;
	LinearCounterLoad = reg.LinearCounterLoad;

	if (!counting && LengthCounter)
	{
		LinearCounter = LinearCounterLoad;
		active = emulate && LinearCounter && enabled && frequency >= NES_TRIANGLE_MIN_FREQUENCY;
	}
}     

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::TRIANGLE::WriteReg2(const UINT data) 
{ 
	WaveLengthLow = data;
	frequency = NES_APU_TO_FIXED(WaveLengthLow + WaveLengthHigh + 1);
	active = emulate && LengthCounter && LinearCounter && enabled && frequency >= NES_TRIANGLE_MIN_FREQUENCY;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::TRIANGLE::WriteReg3(const UINT data) 
{ 
	counting = FALSE;
	ChangeMode = TRUE;

	REG3 reg;
	reg.latch = data;

	LinearCounter  = LinearCounterLoad;
	LengthCounter  = LengthTable[reg.LengthCount];
	WaveLengthHigh = reg.WaveLengthHigh << 8;
	frequency      = NES_APU_TO_FIXED(WaveLengthLow + WaveLengthHigh + 1);
	active         = emulate && LinearCounter && enabled && frequency >= NES_TRIANGLE_MIN_FREQUENCY;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::TRIANGLE::UpdateQuarter()
{
	if (counting && LinearCounter && !--LinearCounter)
		active = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::TRIANGLE::UpdateWhole()
{
	if (!LinearCounterStart)
	{
		if (!counting && ChangeMode)
		{
			ChangeMode = FALSE;
			counting = TRUE;
		}

		if (LengthCounter && !--LengthCounter)
			active = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG APU::TRIANGLE::Sample()
{
	if (active)
	{
		LONG sum = (((step & 0x10) ? 0x1F : 0x00) ^ step) * PDX_MIN(rate,timer);

		for (timer -= rate; timer < 0;) 
		{
			LONG weight = frequency;
       
			if ((timer += frequency) > 0)
				weight -= timer;

			step = (step + 1) & 0x1F;
			sum += (((step & 0x10) ? 0x1F : 0x00) ^ step) * weight;
		}

		amp = LONG(floor(FLOAT(sum * 512) / FLOAT(rate) + 0.5f));
	}

	return (amp * 21) >> 4;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::TRIANGLE::LoadState(PDXFILE& file)
{
	PDX_TRY(CHANNEL::LoadState(file));

	HEADER header;
	
	if (!file.Read(header))
		return PDX_FAILURE;

	step               = header.step;
	counting           = header.counting;
	ChangeMode         = header.ChangeMode;
	LinearCounter      = header.LinearCounter;
	LinearCounterStart = header.LinearCounterStart;
	LinearCounterLoad  = header.LinearCounterLoad;
	WaveLengthLow      = header.WaveLengthLow;
	WaveLengthHigh     = header.WaveLengthHigh << 8;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::TRIANGLE::SaveState(PDXFILE& file) const
{
	PDX_TRY(CHANNEL::SaveState(file));

	HEADER header;

	header.step				  = step;
	header.counting			  = counting ? 1 : 0;
	header.ChangeMode   	  = ChangeMode ? 1 : 0;
	header.LinearCounter	  = LinearCounter;
	header.LinearCounterStart = LinearCounterStart ? 1 : 0;
	header.LinearCounterLoad  = LinearCounterLoad;
	header.WaveLengthLow	  = WaveLengthLow;
	header.WaveLengthHigh	  = WaveLengthHigh >> 8;

	file << header;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// noise channel reset
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::NOISE::Reset()
{
	CHANNEL::Reset();

	EnvCount = 0;
	EnvRate = 1;

	EnvDecayDisable = 0;
	EnvDecayRate = 0;
	EnvDecayLoop = 0;

	volume = 0;
	bits = 1;
	shifter = 13;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::NOISE::Toggle(const BOOL state)
{
	enabled = state; 

	if (!state) 
	{
		active = 0;
		LengthCounter = 0;
		timer = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID APU::NOISE::UpdateVolume()
{
	volume = NES_APU_OUTPUT(EnvDecayDisable ? EnvDecayRate : EnvCount);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::NOISE::WriteReg0(const UINT data) 
{ 
	REG0 reg;
	reg.latch = data;

	EnvDecayDisable = reg.EnvDecayDisable;
	EnvDecayLoop    = reg.EnvDecayLoop ? 0xF : 0x0;
	EnvDecayRate    = reg.EnvDecayRate;
	EnvRate         = EnvDecayRate + 1;

	UpdateVolume();
}  

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::NOISE::WriteReg2(const UINT data) 
{
	static const USHORT table[] = 
	{
		(0x002 * 2) + 1, (0x004 * 2) + 1, (0x008 * 2) + 1, (0x010 * 2) + 1,
		(0x020 * 2) + 1, (0x030 * 2) + 1, (0x040 * 2) + 1, (0x050 * 2) + 1,
		(0x065 * 2) + 1, (0x07F * 2) + 1, (0x0BE * 2) + 1, (0x0FE * 2) + 1,
		(0x17D * 2) + 1, (0x1FC * 2) + 1, (0x3F9 * 2) + 1, (0x7F2 * 2) + 1
	};

	REG2 reg;
	reg.latch = data;

	frequency = NES_APU_TO_FIXED(table[reg.SampleRate]);
	shifter = reg.Bit93Mode ? 8 : 13;
}         

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::NOISE::WriteReg3(const UINT data) 
{ 
	EnvCount = 0xF;
	UpdateVolume();

	active = emulate && enabled;

	REG3 reg;
	reg.latch = data;
	LengthCounter = LengthTable[reg.LengthCount];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::NOISE::UpdateQuarter()
{
	PDX_ASSERT(EnvRate);

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

VOID APU::NOISE::UpdateWhole()
{
	PDX_ASSERT(EnvDecayLoop || LengthCounter);

	if (!EnvDecayLoop && !--LengthCounter)
		active = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG APU::NOISE::Sample()
{
	if (active)
	{
		LONG weight = PDX_MIN(rate,timer);
		LONG sum = (bits & 0x4000U) ? +weight : -weight;

		for (timer -= rate; timer < 0; )
		{
			weight = frequency;

			if ((timer += frequency) > 0)
				weight -= timer;

			bits = (bits << 1) | (((bits >> 14) ^ (bits >> shifter)) & 0x1);
			sum += (bits & 0x4000U) ? +weight : -weight;
		}

		amp = LONG(floor(FLOAT(volume * sum) / FLOAT(rate) + 0.5f));
	}
	else
	{
		amp -= (amp >> 6);
	}

	return (amp * 13) >> 4;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::NOISE::LoadState(PDXFILE& file)
{
	PDX_TRY(CHANNEL::LoadState(file));

	{
		HEADER header;
		
		if (!file.Read(header))
			return PDX_FAILURE;

		bits            = header.bits;            
		shifter         = header.shifter;        
		EnvCount        = header.EnvCount;
		EnvRate         = header.EnvRate;
		EnvDecayRate    = header.EnvDecayRate;
		EnvDecayDisable = header.EnvDecayDisable;
		EnvDecayLoop    = header.EnvDecayLoop;
	}

	UpdateVolume();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::NOISE::SaveState(PDXFILE& file) const
{
	PDX_TRY(CHANNEL::SaveState(file));

	HEADER header;
	
	header.bits            = bits;
	header.shifter         = shifter;
	header.EnvCount        = EnvCount;
	header.EnvRate         = EnvRate;
	header.EnvDecayRate    = EnvDecayRate;
	header.EnvDecayDisable = EnvDecayDisable;
	header.EnvDecayLoop    = EnvDecayLoop;
	
	file << header;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::DMC::Reset()
{
	CHANNEL::Reset();

	address = 0;
	loop = FALSE;
	LoadedLengthCounter = 1;
	LoadedAddress = 0xC000;
	output = 0;

	cpu.SetLine(CPU::IRQ_DMC,FALSE);
	cpu.DisableDmcCounter();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::DMC::Toggle(const BOOL TurnOn)
{
	cpu.ClearIRQ(CPU::IRQ_DMC);

	if (TurnOn && !enabled)
	{
		BitCounter = 7;
		LengthCounter = LoadedLengthCounter;
		address = LoadedAddress;
		timer = 0;
		cpu.SetDmcLengthCounter(LoadedLengthCounter);
		cpu.SetDmcCounter(0);
	}

	if (!(enabled = TurnOn))
		cpu.DisableDmcCounter();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID APU::DMC::WriteReg0(const UINT data)
{
	REG0 reg;
	reg.latch = data;

	loop = reg.loop;
	cpu.SetLine(CPU::IRQ_DMC,reg.GenerateIrq);

	static const USHORT table[2][16] = 
	{
		{
			0xD60, 0xBE0, 0xAA0, 0xA00,
			0x8F0, 0x7F0, 0x710, 0x6B0,
			0x5F0, 0x500, 0x470, 0x400,
			0x350, 0x2A8, 0x240, 0x1B0
		},
		{
			0xD40, 0xBC0, 0xA80, 0x9E0,
			0x8D0, 0x7D0, 0x6F0, 0x690,
			0x5D0, 0x4E0, 0x450, 0x3E0,
			0x330, 0x288, 0x220, 0x190
		}
	};

	frequency = NES_APU_TO_FIXED(table[pal][reg.frequency]); 
 
	if (enabled)
	{
		cpu.SetDmcCounter(frequency);
		cpu.SetDmcLengthCounter(LengthCounter);
	}

	frequency >>= 3; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID APU::DMC::WriteReg1(UINT data)
{
	data &= 0x7F;
	amp += NES_APU_OUTPUT(data - output);
	output = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID APU::DMC::WriteReg2(const UINT data)
{
	LoadedAddress = 0xC000 + (data << 6);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID APU::DMC::WriteReg3(const UINT data)
{
	LoadedLengthCounter = (data << 4) + 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG APU::DMC::Sample()
{
	amp -= (amp >> 7);

	if (enabled && emulate)
	{
		for (timer += rate; timer > 0; timer -= frequency)
		{
			if (++BitCounter == 8)
			{
				BitCounter = 0;
				latch = cpu.Peek(address);
				address = 0x8000U + ((address + 1) & 0x7FFF);

				if (!--LengthCounter)
				{
					if (loop)
					{
						LengthCounter = LoadedLengthCounter;
						address = LoadedAddress;
					}
					else
					{
						enabled = FALSE;
						timer = 0;
						break;
					}
				}
			}

			if ((latch >> BitCounter) & 0x1)
			{
				if (output < 0x7E)
				{
					output += 2;
					amp    += NES_APU_OUTPUT(2);
				}
			}
			else
			{
				if (output > 0x01)
				{
					output -= 2;
					amp    -= NES_APU_OUTPUT(2);
				}
			}
		}
	}

	return (amp * 13) >> 4;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::DMC::LoadState(PDXFILE& file)
{
	PDX_TRY(CHANNEL::LoadState(file));

	HEADER header;
	
	if (!file.Read(header))
		return PDX_FAILURE;

	BitCounter          = header.BitCounter;
	LengthCounter       = header.LengthCounter;
	latch               = header.latch;
	address             = header.address;
	LoadedAddress       = header.LoadedAddress;
	LoadedLengthCounter = header.LoadedLengthCounter;
	loop                = header.loop ? 1 : 0;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::DMC::SaveState(PDXFILE& file) const
{
	PDX_TRY(CHANNEL::SaveState(file));

	HEADER header;

	header.BitCounter          = BitCounter;
	header.LengthCounter       = LengthCounter;
	header.latch               = latch;
	header.address             = address;
	header.LoadedAddress       = LoadedAddress;
	header.LoadedLengthCounter = LoadedLengthCounter;
	header.loop                = loop;

	file << header;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::LoadState(PDXFILE& file)
{
	{
		HEADER header;
		
		if (!file.Read(header))
			return PDX_FAILURE;

		cycles.elapsed      = header.ElapsedCycles;
		cycles.FrameInit    = header.FrameInit;
		cycles.FrameCounter = header.FrameCounter; 
		cycles.RateCounter  = header.RateCounter; 
		cycles.PhaseIndex   = header.PhaseIndex;           
	}

	PDX_TRY( square1.LoadState  (file) );
	PDX_TRY( square2.LoadState  (file) );
	PDX_TRY( triangle.LoadState (file) );
	PDX_TRY( noise.LoadState    (file) );
	PDX_TRY( dmc.LoadState      (file) );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT APU::SaveState(PDXFILE& file) const
{
	{
		HEADER header;

		header.ElapsedCycles = cycles.elapsed;     
		header.FrameInit     = cycles.FrameInit;
		header.FrameCounter	 = cycles.FrameCounter;
		header.RateCounter	 = cycles.RateCounter;
		header.PhaseIndex    = cycles.PhaseIndex;

		file << header;
	}

	PDX_TRY( square1.SaveState  (file) );
	PDX_TRY( square2.SaveState  (file) );
	PDX_TRY( triangle.SaveState (file) );
	PDX_TRY( noise.SaveState    (file) );
	PDX_TRY( dmc.SaveState      (file) );

	return PDX_OK;
}

NES_NAMESPACE_END
