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

#include "../NstTypes.h"
#include "../NstMap.h"
#include "../NstCpu.h"
#include "../NstPpu.h"
#include "../vssystem/NstVsSystem.h"
#include "../vssystem/NstVsRbiBaseball.h"
#include "../vssystem/NstVsTkoBoxing.h"
#include "../vssystem/NstVsSuperXevious.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VSSYSTEM::DIPSWITCH::DIPSWITCH()
: 
index (0),
mask  (0)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VSSYSTEM::DIPSWITCH::DIPSWITCH(const DIPSWITCH& dip)
: 
index    (dip.index),
mask     (dip.mask),
name     (dip.name),
settings (dip.settings)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VSSYSTEM::DIPSWITCH::DIPSWITCH
(
	const CHAR* const n,   
	const UINT m, 
	const UINT i,
	const CHAR* const c01, const UINT v01,
	const CHAR* const c02, const UINT v02,
	const CHAR* const c03, const UINT v03,
	const CHAR* const c04, const UINT v04,
	const CHAR* const c05, const UINT v05,
	const CHAR* const c06, const UINT v06,
	const CHAR* const c07, const UINT v07,
	const CHAR* const c08, const UINT v08,
	const CHAR* const c09, const UINT v09,
	const CHAR* const c10, const UINT v10,
	const CHAR* const c11, const UINT v11,
	const CHAR* const c12, const UINT v12,
	const CHAR* const c13, const UINT v13,
	const CHAR* const c14, const UINT v14,
	const CHAR* const c15, const UINT v15
)
: 
name  (n),
mask  (m),
index (i)
{
	PDX_ASSERT(n && m && c01 && c02);

	settings.Reserve(8);

	settings.InsertBack( PDX::MakePair( PDXSTRING( c01 ), v01 ) );
	settings.InsertBack( PDX::MakePair( PDXSTRING( c02 ), v02 ) );

	if (!c03) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c03 ), v03 ) );
	if (!c04) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c04 ), v04 ) );
	if (!c05) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c05 ), v05 ) );
	if (!c06) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c06 ), v06 ) );
	if (!c07) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c07 ), v07 ) );
	if (!c08) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c08 ), v08 ) );
	if (!c09) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c09 ), v09 ) );
	if (!c10) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c10 ), v10 ) );
	if (!c11) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c11 ), v11 ) );
	if (!c12) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c12 ), v12 ) );
	if (!c13) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c13 ), v13 ) );
	if (!c14) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c14 ), v14 ) );
	if (!c15) return; settings.InsertBack( PDX::MakePair( PDXSTRING( c15 ), v15 ) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VSSYSTEM::DIPSWITCH& VSSYSTEM::DIPSWITCH::operator = (const DIPSWITCH& DipSwitch)
{
	index    = DipSwitch.index;
	mask     = DipSwitch.mask;
	name     = DipSwitch.name;
	settings = DipSwitch.settings;

	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VSSYSTEM::VSSYSTEM(CONTEXT& context)
: 
cpu               (context.cpu), 
ppu               (context.ppu), 
ColorMap          (context.ColorMapIndex < 4 ? ColorMaps[context.ColorMapIndex] : NULL),
DipSwitches       (context.DipSwitches),
CopyProtectionPPU (context.CopyProtectionPPU),
SecurityPPU       (context.SecurityPPU),
FirstPad          (context.SwapPads ? 1 : 0),
RemapButtons      (context.SwapPads)
{
	for (UINT i=0; i < 8; ++i)
	{
		if (context.ButtonMap[i] != i)
			RemapButtons = TRUE;

		ButtonMap[i] = context.ButtonMap[i];
	}

	flags[0] = 0x00;
	flags[1] = 0x00;

	UpdateDipSwitches( 0, DipSwitches.Size() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID VSSYSTEM::Reset(const BOOL)
{
	LogOutput("VSSYSTEM: reset");

	if (ColorMap)
	{
		LogOutput("VSSYSTEM: scrambled color table present");

		p2007 = cpu->GetPort( 0x2007 );

		for (UINT i=0x2000; i < 0x4000; i += 0x8)
			cpu->SetPort( i + 0x7, this, Peek_2007, Poke_2007 );
	}

	if (CopyProtectionPPU)
	{
		LogOutput("VSSYSTEM: PPU copy protection present");

    	CPU_PORT p2000 = cpu->GetPort( 0x2000 );
    	CPU_PORT p2001 = cpu->GetPort( 0x2001 );

    	p2002 = cpu->GetPort( 0x2002 );

		for (UINT i=0x2000; i < 0x4000; i += 0x8)
		{
			cpu->SetPort( i + 0x0, p2001.object, p2001.reader, p2001.writer );
			cpu->SetPort( i + 0x1, p2000.object, p2000.reader, p2000.writer );
			cpu->SetPort( i + 0x2, this, Peek_2002, Poke_2002 );
		}
	}

	if (RemapButtons)
		LogOutput("VSSYSTEM: swapped input ports 0/1");

	coin = 0;
	flags[0] &= ~COIN;

	p4016 = cpu->GetPort( 0x4016 );
	p4017 = cpu->GetPort( 0x4017 );

	cpu->SetPort( 0x4016, this, Peek_4016, Poke_4016 );
	cpu->SetPort( 0x4017, this, Peek_4017, Poke_4017 );

	Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID VSSYSTEM::GetDipSwitch(const UINT index,IO::DIPSWITCH::CONTEXT& context) const
{
	context.name = DipSwitches[index].name;
	context.settings.Resize( DipSwitches[index].settings.Size() );

	for (UINT i=0; i < DipSwitches[index].settings.Size(); ++i)
		context.settings[i] = DipSwitches[index].settings[i].First();

	context.index = DipSwitches[index].index;
}

VOID VSSYSTEM::SetDipSwitch(const UINT index,const IO::DIPSWITCH::CONTEXT& context)
{
	if (DipSwitches[index].index != context.index)
	{
		DipSwitches[index].index = context.index;
		UpdateDipSwitches( index, index + 1 );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID VSSYSTEM::UpdateDipSwitches(const UINT start,const UINT end)
{
	for (UINT i=start; i < end; ++i)
	{
		const DIPSWITCH& dip = DipSwitches[i];

		// Clear old values

		flags[0] &= ~( ( dip.mask & DIPSWITCH_4016_MASK ) << DIPSWITCH_4016_SHIFT );
		flags[1] &= ~( ( dip.mask & DIPSWITCH_4017_MASK ) << DIPSWITCH_4017_SHIFT );

		// Set new values

		flags[0] |= ( ( dip.settings[dip.index].Second() & DIPSWITCH_4016_MASK ) << DIPSWITCH_4016_SHIFT );
		flags[1] |= ( ( dip.settings[dip.index].Second() & DIPSWITCH_4017_MASK ) << DIPSWITCH_4017_SHIFT );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(VSSYSTEM,2002)
{
	return p2002.Peek( 0x2002 ) | SecurityPPU;
}

NES_POKE(VSSYSTEM,2002)
{
	p2002.Poke( 0x2002, data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(VSSYSTEM,2007)
{
	return p2007.Peek( 0x2007 );
}

NES_POKE(VSSYSTEM,2007)
{
	ppu->Update();

	UINT color = data;

	if (ppu->GetVRamAddress() >= 0x3F00)
	{
		color &= 0x3F;

		if (ColorMap[color] != 0xFF)
			color = ColorMap[color];
	}

	p2007.Poke( 0x2007, color );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(VSSYSTEM,4016)
{
	return (p4016.Peek( 0x4016 ) & ~STATUS_4016_MASK) | flags[0];
}

NES_POKE(VSSYSTEM,4016)
{
	p4016.Poke( 0x4016, data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(VSSYSTEM,4017)
{
	return (p4017.Peek( 0x4017 ) & ~STATUS_4017_MASK) | flags[1];
}

NES_POKE(VSSYSTEM,4017)
{
	p4017.Poke( 0x4017, data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(VSSYSTEM,4020)
{
	return coin;
}

NES_POKE(VSSYSTEM,4020)
{
	coin = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID VSSYSTEM::SetContext(IO::INPUT* const input)
{
	flags[0] &= ~COIN;

	if (input)
	{
		flags[0] |= input->vs.InsertCoin & COIN;

		if (RemapButtons)
		{
			UINT src[2];

			src[0] = input->pad[FirstPad ^ 0].buttons;
			src[1] = input->pad[FirstPad ^ 1].buttons;

			for (UINT i=0; i < 2; ++i)
			{
				UINT& dst = input->pad[i].buttons;

				dst  = ( src[i] & 0x01 ) ? ( 1U << ButtonMap[0] ) : 0x00;
				dst |= ( src[i] & 0x02 ) ? ( 1U << ButtonMap[1] ) : 0x00;
				dst |= ( src[i] & 0x04 ) ? ( 1U << ButtonMap[2] ) : 0x00;
				dst |= ( src[i] & 0x08 ) ? ( 1U << ButtonMap[3] ) : 0x00;
				dst |= ( src[i] & 0x10 ) ? ( 1U << ButtonMap[4] ) : 0x00;
				dst |= ( src[i] & 0x20 ) ? ( 1U << ButtonMap[5] ) : 0x00;
				dst |= ( src[i] & 0x40 ) ? ( 1U << ButtonMap[6] ) : 0x00;
				dst |= ( src[i] & 0x80 ) ? ( 1U << ButtonMap[7] ) : 0x00;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_SWAP_BUTTON(b0,b1)       PDX::Swap(context.ButtonMap[b0],context.ButtonMap[b1]);
#define NES_MAP_COLOR(index)         context.ColorMapIndex = (index);
#define NES_CRACK_PPU(security)      context.CopyProtectionPPU = TRUE; context.SecurityPPU = (security);
#define NES_DIPSKIP()                DipSkip(context);
#define NES_DIPBEGIN(name,mask,dflt) context.DipSwitches.InsertBack(DIPSWITCH(name,mask,dflt
#define NES_DIPVALUE(desc,bits)      ,desc, bits
#define NES_DIPEND()			     ));

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID VSSYSTEM::DipSkip(VSSYSTEM::CONTEXT& context)
{	
	for (UINT i=0x01; i < 0x100; i <<= 1)
	{
		NES_DIPBEGIN( "unknown", i,   0 ) 
		NES_DIPVALUE( "Off",     0x00   ) 
		NES_DIPVALUE( "On",      i      ) 
		NES_DIPEND  (                   )
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VSSYSTEM* VSSYSTEM::New(CPU* const cpu,PPU* const ppu,const ULONG pRomCrc)
{
	switch (pRomCrc)
	{
		// VS. Dual-System Games are unsupported

     	case 0xB90497AAUL: // Tennis
		case 0x008A9C16UL: // Wrecking Crew 
		case 0xAD407F52UL: // Balloon Fight
		case 0x18A93B7BUL: // Mahjong (J)
		case 0x13A91937UL: // Baseball 
		case 0xF5DEBF88UL: // Baseball 
		case 0xF64D7252UL: // Baseball 
		case 0x968A6E9DUL: // Baseball
		case 0xF42DAB14UL: // Ice Climber
		case 0x7D6B764FUL: // Ice Climber

			LogOutput("VSSYSTEM: error, Dual-System games are not supported");			
			MsgWarning("VS. Dual-System games are not supported!");
			return NULL;
	}

	VSSYSTEM* VsSystem;

	VSSYSTEM::CONTEXT context;

	context.cpu = cpu;
	context.ppu = ppu;

	// Credits to the MAME people for the dipswitch discoveries and descriptions.

	switch (pRomCrc)
	{
     	case 0xEB2DBA63UL: // TKO Boxing
		case 0x9818F656UL:

			NES_DIPBEGIN ( "Coinage",            0x03, 0 )
			NES_DIPVALUE ( "1 Coins / 1 Credit", 0x00    )
			NES_DIPVALUE ( "1 Coins / 2 Credit", 0x01    )
			NES_DIPVALUE ( "2 Coins / 1 Credit", 0x02    )
			NES_DIPVALUE ( "3 Coins / 1 Credit", 0x03    )
		    NES_DIPEND	 (                               )
			NES_DIPBEGIN ( "unknown",            0x04, 0 )
			NES_DIPVALUE ( "Off",                0x00    )
			NES_DIPVALUE ( "On",                 0x04    )
			NES_DIPEND   (                               )
			NES_DIPBEGIN ( "unknown",            0x08, 0 )
			NES_DIPVALUE ( "Off",                0x00    )
			NES_DIPVALUE ( "On",                 0x08    )
			NES_DIPEND   (                               )
			NES_DIPBEGIN ( "unknown",            0x10, 0 )
			NES_DIPVALUE ( "Off",                0x00    )
			NES_DIPVALUE ( "On",                 0x10    )
			NES_DIPEND   (                               )
			NES_DIPBEGIN ( "Palette Color",      0x20, 1 )
			NES_DIPVALUE ( "Black",              0x00    )
			NES_DIPVALUE ( "White",              0x20    )
			NES_DIPEND   (                               )
			NES_DIPBEGIN ( "unknown",            0x40, 0 )
			NES_DIPVALUE ( "Off",                0x00    )
			NES_DIPVALUE ( "On",                 0x40    )
			NES_DIPEND   (                               )
			NES_DIPBEGIN ( "unknown",            0x80, 0 )
			NES_DIPVALUE ( "Off",                0x00    )
			NES_DIPVALUE ( "On",                 0x80    )
			NES_DIPEND   (                               )

			VsSystem = new VSTKOBOXING(context);
			break; 

     	case 0x135ADF7CUL: // RBI Baseball

			NES_DIPBEGIN    ( "Coinage",                0x03, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credit",     0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credit",     0x01    )
			NES_DIPVALUE    ( "2 Coins / 1 Credit",     0x02    )
			NES_DIPVALUE    ( "3 Coins / 1 Credit",     0x03    )
		    NES_DIPEND	    (                                   )
			NES_DIPBEGIN    ( "Max. 1p/in, 2p/in, Min", 0x0C, 1 )
			NES_DIPVALUE    ( "2, 1, 3",                0x04    )
			NES_DIPVALUE    ( "2, 2, 4",                0x0C    )
			NES_DIPVALUE    ( "3, 2, 6",                0x00    )
			NES_DIPVALUE    ( "4, 3, 7",                0x08    )
			NES_DIPEND      (                                   )
			NES_DIPBEGIN    ( "Demo Sounds",            0x10, 1 )
			NES_DIPVALUE    ( "Off",                    0x00    )
			NES_DIPVALUE    ( "On",                     0x10    )
			NES_DIPEND      (                                   )
			NES_DIPBEGIN    ( "Color Palette",          0xE0, 0 )
			NES_DIPVALUE    ( "Normal",                 0x80    )
			NES_DIPVALUE    ( "Wrong 1",                0x00    )
			NES_DIPVALUE    ( "Wrong 2",                0x40    )
			NES_DIPVALUE    ( "Wrong 3",                0x20    )
			NES_DIPVALUE    ( "Wrong 4",                0xC0    )
		    NES_DIPEND	    (                                   )
			NES_SWAP_BUTTON ( START, SELECT                     )
			NES_MAP_COLOR   ( 2                                 )

			VsSystem = new VSRBIBASEBALL(context);
			break; 
			
		case 0xED588F00: // Duck Hunt

			NES_DIPBEGIN ( "Coinage",             0x07, 0 )
			NES_DIPVALUE ( "1 Coins / 1 Credit",  0x00    )
			NES_DIPVALUE ( "1 Coins / 2 Credit",  0x04    )
			NES_DIPVALUE ( "1 Coins / 3 Credit",  0x02    )
			NES_DIPVALUE ( "2 Coins / 1 Credit",  0x06    )
			NES_DIPVALUE ( "3 Coins / 1 Credit",  0x01    )
			NES_DIPVALUE ( "4 Coins / 1 Credit",  0x05    )
			NES_DIPVALUE ( "5 Coins / 1 Credit",  0x03    )
			NES_DIPVALUE ( "Free Play",	       0x07    )
		    NES_DIPEND	 (                                )
			NES_DIPBEGIN ( "Difficulty",          0x08, 1 )
			NES_DIPVALUE ( "Easy",                0x00    )
			NES_DIPVALUE ( "Normal",              0x08    )
			NES_DIPVALUE ( "Hard",                0x10    )
			NES_DIPVALUE ( "Very Hard",           0x18    )
			NES_DIPEND	 (                                )
			NES_DIPBEGIN ( "Misses per Game",     0x20, 1 )
			NES_DIPVALUE ( "3",                   0x00    )
			NES_DIPVALUE ( "5",                   0x20    )
			NES_DIPEND	 (                                )
     		NES_DIPBEGIN ( "Bonus Life",          0xC0, 0 )
			NES_DIPVALUE ( "30000",               0x00    )
			NES_DIPVALUE ( "50000",	              0x40    )
			NES_DIPVALUE ( "80000",	              0x80    )
			NES_DIPVALUE ( "100000",	          0xC0    )
		    NES_DIPEND	 (                                )
			
			goto hell;

		case 0x16D3F469UL: // Ninja Jajamaru Kun (J)

			NES_DIPBEGIN    ( "Coinage",             0x07, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credit",  0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credit",  0x04    )
			NES_DIPVALUE    ( "1 Coins / 3 Credit",  0x02    )
			NES_DIPVALUE    ( "1 Coins / 4 Credit",  0x06    )
			NES_DIPVALUE    ( "2 Coins / 1 Credit",  0x01    )
			NES_DIPVALUE    ( "3 Coins / 1 Credit",  0x05    )
			NES_DIPVALUE    ( "4 Coins / 1 Credit",  0x03    )
			NES_DIPVALUE    ( "Free Play",	         0x07    )
		    NES_DIPEND	    (                                )
			NES_DIPBEGIN    ( "Lives",               0x18, 0 )
			NES_DIPVALUE    ( "3",                   0x00    )
			NES_DIPVALUE    ( "4",                   0x10    )
			NES_DIPVALUE    ( "5",                   0x08    )
			NES_DIPEND	    (                                )
			NES_DIPBEGIN    ( "unknown",             0x20, 0 )
			NES_DIPVALUE    ( "Off",                 0x00    )
			NES_DIPVALUE    ( "On",                  0x20    )
			NES_DIPEND      (                                )
			NES_DIPBEGIN    ( "unknown",             0x40, 0 )
			NES_DIPVALUE    ( "Off",                 0x00    )
			NES_DIPVALUE    ( "On",                  0x40    )
			NES_DIPEND      (                                )
			NES_DIPBEGIN    ( "Demo Sounds",         0x80, 1 )
			NES_DIPVALUE    ( "Off",                 0x00    )
			NES_DIPVALUE    ( "On",                  0x80    )
			NES_DIPEND      (                                )
			NES_CRACK_PPU   ( 0x1B /* works better than 40*/ )
			NES_SWAP_BUTTON ( START, SELECT                  )

			goto hell;

		case 0x8850924BUL: // Tetris

			NES_DIPBEGIN    ( "Coinage",            0x03, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credit", 0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credit", 0x02    )
			NES_DIPVALUE    ( "2 Coins / 1 Credit", 0x01    )
			NES_DIPVALUE    ( "3 Coins / 1 Credit", 0x03    )
		    NES_DIPEND	    (                               )
			NES_DIPBEGIN    ( "unknown",            0x04, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x04    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "unknown",            0x08, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x08    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "unknown",            0x10, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x10    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "Palette Color",      0x60, 2 )
			NES_DIPVALUE    ( "Black",              0x40    )
			NES_DIPVALUE    ( "Green",              0x20    )
			NES_DIPVALUE    ( "Grey",               0x60    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "unknown",            0x80, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x80    )
			NES_DIPEND      (                               )
			NES_SWAP_BUTTON ( START, SELECT                 )

			goto hell;

     	case 0x8C0C2DF5UL: // Top Gun
			
			NES_DIPBEGIN  ( "Coinage",             0x07, 0 )
			NES_DIPVALUE  ( "1 Coins / 1 Credit",  0x00    )
			NES_DIPVALUE  ( "1 Coins / 2 Credit",  0x04    )
			NES_DIPVALUE  ( "1 Coins / 3 Credit",  0x02    )
			NES_DIPVALUE  ( "2 Coins / 1 Credit",  0x06    )
			NES_DIPVALUE  ( "3 Coins / 1 Credit",  0x01    )
			NES_DIPVALUE  ( "4 Coins / 1 Credit",  0x05    )
			NES_DIPVALUE  ( "5 Coins / 1 Credit",  0x03    )
			NES_DIPVALUE  ( "Free Play",	       0x07    )
		    NES_DIPEND	  (                                )
     		NES_DIPBEGIN  ( "Lives per Coin",      0x08, 0 )
			NES_DIPVALUE  ( "3 - 12 Max",          0x00    )
			NES_DIPVALUE  ( "2 - 9 Max",	       0x08    )
			NES_DIPEND	  (                                )
     		NES_DIPBEGIN  ( "Bonus",               0x30, 0 )
     		NES_DIPVALUE  ( "30k and every 50k",   0x00    )
     		NES_DIPVALUE  ( "50k and every 100k",  0x20    )
  			NES_DIPVALUE  ( "100k and every 150k", 0x10    )
			NES_DIPVALUE  ( "200k and every 200k", 0x30    )
		    NES_DIPEND	  (                                )
			NES_DIPBEGIN  ( "Difficulty",          0x40, 0 )
			NES_DIPVALUE  ( "Normal",              0x00    )
			NES_DIPVALUE  ( "Hard",                0x40    )
			NES_DIPEND	  (                                )
			NES_DIPBEGIN  ( "Demo Sounds",         0x80, 1 )
			NES_DIPVALUE  ( "Off",                 0x00    )
			NES_DIPVALUE  ( "On",                  0x80    )
			NES_DIPEND	  (                                )
			NES_CRACK_PPU ( 0x1B                           )

			goto hell;

		case 0x70901B25UL: // Slalom
			
			NES_DIPBEGIN    ( "Coinage",             0x07, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credit",  0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credit",  0x04    )
			NES_DIPVALUE    ( "1 Coins / 3 Credit",  0x02    )
			NES_DIPVALUE    ( "2 Coins / 1 Credit",  0x06    )
			NES_DIPVALUE    ( "3 Coins / 1 Credit",  0x01    )
			NES_DIPVALUE    ( "4 Coins / 1 Credit",  0x05    )
			NES_DIPVALUE    ( "5 Coins / 1 Credit",  0x03    )
			NES_DIPVALUE    ( "Free Play",  	     0x07    )
		    NES_DIPEND	    (                                )
			NES_DIPBEGIN    ( "Freestyle Points",    0x08, 0 )
			NES_DIPVALUE    ( "Left / Right",        0x00,   )
			NES_DIPVALUE    ( "Hold Time",           0x08    )
		    NES_DIPEND	    (                                )
			NES_DIPBEGIN    ( "Difficulty",          0x30, 1 )
			NES_DIPVALUE    ( "Easy",                0x00    )
			NES_DIPVALUE    ( "Normal",              0x10    )
			NES_DIPVALUE    ( "Hard",                0x20    )
			NES_DIPVALUE    ( "Hardest",             0x30    )
		    NES_DIPEND	    (                                )
			NES_DIPBEGIN    ( "Allow Continue",      0x40, 1 )
			NES_DIPVALUE    ( "No",                  0x40    )
			NES_DIPVALUE    ( "Yes",                 0x00    )
		    NES_DIPEND	    (                                )
			NES_DIPBEGIN    ( "Inverted input",      0x80, 0 )
			NES_DIPVALUE    ( "Off",                 0x00    )
			NES_DIPVALUE    ( "On",                  0x80    )
		    NES_DIPEND	    (                                )
			NES_SWAP_BUTTON ( START, SELECT                  )
			NES_MAP_COLOR   ( 1                              )

			goto hell;

		case 0xCF36261E: // Sky Kid

			NES_DIPBEGIN    ( "unknown",            0x01, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x01    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "unknown",            0x02, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x02    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "Lives",              0x04, 0 )
			NES_DIPVALUE    ( "2",                  0x00    )
			NES_DIPVALUE    ( "3",                  0x04    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "Coinage",            0x18, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credit", 0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credit", 0x08    )
			NES_DIPVALUE    ( "2 Coins / 1 Credit", 0x10    )
			NES_DIPVALUE    ( "3 Coins / 1 Credit", 0x18    )
		    NES_DIPEND	    (                               )
			NES_DIPBEGIN    ( "Color Palette",      0xE0, 0 )
			NES_DIPVALUE    ( "Normal",             0x20    )
			NES_DIPVALUE    ( "Wrong 1",            0x00    )
			NES_DIPVALUE    ( "Wrong 2",            0x40    )
			NES_DIPVALUE    ( "Wrong 3",            0x80    )
			NES_DIPVALUE    ( "Wrong 4",            0xC0    )
		    NES_DIPEND	    (                               )
			NES_SWAP_BUTTON ( START, SELECT                 )

			goto hell;

		case 0xE1AA8214: // Star Luster

			NES_DIPBEGIN    ( "Coinage",            0x03, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credit", 0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credit", 0x02    )
			NES_DIPVALUE    ( "2 Coins / 1 Credit", 0x01    )
			NES_DIPVALUE    ( "3 Coins / 1 Credit", 0x03    )
		    NES_DIPEND	    (                               )
			NES_DIPBEGIN    ( "unknown",            0x04, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x04    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "unknown",            0x08, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x08    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "unknown",            0x10, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x10    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "Palette Color",      0x60, 0 )
			NES_DIPVALUE    ( "Black",              0x40    )
			NES_DIPVALUE    ( "Green",              0x20    )
			NES_DIPVALUE    ( "Grey",               0x60    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "unknown",            0x80, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x80    )
			NES_DIPEND      (                               )
			NES_SWAP_BUTTON ( START, SELECT                 )

			goto hell;

		case 0xD5D7EAC4UL: // Dr. Mario
			
			NES_DIPBEGIN    ( "Drop Rate Increases After", 0x03, 0 )
			NES_DIPVALUE    ( "7 Pills",                   0x00    )
			NES_DIPVALUE    ( "8 Pills",                   0x01    )
			NES_DIPVALUE    ( "9 Pills",                   0x02    )
			NES_DIPVALUE    ( "10 Pills",                  0x03    )
		    NES_DIPEND	    (                                      )
			NES_DIPBEGIN    ( "Virus Level",               0x0C, 0 )
			NES_DIPVALUE    ( "1",                         0x00    )
			NES_DIPVALUE    ( "3",                         0x04    )
			NES_DIPVALUE    ( "5",                         0x08    )
			NES_DIPVALUE    ( "7",                         0x0C    )
		    NES_DIPEND	    (                                      )
			NES_DIPBEGIN    ( "Drop Speed Up",             0x30, 0 )
			NES_DIPVALUE    ( "Slow",                      0x00    )
			NES_DIPVALUE    ( "Medium",                    0x10    )
			NES_DIPVALUE    ( "Fast",                      0x20    )
			NES_DIPVALUE    ( "Fastest",                   0x30    )
		    NES_DIPEND	    (                                      )
			NES_DIPBEGIN    ( "Free Play",                 0x40, 0 )
			NES_DIPVALUE    ( "Off",                       0x00    )
			NES_DIPVALUE    ( "On",                        0x40    )
		    NES_DIPEND	    (                                      )
			NES_DIPBEGIN    ( "Demo Sounds",               0x80, 1 )
			NES_DIPVALUE    ( "Off",                       0x00    )
			NES_DIPVALUE    ( "On",                        0x80    )
		    NES_DIPEND	    (                                      )
			NES_SWAP_BUTTON ( START, SELECT                        )
			NES_MAP_COLOR   ( 2                                    )

			goto hell;

		case 0xFFBEF374UL: // Castlevania
			
			NES_DIPBEGIN  ( "Coinage",            0x07, 0 )
			NES_DIPVALUE  ( "1 Coins / 1 Credit", 0x00    )
			NES_DIPVALUE  ( "1 Coins / 2 Credit", 0x04    )
			NES_DIPVALUE  ( "1 Coins / 3 Credit", 0x02    )
			NES_DIPVALUE  ( "2 Coins / 1 Credit", 0x06    )
			NES_DIPVALUE  ( "3 Coins / 1 Credit", 0x01    )
			NES_DIPVALUE  ( "4 Coins / 1 Credit", 0x05    )
			NES_DIPVALUE  ( "5 Coins / 1 Credit", 0x03    )
			NES_DIPVALUE  ( "Free Play",	      0x07    )
		    NES_DIPEND	  (                               )
     		NES_DIPBEGIN  ( "Lives",              0x08, 1 )
			NES_DIPVALUE  ( "2",                  0x08    )
			NES_DIPVALUE  ( "3",	              0x00    )
			NES_DIPEND    (                               )
     		NES_DIPBEGIN  ( "Bonus",              0x30, 0 )
     		NES_DIPVALUE  ( "100k",               0x00    )
     		NES_DIPVALUE  ( "200k",               0x20    )
  			NES_DIPVALUE  ( "300k",               0x10    )
			NES_DIPVALUE  ( "400k",               0x30    )
			NES_DIPEND    (                               )
			NES_DIPBEGIN  ( "Difficulty",         0x40, 0 )
			NES_DIPVALUE  ( "Normal",             0x00    )
			NES_DIPVALUE  ( "Hard",               0x40    )
			NES_DIPEND	  (                               )
			NES_MAP_COLOR ( 1                             )

			goto hell;

		case 0xE2C0A2BEUL: // Platoon
			
			NES_DIPBEGIN  ( "unknown",            0x01, 0 )
			NES_DIPVALUE  ( "Off",                0x00    )
			NES_DIPVALUE  ( "On",                 0x01    )
			NES_DIPEND    (                               )
			NES_DIPBEGIN  ( "unknown",            0x02, 0 )
			NES_DIPVALUE  ( "Off",                0x00    )
			NES_DIPVALUE  ( "On",                 0x02    )
			NES_DIPEND    (                               )
			NES_DIPBEGIN  ( "Demo Sounds",        0x04, 1 )
			NES_DIPVALUE  ( "Off",                0x00    )
			NES_DIPVALUE  ( "On",                 0x04    )
			NES_DIPEND	  (                               )
			NES_DIPBEGIN  ( "unknown",            0x08, 0 )
			NES_DIPVALUE  ( "Off",                0x00    )
			NES_DIPVALUE  ( "On",                 0x08    )
			NES_DIPEND    (                               )
			NES_DIPBEGIN  ( "unknown",            0x10, 0 )
			NES_DIPVALUE  ( "Off",                0x00    )
			NES_DIPVALUE  ( "On",                 0x10    )
			NES_DIPEND    (                               )
			NES_DIPBEGIN  ( "Coinage",            0xE0, 0 )
			NES_DIPVALUE  ( "1 Coins / 1 Credit", 0x00    )
			NES_DIPVALUE  ( "1 Coins / 2 Credit", 0x20    )
			NES_DIPVALUE  ( "1 Coins / 3 Credit", 0x40    )
			NES_DIPVALUE  ( "2 Coins / 1 Credit", 0x60    )
			NES_DIPVALUE  ( "3 Coins / 1 Credit", 0x80    )
			NES_DIPVALUE  ( "4 Coins / 1 Credit", 0xA0    )
     		NES_DIPVALUE  ( "5 Coins / 1 Credit", 0xC0    )
			NES_DIPVALUE  ( "Free Play",	      0xE0    )
		    NES_DIPEND	  (                               )
			NES_MAP_COLOR ( 0                             )

			goto hell;

		case 0xCBE85490UL: // Excitebike
		case 0x29155E0CUL: // Excitebike (alt)
			
			NES_DIPBEGIN  ( "Coinage",                  0x07, 0 )
			NES_DIPVALUE  ( "1 Coins / 1 Credit",       0x00    )
			NES_DIPVALUE  ( "1 Coins / 2 Credit",       0x04    )
			NES_DIPVALUE  ( "1 Coins / 3 Credit",       0x02    )
			NES_DIPVALUE  ( "1 Coins / 4 Credit",       0x06    )
			NES_DIPVALUE  ( "2 Coins / 1 Credit",       0x01    )
			NES_DIPVALUE  ( "3 Coins / 1 Credit",       0x05    )
			NES_DIPVALUE  ( "4 Coins / 1 Credit",       0x03    )
			NES_DIPVALUE  ( "Free Play",	            0x07    )
		    NES_DIPEND	  (                                     )
			NES_DIPBEGIN  ( "Bonus",                    0x18, 0 )
			NES_DIPVALUE  ( "100k and Every 50k",       0x00    )
			NES_DIPVALUE  ( "Every 100k",               0x10    )
			NES_DIPVALUE  ( "100k Only",                0x08    )
			NES_DIPVALUE  ( "None",                     0x18    )
		    NES_DIPEND	  (                                     )
			NES_DIPBEGIN  ( "1st Half Qualifying Time", 0x20, 0 )
			NES_DIPVALUE  ( "Normal",                   0x00    )
			NES_DIPVALUE  ( "Hard",                     0x20    )
		    NES_DIPEND	  (                                     )
			NES_DIPBEGIN  ( "2nd Half Qualifying Time", 0x40, 0 )
			NES_DIPVALUE  ( "Normal",                   0x00    )
			NES_DIPVALUE  ( "Hard",                     0x40    )
		    NES_DIPEND	  (                                     )
			NES_MAP_COLOR ( pRomCrc == 0x29155E0CUL ? 3 : 2     )

			goto hell;

		case 0x07138C06UL: // Clu Clu Land
			
			NES_DIPBEGIN    ( "Coinage",            0x07, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credit", 0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credit", 0x04    )
			NES_DIPVALUE    ( "1 Coins / 3 Credit", 0x02    )
			NES_DIPVALUE    ( "1 Coins / 4 Credit", 0x06    )
			NES_DIPVALUE    ( "2 Coins / 1 Credit", 0x01    )
			NES_DIPVALUE    ( "3 Coins / 1 Credit", 0x05    )
			NES_DIPVALUE    ( "4 Coins / 1 Credit", 0x03    )
			NES_DIPVALUE    ( "Free Play",	        0x07    )
		    NES_DIPEND	    (                               )
			NES_DIPBEGIN    ( "unknown",            0x08, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x08    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "unknown",            0x10, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x10    )
			NES_DIPEND      (                               )
     		NES_DIPBEGIN    ( "Lives",              0x60, 1 )
			NES_DIPVALUE    ( "2",                  0x60    )
			NES_DIPVALUE    ( "3",	                0x00    )
			NES_DIPVALUE    ( "4",                  0x40    )
			NES_DIPVALUE    ( "5",                  0x20    )
			NES_DIPEND      (                               )
			NES_DIPBEGIN    ( "unknown",            0x80, 0 )
			NES_DIPVALUE    ( "Off",                0x00    )
			NES_DIPVALUE    ( "On",                 0x80    )
			NES_DIPEND      (                               )
			NES_SWAP_BUTTON ( START, SELECT                 )
			NES_MAP_COLOR   ( 3                             )

			goto hell;

		case 0x43A357EFUL: // Ice Climber
			
			NES_DIPBEGIN    ( "Coinage",              0x07, 0 )
     		NES_DIPVALUE    ( "1 Coins / 1 Credit",   0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credit",   0x04    )
			NES_DIPVALUE    ( "1 Coins / 3 Credit",   0x02    )
			NES_DIPVALUE    ( "1 Coins / 4 Credit",   0x06    )
			NES_DIPVALUE    ( "2 Coins / 1 Credit",   0x01    )
			NES_DIPVALUE    ( "3 Coins / 1 Credit",   0x05    )
			NES_DIPVALUE    ( "4 Coins / 1 Credit",   0x03    )
			NES_DIPVALUE    ( "Free Play",	          0x07    )
		    NES_DIPEND	    (                                 )
     		NES_DIPBEGIN    ( "Lives",                0x18, 0 )
			NES_DIPVALUE    ( "3",                    0x00    )
			NES_DIPVALUE    ( "4",	                  0x10    )
			NES_DIPVALUE    ( "5",                    0x08    )
			NES_DIPVALUE    ( "7",	                  0x18    )
		    NES_DIPEND	    (                                 )
			NES_DIPBEGIN    ( "Difficulty",           0x20, 0 )
			NES_DIPVALUE    ( "Normal",               0x00    )
			NES_DIPVALUE    ( "Hard",                 0x20    )
			NES_DIPEND	    (                                 )
			NES_DIPBEGIN    ( "Time before the bear", 0x40, 0 )
			NES_DIPVALUE    ( "Long",                 0x00    )
			NES_DIPVALUE    ( "Short",                0x40    )
			NES_DIPEND	    (                                 )
			NES_SWAP_BUTTON ( START, SELECT                   )
			NES_MAP_COLOR   ( 3                               )

			goto hell;

		case 0x737DD1BFUL: // Super Mario Bros	
		case 0x4BF3972DUL: 
		case 0x8B60CC58UL:
		case 0x8192C804UL: 

			NES_DIPBEGIN    ( "Coinage",            0x07, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credit", 0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credit", 0x06    )
			NES_DIPVALUE    ( "1 Coins / 3 Credit", 0x01    )
			NES_DIPVALUE    ( "1 Coins / 4 Credit", 0x05    )
     		NES_DIPVALUE    ( "1 Coins / 5 Credit", 0x03    )
			NES_DIPVALUE    ( "2 Coins / 1 Credit", 0x04    )
			NES_DIPVALUE    ( "3 Coins / 1 Credit", 0x02    )
			NES_DIPVALUE    ( "Free Play",	        0x07    )
		    NES_DIPEND	    (                               )
     		NES_DIPBEGIN    ( "Lives",              0x08, 1 )
			NES_DIPVALUE    ( "2",                  0x08    )
			NES_DIPVALUE    ( "3",	                0x00    )
		    NES_DIPEND	    (                               )
     		NES_DIPBEGIN    ( "Bonus Life",         0x30, 0 )
			NES_DIPVALUE    ( "100",                0x00    )
			NES_DIPVALUE    ( "150",	            0x20    )
			NES_DIPVALUE    ( "200",	            0x10    )
			NES_DIPVALUE    ( "250",	            0x30    )
		    NES_DIPEND	    (                               )
     		NES_DIPBEGIN    ( "Timer",              0x40, 0 )
			NES_DIPVALUE    ( "Normal",             0x00    )
			NES_DIPVALUE    ( "Fast",	            0x40    )
		    NES_DIPEND	    (                               )
     		NES_DIPBEGIN    ( "Continue Lives",     0x80, 0 )
			NES_DIPVALUE    ( "3",                  0x80    )
			NES_DIPVALUE    ( "4",	                0x00    )
		    NES_DIPEND	    (                               )
			NES_SWAP_BUTTON ( START, SELECT                 )
			NES_MAP_COLOR   ( 3                             )

			goto hell;

		case 0xEC461DB9UL: // Pinball
		case 0xE528F651UL: // Pinball (alt)
			
			NES_DIPBEGIN    ( "Coinage",            0x07, 0   )
			NES_DIPVALUE    ( "1 Coins / 1 Credit", 0x01      )
			NES_DIPVALUE    ( "1 Coins / 2 Credit", 0x06      )
			NES_DIPVALUE    ( "1 Coins / 3 Credit", 0x02      )
     		NES_DIPVALUE    ( "1 Coins / 4 Credit", 0x04      )
			NES_DIPVALUE    ( "2 Coins / 1 Credit", 0x05      )
			NES_DIPVALUE    ( "3 Coins / 1 Credit", 0x03      )
			NES_DIPVALUE    ( "4 Coins / 1 Credit", 0x07      )
			NES_DIPVALUE    ( "Free Play",          0x00      )
		    NES_DIPEND	    (                                 )
			NES_DIPBEGIN    ( "unknown",            0x08, 0   )
			NES_DIPVALUE    ( "Off",                0x00      )
			NES_DIPVALUE    ( "On",                 0x08      )
			NES_DIPEND      (                                 )
			NES_DIPBEGIN    ( "unknown",            0x10, 0   )
			NES_DIPVALUE    ( "Off",                0x00      )
			NES_DIPVALUE    ( "On",                 0x10      )
			NES_DIPEND      (                                 )
			NES_DIPBEGIN    ( "Balls",              0x60, 1   )
			NES_DIPVALUE    ( "2",                  0x60      )
			NES_DIPVALUE    ( "3",                  0x00      )
			NES_DIPVALUE    ( "4",                  0x40      )
			NES_DIPVALUE    ( "5",                  0x20      )
			NES_DIPEND      (                                 )
			NES_DIPBEGIN    ( "Ball Speed",         0x80, 0   )
			NES_DIPVALUE    ( "Normal",             0x00      )
			NES_DIPVALUE    ( "Fast",               0x80      )
			NES_DIPEND      (                                 )
			NES_SWAP_BUTTON ( START, SELECT                   )
			NES_MAP_COLOR   ( pRomCrc == 0xEC461DB9UL ? 0 : 3 )

			goto hell;

		case 0x0B65A917UL: // Mach Rider
		case 0x8A6A9848UL:
			
			NES_DIPBEGIN    ( "Coinage",             0x07, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credits", 0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credits", 0x04    )
			NES_DIPVALUE    ( "1 Coins / 3 Credits", 0x02    )
			NES_DIPVALUE    ( "1 Coins / 4 Credits", 0x06    )
			NES_DIPVALUE    ( "2 Coins / 1 Credits", 0x01    )
			NES_DIPVALUE    ( "3 Coins / 1 Credits", 0x05    )
     		NES_DIPVALUE    ( "4 Coins / 1 Credits", 0x03    )
			NES_DIPVALUE    ( "Free Play",	         0x07    )
			NES_DIPEND      (                                )
			NES_DIPBEGIN    ( "Time",				 0x18, 0 )
			NES_DIPVALUE    ( "280",             	 0x00    )
			NES_DIPVALUE    ( "250",	             0x10    )
			NES_DIPVALUE    ( "220",	             0x08    )
			NES_DIPVALUE    ( "200",	             0x18    )
			NES_DIPEND      (                                )
			NES_DIPBEGIN    ( "unknown",             0x20, 0 )
			NES_DIPVALUE    ( "Off",                 0x00    )
			NES_DIPVALUE    ( "On",                  0x20    )
			NES_DIPEND      (                                )
			NES_DIPBEGIN    ( "unknown",             0x40, 0 )
			NES_DIPVALUE    ( "Off",                 0x00    )
			NES_DIPVALUE    ( "On",                  0x40    )
			NES_DIPEND      (                                )
			NES_DIPBEGIN    ( "unknown",             0x80, 0 )
			NES_DIPVALUE    ( "Off",                 0x00    )
			NES_DIPVALUE    ( "On",                  0x80    )
			NES_DIPEND      (                                )
			NES_SWAP_BUTTON ( START, SELECT                  )
			NES_MAP_COLOR   ( 1                              )

			goto hell;

		case 0x46914E3EUL: // Soccer
			
			NES_DIPBEGIN    ( "Coinage",             0x07, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credits", 0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credits", 0x04    )
			NES_DIPVALUE    ( "1 Coins / 3 Credits", 0x02    )
     		NES_DIPVALUE    ( "1 Coins / 4 Credits", 0x06    )
			NES_DIPVALUE    ( "2 Coins / 1 Credits", 0x01    )
			NES_DIPVALUE    ( "3 Coins / 1 Credits", 0x05    )
			NES_DIPVALUE    ( "4 Coins / 1 Credits", 0x03    )
			NES_DIPVALUE    ( "Free Play",	         0x07    )
			NES_DIPEND      (                                )
			NES_DIPBEGIN    ( "Points Timer",        0x18, 2 )
			NES_DIPVALUE    ( "600 Pts",             0x00    )
			NES_DIPVALUE    ( "800 Pts",             0x10    )
			NES_DIPVALUE    ( "1000 Pts",            0x08    )
			NES_DIPVALUE    ( "1200 Pts",            0x18    )
			NES_DIPEND      (                                )
			NES_DIPBEGIN    ( "Difficulty",          0x40, 1 )
			NES_DIPVALUE    ( "Easy",                0x00    )
			NES_DIPVALUE    ( "Normal",              0x40    )
			NES_DIPVALUE    ( "Hard",                0x20    )
			NES_DIPVALUE    ( "Very Hard",           0x60    )
		    NES_DIPEND	    (                                )
			NES_SWAP_BUTTON ( START, SELECT                  )
			NES_MAP_COLOR   ( 2                              )

			goto hell;

		case 0x70433F2CUL: // Battle City
		case 0x8D15A6E6UL: // bad .nes

			NES_DIPBEGIN    ( "Credits for 2 Players", 0x01, 1 )
			NES_DIPVALUE    ( "1",                     0x00    )
			NES_DIPVALUE    ( "2",                     0x01    )
			NES_DIPEND      (                                  )
			NES_DIPBEGIN    ( "Lives",                 0x02, 0 )
			NES_DIPVALUE    ( "3",                     0x00    )
			NES_DIPVALUE    ( "5",                     0x02    )
			NES_DIPEND      (                                  )
			NES_DIPBEGIN    ( "Demo Sounds",           0x04, 1 )
			NES_DIPVALUE    ( "Off",                   0x00    )
			NES_DIPVALUE    ( "On",                    0x04    )
		    NES_DIPEND	    (                                  )
			NES_DIPBEGIN    ( "unknown",               0x08, 0 )
			NES_DIPVALUE    ( "Off",                   0x00    )
			NES_DIPVALUE    ( "On",                    0x08    )
			NES_DIPEND      (                                  )
			NES_DIPBEGIN    ( "unknown",               0x10, 0 )
			NES_DIPVALUE    ( "Off",                   0x00    )
			NES_DIPVALUE    ( "On",                    0x10    )
			NES_DIPEND      (                                  )
			NES_DIPBEGIN    ( "unknown",               0x20, 0 )
			NES_DIPVALUE    ( "Off",                   0x00    )
			NES_DIPVALUE    ( "On",                    0x20    )
			NES_DIPEND      (                                  )
			NES_DIPBEGIN    ( "Color Palette",         0xC0, 0 )
			NES_DIPVALUE    ( "Normal",                0x80    )
			NES_DIPVALUE    ( "Wrong 1",               0x00    )
			NES_DIPVALUE    ( "Wrong 2",               0x40    )
			NES_DIPVALUE    ( "Wrong 3",               0xC0    )
		    NES_DIPEND	    (                                  )
			NES_SWAP_BUTTON ( START, SELECT                    )
			NES_MAP_COLOR   ( 2                                )

			goto hell;

		case 0xD99A2087UL: // Gradius
			
			NES_DIPBEGIN    ( "Coinage",             0x07, 0 )
			NES_DIPVALUE    ( "1 Coins / 1 Credits", 0x00    )
			NES_DIPVALUE    ( "1 Coins / 2 Credits", 0x04    )
			NES_DIPVALUE    ( "1 Coins / 3 Credits", 0x02    )
     		NES_DIPVALUE    ( "2 Coins / 1 Credits", 0x06    )
			NES_DIPVALUE    ( "3 Coins / 1 Credits", 0x01    )
			NES_DIPVALUE    ( "4 Coins / 1 Credits", 0x05    )
			NES_DIPVALUE    ( "5 Coins / 1 Credits", 0x03    )
			NES_DIPVALUE    ( "Free Play",           0x07    )
			NES_DIPEND      (                                )
			NES_DIPBEGIN    ( "Lives",               0x08, 0 )
			NES_DIPVALUE    ( "3",                   0x08    )
			NES_DIPVALUE    ( "4",                   0x00    )
			NES_DIPEND      (                                )
     		NES_DIPBEGIN    ( "Bonus",               0x30, 0 )
			NES_DIPVALUE    ( "100k",                0x00    )
			NES_DIPVALUE    ( "200k",	             0x20    )
			NES_DIPVALUE    ( "300k",	             0x10    )
			NES_DIPVALUE    ( "400k",	             0x30    )
		    NES_DIPEND	    (                                )
			NES_DIPBEGIN    ( "Difficulty",          0x40, 0 )
			NES_DIPVALUE    ( "Normal",              0x00    )
			NES_DIPVALUE    ( "Hard",                0x40    )
			NES_DIPEND	    (                                )
			NES_DIPBEGIN    ( "Demo Sounds",         0x80, 1 )
			NES_DIPVALUE    ( "Off",                 0x00    )
			NES_DIPVALUE    ( "On",                  0x80    )
			NES_DIPEND	    (                                )
			NES_SWAP_BUTTON ( START, SELECT                  )
			NES_MAP_COLOR   ( 0                              )

			goto hell;

		case 0x1E438D52UL: // Goonies
			
			NES_DIPBEGIN  ( "Coinage",             0x07, 0 )
			NES_DIPVALUE  ( "1 Coins / 1 Credits", 0x00    )
			NES_DIPVALUE  ( "1 Coins / 2 Credits", 0x04    )
			NES_DIPVALUE  ( "1 Coins / 3 Credits", 0x02    )
     		NES_DIPVALUE  ( "2 Coins / 1 Credits", 0x06    )
			NES_DIPVALUE  ( "3 Coins / 1 Credits", 0x01    )
			NES_DIPVALUE  ( "4 Coins / 1 Credits", 0x05    )
			NES_DIPVALUE  ( "5 Coins / 1 Credits", 0x03    )
			NES_DIPVALUE  ( "Free Play",	       0x07    )
			NES_DIPEND    (                                )
			NES_DIPBEGIN  ( "Lives",               0x08, 0 )
			NES_DIPVALUE  ( "3",                   0x00    )
			NES_DIPVALUE  (	"2",                   0x08    )
			NES_DIPEND    (                                )
			NES_DIPBEGIN  ( "unknown",             0x10, 0 )
			NES_DIPVALUE  ( "Off",                 0x00    )
			NES_DIPVALUE  ( "On",                  0x10    )
			NES_DIPEND    (                                )
			NES_DIPBEGIN  ( "unknown",             0x20, 0 )
			NES_DIPVALUE  ( "Off",                 0x00    )
			NES_DIPVALUE  ( "On",                  0x20    )
			NES_DIPEND    (                                )
			NES_DIPBEGIN  ( "Timer",               0x40, 0 )
			NES_DIPVALUE  ( "Normal",              0x00    )
			NES_DIPVALUE  ( "Fast",                0x40    )
			NES_DIPEND    (                                )
			NES_DIPBEGIN  ( "Demo Sounds",         0x80, 1 )
			NES_DIPVALUE  ( "Off",                 0x00    )
			NES_DIPVALUE  ( "On",                  0x80    )
			NES_DIPEND	  (                                )
			NES_MAP_COLOR ( 2                              )

			goto hell;

		case 0xFF5135A3UL: // Hogan's Alley
			
			NES_DIPBEGIN  ( "Coinage",             0x07, 0 )
			NES_DIPVALUE  ( "5 Coins / 1 Credits", 0x03    )
			NES_DIPVALUE  ( "4 Coins / 1 Credits", 0x05    )
			NES_DIPVALUE  ( "3 Coins / 1 Credits", 0x01    )
     		NES_DIPVALUE  ( "2 Coins / 1 Credits", 0x06    )
			NES_DIPVALUE  ( "1 Coins / 1 Credits", 0x00    )
			NES_DIPVALUE  ( "1 Coins / 2 Credits", 0x04    )
			NES_DIPVALUE  ( "1 Coins / 3 Credits", 0x02    )
			NES_DIPVALUE  ( "Free Play",	       0x07    )
			NES_DIPEND    (                                )
			NES_DIPBEGIN  ( "Difficulty",          0x18, 1 )
			NES_DIPVALUE  ( "Easy",                0x00    )
			NES_DIPVALUE  ( "Normal",              0x08    )
			NES_DIPVALUE  ( "Hard",                0x10    )
			NES_DIPVALUE  ( "Very Hard",           0x18    )
			NES_DIPEND	  (                                )
			NES_DIPBEGIN  ( "Misses per Game",     0x20, 1 )
			NES_DIPVALUE  ( "3",                   0x00    )
			NES_DIPVALUE  ( "5",                   0x20    )
			NES_DIPEND	  (                                )
     		NES_DIPBEGIN  ( "Bonus Life",          0xC0, 0 )
			NES_DIPVALUE  ( "30000",               0x00    )
			NES_DIPVALUE  ( "50000",	           0x40    )
			NES_DIPVALUE  ( "80000",	           0x80    )
			NES_DIPVALUE  ( "100000",	           0xC0    )
		    NES_DIPEND	  (                                )
			NES_MAP_COLOR ( 0                              )

			goto hell;

		case 0x17AE56BEUL: // Freedom Force
			
			NES_DIPSKIP()
			NES_MAP_COLOR(0)
			goto hell;

		case 0xF9D3B0A3UL: // Super Xevious
		case 0x66BB838FUL: // Super Xevious
			
			NES_DIPSKIP()
			VsSystem = new VSSUPERXEVIOUS(context);
			break;

		case 0xCC2C4B5DUL: // Golf
		case 0x86167220UL: // Lady Golf

			NES_DIPBEGIN  ( "Coinage",                 0x07, 0 )
     		NES_DIPVALUE  ( "1 Coins / 1 Credits",     0x01    )
			NES_DIPVALUE  ( "1 Coins / 2 Credits",     0x06    )
			NES_DIPVALUE  ( "1 Coins / 3 Credits",     0x02    )
			NES_DIPVALUE  ( "1 Coins / 4 Credits",     0x04    )
			NES_DIPVALUE  ( "2 Coins / 1 Credits",     0x05    )
			NES_DIPVALUE  ( "3 Coins / 1 Credits",     0x03    )
			NES_DIPVALUE  ( "4 Coins / 1 Credits",     0x07    )
			NES_DIPVALUE  ( "Free Play",	           0x00    )
			NES_DIPEND    (                                    )
			NES_DIPBEGIN  ( "Hole Size",               0x08, 0 )
			NES_DIPVALUE  ( "Large",                   0x00    )
			NES_DIPVALUE  ( "Small",                   0x08    )
			NES_DIPEND	  (                                    )
			NES_DIPBEGIN  ( "Points per Stroke",       0x10, 0 )
			NES_DIPVALUE  ( "Easier",                  0x00    )
			NES_DIPVALUE  ( "Harder",                  0x10    )
			NES_DIPEND	  (                                    )
			NES_DIPBEGIN  ( "Starting Points",         0x60, 0 )
			NES_DIPVALUE  ( "10",                      0x00    )
			NES_DIPVALUE  ( "13",                      0x40    )
			NES_DIPVALUE  ( "16",                      0x20    )
			NES_DIPVALUE  ( "20",                      0x60    )
			NES_DIPEND	  (                                    )
			NES_DIPBEGIN  ( "Difficulty Vs. Computer", 0x80, 0 )
			NES_DIPVALUE  ( "Easy",                    0x00    )
			NES_DIPVALUE  ( "Hard",                    0x80    )
			NES_DIPEND	  (                                    )
			NES_MAP_COLOR ( 1                                  )

			goto hell;

		case 0xB90497AA: // Tennis

			NES_DIPBEGIN ( "Difficulty Vs. Computer", 0x03, 1 )
     		NES_DIPVALUE ( "Easy",                    0x00    )
			NES_DIPVALUE ( "Normal",                  0x02    )
			NES_DIPVALUE ( "Hard",                    0x01    )
			NES_DIPVALUE ( "Very Hard",               0x03    )
			NES_DIPEND   (                                    )
			NES_DIPBEGIN ( "Difficulty Vs. Player",   0x0C, 1 )
     		NES_DIPVALUE ( "Easy",                    0x00    )
			NES_DIPVALUE ( "Normal",                  0x08    )
			NES_DIPVALUE ( "Hard",                    0x04    )
			NES_DIPVALUE ( "Very Hard",               0x0C    )
			NES_DIPEND   (                                    )
			NES_DIPBEGIN ( "Raquet Size",             0x10, 0 )
     		NES_DIPVALUE ( "Large",                   0x00    )
			NES_DIPVALUE ( "Small",                   0x10    )
			NES_DIPEND   (                                    )
			NES_DIPBEGIN ( "Extra Score",             0x20, 0 )
     		NES_DIPVALUE ( "1 Set",                   0x00    )
			NES_DIPVALUE ( "1 Game",                  0x20    )
			NES_DIPEND   (                                    )
			NES_DIPBEGIN ( "Court Color",             0x40, 0 )
     		NES_DIPVALUE ( "Green",                   0x00    )
			NES_DIPVALUE ( "Blue",                    0x40    )
			NES_DIPEND   (                                    )
			NES_DIPBEGIN ( "Copyright",               0x80, 0 )
     		NES_DIPVALUE ( "Japan",                   0x00    )
			NES_DIPVALUE ( "USA",                     0x80    )
			NES_DIPEND   (                                    )

			goto hell;

		default:

			NES_DIPSKIP()

        hell:

			VsSystem = new VSSYSTEM(context);
			break;
	}

	return VsSystem;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const U8 VSSYSTEM::ColorMaps[4][64] =
{
	{
		0x35, 0xFF, 0x16, 0x22, 0x1C, 0xFF, 0xFF, 0x15,
		0xFF, 0x00, 0x27, 0x05, 0x04, 0x27, 0x08, 0x30,
		0x21, 0xFF, 0xFF, 0x29, 0x3C, 0xFF, 0x36, 0x12,
		0xFF, 0x2B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01,
		0xFF, 0x31, 0xFF, 0x2A, 0x2C, 0x0C, 0xFF, 0xFF,
		0xFF, 0x07, 0x34, 0x06, 0x13, 0xFF, 0x26, 0x0F,
		0xFF, 0x19, 0x10, 0x0A, 0xFF, 0xFF, 0xFF, 0x17,
		0xFF, 0x11, 0x09, 0xFF, 0xFF, 0x25, 0x18, 0xFF 
	},
	{
		0xFF, 0x27, 0x18, 0xFF, 0x3A, 0x25, 0xFF, 0x31,
		0x16, 0x13, 0x38, 0x34, 0x20, 0x23, 0xFF, 0x0B,
		0xFF, 0x21, 0x06, 0xFF, 0x1B, 0x29, 0xFF, 0x22,
		0xFF, 0x24, 0xFF, 0xFF, 0xFF, 0x08, 0xFF, 0x03,
		0xFF, 0x36, 0x26, 0x33, 0x11, 0xFF, 0x10, 0x02,
		0x14, 0xFF, 0x00, 0x09, 0x12, 0x0F, 0xFF, 0x30,
		0xFF, 0xFF, 0x2A, 0x17, 0x0C, 0x01, 0x15, 0x19,
		0xFF, 0x2C, 0x07, 0x37, 0xFF, 0x05, 0xFF, 0xFF 
	},
	{
		0xFF, 0xFF, 0xFF, 0x10, 0x1A, 0x30, 0x31, 0x09,
		0x01, 0x0F, 0x36, 0x08, 0x15, 0xFF, 0xFF, 0xF0,
		0x22, 0x1C, 0xFF, 0x12, 0x19, 0x18, 0x17, 0xFF,
		0x00, 0xFF, 0xFF, 0x02, 0x16, 0x06, 0xFF, 0x35,
		0x23, 0xFF, 0x8B, 0xF7, 0xFF, 0x27, 0x26, 0x20,
		0x29, 0xFF, 0x21, 0x24, 0x11, 0xFF, 0xEF, 0xFF,
		0x2C, 0xFF, 0xFF, 0xFF, 0x07, 0xF9, 0x28, 0xFF,
		0x0A, 0xFF, 0x32, 0x37, 0x13, 0xFF, 0xFF, 0x0C 
	},
	{
		0x18, 0xFF, 0x1C, 0x89, 0xFF, 0xFF, 0x01, 0x17,
		0x10, 0x0F, 0x2A, 0xFF, 0x36, 0x37, 0x1A, 0xFF,
		0x25, 0xFF, 0x12, 0xFF, 0x0F, 0xFF, 0xFF, 0x26,
		0xFF, 0xFF, 0x22, 0xFF, 0xFF, 0x0F, 0x3A, 0x21,
		0x05, 0x0A, 0x07, 0xC2, 0x13, 0xFF, 0x00, 0x15,
		0x0C, 0xFF, 0x11, 0xFF, 0xFF, 0x38, 0xFF, 0xFF,
		0xFF, 0xFF, 0x08, 0x45, 0xFF, 0xFF, 0x30, 0x3C,
		0x0F, 0x27, 0xFF, 0x60, 0x29, 0xFF, 0x30, 0x09 
	}
};

NES_NAMESPACE_END
