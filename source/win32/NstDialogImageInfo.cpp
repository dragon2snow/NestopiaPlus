////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#include "NstWindowParam.hpp"
#include "NstDialogImageInfo.hpp"
#include "NstManagerEmulator.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include "../core/api/NstApiFds.hpp"
#include "../core/api/NstApiNsf.hpp"

namespace Nestopia
{
	using namespace Window;

	struct ImageInfo::Handlers
	{
		static const MsgHandler::Entry<ImageInfo> messages[];
		static const MsgHandler::Entry<ImageInfo> commands[];
	};

	const MsgHandler::Entry<ImageInfo> ImageInfo::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &ImageInfo::OnInitDialog }
	};

	const MsgHandler::Entry<ImageInfo> ImageInfo::Handlers::commands[] =
	{
		{ IDC_ROM_INFO_OK, &ImageInfo::OnCmdOk }
	};

	ImageInfo::ImageInfo(Managers::Emulator& e)
	: dialog(IDD_IMAGEINFO,this,Handlers::messages,Handlers::commands), emulator(e)
	{
		dialog.Open();
	}

	ibool ImageInfo::OnInitDialog(Param&)
	{
		struct State
		{
			static cstring Get(const ibool state)
			{
				return (state ? "yes" : "no");
			}

			static cstring Get(const Nes::Cartridge::State state)
			{
				return 
				(
					state == Nes::Cartridge::YES ? "yes" : 
			     	state == Nes::Cartridge::NO  ? "no" : 
		                                   	       "unknown"
				);
			}
		};

		String::Smart<512> text;

		switch (emulator.Is(Nes::Machine::CARTRIDGE|Nes::Machine::DISK|Nes::Machine::SOUND))
		{
	     	case Nes::Machine::CARTRIDGE:
			{
				const Nes::Cartridge::Info& info = *Nes::Cartridge(emulator).GetInfo();

				cstring const mode =
				(
					info.system == Nes::Cartridge::SYSTEM_NTSC_PAL ? "NTSC/PAL" :
					info.system == Nes::Cartridge::SYSTEM_PAL      ? "PAL" :
					info.system == Nes::Cartridge::SYSTEM_VS       ? "VS" :
					info.system == Nes::Cartridge::SYSTEM_PC10     ? "PC10" :
		                                                         	 "NTSC"
				);

				text << "File:       "	   << emulator.GetImagePath()
					 << "\r\ncrc:        " << String::Hex( (u32) info.crc );

				if (info.name.size())
					 text << "\r\nName:       " << info.name.c_str();

				if (info.maker.size())
					text << "\r\nMaker:      " << info.maker.c_str();

			    text << "\r\nSystem:     " << mode
					 << "\r\nBoard:      " << info.board.c_str()
					 << "\r\nMapper:     " << info.mapper
					 << "\r\nPRG-ROM:    " << (uint) (info.pRom / 1024U) << "k ";

				if (info.pRom)
					text << "crc: " << String::Hex( (u32) info.pRomCrc );

				text << "\r\nCHR-ROM:    " << (uint) (info.cRom / 1024U) << "k ";

				if (info.cRom)
					text << "crc: " << String::Hex( (u32) info.cRomCrc );

				cstring const mirroring =
				(
					info.mirroring == Nes::Cartridge::MIRROR_HORIZONTAL ? "horizontal" :
					info.mirroring == Nes::Cartridge::MIRROR_VERTICAL   ? "vertical" :
					info.mirroring == Nes::Cartridge::MIRROR_FOURSCREEN ? "four-screen" :
					info.mirroring == Nes::Cartridge::MIRROR_ZERO       ? "$2000" :
					info.mirroring == Nes::Cartridge::MIRROR_ONE        ? "$2400" :
					info.mirroring == Nes::Cartridge::MIRROR_CONTROLLED ? "mapper controlled" :
					                                                      "unknown"
				);

				cstring const condition =
				(
					info.condition == Nes::Cartridge::YES ? "good" : 
		     		info.condition == Nes::Cartridge::NO  ? "bad" : 
		                                               		"unknown"
				);

				text << "\r\nMirroring:  " << mirroring
					 << "\r\nBattery:    " << State::Get( info.battery );

				if (info.battery)
					text << "\r\nFile:       " << emulator.GetSavePath();

				text << "\r\nTrainer:    " << State::Get( info.trained )
					 << "\r\nCondition:  " << condition;

				break;
			}

	    	case Nes::Machine::DISK:
			{
				const Nes::Fds fds(emulator);

				text << "File:        "	    << emulator.GetImagePath()
					 << "\r\nDisks:       " << fds.GetNumDisks()
					 << "\r\nFile Header: " << State::Get(fds.HasHeader());

				break;
			}

	     	case Nes::Machine::SOUND:
			{
				const Nes::Nsf nsf(emulator);

				cstring const mode =
				(
					nsf.GetMode() == Nes::Nsf::TUNE_MODE_NTSC ? "NTSC" :
			       	nsf.GetMode() == Nes::Nsf::TUNE_MODE_PAL  ? "PAL" :
			                                                	"NTSC/PAL"
				);

				text << "File:           "	   << emulator.GetImagePath();

				if (*nsf.GetName())
					text << "\r\nName:           " << nsf.GetName();

				if (*nsf.GetArtist())
					text << "\r\nArtist:         " << nsf.GetArtist();

				if (*nsf.GetMaker())
					text << "\r\nMaker:          " << nsf.GetMaker();

				text << "\r\nSystem:         " << mode
					 << "\r\nSongs:          " << nsf.GetNumSongs()
					 << "\r\nStarting Song   " << (nsf.GetStartingSong() + 1)
					 << "\r\nExtra Chips:   ";

				if (const uint chips = nsf.GetChips())
				{
					if ( chips & Nes::Nsf::CHIP_MMC5  ) text << " MMC5";
					if ( chips & Nes::Nsf::CHIP_VRC6  ) text << " VRC6";
					if ( chips & Nes::Nsf::CHIP_VRC7  ) text << " VRC7";
					if ( chips & Nes::Nsf::CHIP_N106  ) text << " N106";
					if ( chips & Nes::Nsf::CHIP_FME07 ) text << " FME-07";
					if ( chips & Nes::Nsf::CHIP_FDS   )	text << " FDS";
				}
				else
				{
					text << " no";
				}

				text << "\r\nBank Switching: " << State::Get( nsf.UsesBankSwitching() )
					 << "\r\nLoad Address:   " << String::Hex( (u16) nsf.GetLoadAddress() )
					 << "\r\nInit Address:   " << String::Hex( (u16) nsf.GetInitAddress() )
					 << "\r\nPlay Address:   " << String::Hex( (u16) nsf.GetPlayAddress() );

				break;
			}
		}

		if (text.Size())
			dialog.Edit(IDC_ROM_INFO_EDIT) << text;

		return TRUE;
	}

	ibool ImageInfo::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}
}
