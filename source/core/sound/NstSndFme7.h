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

#ifndef NST_SNDFME7_H
#define NST_SNDFME7_H

#include "../NstApu.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class SNDFME7
{
public:

	SNDFME7(CPU&);
	~SNDFME7();

	VOID Reset();

	VOID Poke_C000(const UINT);
	VOID Poke_E000(const UINT);

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;

private:

	class FME7CHANNEL
	{
	public:

		VOID Reset(const BOOL);

		PDXRESULT LoadState(PDXFILE&);
		PDXRESULT SaveState(PDXFILE&) const;

	protected:

		UINT active;
		LONG frequency;
		LONG timer;
		LONG amp;
	};

	class SQUARE : public FME7CHANNEL
	{
	public:

		VOID Reset(const BOOL);

		VOID WriteReg0(const UINT,const BOOL);
		VOID WriteReg1(const UINT,const BOOL);
		VOID WriteReg2(const UINT);
		VOID WriteReg3(const UINT);

		LONG Sample(const ULONG,const LONG,const LONG);

		PDXRESULT LoadState(PDXFILE&);
		PDXRESULT SaveState(PDXFILE&) const;

	private:

		enum
		{
			SOUND_ENVELOPE = 0x1,
			SOUND_NOISE	   = 0x8
		};

		UINT FreqLo;
		UINT FreqHi;
		UINT flags;
		LONG volume;
		BOOL UseEnv;
		UINT FlipFlop;
	};

	class ENVELOPE : public FME7CHANNEL
	{
	public:

		VOID Reset(const BOOL);
		LONG Update(const ULONG);

		VOID WriteReg0(const UINT,const BOOL);
		VOID WriteReg1(const UINT,const BOOL);
		VOID WriteReg2(const UINT);

		PDXRESULT LoadState(PDXFILE&);
		PDXRESULT SaveState(PDXFILE&) const;

	private:

		UINT FreqLo;
		UINT FreqHi;
		UINT TableIndex;

		const I8* oscilator;
	};

	class NOISE : public FME7CHANNEL
	{
	public:

		VOID Reset(const BOOL);
		LONG Update(const ULONG);

		VOID WriteReg(const UINT,const BOOL);

		PDXRESULT LoadState(PDXFILE&);
		PDXRESULT SaveState(PDXFILE&) const;

	private:

		ULONG ring;
	};

	class CHANNEL : public APU::CHANNEL
	{
	public:

		VOID Reset();

		VOID WriteReg0(const UINT);
		VOID WriteReg1(const UINT);

		PDXRESULT LoadState(PDXFILE&);
		PDXRESULT SaveState(PDXFILE&) const;

	private:

		LONG Sample();

		UINT     address;
		SQUARE   square[3];
		ENVELOPE envelope;
		NOISE    noise;
	};

	APU& apu;

	CHANNEL channel;
};

NES_NAMESPACE_END

#endif
