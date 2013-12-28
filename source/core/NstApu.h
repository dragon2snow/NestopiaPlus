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

#ifndef NST_APU_H
#define NST_APU_H

class PDXFILE;

NES_NAMESPACE_BEGIN

class CPU;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_APU_TO_FIXED(x)   (pal ? NES_CPU_TO_PAL(x) : NES_CPU_TO_NTSC(x))
#define NES_APU_FROM_FIXED(x) ((x) / (pal ? NES_CPU_PAL_FIXED : NES_CPU_NTSC_FIXED))
#define NES_APU_OUTPUT(x)     ((x) << 8)

////////////////////////////////////////////////////////////////////////////////////////
// APU
////////////////////////////////////////////////////////////////////////////////////////

class APU
{
public:

	APU(CPU&);

	VOID Reset();
	VOID ClearBuffers();
	VOID SetContext(const IO::SFX::CONTEXT&);
	VOID GetContext(IO::SFX::CONTEXT&) const;
	VOID SetMode(const MODE);
	VOID BeginFrame(IO::SFX* const);
	VOID EndFrame();
	VOID Update();

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;

	UINT Peek_4015();
	VOID Poke_4015(const UINT);
	VOID Poke_4017(const UINT);

	LONG GetDmcFrequency()     const;
	LONG GetDmcLengthCounter() const;
	BOOL IsDmcLooped()         const;

	class CHANNEL
	{
	public:
		
		CHANNEL();

		virtual BOOL IsActive() const
		{ return enabled && LengthCounter; }

		virtual VOID Reset();
		virtual LONG Sample() = 0;
		
		virtual VOID UpdateQuarter() {}
		virtual VOID UpdateHalf()    {}
		virtual VOID UpdateWhole()   {}

		VOID SetContext(const ULONG,const BOOL,const BOOL);

	protected:

		virtual VOID UpdateContext() {}

		PDXRESULT SaveState(PDXFILE&) const;
		PDXRESULT LoadState(PDXFILE&);

		UINT  active;
		UINT  enabled;
		UINT  LengthCounter;
		LONG  frequency;
		LONG  timer;
		LONG  amp;
		UINT  pal;
		ULONG rate;
		BOOL  emulate;

		static const UCHAR LengthTable[32];

       #pragma pack(push,1)

		struct HEADER 
		{
			I32 frequency;
			I32 timer;
			I32 amp;
			I32 sample;
			U16	LengthCounter;
			U8  active : 1;
			U8  enabled : 1;
		};

       #pragma pack(pop)
	};

	VOID HookChannel(CHANNEL* const);
	VOID ReleaseChannel(const CHANNEL* const);

private:

	enum
	{
		RESET_CYCLES = 2048
	};

	NES_DECL_POKE( 4000 );
	NES_DECL_POKE( 4001 );
	NES_DECL_POKE( 4002 );
	NES_DECL_POKE( 4003 );
	NES_DECL_POKE( 4004 );
	NES_DECL_POKE( 4005 );
	NES_DECL_POKE( 4006 );
	NES_DECL_POKE( 4007 );
	NES_DECL_POKE( 4008 );
	NES_DECL_POKE( 400A );
	NES_DECL_POKE( 400B );
	NES_DECL_POKE( 400C );
	NES_DECL_POKE( 400E );
	NES_DECL_POKE( 400F );
	NES_DECL_POKE( 4010 );
	NES_DECL_POKE( 4011 );
	NES_DECL_POKE( 4012 );
	NES_DECL_POKE( 4013 );
	NES_DECL_PEEK( 4xxx );

	LONG Sample();
	VOID Synchronize();
	VOID Synchronize(const ULONG);
	VOID UpdatePhase();

	PDX_NO_INLINE VOID UpdateBuffer(const TSIZE);

	typedef PDXARRAY<CHANNEL* const> EXTCHANNELS;

	class SQUARE : public CHANNEL
	{
	public:

		SQUARE(const UINT index)
		: complement(index ? 0 : 1) {}

		VOID Reset();
		VOID Toggle(const BOOL);
		
		VOID WriteReg0(const UINT);
		VOID WriteReg1(const UINT);
		VOID WriteReg2(const UINT);
		VOID WriteReg3(const UINT);

		LONG Sample();
		VOID UpdateHalf();
		VOID UpdateQuarter();
		VOID UpdateWhole();

		PDXRESULT SaveState(PDXFILE&) const;
		PDXRESULT LoadState(PDXFILE&);

