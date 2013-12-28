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

#include "NstMappers.h"
#include "NstMapper090.h"
	 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER90::MAPPER90(CONTEXT& c)
: 
MAPPER (c,&status,&CiRomBanks+4),
CiRam  (n2k),
CiRom  (NULL),
ExRom  (NULL)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER90::~MAPPER90() 
{
	delete CiRom;
	delete ExRom;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER90::Reset()
{
	mk3 = (pRomCrc == 0x2A268152UL); // Mortal Kombat 3 - Special 56 Peoples

	delete CiRom; CiRom = new CIROM( cRom.Ram(), cRom.Size() );
	delete ExRom; ExRom = new EXROM( pRom.Ram(), pRom.Size() );

	cpu.SetPort( 0x5000, this, Peek_5000, Poke_Nop  );
	cpu.SetPort( 0x5800, this, Peek_5800, Poke_5800 );
	cpu.SetPort( 0x5801, this, Peek_5801, Poke_5801 );
	cpu.SetPort( 0x5803, this, Peek_5803, Poke_5803 );

	for (ULONG i=0x0000; i < 0x0FFF; i += 0x8)
	{
		cpu.SetPort( 0x8000 + i, this, Peek_8000, Poke_8000 );
		cpu.SetPort( 0x8001 + i, this, Peek_8000, Poke_8001 );
		cpu.SetPort( 0x8002 + i, this, Peek_8000, Poke_8002 );
		cpu.SetPort( 0x8003 + i, this, Peek_8000, Poke_8003 );
		cpu.SetPort( 0x8004 + i, this, Peek_8000, Poke_8000 );
		cpu.SetPort( 0x8005 + i, this, Peek_8000, Poke_8001 );
		cpu.SetPort( 0x8006 + i, this, Peek_8000, Poke_8002 );
		cpu.SetPort( 0x8007 + i, this, Peek_8000, Poke_8003 );
		cpu.SetPort( 0x9000 + i, this, Peek_9000, Poke_9000 );
		cpu.SetPort( 0x9001 + i, this, Peek_9000, Poke_9001 );
		cpu.SetPort( 0x9002 + i, this, Peek_9000, Poke_9002 );
		cpu.SetPort( 0x9003 + i, this, Peek_9000, Poke_9003 );
		cpu.SetPort( 0x9004 + i, this, Peek_9000, Poke_9004 );
		cpu.SetPort( 0x9005 + i, this, Peek_9000, Poke_9005 );
		cpu.SetPort( 0x9006 + i, this, Peek_9000, Poke_9006 );
		cpu.SetPort( 0x9007 + i, this, Peek_9000, Poke_9007 );
		cpu.SetPort( 0xA000 + i, this, Peek_A000, Poke_A000 );
		cpu.SetPort( 0xA001 + i, this, Peek_A000, Poke_A001 );
		cpu.SetPort( 0xA002 + i, this, Peek_A000, Poke_A002 );
		cpu.SetPort( 0xA003 + i, this, Peek_A000, Poke_A003 );
		cpu.SetPort( 0xA004 + i, this, Peek_A000, Poke_A004 );
		cpu.SetPort( 0xA005 + i, this, Peek_A000, Poke_A005 );
		cpu.SetPort( 0xA006 + i, this, Peek_A000, Poke_A006 );
		cpu.SetPort( 0xA007 + i, this, Peek_A000, Poke_A007 );
		cpu.SetPort( 0xB000 + i, this, Peek_B000, Poke_B000 );
		cpu.SetPort( 0xB001 + i, this, Peek_B000, Poke_B001 );
		cpu.SetPort( 0xB002 + i, this, Peek_B000, Poke_B002 );
		cpu.SetPort( 0xB003 + i, this, Peek_B000, Poke_B003 );
		cpu.SetPort( 0xB004 + i, this, Peek_B000, Poke_B004 );
		cpu.SetPort( 0xB005 + i, this, Peek_B000, Poke_B005 );
		cpu.SetPort( 0xB006 + i, this, Peek_B000, Poke_B006 );
		cpu.SetPort( 0xB007 + i, this, Peek_B000, Poke_B007 );
		cpu.SetPort( 0xC002 + i, this, Peek_C000, Poke_C002 );
		cpu.SetPort( 0xC003 + i, this, Peek_C000, Poke_C003 );
		cpu.SetPort( 0xC004 + i, this, Peek_C000, Poke_C003 );
		cpu.SetPort( 0xC005 + i, this, Peek_C000, Poke_C005 );

		if (mk3)
			cpu.SetPort( 0xC006 + i, this, Peek_C000, Poke_C006 );

		cpu.SetPort( 0xD000 + i, this, Peek_D000, Poke_D000 );
		cpu.SetPort( 0xD001 + i, this, Peek_D000, Poke_D001 );
	}

	ppu.SetPort( 0x0000, 0x1FFF, this, Peek_CRom , Poke_Nop   );
	ppu.SetPort( 0x2000, 0x2FFF, this, Peek_CiRam, Poke_CiRam );

	cpu.SetEvent( &ppu, PPU::Update );

	pRom.SwapBanks<n32k,0x0000>(pRom.NumBanks<n32k>() - 1);

	pRomBanks[0] = pRom.NumBanks<n8k>() - 4;
	pRomBanks[1] = pRom.NumBanks<n8k>() - 3;
	pRomBanks[2] = pRom.NumBanks<n8k>() - 2;
	pRomBanks[3] = pRom.NumBanks<n8k>() - 1;

	cRomBanks[0].d = 0;
	cRomBanks[1].d = 1;
	cRomBanks[2].d = 2;
	cRomBanks[3].d = 3;
	cRomBanks[4].d = 4;
	cRomBanks[5].d = 5;
	cRomBanks[6].d = 6;
	cRomBanks[7].d = 7;

	CiRomBanks[0].d = 0;
	CiRomBanks[1].d = 0;
	CiRomBanks[2].d = 0;
	CiRomBanks[3].d = 0;

	status       = 0;
	mirror       = 0;
	AddressLatch = 0;
	mul[0]       = 0;
	mul[1]       = 0;
	latch        = 0;
	IrqOffset    = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER90::LoadState(PDXFILE& file)
{
	PDX_TRY(MAPPER::LoadState(file));
	PDX_TRY(CiRam.LoadState(file));
	PDX_TRY(CiRom->LoadState(file));
	return ExRom->LoadState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER90::SaveState(PDXFILE& file) const
{
	PDX_TRY(MAPPER::SaveState(file));
	PDX_TRY(CiRam.SaveState(file));
	PDX_TRY(CiRom->SaveState(file));
	return ExRom->SaveState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER90,5800) { mul[0] = data; }
NES_POKE(MAPPER90,5801) { mul[1] = data; }
NES_POKE(MAPPER90,5803) { latch = data;  }

NES_PEEK(MAPPER90,5000) { return 0x80; }
NES_PEEK(MAPPER90,5800) { return ((mul[0] * mul[1]) >> 0) & 0xFF; }
NES_PEEK(MAPPER90,5801) { return ((mul[0] * mul[1]) >> 8) & 0xFF; }
NES_PEEK(MAPPER90,5803) { return latch; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER90,8000) { pRomBanks[0] = data; UpdatePRom(); }
NES_POKE(MAPPER90,8001) { pRomBanks[1] = data; UpdatePRom(); }
NES_POKE(MAPPER90,8002) { pRomBanks[2] = data; UpdatePRom(); }
NES_POKE(MAPPER90,8003) { pRomBanks[3] = data; UpdatePRom(); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER90,9000) { cRomBanks[0].b.l = data; UpdateCRom(); }
NES_POKE(MAPPER90,9001) { cRomBanks[1].b.l = data; UpdateCRom(); }
NES_POKE(MAPPER90,9002) { cRomBanks[2].b.l = data; UpdateCRom(); }
NES_POKE(MAPPER90,9003) { cRomBanks[3].b.l = data; UpdateCRom(); }
NES_POKE(MAPPER90,9004) { cRomBanks[4].b.l = data; UpdateCRom(); }
NES_POKE(MAPPER90,9005) { cRomBanks[5].b.l = data; UpdateCRom(); }
NES_POKE(MAPPER90,9006) { cRomBanks[6].b.l = data; UpdateCRom(); }
NES_POKE(MAPPER90,9007) { cRomBanks[7].b.l = data; UpdateCRom(); }
NES_POKE(MAPPER90,A000) { cRomBanks[0].b.h = data; UpdateCRom(); }
NES_POKE(MAPPER90,A001) { cRomBanks[1].b.h = data; UpdateCRom(); }
NES_POKE(MAPPER90,A002) { cRomBanks[2].b.h = data; UpdateCRom(); }
NES_POKE(MAPPER90,A003) { cRomBanks[3].b.h = data; UpdateCRom(); }
NES_POKE(MAPPER90,A004) { cRomBanks[4].b.h = data; UpdateCRom(); }
NES_POKE(MAPPER90,A005) { cRomBanks[5].b.h = data; UpdateCRom(); }
NES_POKE(MAPPER90,A006) { cRomBanks[6].b.h = data; UpdateCRom(); }
NES_POKE(MAPPER90,A007) { cRomBanks[7].b.h = data; UpdateCRom(); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER90,B000) { CiRomBanks[0].b.l = data; UpdateCiRom(); }
NES_POKE(MAPPER90,B001) { CiRomBanks[1].b.l = data; UpdateCiRom(); }
NES_POKE(MAPPER90,B002) { CiRomBanks[2].b.l = data; UpdateCiRom(); }
NES_POKE(MAPPER90,B003) { CiRomBanks[3].b.l = data; UpdateCiRom(); }
NES_POKE(MAPPER90,B004) { CiRomBanks[0].b.h = data; UpdateCiRom(); }
NES_POKE(MAPPER90,B005) { CiRomBanks[1].b.h = data; UpdateCiRom(); }
NES_POKE(MAPPER90,B006) { CiRomBanks[2].b.h = data; UpdateCiRom(); }
NES_POKE(MAPPER90,B007) { CiRomBanks[3].b.h = data; UpdateCiRom(); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER90,C002) 
{
	SetIrqEnable(FALSE);
}

NES_POKE(MAPPER90,C003) 
{
	if (!IsIrqEnabled())
		IrqCount = IrqLatch;

	SetIrqEnable(TRUE);
}

NES_POKE(MAPPER90,C005) 
{
	if (mk3)
	{
		if (IrqOffset & 0x80) IrqCount = IrqLatch = data ^ (IrqOffset | 0x01);
		else                  IrqCount = IrqLatch = data | (IrqOffset & 0x27);
	}
	else
	{
		IrqCount = IrqLatch = data;
	}
}

NES_POKE(MAPPER90,C006) 
{
	IrqOffset = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER90,D000) 
{
	status = data;
	UpdatePRom();
	UpdateCRom();
	UpdateCiRom();
}

NES_POKE(MAPPER90,D001) 
{
	mirror = data & SELECT_MIRRORING;
	UpdateCiRom();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER90::UpdateCRom()
{
	ppu.Update();

	switch (status & CROM_BANK_MODE)
	{
     	case CROM_BANK_MODE_0:

			cRom.SwapBanks<n8k,0x0000>( cRomBanks[0].d );
			return;

		case CROM_BANK_MODE_1:

			cRom.SwapBanks<n4k,0x0000>( cRomBanks[0].d );
			cRom.SwapBanks<n4k,0x1000>( cRomBanks[4].d );
			return;

		case CROM_BANK_MODE_2:

			cRom.SwapBanks<n2k,0x0000>( cRomBanks[0].d );
			cRom.SwapBanks<n2k,0x0800>( cRomBanks[2].d );
			cRom.SwapBanks<n2k,0x1000>( cRomBanks[4].d );
			cRom.SwapBanks<n2k,0x1800>( cRomBanks[6].d );
			return;

		case CROM_BANK_MODE_3:

			cRom.SwapBanks<n1k,0x0000>( cRomBanks[0].d );
			cRom.SwapBanks<n1k,0x0400>( cRomBanks[1].d );
			cRom.SwapBanks<n1k,0x0800>( cRomBanks[2].d );
			cRom.SwapBanks<n1k,0x0C00>( cRomBanks[3].d );
			cRom.SwapBanks<n1k,0x1000>( cRomBanks[4].d );
			cRom.SwapBanks<n1k,0x1400>( cRomBanks[5].d );
			cRom.SwapBanks<n1k,0x1800>( cRomBanks[6].d );
			cRom.SwapBanks<n1k,0x1C00>( cRomBanks[7].d );
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER90::UpdatePRom()
{
	apu.Update();

	switch (status & PROM_BANK_MODE)
	{
     	case PROM_BANK_MODE_0:

			pRom.SwapBanks<n32k,0x0000>( pRom.NumBanks<n32k>() - 1 );
			return;

		case PROM_BANK_MODE_1:

			pRom.SwapBanks<n16k,0x0000>( pRomBanks[1] );
			pRom.SwapBanks<n16k,0x4000>( pRom.NumBanks<n16k>() - 1 );
			return;

		case PROM_BANK_MODE_2:

			pRom.SwapBanks<n8k,0x0000>( pRomBanks[0] );
			pRom.SwapBanks<n8k,0x2000>( pRomBanks[1] );
			pRom.SwapBanks<n8k,0x4000>( pRomBanks[2] );

			if (status & PROM_BANK_LAST)
			{
				pRom.SwapBanks<n8k,0x6000>( pRomBanks[3] );
			}
			else
			{
				if (status & EXROM_BANK_SWAP)
				{
					ExRom->SwapBanks<n8k,0x0000>( pRomBanks[3] );
					cpu.SetPort( 0x6000, 0x7FFF, this, Peek_ExRom, Poke_Nop );
				}

				pRom.SwapBanks<n8k,0x6000>( pRom.NumBanks<n8k>() - 1 );
			}
			return;

		case PROM_BANK_MODE_3:

			pRom.SwapBanks<n8k,0x0000>( pRomBanks[3] );
			pRom.SwapBanks<n8k,0x2000>( pRomBanks[2] );
			pRom.SwapBanks<n8k,0x4000>( pRomBanks[1] );
			pRom.SwapBanks<n8k,0x6000>( pRomBanks[0] );
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER90::UpdateCiRom() 
{
	ppu.Update();

	if ((status & SELECT_CIROM) && !mk3)
	{
		if (CiRomBanks[0].d == 0 || CiRomBanks[1].d == 1 || CiRomBanks[2].d == 2 || CiRomBanks[3].d == 3)
		{
			status &= ~SELECT_CIROM;
		}
		else
		{
			CiRom->SwapBanks<n1k,0x0000>( CiRomBanks[0].d );
			CiRom->SwapBanks<n1k,0x0400>( CiRomBanks[1].d );
			CiRom->SwapBanks<n1k,0x0800>( CiRomBanks[2].d );
			CiRom->SwapBanks<n1k,0x0C00>( CiRomBanks[3].d );

			ppu.SetPort( 0x2000, 0x2FFF, this, Peek_CiRom, Poke_Nop );
		}
	}
	else
	{
    	static const UCHAR select[4][4] =
       	{
     		{0,1,0,1},
     		{0,0,1,1},
     		{0,0,0,0},
     		{1,1,1,1}
     	};

		CiRam.SwapBanks<n1k,0x0000>( select[mirror][0] );
		CiRam.SwapBanks<n1k,0x0400>( select[mirror][1] );
		CiRam.SwapBanks<n1k,0x0800>( select[mirror][2] );
		CiRam.SwapBanks<n1k,0x0C00>( select[mirror][3] );

		ppu.SetPort( 0x2000, 0x2FFF, this, Peek_CiRam, Poke_CiRam );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER90::IrqSync(const UINT address)
{
	const UINT tmp = address & 0x1000;

	if (tmp && !AddressLatch && IrqCount && !--IrqCount)
	{
		if (IsIrqEnabled())
		{
			SetIrqEnable(FALSE);
			cpu.DoIRQ();
		}
	}

	AddressLatch = tmp;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER90,CRom)
{
	IrqSync(address);
	return cRom[address];
}

NES_PEEK(MAPPER90,CiRam) 
{ 
	return CiRam[address - 0x2000];    
}

NES_POKE(MAPPER90,CiRam) 
{ 
	CiRam[address - 0x2000] = data; 
}

NES_PEEK(MAPPER90,CiRom) 
{ 
	return (*CiRom)[address - 0x2000]; 
}

NES_PEEK(MAPPER90,ExRom) 
{ 
	return (*ExRom)[address - 0x6000]; 
}

NES_NAMESPACE_END
