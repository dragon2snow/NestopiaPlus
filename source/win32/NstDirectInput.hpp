////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#ifndef NST_DIRECTX_DIRECTINPUT_H
#define NST_DIRECTX_DIRECTINPUT_H

#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include "NstDirectX.hpp"
#include "NstObjectPod.hpp"
#include "NstObjectHeap.hpp"
#include "NstObjectStack.hpp"
#include "NstCollectionVector.hpp"
#include <dinput.h>

namespace Nestopia
{
	namespace DirectX
	{
		class DirectInput
		{  
			class Keyboard;
			class Joystick;

		public:

			explicit DirectInput(HWND);
			~DirectInput();

			enum 
			{			
				MAX_JOYSTICKS = 32
			};

			enum
			{
				AXIS_X        = 0x001,
				AXIS_Y        = 0x002,
				AXIS_Z        = 0x004,
				AXIS_RX       = 0x008,
				AXIS_RY       = 0x010,
				AXIS_RZ       = 0x020,
				AXIS_SLIDER_0 = 0x040,
				AXIS_SLIDER_1 = 0x080,
				AXIS_POV_0	  = 0x100,
				AXIS_POV_1	  = 0x200,
				AXIS_POV_2	  = 0x400,
				AXIS_POV_3	  = 0x800,
				AXIS_ALL      = 0xFFF
			};

			enum ScanResult
			{
				SCAN_INVALID_KEY = -1,
				SCAN_NO_KEY,
				SCAN_GOOD_KEY
			};

			enum PollChoice
			{
				POLL_OPTIMIZED,
				POLL_ALLDEVICES
			};

			class Key;
			typedef String::Stack<64> KeyName;

			void Poll(PollChoice=POLL_ALLDEVICES);
			void Acquire(ibool=FALSE);
			void Unacquire();
			void Optimize(const Key*,uint);
			ibool MapKey(Key&,cstring,const System::Guid* = NULL,uint=0) const;

			ScanResult ScanKey(Key&,uint=AXIS_ALL) const;
			KeyName GetKeyName(const Key&) const;

			class Key
			{
				friend class DirectInput;
				friend class Keyboard;
				friend class Joystick;

			public:

				ibool MapVirtualKey(uint,uint,uint,uint);
				ibool MapVirtualKey(cstring);
				ibool GetVirtualKey(ACCEL&) const;

			private:

				union
				{
					const BYTE* data;
					DWORD vKey;
				};

				uint (*code)(const void* const) throw();

			public:

				Key()
				: data(NULL), code(KeyNone) {}

				ibool operator == (const Key& key) const
				{
					return data == key.data && code == key.code;
				}

				void Unmap()
				{
					data = NULL;
					code = KeyNone;
				}

				ibool IsAssigned() const
				{
					return data != NULL;
				}

				uint GetState() const
				{
					return code( data );
				}

				template<typename Value>
				void GetState(Value& value,uint mask) const
				{
					value |= code( data ) & mask;
				}
			};

		private:

			class Base
			{
				static IDirectInput8& Create();

			public:

				Base(HWND);
				~Base();

				IDirectInput8& com;
				HWND const hWnd;
			};

			class Keyboard
			{
			public:
				
				Keyboard(Base&);
				~Keyboard();

				enum 
				{
					MAX_KEYS = 256,
					COOPERATIVE_FLAGS = DISCL_FOREGROUND|DISCL_NONEXCLUSIVE|DISCL_NOWINKEY
				};

				NST_NO_INLINE void Acquire(ibool=FALSE);
				NST_FORCE_INLINE void Poll(ibool=FALSE);
				void Unacquire();

				ibool Map(Key&,uint) const;
				ScanResult Scan(Key&) const;
				void SetCooperativeLevel(HWND,DWORD=COOPERATIVE_FLAGS) const;
				ibool IsAssigned(const Key&) const;
				cstring GetName(const Key&) const;

				inline void Enable(ibool=TRUE);

			private:

				static IDirectInputDevice8& Create(IDirectInput8&);
				static ibool MustPoll(IDirectInputDevice8&);

				void Clear();

				typedef Object::Heap<BYTE,MAX_KEYS> Buffer;

