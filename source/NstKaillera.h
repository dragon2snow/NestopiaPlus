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

#ifndef NST_KAILLERA_H
#define NST_KAILLERA_H

#include "paradox/PdxMap.h"
#include "kaillera/kailleraclient.h"
#include "windows/NstChatWindow.h"
#include "windows/NstMenu.h"
#include "NstNes.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class KAILLERA
{
public:

	KAILLERA();
	~KAILLERA();

	typedef const PDXARRAY<PDXSTRING> FILELIST;

	inline BOOL IsSupported() const
	{ return hDLL != NULL; }

	inline BOOL IsConnected() const
	{ return connected; }

	VOID Update(NES::IO::INPUT&);
	VOID Launch(FILELIST&,const BOOL,const BOOL);

private:

	typedef PDXMAP<const CHAR*,PDXSTRING> GAMELIST;
	typedef PDXARRAY<HWND> KAILLERAWINDOWS;

	struct KAILLERAERROR
	{
		enum
		{
			LOADGAME = 1,
			GENERIC  = 2
		};

		inline KAILLERAERROR(const INT c)
		: code(c) {}

		INT code;
	};

	struct SETTINGS
	{
		enum {MENUITEMS=0x10};

		SETTINGS()
		:
		WinProc         (0),
		NoStatusBar     (false),
		SwitchedScreen  (false),
		RunInBackground (-1),
		ConfirmReset    (-1)
		{
            for (UINT i=0; i < MENUITEMS; ++i)
				MenuItems[i] = -1;
		}

		LONG_PTR WinProc;
		bool NoStatusBar;
		bool SwitchedScreen;
		CHAR RunInBackground;
		CHAR ConfirmReset;
		CHAR MenuItems[MENUITEMS];
		NSTMENU menu;
	};

	VOID MainLoop        ();
	VOID PushKaillera    (KAILLERAWINDOWS&);
	VOID PopKaillera     (const KAILLERAWINDOWS&);
	VOID PushSettings    (SETTINGS&,const INT);
	VOID PopSettings     (SETTINGS&,const INT);
	VOID PushAccelerator (const INT);
	VOID PopAccelerator  ();
	VOID UpdateGameList  (FILELIST&,const BOOL);
	VOID InitInput       (const INT,const INT);
	VOID LoadGame        (const CHAR* const);
	VOID Unconnect       ();

	VOID OnPacketCommand(NES::IO::INPUT&);
	VOID OnToggleMenu();
	VOID OnInsertCoin(const UINT);
	BOOL OnCommand(const WPARAM);

	static VOID FindDatabaseName(const PDXSTRING&,PDXSTRING&);

	template<class FPTR> 
	BOOL ImportFunction(FPTR&,const CHAR* const) const;

	static VOID FindKailleraWindows(PDXARRAY<HWND>&);

	static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
	static LRESULT CALLBACK GetMsgProc(INT,WPARAM,LPARAM);

	LRESULT MsgProc(const HWND,const UINT,const WPARAM,const LPARAM);

	INT StartNetPlay(CHAR*,const INT,const INT);

	static INT  WINAPI GameProc(CHAR*,INT,INT);
	static VOID WINAPI ChatRecievedProc(CHAR*,CHAR*);
	static VOID WINAPI ClientDroppedProc(CHAR*,INT);

	typedef INT (WINAPI *KAILLERA_GETVERSION)(CHAR*);
	typedef INT (WINAPI *KAILLERA_INIT)();
	typedef INT (WINAPI *KAILERRA_SHUTDOWN)();
	typedef INT (WINAPI *KAILLERA_SETINFOS)(kailleraInfos*);
	typedef INT (WINAPI *KAILLERA_SELECTSERVERDIALOG)(HWND);
	typedef INT (WINAPI *KAILLERA_MODIFYPLAYVALUES)(VOID*,INT);
	typedef INT (WINAPI *KAILLERA_CHATSEND)(CHAR*);
	typedef INT (WINAPI *KAILLERA_ENDGAME)();

	struct PACKET
	{
		inline PACKET()
		{ data[0] = data[1] = 0; }

		enum
		{
			SIZE                = sizeof(U8) * 2,
			INPUT               = b11111111,
			COUNTER             = b01111111,
			COMMAND_MODE        = b10000000,
			COMMAND             = b00001111,
			COMMAND_DATA        = b11110000,
			COMMAND_DATA_SHIFT  = 4,
			COMMAND_RESET       = 1,
			COMMAND_PAUSE       = 2,
			COMMAND_INSERT_DISK	= 3,
			COMMAND_EJECT_DISK  = 4,
			COMMAND_DISK_SIDE   = 5,
			COMMAND_INSERT_COIN = 6
		};

		U8 data[2];
	};

	typedef UINT (*GET_INPUT)(const NES::IO::INPUT&);
	typedef VOID (*SET_INPUT)(NES::IO::INPUT&,const UINT);

	static UINT GetPad0     (const NES::IO::INPUT&);
	static UINT GetPad1     (const NES::IO::INPUT&);
	static UINT GetPad2     (const NES::IO::INPUT&);
	static UINT GetPad3     (const NES::IO::INPUT&);
	static UINT GetZapper   (const NES::IO::INPUT&);
	static UINT GetPaddle   (const NES::IO::INPUT&);
	static UINT GetPowerPad (const NES::IO::INPUT&);
	static UINT GetNone     (const NES::IO::INPUT&);

	static VOID SetPad0     (NES::IO::INPUT&,const UINT);
	static VOID SetPad1     (NES::IO::INPUT&,const UINT);
	static VOID SetPad2     (NES::IO::INPUT&,const UINT);
	static VOID SetPad3     (NES::IO::INPUT&,const UINT);
	static VOID SetZapper   (NES::IO::INPUT&,const UINT);
	static VOID SetPaddle   (NES::IO::INPUT&,const UINT);
	static VOID SetPowerPad (NES::IO::INPUT&,const UINT);
	static VOID SetNone     (NES::IO::INPUT&,const UINT);

	static VOID ChatSendProc(PDXSTRING&);

	HHOOK hHook;
	HACCEL hAccel;

	GAMELIST GameList;
	CHATWINDOW ChatWindow;
	NSTMENU menu;

	BOOL connected;
	BOOL PlayFullscreen;
	UINT NumPlayers;
	UINT UpdateCounter;
	BOOL command;

	PACKET packets[8];

	GET_INPUT GetInput;
	SET_INPUT SetInput[4];

	HMODULE hDLL;
	KAILLERA_GETVERSION         kailleraGetVersion;
	KAILLERA_INIT               kailleraInit;
	KAILERRA_SHUTDOWN           kailleraShutdown;
	KAILLERA_SETINFOS           kailleraSetInfos;
	KAILLERA_SELECTSERVERDIALOG kailleraSelectServerDialog;
	KAILLERA_MODIFYPLAYVALUES   kailleraModifyPlayValues;
	KAILLERA_CHATSEND           kailleraChatSend;
	KAILLERA_ENDGAME            kailleraEndGame;
};

#endif
