////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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
#include "NstChecksumCrc32.hpp"
#include "NstState.hpp"
#include "NstImageDatabase.hpp"
#include "NstCartridge.hpp"
#include "NstCartridgeInes.hpp"
#include "NstCartridgeUnif.hpp"
#include "NstMapper.hpp"
#include "mapper/NstMapper188.hpp"
#include "vssystem/NstVsSystem.hpp"
#include "NstPrpTurboFile.hpp"
#include "NstPrpDataRecorder.hpp"
#include "api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Cartridge::Cartridge(Context& context,Result& result)
		:
		Image        (CARTRIDGE),
		mapper       (NULL),
		vs           (NULL),
		turboFile    (NULL),
		dataRecorder (NULL)
		{
			try
			{
				ImageDatabase::Handle databaseHandle = NULL;

				switch (Stream::In(context.stream).Peek32())
				{
					case 0x1A53454EUL: result = Ines( context.stream, prg, chr, wrk, info, context.database, databaseHandle ).GetResult(); break;
					case 0x46494E55UL: result = Unif( context.stream, prg, chr, wrk, info, context.database, databaseHandle ).GetResult(); break;
					default: throw RESULT_ERR_INVALID_FILE;
				}

				if (context.database && databaseHandle)
				{
					info.condition = context.database->Condition(databaseHandle);

					if (info.condition == Api::Cartridge::DUMP_BAD)
						result = RESULT_WARN_BAD_DUMP;

					if (context.database->Encrypted(databaseHandle))
					{
						result = RESULT_WARN_ENCRYPTED_ROM;
						Log::Flush( "Cartridge: file is encrypted!" NST_LINEBREAK );
					}

					if (const uint id = context.database->Input(databaseHandle))
						DetectControllers( id );

					if (context.database->InputEx(databaseHandle) == ImageDatabase::INPUT_EX_TURBOFILE)
					{
						Log::Flush( "Cartridge: Turbo File storage device present" NST_LINEBREAK );
						turboFile = new Peripherals::TurboFile( context.cpu );
					}
				}

				if (info.board.empty())
					info.board = Mapper::GetBoard( info.setup.mapper );

				Mapper::Context settings
				(
					info.setup.mapper,
					context.cpu,
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

				if (wrk.Size())
				{
					ResetWrkRam( 0 );
					LoadBattery();
				}

				info.crc = info.prgCrc = Checksum::Crc32::Compute( prg.Mem(), prg.Size() );

				if (chr.Size())
				{
					info.chrCrc = Checksum::Crc32::Compute( chr.Mem(), chr.Size() );
					info.crc = Checksum::Crc32::Compute( chr.Mem(), chr.Size(), info.crc );
				}

				if (info.setup.system == SYSTEM_VS)
				{
					vs = VsSystem::Create
					(
						context.cpu,
						context.ppu,
						info.setup.ppu,
						info.setup.security == 1 ? VsSystem::MODE_RBI :
						info.setup.security == 2 ? VsSystem::MODE_TKO :
						info.setup.security == 3 ? VsSystem::MODE_XEV :
                                                   VsSystem::MODE_STD,
						info.prgCrc,
						info.setup.version == 0 || (context.database && context.database->Enabled())
					);

					info.setup.ppu = vs->GetPpuType();
				}
				else if (info.setup.system != SYSTEM_PC10)
				{
					dataRecorder = new Peripherals::DataRecorder(context.cpu);
				}
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
			delete vs;
			delete mapper;
		}

		Cartridge::~Cartridge()
		{
			Destroy();
		}

		Result Cartridge::Flush(bool power,bool movie) const
		{
			return movie ? RESULT_OK : SaveBattery( power );
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

		Cartridge::ExternalDevice Cartridge::QueryExternalDevice(ExternalDeviceType type)
		{
			switch (type)
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
					return Image::QueryExternalDevice( type );
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
			}
		}

		void Cartridge::Reset(const bool hard)
		{
			if (hard && info.setup.wrkRam)
				ResetWrkRam( info.setup.wrkRamBacked );

			mapper->Reset( hard );

			if (vs)
				vs->Reset( hard );

			if (turboFile)
				turboFile->Reset();

			if (dataRecorder)
				dataRecorder->Reset();
		}

		void Cartridge::ResetWrkRam(uint i)
		{
			NST_ASSERT( info.setup.wrkRamBacked+info.setup.wrkRam == wrk.Size() && (i == 0 || i == info.setup.wrkRamBacked) );

			for (const uint n=NST_MIN(wrk.Size(),Ines::TRAINER_BEGIN), openBus=info.setup.wrkRamAuto; i < n; ++i)
				wrk[i] = openBus ? (0x6000U + i) >> 8 : Wrk::GARBAGE;

			if (i < Ines::TRAINER_END && info.setup.trainer)
				i = Ines::TRAINER_END;

			for (const uint n=NST_MIN(wrk.Size(),0x2000U), openBus=info.setup.wrkRamAuto; i < n; ++i)
				wrk[i] = openBus ? (0x6000U + i) >> 8 : Wrk::GARBAGE;

			if (i < wrk.Size())
				std::memset( wrk.Mem() + i, Wrk::GARBAGE, wrk.Size() - i );
		}

		void Cartridge::SaveState(State::Saver& state) const
		{
			mapper->SaveState( State::Saver::Subset(state,'M','P','R','\0').Ref() );

			if (vs)
				vs->SaveState( State::Saver::Subset(state,'V','S','S','\0').Ref() );

			if (turboFile)
				turboFile->SaveState( State::Saver::Subset(state,'T','B','F','\0').Ref() );

			if (dataRecorder && dataRecorder->CanSaveState())
				dataRecorder->SaveState( State::Saver::Subset(state,'D','R','C','\0').Ref() );
		}

		void Cartridge::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('M','P','R','\0'):

						mapper->LoadState( State::Loader::Subset(state).Ref() );
						break;

					case NES_STATE_CHUNK_ID('V','S','S','\0'):

						NST_VERIFY( vs );

						if (vs)
							vs->LoadState( State::Loader::Subset(state).Ref() );

						break;

					case NES_STATE_CHUNK_ID('T','B','F','\0'):

						NST_VERIFY( turboFile );

						if (turboFile)
							turboFile->LoadState( State::Loader::Subset(state).Ref() );

						break;

					case NES_STATE_CHUNK_ID('D','R','C','\0'):

						NST_VERIFY( dataRecorder );

						if (dataRecorder)
							dataRecorder->LoadState( State::Loader::Subset(state).Ref() );

						break;
				}

				state.End();
			}
		}

		void Cartridge::LoadBattery()
		{
			NST_ASSERT( info.setup.wrkRamBacked+info.setup.wrkRam == wrk.Size() && wrk.ramCheckSum.IsNull() );

			if (info.setup.wrkRamBacked >= Wrk::MIN_BATTERY_SIZE)
			{
				Api::User::FileData data;
				Api::User::fileIoCallback( Api::User::FILE_LOAD_BATTERY, data );

				if (ulong size = data.size())
				{
					if (size != info.setup.wrkRamBacked)
					{
						if (size > info.setup.wrkRamBacked)
							size = info.setup.wrkRamBacked;

						Log::Flush( "Cartridge: warning, save file and W-RAM size mismatch!" NST_LINEBREAK );
					}

					std::memcpy( wrk.Mem(), &data.front(), size );
				}

				wrk.ramCheckSum = Checksum::Md5::Compute( wrk.Mem(), info.setup.wrkRamBacked );
			}
		}

		Result Cartridge::SaveBattery(bool power) const
		{
			if (mapper)
				mapper->Flush( power );

			NST_ASSERT( info.setup.wrkRamBacked+info.setup.wrkRam == wrk.Size() );

			if (info.setup.wrkRamBacked >= Wrk::MIN_BATTERY_SIZE)
			{
				const Checksum::Md5::Key key( Checksum::Md5::Compute( wrk.Mem(), info.setup.wrkRamBacked ) );

				if (wrk.ramCheckSum != key)
				{
					try
					{
						Api::User::FileData vector( wrk.Mem(), wrk.Mem() + info.setup.wrkRamBacked );
						Api::User::fileIoCallback( Api::User::FILE_SAVE_BATTERY, vector );
						wrk.ramCheckSum = key;
					}
					catch (...)
					{
						return RESULT_WARN_BATTERY_NOT_SAVED;
					}
				}
			}

			return RESULT_OK;
		}

		const void* Cartridge::SearchDatabase(const ImageDatabase& database,const void* file,ulong length)
		{
			return Ines::SearchDatabase( database, static_cast<const u8*>(file), length );
		}

		PpuType Cartridge::QueryPpu(bool yuvConversion)
		{
			if (vs)
				vs->EnableYuvConversion( yuvConversion );

			return info.setup.ppu;
		}

		Mode Cartridge::GetMode() const
		{
			return info.setup.region == REGION_PAL ? MODE_PAL : MODE_NTSC;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Cartridge::BeginFrame(const Api::Input& input,Input::Controllers* controllers)
		{
			if (info.setup.mapper == 188)
				static_cast<Mapper188*>(mapper)->BeginFrame( controllers );

			if (vs)
				vs->BeginFrame( input, controllers );
		}

		void Cartridge::VSync()
		{
			mapper->VSync();

			if (vs)
				vs->VSync();

			if (dataRecorder)
				dataRecorder->VSync();
		}
	}
}
