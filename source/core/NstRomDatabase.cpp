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

#include "../paradox/PdxQuickSort.h"
#include "../paradox/PdxUtilities.h"
#include "NstTypes.h"
#include "NstRomDatabase.h"

#ifdef NES_USE_ROM_DATABASE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include <Windows.h>
#include "../windows/resource/resource.h"

NES_NAMESPACE_BEGIN

#define NST_FROM_HANDLE(h_) PDX_CAST(const ENTRY* const,h_)

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

ROMDATABASE::ROMDATABASE()
{
	const CHAR* iterator = NULL;		
	const CHAR* end = NULL;

	HGLOBAL hGlobal;
	HMODULE hModule = ::GetModuleHandle( NULL );		
	HRSRC hRsrc = ::FindResource( hModule, MAKEINTRESOURCE(IDR_ROMDATABASE1), "RomDatabase" );

	if (hRsrc && (hGlobal = ::LoadResource( hModule, hRsrc )))
	{
		const DWORD size = ::SizeofResource( hModule, hRsrc );

		if (size)
		{
			iterator = (const CHAR*) ::LockResource( hGlobal );
			end = iterator + size;
		}
	}

	if (iterator)
	{
		const TSIZE count = *PDX_CAST(const U16*,iterator);
		iterator += sizeof(U16);

		copyrights.Reserve( count );

		for (TSIZE i=0; i < count; ++i)
		{
			copyrights.InsertBack( iterator );
			iterator += strlen(iterator) + 1;
		}

		while (iterator < end)
		{
			const CHAR* const name = iterator;
			iterator += strlen(iterator) + 1;
			database.InsertBack( ENTRY(name,*PDX_CAST(const INFO*,iterator)) );
			iterator += sizeof(INFO);
		}
	}

	PDXQUICKSORT::Sort( database.Begin(), database.End() );
	database.Defrag();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

ROMDATABASE::HANDLE ROMDATABASE::GetHandle(const ULONG crc) const
{
	DATABASE::CONSTITERATOR iterator( PDX::BinarySearch( database.Begin(), database.End(), crc ) );
	return (iterator != database.End() ? &(*iterator) : NULL);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const CHAR* ROMDATABASE::Name(HANDLE h) const 
{ 
	return NST_FROM_HANDLE(h)->name;
}

const CHAR* ROMDATABASE::Copyright(HANDLE h) const 
{ 
	const TSIZE index = NST_FROM_HANDLE(h)->info.copyright;
	return (index < copyrights.Size() ? copyrights[index] : ""); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SYSTEM ROMDATABASE::System(HANDLE h) const 
{
	const INFO& info = NST_FROM_HANDLE(h)->info; 

	SYSTEM system;

	if      ( info.p10  ) system = SYSTEM_PC10;
	else if ( info.vs   ) system = SYSTEM_VS;
	else if ( info.ntsc ) system = SYSTEM_NTSC;
	else if ( info.pal  ) system = SYSTEM_PAL;
	else		   	      system = SYSTEM_NTSC;

	return system;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MIRRORING ROMDATABASE::Mirroring(HANDLE h) const 
{ 
	return MIRRORING(NST_FROM_HANDLE(h)->info.mirroring); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

ULONG ROMDATABASE::Crc          (HANDLE h) const { return NST_FROM_HANDLE( h )->info.crc;             }
ULONG ROMDATABASE::pRomCrc      (HANDLE h) const { return NST_FROM_HANDLE( h )->info.pRomCrc;         }
TSIZE ROMDATABASE::pRomSize     (HANDLE h) const { return NST_FROM_HANDLE( h )->info.pRomSize * n16k; }
TSIZE ROMDATABASE::cRomSize     (HANDLE h) const { return NST_FROM_HANDLE( h )->info.cRomSize * n8k;  }
TSIZE ROMDATABASE::wRamSize     (HANDLE h) const { return NST_FROM_HANDLE( h )->info.wRamSize * n8k;  }	
UINT  ROMDATABASE::Mapper       (HANDLE h) const { return NST_FROM_HANDLE( h )->info.mapper;          }
BOOL  ROMDATABASE::HasBattery   (HANDLE h) const { return NST_FROM_HANDLE( h )->info.battery;         }
BOOL  ROMDATABASE::HasTrainer   (HANDLE h) const { return NST_FROM_HANDLE( h )->info.trainer;         }
BOOL  ROMDATABASE::IsBad        (HANDLE h) const { return NST_FROM_HANDLE( h )->info.bad;             }
BOOL  ROMDATABASE::IsHacked     (HANDLE h) const { return NST_FROM_HANDLE( h )->info.hack;            }
BOOL  ROMDATABASE::IsTranslated (HANDLE h) const { return NST_FROM_HANDLE( h )->info.translation;     }
BOOL  ROMDATABASE::IsUnlicenced (HANDLE h) const { return NST_FROM_HANDLE( h )->info.unlicensed;      }
BOOL  ROMDATABASE::IsBootleg    (HANDLE h) const { return NST_FROM_HANDLE( h )->info.bootleg;         }

NES_NAMESPACE_END

#endif