	private:

		VOID UpdateVolume();
		BOOL IsValidFrequency() const;

		LONG  volume;
		UINT  step;
		UINT  DutyPeriod;
		UINT  EnvCount;
		UINT  EnvRate;
		UINT  EnvDecayRate;
		UINT  EnvDecayDisable;
		UINT  EnvDecayLoop;
		UINT  SweepRate;
		UINT  SweepUpdateRate;
		UINT  SweepShift;
		UINT  SweepDecrease;
		BOOL  SweepEnabled;
		ULONG SweepCarry;
		UINT  WaveLengthLow;
		UINT  WaveLengthHigh;
		BOOL  ValidFrequency;
		
		const UINT complement;

       #pragma pack(push,1)

		struct HEADER
		{
			U8  step             : 4;
			U8  DutyPeriod       : 4;
			U8  EnvCount         : 4;
			U8  EnvDecayRate     : 4;
			U8  EnvRate          : 5;
			U8  SweepShift       : 3;
			U8  EnvDecayLoop     : 4;
			U8  SweepRate        : 4;
			U8  EnvDecayDisable  : 1;
			U8  WaveLengthHigh   : 3;
			U8  SweepUpdateRate  : 4;
			U8  WaveLengthLow;
			U8  SweepDecrease    : 1;
			U8  SweepEnabled     : 1;
			U32 SweepCarry;
		};

		union REG0
		{
			struct
			{
				U8 latch;
			};

			struct  
			{
				U8 EnvDecayRate    : 4;
				U8 EnvDecayDisable : 1;
				U8 EnvDecayLoop    : 1;
				U8 DutyCycleType   : 2;
			};
		};

		union REG1
		{
			struct
			{
				U8 latch;
			};

			struct  
			{
				U8 SweepShift      : 3;
				U8 SweepDecrease   : 1;
				U8 SweepUpdateRate : 3;
				U8 SweepEnabled    : 1;
			};
		};

		union REG3
		{
			struct  
			{
				U8 latch;
			};

			struct  
			{
                U8 WaveLengthHigh : 3;
				U8 LengthCount    : 5;
			};
		};

       #pragma pack(pop)
	};

	class DMC;

	class TRIANGLE : public CHANNEL
	{
	public:

		TRIANGLE(const DMC& d)
		: dmc(d) {}

		VOID Reset();
		VOID Toggle(const BOOL);
		
		VOID WriteReg0(const UINT);
		VOID WriteReg2(const UINT);
		VOID WriteReg3(const UINT);
		
		LONG Sample();
		VOID UpdateQuarter();
		VOID UpdateWhole();

		PDXRESULT SaveState(PDXFILE&) const;
		PDXRESULT LoadState(PDXFILE&);

	private:

		UINT step;
		BOOL counting;
		BOOL ChangeMode;
		UINT LinearCounter;
		BOOL LinearCounterStart;
		UINT LinearCounterLoad;
		UINT WaveLengthLow;
		UINT WaveLengthHigh;

		const DMC& dmc;

       #pragma pack(push,1)

		struct HEADER
		{
			U8 step               : 5;
			U8 WaveLengthHigh     : 3;
			U8 counting           : 1;
			U8 LinearCounter      : 7;
			U8 LinearCounterStart : 1;
			U8 LinearCounterLoad  : 7;
			U8 ChangeMode         : 1;
			U8 WaveLengthLow;
		};

		union REG0
		{
			struct  
			{
				U8 latch;
			};

			struct  
			{
				U8 LinearCounterLoad  : 7;
				U8 LinearCounterStart : 1;
			};
		};

		union REG3
		{
			struct  
			{
				U8 latch;
			};

			struct  
			{
				U8 WaveLengthHigh : 3;
				U8 LengthCount    : 5;
			};
		};

       #pragma pack(pop)
  	};

	class NOISE : public CHANNEL
	{
	public:

		NOISE(const DMC& d)
		: dmc(d) {}

		VOID Reset();
		VOID Toggle(const BOOL);
		
		VOID WriteReg0(const UINT);
		VOID WriteReg2(const UINT);
		VOID WriteReg3(const UINT);
		
		LONG Sample();
		VOID UpdateQuarter();
		VOID UpdateWhole();

		PDXRESULT SaveState(PDXFILE&) const;
		PDXRESULT LoadState(PDXFILE&);

	private:

		VOID UpdateVolume();

