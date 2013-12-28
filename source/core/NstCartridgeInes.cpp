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

#include <cstring>
#include "NstStream.hpp"
#include "NstLog.hpp"
#include "NstCrc32.hpp"
#include "NstImageDatabase.hpp"
#include "NstCartridge.hpp"
#include "NstCartridgeInes.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Cartridge::Ines::Ines
		(
			StdStream s,
			Ram& p,
			Ram& c,
			Ram& w,
			Api::Cartridge::Info& i,
			const ImageDatabase* d,
			ImageDatabase::Handle& h
		)
		:
		stream         (s),
		prg            (p),
		chr            (c),
		wrk            (w),
		info           (i),
		database       (d),
		databaseHandle (h)
		{
			NST_COMPILE_ASSERT
			(
				PPU_RP2C03B     ==  1 &&
				PPU_RP2C03G     ==  2 &&
				PPU_RP2C04_0001 ==  3 &&
				PPU_RP2C04_0002 ==  4 &&
				PPU_RP2C04_0003 ==  5 &&
				PPU_RP2C04_0004 ==  6 &&
				PPU_RC2C03B     ==  7 &&
				PPU_RC2C03C     ==  8 &&
				PPU_RC2C05_01   ==  9 &&
				PPU_RC2C05_02   == 10 &&
				PPU_RC2C05_03   == 11 &&
				PPU_RC2C05_04   == 12 &&
				PPU_RC2C05_05   == 13
			);

			result = Collect();

			if (NES_FAILED(result))
				throw RESULT_ERR_CORRUPT_FILE;

			const dword prgSkip = Process();

			if (!info.setup.prgRom)
				throw RESULT_ERR_CORRUPT_FILE;

			if (info.setup.mapper > 255)
				throw RESULT_ERR_UNSUPPORTED_MAPPER;

			wrk.Set( info.setup.wrkRam + info.setup.wrkRamBacked );

			if (info.setup.trainer)
				stream.Read( wrk.Mem() + TRAINER_BEGIN, TRAINER_LENGTH );

			prg.Set( info.setup.prgRom );
			stream.Read( prg.Mem(), info.setup.prgRom );

			if (prgSkip)
				stream.Seek( prgSkip );

			chr.Set( info.setup.chrRom );

			if (info.setup.chrRom)
				stream.Read( chr.Mem(), info.setup.chrRom );
		}

		Result Cartridge::Ines::Collect()
		{
			info.Clear();

			byte header[16];
			stream.Read( header );

			return ReadHeader( info.setup, header, 16 );
		}

		Result Cartridge::Ines::ReadHeader(Api::Cartridge::Setup& setup,const byte* const file,const ulong length)
		{
			if (file == NULL)
				return RESULT_ERR_INVALID_PARAM;

			if
			(
				length < 4 ||
				file[0] != Ascii<'N'>::V ||
				file[1] != Ascii<'E'>::V ||
				file[2] != Ascii<'S'>::V ||
				file[3] != 0x1A
			)
				return RESULT_ERR_INVALID_FILE;

			if (length < 16)
				return RESULT_ERR_CORRUPT_FILE;

			byte header[16];
			std::memcpy( header, file, 16 );

			Result result = RESULT_OK;

			setup.version = ((header[7] & 0xCU) == 0x8 ? 2 : 0);

			if (!setup.version)
			{
				for (uint i=10; i < 16; ++i)
				{
					if (header[i])
					{
						header[7] = 0;
						header[8] = 0;
						header[9] = 0;
						result = RESULT_WARN_BAD_FILE_HEADER;
						break;
					}
				}
			}

			setup.prgRom = header[4];
			setup.chrRom = header[5];

			if (setup.version)
			{
				setup.prgRom |= uint(header[9]) << 8 & 0xF00;
				setup.chrRom |= uint(header[9]) << 4 & 0xF00;
			}

			setup.prgRom *= SIZE_16K;
			setup.chrRom *= SIZE_8K;

			setup.trainer = bool(header[6] & 0x4U);

			setup.mapper = (header[6] >> 4) | (header[7] & 0xF0U);
			setup.subMapper = 0;

			if (setup.version)
			{
				setup.mapper |= uint(header[8]) << 8 & 0x100;
				setup.subMapper = header[8] >> 4;
			}

			if (header[6] & 0x8U)
			{
				setup.mirroring = Api::Cartridge::MIRROR_FOURSCREEN;
			}
			else if (header[6] & 0x1U)
			{
				setup.mirroring = Api::Cartridge::MIRROR_VERTICAL;
			}
			else
			{
				setup.mirroring = Api::Cartridge::MIRROR_HORIZONTAL;
			}

			setup.security = 0;

			if (header[7] & 0x1U)
			{
				setup.system = SYSTEM_VS;
				setup.ppu = PPU_RP2C03B;

				if (setup.version)
				{
					if ((header[13] & 0xFU) < 13)
						setup.ppu = static_cast<PpuType>((header[13] & 0xFU) + 1);

					if ((header[13] >> 4) < 4)
						setup.security = header[13] >> 4;
				}
			}
			else if (setup.version && (header[7] & 0x2U))
			{
				setup.system = SYSTEM_PC10;
				setup.ppu = PPU_RP2C03B;
			}
			else
			{
				setup.system = SYSTEM_HOME;
				setup.ppu = PPU_RP2C02;
			}

			if (setup.version && (header[12] & 0x2U))
			{
				setup.region = REGION_BOTH;
			}
			else if (header[setup.version ? 12 : 9] & 0x1U)
			{
				setup.region = REGION_PAL;
			}
			else
			{
				setup.region = REGION_NTSC;
			}

			if (setup.version)
			{
				setup.wrkRam       = ((header[10]) & 0xFU) - 1U < 14 ? 64UL << (header[10] & 0xFU) : 0;
				setup.wrkRamBacked = ((header[10]) >>   4) - 1U < 14 ? 64UL << (header[10] >>   4) : 0;
				setup.chrRam       = ((header[11]) & 0xFU) - 1U < 14 ? 64UL << (header[11] & 0xFU) : 0;
				setup.chrRamBacked = ((header[11]) >>   4) - 1U < 14 ? 64UL << (header[11] >>   4) : 0;
			}
			else
			{
				setup.wrkRam       = ((header[6] & 0x2U) ? 0 : header[8] * dword(SIZE_8K));
				setup.wrkRamBacked = ((header[6] & 0x2U) ? NST_MAX(header[8],1U) * dword(SIZE_8K) : 0);
				setup.chrRam       = (setup.chrRom ? 0 : SIZE_8K);
				setup.chrRamBacked = 0;
			}

			return result;
		}

		Result Cartridge::Ines::WriteHeader(const Api::Cartridge::Setup& setup,byte* const file,const ulong length)
		{
			if
			(
				(file == NULL || length < 16) ||
				(setup.prgRom > (setup.version ? 0xFFFUL * SIZE_16K : 0xFFUL * SIZE_16K)) ||
				(setup.chrRom > (setup.version ? 0xFFFUL * SIZE_8K : 0xFFUL * SIZE_8K)) ||
				(setup.mapper > (setup.version ? 0x1FFUL : 0xFF)) ||
				(setup.version && setup.subMapper > 0xF)
			)
				return RESULT_ERR_INVALID_PARAM;

			byte header[16] =
			{
				Ascii<'N'>::V,
				Ascii<'E'>::V,
				Ascii<'S'>::V,
				0x1A,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00
			};

			if (setup.version)
				header[7] |= 0x8U;

			header[4] = (setup.prgRom / SIZE_16K) & 0xFF;
			header[5] = (setup.chrRom / SIZE_8K) & 0xFF;

			if (setup.version)
			{
				header[9] |= (setup.prgRom / SIZE_16K) >> 8;
				header[9] |= (setup.chrRom / SIZE_8K) >> 4 & 0xF0;
			}

			if (setup.mirroring == Api::Cartridge::MIRROR_FOURSCREEN)
			{
				header[6] |= 0x8U;
			}
			else if (setup.mirroring == Api::Cartridge::MIRROR_VERTICAL)
			{
				header[6] |= 0x1U;
			}

			if (setup.wrkRamBacked)
				header[6] |= 0x2U;

			if (setup.trainer)
				header[6] |= 0x4U;

			if (setup.system == SYSTEM_VS)
			{
				header[7] |= 0x1U;
			}
			else if (setup.version && setup.system == SYSTEM_PC10)
			{
				header[7] |= 0x2U;
			}

			header[6] |= setup.mapper << 4 & 0xF0U;
			header[7] |= setup.mapper & 0xF0U;

			if (setup.version)
			{
				header[8] |= setup.mapper >> 8;
				header[8] |= setup.subMapper << 4;

				uint i, data;

				for (i=0, data=setup.wrkRam >> 7; data; data >>= 1, ++i)
				{
					if (i > 0xF)
						return RESULT_ERR_INVALID_PARAM;
				}

				header[10] |= i;

				for (i=0, data=setup.wrkRamBacked >> 7; data; data >>= 1, ++i)
				{
					if (i > 0xF)
						return RESULT_ERR_INVALID_PARAM;
				}

				header[10] |= i << 4;

				for (i=0, data=setup.chrRam >> 7; data; data >>= 1, ++i)
				{
					if (i > 0xF)
						return RESULT_ERR_INVALID_PARAM;
				}

				header[11] |= i;

				for (i=0, data=setup.chrRamBacked >> 7; data; data >>= 1, ++i);
				{
					if (i > 0xF)
						return RESULT_ERR_INVALID_PARAM;
				}

				header[11] |= i << 4;

				if (setup.region == REGION_BOTH)
				{
					header[12] |= 0x2U;
				}
				else if (setup.region == REGION_PAL)
				{
					header[12] |= 0x1U;
				}

				if (setup.system == SYSTEM_VS)
				{
					if (setup.ppu > 0xF || setup.security > 0xF)
						return RESULT_ERR_INVALID_PARAM;

					if (setup.ppu)
						header[13] = setup.ppu - 1;

					header[13] |= setup.security << 4;
				}
			}
			else
			{
				header[8] = setup.wrkRam / SIZE_8K;
				header[9] = (setup.region == REGION_PAL ? 0x1 : 0x0);
			}

			std::memcpy( file, header, 16 );

			return RESULT_OK;
		}

		dword Cartridge::Ines::Process()
		{
			Log log;

			static const char title[] = "Ines: ";

			if (info.setup.version)
				log << title << "version 2.0 detected" NST_LINEBREAK;

			if (result == RESULT_WARN_BAD_FILE_HEADER)
				log << title << "warning, unknown or invalid header data!" NST_LINEBREAK;

			log << title
				<< (info.setup.prgRom / SIZE_1K)
				<< "k PRG-ROM set" NST_LINEBREAK;

			if (info.setup.chrRom)
			{
				log << title
					<< (info.setup.chrRom / SIZE_1K)
					<< "k CHR-ROM set" NST_LINEBREAK;
			}

			if (info.setup.chrRam)
			{
				log << title
					<< (info.setup.chrRam % SIZE_1K ? info.setup.chrRam : info.setup.chrRam / SIZE_1K)
					<< (info.setup.chrRam % SIZE_1K ? " bytes" : "k")
					<< " CHR-RAM set" NST_LINEBREAK;
			}

			if (info.setup.chrRamBacked)
			{
				log << title
					<< (info.setup.chrRamBacked % SIZE_1K ? info.setup.chrRamBacked : info.setup.chrRamBacked / SIZE_1K)
					<< (info.setup.chrRamBacked % SIZE_1K ? " bytes" : "k")
					<< " battery-backed CHR-RAM set" NST_LINEBREAK;
			}

			if (info.setup.wrkRam)
			{
				log << title
					<< (info.setup.wrkRam % SIZE_1K ? info.setup.wrkRam : info.setup.wrkRam / SIZE_1K)
					<< (info.setup.wrkRam % SIZE_1K ? " byte" : "k")
					<< " W-RAM set" NST_LINEBREAK;
			}

			if (info.setup.wrkRamBacked)
			{
				log << title
					<< (info.setup.wrkRamBacked % SIZE_1K ? info.setup.wrkRamBacked : info.setup.wrkRamBacked / SIZE_1K)
					<< (info.setup.wrkRamBacked % SIZE_1K ? " byte" : "k")
					<< (info.setup.wrkRamBacked >= SIZE_1K ? " battery-backed W-RAM set" NST_LINEBREAK : " non-volatile W-RAM set" NST_LINEBREAK);
			}

			log << title <<
			(
				info.setup.mirroring == Api::Cartridge::MIRROR_FOURSCREEN ? "four-screen" :
				info.setup.mirroring == Api::Cartridge::MIRROR_VERTICAL   ? "vertical" :
																			"horizontal"
			) << " mirroring set" NST_LINEBREAK;

			log << title <<
			(
				info.setup.region == REGION_BOTH ? "NTSC/PAL" :
				info.setup.region == REGION_PAL  ? "PAL":
                                                   "NTSC"
			) << " set" NST_LINEBREAK;

			if (info.setup.system == SYSTEM_VS)
			{
				log << title << "VS System set" NST_LINEBREAK;

				if (info.setup.version)
				{
					if (info.setup.ppu)
					{
						static cstring const names[] =
						{
							"RP2C03B",
							"RP2C03G",
							"RP2C04-0001",
							"RP2C04-0002",
							"RP2C04-0003",
							"RP2C04-0004",
							"RC2C03B",
							"RC2C03C",
							"RC2C05-01",
							"RC2C05-02",
							"RC2C05-03",
							"RC2C05-04",
							"RC2C05-05"
						};

						NST_ASSERT( info.setup.ppu < 1+sizeof(array(names)) );
						log << title << names[info.setup.ppu-1] << " PPU set" NST_LINEBREAK;
					}

					if (info.setup.security)
					{
						static const cstring names[] =
						{
							"RBI Baseball",
							"TKO Boxing",
							"Super Xevious"
						};

						NST_ASSERT( info.setup.security < 1+sizeof(array(names)) );
						log << title << names[info.setup.security-1] << " VS mode set" NST_LINEBREAK;
					}
				}
			}
			else if (info.setup.system == SYSTEM_PC10)
			{
				log << title << "Playchoice 10 set" NST_LINEBREAK;
			}

			log << title << "mapper " << info.setup.mapper << " set";

			if (info.setup.system != SYSTEM_VS && (info.setup.mapper == VS_MAPPER_99 || info.setup.mapper == VS_MAPPER_151))
			{
				info.setup.system = SYSTEM_VS;
				info.setup.ppu = PPU_RP2C03B;
				log << ", forcing VS System";
			}

			log << NST_LINEBREAK;

			if (info.setup.version && info.setup.subMapper)
				log << title << "unknown sub-mapper " << info.setup.subMapper << " set" NST_LINEBREAK;

			if (info.setup.trainer)
				log << title << "trainer set" NST_LINEBREAK;

			databaseHandle = NULL;
			const dword prgSkip = database ? Repair( log ) : 0;

			if (info.setup.trainer && info.setup.wrkRamBacked+info.setup.wrkRam < SIZE_8K)
			{
				info.setup.wrkRam = SIZE_8K - info.setup.wrkRamBacked;
				log << title << "warning, forcing 8k of W-RAM for trainer" NST_LINEBREAK;
			}

			return prgSkip;
		}

		ImageDatabase::Handle Cartridge::Ines::SearchDatabase(const ImageDatabase& database,const byte* const stream,ulong maxlength)
		{
			ImageDatabase::Handle handle = NULL;

			if (stream && maxlength > 16+MIN_CRC_SEARCH_STRIDE)
			{
				maxlength -= 16;

				if (maxlength > MAX_CRC_SEARCH_LENGTH)
					maxlength = MAX_CRC_SEARCH_LENGTH;
				else
					maxlength -= maxlength % MIN_CRC_SEARCH_STRIDE;

				dword length =
				(
					(stream[4] | ((stream[7] & 0xCU) == 0x8 ? uint(stream[9]) << 8 & 0xF00UL : 0UL)) * SIZE_16K +
					(stream[5] | ((stream[7] & 0xCU) == 0x8 ? uint(stream[9]) << 4 & 0xF00UL : 0UL)) * SIZE_8K
				);

				if (length > maxlength)
					length = maxlength;

				const dword crc = Crc32::Compute( stream+16, length );

				handle = database.Search( crc );

				if (handle == NULL && maxlength > length)
					handle = database.Search( Crc32::Compute( stream+16+length, maxlength-length, crc ) );
			}

			return handle;
		}

		dword Cartridge::Ines::Repair(Log& log)
		{
			NST_ASSERT( database );

			ImageDatabase::Handle handle = NULL;

			for (dword count=0,crc=0xFFFFFFFF,crcIt=0xFFFFFFFF,checkpoint=info.setup.prgRom+info.setup.chrRom+1;;)
			{
				const uint data = stream.SafeRead8();

				if (data > 0xFF || ++count == MAX_CRC_SEARCH_LENGTH+1)
				{
					handle = database->Search( crc ^ 0xFFFFFFFF );
					stream.Seek( -idword(count) );
					break;
				}
				else if (count == checkpoint)
				{
					handle = database->Search( crc ^ 0xFFFFFFFF );

					if (handle)
					{
						stream.Seek( -idword(count) );
						break;
					}
				}

				crcIt = Crc32::Compute( crcIt, data );

				if (count % MIN_CRC_SEARCH_STRIDE == 0)
					crc = crcIt;
			}

			if (handle == NULL)
				return 0;

			databaseHandle = handle;

			NST_VERIFY( database->PrgRom(handle) );

			if (!info.setup.version && info.setup.system == SYSTEM_HOME && database->GetSystem(handle) == SYSTEM_PC10)
			{
				info.setup.system = SYSTEM_PC10;
				info.setup.ppu = PPU_RP2C03B;
			}

			if (!database->Enabled() || !database->PrgRom(handle))
				return 0;

			const dword prgSkip = database->PrgRomSkip(handle);
			const dword chrSkip = database->ChrRomSkip(handle);

			if (result == RESULT_OK)
			{
				if
				(
					(info.setup.prgRom != database->PrgRom(handle)+prgSkip && (info.setup.prgRom != database->PrgRom(handle) || database->ChrRom(handle))) ||
					(info.setup.chrRom != database->ChrRom(handle)+chrSkip && (info.setup.chrRom != database->ChrRom(handle)))
				)
					result = RESULT_WARN_INCORRECT_FILE_HEADER;
			}

			static const char title[] = "Database: warning, ";

			if (info.setup.prgRom != database->PrgRom(handle))
			{
				const dword prgRom = info.setup.prgRom;
				info.setup.prgRom = database->PrgRom(handle);

				log << title
					<< "changed PRG-ROM size: "
					<< (prgRom / SIZE_1K)
					<< "k to: "
					<< (info.setup.prgRom / SIZE_1K)
					<< "k" NST_LINEBREAK;
			}

			if (info.setup.chrRom != database->ChrRom(handle))
			{
				const dword chrRom = info.setup.chrRom;
				info.setup.chrRom = database->ChrRom(handle);

				log << title;

				if (chrSkip == SIZE_8K && info.setup.chrRom+chrSkip == chrRom && database->GetSystem(handle) == SYSTEM_PC10)
				{
					log << "ignored last 8k CHR-ROM" NST_LINEBREAK;
				}
				else
				{
					log << "changed CHR-ROM size: "
						<< (chrRom / SIZE_1K)
						<< "k to: "
						<< (info.setup.chrRom / SIZE_1K)
						<< "k" NST_LINEBREAK;
				}
			}

			if (info.setup.wrkRam != database->WrkRam(handle))
			{
				const dword wrkRam = info.setup.wrkRam;
				info.setup.wrkRam = database->WrkRam(handle);

				log << title
					<< "changed W-RAM: "
					<< (wrkRam % SIZE_1K ? wrkRam : wrkRam / SIZE_1K)
					<< (wrkRam % SIZE_1K ? " bytes to: " : "k to: ")
					<< (info.setup.wrkRam % SIZE_1K ? info.setup.wrkRam : info.setup.wrkRam / SIZE_1K)
					<< (info.setup.wrkRam % SIZE_1K ? " bytes" NST_LINEBREAK : "k" NST_LINEBREAK);
			}

			if (info.setup.wrkRamBacked != database->WrkRamBacked(handle))
			{
				const dword wrkRamBacked = info.setup.wrkRamBacked;
				info.setup.wrkRamBacked = database->WrkRamBacked(handle);

				log << title
					<< "changed non-volatile W-RAM: "
					<< (wrkRamBacked % SIZE_1K ? wrkRamBacked : wrkRamBacked / SIZE_1K)
					<< (wrkRamBacked % SIZE_1K ? " bytes to: " : "k to: ")
					<< (info.setup.wrkRamBacked % SIZE_1K ? info.setup.wrkRamBacked : info.setup.wrkRamBacked / SIZE_1K)
					<< (info.setup.wrkRamBacked % SIZE_1K ? " bytes" NST_LINEBREAK : "k" NST_LINEBREAK);
			}

			if (info.setup.mapper != database->Mapper(handle))
			{
				const uint mapper = info.setup.mapper;
				info.setup.mapper = database->Mapper(handle);

				if (result == RESULT_OK)
					result = RESULT_WARN_INCORRECT_FILE_HEADER;

				log << title
					<< "changed mapper "
					<< mapper
					<< " to "
					<< info.setup.mapper
					<< NST_LINEBREAK;
			}

			if (info.setup.mirroring != database->GetMirroring(handle))
			{
				const Api::Cartridge::Mirroring mirroring = info.setup.mirroring;
				info.setup.mirroring = database->GetMirroring(handle);

				if (result == RESULT_OK)
				{
					switch (info.setup.mirroring)
					{
						case Api::Cartridge::MIRROR_HORIZONTAL:
						case Api::Cartridge::MIRROR_VERTICAL:
						case Api::Cartridge::MIRROR_FOURSCREEN:

							result = RESULT_WARN_INCORRECT_FILE_HEADER;
							break;
					}
				}

				NST_COMPILE_ASSERT
				(
					Api::Cartridge::MIRROR_HORIZONTAL < 6 &&
					Api::Cartridge::MIRROR_VERTICAL   < 6 &&
					Api::Cartridge::MIRROR_FOURSCREEN < 6 &&
					Api::Cartridge::MIRROR_ZERO       < 6 &&
					Api::Cartridge::MIRROR_ONE        < 6 &&
					Api::Cartridge::MIRROR_CONTROLLED < 6
				);

				cstring types[6];

				types[ Api::Cartridge::MIRROR_HORIZONTAL ] = "horizontal mirroring";
				types[ Api::Cartridge::MIRROR_VERTICAL   ] = "vertical mirroring";
				types[ Api::Cartridge::MIRROR_FOURSCREEN ] = "four-screen mirroring";
				types[ Api::Cartridge::MIRROR_ZERO       ] = "$2000 mirroring";
				types[ Api::Cartridge::MIRROR_ONE        ] = "$2400 mirroring";
				types[ Api::Cartridge::MIRROR_CONTROLLED ] = "mapper controlled mirroring";

				log << title
					<< "changed "
					<< types[mirroring]
					<< " to "
					<< types[info.setup.mirroring]
					<< NST_LINEBREAK;
			}

			if (info.setup.trainer != database->Trainer(handle))
			{
				info.setup.trainer = database->Trainer(handle);

				if (result == RESULT_OK)
					result = RESULT_WARN_INCORRECT_FILE_HEADER;

				log << title << (info.setup.trainer ? "enabled trainer" NST_LINEBREAK : "ignored trainer" NST_LINEBREAK);
			}

			if (info.setup.system != database->GetSystem(handle))
			{
				const System system = info.setup.system;
				info.setup.system = database->GetSystem(handle);

				if (result == RESULT_OK && system != SYSTEM_PC10 && info.setup.system != SYSTEM_PC10)
					result = RESULT_WARN_INCORRECT_FILE_HEADER;

				log << title
					<< "changed "
					<< (system == SYSTEM_VS ? "VS" : system == SYSTEM_PC10 ? "Playchoice" : "home")
					<< " system to "
					<< (info.setup.system == SYSTEM_VS ? "VS" : info.setup.system == SYSTEM_PC10 ? "Playchoice" : "home")
					<< " system" NST_LINEBREAK;
			}

			if (info.setup.region != database->GetRegion(handle))
			{
				const Region region = info.setup.region;
				info.setup.region = database->GetRegion(handle);

				if (result == RESULT_OK && region != REGION_BOTH && info.setup.region != REGION_BOTH)
					result = RESULT_WARN_INCORRECT_FILE_HEADER;

				log << title
					<< "changed "
					<< (region == REGION_BOTH ? "NTSC/PAL" : region == REGION_PAL ? "PAL" : "NTSC")
					<< " to "
					<< (info.setup.region == REGION_BOTH ? "NTSC/PAL" : info.setup.region == REGION_PAL ? "PAL" : "NTSC")
					<< NST_LINEBREAK;
			}

			return prgSkip;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
