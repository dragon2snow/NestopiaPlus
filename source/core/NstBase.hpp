////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#ifndef NST_BASE_H
#define NST_BASE_H

#include "api/NstApiConfig.hpp"

//--------------------------------------------------------------------------------------
// Microsoft Visual C++
//--------------------------------------------------------------------------------------

#ifdef _MSC_VER
#define NST_MSVC _MSC_VER
#else
#define NST_MSVC 0
#endif

//--------------------------------------------------------------------------------------
// Intel C/C++ Compiler
//--------------------------------------------------------------------------------------

#ifdef __INTEL_COMPILER
#define NST_ICC __INTEL_COMPILER
#else
#define NST_ICC 0
#endif

//--------------------------------------------------------------------------------------
// GNU Compiler Collection
//--------------------------------------------------------------------------------------

#ifdef __GNUC__
#define NST_GCC (__GNUC__ * 100 + __GNUC_MINOR__)
#else
#define NST_GCC 0
#endif

//--------------------------------------------------------------------------------------
// Borland C++
//--------------------------------------------------------------------------------------

#ifdef __BORLANDC__
#define NST_BCB __BORLANDC__
#else
#define NST_BCB 0
#endif

//--------------------------------------------------------------------------------------
// Metrowerks CodeWarrior
//--------------------------------------------------------------------------------------

#ifdef __MWERKS__
#define NST_MWERKS __MWERKS__
#else
#define NST_MWERKS 0
#endif

//--------------------------------------------------------------------------------------

#ifdef NST_PRAGMA_ONCE
#pragma once
#elif NST_MSVC >= 1020 || NST_MWERKS >= 0x3000
#pragma once
#define NST_PRAGMA_ONCE
#endif

//--------------------------------------------------------------------------------------

#ifndef NST_CALL
#define NST_CALL
#endif

namespace Nes
{
	typedef signed char schar;
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;
	typedef unsigned long ulong;

	enum Result
	{
		RESULT_ERR_WRONG_MODE               = -13,
		RESULT_ERR_MISSING_BIOS             = -12,
		RESULT_ERR_UNSUPPORTED_MAPPER       = -11,
		RESULT_ERR_UNSUPPORTED_VSSYSTEM     = -10,
		RESULT_ERR_UNSUPPORTED_FILE_VERSION =  -9,
		RESULT_ERR_UNSUPPORTED              =  -8,
		RESULT_ERR_INVALID_CRC              =  -7,
		RESULT_ERR_CORRUPT_FILE             =  -6,
		RESULT_ERR_INVALID_FILE             =  -5,
		RESULT_ERR_INVALID_PARAM            =  -4,
		RESULT_ERR_NOT_READY                =  -3,
		RESULT_ERR_OUT_OF_MEMORY            =  -2,
		RESULT_ERR_GENERIC                  =  -1,
		RESULT_OK                           =   0,
		RESULT_NOP                          =  +1,
		RESULT_WARN_BAD_DUMP                =  +2,
		RESULT_WARN_BAD_PROM                =  +3,
		RESULT_WARN_BAD_CROM                =  +4,
		RESULT_WARN_BAD_FILE_HEADER         =  +5,
		RESULT_WARN_SAVEDATA_LOST           =  +6,
		RESULT_WARN_ENCRYPTED_ROM           =  +7,
		RESULT_WARN_INCORRECT_FILE_HEADER   =  +8,
		RESULT_WARN_DATA_REPLACED           =  +9
	};

	namespace Clocks
	{
		enum
		{
			M2_MUL      = 6,
			NTSC_DIV    = 11,
			NTSC_CLK    = 39375000UL * M2_MUL,
			NTSC_HVSYNC = 525UL * 455 * NTSC_DIV * M2_MUL / 4,
			PAL_DIV     = 8,
			PAL_CLK     = 35468950UL * M2_MUL,
			PAL_HVSYNC  = 625UL * 1418758 / (10000/PAL_DIV) * M2_MUL
		};

		enum
		{
			RP2A03_CC = 12,
			RP2A07_CC = 16
		};

		enum
		{
			RP2C02_CC        = 4,
			RP2C02_HACTIVE   = RP2C02_CC * 256,
			RP2C02_HBLANK    = RP2C02_CC * 85,
			RP2C02_HSYNC     = RP2C02_HACTIVE + RP2C02_HBLANK,
			RP2C02_VACTIVE   = 240,
			RP2C02_VSLEEP    = 1,
			RP2C02_VINT      = 20,
			RP2C02_VDUMMY    = 1,
			RP2C02_VBLANK    = RP2C02_VSLEEP + RP2C02_VINT + RP2C02_VDUMMY,
			RP2C02_VSYNC     = RP2C02_VACTIVE + RP2C02_VBLANK,
			RP2C02_HVINT     = RP2C02_VINT * ulong(RP2C02_HSYNC),
			RP2C02_HVSYNC_0  = RP2C02_VSYNC * ulong(RP2C02_HSYNC),
			RP2C02_HVSYNC_1  = RP2C02_VSYNC * ulong(RP2C02_HSYNC) - RP2C02_CC,
			RP2C02_HVSYNC    = (RP2C02_HVSYNC_0 + ulong(RP2C02_HVSYNC_1)) / 2,
			RP2C02_FPS       = (NTSC_CLK + NTSC_DIV * ulong(RP2C02_HVSYNC) / 2) / (NTSC_DIV * ulong(RP2C02_HVSYNC)),
			RP2C07_CC        = 5,
			RP2C07_HACTIVE   = RP2C07_CC * 256,
			RP2C07_HBLANK    = RP2C07_CC * 85,
			RP2C07_HSYNC     = RP2C07_HACTIVE + RP2C07_HBLANK,
			RP2C07_VACTIVE   = 240,
			RP2C07_VSLEEP    = 1,
			RP2C07_VINT      = 70,
			RP2C07_VDUMMY    = 1,
			RP2C07_VBLANK    = RP2C07_VSLEEP + RP2C07_VINT + RP2C07_VDUMMY,
			RP2C07_VSYNC     = RP2C07_VACTIVE + RP2C07_VBLANK,
			RP2C07_HVINT     = RP2C07_VINT * ulong(RP2C07_HSYNC),
			RP2C07_HVSYNC    = RP2C07_VSYNC * ulong(RP2C07_HSYNC),
			RP2C07_FPS       = (PAL_CLK + PAL_DIV * ulong(RP2C07_HVSYNC) / 2) / (PAL_DIV * ulong(RP2C07_HVSYNC))
		};
	}
}

#define NES_FAILED(x_) ((x_) < Nes::RESULT_OK)
#define NES_SUCCEEDED(x_) ((x_) >= Nes::RESULT_OK)

#endif
