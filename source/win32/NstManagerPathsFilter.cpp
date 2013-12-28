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

#include "NstString.hpp"
#include "NstResourceString.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerPathsFilter.hpp"

namespace Nestopia
{
	#define NST_FILTER_INES_ALL         ", *.nes"
	#define NST_FILTER_UNIF_ALL         ", *.unf"
	#define NST_FILTER_FDS_ALL          ", *.fds"
	#define NST_FILTER_NSF_ALL          ", *.nsf"
	#define NST_FILTER_ROM_ALL          ", *.rom"
	#define NST_FILTER_SCRIPT_ALL       ", *.nsp"
	#define NST_FILTER_BATTERY_ALL      ", *.sav"
	#define NST_FILTER_TAPE_ALL         ", *.tp"
	#define NST_FILTER_STATE_ALL        ", *.nst"
	#define NST_FILTER_MOVIE_ALL        ", *.nsv"
	#define NST_FILTER_IPS_ALL          ", *.ips"
	#define NST_FILTER_SLOTS_ALL        ", *.ns1..9"
	#define NST_FILTER_PALETTE_ALL      ", *.pal"
	#define NST_FILTER_WAVE_ALL         ", *.wav"
	#define NST_FILTER_AVI_ALL          ", *.avi"
	#define NST_FILTER_ARCHIVE_ALL      ", *.zip, *.rar, *.7z"
	#define NST_FILTER_INES             ";*.nes"
	#define NST_FILTER_UNIF             ";*.unf;*.unif"
	#define NST_FILTER_FDS              ";*.fds"
	#define NST_FILTER_NSF              ";*.nsf"
	#define NST_FILTER_ROM              ";*.rom"
	#define NST_FILTER_SCRIPT           ";*.nsp"
	#define NST_FILTER_BATTERY          ";*.sav"
	#define NST_FILTER_TAPE             ";*.tp"
	#define NST_FILTER_STATE            ";*.nst"
	#define NST_FILTER_MOVIE            ";*.nsv"
	#define NST_FILTER_IPS              ";*.ips"
	#define NST_FILTER_SLOTS            ";*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9"
	#define NST_FILTER_PALETTE          ";*.pal"
	#define NST_FILTER_WAVE             ";*.wav"
	#define NST_FILTER_AVI              ";*.avi"
	#define NST_FILTER_ARCHIVE          ";*.zip;*.rar;*.7z"
	#define NST_FILTER_INES_DESC        " (*.nes)\t*.nes"
	#define NST_FILTER_UNIF_DESC        " (*.unf,*.unif)\t*.unf;*.unif"
	#define NST_FILTER_FDS_DESC         " (*.fds)\t*.fds"
	#define NST_FILTER_NSF_DESC         " (*.nsf)\t*.nsf"
	#define NST_FILTER_ROM_DESC         " (*.rom)\t*.rom"
	#define NST_FILTER_SCRIPT_DESC      " (*.nsp)\t*.nsp"
	#define NST_FILTER_BATTERY_DESC     " (*.sav)\t*.sav"
	#define NST_FILTER_TAPE_DESC        " (*.tp)\t*.tp"
	#define NST_FILTER_STATE_SLOTS_DESC " (*.nst,*.ns1..9)\t*.nst;*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9"
	#define NST_FILTER_STATE_DESC       " (*.nst)\t*.nst"
	#define NST_FILTER_SLOTS_DESC       " (*.ns1..9)\t*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9"
	#define NST_FILTER_MOVIE_DESC       " (*.nsv)\t*.nsv"
	#define NST_FILTER_IPS_DESC         " (*.ips)\t*.ips"
	#define NST_FILTER_PALETTE_DESC     " (*.pal)\t*.pal"
	#define NST_FILTER_WAVE_DESC        " (*.wav)\t*.wav"
	#define NST_FILTER_AVI_DESC         " (*.avi)\t*.avi"
	#define NST_FILTER_ARCHIVE_DESC     " (*.zip,*.rar,*.7z)\t*.zip;*.rar;*.7z"
	#define NST_FILTER_ALL_DESC         " (*.*)\t*.*\t"

