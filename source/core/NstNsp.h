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

#ifndef NST_NSP_H
#define NST_NSP_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class NSP
{
public:

	PDXRESULT Load(PDXFILE&);
	PDXRESULT Save(PDXFILE&) const;

	VOID SetContext(const IO::NSP::CONTEXT&); 
	VOID GetContext(IO::NSP::CONTEXT&) const;

private:

	VOID ValidateContext();

	PDXRESULT ParseType      (PDXFILE&);
	PDXRESULT ParseComment   (PDXFILE&);
	PDXRESULT ParseMode      (PDXFILE&);
	PDXRESULT ParseGenie     (PDXFILE&);
	PDXRESULT ParsePort      (PDXFILE&);
	PDXRESULT ParseStateSlot (PDXFILE&);

	static PDXRESULT ParseLineEnd  (PDXFILE&);
	static PDXRESULT ParseChoice   (PDXFILE&,PDXSTRING&);
	static PDXRESULT ParseRestType (PDXFILE&,const CHAR* const);
	static VOID      RemoveSpace   (PDXSTRING&);

	IO::NSP::CONTEXT context;
};

NES_NAMESPACE_END

#endif
