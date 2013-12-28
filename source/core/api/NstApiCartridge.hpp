////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
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
#include <vector>
#include "NstApiInput.hpp"
#include "NstApiMachine.hpp"

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
			struct ChooseProfileCaller;

		public:

			template<typename T>
			Cartridge(T& e)
			: Base(e) {}

			struct Profile
			{
				Profile() throw();
				~Profile() throw();

				class Hash : public Core::ImplicitBool<Hash>
				{
				public:

					enum
					{
						SHA1_LENGTH = 40,
						SHA1_WORD_LENGTH = SHA1_LENGTH / 8,
						CRC32_LENGTH = 8,
						CRC32_WORD_LENGTH = CRC32_LENGTH / 8
					};

					Hash() throw();
					Hash(const char*,const char*) throw();
					Hash(const wchar_t*,const wchar_t*) throw();
					Hash(const dword*,dword) throw();

					bool operator <  (const Hash&) const throw();
					bool operator == (const Hash&) const throw();
					bool operator ! () const throw();

					void Compute(const void*,ulong) throw();
					void Assign(const char*,const char*) throw();
					void Assign(const wchar_t*,const wchar_t*) throw();
					void Assign(const dword*,dword) throw();
					void Get(char*,char*) const throw();
					void Clear() throw();

					const dword* GetSha1() const throw();
					dword GetCrc32() const throw();

				private:

					template<typename T>
					void Import(const T*,const T*);

					template<typename T>
					static bool Set(dword&,const T* NST_RESTRICT);

					dword data[CRC32_WORD_LENGTH+SHA1_WORD_LENGTH];
				};

				struct Game
				{
					Game() throw();

					std::wstring title;
					std::wstring altTitle;
					std::wstring clss;
					std::wstring subClss;
					std::wstring catalog;
					std::wstring publisher;
					std::wstring developer;
					std::wstring portDeveloper;
					std::wstring region;
					std::wstring revision;
					Input::Adapter adapter;
					Input::Type controllers[5];
					uint players;
				};

				struct Dump
				{
					Dump() throw();

					enum State
					{
						OK,
						BAD,
						UNKNOWN
					};

					std::wstring by;
					std::wstring date;
					State state;
				};

				struct System
				{
					System() throw();

					enum Type
					{
						NES_NTSC      = Core::SYSTEM_NES_NTSC,
						NES_PAL       = Core::SYSTEM_NES_PAL,
						NES_PAL_A     = Core::SYSTEM_NES_PAL_A,
						NES_PAL_B     = Core::SYSTEM_NES_PAL_B,
						FAMICOM       = Core::SYSTEM_FAMICOM,
						VS_UNISYSTEM  = Core::SYSTEM_VS_UNISYSTEM,
						VS_DUALSYSTEM = Core::SYSTEM_VS_DUALSYSTEM,
						PLAYCHOICE_10 = Core::SYSTEM_PLAYCHOICE_10
					};

					enum Cpu
					{
						CPU_RP2A03 = Core::CPU_RP2A03,
						CPU_RP2A07 = Core::CPU_RP2A07
					};

					enum Ppu
					{
						PPU_RP2C02      = Core::PPU_RP2C02,
						PPU_RP2C03B     = Core::PPU_RP2C03B,
						PPU_RP2C03G     = Core::PPU_RP2C03G,
						PPU_RP2C04_0001 = Core::PPU_RP2C04_0001,
						PPU_RP2C04_0002 = Core::PPU_RP2C04_0002,
						PPU_RP2C04_0003 = Core::PPU_RP2C04_0003,
						PPU_RP2C04_0004 = Core::PPU_RP2C04_0004,
						PPU_RC2C03B     = Core::PPU_RC2C03B,
						PPU_RC2C03C     = Core::PPU_RC2C03C,
						PPU_RC2C05_01   = Core::PPU_RC2C05_01,
						PPU_RC2C05_02   = Core::PPU_RC2C05_02,
						PPU_RC2C05_03   = Core::PPU_RC2C05_03,
						PPU_RC2C05_04   = Core::PPU_RC2C05_04,
						PPU_RC2C05_05   = Core::PPU_RC2C05_05,
						PPU_RP2C07      = Core::PPU_RP2C07
					};

					Type type;
					Cpu cpu;
					Ppu ppu;
				};

				struct Property
				{
					std::wstring name;
					std::wstring value;
				};

				typedef std::vector<Property> Properties;

				class Board
				{
					template<typename T>
					dword GetComponentSize(const T&) const;

					template<typename T>
					bool HasComponentBattery(const T&) const;

				public:

					Board() throw();
					~Board() throw();

					dword GetPrg() const throw();
					dword GetChr() const throw();
					dword GetWram() const throw();
					dword GetVram() const throw();
					bool HasBattery() const throw();
					bool HasWramBattery() const throw();
					bool HasMmcBattery() const throw();

					enum
					{
						SOLDERPAD_H = 0x1,
						SOLDERPAD_V = 0x2,
						NO_MAPPER = 0xFFFF
					};

					struct Pin
					{
						Pin() throw();

						uint number;
						std::wstring function;
					};

					typedef std::vector<Pin> Pins;

					struct Sample
					{
						Sample() throw();

						uint id;
						std::wstring file;
					};

					typedef std::vector<Sample> Samples;

					struct Rom
					{
						Rom() throw();

						dword id;
						dword size;
						std::wstring name;
						std::wstring file;
						std::wstring package;
						Pins pins;
						Hash hash;
					};

					struct Ram
					{
						Ram() throw();

						dword id;
						dword size;
						std::wstring file;
						std::wstring package;
						Pins pins;
						bool battery;
					};

					struct Chip
					{
						Chip() throw();

						std::wstring type;
						std::wstring file;
						std::wstring package;
						Pins pins;
						Samples samples;
						bool battery;
					};

					typedef std::vector<Rom> Roms;
					typedef std::vector<Ram> Rams;
					typedef std::vector<Chip> Chips;
					typedef Roms Prg;
					typedef Roms Chr;
					typedef Rams Wram;
					typedef Rams Vram;

					std::wstring type;
					std::wstring cic;
					std::wstring pcb;
					Prg prg;
					Chr chr;
					Wram wram;
					Vram vram;
					Chips chips;
					uint solderPads;
					uint mapper;
				};

				Hash hash;
				Dump dump;
				Game game;
				System system;
				Board board;
				Properties properties;
				bool multiRegion;
			};

			class Database
			{
				Core::Machine& emulator;

				bool Create();

			public:

				Database(Core::Machine& e)
				: emulator(e) {}

				class Entry : public Core::ImplicitBool<Entry>
				{
				public:

					Result GetProfile(Profile&) const throw();

					const wchar_t*        GetTitle      () const throw();
					const wchar_t*        GetRegion     () const throw();
					Profile::System::Type GetSystem     () const throw();
					bool                  IsMultiRegion () const throw();
					const Profile::Hash*  GetHash       () const throw();
					dword                 GetPrgRom     () const throw();
					dword                 GetChrRom     () const throw();
					uint                  GetWram       () const throw();
					uint                  GetVram       () const throw();
					uint                  GetMapper     () const throw();
					bool                  HasBattery    () const throw();
					Profile::Dump::State  GetDumpState  () const throw();

				private:

					friend class Database;
					const void* ref;

					Entry(const void* r)
					: ref(r) {}

				public:

					Entry()
					: ref(NULL) {}

					bool operator ! () const
					{
						return !ref;
					}
				};

				Result Load(std::istream&) throw();
				Result Load(std::istream&,std::istream&) throw();
				void   Unload() throw();
				Result Enable(bool=true) throw();
				bool   IsEnabled() const throw();
				bool   IsLoaded() const throw();
				Entry  FindEntry(const Profile::Hash&,Machine::FavoredSystem) const throw();
				Entry  FindEntry(const void*,ulong,Machine::FavoredSystem) const throw();
			};

			struct NesHeader
			{
				NesHeader() throw();

				void Clear() throw();

				Result Import(const void*,ulong) throw();
				Result Export(void*,ulong) const throw();

				enum
				{
					MAX_PRG_ROM = 0x4000 * 0xFFFUL,
					MAX_CHR_ROM = 0x2000 * 0xFFFUL
				};

				enum Region
				{
					REGION_NTSC = 1,
					REGION_PAL,
					REGION_BOTH
				};

				enum System
				{
					SYSTEM_CONSOLE,
					SYSTEM_VS,
					SYSTEM_PC10
				};

				enum Ppu
				{
					PPU_RP2C02      = Core::PPU_RP2C02,
					PPU_RP2C03B     = Core::PPU_RP2C03B,
					PPU_RP2C03G     = Core::PPU_RP2C03G,
					PPU_RP2C04_0001 = Core::PPU_RP2C04_0001,
					PPU_RP2C04_0002 = Core::PPU_RP2C04_0002,
					PPU_RP2C04_0003 = Core::PPU_RP2C04_0003,
					PPU_RP2C04_0004 = Core::PPU_RP2C04_0004,
					PPU_RC2C03B     = Core::PPU_RC2C03B,
					PPU_RC2C03C     = Core::PPU_RC2C03C,
					PPU_RC2C05_01   = Core::PPU_RC2C05_01,
					PPU_RC2C05_02   = Core::PPU_RC2C05_02,
					PPU_RC2C05_03   = Core::PPU_RC2C05_03,
					PPU_RC2C05_04   = Core::PPU_RC2C05_04,
					PPU_RC2C05_05   = Core::PPU_RC2C05_05,
					PPU_RP2C07      = Core::PPU_RP2C07
				};

				enum Mirroring
				{
					MIRRORING_HORIZONTAL,
					MIRRORING_VERTICAL,
					MIRRORING_FOURSCREEN,
					MIRRORING_SINGLESCREEN,
					MIRRORING_CONTROLLED
				};

				System system;
				Region region;
				dword prgRom;
				dword prgRam;
				dword prgNvRam;
				dword chrRom;
				dword chrRam;
				dword chrNvRam;
				Ppu ppu;
				Mirroring mirroring;
				ushort mapper;
				uchar subMapper;
				uchar version;
				uchar security;
				bool trainer;
			};

			const Profile* GetProfile() const throw();

			static Result ReadRomset(std::istream&,Machine::FavoredSystem,bool,Profile&) throw();
			static Result ReadInes(std::istream&,Machine::FavoredSystem,Profile&) throw();
			static Result ReadUnif(std::istream&,Machine::FavoredSystem,Profile&) throw();

			Database GetDatabase() throw()
			{
				return emulator;
			}

			enum
			{
				CHOOSE_DEFAULT_PROFILE = INT_MAX
			};

			typedef uint (NST_CALLBACK *ChooseProfileCallback) (UserData,const Profile*,const std::wstring*,uint);

			static ChooseProfileCaller chooseProfileCallback;
		};

		struct Cartridge::ChooseProfileCaller : Core::UserCallback<Cartridge::ChooseProfileCallback>
		{
			uint operator () (const Profile* profiles,const std::wstring* names,uint count) const
			{
				return function ? function( userdata, profiles, names, count ) : CHOOSE_DEFAULT_PROFILE;
			}
		};
	}
}

#if NST_MSVC >= 1200
#pragma warning( pop )
#endif

#endif
