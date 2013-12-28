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
			void Unacquire() const;
			void Optimize(const Key*,uint);
			ibool MapKey(Key&,cstring,const System::Guid* = NULL,uint=0) const;

			ScanResult ScanKey(Key&) const;
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

				uint (*code)(const void*) throw();

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

			typedef ComInterface<IDirectInputDevice8> Device;

			class Manager : public ComInterface<IDirectInput8>
			{
			public:

				explicit Manager(HWND);

			private:

				HWND const hWnd;

			public:

				HWND Window() const
				{
					return hWnd;
				}
			};

			class Keyboard : Device
			{
			public:

				explicit Keyboard(const Manager&);
				~Keyboard();

				enum 
				{
					MAX_KEYS = 256,
					COOPERATIVE_FLAGS = DISCL_FOREGROUND|DISCL_NONEXCLUSIVE|DISCL_NOWINKEY
				};

				void Acquire(ibool=FALSE);
				ibool Map(Key&,uint) const;
				ScanResult Scan(Key&) const;
				void SetCooperativeLevel(HWND,DWORD=COOPERATIVE_FLAGS) const;
				ibool IsAssigned(const Key&) const;
				cstring GetName(const Key&) const;

			private:

				void Clear();

				typedef Object::Heap<BYTE,MAX_KEYS> Buffer;

				ibool enabled;
				ibool mustPoll;
				Buffer buffer;

			public:

				void Unacquire() const
				{
					pointer->Unacquire();
				}

				const BYTE* GetBuffer() const
				{
					return buffer;
				}

				void Enable(ibool enable=TRUE)
				{
					enabled = enable;
				}

				void Poll(ibool force=FALSE)
				{
					if (enabled | force)
					{	
						if (mustPoll)
							pointer->Poll();

						if (SUCCEEDED(pointer->GetDeviceState( Buffer::SIZE, buffer )))
							return;

						Acquire();
					}
				}
			};

			class Joystick : Device
			{
			public:

				Joystick(const Manager&,const GUID&);
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

				void Acquire(ibool=FALSE);
				ibool Map(Key&,cstring) const;
				ibool Scan(Key&) const;
				ibool IsAssigned(const Key&) const;
				cstring GetName(const Key&) const;

			private:

				static Device Create(const Manager&,const GUID&);

				NST_NO_INLINE void OnError(HRESULT);
				void Clear();

				enum 
				{
					TABLE_KEYS = 32,
					AXIS_MIN_RANGE = -32768,
					AXIS_MAX_RANGE = +32767,
					AXIS_DEADZONE = 5000
				};

				class Caps
				{
				public:

					Caps(const Device&,const GUID&);

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
						NUM_POVS      = 4,
						NUM_BUTTONS   = 32
					};

					ibool mustPoll;
					const System::Guid guid;
					ushort numButtons;
					ushort axis;

				private:

					static BOOL CALLBACK EnumDeviceObjectsProc(LPCDIDEVICEOBJECTINSTANCE,LPVOID);
				};

				ibool enabled;
				Object::Pod<DIJOYSTATE> state;
				const Caps caps;

				struct Lookup;
				static const Lookup table[TABLE_KEYS];

			public:

				const System::Guid& Guid() const
				{
					return caps.guid;
				}

				void Unacquire() const
				{
					pointer->Unacquire();
				}

				void Enable(ibool enable=TRUE)
				{
					enabled = enable;
				}

				void Poll(ibool force=FALSE)
				{
					if (enabled | force)
					{
						HRESULT hResult;

						if (caps.mustPoll)
						{
							hResult = pointer->Poll();

							if (FAILED(hResult))
								goto hell;
						}

						hResult = pointer->GetDeviceState( sizeof(state), &state );

						if (SUCCEEDED(hResult))
							return;

					hell:

						OnError( hResult );
					}
				}
			};

			typedef Collection::Vector< Object::Stack<Joystick> > Joysticks;

			static BOOL CALLBACK EnumJoysticks(LPCDIDEVICEINSTANCE,LPVOID);

			static uint KeyDown     (const void*) throw(); 
			static uint KeyNegDir   (const void*) throw();
			static uint KeyPosDir   (const void*) throw();        
			static uint KeyPovUp    (const void*) throw();
			static uint KeyPovRight (const void*) throw();
			static uint KeyPovDown  (const void*) throw();
			static uint KeyPovLeft  (const void*) throw();
			static uint KeyNone     (const void*) throw(); 

			Manager manager;
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

			void EndScanMode() const
			{
				Unacquire();
				keyboard.SetCooperativeLevel( manager.Window() );
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
				return joysticks[index]->Guid();
			}

			ibool IsAnyPressed() const
			{
				Key key;
				return ScanKey( key ) != SCAN_NO_KEY;
			}
		};
	}
}

#endif
