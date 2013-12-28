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

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL MACHINE::IsOn()        const { return on;                }
inline BOOL MACHINE::IsOff()       const { return !on;               }
inline BOOL MACHINE::IsPaused()    const { return paused;            }
inline BOOL MACHINE::IsNsf()       const { return nsf != NULL;       }
inline BOOL MACHINE::IsCartridge() const { return cartridge != NULL; }
inline BOOL MACHINE::IsFds()       const { return fds != NULL;       }
inline BOOL MACHINE::IsVs()        const { return VsSystem != NULL;  }
inline BOOL MACHINE::IsImage()     const { return cartridge || fds;  }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline CONTROLLERTYPE MACHINE::ConnectedController(const UINT port) const
{
	PDX_ASSERT(port < 5);
	return (port < 4) ? controller[port]->Type() : (expansion ? expansion->Type() : CONTROLLER_UNCONNECTED);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL MACHINE::IsAnyControllerConnected(const CONTROLLERTYPE type) const
{
	return 
	(
     	( controller[0]->Type() == type ) ||
		( controller[1]->Type() == type ) ||
		( controller[2]->Type() == type ) ||
		( controller[3]->Type() == type ) ||
		( expansion && expansion->Type() == type )
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL MACHINE::IsAnyControllerConnected(const CONTROLLERTYPE p0,const CONTROLLERTYPE p1) const
{
	return 
	(
    	IsAnyControllerConnected( p0 ) ||
		IsAnyControllerConnected( p1 )
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL MACHINE::IsAnyControllerConnected(const CONTROLLERTYPE p0,const CONTROLLERTYPE p1,const CONTROLLERTYPE p2) const
{
	return 
	(
		IsAnyControllerConnected( p0 ) ||
		IsAnyControllerConnected( p1 ) ||
		IsAnyControllerConnected( p2 )
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL MACHINE::IsAnyControllerConnected(const CONTROLLERTYPE p0,const CONTROLLERTYPE p1,const CONTROLLERTYPE p2,const CONTROLLERTYPE p3) const
{
	return 
	(
		IsAnyControllerConnected( p0 ) ||
		IsAnyControllerConnected( p1 ) ||
		IsAnyControllerConnected( p2 ) ||
		IsAnyControllerConnected( p3 )
	);
}
