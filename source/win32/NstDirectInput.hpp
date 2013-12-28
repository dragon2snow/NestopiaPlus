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

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif

#include <MMSystem.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace Nestopia
{
	namespace DirectX
	{
		class DirectInput
		{
			class Keyboard;
			class Joystick;

		public:

			DirectInput(HWND);
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

			void  Acquire();
			void  Unacquire();
			void  Update(uint=UINT_MAX-1);
			ibool Poll();
			void  StaticPoll();
			void  Build(const Key*,uint,uint);
			ibool MapKey(Key&,tstring,const System::Guid* = NULL,uint=0) const;
			const HeapString GetKeyName(const Key&) const;

			void BeginScanMode(HWND) const;
			ScanResult ScanKey(Key&,ScanMode=SCAN_MODE_ALL) const;
			void EndScanMode() const;

			class Key
			{
				friend class DirectInput;
				friend class Keyboard;
				friend class Joystick;

			public:

				ibool MapVirtualKey(uint,uint,uint,uint);
				ibool MapVirtualKey(GenericString);
				ibool GetVirtualKey(ACCEL&) const;
				ibool IsVirtualKey() const;
				ibool GetToggle(ibool&) const;

			private:

				union
				{
					const BYTE* data;
					DWORD vKey;
				};

				uint (*code)(const void* const);

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

				ibool Assigned() const
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

			class Notifier
			{
				void Run();

				enum
				{
					EVENT_INPUT,
					EVENT_STOP,
					EVENT_ENTER
				};

				HANDLE events[3];
				volatile uint timeInput;
				uint signal;
				volatile uint signaled;

			public:

				Notifier();
				~Notifier();

				HANDLE Start(bool);
				void   Stop();

				inline ibool Running() const;
				inline ibool Update(uint=UINT_MAX-1);
				inline uint  Flush();
				inline void  Reset();
				inline ibool Signaling() const;
			};

			class Device
			{
			public:

				void Enable(ibool,ibool);

			protected:

				Device(IDirectInputDevice8&);
				~Device();

				ibool Acquire(void*,uint);
				void  Unacquire();

				inline ibool Update(uint);
				inline ibool Poll();
				inline uint  Flush();
				inline void  Reset();
				inline void  StaticPoll() const;
				inline ibool Signaling() const;

				IDirectInputDevice8& com;

			private:

				ibool mustPoll;
				Notifier notifier;
			};

			class Keyboard : public Device
			{
			public:

				Keyboard(Base&);
				~Keyboard();

				enum
				{
					MAX_KEYS = NUM_KEYBOARD_KEYS,
				};

				void Acquire();
				void Unacquire();

				NST_FORCE_INLINE void Update(uint);
				NST_FORCE_INLINE uint Poll();

				ibool Map(Key&,tstring) const;
				ibool Map(Key&,uint) const;

				void BeginScanMode(HWND) const;
				ibool Scan(u8 (&)[MAX_KEYS]) const;
				ScanResult Scan(Key&) const;
				void EndScanMode() const;

				ibool Assigned(const Key&) const;
				tstring GetName(const Key&) const;

			private:

				enum
				{
					COOPERATIVE_FLAGS = DISCL_FOREGROUND|DISCL_NONEXCLUSIVE|DISCL_NOWINKEY,
					SCAN_COOPERATIVE_FLAGS = DISCL_BACKGROUND|DISCL_NONEXCLUSIVE
				};

				static BOOL CALLBACK EnumObjects(LPCDIDEVICEOBJECTINSTANCE,LPVOID);
				static IDirectInputDevice8& Create(IDirectInput8&);

				void SetCooperativeLevel(HWND,DWORD) const;
				void Clear();
				void GetState();

				NST_NO_INLINE void OnError(HRESULT);

				typedef Object::Heap<BYTE,MAX_KEYS> Buffer;

				Buffer buffer;
				HWND const hWnd;

				static HeapString keyNames[MAX_KEYS];

			public:

				const u8* GetBuffer() const
				{
					return buffer;
				}
			};

			class Joystick : public Device
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

				void Acquire();
				void Unacquire();
				void StaticPoll();

				NST_FORCE_INLINE void Update(uint);
				NST_FORCE_INLINE uint Poll();

				ibool Map(Key&,tstring) const;

				void BeginScanMode() const;
				ibool Scan(Key&) const;
				void EndScanMode() const;

				ibool Assigned(const Key&) const;
				ibool SetAxisDeadZone(uint);
				tstring GetName(const Key&) const;

			private:

				static IDirectInputDevice8& Create(Base&,const GUID&);

				void Clear();
				void GetState();

				NST_NO_INLINE void OnError(HRESULT);

				class Caps
				{
					struct Context
					{
						Context(Caps&,IDirectInputDevice8&);

						Caps& caps;
						IDirectInputDevice8& com;
						uint numButtons;
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
					long lX;
					long lY;
					long lZ;
					long lRx;
					long lRy;
					long lRz;
					ibool must;

				public:

					Calibrator();

					void Reset(DIJOYSTATE&);

					inline void Fix(DIJOYSTATE&) const;
				};

				const Caps caps;
				Object::Pod<DIJOYSTATE> state;
				mutable Calibrator calibrator;
				ibool scanEnabled;
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

				void ScanEnable(ibool enable)
				{
					scanEnabled = enable;
				}

				ibool ScanEnabled() const
				{
					return scanEnabled;
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
			};

			typedef Collection::Vector<Joystick> Joysticks;

			static BOOL CALLBACK EnumJoysticks(LPCDIDEVICEINSTANCE,LPVOID);

			static uint KeyDown     (const void* const);
			static uint KeyNegDir   (const void* const);
			static uint KeyPosDir   (const void* const);
			static uint KeyPovUp    (const void* const);
			static uint KeyPovRight (const void* const);
			static uint KeyPovDown  (const void* const);
			static uint KeyPovLeft  (const void* const);
			static uint KeyNone     (const void* const);

			Base base;
			Keyboard keyboard;
			Joysticks joysticks;

		public:

			ibool MapKeyboard(Key& key,uint code) const
			{
				key.Unmap();
				return keyboard.Map( key, code );
			}

			uint NumJoysticks() const
			{
				return joysticks.Size();
			}

			ibool JoystickScanEnabled(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].ScanEnabled();
			}

			void ScanEnableJoystick(uint index,ibool enable)
			{
				NST_ASSERT( index < joysticks.Size() );
				joysticks[index].ScanEnable( enable );
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

			ibool AnyPressed() const
			{
				Key key;
				return ScanKey( key ) != SCAN_NO_KEY;
			}
		};
	}
}

#endif
