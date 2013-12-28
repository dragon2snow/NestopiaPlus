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
#include "NstIoFile.hpp"
#include "NstWindowUser.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogInesHeader.hpp"

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_INES_HEADER_CHRROM == IDC_INES_HEADER_PRGROM + 1 &&
			IDC_INES_HEADER_WRKRAM == IDC_INES_HEADER_PRGROM + 2 &&
			IDC_INES_HEADER_MAPPER == IDC_INES_HEADER_PRGROM + 3
		);

		NST_COMPILE_ASSERT
		(
			IDC_INES_HEADER_RESERVED_1 == IDC_INES_HEADER_RESERVED_0 + 1 &&
			IDC_INES_HEADER_RESERVED_2 == IDC_INES_HEADER_RESERVED_0 + 2 &&
			IDC_INES_HEADER_RESERVED_3 == IDC_INES_HEADER_RESERVED_0 + 3 &&
			IDC_INES_HEADER_RESERVED_4 == IDC_INES_HEADER_RESERVED_0 + 4 &&
			IDC_INES_HEADER_RESERVED_5 == IDC_INES_HEADER_RESERVED_0 + 5 &&
			IDC_INES_HEADER_RESERVED_6 == IDC_INES_HEADER_RESERVED_0 + 6 &&
			IDC_INES_HEADER_RESERVED_7 == IDC_INES_HEADER_RESERVED_0 + 7
		);

		struct InesHeader::Handlers
		{
			static const MsgHandler::Entry<InesHeader> messages[];
			static const MsgHandler::Entry<InesHeader> commands[];
		};

		const MsgHandler::Entry<InesHeader> InesHeader::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &InesHeader::OnInitDialog },
		};

		const MsgHandler::Entry<InesHeader> InesHeader::Handlers::commands[] =
		{
			{ IDC_INES_HEADER_ORIGINAL, &InesHeader::OnCmdOriginal },
			{ IDC_INES_HEADER_DETECT,   &InesHeader::OnCmdDetect   },
			{ IDC_INES_HEADER_SAVE,     &InesHeader::OnCmdSave     }
		};

		InesHeader::InesHeader(const Nes::Cartridge::Database& db,const Managers::Paths& p)
		: dialog(IDD_INES_HEADER,this,Handlers::messages,Handlers::commands), database(db), paths(p) {}

		uint InesHeader::Import(const Path& loadPath,Collection::Buffer& buffer)
		{
			NST_ASSERT( buffer.Empty() );

			try
			{
				Io::File file( loadPath, Io::File::COLLECT );

				if (file.Size() >= sizeof(Header) && file.Peek<u32>() == Header::SIGNATURE)
				{
					file.Stream() >> buffer;
					return 0;
				}
				else
				{
					return IDS_FILE_ERR_INVALID;
				}
			}
			catch (Io::File::Exception ids)
			{
				return ids;
			}
			catch (const std::bad_alloc&)
			{
				return IDS_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return IDS_ERR_GENERIC;
			}
		}

		uint InesHeader::Export(const Path& savePath,const Collection::Buffer& buffer)
		{
			NST_ASSERT( buffer.Size() >= sizeof(Header) );

			try
			{
				Io::File file( savePath, Io::File::READ|Io::File::WRITE );

				if (file.Size() == 0 || file.Peek<u32>() == Header::SIGNATURE)
				{
					file.Stream() << buffer;
					return 0;
				}
				else
				{
					return IDS_FILE_ERR_INVALID;
				}
			}
			catch (Io::File::Exception i)
			{
				return i;
			}
			catch (const std::bad_alloc&)
			{
				return IDS_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return IDS_ERR_GENERIC;
			}
		}

		void InesHeader::Open(const Path& p)
		{
			if (p.Length())
			{
				{
					Collection::Buffer buffer;

					if (const uint result = Import( p, buffer ))
					{
						User::Fail( result );
						return;
					}

					imageSize = buffer.Size() - sizeof(Header);
					std::memcpy( &header, buffer.Ptr(), sizeof(Header) );

					dbEntry = database.FindEntry( buffer.Ptr(), buffer.Size() );
				}

				path = &p;
				dialog.Open();
			}
		}

		ibool InesHeader::Bad(const Header& header,const ulong imageSize)
		{
			return
			(
				(header.pal & ~Header::PAL) ||
				*reinterpret_cast<const u32*>(header.reserved+0) ||
				*reinterpret_cast<const u16*>(header.reserved+4) ||
				(header.num16kPRomBanks * Nes::Core::SIZE_16K + header.num8kCRomBanks * Nes::Core::SIZE_8K + ((header.flags & Header::TRAINER) ? 0x200U : 0x000U) > imageSize)
			);
		}

		ibool InesHeader::OnInitDialog(Param&)
		{
			dialog.Control( IDC_INES_HEADER_DETECT ).Enable( dbEntry != NULL );

			for (uint i=IDC_INES_HEADER_PRGROM; i <= IDC_INES_HEADER_MAPPER; ++i)
				dialog.Edit( i ).Limit( 3 );

			for (uint i=IDC_INES_HEADER_RESERVED_0; i <= IDC_INES_HEADER_RESERVED_7; ++i)
				dialog.Edit( i ).Limit( 2 );

			UpdateOriginal();

			return true;
		}

		ibool InesHeader::Save(Header& saveHeader) const
		{
			saveHeader.signature = Header::SIGNATURE;

			uint flags = 0;

			for (uint i=IDC_INES_HEADER_PRGROM; i <= IDC_INES_HEADER_MAPPER; ++i)
			{
				HeapString string;
				uint data;

				if (!(dialog.Edit( i ).Text() >> string) || !(string >> data) || data > 0xFF)
					return false;

				switch (i)
				{
					case IDC_INES_HEADER_PRGROM: saveHeader.num16kPRomBanks = data; break;
					case IDC_INES_HEADER_CHRROM: saveHeader.num8kCRomBanks = data; break;
					case IDC_INES_HEADER_WRKRAM: saveHeader.num8kWRamBanks = data; break;
					case IDC_INES_HEADER_MAPPER: flags |= ((data & 0x0F) << 4) | ((data & 0xF0) << 8); break;
				}
			}

			if (dialog.CheckBox( IDC_INES_HEADER_BATTERY ).Checked())
				flags |= Header::BATTERY;

			if (dialog.CheckBox( IDC_INES_HEADER_TRAINER ).Checked())
				flags |= Header::TRAINER;

			if (dialog.CheckBox( IDC_INES_HEADER_VSSYSTEM ).Checked())
				flags |= Header::VS;

			if (dialog.RadioButton( IDC_INES_HEADER_VERTICAL ).Checked())
			{
				flags |= Header::VERTICAL;
			}
			else if (dialog.RadioButton( IDC_INES_HEADER_FOURSCREEN ).Checked())
			{
				flags |= Header::FOURSCREEN;
			}

			for (uint i=IDC_INES_HEADER_RESERVED_0; i <= IDC_INES_HEADER_RESERVED_7; ++i)
			{
				HeapString string;

				if (!(dialog.Edit( i ).Text() >> string))
					return false;

				string.Insert( 0, _T("0x") );

				uint data;

				if (!(string >> data) || data > 0xFF)
					return false;

				if (i >= IDC_INES_HEADER_RESERVED_2)
				{
					saveHeader.reserved[i - IDC_INES_HEADER_RESERVED_2] = data;
				}
				else if (i == IDC_INES_HEADER_RESERVED_1)
				{
					if (data > 0x7F)
						return false;

					saveHeader.pal = data << 1;
				}
				else
				{
					if (data > 0x7)
						return false;

					flags |= (data << 9);
				}
			}

			saveHeader.flags = flags;

			if (dialog.CheckBox( IDC_INES_HEADER_PAL ).Checked())
				saveHeader.pal |= Header::PAL;

			return true;
		}

		void InesHeader::UpdateOriginal() const
		{
			dialog.Edit( IDC_INES_HEADER_PRGROM ).Text() << header.num16kPRomBanks;
			dialog.Edit( IDC_INES_HEADER_CHRROM ).Text() << header.num8kCRomBanks;
			dialog.Edit( IDC_INES_HEADER_WRKRAM ).Text() << header.num8kWRamBanks;

			dialog.Edit( IDC_INES_HEADER_MAPPER ).Text() <<
			(
				((header.flags & Header::MAPPER_LO) >> 4) |
				((header.flags & Header::MAPPER_HI) >> 8)
			);

			dialog.CheckBox( IDC_INES_HEADER_BATTERY  ).Check( header.flags & Header::BATTERY );
			dialog.CheckBox( IDC_INES_HEADER_TRAINER  ).Check( header.flags & Header::TRAINER );
			dialog.CheckBox( IDC_INES_HEADER_VSSYSTEM ).Check( header.flags & Header::VS      );
			dialog.CheckBox( IDC_INES_HEADER_PAL      ).Check( header.pal   & Header::PAL     );

			dialog.RadioButton( IDC_INES_HEADER_HORIZONTAL ).Check( (header.flags & (Header::FOURSCREEN|Header::VERTICAL)) == 0 );
			dialog.RadioButton( IDC_INES_HEADER_VERTICAL   ).Check( (header.flags & (Header::FOURSCREEN|Header::VERTICAL)) == Header::VERTICAL );
			dialog.RadioButton( IDC_INES_HEADER_FOURSCREEN ).Check( (header.flags & Header::FOURSCREEN) == Header::FOURSCREEN );

			dialog.Edit( IDC_INES_HEADER_RESERVED_0 ).Text() << HexString( (u8) ((header.flags & 0x0E00) >> 9), true ).Ptr();
			dialog.Edit( IDC_INES_HEADER_RESERVED_1 ).Text() << HexString( (u8) ((header.pal & 0xFE) >> 1), true ).Ptr();

			for (uint i=IDC_INES_HEADER_RESERVED_2; i <= IDC_INES_HEADER_RESERVED_7; ++i)
				dialog.Edit( i ).Text() << HexString( (u8) header.reserved[i-IDC_INES_HEADER_RESERVED_2], true ).Ptr();
		}

		void InesHeader::UpdateDetect() const
		{
			if (dbEntry)
			{
				dialog.Edit( IDC_INES_HEADER_PRGROM ).Text() << (uint) (database.GetPRomSize(dbEntry) / Nes::Core::SIZE_16K);
				dialog.Edit( IDC_INES_HEADER_CHRROM ).Text() << (uint) (database.GetCRomSize(dbEntry) / Nes::Core::SIZE_8K);
				dialog.Edit( IDC_INES_HEADER_WRKRAM ).Text() << (uint) (database.GetWRamSize(dbEntry) / Nes::Core::SIZE_8K);

				dialog.Edit( IDC_INES_HEADER_MAPPER ).Text() << database.GetMapper(dbEntry);

				dialog.CheckBox( IDC_INES_HEADER_BATTERY ).Check( database.HasBattery(dbEntry) );
				dialog.CheckBox( IDC_INES_HEADER_TRAINER ).Check( database.HasTrainer(dbEntry) );

				const Nes::Cartridge::System system = database.GetSystem(dbEntry);

				dialog.CheckBox( IDC_INES_HEADER_VSSYSTEM ).Check( system == Nes::Cartridge::SYSTEM_VS  );
				dialog.CheckBox( IDC_INES_HEADER_PAL      ).Check( system == Nes::Cartridge::SYSTEM_PAL );

				const Nes::Cartridge::Mirroring mirroring = database.GetMirroring(dbEntry);

				dialog.RadioButton( IDC_INES_HEADER_HORIZONTAL ).Check( mirroring != Nes::Cartridge::MIRROR_VERTICAL && mirroring != Nes::Cartridge::MIRROR_FOURSCREEN );
				dialog.RadioButton( IDC_INES_HEADER_VERTICAL   ).Check( mirroring == Nes::Cartridge::MIRROR_VERTICAL );
				dialog.RadioButton( IDC_INES_HEADER_FOURSCREEN ).Check( mirroring == Nes::Cartridge::MIRROR_FOURSCREEN );

				for (uint i=IDC_INES_HEADER_RESERVED_0; i <= IDC_INES_HEADER_RESERVED_7; ++i)
					dialog.Edit( i ).Text() << "00";
			}
		}

		ibool InesHeader::OnCmdOriginal(Param& param)
		{
			if (param.Button().Clicked())
				UpdateOriginal();

			return true;
		}

		ibool InesHeader::OnCmdDetect(Param& param)
		{
			if (param.Button().Clicked())
				UpdateDetect();

			return true;
		}

		ibool InesHeader::OnCmdSave(Param& param)
		{
			if (param.Button().Clicked())
			{
				Header saveHeader;

				if (!Save( saveHeader ))
				{
					User::Warn( IDS_INES_HEADER_INVALID, IDS_TITLE_ERROR );
					return true;
				}

				if (Bad( saveHeader, imageSize ) && !User::Confirm( IDS_INES_HEADER_UNSAFE ))
					return true;

				const Path savePath( paths.BrowseSave( Managers::Paths::File::INES, Managers::Paths::DONT_SUGGEST, *path ) );

				if (savePath.Empty())
					return true;

				Collection::Buffer buffer;
				uint result = Import( *path, buffer );

				if (result == 0)
				{
					NST_ASSERT( buffer.Size() >= sizeof(Header) );
					std::memcpy( buffer.Ptr(), &saveHeader, sizeof(Header) );

					result = Export( savePath, buffer );

					if (result == 0)
					{
						dialog.Close();
						return true;
					}
				}

				User::Fail( result );
			}

			return true;
		}
	}
}
