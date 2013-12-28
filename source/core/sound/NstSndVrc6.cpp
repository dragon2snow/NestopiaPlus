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
#include "NstSndVrc6.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDVRC6::SNDVRC6(CPU* const c)
: 
cpu (c),
apu (c->GetAPU())
{
	apu->HookChannel( PDX_STATIC_CAST(APU::CHANNEL* const,square+0)  );
	apu->HookChannel( PDX_STATIC_CAST(APU::CHANNEL* const,square+1)  );
	apu->HookChannel( PDX_STATIC_CAST(APU::CHANNEL* const,&sawtooth) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SNDVRC6::~SNDVRC6()
{
	apu->ReleaseChannel( PDX_STATIC_CAST(APU::CHANNEL* const,square+0)  );
	apu->ReleaseChannel( PDX_STATIC_CAST(APU::CHANNEL* const,square+1)  );
	apu->ReleaseChannel( PDX_STATIC_CAST(APU::CHANNEL* const,&sawtooth) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDVRC6::Reset()
{
	square[0].Reset();
	square[1].Reset();
	sawtooth.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDVRC6::LoadState(PDXFILE& file)
{
	PDX_TRY( square[0].LoadState( file ) );
	PDX_TRY( square[1].LoadState( file ) );
	PDX_TRY( sawtooth.LoadState( file )  );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDVRC6::SaveState(PDXFILE& file) const
{
	PDX_TRY( square[0].SaveState( file ) );
	PDX_TRY( square[1].SaveState( file ) );
	PDX_TRY( sawtooth.SaveState( file )  );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDVRC6::SQUARE::LoadState(PDXFILE& file)
{
	PDX_TRY(APU::CHANNEL::LoadState(file));

	if (!file.Readable(sizeof(U8) * 11))
		return PDX_FAILURE;

	duty           = file.Read<U8>();
	step           = file.Read<U8>();
	volume         = file.Read<I32>();
	digitized      = file.Read<U8>();
	WaveLengthLow  = file.Read<U16>();
	WaveLengthHigh = file.Read<U16>();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDVRC6::SQUARE::SaveState(PDXFILE& file) const
{
	PDX_TRY(APU::CHANNEL::SaveState(file));

	file << U8  ( duty           );
	file << U8  ( step           );
	file << I32 ( volume         );
	file << U8  ( digitized      );
	file << U16 ( WaveLengthLow  );
	file << U16 ( WaveLengthHigh );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDVRC6::SAWTOOTH::LoadState(PDXFILE& file)
{
	PDX_TRY(APU::CHANNEL::LoadState(file));

	if (!file.Readable(sizeof(U8) * 9))
		return PDX_FAILURE;

	WaveLengthLow  = file.Read<U16>();
	WaveLengthHigh = file.Read<U16>();
	phase          = file.Read<I32>();
	step           = file.Read<U8>();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SNDVRC6::SAWTOOTH::SaveState(PDXFILE& file) const
{
	PDX_TRY(APU::CHANNEL::SaveState(file));

	file << U16 ( WaveLengthLow  );
	file << U16 ( WaveLengthHigh );
	file << I32 ( phase          );
	file << U8  ( step           );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDVRC6::WriteSquare1Reg0  (const UINT data) { apu->Update(); square[0].WriteReg0 ( data ); }
VOID SNDVRC6::WriteSquare1Reg1  (const UINT data) { apu->Update(); square[0].WriteReg1 ( data ); }
VOID SNDVRC6::WriteSquare1Reg2  (const UINT data) { apu->Update(); square[0].WriteReg2 ( data ); }
VOID SNDVRC6::WriteSquare2Reg0  (const UINT data) { apu->Update(); square[1].WriteReg0 ( data ); }
VOID SNDVRC6::WriteSquare2Reg1  (const UINT data) { apu->Update(); square[1].WriteReg1 ( data ); }
VOID SNDVRC6::WriteSquare2Reg2  (const UINT data) { apu->Update(); square[1].WriteReg2 ( data ); }
VOID SNDVRC6::WriteSawToothReg0 (const UINT data) { apu->Update(); sawtooth.WriteReg0  ( data ); }
VOID SNDVRC6::WriteSawToothReg1 (const UINT data) { apu->Update(); sawtooth.WriteReg1  ( data ); }
VOID SNDVRC6::WriteSawToothReg2 (const UINT data) { apu->Update(); sawtooth.WriteReg2  ( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SNDVRC6::SQUARE::Reset()
{
	APU::CHANNEL::Reset();

	WaveLengthLow = 0;
	WaveLengthHigh = 0;
	digitized = 0;
	volume = 0;
	step = 0;
	duty = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDVRC6::SQUARE::WriteReg0(const UINT data)
{
	REG0 reg;
	reg.latch = data;

	duty      = reg.duty + 1;
	volume    = NES_APU_OUTPUT(reg.volume);
	digitized = reg.digitized;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDVRC6::SQUARE::WriteReg1(const UINT data)
{
	WaveLengthLow = data;
	frequency     = NES_APU_TO_FIXED(WaveLengthLow + WaveLengthHigh + 1);
	active        = emulate && enabled && frequency;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDVRC6::SQUARE::WriteReg2(const UINT data)
{
	REG2 reg;
	reg.latch = data;

	WaveLengthHigh = reg.WaveLengthHigh << 8;
	frequency      = NES_APU_TO_FIXED(WaveLengthLow + WaveLengthHigh + 1);
	enabled        = reg.enable;
	active         = emulate && enabled && frequency;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG SNDVRC6::SQUARE::Sample()
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
			else if (step == duty)
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

VOID SNDVRC6::SAWTOOTH::Reset()
{
	APU::CHANNEL::Reset();

	WaveLengthLow = 0;
	WaveLengthHigh = 0;
	phase = 0;
	step = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDVRC6::SAWTOOTH::WriteReg0(const UINT data)
{
	REG0 reg;
	reg.latch = data;
	phase = NES_APU_OUTPUT(reg.phase) >> 3;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDVRC6::SAWTOOTH::WriteReg1(const UINT data)
{
	WaveLengthLow = data;
	frequency     = NES_APU_TO_FIXED(WaveLengthLow + WaveLengthHigh + 1) << 1;
	active        = emulate && enabled && frequency;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID SNDVRC6::SAWTOOTH::WriteReg2(const UINT data)
{
	REG2 reg;
	reg.latch = data;

	WaveLengthHigh = reg.WaveLengthHigh << 8;
	frequency      = NES_APU_TO_FIXED(WaveLengthLow + WaveLengthHigh + 1) << 1;
	enabled        = reg.enable;
	active         = emulate && enabled && frequency;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LONG SNDVRC6::SAWTOOTH::Sample()
{
	if (active)
	{
		for (timer += rate; timer > 0; timer -= frequency)
		{
			amp += phase;

			if (++step == 7)
			{
				step = 0;
				amp = 0;
			}
		}
	}
	else
	{
		amp -= (amp >> 7);
	}

	return amp;
}

NES_NAMESPACE_END