				ibool enabled;
				IDirectInputDevice8& com;
				const ibool mustPoll;
				Buffer buffer;

			public:

				const BYTE* GetBuffer() const
				{
					return buffer;
				}
			};

			class Joystick
			{
			public:

				Joystick(Base&,const GUID&);
				~Joystick();

				enum
				{
					POV_CENTER    = 0xFFFF,
					POV_UPRIGHT   =  45 * DI_DEGREES,
					POV_DOWNRIGHT = 135 * DI_DEGREES,
					POV_DOWNLEFT  =	225 * DI_DEGREES,
					POV_UPLEFT    = 315 * DI_DEGREES
				};

				enum Exception 
				{
					ERR_API
				};

				NST_NO_INLINE void Acquire(ibool=FALSE);
				NST_FORCE_INLINE void Poll(ibool=FALSE);
				void Unacquire();

				ibool Map(Key&,cstring) const;
				ibool Scan(Key&,uint) const;
				ibool IsAssigned(const Key&) const;
				cstring GetName(const Key&) const;

				inline void Enable(ibool=TRUE);

			private:

				static IDirectInputDevice8& Create(Base&,const GUID&);

				NST_NO_INLINE void OnError(HRESULT);
				void Clear();

				class Caps
				{
					struct Context
					{
						Caps& caps;
						IDirectInputDevice8& com;
						uint numButtons;

						Context(Caps& c,IDirectInputDevice8& d)
						: caps(c), com(d), numButtons(0) {}
					};

					enum
					{
						AXIS_MIN_RANGE = -1000,
						AXIS_MAX_RANGE = +1000,
						AXIS_DEADZONE  = 5000
					};

					static BOOL CALLBACK EnumObjectsProc(LPCDIDEVICEOBJECTINSTANCE,LPVOID);

				public:

					Caps(IDirectInputDevice8&,const GUID&);

					enum
					{
						NUM_POVS    = 4,
						NUM_BUTTONS = 32
					};

					ibool mustPoll;
					uint axes;
					const System::Guid guid;
				};

				ibool enabled;
				IDirectInputDevice8& com;
				const Caps caps;
				Object::Pod<DIJOYSTATE> state;

				enum 
				{
					TABLE_KEYS = 32
				};

				struct Lookup;
				static const Lookup table[TABLE_KEYS];

			public:

				const System::Guid& GetGuid() const
				{
					return caps.guid;
				}
			};

			typedef Collection::Vector< Object::Stack<Joystick> > Joysticks;

			static BOOL CALLBACK EnumJoysticks(LPCDIDEVICEINSTANCE,LPVOID);

			static uint KeyDown     (const void* const) throw(); 
			static uint KeyNegDir   (const void* const) throw();
			static uint KeyPosDir   (const void* const) throw();        
			static uint KeyPovUp    (const void* const) throw();
			static uint KeyPovRight (const void* const) throw();
			static uint KeyPovDown  (const void* const) throw();
			static uint KeyPovLeft  (const void* const) throw();
			static uint KeyNone     (const void* const) throw(); 

			Base base;
			Keyboard keyboard;
			Joysticks joysticks;

		public:

			enum
			{
				NUM_KEYBOARD_KEYS = Keyboard::MAX_KEYS
			};

			void BeginScanMode(HWND hWnd)
			{
				keyboard.SetCooperativeLevel( hWnd, DISCL_FOREGROUND|DISCL_EXCLUSIVE );
				Acquire( TRUE );
			}

			void EndScanMode()
			{
				Unacquire();
				keyboard.SetCooperativeLevel( base.hWnd );
			}

			const BYTE* GetKeyboardBuffer() const
			{
				return keyboard.GetBuffer();
			}

			ibool MapKeyboard(Key& key,uint code) const
			{
				key.Unmap();
				return keyboard.Map( key, code );
			}

			uint NumJoysticks() const
			{
				return joysticks.Size();
			}

			const System::Guid& GetJoystickGuid(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index]->GetGuid();
			}

			ibool IsAnyPressed(uint axes=AXIS_ALL) const
			{
				Key key;
				return ScanKey( key, axes ) != SCAN_NO_KEY;
			}
		};
	}
}

#endif
