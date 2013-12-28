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

#ifndef NST_API_CARTRIDGE_H
#define NST_API_CARTRIDGE_H

#include <iosfwd>
#include <string>
#include "NstApiInput.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		class Cartridge : public Base
		{
		public:

			template<typename T>
			Cartridge(T& e)
			: Base(e) {}

			enum System
			{
				SYSTEM_HOME,
				SYSTEM_VS,
				SYSTEM_PC10
			};

			enum Region
			{
				REGION_NTSC = 1,
				REGION_PAL,
				REGION_BOTH
			};

			enum Cpu
			{
				CPU_RP2A03,
				CPU_RP2A07
			};

			enum Ppu
			{
				PPU_RP2C02,
				PPU_RP2C03B,
				PPU_RP2C03G,
				PPU_RP2C04_0001,
				PPU_RP2C04_0002,
				PPU_RP2C04_0003,
				PPU_RP2C04_0004,
				PPU_RC2C03B,
				PPU_RC2C03C,
				PPU_RC2C05_01,
				PPU_RC2C05_02,
				PPU_RC2C05_03,
				PPU_RC2C05_04,
				PPU_RC2C05_05,
				PPU_RP2C07
			};

			enum Mirroring
			{
				MIRROR_HORIZONTAL,
				MIRROR_VERTICAL,
				MIRROR_FOURSCREEN,
				MIRROR_ZERO,
				MIRROR_ONE,
				MIRROR_CONTROLLED
			};

			enum Condition
			{
				DUMP_OK,
				DUMP_REPAIRABLE,
				DUMP_BAD,
				DUMP_UNKNOWN
			};

			struct Setup
			{
				Setup() throw();

				void Clear() throw();

				System system;
				Region region;
				ulong prgRom;
				ulong wrkRam;
				ulong wrkRamBacked;
				ulong chrRom;
				ulong chrRam;
				ulong chrRamBacked;
				Cpu cpu;
				Ppu ppu;
				Mirroring mirroring;
				ushort mapper;
				ushort subMapper;
				uchar security;
				uchar version;
				bool trainer;
				bool wrkRamAuto;
			};

			static Result NST_CALL ReadNesHeader(Setup&,const void*,ulong) throw();
			static Result NST_CALL WriteNesHeader(const Setup&,void*,ulong) throw();

			class Database
			{
				Core::Machine& emulator;

				bool Create();

			public:

				Database(Core::Machine& e)
				: emulator(e) {}

				typedef const void* Entry;

				Result Load(std::istream&) throw();
				void   Unload() throw();
				Result Enable(bool=true) throw();
				bool   IsEnabled() const throw();
				bool   IsLoaded() const throw();
				Entry  FindEntry(ulong) const throw();
				Entry  FindEntry(const void*,ulong) const throw();

				System    GetSystem       (Entry) const throw();
				Region    GetRegion       (Entry) const throw();
				Mirroring GetMirroring    (Entry) const throw();
				ulong     GetCrc          (Entry) const throw();
				ulong     GetPrgRom       (Entry) const throw();
				ulong     GetWrkRam       (Entry) const throw();
				ulong     GetWrkRamBacked (Entry) const throw();
				ulong     GetChrRom       (Entry) const throw();
				ulong     GetChrRam       (Entry) const throw();
				ulong     GetChrRamBacked (Entry) const throw();
				uint      GetMapper       (Entry) const throw();
				Condition GetCondition    (Entry) const throw();
				bool      HasTrainer      (Entry) const throw();
			};

			Database GetDatabase() throw()
			{
				return emulator;
			}

			struct Info
			{
				Info() throw();
				~Info() throw();

				void Clear() throw();

				Setup          setup;
				std::string    name;
				std::string    maker;
				std::string    board;
				ulong          crc;
				ulong          prgCrc;
				ulong          chrCrc;
				Input::Type    controllers[5];
				Input::Adapter adapter;
				Condition      condition;
			};

			const Info* GetInfo() const throw();
		};
	}
}

#if NST_MSVC >= 1200
#pragma warning( pop )
#endif

#endif
