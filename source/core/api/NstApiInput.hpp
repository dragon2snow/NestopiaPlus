////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
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

#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 304 444 )
#elif NST_MSVC >= 1200
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
				NUM_CONTROLLERS = 27
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

				Controllers() throw();

				struct Pad
				{
					enum
					{
						A      = 0x01,
						B      = 0x02,
						SELECT = 0x04,
						START  = 0x08,
						UP     = 0x10,
						DOWN   = 0x20,
						LEFT   = 0x40,
						RIGHT  = 0x80
					};

					enum
					{
						MIC = 0x4
					};

					uint buttons;
					uint mic;
					uint allowSimulAxes;

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
					PowerPad() throw();

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

				struct PowerGlove
				{
					PowerGlove() throw();

					enum
					{
						DISTANCE_IN  =  1,
						DISTANCE_OUT = -1,
						ROLL_LEFT    = -1,
						ROLL_RIGHT   =  1
					};

					enum Gesture
					{
						GESTURE_OPEN   = 0x00,
						GESTURE_FINGER = 0x0F,
						GESTURE_FIST   = 0xFF
					};

					enum
					{
						SELECT = 0x1,
						START  = 0x2
					};

					uchar x;
					uchar y;
					schar distance;
					schar wrist;
					Gesture gesture;
					uint buttons;

					typedef bool (NST_CALLBACK *PollCallback) (void*,PowerGlove&);

					static PollCaller1<PowerGlove> callback;
				};

				struct Mouse
				{
					uint x;
					uint y;
					uint button;

					Mouse()
					: x(0), y(0), button(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,Mouse&);

					static PollCaller1<Mouse> callback;
				};

				struct FamilyTrainer
				{
					FamilyTrainer() throw();

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
					FamilyKeyboard() throw();

					enum
					{
						NUM_PARTS = 9,
						NUM_MODES = 2
					};

					uchar parts[NUM_PARTS+3];

					typedef bool (NST_CALLBACK *PollCallback) (void*,FamilyKeyboard&,uint,uint);

					static PollCaller3<FamilyKeyboard> callback;
				};

				struct SuborKeyboard
				{
					SuborKeyboard() throw();

					enum
					{
						NUM_PARTS = 10,
						NUM_MODES = 2
					};

					uchar parts[NUM_PARTS+2];

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

				struct HoriTrack
				{
					enum
					{
						BUTTON_A      = Pad::A,
						BUTTON_B      = Pad::B,
						BUTTON_SELECT = Pad::SELECT,
						BUTTON_START  = Pad::START,
						BUTTON_UP     = Pad::UP,
						BUTTON_DOWN   = Pad::DOWN,
						BUTTON_LEFT   = Pad::LEFT,
						BUTTON_RIGHT  = Pad::RIGHT
					};

					enum
					{
						MODE_REVERSED = 0x1,
						MODE_LOWSPEED = 0x2
					};

					uint x;
					uint y;
					uint buttons;
					uint mode;

					HoriTrack()
					: x(0), y(0), buttons(0), mode(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,HoriTrack&);

					static PollCaller1<HoriTrack> callback;
				};

				struct Pachinko
				{
					enum
					{
						BUTTON_A      = Pad::A,
						BUTTON_B      = Pad::B,
						BUTTON_SELECT = Pad::SELECT,
						BUTTON_START  = Pad::START,
						BUTTON_UP     = Pad::UP,
						BUTTON_DOWN   = Pad::DOWN,
						BUTTON_LEFT   = Pad::LEFT,
						BUTTON_RIGHT  = Pad::RIGHT
					};

					enum
					{
						MIN_THROTTLE = -64,
						MAX_THROTTLE = +63
					};

					uint buttons;
					int throttle;

					Pachinko()
					: buttons(0), throttle(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,Pachinko&);

					static PollCaller1<Pachinko> callback;
				};

				struct VsSystem
				{
					enum
					{
						COIN_1 = 0x20,
						COIN_2 = 0x40
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

				struct KonamiHyperShot
				{
					enum
					{
						PLAYER1_BUTTON_1 = 0x02,
						PLAYER1_BUTTON_2 = 0x04,
						PLAYER2_BUTTON_1 = 0x08,
						PLAYER2_BUTTON_2 = 0x10
					};

					uint buttons;

					KonamiHyperShot()
					: buttons(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,KonamiHyperShot&);

					static PollCaller1<KonamiHyperShot> callback;
				};

				struct BandaiHyperShot
				{
					uint x;
					uint y;
					uint fire;
					uint move;

					BandaiHyperShot()
					: x(0), y(0), fire(0), move(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,BandaiHyperShot&);

					static PollCaller1<BandaiHyperShot> callback;
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
						PART_3_CHI    = 0x08,
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
						PART_2_RIGHT_JAB  = 0x08,
						PART_2_BODY       = 0x04,
						PART_2_LEFT_JAB   = 0x02,
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

				struct KaraokeStudio
				{
					enum
					{
						A   = 0x1,
						B   = 0x2,
						MIC = 0x4
					};

					uint buttons;

					KaraokeStudio()
					: buttons(0) {}

					typedef bool (NST_CALLBACK *PollCallback) (void*,KaraokeStudio&);

					static PollCaller1<KaraokeStudio> callback;
				};

				Pad pad[NUM_PADS];
				Zapper zapper;
				Paddle paddle;
				PowerPad powerPad;
				PowerGlove powerGlove;
				Mouse mouse;
				FamilyTrainer familyTrainer;
				FamilyKeyboard familyKeyboard;
				SuborKeyboard suborKeyboard;
				DoremikkoKeyboard doremikkoKeyboard;
				HoriTrack horiTrack;
				Pachinko pachinko;
				VsSystem vsSystem;
				OekaKidsTablet oekaKidsTablet;
				KonamiHyperShot konamiHyperShot;
				BandaiHyperShot bandaiHyperShot;
				CrazyClimber crazyClimber;
				Mahjong mahjong;
				ExcitingBoxing excitingBoxing;
				TopRider topRider;
				PokkunMoguraa pokkunMoguraa;
				PartyTap partyTap;
				KaraokeStudio karaokeStudio;
			};
		}
	}

	namespace Api
	{
		class Input : public Base
		{
			struct ControllerCaller;
			struct AdapterCaller;

		public:

			template<typename T>
			Input(T& e)
			: Base(e) {}

			enum
			{
				NUM_CONTROLLERS = Core::Input::NUM_CONTROLLERS,
				NUM_PADS = Core::Input::NUM_PADS
			};

			enum Adapter
			{
				ADAPTER_NES,
				ADAPTER_FAMICOM
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
				POWERGLOVE,
				MOUSE,
				ROB,
				FAMILYTRAINER,
				FAMILYKEYBOARD,
				SUBORKEYBOARD,
				DOREMIKKOKEYBOARD,
				HORITRACK,
				PACHINKO,
				OEKAKIDSTABLET,
				KONAMIHYPERSHOT,
				BANDAIHYPERSHOT,
				CRAZYCLIMBER,
				MAHJONG,
				EXCITINGBOXING,
				TOPRIDER,
				POKKUNMOGURAA,
				PARTYTAP,
				TURBOFILE,
				BARCODEWORLD
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

			Result AutoSelectController(uint) throw();
			void AutoSelectControllers() throw();
			Result AutoSelectAdapter() throw();

			Result ConnectController(uint,Type) throw();
			Result ConnectAdapter(Adapter) throw();

			Type GetConnectedController(uint) const throw();
			Adapter GetConnectedAdapter() const throw();

			bool IsControllerConnected(Type) const throw();

			typedef void (NST_CALLBACK *ControllerCallback) (UserData,uint,Type);
			typedef void (NST_CALLBACK *AdapterCallback) (UserData,Adapter);

			static ControllerCaller controllerCallback;
			static AdapterCaller adapterCallback;
		};

		struct Input::ControllerCaller : Core::UserCallback<Input::ControllerCallback>
		{
			void operator () (uint port,Type type) const
			{
				if (function)
					function( userdata, port, type );
			}
		};

		struct Input::AdapterCaller : Core::UserCallback<Input::AdapterCallback>
		{
			void operator () (Adapter adapter) const
			{
				if (function)
					function( userdata, adapter );
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
