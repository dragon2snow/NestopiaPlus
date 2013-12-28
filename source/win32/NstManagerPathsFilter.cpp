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

#include "NstString.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerPathsFilter.hpp"

namespace Nestopia
{
    #define NST_FILTER_ALL              "All supported files"
    #define NST_FILTER_INES_ALL         ", *.nes"
    #define NST_FILTER_UNIF_ALL         ", *.unf"
    #define NST_FILTER_FDS_ALL          ", *.fds"
    #define NST_FILTER_NSF_ALL          ", *.nsf"
    #define NST_FILTER_ROM_ALL          ", *.rom"
    #define NST_FILTER_SCRIPT_ALL       ", *.nsp"
    #define NST_FILTER_BATTERY_ALL      ", *.sav"
    #define NST_FILTER_STATE_ALL        ", *.nst"
    #define NST_FILTER_MOVIE_ALL        ", *.nsv"
    #define NST_FILTER_IPS_ALL          ", *.ips"
    #define NST_FILTER_SLOTS_ALL        ", *.ns1..9"
    #define NST_FILTER_PALETTE_ALL      ", *.pal"
    #define NST_FILTER_WAVE_ALL         ", *.wav"
    #define NST_FILTER_ARCHIVE_ALL      ", *.zip, *.rar, *.7z"
    #define NST_FILTER_INES             ";*.nes"
    #define NST_FILTER_UNIF             ";*.unf;*.unif"
    #define NST_FILTER_FDS              ";*.fds"
    #define NST_FILTER_NSF              ";*.nsf"
    #define NST_FILTER_ROM              ";*.rom"
    #define NST_FILTER_SCRIPT           ";*.nsp"
    #define NST_FILTER_BATTERY          ";*.sav"
    #define NST_FILTER_STATE            ";*.nst"
    #define NST_FILTER_MOVIE            ";*.nsv"
    #define NST_FILTER_IPS              ";*.ips"
    #define NST_FILTER_SLOTS            ";*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9"
    #define NST_FILTER_PALETTE          ";*.pal"
    #define NST_FILTER_WAVE             ";*.wav"
    #define NST_FILTER_ARCHIVE          ";*.zip;*.rar;*.7z"
    #define NST_FILTER_INES_DESC        "\0iNes ROM Images (*.nes)\0*.nes"
    #define NST_FILTER_UNIF_DESC        "\0UNIF ROM Images (*.unf,*.unif)\0*.unf;*.unif"
    #define NST_FILTER_FDS_DESC         "\0Famicom Disk System Images (*.fds)\0*.fds"
    #define NST_FILTER_NSF_DESC         "\0NES Sound Files (*.nsf)\0*.nsf"
    #define NST_FILTER_ROM_DESC         "\0ROM Images (*.rom)\0*.rom"
    #define NST_FILTER_SCRIPT_DESC      "\0Nestopia Script Files (*.nsp)\0*.nsp"
    #define NST_FILTER_BATTERY_DESC     "\0Battery RAM files (*.sav)\0*.sav"
    #define NST_FILTER_STATE_SLOTS_DESC "\0Nestopia State Files (*.nst,*.ns1..9)\0*.nst;*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9"
    #define NST_FILTER_STATE_DESC       "\0Nestopia State Files (*.nst)\0*.nst"
    #define NST_FILTER_SLOTS_DESC       "\0Nestopia State Slot Files (*.ns1..9)\0*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9"
    #define NST_FILTER_MOVIE_DESC       "\0Nestopia Movie Files (*.nsv)\0*.nsv"
    #define NST_FILTER_IPS_DESC         "\0IPS files (*.ips)\0*.ips"
    #define NST_FILTER_PALETTE_DESC     "\0NES Palette Files (*.pal)\0*.pal"
    #define NST_FILTER_WAVE_DESC        "\0WAVE Files (*.wav)\0*.wav"
    #define NST_FILTER_ARCHIVE_DESC     "\0Archives (*.zip,*.rar,*.7z)\0*.zip;*.rar;*.7z"
    #define NST_FILTER_ALL_DESC         "\0All files (*.*)\0*.*\0"

