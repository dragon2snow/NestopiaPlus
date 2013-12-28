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

#include "NstWindowParam.hpp"
#include "NstResourceString.hpp"
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
		{ IDC_IMAGE_INFO_OK, &ImageInfo::OnCmdOk }
	};

	ImageInfo::ImageInfo(Managers::Emulator& e)
	: dialog(IDD_IMAGE_INFO,this,Handlers::messages,Handlers::commands), emulator(e)
	{
	}

	ibool ImageInfo::OnInitDialog(Param&)
	{
		struct Table
		{
			static void Tab(HeapString* const strings,const uint count,const ibool fixed)
			{
				if (fixed)
				{
					uint maxTab = 0;

					for (uint i=0; i < count; ++i)
						maxTab = NST_MAX(maxTab,strings[i].Length());

					maxTab += 2;

					for (uint i=0; i < count; ++i)
					{
						if (uint pos = strings[i].Length())
						{
							strings[i].Resize( maxTab );
							strings[i][pos] = ':';

							while (++pos < maxTab)
								strings[i][pos] = ' ';
						}
					}
				}
				else for (uint i=0; i < count; ++i)
				{
					if (strings[i].Length())
						strings[i] << ": ";
				}
			}

			static void Output(HeapString& output,HeapString* const strings,const uint count)
			{
				for (uint i=0; i < count; ++i)
				{
					if (strings[i].Length())
					{
						output << strings[i];

						if (i < count-1)
							output << "\r\n";
					}
				}
			}
		};

		HeapString text;

		const ibool fixedFont = dialog.Edit(IDC_IMAGE_INFO_EDIT).IsFixedFont();

		switch (emulator.Is(Nes::Machine::CARTRIDGE|Nes::Machine::DISK|Nes::Machine::SOUND))
		{
			case Nes::Machine::CARTRIDGE:
			{
				const Nes::Cartridge::Info& info = *Nes::Cartridge(emulator).GetInfo();

				HeapString types[] =
				{
                                         Resource::String( IDS_TEXT_FILE        ),
                                         Resource::String( IDS_TEXT_DIRECTORY   ),
                                                       _T( "CRC"                ),
					info.name.size() ?   Resource::String( IDS_TEXT_NAME        ) : HeapString(),
					info.maker.size() ?  Resource::String( IDS_TEXT_MAKER       ) : HeapString(),
                                         Resource::String( IDS_TEXT_REGION      ),
                                         Resource::String( IDS_TEXT_CHIPSBOARDS ),
					info.mapper <= 255 ? Resource::String( IDS_TEXT_MAPPER      ) : HeapString(),
                                                       _T( "PRG-ROM"            ),
                                                       _T( "CHR-ROM"            ),
                                         Resource::String( IDS_TEXT_MIRRORING   ),
                                         Resource::String( IDS_TEXT_BATTERY     ),
					info.battery ?       Resource::String( IDS_TEXT_FILE        ) : HeapString(),
					info.battery ?       Resource::String( IDS_TEXT_DIRECTORY   ) : HeapString(),
                                         Resource::String( IDS_TEXT_TRAINER     ),
                                         Resource::String( IDS_TEXT_CONDITION   )
				};

				Table::Tab( types, NST_COUNT(types), fixedFont );

				types[0] << emulator.GetImagePath().File();
				types[1] << emulator.GetImagePath().Directory();
				types[2] << HexString( (u32) info.crc );

				if (info.name.size())
					types[3].Import( info.name.c_str() );

				if (info.maker.size())
					types[4].Import( info.maker.c_str() );

				types[5] <<
				(
					info.system == Nes::Cartridge::SYSTEM_NTSC_PAL ? "NTSC/PAL" :
					info.system == Nes::Cartridge::SYSTEM_PAL      ? "PAL"      :
					info.system == Nes::Cartridge::SYSTEM_VS       ? "VS"       :
					info.system == Nes::Cartridge::SYSTEM_PC10     ? "PC10"     :
                                                                     "NTSC"
				);

				types[6].Import( info.board.c_str() );

				if (types[7].Length())
					types[7] << info.mapper;

				types[8] << (uint) (info.pRom / 1024) << "k";

				if (info.pRom)
					types[8] << ", CRC: " << HexString( (u32) info.pRomCrc );

				types[9] << (uint) (info.cRom / 1024) << "k";

				if (info.cRom)
					types[9] << ", CRC: " << HexString( (u32) info.cRomCrc );

				types[10] <<
				(
					info.mirroring == Nes::Cartridge::MIRROR_HORIZONTAL ? Resource::String( IDS_TEXT_HORIZONTAL       ) :
					info.mirroring == Nes::Cartridge::MIRROR_VERTICAL   ? Resource::String( IDS_TEXT_VERTICAL         ) :
					info.mirroring == Nes::Cartridge::MIRROR_FOURSCREEN ? Resource::String( IDS_TEXT_FOURSCREEN       ) :
					info.mirroring == Nes::Cartridge::MIRROR_ZERO       ?               _T( "$2000"                   ) :
					info.mirroring == Nes::Cartridge::MIRROR_ONE        ?               _T( "$2400"                   ) :
					info.mirroring == Nes::Cartridge::MIRROR_CONTROLLED ? Resource::String( IDS_TEXT_MAPPERCONTROLLED ) :
                                                                          Resource::String( IDS_TEXT_UNKNOWN          )
				);

				types[11] << Resource::String
				(
					info.battery == Nes::Cartridge::YES ? IDS_TEXT_YES :
					info.battery == Nes::Cartridge::NO  ? IDS_TEXT_NO :
                                                          IDS_TEXT_UNKNOWN
				);

				if (info.battery)
				{
					types[12] << emulator.GetSavePath().File();
					types[13] << emulator.GetSavePath().Directory();
				}

				types[14] << Resource::String( info.trained ? IDS_TEXT_YES : IDS_TEXT_NO );

				types[15] << Resource::String
				(
					info.condition == Nes::Cartridge::YES ? IDS_TEXT_GOOD :
					info.condition == Nes::Cartridge::NO  ? IDS_TEXT_BAD :
															IDS_TEXT_UNKNOWN
				);

				Table::Output( text, types, NST_COUNT(types) );
				break;
			}

			case Nes::Machine::DISK:
			{
				const Nes::Fds fds(emulator);

				HeapString types[] =
				{
					Resource::String( IDS_TEXT_FILE      ),
					Resource::String( IDS_TEXT_DIRECTORY ),
					Resource::String( IDS_TEXT_DISKS     ),
					Resource::String( IDS_TEXT_HEADER    )
				};

				Table::Tab( types, NST_COUNT(types), fixedFont );

				types[0] << emulator.GetImagePath().File();
				types[1] << emulator.GetImagePath().Directory();
				types[2] << fds.GetNumDisks();
				types[3] << Resource::String( fds.HasHeader() ? IDS_TEXT_YES : IDS_TEXT_NO );

				Table::Output( text, types, NST_COUNT(types) );
				break;
			}

			case Nes::Machine::SOUND:
			{
				const Nes::Nsf nsf(emulator);

				HeapString types[] =
				{
											Resource::String( IDS_TEXT_FILE          ),
											Resource::String( IDS_TEXT_DIRECTORY     ),
					*nsf.GetName() ?        Resource::String( IDS_TEXT_NAME          ) : HeapString(),
					*nsf.GetArtist() ?      Resource::String( IDS_TEXT_ARTIST        ) : HeapString(),
					*nsf.GetMaker() ?       Resource::String( IDS_TEXT_MAKER         ) : HeapString(),
											Resource::String( IDS_TEXT_REGION        ),
											Resource::String( IDS_TEXT_SONGS         ),
					nsf.GetNumSongs() > 1 ? Resource::String( IDS_TEXT_STARTINGSONG  ) : HeapString(),
											Resource::String( IDS_TEXT_EXTRACHIPS    ),
											Resource::String( IDS_TEXT_BANKSWITCHING ),
											Resource::String( IDS_TEXT_LOADADDRESS   ),
											Resource::String( IDS_TEXT_INITADDRESS   ),
											Resource::String( IDS_TEXT_PLAYADDRESS   )
				};

				Table::Tab( types, NST_COUNT(types), fixedFont );

				types[0] << emulator.GetImagePath().File();
				types[1] << emulator.GetImagePath().Directory();

				if (*nsf.GetName())
					types[2].Import( nsf.GetName() );

				if (*nsf.GetArtist())
					types[3].Import( nsf.GetArtist() );

				if (*nsf.GetMaker())
					types[4].Import( nsf.GetMaker() );

				types[5] <<
				(
					nsf.GetMode() == Nes::Nsf::TUNE_MODE_NTSC ? "NTSC"     :
					nsf.GetMode() == Nes::Nsf::TUNE_MODE_PAL  ? "PAL"      :
																"NTSC/PAL"
				);

				types[6] << nsf.GetNumSongs();

				if (nsf.GetNumSongs() > 1)
					types[7] << (nsf.GetStartingSong() + 1);

				if (const uint chips = nsf.GetChips())
				{
					cstring c = "";

					if ( chips & Nes::Nsf::CHIP_MMC5 ) { types[8]      << "MMC5";      c = "+"; }
					if ( chips & Nes::Nsf::CHIP_VRC6 ) { types[8] << c << "VRC6";      c = "+"; }
					if ( chips & Nes::Nsf::CHIP_VRC7 ) { types[8] << c << "VRC7";      c = "+"; }
					if ( chips & Nes::Nsf::CHIP_N106 ) { types[8] << c << "N106";      c = "+"; }
					if ( chips & Nes::Nsf::CHIP_S5B  ) { types[8] << c << "Sunsoft5B"; c = "+"; }
					if ( chips & Nes::Nsf::CHIP_FDS  ) { types[8] << c << "FDS";       c = "+"; }
				}
				else
				{
					types[8] << Resource::String( IDS_TEXT_NO );
				}

				types[9] << Resource::String( nsf.UsesBankSwitching() ? IDS_TEXT_YES : IDS_TEXT_NO );

				types[10] << HexString( (u16) nsf.GetLoadAddress() );
				types[11] << HexString( (u16) nsf.GetInitAddress() );
				types[12] << HexString( (u16) nsf.GetPlayAddress() );

				Table::Output( text, types, NST_COUNT(types) );
				break;
			}
		}

		if (text.Length())
		{
			dialog.Edit(IDC_IMAGE_INFO_EDIT) << text.Ptr();

			Point delta = dialog.Edit(IDC_IMAGE_INFO_EDIT).GetMaxTextSize();
			Point size = dialog.Edit(IDC_IMAGE_INFO_EDIT).GetWindow().GetSize();

			if (delta.x > size.x)
			{
				delta.x = size.x;
				delta.y += ::GetSystemMetrics(SM_CXVSCROLL);
				::ShowScrollBar( dialog.Control(IDC_IMAGE_INFO_EDIT).GetWindow(), SB_HORZ, true );
			}

			if (delta != size)
			{
				dialog.Edit(IDC_IMAGE_INFO_EDIT).GetWindow().Resize( delta );
				delta = size - delta;

				size = dialog.Control(IDC_IMAGE_INFO_OK).GetWindow().GetPosition();
				::ScreenToClient( dialog, &size );
				dialog.Control(IDC_IMAGE_INFO_OK).GetWindow().Move( size-delta );

				dialog.Resize( dialog.GetSize() - delta );
			}
		}

		return true;
	}

	ibool ImageInfo::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return true;
	}
}
