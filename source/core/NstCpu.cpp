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

#include <ctime>
#include "../paradox/PdxFile.h"
#include "NstTypes.h"
#include "NstCpu.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// processor registers/pointers/flags
////////////////////////////////////////////////////////////////////////////////////////

#undef PCL
#undef PCH
#undef PCW
#undef PCD

#define PCL pc.b.l
#define PCH pc.b.h
#define PCW pc.w.l
#define PCD pc.d

#undef A
#undef X
#undef Y
#undef T
#undef SP

#define A a.b.l
#define X x.b.l
#define Y y.b.l
#define T t.b.l
#define SP sp.b.l

#undef AD 
#undef XD 
#undef YD 
#undef TD 
#undef SPD

#define AD a.d
#define XD x.d
#define YD y.d
#define TD t.d
#define SPD	sp.d

#undef EAL
#undef EAH
#undef EAW
#undef EAD

#define EAL ea.b.l
#define EAH ea.b.h
#define EAW ea.w.l
#define EAD ea.d

#undef FN
#undef FZ
#undef FC
#undef FI
#undef FV
#undef FD

#define FN flags.n.b.l
#define FZ flags.z.b.l
#define FC flags.c.b.l
#define FI flags.i.b.l
#define FV flags.v.b.l
#define FD flags.d.b.l

#undef FND
#undef FZD
#undef FCD
#undef FID
#undef FVD
#undef FDD

#define FND flags.n.d
#define FZD flags.z.d
#define FCD flags.c.d
#define FID flags.i.d
#define FVD flags.v.d
#define FDD flags.d.d

////////////////////////////////////////////////////////////////////////////////////////
// helper macros
////////////////////////////////////////////////////////////////////////////////////////

#undef CPU_PUSH             
#undef CPU_PULL              
#undef CPU_FNZ              		 
#undef CPU_READ_ZPG_BYTE    
#undef CPU_READ_ZPG_WORD    
#undef CPU_READ_BYTE         
#undef CPU_READ_WORD        
#undef CPU_WRITE_BYTE
#undef CPU_WRITE_ZPG     
#undef CPU_READ_PCB             
#undef CPU_READ_PCW    
#undef CPU_EAT_CYCLES

#define CPU_PUSH(v)          ram[STACK_OFFSET + SP--] = (v)
#define CPU_PULL()           ram[STACK_OFFSET + ++SP]
#define CPU_FNZ(v)           FN=FZ=(v)
#define CPU_FNZD(v)          FND=FZD=(v)
#define CPU_READ_ZPG_BYTE(v) (cache = ram[v])
#define CPU_READ_ZPG_WORD(v) (ram[v] + ((cache = ram[((v)+1) & 0xFF]) << 8))
#define CPU_READ_BYTE(v)	 (cache = map[v])
#define CPU_READ_WORD(v)	 ((cache = map[v]) + ((cache = map[(v)+1]) << 8))
#define CPU_WRITE_BYTE(v,w)  map.Poke(v,w)
#define CPU_WRITE_ZPG(v,w)   ram[v] = (w)
#define CPU_READ_PCB()       (cache = map[PCD++])
#define CPU_READ_PCW()       ((cache = map[PCD]) + ((cache = map[(++PCD)++]) << 8))
#define CPU_EAT_CYCLES(n)    cycles += CycleTable[pal][(n)-1]

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const ULONG CPU::CycleTable[2][8] =
{
	{
		NES_CPU_TO_NTSC(1),
		NES_CPU_TO_NTSC(2),
		NES_CPU_TO_NTSC(3),
		NES_CPU_TO_NTSC(4),
		NES_CPU_TO_NTSC(5),
		NES_CPU_TO_NTSC(6),
		NES_CPU_TO_NTSC(7),
		NES_CPU_TO_NTSC(8)
	},
	{
		NES_CPU_TO_PAL(1),
		NES_CPU_TO_PAL(2),
		NES_CPU_TO_PAL(3),
		NES_CPU_TO_PAL(4),
		NES_CPU_TO_PAL(5),
		NES_CPU_TO_PAL(6),
		NES_CPU_TO_PAL(7),
		NES_CPU_TO_PAL(8)
	}
};

////////////////////////////////////////////////////////////////////////////////////////
// opcode function pointer table
////////////////////////////////////////////////////////////////////////////////////////

