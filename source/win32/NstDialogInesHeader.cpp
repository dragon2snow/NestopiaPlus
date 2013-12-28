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

#include "NstWindowParam.hpp"
#include "NstIoFile.hpp"
#include "NstWindowUser.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include "../core/NstCrc32.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogInesHeader.hpp"

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		IDC_INESHEADER_CHRROM == IDC_INESHEADER_PRGROM + 1 &&
		IDC_INESHEADER_WRKRAM == IDC_INESHEADER_PRGROM + 2 &&
		IDC_INESHEADER_MAPPER == IDC_INESHEADER_PRGROM + 3
	);

	NST_COMPILE_ASSERT
	(
		IDC_INESHEADER_RESERVED_1 == IDC_INESHEADER_RESERVED_0 + 1 &&
		IDC_INESHEADER_RESERVED_2 == IDC_INESHEADER_RESERVED_0 + 2 &&
		IDC_INESHEADER_RESERVED_3 == IDC_INESHEADER_RESERVED_0 + 3 &&
		IDC_INESHEADER_RESERVED_4 == IDC_INESHEADER_RESERVED_0 + 4 &&
		IDC_INESHEADER_RESERVED_5 == IDC_INESHEADER_RESERVED_0 + 5 &&
		IDC_INESHEADER_RESERVED_6 == IDC_INESHEADER_RESERVED_0 + 6 &&
		IDC_INESHEADER_RESERVED_7 == IDC_INESHEADER_RESERVED_0 + 7
	);

	using namespace Window;

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
		{ IDC_INESHEADER_ORIGINAL, &InesHeader::OnCmdOriginal },
		{ IDC_INESHEADER_DETECT,   &InesHeader::OnCmdDetect   },
		{ IDC_INESHEADER_CANCEL,   &InesHeader::OnCmdCancel   },
		{ IDC_INESHEADER_SAVE,     &InesHeader::OnCmdSave     }
	};
  
	InesHeader::InesHeader(const Nes::Cartridge::Database& db,const Managers::Paths& p)
	: dialog(IDD_INESHEADER,this,Handlers::messages,Handlers::commands), database(db), paths(p) {}

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

				dbEntry = database.FindEntry
				( 
					buffer.Ptr(), 
					buffer.Size(), 
					header.num16kPRomBanks * Nes::Core::SIZE_16K + header.num8kCRomBanks * Nes::Core::SIZE_8K
				);
			}

			path = &p;
			dialog.Open();
		}
	}

	ibool InesHeader::IsBad(const Header& header,const ulong imageSize)
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
		dialog.Control( IDC_INESHEADER_DETECT ).Enable( dbEntry != NULL );
		
		for (uint i=IDC_INESHEADER_PRGROM; i <= IDC_INESHEADER_MAPPER; ++i)
			dialog.Edit( i ).Limit( 3 );

		for (uint i=IDC_INESHEADER_RESERVED_0; i <= IDC_INESHEADER_RESERVED_7; ++i)
			dialog.Edit( i ).Limit( 2 );

		UpdateOriginal();

		return TRUE;
	}

	ibool InesHeader::Save(Header& saveHeader) const
	{
		saveHeader.signature = Header::SIGNATURE;

		uint flags = 0;

		for (uint i=IDC_INESHEADER_PRGROM; i <= IDC_INESHEADER_MAPPER; ++i)
		{
			HeapString string;
			uint data;

			if (!(dialog.Edit( i ).Text() >> string) || !(string >> data) || data > 0xFF)
				return FALSE;

			switch (i)
			{
				case IDC_INESHEADER_PRGROM: saveHeader.num16kPRomBanks = (u8) data; break;
				case IDC_INESHEADER_CHRROM: saveHeader.num8kCRomBanks = (u8) data; break;
				case IDC_INESHEADER_WRKRAM: saveHeader.num8kWRamBanks = (u8) data; break;
				case IDC_INESHEADER_MAPPER: flags |= ((data & 0x0F) << 4) | ((data & 0xF0) << 8); break;
			}
		}

		if (dialog.CheckBox( IDC_INESHEADER_BATTERY ).IsChecked())
			flags |= Header::BATTERY;

		if (dialog.CheckBox( IDC_INESHEADER_TRAINER ).IsChecked()) 
			flags |= Header::TRAINER;

		if (dialog.CheckBox( IDC_INESHEADER_VSSYSTEM ).IsChecked()) 
			flags |= Header::VS;

		if (dialog.RadioButton( IDC_INESHEADER_VERTICAL ).IsChecked())
		{
			flags |= Header::VERTICAL;
		}
		else if (dialog.RadioButton( IDC_INESHEADER_FOURSCREEN ).IsChecked())
		{
			flags |= Header::FOURSCREEN;
		}

		for (uint i=IDC_INESHEADER_RESERVED_0; i <= IDC_INESHEADER_RESERVED_7; ++i)
		{
			HeapString string;

			if (!(dialog.Edit( i ).Text() >> string))
				return FALSE;

			string.Insert( 0, _T("0x") );
			
			uint data;

			if (!(string >> data) || data > 0xFF)
				return FALSE;

			if (i >= IDC_INESHEADER_RESERVED_2)
			{
				saveHeader.reserved[i - IDC_INESHEADER_RESERVED_2] = (u8) data;
			}
			else if (i == IDC_INESHEADER_RESERVED_1)
			{
				if (data > 0x7F)
					return FALSE;

				saveHeader.pal = (u8) (data << 1);
			}
			else
			{
				if (data > 0x7)
					return FALSE;

				flags |= (data << 9);
			}
		}

		saveHeader.flags = (u16) flags;

		if (dialog.CheckBox( IDC_INESHEADER_PAL ).IsChecked())
			saveHeader.pal |= Header::PAL;

		return TRUE;
	}

	void InesHeader::UpdateOriginal() const
	{
		dialog.Edit( IDC_INESHEADER_PRGROM ).Text() << header.num16kPRomBanks;
		dialog.Edit( IDC_INESHEADER_CHRROM ).Text() << header.num8kCRomBanks;
		dialog.Edit( IDC_INESHEADER_WRKRAM ).Text() << header.num8kWRamBanks;

		dialog.Edit( IDC_INESHEADER_MAPPER ).Text() <<
		(
     		((header.flags & Header::MAPPER_LO) >> 4) | 
			((header.flags & Header::MAPPER_HI) >> 8)
		);

		dialog.CheckBox( IDC_INESHEADER_BATTERY  ).Check( header.flags & Header::BATTERY );
		dialog.CheckBox( IDC_INESHEADER_TRAINER  ).Check( header.flags & Header::TRAINER );
		dialog.CheckBox( IDC_INESHEADER_VSSYSTEM ).Check( header.flags & Header::VS      );
		dialog.CheckBox( IDC_INESHEADER_PAL      ).Check( header.pal   & Header::PAL     );

		dialog.RadioButton( IDC_INESHEADER_HORIZONTAL ).Check( (header.flags & (Header::FOURSCREEN|Header::VERTICAL)) == 0 );
		dialog.RadioButton( IDC_INESHEADER_VERTICAL   ).Check( (header.flags & (Header::FOURSCREEN|Header::VERTICAL)) == Header::VERTICAL );
		dialog.RadioButton( IDC_INESHEADER_FOURSCREEN ).Check( (header.flags & Header::FOURSCREEN) == Header::FOURSCREEN );

		dialog.Edit( IDC_INESHEADER_RESERVED_0 ).Text() << HexString( (u8) ((header.flags & 0x0E00) >> 9), true ).Ptr();
		dialog.Edit( IDC_INESHEADER_RESERVED_1 ).Text() << HexString( (u8) ((header.pal & 0xFE) >> 1), true ).Ptr();

		for (uint i=IDC_INESHEADER_RESERVED_2; i <= IDC_INESHEADER_RESERVED_7; ++i)
			dialog.Edit( i ).Text() << HexString( (u8) header.reserved[i-IDC_INESHEADER_RESERVED_2], true ).Ptr();
	}

	void InesHeader::UpdateDetect() const
	{
		if (dbEntry)
		{
			dialog.Edit( IDC_INESHEADER_PRGROM ).Text() << (uint) (database.GetPRomSize(dbEntry) / Nes::Core::SIZE_16K);
			dialog.Edit( IDC_INESHEADER_CHRROM ).Text() << (uint) (database.GetCRomSize(dbEntry) / Nes::Core::SIZE_8K);
			dialog.Edit( IDC_INESHEADER_WRKRAM ).Text() << (uint) (database.GetWRamSize(dbEntry) / Nes::Core::SIZE_8K);

			dialog.Edit( IDC_INESHEADER_MAPPER ).Text() << database.GetMapper(dbEntry);

			dialog.CheckBox( IDC_INESHEADER_BATTERY ).Check( database.HasBattery(dbEntry) );
			dialog.CheckBox( IDC_INESHEADER_TRAINER ).Check( database.HasTrainer(dbEntry) );

			const Nes::Cartridge::System system = database.GetSystem(dbEntry);

			dialog.CheckBox( IDC_INESHEADER_VSSYSTEM ).Check( system == Nes::Cartridge::SYSTEM_VS  );
			dialog.CheckBox( IDC_INESHEADER_PAL      ).Check( system == Nes::Cartridge::SYSTEM_PAL );

			const Nes::Cartridge::Mirroring mirroring = database.GetMirroring(dbEntry);

			dialog.RadioButton( IDC_INESHEADER_HORIZONTAL ).Check( mirroring != Nes::Cartridge::MIRROR_VERTICAL && mirroring != Nes::Cartridge::MIRROR_FOURSCREEN );
			dialog.RadioButton( IDC_INESHEADER_VERTICAL   ).Check( mirroring == Nes::Cartridge::MIRROR_VERTICAL );
			dialog.RadioButton( IDC_INESHEADER_FOURSCREEN ).Check( mirroring == Nes::Cartridge::MIRROR_FOURSCREEN );

			for (uint i=IDC_INESHEADER_RESERVED_0; i <= IDC_INESHEADER_RESERVED_7; ++i)
				dialog.Edit( i ).Text() << "00";
		}
	}

	ibool InesHeader::OnCmdOriginal(Param& param)
	{
		if (param.Button().IsClicked())
			UpdateOriginal();

		return TRUE;
	}

	ibool InesHeader::OnCmdDetect(Param& param)
	{
		if (param.Button().IsClicked())
			UpdateDetect();

		return TRUE;
	}

	ibool InesHeader::OnCmdCancel(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}

	ibool InesHeader::OnCmdSave(Param& param)
	{
		if (param.Button().IsClicked())
		{
			Header saveHeader;

			if (!Save( saveHeader ))
			{
				User::Warn( IDS_INESHEADER_INVALID, IDS_TITLE_ERROR );
				return TRUE;
			}
			
			if (IsBad( saveHeader, imageSize ) && !User::Confirm( IDS_INESHEADER_UNSAFE ))
				return TRUE;

			const Path savePath( paths.BrowseSave( Managers::Paths::File::INES, Managers::Paths::DONT_SUGGEST, *path ) );

			if (savePath.Empty())
				return TRUE;
			
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
					return TRUE;
				}
			}

			User::Fail( result );					
		}

		return TRUE;
	}
}
