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

#pragma once

#ifndef NST_PALETTE_H
#define NST_PALETTE_H

////////////////////////////////////////////////////////////////////////////////////////
// Palette
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class PALETTE
{
public:

	enum TYPE
	{
		INTERNAL,
		CUSTOM,
		EMULATE
	};

	PALETTE();

	VOID SetBrightness (const INT);
	VOID SetSaturation (const INT);
	VOID SetHue        (const INT);	
	VOID SetType       (const TYPE);
	VOID Import        (const U8* const);

	BOOL Update();

	const U8* GetData() const;

private:

	enum
	{
		HUE_OFFSET = 241,
		DEFAULT_HUE = 128,
		DEFAULT_BRIGHTNESS = 128,
		DEFAULT_SATURATION = 128
	};

	static VOID ToHSV (DOUBLE,DOUBLE,DOUBLE,DOUBLE&,DOUBLE&,DOUBLE&);
	static VOID ToRGB (DOUBLE,DOUBLE,DOUBLE,DOUBLE&,DOUBLE&,DOUBLE&);

	PDX_NO_INLINE VOID ComputeTV();
	PDX_NO_INLINE VOID ComputeCustom();

	BOOL NeedUpdate;
	TYPE type;

	INT hue;
	INT brightness;
	INT saturation;

	U8 palette[64*8][3];
	U8 custom[64][3];

	static const DOUBLE emphasis[8][3];
	static const U8 DefaultPalette[64][3];
};

#include "NstPalette.inl"

NES_NAMESPACE_END

#endif
