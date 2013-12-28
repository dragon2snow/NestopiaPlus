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

inline NES_PEEK(PPU,cRam)
{
	PDX_ASSERT(address < 0x2000);
	return cRam[address];
}

inline NES_POKE(PPU,cRam)
{
	PDX_ASSERT(address < 0x2000);
	cRam[address] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline NES_PEEK(PPU,CiRam)
{
	PDX_ASSERT(address < 0x2000);
	return CiRam[address];
}

inline NES_POKE(PPU,CiRam)
{
	PDX_ASSERT(address < 0x2000);
	CiRam[address] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline NES_PEEK(PPU,PalRam)
{
	PDX_ASSERT(address < 0x20);
	return PalRam[address];
}

inline NES_POKE(PPU,PalRam)
{
	PDX_ASSERT(address < 0x20);
	PalRam[address] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
// setup the name table mirroring
////////////////////////////////////////////////////////////////////////////////////////

inline VOID PPU::SetMirroring(const UINT b1,const UINT b2,const UINT b3,const UINT b4)
{
	PDX_ASSERT(b1 < 4 && b2 < 4 && b3 < 4 && b4 < 4);

	Update();

	CiRam.SwapBanks<n1k,0x0000>(b1);
	CiRam.SwapBanks<n1k,0x0400>(b2);
	CiRam.SwapBanks<n1k,0x0800>(b3);
	CiRam.SwapBanks<n1k,0x0C00>(b4);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class OBJECT,class READER,class WRITER>
VOID PPU::SetPort(const UINT address,OBJECT* object,READER reader,WRITER writer)
{
	vRam.SetPort( address, object, reader, writer );
}

template<class OBJECT,class READER,class WRITER>
VOID PPU::SetPort(const UINT first,const UINT last,OBJECT* object,READER reader,WRITER writer)
{
	vRam.SetPort( first, last, object, reader, writer );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline PPU::PORT& PPU::GetPort(const UINT address)
{
	return vRam.GetPort(address);
}

inline const PPU::PORT& PPU::GetPort(const UINT address) const
{
	return vRam.GetPort(address);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline INT PPU::GetScanLine() const
{
	return ScanLine;
}

inline UINT PPU::GetVRamAddress() const
{
	return vRamAddress;
}