	Managers::Paths::Filter::Filter(const File::Types types)
	{
		NST_ASSERT( types.Word() );

		(*this) << Resource::String(IDS_FILES_SUPPORTED);

		uint offset = Length();

		if (types( File::INES    )) (*this) << NST_FILTER_INES_ALL;
		if (types( File::UNIF    )) (*this) << NST_FILTER_UNIF_ALL;
		if (types( File::FDS     )) (*this) << NST_FILTER_FDS_ALL;
		if (types( File::NSF     )) (*this) << NST_FILTER_NSF_ALL;
		if (types( File::ROM     )) (*this) << NST_FILTER_ROM_ALL;
		if (types( File::SCRIPT  )) (*this) << NST_FILTER_SCRIPT_ALL;
		if (types( File::BATTERY )) (*this) << NST_FILTER_BATTERY_ALL;
		if (types( File::TAPE    )) (*this) << NST_FILTER_TAPE_ALL;
		if (types( File::STATE   )) (*this) << NST_FILTER_STATE_ALL;
		if (types( File::SLOTS   )) (*this) << NST_FILTER_SLOTS_ALL;
		if (types( File::MOVIE   )) (*this) << NST_FILTER_MOVIE_ALL;
		if (types( File::IPS     )) (*this) << NST_FILTER_IPS_ALL;
		if (types( File::PALETTE )) (*this) << NST_FILTER_PALETTE_ALL;
		if (types( File::WAVE    )) (*this) << NST_FILTER_WAVE_ALL;
		if (types( File::AVI     )) (*this) << NST_FILTER_AVI_ALL;
		if (types( File::ARCHIVE )) (*this) << NST_FILTER_ARCHIVE_ALL;

		if (Length() <= MAX_FILE_DIALOG_WIDTH)
		{
			At(offset + 0) = ' ';
			At(offset + 1) = '(';
			(*this) << ')';

			offset = Length();
		}
		else
		{
			ShrinkTo( offset );
		}

		if (types( File::INES    )) (*this) << NST_FILTER_INES;
		if (types( File::UNIF    )) (*this) << NST_FILTER_UNIF;
		if (types( File::FDS     )) (*this) << NST_FILTER_FDS;
		if (types( File::NSF     )) (*this) << NST_FILTER_NSF;
		if (types( File::ROM     )) (*this) << NST_FILTER_ROM;
		if (types( File::SCRIPT  )) (*this) << NST_FILTER_SCRIPT;
		if (types( File::BATTERY )) (*this) << NST_FILTER_BATTERY;
		if (types( File::TAPE    )) (*this) << NST_FILTER_TAPE;
		if (types( File::STATE   )) (*this) << NST_FILTER_STATE;
		if (types( File::MOVIE   )) (*this) << NST_FILTER_MOVIE;
		if (types( File::IPS     )) (*this) << NST_FILTER_IPS;
		if (types( File::SLOTS   )) (*this) << NST_FILTER_SLOTS;
		if (types( File::PALETTE )) (*this) << NST_FILTER_PALETTE;
		if (types( File::WAVE    )) (*this) << NST_FILTER_WAVE;
		if (types( File::AVI     )) (*this) << NST_FILTER_AVI;
		if (types( File::ARCHIVE )) (*this) << NST_FILTER_ARCHIVE;

		At(offset) = '\t';

		if (types( File::INES     )) (*this) << '\t' << Resource::String( IDS_FILES_INES    ) << NST_FILTER_INES_DESC;
		if (types( File::UNIF     )) (*this) << '\t' << Resource::String( IDS_FILES_UNIF    ) << NST_FILTER_UNIF_DESC;
		if (types( File::FDS      )) (*this) << '\t' << Resource::String( IDS_FILES_FDS     ) << NST_FILTER_FDS_DESC;
		if (types( File::NSF      )) (*this) << '\t' << Resource::String( IDS_FILES_NSF     ) << NST_FILTER_NSF_DESC;
		if (types( File::ROM      )) (*this) << '\t' << Resource::String( IDS_FILES_ROM     ) << NST_FILTER_ROM_DESC;
		if (types( File::SCRIPT   )) (*this) << '\t' << Resource::String( IDS_FILES_SCRIPT  ) << NST_FILTER_SCRIPT_DESC;
		if (types( File::BATTERY  )) (*this) << '\t' << Resource::String( IDS_FILES_BATTERY ) << NST_FILTER_BATTERY_DESC;
		if (types( File::TAPE     )) (*this) << '\t' << Resource::String( IDS_FILES_TAPE    ) << NST_FILTER_TAPE_DESC;

		switch (types( File::STATE|File::SLOTS ))
		{
			case File::STATE:             (*this) << '\t' << Resource::String( IDS_FILES_STATE       ) << NST_FILTER_STATE_DESC;       break;
			case File::SLOTS:             (*this) << '\t' << Resource::String( IDS_FILES_STATE_SLOTS ) << NST_FILTER_SLOTS_DESC;       break;
			case File::STATE|File::SLOTS: (*this) << '\t' << Resource::String( IDS_FILES_STATE       ) << NST_FILTER_STATE_SLOTS_DESC; break;
		}

		if (types( File::MOVIE    )) (*this) << '\t' << Resource::String( IDS_FILES_MOVIE   ) << NST_FILTER_MOVIE_DESC;
		if (types( File::IPS      )) (*this) << '\t' << Resource::String( IDS_FILES_IPS     ) << NST_FILTER_IPS_DESC;
		if (types( File::PALETTE  )) (*this) << '\t' << Resource::String( IDS_FILES_PALETTE ) << NST_FILTER_PALETTE_DESC;
		if (types( File::WAVE     )) (*this) << '\t' << Resource::String( IDS_FILES_WAVE    ) << NST_FILTER_WAVE_DESC;
		if (types( File::AVI      )) (*this) << '\t' << Resource::String( IDS_FILES_AVI     ) << NST_FILTER_AVI_DESC;
		if (types( File::ARCHIVE  )) (*this) << '\t' << Resource::String( IDS_FILES_ARCHIVE ) << NST_FILTER_ARCHIVE_DESC;

		(*this) << '\t' << Resource::String( IDS_FILES_ALL ) << NST_FILTER_ALL_DESC;
	}
}
