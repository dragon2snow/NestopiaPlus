////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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
#include <new>
#include "NstLog.hpp"
#include "NstCrc32.hpp"
#include "NstState.hpp"
#include "NstImageDatabase.hpp"
#include "NstCartridge.hpp"
#include "NstInes.hpp"
#include "NstUnif.hpp"
#include "NstMapper.hpp"
#include "vssystem/NstVsSystem.hpp"
#include "NstTurboFile.hpp"
#include "NstDataRecorder.hpp"
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
		dataRecorder (NULL),
		wRamAuto     (false)
		{
			try
			{
				result = RESULT_OK;

				switch (Stream::In(context.stream).Peek32())
				{
					case 0x1A53454EUL: { Ines ines( context.stream, pRom, cRom, wRam, info, context.database, result ); break; }
					case 0x46494E55UL: { Unif unif( context.stream, pRom, cRom, wRam, info, context.database, result ); break; }
					default: throw RESULT_ERR_INVALID_FILE;
				}
		
				if (info.system != Api::Cartridge::SYSTEM_VS)
					DetectVS();
			
				if (DetectEncryption())
					result = RESULT_WARN_ENCRYPTED_ROM;
			
				DetectControllers();
				DetectTurboFile( context );
			
				if (vs == NULL)
					dataRecorder = new DataRecorder(context.cpu);
			
				if (!InitInfo( context.database ))
				{
					if (result == RESULT_OK)
						result = RESULT_WARN_BAD_DUMP;
				}
			
				ResetWRam();
			
				if (info.battery)
					LoadBattery();
			
				if (info.system == Api::Cartridge::SYSTEM_VS)
					vs = VsSystem::Create( context.cpu, context.ppu, info.pRomCrc );
			
				pRom.Arrange( SIZE_16K );
				cRom.Arrange( SIZE_8K );
			
				Mapper::Context settings
				(
					info.mapper,
					context.cpu,
					context.ppu,
					pRom,
					cRom,
					wRam,
					info.mirroring == Api::Cartridge::MIRROR_HORIZONTAL ? Ppu::NMT_HORIZONTAL :
       				info.mirroring == Api::Cartridge::MIRROR_VERTICAL   ? Ppu::NMT_VERTICAL :
     				info.mirroring == Api::Cartridge::MIRROR_FOURSCREEN ? Ppu::NMT_FOURSCREEN :
      				info.mirroring == Api::Cartridge::MIRROR_ZERO       ? Ppu::NMT_ZERO :
       				info.mirroring == Api::Cartridge::MIRROR_ONE        ? Ppu::NMT_ONE : 
				                                                          Ppu::NMT_CONTROLLED,
					info.battery,
					info.pRomCrc
	     	 	);
			
				mapper = Mapper::Create( settings );
			
				if (wRam.Size())
				{
					wRamAuto = settings.wRamAuto;
			
					if (info.battery)
						batteryCheckSum = Md5::Compute( wRam.Mem(), wRam.Size() );
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
			VsSystem::Destroy( vs );
			Mapper::Destroy( mapper );
		}

		Cartridge::~Cartridge()
		{
			Destroy();
		}
	
		bool Cartridge::InitInfo(const ImageDatabase* const database)
		{
			if (info.board.empty())
				info.board = Mapper::GetBoard( info.mapper );
	
			if (!info.crc || !database)
				return true;
	
			ImageDatabase::Handle handle( database->GetHandle( info.crc ) );
	
			if (!handle)
				return true;

			if (database->IsBad( handle ))
			{
				info.condition = Api::Cartridge::NO;
				return false;
			}
	
			return true;
		}
		
		uint Cartridge::GetDesiredController(uint port) const
		{
			NST_ASSERT( port < Api::Input::NUM_CONTROLLERS );
			return info.controllers[port];
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

		void Cartridge::DetectControllers()
		{
			switch (info.pRomCrc)
			{
				case 0xFBFC6A6CUL: // Adventures of Bayou Billy, The (E)
				case 0xCB275051UL: // Adventures of Bayou Billy, The (U)
				case 0xFB69C131UL: // Baby Boomer (JUE)
				case 0xF2641AD0UL: // Barker Bill's Trick Shooting (U)
				case 0x639CB8B8UL: // Chiller
				case 0xBC1DCE96UL: // -||-
				case 0xBE1BF10CUL: // -||-
				case 0x320194F8UL: // -||-
				case 0x90CA616DUL: // Duck Hunt (JUE)
				case 0x59E3343FUL: // Freedom Force (U)
				case 0x242A270CUL: // Gotcha! (U)
				case 0x7B5BD2DEUL: // Gumshoe (UE)
				case 0x255B129CUL: // Gun Sight (J)
				case 0x8963AE6EUL: // Hogan's Alley (JU)
				case 0x51D2112FUL: // Laser Invasion (U)
				case 0x0A866C94UL: // Lone Ranger, The (U)
				case 0xE4C04EEAUL: // Mad City (J)
				case 0x9EEF47AAUL: // Mechanized Attack (U)
				case 0xC2DB7551UL: // Shooting Range (U)
				case 0x0CD00488UL: // Space Shadow
				case 0x03B6596CUL: // -||-
				case 0x81069812UL: // Super Mario Bros / Duck Hunt (U)
				case 0xE8F8F7A5UL: // -||- (E)
				case 0xD4F018F5UL: // Super Mario Bros / Duck Hunt / Track Meet
				case 0x163E86C0UL: // To The Earth (U)
				case 0x389960DBUL: // Wild Gunman (JUE)
				case 0x0D3CF705UL: // -||- (J)
				case 0x4A60A644UL: // -||-
				case 0x1388AEB9UL: // Operation Wolf (U)
				case 0x42d893E4UL: // -||- (J)
				case 0x54C34223UL: // -||- (E)
				case 0xACD34C1DUL: // -||-
				case 0x694041D7UL: // -||-
			
					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::ZAPPER;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::UNCONNECTED;
					break;
			
				case 0xED588F00UL: // VS Duck Hunt
				case 0x17AE56BEUL: // VS Freedom Force
				case 0xFF5135A3UL: // VS Hogan's Alley
			
					info.controllers[0] = Api::Input::ZAPPER;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::UNCONNECTED;
					break;
			
				case 0x35893B67UL: // Arkanoid (J)
				case 0x9D0B83E0UL: // -||-
				case 0x95DBB274UL: // -||-
				case 0xAE56518EUL: // -||-
				case 0xAF4A2A1DUL: // -||-
				case 0x6267FBD1UL: // Arkanoid 2 (J)
				case 0x9C6868A8UL: // -||-
				case 0x8FBF8C2CUL: // -||-
				case 0xD04D6C50UL: // -||-
			
					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::PAD2;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::PADDLE;
					break;
			
				case 0xB8BB48D3UL: // Arkanoid (U)
			
					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::PADDLE;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::UNCONNECTED;
					break;
			
				case 0xBC5F6C94UL: // Athletic World (U)
				case 0xD836A90BUL: // Dance Aerobics (U)
				case 0xFD37CA4CUL: // Short Order - Eggsplode (U)
				case 0x96C4CE38UL: // -||-
				case 0x29292EE4UL: // -||-
				case 0x987DCDA3UL: // Street Cop (U)
				case 0xDD978A90UL: // Stadium Events (U)
				case 0xD06CEB9AUL: // Stadium Events (E)
				case 0x2D76A271UL: // World Track Meet (U)
				case 0xF210E68FUL: // Super Team Games (U)

					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::POWERPAD;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::UNCONNECTED;
					break;

				case 0xF8DA2506UL: // Aerobics Studio (J)
				case 0x8C8FA83BUL: // Athletic World (J)
				case 0xCA26A0F1UL: // Dai Undoukai (J)
				case 0xEE11FE78UL: // Fuuun! Takeshi Shiro 2 (J)
				case 0x28068B8CUL: // Fuuun! Takeshi Jou 2 (J) bad
				case 0x7E704A14UL: // Jogging Race (J)
				case 0x10BB8F9AUL: // Manhattan Police (J)
				case 0xAD3DF455UL: // Meiro Dai Sakusen (J)
				case 0x2330A5D3UL: // Rairai Kyonshiizu (J)
				case 0xB8C54977UL: // -||-
				case 0x8A5B72C0UL: // Running Stadium (J)
				case 0x59794F2DUL: // Totsugeki Fuuun Takeshi Jou (J)

					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::FAMILYTRAINER;
					break;

				case 0x868FCD89UL: // Family Basic (Ver 1.0)
				case 0xF9DEF527UL: // Family Basic (Ver 2.0)
				case 0xDE34526EUL: // Family Basic (Ver 2.1a)
				case 0xF050B611UL: // Family Basic (Ver 3)
				case 0x3AAEED3FUL: // Family Basic (Ver 3) (Alt)
				case 0xDA03D908UL: // Playbox BASIC (Ver 1.0)
				case 0x2D6B7E5AUL: // Playbox BASIC

					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::PAD2;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::FAMILYKEYBOARD;
					break;

				case 0x903A95EBUL: // Magistr v1.0
				case 0x7BDD12F3UL: // Simbas v1.0
				case 0x1460EC7BUL: // Subor v1.0
				case 0x82F1FB96UL: // Subor v1.0 (Russian)
				case 0x589B6B0DUL: // Supor v2.0
				case 0x41401C6DUL: // Supor v4.0
				case 0x41EF9AC4UL: // -||-
				case 0x5E073A1BUL: // Supor English (Chinese)
				case 0x8b265862UL: // -||-
				case 0xABB2F974UL: // Study and Game 32-in-1
				case 0xD5D6EAC4UL: // Edu (Asia)
				case 0x368C19A8UL: // LIKO Study Cartridge
				case 0x543AB532UL: // LIKO Color Lines
				case 0xB066111AUL: // Text Editor (Russian)

					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::PAD2;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::SUBORKEYBOARD;
					break;

				case 0x3B997543UL: // Gauntlet 2 (E)
				case 0x2C609B52UL: // Gauntlet 2 (U)
				case 0x1352F1B9UL: // Greg Norman's Golf Power (U) 
				case 0xAB8371F6UL: // Kings of the Beach (U)
				case 0x0939852FUL: // M.U.L.E. (U)
				case 0xDA2CB59AUL: // Nightmare on Elm Street, A (U)
				case 0x9EDD2159UL: // R.C. Pro-Am 2 (U)
				case 0xD3428E2EUL: // Super Jeopardy! (U)
				case 0x15E98A14UL: // Super Spike V'Ball (U)
				case 0xEBB9DF3DUL: // Super Spike V'Ball (E)
				case 0x35190A3FUL: // Super Spike V'Ball - World Cup (U)
				case 0x3417EC46UL: // Swords and Serpents (U)
				case 0xD153CAF6UL: // Swords and Serpents (E)
				case 0x85C5B6B4UL: // Nekketsu Kakutou Densetsu
				case 0x3B7B3BE1UL: // -||- (T)
				case 0x9771A019UL: // -||- (T)
				case 0xAAFED9B4UL: // -||-
				case 0x7BD7B849UL: // Nekketsu Koukou - Dodgeball Bu
				case 0xDEDDD5E5UL: // Kunio Kun no Nekketsu Soccer League
				case 0x6EF0C08EUL: // -||- (T)
				case 0x6375C11EUL: // -||- (T)
				case 0xF985AC97UL: // -||- (T)
				case 0xBA11692DUL: // -||- (T)
				case 0x4FB460CDUL: // Nekketsu! Street Basket - Ganbare Dunk Heroes
				case 0x457BC688UL: // -||- (T)
				case 0x5D4A01A9UL: // -||- (T)
			
					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::PAD2;
					info.controllers[2] = Api::Input::PAD3;
					info.controllers[3] = Api::Input::PAD4;
					info.controllers[4] = Api::Input::UNCONNECTED;
					break;
			
				case 0xC3C0811DUL: // Oeka Kids - Anpanman no Hiragana Daisuki (J)
				case 0x9D048EA4UL: // Oeka Kids - Anpanman to Oekaki Shiyou (J)
			
					info.controllers[0] = Api::Input::UNCONNECTED;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::OEKAKIDSTABLET;
					break;
			
				case 0xFF6621CEUL: // Hyper Olympic
				case 0x3FC5293EUL: // -||-
				case 0x39AB6510UL: // -||-
				case 0xDB9418E8UL: // -||-
				case 0xAC98CD70UL: // Hyper Sports
				case 0x435F621EUL: // -||-
			
					info.controllers[0] = Api::Input::UNCONNECTED;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::HYPERSHOT;
					break;
			
				case 0xC68363F6UL: // Crazy Climber
				case 0x814188EEUL: // -||-
				case 0xD9934AEFUL: // -||-
				case 0xC1DC5B12UL: // -||-
			
					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::PAD2;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::CRAZYCLIMBER;
					break;
			
				case 0x9FAE4D46UL: // Ide Yousuke Meijin no Jissen Mahjong
				case 0x3BCEBB61UL: // -||-
				case 0x7B44FB2AUL: // Ide Yousuke Meijin no Jissen Mahjong 2
			
					info.controllers[0] = Api::Input::UNCONNECTED;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::MAHJONG;
					break;
			
				case 0x786148B6UL: // Exciting Boxing
			
					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::PAD2;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::EXCITINGBOXING;
					break;
			
				case 0x20D22251UL: // Top Rider
				case 0x26171D7DUL: // -||-
			
					info.controllers[0] = Api::Input::UNCONNECTED;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::TOPRIDER;
					break;
			
				case 0x3993B4EBUL: // Super Mogura Tataki!! - Pokkun Moguraa
			
					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::POKKUNMOGURAA;
					break;

				case 0xE44001D8UL: // Casino Derby
				case 0xD9F45BE9UL: // Gimmi a Break - Shijou Saikyou no Quiz Ou Ketteisen
				case 0x1545BD13UL: // Gimmi a Break - Shijou Saikyou no Quiz Ou Ketteisen 2
				case 0xC1BA8BB9UL: // Project Q

					info.controllers[0] = Api::Input::PAD1;
					info.controllers[1] = Api::Input::UNCONNECTED;
					info.controllers[2] = Api::Input::UNCONNECTED;
					info.controllers[3] = Api::Input::UNCONNECTED;
					info.controllers[4] = Api::Input::PARTYTAP;
					break;
			}	
		}
	
		void Cartridge::DetectVS()
		{
			switch (info.pRomCrc)
			{
				// UniSystem
				case 0xEB2DBA63UL: // TKO Boxing
				case 0x9818F656UL: // -||-
				case 0x135ADF7CUL: // RBI Baseball
				case 0xED588F00UL: // Duck Hunt
				case 0x16D3F469UL: // Ninja Jajamaru Kun (J)
				case 0x8850924BUL: // Tetris
				case 0x8C0C2DF5UL: // Top Gun
				case 0x70901B25UL: // Slalom
				case 0xCF36261EUL: // Sky Kid
				case 0xE1AA8214UL: // Star Luster
				case 0xD5D7EAC4UL: // Dr. Mario
				case 0xFFBEF374UL: // Castlevania
				case 0xE2C0A2BEUL: // Platoon
				case 0xCBE85490UL: // Excitebike
				case 0x29155E0CUL: // Excitebike (alt)
				case 0x07138C06UL: // Clu Clu Land
				case 0x43A357EFUL: // Ice Climber
				case 0xD4EB5923UL: // -||-
				case 0x737DD1BFUL: // Super Mario Bros	
				case 0x4BF3972DUL: // -||-
				case 0x8B60CC58UL: // -||-
				case 0x8192C804UL: // -||-
				case 0xEC461DB9UL: // Pinball
				case 0xE528F651UL: // Pinball (alt)
				case 0x0B65A917UL: // Mach Rider
				case 0x8A6A9848UL: // -||-
				case 0xAE8063EFUL: // -||- (Japan, Fighting Course)
				case 0x46914E3EUL: // Soccer
				case 0x70433F2CUL: // Battle City
				case 0x8D15A6E6UL: // -||-
				case 0xD99A2087UL: // Gradius
				case 0x1E438D52UL: // Goonies
				case 0xFF5135A3UL: // Hogan's Alley
				case 0x17AE56BEUL: // Freedom Force
				case 0xF9D3B0A3UL: // Super Xevious
				case 0x66BB838FUL: // -||-
				case 0x9924980AUL: // -||-
				case 0xCC2C4B5DUL: // Golf
				case 0x86167220UL: // Lady Golf
				case 0xA93A5AEEUL: // Stroke and Match Golf
				case 0xC99EC059UL: // Raid on Bungeling Bay
				case 0xCA85E56DUL: // Mighty Bomb Jack
			
				// DualSystem
				case 0xB90497AAUL: // Tennis
				case 0x008A9C16UL: // Wrecking Crew 
				case 0xAD407F52UL: // Balloon Fight
				case 0x18A93B7BUL: // Mahjong (J)
				case 0x13A91937UL: // Baseball 
				case 0xF5DEBF88UL: // -||-
				case 0xF64D7252UL: // -||-
				case 0x968A6E9DUL: // -||-
				case 0xF42DAB14UL: // Ice Climber
				case 0x7D6B764FUL: // -||-
			
					info.system = Api::Cartridge::SYSTEM_VS;
					break;
			}
		}

		void Cartridge::DetectTurboFile(Context& context)
		{
			NST_ASSERT( turboFile == NULL );

			switch (info.pRomCrc)
			{
				case 0xE792DE94UL: // Best Play - Pro Yakyuu (J)
				case 0x042F17C4UL: // -||-
				case 0xF79D684AUL: // -||-
				case 0xC2EF3422UL: // Best Play - Pro Yakyuu 2 (J)
				case 0xCB810E8FUL: // -||-
				case 0x236CBA4DUL: // -||-
				case 0x974E8840UL: // Best Play - Pro Yakyuu '90 (J)
				case 0xB8747ABFUL: // Best Play - Pro Yakyuu Special (J)
				case 0xF7D51D87UL: // -||-
				case 0xE3C2C174UL: // -||-
				case 0x9FA1C11FUL: // Castle Excellent (J)
				case 0x0B0D4D1BUL: // Derby Stallion - Zenkoku Ban (J)
				case 0x728C3D98UL: // Downtown - Nekketsu Monogatari (J)
				case 0xD68A6F33UL: // Dungeon Kid (J)
				case 0x3A51EB04UL: // Fleet Commander (J)
				case 0x7C46998BUL: // Haja no Fuuin (J)
				case 0x7E5D2F1AUL: // Itadaki Street - Watashi no Mise ni Yottette (J)
				case 0xCEE5857BUL: // Ninjara Hoi! (J)
				case 0xCB8F9273UL: // -||-
				case 0x50EC5E8BUL: // Wizardry - Legacy of Llylgamyn (J)
				case 0x343E9146UL: // Wizardry - Proving Grounds of the Mad Overlord (J)
				case 0x33D07E45UL: // Wizardry - The Knight of Diamonds (J)

					turboFile = new TurboFile( context.cpu );
					break;
			}
		}
	
		bool Cartridge::DetectEncryption()
		{
			switch (info.pRomCrc)
			{
				case 0xA80A2185UL: // Pikachu Y2K
				case 0x0A4CF093UL: // Shui Hu Zhuan
				case 0x7F25140EUL: // San Shi Liu Ji
				case 0xAA34D5D5UL: // Duo Bao Xiao Ying Hao - Guang Ming yu An Hei Chuan Shuo
				case 0xDF6773D0UL: // Yu Zhou Zhan Jiang - Space General
				case 0x82A879B7UL: // Yong Zhe Dou E Long - Dragon Quest 5
				case 0x260D32E0UL: // Yong Zhe Dou E Long - Dragon Quest 7
				case 0x5E09D30FUL: // Yin He Shi Dai
				case 0xB68266C9UL: // Dragon Quest
				case 0x03D6B055UL: // Dong Fang de Chuan Shuo - The Hyrule Fantasy (Zelda)
				case 0x746E2815UL: // San Guo Zhi
			
					Log::Flush( "Cartridge: game is encrypted!" NST_LINEBREAK );
					return true;
			}
	
			return false;
		}
	
		void Cartridge::ResetWRam()
		{
			NST_ASSERT( (wRam.Size() % SIZE_8K) == 0 );

			if (wRam.Size())
			{
				const uint mask = wRamAuto ? 0xFF : 0x00;

				for (uint i=0x6000U; i < 0x7000U; ++i)
					wRam[i-0x6000U] = (i >> 8) & mask;

				for (uint i=(info.trained ? 0x7200U : 0x7000U); i < 0x8000U; ++i)
					wRam[i-0x6000U] = (i >> 8) & mask;

				std::memset( wRam.Mem(0x2000U), 0x00, wRam.Size() - 0x2000U );
			}
		}

		void Cartridge::Reset(const bool hard)
		{
			if (hard && !info.battery)
				ResetWRam();

			mapper->Reset( hard );
	
			if (vs)
				vs->Reset( hard );

			if (turboFile)
				turboFile->Reset();

			if (dataRecorder)
				dataRecorder->Reset();
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
			NST_ASSERT( info.battery && batteryCheckSum.IsNull() );
	
			std::vector<u8> data;
			Api::User::fileIoCallback( Api::User::FILE_LOAD_BATTERY, data );
	
			if (ulong size = data.size())
			{
				ulong pad = 0;
	
				if (size > SIZE_8K * 0xFFUL)
				{
					size = SIZE_8K * 0xFFUL;
					Log::Flush( "Cartridge: warning, save data size is too big! Only the first 2040k will be used!" NST_LINEBREAK );
				}
				else
				{
					pad = size % SIZE_8K;
	
					if (pad)
						Log::Flush( "Cartridge: warning, save data size is not evenly divisible by 8k!" NST_LINEBREAK );
				}
	
				if (wRam.Size() < size)
				{
					// Current WRAM size is too small. Resize and do zero-padding.
					wRam.Set( size + (pad ? SIZE_8K - pad : 0) );
					std::memset( wRam.Mem(size), 0x00, wRam.Size() - size );
				}
	
				std::memcpy( wRam.Mem(), &data.front(), size );
			}
		}
	
		Result Cartridge::SaveBattery() const
		{
			if (info.battery && wRam.Size())
			{
				const Md5::Key key( Md5::Compute( wRam.Mem(), wRam.Size() ) );

				if (batteryCheckSum != key)
				{
					try
					{
						std::vector<u8> vector( wRam.Mem(), wRam.Mem( wRam.Size() ) );	
						batteryCheckSum = key;
						Api::User::fileIoCallback( Api::User::FILE_SAVE_BATTERY, vector );
					}
					catch (...)
					{
						return RESULT_WARN_BATTERY_NOT_SAVED;
					}
				}
			}
	
			return RESULT_OK;
		}
	
		const void* Cartridge::SearchDatabase(const ImageDatabase& database,const void* data,ulong fullsize,const ulong compsize)
		{
			const void* entry = NULL;

			if (data && fullsize > 16)
			{
				fullsize -= 16;
				data = static_cast<const void*>(static_cast<const u8*>(data) + 16);
				entry = database.GetHandle( Crc32::Compute( data, fullsize ) );

				if (entry == NULL && compsize && compsize < fullsize)
					entry = database.GetHandle( Crc32::Compute( data, compsize ) );
			}

			return entry;
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		void Cartridge::BeginFrame(const Api::Input& input,Input::Controllers* controllers)
		{
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
	
		Mode Cartridge::GetMode() const
		{
			return info.system == Api::Cartridge::SYSTEM_PAL ? MODE_PAL : MODE_NTSC;
		}
	}
}
