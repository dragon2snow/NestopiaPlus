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
#include "NstManagerEmulator.hpp"
#include "NstDialogImageInfo.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include "../core/api/NstApiFds.hpp"
#include "../core/api/NstApiNsf.hpp"

namespace Nestopia
{
	namespace Window
	{
		ImageInfo::ImageInfo(Managers::Emulator& e)
		:
		dialog   (IDD_IMAGE_INFO,WM_INITDIALOG,this,&ImageInfo::OnInitDialog),
		emulator (e)
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

			const ibool fixedFont = dialog.Edit(IDC_IMAGE_INFO_EDIT).FixedFont();

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
						!info.name.empty() ?       Resource::String( IDS_TEXT_NAME        ) : HeapString(),
						!info.maker.empty() ?      Resource::String( IDS_TEXT_MAKER       ) : HeapString(),
                                                   Resource::String( IDS_TEXT_SYSTEM      ),
                                                   Resource::String( IDS_TEXT_CHIPSBOARDS ),
						info.setup.mapper <= 255 ? Resource::String( IDS_TEXT_MAPPER      ) : HeapString(),
                                                                 _T( "PRG-ROM"            ),
						info.setup.chrRom ?                      _T( "CHR-ROM"            ) : HeapString(),
						info.setup.chrRam ?                      _T( "CHR-RAM"            ) : HeapString(),
						info.setup.wrkRam +
						info.setup.wrkRamBacked ?                _T( "W-RAM"              ) : HeapString(),
                                                   Resource::String( IDS_TEXT_MIRRORING   ),
                                                   Resource::String( IDS_TEXT_BATTERY     ),
						info.setup.wrkRamBacked ?  Resource::String( IDS_TEXT_FILE        ) : HeapString(),
						info.setup.wrkRamBacked ?  Resource::String( IDS_TEXT_DIRECTORY   ) : HeapString(),
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
						info.setup.system == Nes::SYSTEM_VS   ? "VS"       :
						info.setup.system == Nes::SYSTEM_PC10 ? "PC10"     :
						info.setup.region == Nes::REGION_BOTH ? "NTSC/PAL" :
						info.setup.region == Nes::REGION_PAL  ? "PAL"      :
																"NTSC"
					);

					types[6].Import( info.board.c_str() );

					if (types[7].Length())
						types[7] << info.setup.mapper;

					types[8] << (uint) (info.setup.prgRom / 1024) << "k";

					if (info.setup.prgRom)
						types[8] << ", CRC: " << HexString( (u32) info.prgCrc );

					if (info.setup.chrRom)
						types[9] << (uint) (info.setup.chrRom / 1024) << "k, CRC: " << HexString( (u32) info.chrCrc );

					if (info.setup.chrRam)
						types[10] << (uint) (info.setup.chrRam / 1024) << "k";

					if (info.setup.wrkRam+info.setup.wrkRamBacked)
						types[11] << (uint) ((info.setup.wrkRam+info.setup.wrkRamBacked) / 1024) << (info.setup.wrkRamAuto ? "k auto" : "k");

					types[12] <<
					(
						info.setup.mirroring == Nes::Cartridge::MIRROR_HORIZONTAL ? Resource::String( IDS_TEXT_HORIZONTAL       ) :
						info.setup.mirroring == Nes::Cartridge::MIRROR_VERTICAL   ? Resource::String( IDS_TEXT_VERTICAL         ) :
						info.setup.mirroring == Nes::Cartridge::MIRROR_FOURSCREEN ? Resource::String( IDS_TEXT_FOURSCREEN       ) :
						info.setup.mirroring == Nes::Cartridge::MIRROR_ZERO       ?               _T( "$2000"                   ) :
						info.setup.mirroring == Nes::Cartridge::MIRROR_ONE        ?               _T( "$2400"                   ) :
						info.setup.mirroring == Nes::Cartridge::MIRROR_CONTROLLED ? Resource::String( IDS_TEXT_MAPPERCONTROLLED ) :
																					Resource::String( IDS_TEXT_UNKNOWN          )
					);

					types[13] << Resource::String
					(
						info.setup.wrkRamBacked ? IDS_TEXT_YES : IDS_TEXT_NO
					);

					if (info.setup.wrkRamBacked)
					{
						types[14] << emulator.GetSavePath().File();
						types[15] << emulator.GetSavePath().Directory();
					}

					types[16] << Resource::String( info.setup.trainer ? IDS_TEXT_YES : IDS_TEXT_NO );

					types[17] << Resource::String
					(
						info.condition == Nes::Cartridge::DUMP_OK         ? IDS_TEXT_OK :
						info.condition == Nes::Cartridge::DUMP_BAD        ? IDS_TEXT_BAD :
						info.condition == Nes::Cartridge::DUMP_REPAIRABLE ? IDS_TEXT_REPAIRABLE :
																			IDS_TEXT_UNKNOWN
					);

					Table::Output( text, types, NST_COUNT(types) );
					break;
				}

				case Nes::Machine::DISK:
				{
					Nes::Fds fds(emulator);

					std::vector<HeapString> types( 3 + NST_MAX(fds.GetNumSides()/2,1) );

					types[0] = Resource::String( IDS_TEXT_FILE      );
					types[1] = Resource::String( IDS_TEXT_DIRECTORY );
					types[2] = Resource::String( IDS_TEXT_HEADER    );

					for (uint i=3, n=types.size(); i < n; ++i)
						types[i] = Resource::String( IDS_TEXT_DISK ).Invoke( i-3+1 );

					Table::Tab( &types.front(), types.size(), fixedFont );

					types[0] << emulator.GetImagePath().File();
					types[1] << emulator.GetImagePath().Directory();
					types[2] << Resource::String( fds.HasHeader() ? IDS_TEXT_YES : IDS_TEXT_NO );

					for (uint i=0, n=fds.GetNumSides(); i < n; ++i)
					{
						Nes::Fds::DiskData data;
						fds.GetDiskData( i, data );

						uint size = 0;

						for (Nes::Fds::DiskData::Files::const_iterator it(data.files.begin()), end(data.files.end()); it != end; ++it)
							size += it->data.size();

						types[3+i/2] << (i % 2 ? ", B: " : "A: ")
                                     << (uint) (size / 1024)
                                     << "k "
                                     << Resource::String( IDS_TEXT_IN_FILES ).Invoke( data.files.size() );

						if (!data.raw.empty())
							types[3+i/2] << ", " << Resource::String( IDS_TEXT_TRAILING_DATA ).Invoke( data.raw.size() );
					}

					Table::Output( text, &types.front(), types.size() );
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

				Point size( dialog.Edit(IDC_IMAGE_INFO_EDIT).GetMaxTextSize() );
				Point delta( dialog.Edit(IDC_IMAGE_INFO_EDIT).GetWindow().Size() );

				if (size.x > delta.x)
				{
					size.x = delta.x;
					size.y += ::GetSystemMetrics(SM_CXVSCROLL);
					::ShowScrollBar( dialog.Control(IDC_IMAGE_INFO_EDIT).GetWindow(), SB_HORZ, true );
				}

				if (size != delta)
				{
					dialog.Edit(IDC_IMAGE_INFO_EDIT).GetWindow().Size() = size;
					delta -= size;
					dialog.Control(IDOK).GetWindow().Position() -= delta;
					dialog.Size() -= delta;
				}
			}

			return true;
		}
	}
}
