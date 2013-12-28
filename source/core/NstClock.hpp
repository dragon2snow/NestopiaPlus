////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#ifndef NST_CLOCK_H
#define NST_CLOCK_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstCpu.hpp"
#include "NstPpu.hpp"

namespace Nes
{
	namespace Core
	{
		class Cpu;
		class Ppu;

		namespace Clock
		{
			template<typename Unit,uint Divider=1U>
			class M2
			{
			public:

				M2(Cpu&);

				template<typename Param>
				M2(Cpu&,Param&);

				template<typename Param>
				M2(Cpu&,const Param&);

				enum
				{
					IRQ_DELAY_CYCLES = 2
				};

				void Reset(bool,bool);
				void VSync();

			private:

				NES_DECL_HOOK( Signaled )

				Cycle count;
				Cpu& cpu;
				ibool enabled;

			public:

				Unit unit;

				ibool EnableLine(ibool enable)
				{
					return enabled = enable;
				}

				ibool IsLineEnabled() const
				{
					return enabled;
				}

				void Update()
				{
					NES_CALL_HOOK(M2,Signaled);
				}

				void ClearIRQ()	const
				{
					cpu.ClearIRQ();
				}
			};

			template<typename Unit,uint Divider>
			M2<Unit,Divider>::M2(Cpu& c)
			: count(0), cpu(c), enabled(false)
			{
			}

			template<typename Unit,uint Divider>
			template<typename Param>
			M2<Unit,Divider>::M2(Cpu& c,Param& p)
			: count(0), cpu(c), enabled(false), unit(p)
			{
			}

			template<typename Unit,uint Divider>
			template<typename Param>
			M2<Unit,Divider>::M2(Cpu& c,const Param& p)
			: count(0), cpu(c), enabled(false), unit(p)
			{
			}

			template<typename Unit,uint Divider>
			void M2<Unit,Divider>::Reset(const bool hard,const bool enable)
			{
				enabled = enable;
				count = 0;
				unit.Reset( hard );
				cpu.AddHook( Hook(this,&M2::Hook_Signaled) );
			}

			template<typename Unit,uint Divider>
            #define NES_M2_FUNC_T M2<Unit,Divider> // template comma vs. macro comma
			NES_HOOK(NES_M2_FUNC_T,Signaled)
            #undef NES_M2_FUNC_T
			{
				NST_COMPILE_ASSERT( Divider <= 8 );

				while (count < cpu.GetMasterClockCycles())
				{
					if (enabled && unit.Signal())
						cpu.DoIRQ( Cpu::IRQ_EXT, count + cpu.GetMasterClockCycle(IRQ_DELAY_CYCLES) );

					count += cpu.GetMasterClockCycle( Divider );
				}
			}

			template<typename Unit,uint Divider>
			void M2<Unit,Divider>::VSync()
			{
				count = (count > cpu.GetMasterClockFrameCycles() ? count - cpu.GetMasterClockFrameCycles() : 0);
			}

			template<typename Unit>
			class A12
			{
			public:

				enum IrqDelay
				{
					NO_IRQ_DELAY,
					IRQ_DELAY
				};

				enum
				{
					IRQ_DELAY_CYCLES = 2
				};

				A12(Cpu&,Ppu&,uint=0,IrqDelay=NO_IRQ_DELAY);

				template<typename Param>
				A12(Cpu&,Ppu&,uint,IrqDelay,Param&);

				void Reset(bool,bool);
				void VSync();

			private:

				NES_DECL_LINE( Signaled )

				struct Base
				{
					NST_COMPILE_ASSERT( MODE_NTSC == 0 && MODE_PAL == 1 );

					Base(uint);
					Cycle clock[2];
				};

				Cycle count;
				Cycle duration;
				Cycle delay;
				Cpu& cpu;
				Ppu& ppu;

			public:

				Unit unit;

			private:

				const Base base;

			public:

				void EnableLine(const ibool enable)
				{
					if (enable)
						ppu.ConnectA12( this, &A12::Line_Signaled );
					else
						ppu.DisconnectA12();
				}

				bool IsLineEnabled() const
				{
					return ppu.IsA12Connected();
				}

				void Update() const
				{
					ppu.Update();
				}

				void ClearIRQ() const
				{
					cpu.ClearIRQ();
				}
			};

			template<typename Unit>
			A12<Unit>::Base::Base(uint d)
			{
				clock[MODE_NTSC] = d * Ppu::MC_DIV_NTSC;
				clock[MODE_PAL] = d * Ppu::MC_DIV_PAL;
			}

			template<typename Unit>
			A12<Unit>::A12(Cpu& c,Ppu& p,uint b,IrqDelay d)
			: count(0), delay(d), cpu(c), ppu(p), base(b) 
			{
				VSync();
			}
											 
			template<typename Unit> template<typename Param>
			A12<Unit>::A12(Cpu& c,Ppu& p,uint b,IrqDelay d,Param& a)
			: count(0), delay(d), cpu(c), ppu(p), unit(a), base(b) 
			{
				VSync();
			}

			template<typename Unit>
			void A12<Unit>::Reset(const bool hard,const bool enable)
			{
				count = 0;
				VSync();
				unit.Reset( hard );
				EnableLine( enable );
				ppu.EnableCpuSynchronization();
			}

			template<typename Unit>
			NES_LINE(A12<Unit>,Signaled)
			{
				const Cycle target = count;
				count = cycle + duration;

				if (cycle >= target && unit.Signal())
					cpu.DoIRQ( Cpu::IRQ_EXT, cycle + delay );
			}

			template<typename Unit>
			void A12<Unit>::VSync()
			{
				count = (count > cpu.GetMasterClockFrameCycles() ? count - cpu.GetMasterClockFrameCycles() : 0);
				duration = base.clock[cpu.GetMode()];

				if (delay)
					delay = cpu.GetMasterClockCycle( IRQ_DELAY_CYCLES );
			}
		}
	}
}

#endif
