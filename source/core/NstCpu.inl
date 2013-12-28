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

inline APU& CPU::GetAPU()
{ 
	return apu; 
}

inline const APU& CPU::GetAPU() const
{ 
	return apu; 
}
						
////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::DoNMI()
{
	if (IntEn & NMI)
		DoNMISR();
}

inline VOID CPU::DoIRQ(const UINT line)
{
	IntLow |= (IntEn & line);
}

inline VOID CPU::ClearIRQ(const UINT line)
{
	IntLow &= ~line;
}

inline VOID CPU::TryIRQ()
{
	IntLow |= IRQ_TMP;
}

inline VOID CPU::SetLine(const UINT line,const BOOL state)
{
	if (state)
	{
		IntEn |= line;
	}
	else
	{
		IntEn &= ~line;
		IntLow &= ~line;
	}
}

inline BOOL CPU::IsIRQ(const UINT line) const
{
	return IntLow & line;
}

inline BOOL CPU::IsLine(const UINT line) const
{
	return IntEn & line;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::ResetCycles()
{
	cycles = 0;
	FrameCounter = LONG_MAX;
	DmcCounter = LONG_MAX;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::SetFrameCycles(const ULONG count)
{
	FrameCycles = count;
}

inline VOID CPU::AdvanceCycles(const ULONG count)
{ 
	cycles += count;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<CPU::CYCLETYPE TYPE>
inline ULONG CPU::GetFrameCycles<TYPE>() const
{ 
	return FrameCycles; 
}

template<>
inline ULONG CPU::GetFrameCycles<CPU::CYCLE_NTSC>() const
{ 
	return FrameCycles / NES_CPU_NTSC_FIXED; 
}

template<>
inline ULONG CPU::GetFrameCycles<CPU::CYCLE_PAL>() const
{ 
	return FrameCycles / NES_CPU_PAL_FIXED; 
}

template<>
inline ULONG CPU::GetFrameCycles<CPU::CYCLE_AUTO>() const
{ 
	return FrameCycles / (pal ? NES_CPU_PAL_FIXED : NES_CPU_NTSC_FIXED); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<CPU::CYCLETYPE TYPE>
inline ULONG CPU::GetCycles<TYPE>() const
{ 
	return cycles; 
}

template<>
inline ULONG CPU::GetCycles<CPU::CYCLE_NTSC>() const
{ 
	return cycles / NES_CPU_NTSC_FIXED; 
}

template<>
inline ULONG CPU::GetCycles<CPU::CYCLE_PAL>() const
{ 
	return cycles / NES_CPU_PAL_FIXED; 
}

template<>
inline ULONG CPU::GetCycles<CPU::CYCLE_AUTO>() const
{ 
	return cycles / (pal ? NES_CPU_PAL_FIXED : NES_CPU_NTSC_FIXED); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::SetDmcLengthCounter(const UINT count)
{
	DmcLengthCounter = count;
}

inline VOID CPU::SetDmcCounter(const LONG count)
{
	DmcCounter = cycles + count;
}

inline VOID CPU::DisableDmcCounter()
{
	DmcCounter = LONG_MAX;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::Peek(const UINT address)
{ 
	return map.Peek(address);
}

inline VOID CPU::Poke(const UINT address,const UINT data)
{
	return map.Poke(address,data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class OBJECT,class READER,class WRITER> 
inline VOID CPU::SetPort(const UINT address,OBJECT* object,READER reader,WRITER writer)
{
	map.SetPort(address,object,reader,writer);
}

template<class OBJECT,class READER,class WRITER> 
inline VOID CPU::SetPort(const UINT first,const UINT last,OBJECT* object,READER reader,WRITER writer)
{
	map.SetPort(first,last,object,reader,writer);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline CPU::PORT& CPU::GetPort(const UINT address)
{
	return map.GetPort(address);
}

inline const CPU::PORT& CPU::GetPort(const UINT address) const
{
	return map.GetPort(address);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class OBJECT,class FUNCTION>
VOID CPU::SetEvent(OBJECT* object,FUNCTION function)
{
	if (PDX::Find(events.Begin(),events.End(),function) == events.End())
	{
		events.InsertBack(EVENT(object,function));
		events.Defrag();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class ANY>
VOID CPU::RemoveEvent(ANY any)
{
	EVENTS::ITERATOR iterator = PDX::Find(events.Begin(),events.End(),any);

	if (iterator != events.End())
	{
		events.Erase(iterator);
		events.Defrag();
	}
}