const CPU::INSTRUCTION CPU::instructions[0x100] =
{
	op0x00, op0x01, op0x02, op0x03, op0x04, op0x05, op0x06, op0x07,
	op0x08, op0x09, op0x0A, op0x0B, op0x0C, op0x0D, op0x0E, op0x0F,
	op0x10, op0x11, op0x12, op0x13, op0x14, op0x15, op0x16, op0x17,
	op0x18, op0x19, op0x1A, op0x1B, op0x1C, op0x1D, op0x1E, op0x1F,
	op0x20, op0x21, op0x22, op0x23, op0x24, op0x25, op0x26, op0x27,
	op0x28, op0x29, op0x2A, op0x2B, op0x2C, op0x2D, op0x2E, op0x2F,
	op0x30, op0x31, op0x32, op0x33, op0x34, op0x35, op0x36, op0x37,
	op0x38, op0x39, op0x3A, op0x3B, op0x3C, op0x3D, op0x3E, op0x3F,
	op0x40, op0x41, op0x42, op0x43, op0x44, op0x45, op0x46, op0x47,
	op0x48, op0x49, op0x4A, op0x4B, op0x4C, op0x4D, op0x4E, op0x4F,
	op0x50, op0x51, op0x52, op0x53, op0x54, op0x55, op0x56, op0x57,
	op0x58, op0x59, op0x5A, op0x5B, op0x5C, op0x5D, op0x5E, op0x5F,
	op0x60, op0x61, op0x62, op0x63, op0x64, op0x65, op0x66, op0x67,
	op0x68, op0x69, op0x6A, op0x6B, op0x6C, op0x6D, op0x6E, op0x6F,
	op0x70, op0x71, op0x72, op0x73, op0x74, op0x75, op0x76, op0x77,
	op0x78, op0x79, op0x7A, op0x7B, op0x7C, op0x7D, op0x7E, op0x7F,
	op0x80, op0x81, op0x82, op0x83, op0x84, op0x85, op0x86, op0x87,
	op0x88, op0x89, op0x8A, op0x8B, op0x8C, op0x8D, op0x8E, op0x8F,
	op0x90, op0x91, op0x92, op0x93, op0x94, op0x95, op0x96, op0x97,
	op0x98, op0x99, op0x9A, op0x9B, op0x9C, op0x9D, op0x9E, op0x9F,
	op0xA0, op0xA1, op0xA2, op0xA3, op0xA4, op0xA5, op0xA6, op0xA7,
	op0xA8, op0xA9, op0xAA, op0xAB, op0xAC, op0xAD, op0xAE, op0xAF,
	op0xB0, op0xB1, op0xB2, op0xB3, op0xB4, op0xB5, op0xB6, op0xB7,
	op0xB8, op0xB9, op0xBA, op0xBB, op0xBC, op0xBD, op0xBE, op0xBF,
	op0xC0, op0xC1, op0xC2, op0xC3, op0xC4, op0xC5, op0xC6, op0xC7,
	op0xC8, op0xC9, op0xCA, op0xCB, op0xCC, op0xCD, op0xCE, op0xCF,
	op0xD0, op0xD1, op0xD2, op0xD3, op0xD4, op0xD5, op0xD6, op0xD7,
	op0xD8, op0xD9, op0xDA, op0xDB, op0xDC, op0xDD, op0xDE, op0xDF,
	op0xE0, op0xE1, op0xE2, op0xE3, op0xE4, op0xE5, op0xE6, op0xE7,
	op0xE8, op0xE9, op0xEA, op0xEB, op0xEC, op0xED, op0xEE, op0xEF,
	op0xF0, op0xF1, op0xF2, op0xF3, op0xF4, op0xF5, op0xF6, op0xF7,
	op0xF8, op0xF9, op0xFA, op0xFB, op0xFC, op0xFD, op0xFE, op0xFF
};

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