	Managers::Paths::Filter::Filter(const File::Types types)
	{
		NST_ASSERT( types.Word() );

		(*this) << NST_FILTER_ALL;

		uint offset = Size();

		if (types( File::INES    )) (*this) << NST_FILTER_INES_ALL;
		if (types( File::UNIF    )) (*this) << NST_FILTER_UNIF_ALL;
		if (types( File::FDS     )) (*this) << NST_FILTER_FDS_ALL;
		if (types( File::NSF     )) (*this) << NST_FILTER_NSF_ALL;
		if (types( File::ROM     )) (*this) << NST_FILTER_ROM_ALL;
		if (types( File::SCRIPT  )) (*this) << NST_FILTER_SCRIPT_ALL;
		if (types( File::BATTERY )) (*this) << NST_FILTER_BATTERY_ALL;
		if (types( File::STATE   )) (*this) << NST_FILTER_STATE_ALL;
		if (types( File::SLOTS   )) (*this) << NST_FILTER_SLOTS_ALL;
		if (types( File::MOVIE   )) (*this) << NST_FILTER_MOVIE_ALL;
		if (types( File::IPS     )) (*this) << NST_FILTER_IPS_ALL;
		if (types( File::PALETTE )) (*this) << NST_FILTER_PALETTE_ALL;
		if (types( File::WAVE    )) (*this) << NST_FILTER_WAVE_ALL;
		if (types( File::ARCHIVE )) (*this) << NST_FILTER_ARCHIVE_ALL;

		if (Size() <= MAX_FILE_DIALOG_WIDTH)
		{
			(*this)[offset + 0] = ' ';
			(*this)[offset + 1] = '(';
			(*this) << ')';

			offset = Size();
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
		if (types( File::STATE   )) (*this) << NST_FILTER_STATE;  
		if (types( File::MOVIE   )) (*this) << NST_FILTER_MOVIE;  
		if (types( File::IPS     )) (*this) << NST_FILTER_IPS;    
		if (types( File::SLOTS   )) (*this) << NST_FILTER_SLOTS;  
		if (types( File::PALETTE )) (*this) << NST_FILTER_PALETTE;
		if (types( File::WAVE    )) (*this) << NST_FILTER_WAVE;   
		if (types( File::ARCHIVE )) (*this) << NST_FILTER_ARCHIVE;

		(*this)[offset] = '\0';

		if (types( File::INES     )) (*this) << NST_FILTER_INES_DESC;  
		if (types( File::UNIF     )) (*this) << NST_FILTER_UNIF_DESC;  
		if (types( File::FDS      )) (*this) << NST_FILTER_FDS_DESC;   
		if (types( File::NSF      )) (*this) << NST_FILTER_NSF_DESC;   
		if (types( File::ROM      )) (*this) << NST_FILTER_ROM_DESC;   
		if (types( File::SCRIPT   )) (*this) << NST_FILTER_SCRIPT_DESC;
		if (types( File::BATTERY  )) (*this) << NST_FILTER_BATTERY_DESC;

		switch (types( File::STATE|File::SLOTS ))
		{
			case File::STATE:	          (*this) << NST_FILTER_STATE_DESC;       break;
			case File::SLOTS:             (*this) << NST_FILTER_SLOTS_DESC;       break;
			case File::STATE|File::SLOTS: (*this) << NST_FILTER_STATE_SLOTS_DESC; break;
		}

		if (types( File::MOVIE    )) (*this) << NST_FILTER_MOVIE_DESC;   
		if (types( File::IPS      )) (*this) << NST_FILTER_IPS_DESC;     
		if (types( File::PALETTE  )) (*this) << NST_FILTER_PALETTE_DESC; 
		if (types( File::WAVE     )) (*this) << NST_FILTER_WAVE_DESC;    
		if (types( File::ARCHIVE  )) (*this) << NST_FILTER_ARCHIVE_DESC; 

		(*this) << NST_FILTER_ALL_DESC;
	}
}
