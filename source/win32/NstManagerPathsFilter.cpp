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
#include "NstManagerPaths.hpp"
#include "NstManagerPathsFilter.hpp"

namespace Nestopia
{
    #define NST_FILTER_ALL              _T("All supported files")
    #define NST_FILTER_INES_ALL         _T(", *.nes")
    #define NST_FILTER_UNIF_ALL         _T(", *.unf")
    #define NST_FILTER_FDS_ALL          _T(", *.fds")
    #define NST_FILTER_NSF_ALL          _T(", *.nsf")
    #define NST_FILTER_ROM_ALL          _T(", *.rom")
    #define NST_FILTER_SCRIPT_ALL       _T(", *.nsp")
    #define NST_FILTER_BATTERY_ALL      _T(", *.sav")
    #define NST_FILTER_TAPE_ALL         _T(", *.tp")
    #define NST_FILTER_STATE_ALL        _T(", *.nst")
    #define NST_FILTER_MOVIE_ALL        _T(", *.nsv")
    #define NST_FILTER_IPS_ALL          _T(", *.ips")
    #define NST_FILTER_SLOTS_ALL        _T(", *.ns1..9")
    #define NST_FILTER_PALETTE_ALL      _T(", *.pal")
    #define NST_FILTER_WAVE_ALL         _T(", *.wav")
    #define NST_FILTER_AVI_ALL          _T(", *.avi")
    #define NST_FILTER_ARCHIVE_ALL      _T(", *.zip, *.rar, *.7z")
    #define NST_FILTER_INES             _T(";*.nes")
    #define NST_FILTER_UNIF             _T(";*.unf;*.unif")
    #define NST_FILTER_FDS              _T(";*.fds")
    #define NST_FILTER_NSF              _T(";*.nsf")
    #define NST_FILTER_ROM              _T(";*.rom")
    #define NST_FILTER_SCRIPT           _T(";*.nsp")
    #define NST_FILTER_BATTERY          _T(";*.sav")
    #define NST_FILTER_TAPE             _T(";*.tp")
    #define NST_FILTER_STATE            _T(";*.nst")
    #define NST_FILTER_MOVIE            _T(";*.nsv")
    #define NST_FILTER_IPS              _T(";*.ips")
    #define NST_FILTER_SLOTS            _T(";*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9")
    #define NST_FILTER_PALETTE          _T(";*.pal")
    #define NST_FILTER_WAVE             _T(";*.wav")
    #define NST_FILTER_AVI              _T(";*.avi")
    #define NST_FILTER_ARCHIVE          _T(";*.zip;*.rar;*.7z")
    #define NST_FILTER_INES_DESC        _T("\tiNes ROM Images (*.nes)\t*.nes")
    #define NST_FILTER_UNIF_DESC        _T("\tUNIF ROM Images (*.unf,*.unif)\t*.unf;*.unif")
    #define NST_FILTER_FDS_DESC         _T("\tFamicom Disk System Images (*.fds)\t*.fds")
    #define NST_FILTER_NSF_DESC         _T("\tNES Sound Files (*.nsf)\t*.nsf")
    #define NST_FILTER_ROM_DESC         _T("\tROM Images (*.rom)\t*.rom")
    #define NST_FILTER_SCRIPT_DESC      _T("\tNestopia Script Files (*.nsp)\t*.nsp")
    #define NST_FILTER_BATTERY_DESC     _T("\tBattery RAM files (*.sav)\t*.sav")
    #define NST_FILTER_TAPE_DESC        _T("\tTape files (*.tp)\t*.tp")
    #define NST_FILTER_STATE_SLOTS_DESC _T("\tNestopia State Files (*.nst,*.ns1..9)\t*.nst;*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9")
    #define NST_FILTER_STATE_DESC       _T("\tNestopia State Files (*.nst)\t*.nst")
    #define NST_FILTER_SLOTS_DESC       _T("\tNestopia State Slot Files (*.ns1..9)\t*.ns1;*.ns2;*.ns3;*.ns4;*.ns5;*.ns6;*.ns7;*.ns8;*.ns9")
    #define NST_FILTER_MOVIE_DESC       _T("\tNestopia Movie Files (*.nsv)\t*.nsv")
    #define NST_FILTER_IPS_DESC         _T("\tIPS files (*.ips)\t*.ips")
    #define NST_FILTER_PALETTE_DESC     _T("\tNES Palette Files (*.pal)\t*.pal")
    #define NST_FILTER_WAVE_DESC        _T( "\tWAVE Files (*.wav)\t*.wav")
    #define NST_FILTER_AVI_DESC         _T( "\tAVI Files (*.avi)\t*.avi")
    #define NST_FILTER_ARCHIVE_DESC     _T("\tArchives (*.zip,*.rar,*.7z)\t*.zip;*.rar;*.7z")
    #define NST_FILTER_ALL_DESC         _T("\tAll files (*.*)\t*.*\t")

	Managers::Paths::Filter::Filter(const File::Types types)
	{
		NST_ASSERT( types.Word() );

		(*this) << NST_FILTER_ALL;

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

		if (types( File::INES     )) (*this) << NST_FILTER_INES_DESC;  
		if (types( File::UNIF     )) (*this) << NST_FILTER_UNIF_DESC;  
		if (types( File::FDS      )) (*this) << NST_FILTER_FDS_DESC;   
		if (types( File::NSF      )) (*this) << NST_FILTER_NSF_DESC;   
		if (types( File::ROM      )) (*this) << NST_FILTER_ROM_DESC;   
		if (types( File::SCRIPT   )) (*this) << NST_FILTER_SCRIPT_DESC;
		if (types( File::BATTERY  )) (*this) << NST_FILTER_BATTERY_DESC;
		if (types( File::TAPE     )) (*this) << NST_FILTER_TAPE_DESC;

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
		if (types( File::AVI      )) (*this) << NST_FILTER_AVI_DESC;
		if (types( File::ARCHIVE  )) (*this) << NST_FILTER_ARCHIVE_DESC; 

		(*this) << NST_FILTER_ALL_DESC;
	}
}