CPU::CPU()
:
FrameCycles (0),
pal         (0),
apu         (*this)
{
	ResetPorts();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::ResetPorts()
{
	map.SetPort( 0x0000, 0xFFFF, this, Peek_nop, Poke_nop );
	map.SetPort( 0x4015, this, Peek_4015, Poke_4015 );
	map.SetPort( 0xFFFC, this, Peek_jam_1, Poke_nop );
	map.SetPort( 0xFFFD, this, Peek_jam_2, Poke_nop );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::SetMode(const MODE mode)
{
	cycles = 0;
	pal = (mode == MODE_PAL ? 1 : 0);
	apu.SetMode( mode );
}

////////////////////////////////////////////////////////////////////////////////////////
// ram memory read/write
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(CPU,ram)
{
	return ram[address & 0x7FF];
}

NES_POKE(CPU,ram)
{
	ram[address & 0x7FF] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
// invalid memory read/write - do nothing
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(CPU,nop)
{
}

NES_PEEK(CPU,nop)
{ 
	return cache; 
}

NES_PEEK(CPU,jam_1)
{
	--PCW;
	return 0xFC;
}

NES_PEEK(CPU,jam_2)
{
	return 0xFF;
}

////////////////////////////////////////////////////////////////////////////////////////
// Accumulator "addressing"
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::Acc()
{ 
	CPU_EAT_CYCLES(2);
	return AD;
}

////////////////////////////////////////////////////////////////////////////////////////
// Immediate addressing
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::rImm() 
{ 
	const UINT data = CPU_READ_PCB();
	CPU_EAT_CYCLES(2);

	return data;
}

////////////////////////////////////////////////////////////////////////////////////////
// Absolute addressing
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::rAbs() 
{ 
    const UINT data = CPU_READ_PCW();	
	CPU_EAT_CYCLES(4);

	return CPU_READ_BYTE(data);
}

inline UINT CPU::rwAbs(UINT& address) 
{ 
    const UINT tmp = CPU_READ_PCW();
    CPU_EAT_CYCLES(4);

	address = tmp;
    const UINT data = CPU_READ_BYTE(tmp);

	CPU_EAT_CYCLES(1);
	CPU_WRITE_BYTE(tmp,data);
	CPU_EAT_CYCLES(1);

	return data;
}

inline UINT CPU::wAbs() 
{ 
    const UINT address = CPU_READ_PCW();
    CPU_EAT_CYCLES(4);

	return address;
}

////////////////////////////////////////////////////////////////////////////////////////
// Zero page addressing
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::rZpg() 
{ 
	UINT data = CPU_READ_PCB();
	data = CPU_READ_ZPG_BYTE(data);  

	CPU_EAT_CYCLES(3);

	return data;
}

inline UINT CPU::rwZpg(UINT& address)
{
	address = CPU_READ_PCB();
	const UINT data = CPU_READ_ZPG_BYTE(address);  
	
	CPU_EAT_CYCLES(5);

	return data;
}

inline UINT CPU::wZpg()
{
	const UINT address = CPU_READ_PCB();	
	CPU_EAT_CYCLES(3);

	return address;
}

////////////////////////////////////////////////////////////////////////////////////////
// Zero page indexed addressing (X)
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::rZpgX() 
{ 
    UINT data = (XD + CPU_READ_PCB()) & 0xFF;
	data = CPU_READ_ZPG_BYTE(data);  

	CPU_EAT_CYCLES(4);

	return data;
}

inline UINT CPU::rwZpgX(UINT& address) 
{
	UINT data = (XD + CPU_READ_PCB()) & 0xFF;
	address = data;

	data = CPU_READ_ZPG_BYTE(data);  
	CPU_EAT_CYCLES(6);

	return data;
}

inline UINT CPU::wZpgX() 
{ 
	const UINT data = (XD + CPU_READ_PCB()) & 0xFF;
	CPU_EAT_CYCLES(4);

	return data;
}

////////////////////////////////////////////////////////////////////////////////////////
// Zero page indexed addressing (Y)
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::rZpgY() 
{ 
    UINT data = (YD + CPU_READ_PCB()) & 0xFF;
	data = CPU_READ_ZPG_BYTE(data);  

	CPU_EAT_CYCLES(4);

	return data;
}

inline UINT CPU::rwZpgY(UINT& address) 
{ 
	UINT data = (YD + CPU_READ_PCB()) & 0xFF;
	address = data;

	data = CPU_READ_ZPG_BYTE(data);  
	CPU_EAT_CYCLES(6);

	return data;
}

inline UINT CPU::wZpgY() 
{ 
	const UINT data = (YD + CPU_READ_PCB()) & 0xFF;
	CPU_EAT_CYCLES(4);

	return data;
}

////////////////////////////////////////////////////////////////////////////////////////
// Absolute indexed addressing (X)
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::rAbsX() 
{ 
	PDXWORD address;

	address.d = CPU_READ_PCW();
	address.b.l += XD;

	CPU_EAT_CYCLES(4);
	UINT data = CPU_READ_BYTE(address.d);      

	if (XD > address.b.l)
	{
		++address.b.h;

		CPU_EAT_CYCLES(1);
		data = CPU_READ_BYTE(address.d);
	}

	return data;
}

inline UINT CPU::rwAbsX(UINT& address) 
{
	PDXWORD tmp;

	tmp.d = CPU_READ_PCW();
	tmp.b.l += XD;

	CPU_EAT_CYCLES(4);
	CPU_READ_BYTE(tmp.d);
	CPU_EAT_CYCLES(1);

	if (XD > tmp.b.l) 
		++tmp.b.h;
	
	address = tmp.d;
	const UINT data = CPU_READ_BYTE(tmp.d);

	CPU_EAT_CYCLES(1);
	CPU_WRITE_BYTE(tmp.d,data);
	CPU_EAT_CYCLES(1);

	return data;
}

inline UINT CPU::wAbsX() 
{
	PDXWORD address;

	address.d = CPU_READ_PCW();
	address.b.l += XD;

	CPU_EAT_CYCLES(4);
	CPU_READ_BYTE(address.d);
	CPU_EAT_CYCLES(1);

	if (XD > address.b.l) 
		++address.b.h;
	
	return address.d;
}

////////////////////////////////////////////////////////////////////////////////////////
// Absolute indexed addressing (Y)
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::rAbsY() 
{ 
	PDXWORD address;

	address.d = CPU_READ_PCW();
	address.b.l += YD;

	CPU_EAT_CYCLES(4);
	UINT data = CPU_READ_BYTE(address.d);      

	if (YD > address.b.l)
	{
		++address.b.h;

		CPU_EAT_CYCLES(1);
		data = CPU_READ_BYTE(address.d);
	}

	return data;
}

inline UINT CPU::rwAbsY(UINT& address) 
{
	PDXWORD tmp;

	tmp.d = CPU_READ_PCW();
	tmp.b.l += YD;

	CPU_EAT_CYCLES(4);
	CPU_READ_BYTE(tmp.d);
	CPU_EAT_CYCLES(1);

	if (YD > tmp.b.l) 
		++tmp.b.h;
	
	address = tmp.d;
	const UINT data = CPU_READ_BYTE(tmp.d);

	CPU_EAT_CYCLES(1);
	CPU_WRITE_BYTE(tmp.d,data);
	CPU_EAT_CYCLES(1);

	return data;
}

inline UINT CPU::wAbsY() 
{
	PDXWORD address;

	address.d = CPU_READ_PCW();
	address.b.l += YD;

	CPU_EAT_CYCLES(4);
	CPU_READ_BYTE(address.d);
	CPU_EAT_CYCLES(1);

	if (YD > address.b.l) 
		++address.b.h;
	
	return address.d;
}

////////////////////////////////////////////////////////////////////////////////////////
// Indexed indirect addressing
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::rIndX() 
{ 
	UINT data = (XD + CPU_READ_PCB()) & 0xFF;
	data = CPU_READ_ZPG_WORD(data);

	CPU_EAT_CYCLES(6);

	return CPU_READ_BYTE(data);      
}

inline UINT CPU::rwIndX(UINT& address) 
{ 
	UINT tmp = (XD + CPU_READ_PCB()) & 0xFF;
	tmp = CPU_READ_ZPG_WORD(tmp);

	CPU_EAT_CYCLES(6);

	address = tmp;
	const UINT data = CPU_READ_BYTE(tmp);

	CPU_EAT_CYCLES(1);
	CPU_WRITE_BYTE(tmp,data);
	CPU_EAT_CYCLES(1);

	return data;
}

inline UINT CPU::wIndX() 
{ 
	UINT data = (XD + CPU_READ_PCB()) & 0xFF;
	data = CPU_READ_ZPG_WORD(data);

	CPU_EAT_CYCLES(6);

	return data;
}

////////////////////////////////////////////////////////////////////////////////////////
// Indirect indexed addressing
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::rIndY() 
{ 
	PDXWORD address;

	address.d = CPU_READ_PCB();
	address.d = CPU_READ_ZPG_WORD(address.d);
	address.b.l += YD;

	CPU_EAT_CYCLES(5);
	UINT data = CPU_READ_BYTE(address.d);      

	if (YD > address.b.l)
	{
		++address.b.h;

		CPU_EAT_CYCLES(1);
		data = CPU_READ_BYTE(address.d);
	}

	return data;
}

inline UINT CPU::rwIndY(UINT& address) 
{ 
	PDXWORD tmp;

	tmp.d = CPU_READ_PCB();
	tmp.d = CPU_READ_ZPG_WORD(tmp.d);
	tmp.b.l += YD;

	CPU_EAT_CYCLES(5);
	CPU_READ_BYTE(tmp.d);      
	CPU_EAT_CYCLES(1);

	if (YD > tmp.b.l)
		++tmp.b.h;

	address = tmp.d;
	const UINT data = CPU_READ_BYTE(tmp.d);

	CPU_EAT_CYCLES(1);
	CPU_WRITE_BYTE(tmp.d,data);
	CPU_EAT_CYCLES(1);

	return data;
}

inline UINT CPU::wIndY() 
{ 
	PDXWORD address;

	address.d = CPU_READ_PCB();
	address.d = CPU_READ_ZPG_WORD(address.d);
	address.b.l += YD;

	CPU_EAT_CYCLES(5);
	CPU_READ_BYTE(address.d);      
	CPU_EAT_CYCLES(1);

	if (YD > address.b.l)
		++address.b.h;

	return address.d;
}

////////////////////////////////////////////////////////////////////////////////////////
// relative addressing
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::Branch(const UINT taken)
{
	UINT count;

	if (taken)
	{
		PDXWORD tmp;
		tmp.d = CPU_READ_PCB();
		tmp.w.l = PCW + PDX_CAST_REF(I8,tmp.b.l);
		count = (PCH == tmp.b.h ? 3 : 4);
		PCD = tmp.d;
	}
	else
	{
		count = 2;
		++PCD;
	}

	return count;
}

////////////////////////////////////////////////////////////////////////////////////////
// store the modified data
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::sMem(const UINT address,const UINT data)
{
	CPU_WRITE_BYTE(address,data);
}

inline VOID CPU::sZpg(const UINT address,const UINT data)
{
	CPU_WRITE_ZPG(address,data);
}

////////////////////////////////////////////////////////////////////////////////////////
// load instructions
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::Lda(const UINT data) { CPU_FNZD( AD = data ); }	      
inline VOID CPU::Ldx(const UINT data) { CPU_FNZD( XD = data ); }
inline VOID CPU::Ldy(const UINT data) { CPU_FNZD( YD = data ); }

////////////////////////////////////////////////////////////////////////////////////////
// store instructions
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::Sta() const { return AD; }
inline UINT CPU::Stx() const { return XD; }
inline UINT CPU::Sty() const { return YD; }  

////////////////////////////////////////////////////////////////////////////////////////
// transfer instructions
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::Tax() { CPU_FNZD( XD = AD ); }           
inline VOID CPU::Tay() { CPU_FNZD( YD = AD ); }
inline VOID CPU::Txa() { CPU_FNZD( AD = XD ); }
inline VOID CPU::Tya() { CPU_FNZD( AD = YD ); }           

////////////////////////////////////////////////////////////////////////////////////////
// flow control instructions
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::JmpAbs() 
{
	PCD = CPU_READ_WORD(PCD);
}       

inline VOID CPU::JmpInd() 
{
	// 6502 weirdness, page crossing is not handled

	const UINT tmp = CPU_READ_WORD(PCD);

	if ((tmp & 0xFF) == 0xFF)
	{
		PCL = CPU_READ_BYTE(tmp);
		PCH = CPU_READ_BYTE(tmp & 0xFF00);
	}
    else
	{
		PCD = CPU_READ_WORD(tmp);
	}
}       

inline VOID CPU::Jsr() 
{
	// 6502 weirdness, return address pushed on the 
	// stack by JSR is one less than the actual next

	++PCD;
	CPU_PUSH(PCH);
	CPU_PUSH(PCL);
	PCD = CPU_READ_WORD(PCW - 1);
}           

inline VOID CPU::Rts() 
{
	PCL = CPU_PULL();
	PCH = CPU_PULL();
	++PCD;
}           

inline VOID CPU::Rti() 
{
	flags.Unpack(CPU_PULL());
	PCL = CPU_PULL();
	PCH = CPU_PULL();
}   

inline UINT CPU::Bcc() { return Branch(!FCD);              }           
inline UINT CPU::Bcs() { return Branch(FCD);               }
inline UINT CPU::Beq() { return Branch(!FZD);              }		      
inline UINT CPU::Bmi() { return Branch(FND & FLAGS::N);    }
inline UINT CPU::Bne() { return Branch(FZD);               }		      
inline UINT CPU::Bpl() { return Branch(!(FND & FLAGS::N)); }		      
inline UINT CPU::Bvc() { return Branch(!FVD);              }		      
inline UINT CPU::Bvs() { return Branch(FVD);               }

////////////////////////////////////////////////////////////////////////////////////////
// math operations (integrals only, there's no decimal support for the N2A03)
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::Adc(const UINT data) 
{
	PDXWORD tmp;
	tmp.d = AD + data + (FCD ? 0x01 : 0x00);
	FVD = (~(AD ^ data)) & (AD ^ tmp.d) & FLAGS::N;
	FCD = tmp.d > 0xFF;
	CPU_FNZD(AD = tmp.b.l);
}

inline VOID CPU::Sbc(const UINT data) 
{
	PDXWORD tmp;
	tmp.d = AD - data - (FCD ? 0x00 : 0x01);
	FVD = (AD ^ data) & (AD ^ tmp.d) & FLAGS::N;
	FCD = tmp.d <= 0xFF;
	CPU_FNZD(AD = tmp.b.l);
}

////////////////////////////////////////////////////////////////////////////////////////
// logical operations
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::And(const UINT data) { CPU_FNZD( AD &= data ); }
inline VOID CPU::Ora(const UINT data) { CPU_FNZD( AD |= data ); }
inline VOID CPU::Eor(const UINT data) { CPU_FNZD( AD ^= data ); }
     
inline VOID CPU::Bit(const UINT data) 
{
	FND = data;
	FZD = data & AD;
	FVD = data & FLAGS::V;
}	      

inline VOID CPU::Cmp(const UINT data) 
{
	FCD = AD >= data;
	CPU_FNZ(A - data);
}

inline VOID CPU::Cpx(const UINT data) 
{
	FCD = XD >= data;
	CPU_FNZ(X - data);
}

inline VOID CPU::Cpy(const UINT data) 
{
	FCD = YD >= data;
	CPU_FNZ(Y - data);
}

////////////////////////////////////////////////////////////////////////////////////////
// shift operations
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::Asl(U8 data) 
{
	FCD = data & 0x80;
	CPU_FNZ(data <<= 1);
	return data;
}

inline UINT CPU::Lsr(UINT data) 
{
	FCD = data & 0x01;
	CPU_FNZD(data >>= 1);
	return data;
}

inline UINT CPU::Rol(U8 data) 
{
	const UINT tmp = (FCD ? 0x01 : 0x00);
	FCD = data & 0x80;
	CPU_FNZ(data = ((data << 1) | tmp)); 
	return data;
}

inline UINT CPU::Ror(UINT data) 
{
	const UINT tmp = (FCD ? 0x80 : 0x00);
	FCD = data & 0x01;
	CPU_FNZD(data = ((data >> 1) | tmp));
	return data;
}

////////////////////////////////////////////////////////////////////////////////////////
// increment and decrement operations
////////////////////////////////////////////////////////////////////////////////////////

inline UINT CPU::Dec(U8 data) { CPU_FNZD( --data ); return data; }
inline UINT CPU::Inc(U8 data) { CPU_FNZD( ++data ); return data; } 

inline VOID CPU::Dex() { CPU_FNZD( --X ); }           
inline VOID CPU::Dey() { CPU_FNZD( --Y ); }           
inline VOID CPU::Inx() { CPU_FNZD( ++X ); }           
inline VOID CPU::Iny() { CPU_FNZD( ++Y ); }           

////////////////////////////////////////////////////////////////////////////////////////
// flags instructions
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::Clc() { FCD = 0; }
inline VOID CPU::Cld() { FDD = 0; }
inline VOID CPU::Clv() { FVD = 0; }           
inline VOID CPU::Sec() { FCD = 1; }
inline VOID CPU::Sed() { FDD = 1; }

inline VOID CPU::Sei() 
{
	const UINT i = FID; 
	FID = 1;

	if (!i)
	{
    	// if the flag was previously cleared a delay of 
		// one instruction will occur before interrupts will 
		// be disabled

		if (Step() != 0x40 && IntLow)
		{
			// The interrupt will only occur if the next 
			// executed instruction was not RTI

			DoISR();
		}
	}
}

inline VOID CPU::Cli() 
{
	const UINT i = FID; 
	FID = 0;

	if (i)
	{
    	// if the flag was not previously set a delay 
		// of one instruction will occur before pending 
		// interrupts are handled

		Step();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// stack operations
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::Plp() 
{
	const UINT i = FID;
	flags.Unpack(CPU_PULL());          

	if (bool(i) != bool(FID))
	{
    	// if the I flag was changed a delay of one instruction 
		// will occur before using the new state

		if (Step() != 0x40 && IntLow && !i)
		{
			// The interrupt will only occur if the next 
			// executed instruction was not RTI

			DoISR();
		}
	}
}

inline VOID CPU::Php() { CPU_PUSH(flags.Pack() | FLAGS::B); }           
inline VOID CPU::Pha() { CPU_PUSH(AD);                      }           
inline VOID CPU::Pla() { CPU_FNZD(AD = CPU_PULL());         }           
inline VOID CPU::Tsx() { CPU_FNZD(XD = SPD);                }
inline VOID CPU::Txs() { SPD = XD;                          }

////////////////////////////////////////////////////////////////////////////////////////
// undocumented instructions, some of them very unreliable but most of them	will 
// probably never be executed anyway - they're here for the hackers.
////////////////////////////////////////////////////////////////////////////////////////

inline VOID CPU::Anc(const UINT data) 
{
	Log("CPU: unofficial opcode executed - ANC",0);
	CPU_FNZD(AD &= data);
	FCD = FND & FLAGS::N;
}

inline VOID CPU::Ane(const UINT data) 
{
	Log("CPU: unofficial opcode executed - ANE",1);
	CPU_FNZD(AD = (AD | 0xEE) & XD & data);
}

inline VOID CPU::Arr(const UINT data) 
{
	Log("CPU: unofficial opcode executed - ARR",2);
	CPU_FNZD(AD = ((data & AD) >> 1) | (FCD ? 0x80 : 0x00));
	FCD = AD & 0x40;
	FVD = ((AD >> 6) ^ (AD >> 5)) & 1;
}

inline VOID CPU::Asr(UINT data) 
{
	Log("CPU: unofficial opcode executed - ASR",3);
	data &= AD;
	FCD = data & 0x01;
	CPU_FNZD(AD = data >> 1);
}

inline UINT CPU::Dcp(U8 data) 
{
	Log("CPU: unofficial opcode executed - DCP",4);
	Cmp(--data);
	return data;
}

inline UINT CPU::Isb(U8 data) 
{
	Log("CPU: unofficial opcode executed - ISB",5);
	Sbc(++data);
	return data;
}

inline VOID CPU::Las(const UINT data) 
{
	Log("CPU: unofficial opcode executed - LAS",6);
	CPU_FNZD(AD = XD = SPD = (SPD & data));
}

inline VOID CPU::Lax(const UINT data) 
{ 
	Log("CPU: unofficial opcode executed - LAX",7);
	CPU_FNZD(XD = AD = data); 
}       

inline VOID CPU::Lxa(const UINT data) 
{
	Log("CPU: unofficial opcode executed - LXA",8);
	CPU_FNZD(XD = AD = (AD & data));
}

inline UINT CPU::Rla(U8 data) 
{
	Log("CPU: unofficial opcode executed - RLA",9);
	const UINT tmp = (FCD ? 0x01 : 0x00);
	FCD = data & 0x80;
	data = ((data << 1) | tmp);
	CPU_FNZD(AD &= data);
	return data;
}      

inline UINT CPU::Rra(UINT data) 
{
	Log("CPU: unofficial opcode executed - RRA",10);
	const UINT tmp = (FCD ? 0x80 : 0x00);
	FCD = data & 0x01;
	data = (data >> 1) | tmp;
	Adc(data);
	return data;
} 

inline UINT CPU::Sax() 
{
	Log("CPU: unofficial opcode executed - SAX",11);
	return AD & XD;
}  

inline VOID CPU::Sbx(const UINT data) 
{
	Log("CPU: unofficial opcode executed - SBX",12);
	PDXWORD tmp;
	tmp.d = (AD & XD) - data;
	FCD = tmp.d <= 0xFF;
	CPU_FNZD(XD = tmp.b.l);
}

inline UINT CPU::Sha(const UINT address) 
{
	Log("CPU: unofficial opcode executed - SHA",13);
	return AD & XD & U8((address >> 8) + 1);
}

inline UINT CPU::Shs(const UINT address) 
{
	Log("CPU: unofficial opcode executed - SHS",14);
	SPD = AD & XD;
	return SPD & U8((address >> 8) + 1);
}

inline UINT CPU::Shx(const UINT address)
{
	Log("CPU: unofficial opcode executed - SHX",15);
	return XD & U8((address >> 8) + 1);
}

inline UINT CPU::Shy(const UINT address)
{
	Log("CPU: unofficial opcode executed - SHY",16);
	return YD & U8((address >> 8) + 1);
}

inline UINT CPU::Slo(UINT data) 
{
	Log("CPU: unofficial opcode executed - SLO",17);
	FCD = data & 0x80;
	data = (data << 1) & 0xFF;
	CPU_FNZD(AD |= data);
	return data;
}

inline UINT CPU::Sre(UINT data) 
{
	Log("CPU: unofficial opcode executed - SRE",18);
	FCD = data & 0x01;
	data >>= 1;
	CPU_FNZD(AD ^= data);
	return data;
}

inline VOID CPU::Dop(const UINT) { Log("CPU: unofficial opcode executed - DOP",19); }
inline VOID CPU::Top(const UINT) { Log("CPU: unofficial opcode executed - TOP",20); }

////////////////////////////////////////////////////////////////////////////////////////
// interrupts
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::DoISR()
{
	if (!jammed)
	{
		CPU_PUSH(PCH);
		CPU_PUSH(PCL);
		CPU_PUSH(flags.Pack());
		FID = 1;
		FDD = 0;
		PCD = CPU_READ_WORD(IRQ_VECTOR);
		CPU_EAT_CYCLES(INT_CYCLES);
	}
}

VOID CPU::DoNMISR()
{
	if (!jammed)
	{
		// There's apparently some latency 
		// before NMISR is taken..

		Step();

		CPU_PUSH(PCH);
		CPU_PUSH(PCL);
		CPU_PUSH(flags.Pack());
		FID = 1;
		FDD = 0;
		PCD = CPU_READ_WORD(NMI_VECTOR);
		CPU_EAT_CYCLES(INT_CYCLES);
	}
}

inline VOID CPU::Brk() 
{
	Log("CPU: opcode executed - BRK",21);

	++PCD;
	CPU_PUSH(PCH);
	CPU_PUSH(PCL);
	CPU_PUSH(flags.Pack() | FLAGS::B);
	FID = 1;
	FDD = 0;
	PCD = CPU_READ_WORD(IRQ_VECTOR);
}	

inline VOID CPU::Jam() 
{
	PDX_DEBUG_BREAK_MSG("6502 jam");
	
	--PCW;
	
	if (!jammed)
	{
		jammed = TRUE;

		MsgOutput("CPU is jammed..");
		LogOutput("CPU: jammed");
	}

	IntLow = 0;
}         

////////////////////////////////////////////////////////////////////////////////////////
// Clear RAM
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::ClearRAM()
{
	PDXMemZero( ram, RAM_SIZE );
}

////////////////////////////////////////////////////////////////////////////////////////
// reset the cpu
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::Reset(const BOOL hard)
{
	for (UINT i=0; i < 24; ++i)
		logged[i] = false;

	map.SetPort( 0x0000, 0x1FFF, this, Peek_ram, Poke_ram );
	map.SetPort( 0x4015, this, Peek_4015, Poke_4015 );
  
	if (hard)
	{
		srand(UINT(time(NULL)));

   		// maybe a little over the top but some games 
   		// actually read from it unmodified so make
		// sure the ram contains garbage.

		for (UINT i=0; i < RAM_SIZE; ++i)
			ram[i] = rand();

		AD = XD = YD = 0;
		SPD = 0xFF;
		FND = FCD = FVD = FZD = 0;
	}

	FID = 1;
	FDD = 0;
	PCD = CPU_READ_WORD(RESET_VECTOR);
	
	cycles           = pal ? NES_CPU_TO_PAL(RESET_CYCLES) : NES_CPU_TO_NTSC(RESET_CYCLES);
	FrameCounter     = LONG_MAX;
	DmcCounter       = LONG_MAX;
	DmcLengthCounter = 0;
	IntLow           = 0;
	IntEn            = IRQ_FRAME|IRQ_EXT_1|IRQ_EXT_2;
	status           = 0;
	jammed           = FALSE;

	ResetLog();

	apu.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::ResetLog()
{
	LogOutput("CPU: reset");

	PDXSTRING log("CPU: ");

	log.Resize( 5 ); log += "Program Counter: "; log.Append( PCD, PDXSTRING::HEX );     LogOutput( log.String() );
	log.Resize( 5 ); log += "Stack Pointer: ";   log.Append( SPD, PDXSTRING::HEX );     LogOutput( log.String() );
	log.Resize( 5 ); log += "A register: ";      log.Append( AD,  PDXSTRING::HEX );     LogOutput( log.String() );
	log.Resize( 5 ); log += "X register: ";      log.Append( XD,  PDXSTRING::HEX );     LogOutput( log.String() );
	log.Resize( 5 ); log += "Y register: ";      log.Append( YD,  PDXSTRING::HEX );     LogOutput( log.String() );
	log.Resize( 5 ); log += "C flag: ";          log += (FCD ? "1" : "0");      		LogOutput( log.String() );
	log.Resize( 5 ); log += "Z flag: ";          log += (FZD ? "0" : "1");              LogOutput( log.String() );	
	log.Resize( 5 ); log += "I flag: ";          log += (FID ? "1" : "0");              LogOutput( log.String() );
	log.Resize( 5 ); log += "D flag: ";          log += (FDD ? "1" : "0");              LogOutput( log.String() );	
	log.Resize( 5 ); log += "V flag: ";          log += (FVD ? "1" : "0");              LogOutput( log.String() );	
	log.Resize( 5 ); log += "N flag: ";          log += ((FND & FLAGS::N) ? "1" : "0"); LogOutput( log.String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::UpdateFrameCounter()
{
	IntLow |= IRQ_FRAME;
	FrameCounter += (pal ? NES_CPU_MCC_FRAME_PAL : NES_CPU_MCC_FRAME_NTSC);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::UpdateDmcCounter()
{
	PDX_ASSERT(DmcLengthCounter);

	apu.Update();

	// Simulate a DMA byte transfer

	CPU_EAT_CYCLES(DMA_CYCLES);

	if (!--DmcLengthCounter)
	{
		if (apu.IsDmcLooped())
		{
			DmcLengthCounter = apu.GetDmcLengthCounter();
		}
		else 
		{
			DmcCounter = LONG_MAX;

			PDX_ASSERT(!(IntEn & IRQ_DMC) || ((IntEn & IRQ_DMC) && (status & STATUS_INT_IRQ)));

			// IRQ if 4010.7 and 4017.6 are set

			if (status & STATUS_INT_IRQ)
				IntLow |= (IntEn & IRQ_DMC);

			return;
		}
	}

	DmcCounter += apu.GetDmcFrequency();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(CPU,4015)
{
	const UINT value = apu.Peek_4015() | (IntLow & (IRQ_FRAME|IRQ_DMC));
	IntLow &= ~IRQ_FRAME;
	return value;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(CPU,4015)
{
	apu.Poke_4015( data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::Poke_4017(const UINT data)
{
	IntLow &= ~IRQ_FRAME;
	
	if (status = (data & (STATUS_INT_IRQ|STATUS_EXT_IRQ)))
	{
		FrameCounter = LONG_MAX;
	}
	else
	{
		FrameCounter = cycles + (pal ? NES_CPU_MCC_FRAME_PAL : NES_CPU_MCC_FRAME_NTSC);
	}

	apu.Poke_4017( data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT CPU::Peek_4017()
{
	return status;
}

////////////////////////////////////////////////////////////////////////////////////////
// execute a frame
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::Execute()
{
	const ULONG count = FrameCycles;

	switch (events.Size())
	{
     	case 0:
   	    {
			while (cycles < count)
			{
				{
					const UINT opcode = CPU_READ_PCB();
					(this->*instructions[opcode])();
				}

				if (cycles >= DmcCounter)
					UpdateDmcCounter();

				if (cycles >= FrameCounter)
					UpdateFrameCounter();

				if (IntLow)
				{
					IntLow &= ~IRQ_TMP;
					if (!FID) DoISR();
				}
			}
			break;
		}

		case 1:
		{
			const EVENT* const event = events.Begin();

			while (cycles < count)
			{
				{
					const UINT opcode = CPU_READ_PCB();
					(this->*instructions[opcode])();
				}

				event->Execute();

				if (cycles >= DmcCounter)
					UpdateDmcCounter();

				if (cycles >= FrameCounter)
					UpdateFrameCounter();

				if (IntLow)
				{
					IntLow &= ~IRQ_TMP;
					if (!FID) DoISR();
				}
			}
			break;
		}

		default:
		{
			while (cycles < count)
			{
				{
					const UINT opcode = CPU_READ_PCB();
					(this->*instructions[opcode])();
				}

				for (const EVENT* event=events.Begin(); event != events.End(); ++event)
					event->Execute();

				if (cycles >= DmcCounter)
					UpdateDmcCounter();

				if (cycles >= FrameCounter)
					UpdateFrameCounter();

				if (IntLow)
				{
					IntLow &= ~IRQ_TMP;
					if (!FID) DoISR();
				}
			}
		}
	}

	apu.EndFrame();

	cycles -= count;

	if (DmcCounter != LONG_MAX)
		DmcCounter -= count;

	if (FrameCounter != LONG_MAX)
		FrameCounter -= count;
}

////////////////////////////////////////////////////////////////////////////////////////
// execute one instruction
////////////////////////////////////////////////////////////////////////////////////////

UINT CPU::Step()
{
	const UINT opcode = CPU_READ_PCB();
	(this->*instructions[opcode])();
	return opcode;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CPU::Log(const CHAR* const msg,const UINT which)
{
	PDX_DEBUG_BREAK_MSG(msg);

	if (!logged[which])
	{
		logged[which] = true;
		LogOutput( msg );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT CPU::LoadState(PDXFILE& file)
{
	{
		HEADER header;

		if (!file.Read(header))
			return PDX_FAILURE;

		PCD = header.pc;
		AD  = header.a;
		XD  = header.x;
		YD  = header.y;
		SPD = header.sp;

		flags.Unpack(header.flags);

		IntLow = header.IntLow;
		IntEn  = header.IntEn;
		cycles = header.cycles;
		jammed = header.jammed;
	}

	if (!file.Read( ram, ram + RAM_SIZE ))
		return PDX_FAILURE;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT CPU::SaveState(PDXFILE& file) const
{
	{
		HEADER header;

		header.pc     = PCW;
		header.a      = A;
		header.x      = X;
		header.y      = Y;
		header.sp     = SP;
		header.flags  = flags.Pack();
		header.cycles = cycles;
		header.IntLow = IntLow;
		header.IntEn  = IntEn;
		header.jammed = jammed ? 1 : 0;

		file.Write( header );
	}

	file.Write( ram, ram + RAM_SIZE );

	return PDX_OK;
}

NES_NAMESPACE_END

