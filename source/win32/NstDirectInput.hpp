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

#ifndef NST_DIRECTX_DIRECTINPUT_H
#define NST_DIRECTX_DIRECTINPUT_H

#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include "NstDirectX.hpp"
#include "NstObjectPod.hpp"
#include "NstObjectHeap.hpp"
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
				MAX_JOYSTICKS     = 32,
				NUM_KEYBOARD_KEYS = 256,
				AXIS_X            = 0x001,
				AXIS_Y            = 0x002,
				AXIS_Z            = 0x004,
				AXIS_RX           = 0x008,
				AXIS_RY           = 0x010,
				AXIS_RZ           = 0x020,
				AXIS_SLIDER_0     = 0x040,
				AXIS_SLIDER_1     = 0x080,
				AXIS_POV_0        = 0x100,
				AXIS_POV_1        = 0x200,
				AXIS_POV_2        = 0x400,
				AXIS_POV_3        = 0x800,
				AXIS_ALL          = 0xFFF,
				DEADZONE_MAX      = 100,
				DEFAULT_AXES      = AXIS_X|AXIS_Y|AXIS_Z|AXIS_RX|AXIS_RY|AXIS_RZ|AXIS_POV_0|AXIS_POV_1|AXIS_POV_2|AXIS_POV_3,
				DEFAULT_DEADZONE  = 50
			};

			enum ScanMode
			{
				SCAN_MODE_ALL,
				SCAN_MODE_JOY
			};

			enum ScanResult
			{
				SCAN_INVALID_KEY = -1,
				SCAN_NO_KEY,
				SCAN_GOOD_KEY
			};

			class Key;

			void Acquire();
			void Unacquire();
			void Optimize(const Key*,uint);
			ibool MapKey(Key&,tstring,const System::Guid* = NULL,uint=0) const;
			const HeapString GetKeyName(const Key&) const;

			void BeginScanMode(HWND);
			ScanResult ScanKey(Key&,ScanMode=SCAN_MODE_ALL);
			void EndScanMode();

			class Key
			{
				friend class DirectInput;
				friend class Keyboard;
				friend class Joystick;

			public:

				ibool MapVirtKey(uint,uint,uint,uint);
				ibool MapVirtKey(GenericString);
				ibool GetVirtKey(ACCEL&) const;
				ibool IsVirtKey() const;
				ibool GetToggle(ibool&) const;

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
					MAX_KEYS = NUM_KEYBOARD_KEYS,
					COOPERATIVE_FLAGS = DISCL_FOREGROUND|DISCL_NONEXCLUSIVE|DISCL_NOWINKEY
				};

				NST_NO_INLINE void Acquire();
				void Unacquire();

				ibool Map(Key&,tstring) const;
				ibool Map(Key&,uint) const;
				ibool Scan(u8 (&)[MAX_KEYS]);
				ScanResult Scan(Key&);
				void SetCooperativeLevel(HWND,DWORD=COOPERATIVE_FLAGS) const;
				ibool IsAssigned(const Key&) const;
				tstring GetName(const Key&) const;
				inline void Use(ibool);
				inline ibool InUse() const;

			private:

				static BOOL CALLBACK EnumObjects(LPCDIDEVICEOBJECTINSTANCE,LPVOID);
				static IDirectInputDevice8& Create(IDirectInput8&);

				void Clear();

				typedef Object::Heap<BYTE,MAX_KEYS> Buffer;

				ibool inUse;
				IDirectInputDevice8& com;
				Buffer buffer;

				static HeapString keyNames[MAX_KEYS];

			public:

				const u8* GetBuffer() const
				{
					return buffer;
				}

				void Poll()
				{
					if (inUse && (FAILED(com.Poll()) || FAILED(com.GetDeviceState( Buffer::SIZE, buffer ))))
						Acquire();
				}
			};

			class Joystick
			{
			public:

				Joystick(Base&,const DIDEVICEINSTANCE&);
				~Joystick();

				enum
				{
					POV_CENTER    = 0xFFFF,
					POV_UPRIGHT   =  45 * DI_DEGREES,
					POV_DOWNRIGHT = 135 * DI_DEGREES,
					POV_DOWNLEFT  = 225 * DI_DEGREES,
					POV_UPLEFT    = 315 * DI_DEGREES
				};

				enum Exception
				{
					ERR_API
				};

				NST_NO_INLINE void Acquire();
				void Unacquire();

				ibool Map(Key&,tstring) const;
				ibool Scan(Key&);
				ibool IsAssigned(const Key&) const;
				ibool SetAxisDeadZone(uint);
				tstring GetName(const Key&) const;
				inline void Use(ibool);
				inline ibool InUse() const;

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
						AXIS_MAX_RANGE = +1000
					};

					static BOOL CALLBACK EnumObjectsProc(LPCDIDEVICEOBJECTINSTANCE,LPVOID);

				public:

					Caps(IDirectInputDevice8&,const DIDEVICEINSTANCE&);

					enum
					{
						NUM_POVS    = 4,
						NUM_BUTTONS = 32
					};

					uint axes;
					const System::Guid guid;
					const HeapString name;
				};

				class Calibrator
				{
				public:

					Calibrator();

					void Reset(DIJOYSTATE&);

				private:

					ibool must;
					long lX;
					long lY;
					long lZ;
					long lRx;
					long lRy;
					long lRz;

				public:

					ibool Must() const
					{
						return must;
					}

					void Fix(DIJOYSTATE& state) const
					{
						state.lX -= lX;
						state.lY -= lY;
						state.lZ -= lZ;
						state.lRx -= lRx;
						state.lRy -= lRy;
						state.lRz -= lRz;
					}
				};

				ibool enabled;
				ibool inUse;
				IDirectInputDevice8& com;
				const Caps caps;
				Object::Pod<DIJOYSTATE> state;
				Calibrator calibrator;
				uint deadZone;
				uint axes;

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

				const HeapString& GetName() const
				{
					return caps.name;
				}

				uint GetAxisDeadZone() const
				{
					return deadZone;
				}

				uint GetAvailableAxes() const
				{
					return caps.axes;
				}

				uint GetScannerAxes() const
				{
					return axes;
				}

				void Enable(ibool enable)
				{
					enabled = enable;
				}

				ibool IsEnabled() const
				{
					return enabled;
				}

				void SetScannerAxes(uint flags)
				{
					NST_ASSERT( flags <= AXIS_ALL );
					axes = flags;
				}

				void SetScannerAxes(uint flags,ibool on)
				{
					NST_ASSERT( flags <= AXIS_ALL );
					axes = (on ? axes | flags : axes & ~flags);
				}

				void Poll()
				{
					if (inUse)
					{
						HRESULT hResult;

						if (SUCCEEDED(hResult=com.Poll()) && SUCCEEDED(hResult=com.GetDeviceState( sizeof(state), &state )))
							calibrator.Fix( state );
						else
							OnError( hResult );
					}
				}
			};

			typedef Collection::Vector<Joystick> Joysticks;

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

			ibool ScanKeyboard(u8 (&buffer)[Keyboard::MAX_KEYS])
			{
				return keyboard.Scan( buffer );
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

			ibool IsJoystickEnabled(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].IsEnabled();
			}

			void EnableJoystick(uint index,ibool enable)
			{
				NST_ASSERT( index < joysticks.Size() );
				joysticks[index].Enable( enable );
			}

			const System::Guid& GetJoystickGuid(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].GetGuid();
			}

			const HeapString& GetJoystickName(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].GetName();
			}

			ibool SetAxisDeadZone(uint index,uint deadZone)
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].SetAxisDeadZone( deadZone );
			}

			uint GetAxisDeadZone(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].GetAxisDeadZone();
			}

			uint GetAvailableAxes(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].GetAvailableAxes();
			}

			void SetScannerAxes(uint index,uint axes)
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].SetScannerAxes( axes );
			}

			void SetScannerAxes(uint index,uint axes,ibool state)
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].SetScannerAxes( axes, state );
			}

			uint GetScannerAxes(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].GetScannerAxes();
			}

			const u8* GetKeyboardBuffer() const
			{
				return keyboard.GetBuffer();
			}

			ibool IsAnyPressed()
			{
				Key key;
				return ScanKey( key ) != SCAN_NO_KEY;
			}

			void PollJoysticks()
			{
				for (Joysticks::Iterator it=joysticks.Begin(), end=joysticks.End(); it != end; ++it)
					it->Poll();
			}

			void Poll()
			{
				keyboard.Poll();
				PollJoysticks();
			}
		};
	}
}

#endif
