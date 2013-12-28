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

#ifndef NST_SNDVRC6_H
#define NST_SNDVRC6_H

#include "../NstApu.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// KONAMI VRC6
////////////////////////////////////////////////////////////////////////////////////////

class SNDVRC6
{
public:

	SNDVRC6(CPU* const);
	~SNDVRC6();

	VOID Reset();

	VOID WriteSquare1Reg0  (const UINT);
	VOID WriteSquare1Reg1  (const UINT);
	VOID WriteSquare1Reg2  (const UINT);
	VOID WriteSquare2Reg0  (const UINT);
	VOID WriteSquare2Reg1  (const UINT);
	VOID WriteSquare2Reg2  (const UINT);
	VOID WriteSawToothReg0 (const UINT);
	VOID WriteSawToothReg1 (const UINT);
	VOID WriteSawToothReg2 (const UINT);

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;

private:

	class SQUARE : public APU::CHANNEL
	{
	public:

		VOID Reset();
		
		VOID WriteReg0(const UINT data);
		VOID WriteReg1(const UINT data);
		VOID WriteReg2(const UINT data);

		PDXRESULT LoadState(PDXFILE&);
		PDXRESULT SaveState(PDXFILE&) const;

	private:

		LONG Sample();

		UINT duty;
		UINT step;
		LONG volume;
		BOOL digitized;
		UINT WaveLengthLow;
		UINT WaveLengthHigh;

       #pragma pack(push,1)

		union REG0
		{
			struct  
			{
				U8 latch;
			};

			struct  
			{
				U8 volume    : 4;
                U8 duty      : 3;
				U8 digitized : 1;
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
				U8 WaveLengthHigh : 4;
                U8                : 3;
				U8 enable         : 1;
			};
		};

       #pragma pack(pop)
	};

	class SAWTOOTH : public APU::CHANNEL
	{
	public:

		VOID Reset();

		VOID WriteReg0(const UINT);
		VOID WriteReg1(const UINT);
		VOID WriteReg2(const UINT);

		PDXRESULT LoadState(PDXFILE&);
		PDXRESULT SaveState(PDXFILE&) const;

	private:

		LONG Sample();

		UINT  WaveLengthLow;
		UINT  WaveLengthHigh;
		ULONG phase;
		UINT  step;

       #pragma pack(push,1)

		union REG0
		{
			struct  
			{
				U8 latch;
			};

			struct  
			{
				U8 phase : 6;
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
				U8 WaveLengthHigh : 4;
                U8                : 3;
				U8 enable         : 1;
			};
		};

       #pragma pack(pop)
	};

	CPU* const cpu;
	APU* const apu;

	SQUARE square[2];
	SAWTOOTH sawtooth;
};

NES_NAMESPACE_END

#endif
