////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include <Windows.h>
#include "../paradox/PdxString.h"
#include "NstRominfo.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL ROMINFO::DialogProc(HWND hDlg,UINT uMsg,WPARAM,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:
		{
			uggly = FALSE;

			const NES::IO::CARTRIDGE::INFO* const info = nes.GetCartridgeInfo();

			if (!info)
				return TRUE;

			PDXSTRING text;
			PDXSTRING file;
			
			info->file.GetFileName(file);
			
			text << "File:       " << file << "\r\n";
			text << "crc:        ";

			text.Append(info->crc,PDXSTRING::HEX);
			text << "\r\n";

			text << "Name:       " << info->name          << "\r\n";
			text << "Copyright:  " << info->copyright     << "\r\n";
			text << "Board:      " << info->board         << "\r\n";
			text << "Mapper:     " << info->mapper        << "\r\n";
			text << "PRG-ROM:    " << (info->pRom / 1024) << "k ";
			
			if (info->pRom)
			{
				text << "crc: ";
				text.Append(info->pRomCrc,PDXSTRING::HEX);
			}

			text << "\r\n";
			text << "CHR-ROM:    " << (info->cRom / 1024) << "k ";
			
			if (info->cRom)
			{
				text << "crc: ";
     			text.Append(info->cRomCrc,PDXSTRING::HEX);
			}
			
			text << "\r\n";
			text << "System:     ";

			switch (info->system)
			{
       			case NES::SYSTEM_NTSC: text << "NTSC\r\n";         break;
				case NES::SYSTEM_PAL:  text << "PAL\r\n";          break;
				case NES::SYSTEM_VS:   text << "VS\r\n";           break;
				case NES::SYSTEM_PC10: text << "PlayChoice10\r\n"; break;
			}

			text << "Mirroring:  ";

			switch (info->mirroring)
			{
	     		case NES::MIRROR_HORIZONTAL: text << "horizontal\r\n";  break;
	       		case NES::MIRROR_VERTICAL:	 text << "vertical\r\n";    break;
       			case NES::MIRROR_FOURSCREEN: text << "four-screen\r\n"; break;
				case NES::MIRROR_ZERO:       text << "first bank\r\n";  break;
				case NES::MIRROR_ONE:        text << "second bank\r\n"; break;
				case NES::MIRROR_TWO:        text << "third bank\r\n";  break;
				case NES::MIRROR_THREE:      text << "fourth bank\r\n"; break;
			}

			enum
			{
				YES     = NES::IO::CARTRIDGE::YES,
				NO      = NES::IO::CARTRIDGE::NO,
				GOOD    = NES::IO::CARTRIDGE::GOOD,
				BAD     = NES::IO::CARTRIDGE::BAD,
				UNKNOWN	= NES::IO::CARTRIDGE::UNKNOWN
			};

			text << "Battery:    " << (info->battery ? "yes\r\n" : "no\r\n");
			text << "Trainer:    " << (info->trained    == YES  ? "yes\r\n"  : "no\r\n");
			text << "Condition:  " << (info->condition  == GOOD ? "good\r\n" : "bad\r\n");
			text << "Licensed:   " << (info->licensed   == YES  ? "yes\r\n"  : (info->licensed   == NO ? "no\r\n" : "unknown\r\n" ));
			text << "Bootleg:    " << (info->bootleg    == YES  ? "yes\r\n"  : (info->bootleg    == NO ? "no\r\n" : "unknown\r\n" ));
			text << "Hacked:     " << (info->hacked     == YES  ? "yes\r\n"  : (info->hacked     == NO ? "no\r\n" : "unknown\r\n" ));
			text << "Translated: " << (info->translated == YES  ? "yes"      : (info->translated == NO ? "no"     : "unknown"     ));

			::SetDlgItemText( hDlg, IDC_ROM_INFO_EDIT, text.String() );

			return TRUE;
		}     

		case WM_ACTIVATE:
		
			if (!uggly && hDlg)
			{
				// Every time the dialog starts, the text gets automaticaly selected !!!?

				uggly = TRUE;
				::SendMessage( ::GetDlgItem( hDlg, IDC_ROM_INFO_EDIT ), EM_SETSEL, WPARAM(-1), LPARAM(0) );
			}
			return FALSE;
  
     	case WM_CLOSE:			 

     		::EndDialog( hDlg, 0 );
     		return TRUE;
	}

	return FALSE;
}