		LONG volume;
		UINT bits;
		UINT shifter;
		UINT EnvCount;
		UINT EnvRate;
		UINT EnvDecayCount;
		UINT EnvDecayRate;
		UINT EnvDecayDisable;
		UINT EnvDecayLoop;

		const DMC& dmc;

       #pragma pack(push,1)

		struct HEADER
		{
			U8  bits;
			U8  EnvCount        : 4;
			U8  EnvDecayRate    : 4;
			U16 shifter         : 15;
			U8  EnvDecayDisable : 1;
			U8  EnvDecayLoop    : 4;
			U8  EnvRate 		: 5;
		};

		union REG0
		{
			struct  
			{
				U8 latch;
			};

			struct  
			{
				U8 EnvDecayRate    : 4;
				U8 EnvDecayDisable : 1;
				U8 EnvDecayLoop    : 1;
                U8                 : 2;
			};
		};

   		union REG2
   		{
       		struct  
       		{
     			U8 latch;
       		};

       		struct  
       		{
     			U8 SampleRate : 4;
                U8            : 3;
      			U8 Bit93Mode  : 1;
     		};
		};

		union REG3
		{
			struct  
			{
				U8 latch;
			};

			struct  
			{
                U8             : 3;
				U8 LengthCount : 5;
			};
		};

       #pragma pack(pop)
	};

	class DMC : public CHANNEL
	{
	public:

		DMC(CPU& c)
		: cpu(c) {}

		VOID Reset();
		VOID Toggle(const BOOL);

		VOID WriteReg0(const UINT);
		VOID WriteReg1(const UINT);		
		VOID WriteReg2(const UINT);
		VOID WriteReg3(const UINT);

		inline BOOL IsEnabled() const
		{ return enabled; }

		inline LONG GetFrequency() const
		{ return frequency << 3; }

		inline LONG GetLengthCounter() const
		{ return LoadedLengthCounter; }

		inline BOOL IsLooped() const
		{ return loop; }

		LONG Sample();

		PDXRESULT SaveState(PDXFILE&) const;
		PDXRESULT LoadState(PDXFILE&);

	private:

		friend class TRIANGLE;
		friend class NOISE;

		UINT BitCounter;
		INT  LengthCounter;
		UINT latch;
		UINT address;
		UINT LoadedAddress;	 
		UINT output;
		INT  LoadedLengthCounter;
		BOOL loop;
		CPU& cpu;

       #pragma pack(push,1)

		struct HEADER
		{
			I16 LengthCounter;
			U16 address;
			U16 LoadedAddress;
			I16 LoadedLengthCounter;
			U8  latch;
			U8  BitCounter : 4;
			U8  loop : 1;
		};

		union REG0
		{
			struct  
			{
				U8 latch;
			};
  
			struct  
			{
				U8 frequency   : 4;
				U8             : 2;
				U8 loop        : 1;
				U8 GenerateIrq : 1;
			};
		};
  
       #pragma pack(pop)
	};

	struct CYCLES
	{
		CYCLES();

		VOID Reset(const UINT,const BOOL);

		LONG  elapsed;
		ULONG rate;
		LONG  RateCounter;
		LONG  FrameCounter;
		LONG  FrameInit;
		ULONG QuarterFrame;
		UINT  PhaseIndex;
	};

	class BUFFER
	{
	public:

		BUFFER(); 
		~BUFFER();

		VOID Reset(const UINT);

		PDX_NO_INLINE TSIZE Flush(IO::SFX* const);

		inline UINT GetPos() const
		{ return pos; }

		VOID Write(const LONG);		
	
	private:

		PDX_NO_INLINE VOID Flush8(IO::SFX* const,const TSIZE);

		I16* const output;
		TSIZE pos;
		TSIZE start;
		BOOL Bit16;
	};

	BUFFER buffer;
	CYCLES cycles;

	CPU& cpu;

	UINT pal;

	IO::SFX* stream;

	SQUARE      square1;
	SQUARE      square2;
	TRIANGLE    triangle;
	NOISE       noise;
	DMC         dmc;
	EXTCHANNELS ExtChannels;

	IO::SFX::CONTEXT emulation;

   #pragma pack(push,1)

	struct HEADER
	{
		U32 ElapsedCycles;
		I32 RateCounter;
		I32 FrameCounter;	
		I32 FrameInit;
		U8  PhaseIndex : 3;
	};

   #pragma pack(pop)
};

#include "NstApu.inl"

NES_NAMESPACE_END

#endif
