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

#ifndef NST_API_INPUT_H
#define NST_API_INPUT_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstApi.hpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			enum
			{
				NUM_PADS = 4,
				NUM_CONTROLLERS = 19
			};

			class Controllers
			{
				template<typename T> 
				struct PollCaller1 : UserCallback<typename T::PollCallback>
				{
					bool operator () (T& t) const
					{
						return this->function ? this->function( this->userdata, t ) : true;
					}
				};

				template<typename T> 
				struct PollCaller2 : UserCallback<typename T::PollCallback>
				{
					bool operator () (T& t,uint a) const
					{
						return this->function ? this->function( this->userdata, t, a ) : true;
					}
				};

				template<typename T> 
				struct PollCaller3 : UserCallback<typename T::PollCallback>
				{
					bool operator () (T& t,uint a,uint b) const
					{
						return this->function ? this->function( this->userdata, t, a, b ) : true;
					}
				};

			public:

				Controllers();

				struct Pad
				{
					enum
					{
						A      = b00000001,
						B      = b00000010,
						SELECT = b00000100,
						START  = b00001000,
						UP     = b00010000,
						DOWN   = b00100000,
						LEFT   = b01000000,
						RIGHT  = b10000000
					};

					enum
					{
						MIC = 0x4
					};

					uint buttons;
					uint mic;
					bool allowSimulAxes;

					Pad()
					: buttons(0), mic(0), allowSimulAxes(false) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,Pad&,uint);

					static PollCaller2<Pad> callback;
				};

				struct Zapper
				{
					uint x;
					uint y;
					uint fire;

					Zapper()
					: x(0), y(0), fire(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,Zapper&);

					static PollCaller1<Zapper> callback;
				};

				struct Paddle
				{
					uint x;
					uint button;

					Paddle()
					: x(0), button(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,Paddle&);

					static PollCaller1<Paddle> callback;
				};

				struct PowerPad
				{
					PowerPad();

					enum 
					{
						NUM_SIDE_A_BUTTONS = 12,
						NUM_SIDE_B_BUTTONS = 8
					};

					bool sideA[NUM_SIDE_A_BUTTONS];
					bool sideB[NUM_SIDE_B_BUTTONS];

					typedef bool (NST_CALLBACK *PollCallback) (void*,PowerPad&);

					static PollCaller1<PowerPad> callback;
				};

				struct FamilyTrainer
				{
					FamilyTrainer();

					enum 
					{
						NUM_SIDE_A_BUTTONS = 12,
						NUM_SIDE_B_BUTTONS = 8
					};

					bool sideA[NUM_SIDE_A_BUTTONS];
					bool sideB[NUM_SIDE_B_BUTTONS];

					typedef bool (NST_CALLBACK *PollCallback) (void*,FamilyTrainer&);

					static PollCaller1<FamilyTrainer> callback;
				};

				struct FamilyKeyboard
				{
					FamilyKeyboard();

					enum 
					{
						NUM_PARTS = 9,
						NUM_MODES = 2
					};

					uchar parts[NUM_PARTS];

					typedef bool (NST_CALLBACK *PollCallback) (void*,FamilyKeyboard&,uint,uint);

					static PollCaller3<FamilyKeyboard> callback;
				};

				struct SuborKeyboard
				{
					SuborKeyboard();

					enum 
					{
						NUM_PARTS = 10,
						NUM_MODES = 2
					};

					uchar parts[NUM_PARTS];

					typedef bool (NST_CALLBACK *PollCallback) (void*,SuborKeyboard&,uint,uint);

					static PollCaller3<SuborKeyboard> callback;
				};

				struct DoremikkoKeyboard
				{
					enum
					{
						PART_1   = 1,
						PART_2   = 2,
						PART_3   = 3,
						PART_4   = 4,
						PART_5   = 5,
						PART_6   = 6,
						PART_7   = 7,
						MODE_A   = 0,
						MODE_A_0 = 0x02,
						MODE_A_1 = 0x04,
						MODE_A_2 = 0x08,
						MODE_A_3 = 0x10,
						MODE_B   = 1,
						MODE_B_0 = 0x02,
						MODE_B_1 = 0x04
					};

					uint keys;

					DoremikkoKeyboard()
					: keys(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,DoremikkoKeyboard&,uint,uint);

					static PollCaller3<DoremikkoKeyboard> callback;
				};

				struct VsSystem
				{
					enum
					{
						COIN_1 = b00100000,
						COIN_2 = b01000000
					};

					uint insertCoin;

					VsSystem()
					: insertCoin(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,VsSystem&);

					static PollCaller1<VsSystem> callback;
				};

				struct OekaKidsTablet
				{
					uint x;
					uint y;
					uint button;

					OekaKidsTablet()
					: x(0), y(0), button(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,OekaKidsTablet&);

					static PollCaller1<OekaKidsTablet> callback;
				};

				struct HyperShot
				{
					enum
					{
						PLAYER1_BUTTON_1 = b00000010,
						PLAYER1_BUTTON_2 = b00000100,
						PLAYER2_BUTTON_1 = b00001000,
						PLAYER2_BUTTON_2 = b00010000
					};

					uint buttons;

					HyperShot()
					: buttons(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,HyperShot&);

					static PollCaller1<HyperShot> callback;
				};

				struct CrazyClimber
				{
					enum
					{
						RIGHT  = 0x10,
						LEFT   = 0x20,
						UP     = 0x40,
						DOWN   = 0x80
					};

					uint left;
					uint right;

					CrazyClimber()
					: left(0), right(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,CrazyClimber&);

					static PollCaller1<CrazyClimber> callback;
				};

				struct Mahjong
				{
					enum 
					{
						PART_1        = 0x02,
						PART_1_I      = 0x80,
						PART_1_J      = 0x40,
						PART_1_K      = 0x20,
						PART_1_L      = 0x10,
						PART_1_M      = 0x08,
						PART_1_N      = 0x04,
						PART_2        = 0x04,
						PART_2_A      = 0x80,
						PART_2_B      = 0x40,
						PART_2_C      = 0x20,
						PART_2_D      = 0x10,
						PART_2_E      = 0x08,
						PART_2_F      = 0x04,
						PART_2_G      = 0x02,
						PART_2_H      = 0x01,
						PART_3        = 0x06,
						PART_3_START  = 0x80,
						PART_3_SELECT = 0x40,
						PART_3_KAN    = 0x20,
						PART_3_PON    = 0x10,
						PART_3_CHII   = 0x08,
						PART_3_REACH  = 0x04,
						PART_3_RON    = 0x02,
						NUM_PARTS     = 3
					};

					uint buttons;

					Mahjong()
					: buttons(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,Mahjong&,uint);

					static PollCaller2<Mahjong> callback;
				};

				struct ExcitingBoxing
				{
					enum
					{
						PART_1            = 0x00,
						PART_1_RIGHT_HOOK = 0x10,
						PART_1_LEFT_MOVE  = 0x08,
						PART_1_RIGHT_MOVE = 0x04,
						PART_1_LEFT_HOOK  = 0x02,
						PART_2            = 0x02,
						PART_2_STRAIGHT   = 0x10,
						PART_2_RIGHT_JABB = 0x08,
						PART_2_BODY       = 0x04,
						PART_2_LEFT_JABB  = 0x02,
						NUM_PARTS         = 2
					};

					uint buttons;

					ExcitingBoxing()
					: buttons(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,ExcitingBoxing&,uint);

					static PollCaller2<ExcitingBoxing> callback;
				};

				struct TopRider
				{
					enum
					{
						BRAKE       = 0x01,
						ACCEL       = 0x02,
						SELECT      = 0x04,
						START       = 0x08,
						SHIFT_GEAR  = 0x10,
						REAR        = 0x20,
						STEER_LEFT  = 0x40,
						STEER_RIGHT = 0x80
					};

					uint buttons;

					TopRider()
					: buttons(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,TopRider&);

					static PollCaller1<TopRider> callback;
				};

				struct PokkunMoguraa
				{
					enum
					{
						ROW_1    = 0x04,
						ROW_2    = 0x02,
						ROW_3    = 0x01,
						NUM_ROWS = 3,
						BUTTON_1 = 0x02,
						BUTTON_2 = 0x04,
						BUTTON_3 = 0x08,
						BUTTON_4 = 0x10
					};

					uint buttons;

					PokkunMoguraa()
					: buttons(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,PokkunMoguraa&,uint);

					static PollCaller2<PokkunMoguraa> callback;
				};

				struct PartyTap
				{
					enum
					{
						UNIT_1 = 0x04,
						UNIT_2 = 0x08,
						UNIT_3 = 0x10,
						UNIT_4 = 0x20,
						UNIT_5 = 0x40,
						UNIT_6 = 0x80
					};

					uint units;

					PartyTap()
					: units(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,PartyTap&);

					static PollCaller1<PartyTap> callback;
				};

				Pad pad[NUM_PADS];
				Zapper zapper;
				Paddle paddle;
				PowerPad powerPad;
				FamilyTrainer familyTrainer;
				FamilyKeyboard familyKeyboard;
				SuborKeyboard suborKeyboard;
				DoremikkoKeyboard doremikkoKeyboard;
				VsSystem vsSystem;
				OekaKidsTablet oekaKidsTablet;
				HyperShot hyperShot;
				CrazyClimber crazyClimber;
				Mahjong mahjong;
				ExcitingBoxing excitingBoxing;
				TopRider topRider;
				PokkunMoguraa pokkunMoguraa;
				PartyTap partyTap;
			};
		}
	}

	namespace Api
	{
		class Input : public Base
		{
		public:
	
			Input(Emulator& e)
			: Base(e) {}
	
			enum
			{
				NUM_CONTROLLERS = Core::Input::NUM_CONTROLLERS,
				NUM_PADS = Core::Input::NUM_PADS
			};

			enum Type
			{
				UNCONNECTED,
				PAD1,       
				PAD2,       
				PAD3,       
				PAD4,
				ZAPPER,     
				PADDLE,     
				POWERPAD,
				FAMILYTRAINER,
				FAMILYKEYBOARD,
				SUBORKEYBOARD,
				DOREMIKKOKEYBOARD,
				OEKAKIDSTABLET,
				HYPERSHOT,
				CRAZYCLIMBER,
				MAHJONG,
				EXCITINGBOXING,
				TOPRIDER,
				POKKUNMOGURAA,
				PARTYTAP
			};

			enum
			{
				PORT_1,
				PORT_2,
				PORT_3,
				PORT_4,
				EXPANSION_PORT,
				NUM_PORTS
			};
	
			typedef Core::Input::Controllers Controllers;

			Result AutoSelectController(uint);
	
			Result ConnectController(uint,Type);
			Type GetConnectedController(uint) const;
	
			bool IsAnyControllerConnected(Type) const;
			bool IsAnyControllerConnected(Type,Type) const;
			bool IsAnyControllerConnected(Type,Type,Type) const;
			bool IsAnyControllerConnected(Type,Type,Type,Type) const;
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
