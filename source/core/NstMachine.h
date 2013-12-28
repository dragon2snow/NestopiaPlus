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

#ifndef NST_MACHINE_H
#define NST_MACHINE_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class CPU;
class APU;
class PPU;
class FDS;
class CARTRIDGE;
class PALETTE;
class VSSYSTEM;
class GAMEGENIE;
class NSF;
class MOVIE;

////////////////////////////////////////////////////////////////////////////////////////
// the grey box
////////////////////////////////////////////////////////////////////////////////////////

class MACHINE
{
public:

	MACHINE();
	~MACHINE();

	PDXRESULT Power(const BOOL);
	PDXRESULT Reset(const BOOL);
	PDXRESULT Pause(const BOOL);
	PDXRESULT Execute(IO::GFX* const,IO::SFX* const,IO::INPUT* const);

	PDXRESULT Load    (PDXFILE&);
	PDXRESULT LoadNES (PDXFILE&);
	PDXRESULT LoadFDS (PDXFILE&);
	PDXRESULT LoadNSF (PDXFILE&);
	PDXRESULT LoadNST (PDXFILE&);
	PDXRESULT LoadIPS (PDXFILE&);
	PDXRESULT SaveNST (PDXFILE&) const;
	
	PDXRESULT SaveMovie   (const CHAR* const);
	PDXRESULT LoadMovie   (const CHAR* const);
	PDXRESULT CloseMovie  ();
	PDXRESULT StartMovie  ();
	PDXRESULT StopMovie   ();
	PDXRESULT RewindMovie ();
	
	BOOL IsMoviePlaying()   const;
	BOOL IsMovieRecording() const;

	const PDXSTRING* GetMovieFileName() const;

	VOID Unload();

	BOOL IsPAL       () const;
	BOOL IsOn        () const;
	BOOL IsOff       () const;
	BOOL IsPaused    () const;
	BOOL IsCartridge () const;
	BOOL IsFds       () const;
	BOOL IsNsf       () const;
	BOOL IsVs        () const;

	VOID GetGraphicContext (IO::GFX::CONTEXT&) const;
	VOID SetGraphicContext (const IO::GFX::CONTEXT&);
	
	VOID GetAudioContext (IO::SFX::CONTEXT&) const;
	VOID SetAudioContext (const IO::SFX::CONTEXT&);

	PDXRESULT GetGameGenieContext (IO::GAMEGENIE::CONTEXT&);
	PDXRESULT SetGameGenieContext (const IO::GAMEGENIE::CONTEXT&);

	PDXRESULT GetFdsContext(IO::FDS::CONTEXT&) const;
	PDXRESULT SetFdsContext(const IO::FDS::CONTEXT&);

	PDXRESULT GetNsfContext(IO::NSF::CONTEXT&) const;
	PDXRESULT SetNsfContext(const IO::NSF::CONTEXT&);

	VOID GetGeneralContext(IO::GENERAL::CONTEXT&) const;
	VOID SetGeneralContext(const IO::GENERAL::CONTEXT&);

	const IO::CARTRIDGE::INFO* GetCartridgeInfo() const;

	UINT GetNumVsSystemDipSwitches() const;
	PDXRESULT GetVsSystemDipSwitch(const UINT,IO::DIPSWITCH::CONTEXT&) const;
	PDXRESULT SetVsSystemDipSwitch(const UINT,const IO::DIPSWITCH::CONTEXT&);

	VOID SetMode(MODE);

	VOID AutoSelectController();
	VOID ConnectController(const UINT,const CONTROLLERTYPE);
	CONTROLLERTYPE ConnectedController(const UINT) const;

	BOOL IsAnyControllerConnected(const CONTROLLERTYPE) const;
	BOOL IsAnyControllerConnected(const CONTROLLERTYPE,const CONTROLLERTYPE) const;
	BOOL IsAnyControllerConnected(const CONTROLLERTYPE,const CONTROLLERTYPE,const CONTROLLERTYPE) const;
	BOOL IsAnyControllerConnected(const CONTROLLERTYPE,const CONTROLLERTYPE,const CONTROLLERTYPE,const CONTROLLERTYPE) const;

	enum
	{
		NUM_CONTROLLER_PORTS = 5
	};

	class CONTROLLER
	{
	public:

		CONTROLLER(const UINT n,PPU* const p=NULL)
		: port(n), ppu(p), strobe(0) { Reset(); }

		virtual ~CONTROLLER() {}

		inline VOID SetContext(IO::INPUT* const i,const IO::GFX* const g)
		{
			input = i;
			gfx = g;
		}

		virtual VOID Initialize(const ULONG) {}
		virtual VOID Poll() {}
		virtual VOID Reset() {}

		virtual UINT Peek_4016() { return 0x00; }
		virtual UINT Peek_4017() { return 0x00; }

		virtual VOID Poke_4016 (const UINT) {}
		virtual VOID Poke_4017 (const UINT) {}

		virtual ULONG GetState() const { return 0; }
		virtual VOID SetState(const ULONG) {};

		virtual CONTROLLERTYPE Type() const
		{ return CONTROLLER_UNCONNECTED; }

	protected:

		inline BOOL SetStrobe(UINT data)
		{
			data &= 0x1;
			const BOOL reset = strobe && !data;
			strobe = data;
			return reset;
		}

		UINT strobe;

		const UINT port;
		IO::INPUT* input;
		const IO::GFX* gfx;
		PPU* const ppu;
	};

private:

	VOID ApplyIPS(PDXFILE&);
	PDXRESULT ApplyIPS(PDXFILE&,PDXFILE&);

	VOID InitializeControllers();

	NES_DECL_POKE( 4016 );
	NES_DECL_PEEK( 4016 );
	NES_DECL_POKE( 4017 );					    
	NES_DECL_PEEK( 4017 );

	CPU* const cpu;
	APU* const apu;
	PPU* const ppu;
	CARTRIDGE* cartridge;
	CONTROLLER* controller[4];
	CONTROLLER* expansion;
	VSSYSTEM* VsSystem;
	PALETTE* const palette;
	GAMEGENIE* GameGenie;
	FDS* fds;
	NSF* nsf;
	BOOL on;
	BOOL paused;
	MODE mode;
	IO::GENERAL::CONTEXT GeneralContext;
	PDXSTRING RecentImage;
	MOVIE* movie;
};

#include "NstMachine.inl"

NES_NAMESPACE_END

#endif
