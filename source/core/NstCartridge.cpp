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
#include "NstLog.hpp"
#include "NstCrc32.hpp"
#include "NstImageDatabase.hpp"
#include "NstMapper.hpp"
#include "NstCartridge.hpp"
#include "NstCartridgeInes.hpp"
#include "NstCartridgeUnif.hpp"
#include "vssystem/NstVsSystem.hpp"
#include "NstPrpTurboFile.hpp"
#include "NstPrpDataRecorder.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Cartridge::Cartridge(Context& context)
		:
		Image        (CARTRIDGE),
		mapper       (NULL),
		vs           (NULL),
		turboFile    (NULL),
		dataRecorder (NULL)
		{
			NST_COMPILE_ASSERT
			(
				Revision::PPU_RP2C02      - Api::Cartridge::PPU_RP2C02      == 0 &&
				Revision::PPU_RP2C03B     - Api::Cartridge::PPU_RP2C03B     == 0 &&
				Revision::PPU_RP2C03G     - Api::Cartridge::PPU_RP2C03G     == 0 &&
				Revision::PPU_RP2C04_0001 - Api::Cartridge::PPU_RP2C04_0001 == 0 &&
				Revision::PPU_RP2C04_0002 - Api::Cartridge::PPU_RP2C04_0002 == 0 &&
				Revision::PPU_RP2C04_0003 - Api::Cartridge::PPU_RP2C04_0003 == 0 &&
				Revision::PPU_RP2C04_0004 - Api::Cartridge::PPU_RP2C04_0004 == 0 &&
				Revision::PPU_RC2C03B     - Api::Cartridge::PPU_RC2C03B     == 0 &&
				Revision::PPU_RC2C03C     - Api::Cartridge::PPU_RC2C03C     == 0 &&
				Revision::PPU_RC2C05_01   - Api::Cartridge::PPU_RC2C05_01   == 0 &&
				Revision::PPU_RC2C05_02   - Api::Cartridge::PPU_RC2C05_02   == 0 &&
				Revision::PPU_RC2C05_03   - Api::Cartridge::PPU_RC2C05_03   == 0 &&
				Revision::PPU_RC2C05_04   - Api::Cartridge::PPU_RC2C05_04   == 0 &&
				Revision::PPU_RC2C05_05   - Api::Cartridge::PPU_RC2C05_05   == 0 &&
				Revision::PPU_RP2C07      - Api::Cartridge::PPU_RP2C07      == 0
			);

			try
			{
				ImageDatabase::Handle databaseHandle = NULL;

				switch (Stream::In(context.stream).Peek32())
				{
					case INES_ID: context.result = Ines( context.stream, prg, chr, wrk, info, context.database, databaseHandle ).GetResult(); break;
					case UNIF_ID: context.result = Unif( context.stream, prg, chr, wrk, info, context.database, databaseHandle ).GetResult(); break;
					default: throw RESULT_ERR_INVALID_FILE;
				}

				if (context.database && databaseHandle)
				{
					info.condition = context.database->GetCondition(databaseHandle);

					if (info.condition == Api::Cartridge::DUMP_BAD)
						context.result = RESULT_WARN_BAD_DUMP;

					if (context.database->Encrypted(databaseHandle))
					{
						context.result = RESULT_WARN_ENCRYPTED_ROM;
						Log::Flush( "Cartridge: file is encrypted!" NST_LINEBREAK );
					}

					if (const uint id = context.database->Input(databaseHandle))
						DetectControllers( id );

					if (context.database->InputEx(databaseHandle) == ImageDatabase::INPUT_EX_TURBOFILE)
					{
						turboFile = new Peripherals::TurboFile( context.cpu );
						Log::Flush( "Cartridge: Turbo File storage device present" NST_LINEBREAK );
					}
				}

				if (info.board.empty())
					info.board = Mapper::GetBoard( info.setup.mapper );

				Mapper::Context settings
				(
					info.setup.mapper,
					context.cpu,
					context.apu,
					context.ppu,
					prg,
					chr,
					wrk,
					info.setup.mirroring == Api::Cartridge::MIRROR_HORIZONTAL ? Ppu::NMT_HORIZONTAL :
					info.setup.mirroring == Api::Cartridge::MIRROR_VERTICAL   ? Ppu::NMT_VERTICAL :
					info.setup.mirroring == Api::Cartridge::MIRROR_FOURSCREEN ? Ppu::NMT_FOURSCREEN :
					info.setup.mirroring == Api::Cartridge::MIRROR_ZERO       ? Ppu::NMT_ZERO :
					info.setup.mirroring == Api::Cartridge::MIRROR_ONE        ? Ppu::NMT_ONE :
																				Ppu::NMT_CONTROLLED,
					info.setup.wrkRamBacked,
					context.database && databaseHandle && info.setup.mapper == context.database->Mapper(databaseHandle) ? context.database->Attribute(databaseHandle) : 0
				);

				mapper = Mapper::Create( settings );

				info.setup.prgRom = prg.Size();
				info.setup.chrRom = chr.Size();
				info.setup.chrRam = settings.chrRam;
				info.setup.wrkRamAuto = settings.wrkAuto;

				if (info.setup.wrkRamBacked+info.setup.wrkRam != wrk.Size())
				{
					if (info.setup.wrkRamBacked > wrk.Size())
						info.setup.wrkRamBacked = wrk.Size();

					info.setup.wrkRam = wrk.Size() - info.setup.wrkRamBacked;
				}

				wrk.Reset( info, false );
				wrk.Load( info );

				info.prgCrc = Crc32::Compute( prg.Mem(), prg.Size() );
				info.crc = info.prgCrc;

				if (chr.Size())
				{
					info.chrCrc = Crc32::Compute( chr.Mem(), chr.Size() );
					info.crc = Crc32::Compute( chr.Mem(), chr.Size(), info.crc );
				}

				if (info.setup.system == Api::Cartridge::SYSTEM_VS)
				{
					vs = VsSystem::Create
					(
						context.cpu,
						context.ppu,
						static_cast<Revision::Ppu>(info.setup.ppu),
						info.setup.security == 1 ? VsSystem::MODE_RBI :
						info.setup.security == 2 ? VsSystem::MODE_TKO :
						info.setup.security == 3 ? VsSystem::MODE_XEV :
                                                   VsSystem::MODE_STD,
						info.prgCrc,
						info.setup.version == 0 || (context.database && context.database->Enabled())
					);

					info.setup.ppu = static_cast<Api::Cartridge::Ppu>(vs->GetPpuRevion());
				}
				else if (info.setup.system == Api::Cartridge::SYSTEM_HOME)
				{
					dataRecorder = new Peripherals::DataRecorder(context.cpu);
				}

				if (Cartridge::QueryExternalDevice( EXT_DIP_SWITCHES ))
					Log::Flush( "Cartridge: DIP Switches present" NST_LINEBREAK );
			}
			catch (...)
			{
				Destroy();
				throw;
			}
		}

		void Cartridge::Destroy()
		{
			delete dataRecorder;
			delete turboFile;
			VsSystem::Destroy( vs );
			Mapper::Destroy( mapper );
		}

		Cartridge::~Cartridge()
		{
			Destroy();
		}

		uint Cartridge::GetDesiredController(uint port) const
		{
			NST_ASSERT( port < Api::Input::NUM_CONTROLLERS );
			return info.controllers[port];
		}

		uint Cartridge::GetDesiredAdapter() const
		{
			return info.adapter;
		}

		Cartridge::ExternalDevice Cartridge::QueryExternalDevice(ExternalDeviceType deviceType)
		{
			switch (deviceType)
			{
				case EXT_TURBO_FILE:
					return turboFile;

				case EXT_DATA_RECORDER:
					return dataRecorder;

				case EXT_DIP_SWITCHES:

					if (vs)
						return &vs->GetDipSwiches();
					else
						return mapper->QueryDevice( Mapper::DEVICE_DIP_SWITCHES );

				case EXT_BARCODE_READER:
					return mapper->QueryDevice( Mapper::DEVICE_BARCODE_READER );

				default:
					return Image::QueryExternalDevice( deviceType );
			}
		}

		void Cartridge::DetectControllers(uint inputId)
		{
			switch (inputId)
			{
				case ImageDatabase::INPUT_LIGHTGUN:

					info.controllers[1] = Api::Input::ZAPPER;
					break;

				case ImageDatabase::INPUT_LIGHTGUN_VS:

					info.controllers[0] = Api::Input::ZAPPER;
					info.controllers[1] = Api::Input::UNCONNECTED;
					break;

				case ImageDatabase::INPUT_POWERPAD:

					info.controllers[1] = Api::Input::POWERPAD;
					break;

				case ImageDatabase::INPUT_FAMILYTRAINER:

					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::FAMILYTRAINER;
					break;

				case ImageDatabase::INPUT_PADDLE_NES:

					info.controllers[1] = Api::Input::PADDLE;
					break;

				case ImageDatabase::INPUT_PADDLE_FAMICOM:

					info.controllers[4] = Api::Input::PADDLE;
					break;

				case ImageDatabase::INPUT_ADAPTER_FAMICOM:

					info.adapter = Api::Input::ADAPTER_FAMICOM;

				case ImageDatabase::INPUT_ADAPTER_NES:

					info.controllers[2] = Api::Input::PAD3;
					info.controllers[3] = Api::Input::PAD4;
					break;

				case ImageDatabase::INPUT_SUBORKEYBOARD:

					info.controllers[4] = Api::Input::SUBORKEYBOARD;
					break;

				case ImageDatabase::INPUT_FAMILYKEYBOARD:

					info.controllers[4] = Api::Input::FAMILYKEYBOARD;
					break;

				case ImageDatabase::INPUT_PARTYTAP:

					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::PARTYTAP;
					break;

				case ImageDatabase::INPUT_CRAZYCLIMBER:

					info.controllers[4] = Api::Input::CRAZYCLIMBER;
					break;

				case ImageDatabase::INPUT_EXCITINGBOXING:

					info.controllers[4] = Api::Input::EXCITINGBOXING;
					break;

				case ImageDatabase::INPUT_HYPERSHOT:

					info.controllers[0] = Api::Input::UNCONNECTED;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::HYPERSHOT;
					break;

				case ImageDatabase::INPUT_POKKUNMOGURAA:

					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::POKKUNMOGURAA;
					break;

				case ImageDatabase::INPUT_OEKAKIDS:

					info.controllers[0] = Api::Input::UNCONNECTED;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::OEKAKIDSTABLET;
					break;

				case ImageDatabase::INPUT_MAHJONG:

					info.controllers[0] = Api::Input::UNCONNECTED;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::MAHJONG;
					break;

				case ImageDatabase::INPUT_TOPRIDER:

					info.controllers[0] = Api::Input::UNCONNECTED;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::TOPRIDER;
					break;

				case ImageDatabase::INPUT_PAD_SWAP:

					info.controllers[0] = Api::Input::PAD2;
					info.controllers[1] = Api::Input::PAD1;
					break;

				case ImageDatabase::INPUT_ROB:

					info.controllers[1] = Api::Input::ROB;
					break;

				case ImageDatabase::INPUT_POWERGLOVE:

					info.controllers[0] = Api::Input::POWERGLOVE;
					break;
			}
		}

		void Cartridge::Reset(const bool hard)
		{
			if (hard)
				wrk.Reset( info, true );

			mapper->Reset( hard );

			if (vs)
				vs->Reset( hard );

			if (turboFile)
				turboFile->Reset();

			if (dataRecorder)
				dataRecorder->Reset();
		}

		bool Cartridge::PowerOff()
		{
			try
			{
				if (mapper)
					mapper->Sync( Mapper::EVENT_POWER_OFF, NULL );

				wrk.Save( info );

				if (turboFile)
					turboFile->PowerOff();

				if (dataRecorder)
					dataRecorder->PowerOff();

				return true;
			}
			catch (...)
			{
				return false;
			}
		}

		void Cartridge::SaveState(State::Saver& state,const dword baseChunk) const
		{
			state.Begin( baseChunk );

			mapper->SaveState( state, AsciiId<'M','P','R'>::V );

			if (vs)
				vs->SaveState( state, AsciiId<'V','S','S'>::V );

			if (turboFile)
				turboFile->SaveState( state, AsciiId<'T','B','F'>::V );

			if (dataRecorder)
				dataRecorder->SaveState( state, AsciiId<'D','R','C'>::V );

			state.End();
		}

		void Cartridge::LoadState(State::Loader& state)
		{
			if (dataRecorder)
				dataRecorder->Stop();

			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'M','P','R'>::V:

						mapper->LoadState( state );
						break;

					case AsciiId<'V','S','S'>::V:

						NST_VERIFY( vs );

						if (vs)
							vs->LoadState( state );

						break;

					case AsciiId<'T','B','F'>::V:

						NST_VERIFY( turboFile );

						if (turboFile)
							turboFile->LoadState( state );

						break;

					case AsciiId<'D','R','C'>::V:

						NST_VERIFY( dataRecorder );

						if (dataRecorder)
							dataRecorder->LoadState( state );

						break;
				}

				state.End();
			}
		}

		void Cartridge::Wrk::Reset(const Info& info,const bool preserveBattery)
		{
			NST_ASSERT( info.setup.wrkRamBacked+info.setup.wrkRam == Size() );

			uint i = (preserveBattery ? info.setup.wrkRamBacked : 0);

			for (const uint n=NST_MIN(Size(),Ines::TRAINER_BEGIN), openBus=info.setup.wrkRamAuto; i < n; ++i)
				(*this)[i] = openBus ? (0x6000 + i) >> 8 : GARBAGE;

			if (i < Ines::TRAINER_END && info.setup.trainer)
				i = Ines::TRAINER_END;

			for (const uint n=NST_MIN(Size(),0x2000), openBus=info.setup.wrkRamAuto; i < n; ++i)
				(*this)[i] = openBus ? (0x6000 + i) >> 8 : GARBAGE;

			if (i < Size())
				std::memset( Mem() + i, GARBAGE, Size() - i );
		}

		void Cartridge::Wrk::Load(const Info& info)
		{
			NST_ASSERT( info.setup.wrkRamBacked+info.setup.wrkRam == Size() );

			if (info.setup.wrkRamBacked >= MIN_BATTERY_SIZE)
			{
				if (!file.Load( File::LOAD_BATTERY, Mem(), info.setup.wrkRamBacked ))
					Log::Flush( "Cartridge: warning, save file and W-RAM size mismatch!" NST_LINEBREAK );
			}
		}

		void Cartridge::Wrk::Save(const Info& info) const
		{
			NST_ASSERT( info.setup.wrkRamBacked+info.setup.wrkRam == Size() );

			if (info.setup.wrkRamBacked >= MIN_BATTERY_SIZE)
				file.Save( File::SAVE_BATTERY, Mem(), info.setup.wrkRamBacked );
		}

		Revision::Ppu Cartridge::QueryPpu(bool yuvConversion)
		{
			if (vs)
				vs->EnableYuvConversion( yuvConversion );

			return static_cast<Revision::Ppu>(info.setup.ppu);
		}

		Region::Type Cartridge::GetRegion() const
		{
			return info.setup.region == Api::Cartridge::REGION_PAL ? Region::PAL : Region::NTSC;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Cartridge::BeginFrame(const Api::Input& input,Input::Controllers* controllers)
		{
			mapper->Sync( Mapper::EVENT_BEGIN_FRAME, controllers );

			if (vs)
				vs->BeginFrame( input, controllers );
		}

		void Cartridge::VSync()
		{
			mapper->Sync( Mapper::EVENT_END_FRAME, NULL );

			if (vs)
				vs->VSync();

			if (dataRecorder)
				dataRecorder->VSync();
		}
	}
}
