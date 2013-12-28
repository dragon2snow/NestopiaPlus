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

#ifndef NST_CPU_H
#define NST_CPU_H

#include "NstMap.h"
#include "NstApu.h"

class PDXFILE;

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// 6502 (N2A03) CPU class
////////////////////////////////////////////////////////////////////////////////////////

class CPU
{
public:

	enum
	{
		RESET_CYCLES   = 6,
		INT_CYCLES     = 7,
		DMC_DMA_CYCLES = 1,
		NMI_VECTOR     = 0xFFFA,
		RESET_VECTOR   = 0xFFFC,
		IRQ_VECTOR     = 0xFFFE,
		RAM_SIZE       = 0x800,
		STACK_OFFSET   = 0x100
	};

private:

	class EVENT
	{
	private:

		struct OBJECT {};
		typedef VOID (OBJECT::*const FUNCTION)();

	public:

		template<class O,class F> 
		EVENT(O* o,F f)
		: 
		object   (PDX_CAST(OBJECT*,o)), 
		function (PDX_CAST_REF(FUNCTION,f))
		{}

		inline VOID Execute() const
		{
			(object->*function)();
		}

		template<class TYPE>
		inline TYPE* Object() const
		{
			return PDX_CAST(TYPE*,object);
		}

		template<class TYPE>
		inline VOID (TYPE::*Function() const) ()
		{
			typedef VOID (TYPE::*READER)();
			return PDX_CAST_REF(READER,function);
		}

		template<class ANY>
		inline BOOL operator == (ANY any) const
		{ 
			return 
			(
		     	PDX_CAST(const VOID* const,object) == PDX_CAST_REF(const VOID* const,any) || 
				PDX_CAST_REF(const VOID* const,function) == PDX_CAST_REF(const VOID* const,any)
			); 
		}

	private:

		OBJECT* const object;
		FUNCTION function;
	};

public:
  
	enum
	{
		IRQ_FRAME = b01000000,
		IRQ_EXT   = b00000010,
		IRQ_EXT_1 = b00000010,
		IRQ_EXT_2 = b00001000,
		IRQ_DMC   = b10000000,
		IRQ_ANY   = b11001010,
		NMI       = b00000100
	};

	enum
	{
		STATUS_INT_IRQ = b01000000,
		STATUS_EXT_IRQ = b10000000
	};

	enum CYCLETYPE
	{
		CYCLE_MASTER,
		CYCLE_NTSC,
		CYCLE_PAL,
		CYCLE_AUTO
	};

	typedef CPU_PORT PORT;

	CPU();

	PDX_NO_INLINE VOID Reset(const BOOL=FALSE);
	PDX_NO_INLINE VOID ResetPorts();

	VOID Execute();
	UINT Step();
	
	inline VOID SetPC ( const UINT address ) { pc.d = address; }
	inline VOID SetA  ( const UINT data    ) { a.d  = data;    }
	inline VOID SetX  ( const UINT data    ) { x.d  = data;    }
	inline VOID SetY  ( const UINT data    ) { y.d  = data;    }
	inline VOID SetSP ( const UINT offset  ) { sp.d = offset;  }

	inline UINT GetPC () const { return pc.d; }
	inline UINT GetA  () const { return a.d;  }
	inline UINT GetX  () const { return x.d;  }
	inline UINT GetY  () const { return y.d;  }
	inline UINT GetSP () const { return sp.d; }

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;
	
	APU& GetAPU();
	const APU& GetAPU() const;

	VOID DoNMI(const ULONG);
	VOID DoIRQ(const UINT=IRQ_EXT);
	VOID ClearIRQ(const UINT=IRQ_EXT);
	VOID SetLine(const UINT=IRQ_EXT,const BOOL=TRUE);
	BOOL IsLine(const UINT=IRQ_EXT) const;
	BOOL IsIRQ(const UINT=IRQ_EXT) const;
	VOID HandlePendingInterrupts();

	VOID ClearRAM();

	const U8* Ram() const;

	UINT Peek(const UINT);
	VOID Poke(const UINT,const UINT);

	VOID AdvanceCycles(const ULONG);
	VOID ResetCycles();
	
	template<CYCLETYPE>
	ULONG GetCycles() const;
	
	template<CYCLETYPE>
	ULONG GetFrameCycles() const;

	VOID SetupFrame(const ULONG);
	VOID SetDmcDmaClock(const LONG);
	VOID DisableDmcDmaClock();
	VOID SetDmcLengthCounter(const UINT);

	template<class OBJECT,class FUNCTION>
	VOID SetEvent(OBJECT*,FUNCTION);

	template<class ANY> 
	VOID RemoveEvent(ANY);

	template<class OBJECT,class READER,class WRITER>
	VOID SetPort(const UINT,OBJECT*,READER,WRITER);

	template<class OBJECT,class READER,class WRITER>
	VOID SetPort(const UINT,const UINT,OBJECT*,READER,WRITER);

	PORT& GetPort(const UINT);
	const PORT& GetPort(const UINT) const;

	UINT GetCache() const;
	UINT GetStatus() const;
	BOOL IsPAL() const;

	VOID SetMode(const MODE);

	VOID Poke_4017(const UINT);
	UINT Peek_4017();

private:

	enum
	{
		I_DELAY_ON  = b00000001,
		I_DELAY_OFF = b00000010,
		LIC         = b00000100
	};

	VOID ResetLog();
	VOID Log(const CHAR* const,const UINT);

	PDX_COMPILE_ASSERT(U8(U8(0xFF)+U8(0x01))==U8(0x00));

	typedef PDXARRAY<const EVENT> EVENTS;

	NES_DECL_POKE( ram   );
	NES_DECL_PEEK( ram   );
	NES_DECL_POKE( nop   );
	NES_DECL_PEEK( nop   );
	NES_DECL_PEEK( jam_1 );
	NES_DECL_PEEK( jam_2 );

	NES_DECL_POKE( 4015 );
	NES_DECL_PEEK( 4015 );

	PDX_NO_INLINE VOID DoISR();
	PDX_NO_INLINE VOID DoNMISR();
	PDX_NO_INLINE VOID DoFrameIRQ();
	PDX_NO_INLINE VOID DoDmcDma();

	VOID Run0 (const ULONG);
	VOID Run1 (const ULONG);
	VOID Run2 (const ULONG);

	UINT rZpgReg  (const UINT);
	UINT rwZpgReg (const UINT,UINT&);
	UINT wZpgReg  (const UINT);

	UINT rAbsReg  (const UINT);
	UINT rwAbsReg (const UINT,UINT&);
	UINT wAbsReg  (const UINT);

	VOID Branch(const UINT);

	UINT Acc();
    #define rwAcc(p) Acc()

	UINT rImm  ();	
	UINT rZpg  ();
	UINT rZpgX ();
	UINT rZpgY ();
	UINT rAbs  ();
	UINT rAbsX ();
	UINT rAbsY ();
	UINT rIndX ();
	UINT rIndY ();

	UINT rwZpg  (UINT&);
	UINT rwZpgX (UINT&);
	UINT rwZpgY (UINT&);
	UINT rwAbs  (UINT&);
	UINT rwAbsY (UINT&);
	UINT rwAbsX (UINT&);
	UINT rwIndX (UINT&);
	UINT rwIndY (UINT&);

	UINT wZpg  ();
	UINT wZpgX ();
	UINT wZpgY ();
	UINT wAbs  ();
	UINT wAbsX ();
	UINT wAbsY ();
	UINT wIndX ();
	UINT wIndY ();

    #define sZpgX     sZpg
    #define sZpgY     sZpg
    #define sAbs      sMem
    #define sAbsX     sMem
    #define sAbsY     sMem
    #define sIndX     sMem
    #define sIndY     sMem
    #define sAcc(p,v) a.d = (v)    

	VOID sMem (const UINT,const UINT);
	VOID sZpg (const UINT,const UINT);

	VOID Lda (const UINT);	      
	VOID Ldx (const UINT);
	VOID Ldy (const UINT);

	UINT Sta() const;
	UINT Stx() const;
	UINT Sty() const; 

	VOID Tax ();           
	VOID Tay ();
	VOID Txa ();
	VOID Tya ();           

	VOID JmpAbs (); 
	VOID JmpInd (); 
	VOID Jsr    (); 
	VOID Rts    (); 
	VOID Rti    (); 

	VOID Bcc ();           
	VOID Bcs ();
	VOID Beq ();		      
	VOID Bmi ();
	VOID Bne ();		      
	VOID Bpl ();		      
	VOID Bvc ();		      
	VOID Bvs ();

	VOID Adc (const UINT); 
	VOID Sbc (const UINT); 

	VOID And (const UINT);
	VOID Ora (const UINT);
	VOID Eor (const UINT);

	VOID Bit (const UINT); 
	VOID Cmp (const UINT); 
	VOID Cpx (const UINT); 
	VOID Cpy (const UINT); 

	UINT Asl (U8);
	UINT Lsr (UINT); 
	UINT Rol (U8); 
	UINT Ror (UINT); 

	UINT Dec (U8);
	UINT Inc (U8);

	VOID Dex ();           
	VOID Dey ();           
	VOID Inx ();           
	VOID Iny ();           

	VOID Clc ();
	VOID Cld ();
	VOID Clv ();           
	VOID Sec ();
	VOID Sed ();
	VOID Sei ();
	VOID Cli ();

	VOID Php ();          
	VOID Plp ();          
	VOID Pha ();          
	VOID Pla ();          
	VOID Tsx ();
	VOID Txs ();

	VOID Anc (const UINT); 
	VOID Ane (const UINT); 
	VOID Arr (const UINT); 
	VOID Asr (UINT);
	UINT Dcp (U8); 
	UINT Isb (U8); 
	VOID Las (const UINT); 
	VOID Lax (const UINT); 
	VOID Lxa (const UINT); 
	UINT Rla (U8); 
	UINT Rra (UINT);
	UINT Sax (); 
	VOID Sbx (const UINT); 
	UINT Sha (const UINT); 
	UINT Shs (const UINT); 
	UINT Shx (const UINT);
	UINT Shy (const UINT);
	UINT Slo (UINT); 
	UINT Sre (UINT); 

	VOID Dop (const UINT);
	VOID Top (const UINT);

	VOID Brk (); 
	VOID Jam (); 

    #define NES_EAT_CYCLES(ticks) cycles += CyclePtr[(ticks)-1]

    #define NES_I____(instr,hex)	       \
								           \
	PDX_NO_INLINE VOID op##hex()           \
	{							           \
		instr();                           \
	}

    #define NES____C_(dummy,ticks,hex)     \
								           \
	PDX_NO_INLINE VOID op##hex()           \
	{							           \
	    NES_EAT_CYCLES(ticks);		       \
	}

    #define NES_I__C_(instr,ticks,hex)     \
								           \
	PDX_NO_INLINE VOID op##hex()           \
	{							           \
	    NES_EAT_CYCLES(ticks);	           \
		instr();				           \
	}

    #define NES_IR___(instr,addr,hex)      \
									       \
	PDX_NO_INLINE VOID op##hex()           \
	{							           \
    	instr(r##addr());			       \
	}

    #define NES_I_W__(instr,addr,hex)      \
									       \
	PDX_NO_INLINE VOID op##hex()           \
	{							           \
    	s##addr(w##addr(),instr());        \
	}

    #define NES_IRW__(instr,addr,hex)      \
										   \
	PDX_NO_INLINE VOID op##hex()		   \
	{									   \
		UINT p;							   \
		const UINT d = instr(rw##addr(p)); \
		s##addr(p,d);					   \
	}

    #define NES_I_W_A(instr,addr,hex)	   \
										   \
    PDX_NO_INLINE VOID op##hex()		   \
	{									   \
		const UINT p = w##addr();		   \
		s##addr(p,instr(p));			   \
	}

    // param 1 = instruction
	// param 2 = addressing mode
	// param 3 = clock cycles
	// param 4 = opcode

	NES_IR___( Adc, Imm,      0x69 )
	NES_IR___( Adc, Zpg,      0x65 )
	NES_IR___( Adc, ZpgX,     0x75 )
	NES_IR___( Adc, Abs,      0x6D )
	NES_IR___( Adc, AbsX,     0x7D )
	NES_IR___( Adc, AbsY,     0x79 )
	NES_IR___( Adc, IndX,     0x61 )
	NES_IR___( Adc, IndY,     0x71 )
	NES_IR___( Anc, Imm,      0x0B )
	NES_IR___( Anc, Imm,      0x2B ) 
	NES_IR___( And, Imm,      0x29 )   
	NES_IR___( And, Zpg,      0x25 )
	NES_IR___( And, ZpgX,     0x35 )
	NES_IR___( And, Abs,      0x2D )
	NES_IR___( And, AbsX,     0x3D )
	NES_IR___( And, AbsY,     0x39 )
	NES_IR___( And, IndX,     0x21 )
	NES_IR___( And, IndY,     0x31 )
	NES_IR___( Ane, Imm,      0x8B )
	NES_IR___( Arr, Imm,      0x6B )	
	NES_IRW__( Asl, Acc,      0x0A )
	NES_IRW__( Asl, Zpg,      0x06 )
	NES_IRW__( Asl, ZpgX,     0x16 ) 
	NES_IRW__( Asl, Abs,      0x0E )
	NES_IRW__( Asl, AbsX,     0x1E )	
	NES_IR___( Asr, Imm,      0x4B )
	NES_I____( Bcc, 	      0x90 )
	NES_I____( Bcs, 	      0xB0 )
	NES_I____( Beq,  	      0xF0 )  
	NES_IR___( Bit, Zpg,      0x24 )
	NES_IR___( Bit, Abs,      0x2C )
	NES_I____( Bmi, 	      0x30 )
	NES_I____( Bne, 	      0xD0 )
	NES_I____( Bpl, 	      0x10 )	    
	NES_I__C_( Brk,        7, 0x00 )
	NES_I____( Bvc,  		  0x50 )
	NES_I____( Bvs,  	      0x70 )
	NES_I__C_( Clc,        2, 0x18 )
	NES_I__C_( Cld,        2, 0xD8 )
	NES_I__C_( Cli,        2, 0x58 ) 
	NES_I__C_( Clv,        2, 0xB8 )  
	NES_IR___( Cmp, Imm,      0xC9 )  
	NES_IR___( Cmp, Zpg,      0xC5 )
	NES_IR___( Cmp, ZpgX,     0xD5 )   
	NES_IR___( Cmp, Abs,      0xCD )
	NES_IR___( Cmp, AbsX,     0xDD )
	NES_IR___( Cmp, AbsY,     0xD9 )	 
	NES_IR___( Cmp, IndX,     0xC1 )	 
	NES_IR___( Cmp, IndY,     0xD1 )	    
	NES_IR___( Cpx, Imm,      0xE0 )	    
	NES_IR___( Cpx, Zpg,      0xE4 )	    
	NES_IR___( Cpx, Abs,      0xEC )  
	NES_IR___( Cpy, Imm,      0xC0 )	    
	NES_IR___( Cpy, Zpg,      0xC4 )	    
	NES_IR___( Cpy, Abs,      0xCC )  	
	NES_IRW__( Dcp, Zpg,      0xC7 )	 
	NES_IRW__( Dcp, ZpgX,     0xD7 )	 
	NES_IRW__( Dcp, IndX,     0xC3 )	 
	NES_IRW__( Dcp, IndY,     0xD3 )  
	NES_IRW__( Dcp, Abs,      0xCF )	 
	NES_IRW__( Dcp, AbsX,     0xDF )	 
	NES_IRW__( Dcp, AbsY,     0xDB )	    		
	NES_IRW__( Dec, Zpg,      0xC6 )	 
	NES_IRW__( Dec, ZpgX,     0xD6 )	    
	NES_IRW__( Dec, Abs,      0xCE )	 
	NES_IRW__( Dec, AbsX,     0xDE )  	
	NES_I__C_( Dex,        2, 0xCA )  	
	NES_I__C_( Dey,        2, 0x88 )  
	NES_IR___( Dop, Imm,      0x80 )  
	NES_IR___( Dop, Imm,      0x82 )  
	NES_IR___( Dop, Imm,      0x89 )  
	NES_IR___( Dop, Imm,      0xC2 )  
	NES_IR___( Dop, Imm,      0xE2 )	 
	NES_IR___( Dop, Zpg,      0x04 )  
	NES_IR___( Dop, Zpg,      0x44 )	 
	NES_IR___( Dop, Zpg,      0x64 )	 
	NES_IR___( Dop, ZpgX,     0x14 )	 
	NES_IR___( Dop, ZpgX,     0x34 )	 
	NES_IR___( Dop, ZpgX,     0x54 )	 
	NES_IR___( Dop, ZpgX,     0x74 )	    
	NES_IR___( Dop, ZpgX,     0xD4 )	 
	NES_IR___( Dop, ZpgX,     0xF4 )	    
	NES_IR___( Eor, Imm,      0x49 )	 
	NES_IR___( Eor, Zpg,      0x45 )  
	NES_IR___( Eor, ZpgX,     0x55 )  
	NES_IR___( Eor, Abs,      0x4D )  
	NES_IR___( Eor, AbsX,     0x5D )	 
	NES_IR___( Eor, AbsY,     0x59 )  
	NES_IR___( Eor, IndX,     0x41 )	 
	NES_IR___( Eor, IndY,     0x51 )	 
	NES_IRW__( Inc, Zpg,      0xE6 )	    
	NES_IRW__( Inc, ZpgX,     0xF6 )	    
	NES_IRW__( Inc, Abs,      0xEE )	 
	NES_IRW__( Inc, AbsX,     0xFE )	    	
	NES_I__C_( Inx,        2, 0xE8 )	 
	NES_I__C_( Iny,        2, 0xC8 )	 	
	NES_IRW__( Isb, Zpg,      0xE7 )	 
	NES_IRW__( Isb, ZpgX,     0xF7 )	 
	NES_IRW__( Isb, Abs,      0xEF )  
	NES_IRW__( Isb, AbsX,     0xFF )  
	NES_IRW__( Isb, AbsY,     0xFB )	 
	NES_IRW__( Isb, IndX,     0xE3 )  
	NES_IRW__( Isb, IndY,     0xF3 )	 	
	NES_I__C_( Jam,		   2, 0x02 )	 
	NES_I__C_( Jam,		   2, 0x12 )	 
	NES_I__C_( Jam,		   2, 0x22 )  
	NES_I__C_( Jam,		   2, 0x32 )  
	NES_I__C_( Jam,		   2, 0x42 )  
	NES_I__C_( Jam,		   2, 0x52 )	 
	NES_I__C_( Jam,		   2, 0x62 )  
	NES_I__C_( Jam,		   2, 0x72 )	 
	NES_I__C_( Jam,		   2, 0x92 )  
	NES_I__C_( Jam,		   2, 0xB2 )	 
	NES_I__C_( Jam,		   2, 0xD2 )	 
	NES_I__C_( Jam,		   2, 0xF2 )	 	
	NES_I__C_( JmpAbs,     3, 0x4C )  
	NES_I__C_( JmpInd,     5, 0x6C )  
	NES_I__C_( Jsr,        6, 0x20 )	 
	NES_IR___( Las, AbsY,     0xBB )  
	NES_IR___( Lax, Zpg,      0xA7 )	 
	NES_IR___( Lax, ZpgY,     0xB7 )	    
	NES_IR___( Lax, Abs,      0xAF )	    
	NES_IR___( Lax, AbsY,     0xBF )	 
	NES_IR___( Lax, IndX,     0xA3 )	    
	NES_IR___( Lax, IndY,     0xB3 )	 
	NES_IR___( Lda, Imm,      0xA9 )  
	NES_IR___( Lda, Zpg,      0xA5 )	 
	NES_IR___( Lda, ZpgX,     0xB5 )	 
	NES_IR___( Lda, Abs,      0xAD )	    
	NES_IR___( Lda, AbsX,     0xBD )	 
	NES_IR___( Lda, AbsY,     0xB9 )	    
	NES_IR___( Lda, IndX,     0xA1 )	 
	NES_IR___( Lda, IndY,     0xB1 )	 
	NES_IR___( Ldx, Imm,      0xA2 )	 
	NES_IR___( Ldx, Zpg,      0xA6 )	 
	NES_IR___( Ldx, ZpgY,     0xB6 )	 
	NES_IR___( Ldx, Abs,      0xAE )  
	NES_IR___( Ldx, AbsY,     0xBE )  
	NES_IR___( Ldy, Imm,      0xA0 )  
	NES_IR___( Ldy, Zpg,      0xA4 )  
	NES_IR___( Ldy, ZpgX,     0xB4 )	 
	NES_IR___( Ldy, Abs,      0xAC )  
	NES_IR___( Ldy, AbsX,     0xBC )	 	
	NES_IRW__( Lsr, Acc,      0x4A )	 
	NES_IRW__( Lsr, Zpg,      0x46 )	 
	NES_IRW__( Lsr, ZpgX,     0x56 )	 
	NES_IRW__( Lsr, Abs,      0x4E )	    
	NES_IRW__( Lsr, AbsX,     0x5E )	    	
	NES_IR___( Lxa, Imm,      0xAB )	 
	NES____C_( Nop,        2, 0x1A )	    
	NES____C_( Nop,        2, 0x3A )	 
	NES____C_( Nop,        2, 0x5A )	    
	NES____C_( Nop,        2, 0x7A )	    
	NES____C_( Nop,        2, 0xDA )	 
	NES____C_( Nop,        2, 0xEA )	    
	NES____C_( Nop,        2, 0xFA )	 
	NES_IR___( Ora, Imm,      0x09 )  
	NES_IR___( Ora, Zpg,      0x05 ) 	 
	NES_IR___( Ora, ZpgX,     0x15 ) 
	NES_IR___( Ora, Abs,      0x0D )	 
	NES_IR___( Ora, AbsX,     0x1D )	 
	NES_IR___( Ora, AbsY,     0x19 )	 
	NES_IR___( Ora, IndX,     0x01 )	 
	NES_IR___( Ora, IndY,     0x11 )  
	NES_I__C_( Pha,        3, 0x48 )  
	NES_I__C_( Php,        3, 0x08 )  
	NES_I__C_( Pla,        4, 0x68 )	 
	NES_I__C_( Plp,        4, 0x28 )  	
	NES_IRW__( Rla, Zpg,      0x27 )	 
	NES_IRW__( Rla, ZpgX,     0x37 )  
	NES_IRW__( Rla, Abs,      0x2F )	    
	NES_IRW__( Rla, AbsX,     0x3F )	 
	NES_IRW__( Rla, AbsY,     0x3B )	    
	NES_IRW__( Rla, IndX,     0x23 )	 
	NES_IRW__( Rla, IndY,     0x33 )	 	
	NES_IRW__( Rol, Acc,      0x2A )	 
	NES_IRW__( Rol, Zpg,      0x26 )	 
	NES_IRW__( Rol, ZpgX,     0x36 )  
	NES_IRW__( Rol, Abs,      0x2E )  
	NES_IRW__( Rol, AbsX,     0x3E )  	
	NES_IRW__( Ror, Acc,      0x6A )  
	NES_IRW__( Ror, Zpg,      0x66 )	 
	NES_IRW__( Ror, ZpgX,     0x76 )	 
	NES_IRW__( Ror, Abs,      0x6E )	 
	NES_IRW__( Ror, AbsX,     0x7E )	 	
	NES_IRW__( Rra, Zpg,      0x67 )	 
	NES_IRW__( Rra, ZpgX,     0x77 )  
	NES_IRW__( Rra, Abs,      0x6F )	 
	NES_IRW__( Rra, AbsX,     0x7F )  
	NES_IRW__( Rra, AbsY,     0x7B )	 
	NES_IRW__( Rra, IndX,     0x63 )	 
	NES_IRW__( Rra, IndY,     0x73 )	 	
	NES_I__C_( Rti,        6, 0x40 )	 
	NES_I__C_( Rts,        6, 0x60 )  
	NES_I_W__( Sax, Zpg,      0x87 )	 
	NES_I_W__( Sax, ZpgY,     0x97 )  
	NES_I_W__( Sax, Abs,      0x8F )	 
	NES_I_W__( Sax, IndX,     0x83 )	 
	NES_IR___( Sbc, Imm,      0xE9 )	 
	NES_IR___( Sbc, Imm,      0xEB )	 
	NES_IR___( Sbc, Zpg,      0xE5 )	    
	NES_IR___( Sbc, ZpgX,     0xF5 )	 
	NES_IR___( Sbc, Abs,      0xED )	    
	NES_IR___( Sbc, AbsX,     0xFD )	 
	NES_IR___( Sbc, AbsY,     0xF9 )	 
	NES_IR___( Sbc, IndX,     0xE1 )	 
	NES_IR___( Sbc, IndY,     0xF1 )	 
	NES_IR___( Sbx, Imm,      0xCB )  
	NES_I__C_( Sec,        2, 0x38 )	 
	NES_I__C_( Sed,        2, 0xF8 )  
	NES_I__C_( Sei,        2, 0x78 )  
	NES_I_W_A( Sha, AbsY,     0x9F )	 
	NES_I_W_A( Sha, IndY,     0x93 )  
	NES_I_W_A( Shs, AbsY,     0x9B )  
	NES_I_W_A( Shx, AbsY,     0x9E )  
	NES_I_W_A( Shy, AbsX,     0x9C )  
	NES_IRW__( Slo, Zpg,      0x07 )  
	NES_IRW__( Slo, ZpgX,     0x17 )	 
	NES_IRW__( Slo, Abs,      0x0F )	 
	NES_IRW__( Slo, AbsX,     0x1F )	 
	NES_IRW__( Slo, AbsY,     0x1B )	 
	NES_IRW__( Slo, IndX,     0x03 )	 
	NES_IRW__( Slo, IndY,     0x13 )	 
	NES_IRW__( Sre, Zpg,      0x47 )	 
	NES_IRW__( Sre, ZpgX,     0x57 )	 
	NES_IRW__( Sre, Abs,      0x4F )	 
	NES_IRW__( Sre, AbsX,     0x5F )	 
	NES_IRW__( Sre, AbsY,     0x5B )	 
	NES_IRW__( Sre, IndX,     0x43 )	 
	NES_IRW__( Sre, IndY,     0x53 )	 
	NES_I_W__( Sta, Zpg,      0x85 )	 
	NES_I_W__( Sta, ZpgX,     0x95 )	 
	NES_I_W__( Sta, Abs,      0x8D )	 
	NES_I_W__( Sta, AbsX,     0x9D )	 
	NES_I_W__( Sta, AbsY,     0x99 )	 
	NES_I_W__( Sta, IndX,     0x81 )	 
	NES_I_W__( Sta, IndY,     0x91 )	 
	NES_I_W__( Stx, Zpg,      0x86 )	 
	NES_I_W__( Stx, ZpgY,     0x96 )	 
	NES_I_W__( Stx, Abs,      0x8E )	 
	NES_I_W__( Sty, Zpg,      0x84 )	 
	NES_I_W__( Sty, ZpgX,     0x94 )	 
	NES_I_W__( Sty, Abs,      0x8C )	 
	NES_I__C_( Tax,        2, 0xAA )	 
	NES_I__C_( Tay,        2, 0xA8 )	 
	NES_IR___( Top, Abs,      0x0C )	 
	NES_IR___( Top, AbsX,     0x1C )	 
	NES_IR___( Top, AbsX,     0x3C )	 
	NES_IR___( Top, AbsX,     0x5C )	 
	NES_IR___( Top, AbsX,     0x7C )	 
	NES_IR___( Top, AbsX,     0xDC )	 
	NES_IR___( Top, AbsX,     0xFC )	 
	NES_I__C_( Txa,        2, 0x8A )	 
	NES_I__C_( Txs,        2, 0x9A )	 
	NES_I__C_( Tsx,        2, 0xBA )	 
	NES_I__C_( Tya,        2, 0x98 )	 

    #undef rwAcc
    #undef sZpgX
    #undef sZpgY
    #undef sAbs 
    #undef sAbsX
    #undef sAbsY
    #undef sIndX
    #undef sIndY
    #undef sAcc

    #undef NES_I____
    #undef NES____C_
    #undef NES_I__C_
    #undef NES_IR___
    #undef NES_I_W__
    #undef NES_IRW__
    #undef NES_I_W_A

    #undef NES_EAT_CYCLES

	struct FLAGS   
	{	
		enum	   
		{		   
			C = 0x01,  // carry
			Z = 0x02,  // zero
			I = 0x04,  // interrupt enable/disable
			D = 0x08,  // decimal mode (not supported on the N2A03)
			B = 0x10,  // software interrupt
			R = 0x20,  // not used but always set
			V = 0x40,  // overflow
			N = 0x80   // sign
		};

		UINT Pack() const
		{
			// The breakpoint bit will be set on BRK and PHP 
			// and cleared on IRQ and NMI. Leave it unset here 
			// and let the caller decide if to invoke it or not.

			return
			(
       			(c.d ? C : 0) |
				(z.d ? 0 : Z) |
				(i.d ? I : 0) |
				(d.d ? D : 0) |
				(v.d ? V : 0) |
				(n.d & N) |
				R
			);
		}

		VOID Unpack(const UINT f)
		{
			c.d =  (f & C);
			z.d = !(f & Z);
			i.d =  (f & I);
			d.d =  (f & D);
			v.d =  (f & V);	
			n.d =  (f & N);	
		}

		PDXWORD c;
		PDXWORD z;
		PDXWORD i;
		PDXWORD d;
		PDXWORD v;
		PDXWORD n;
	};

	LONG NmiClock;

	UINT IntEn;
	UINT IntLow;
	UINT IntState;

	const ULONG* PDX_RESTRICT CyclePtr;
	ULONG cycles;	

	UINT cache;

	CPU_MAP map;

	PDXWORD pc;
	PDXWORD a;
	PDXWORD x;
	PDXWORD y;		  
	PDXWORD sp;	
	FLAGS flags;		  
		
	LONG FrameClock;
	LONG DmcDmaClock;
	UINT DmcLengthCounter;

	EVENTS events;

	UINT  status;
	ULONG FrameCycles;
	BOOL  jammed;
	UINT  pal;

	U8 ram[RAM_SIZE];
	
	APU apu;

	bool logged[24];

	static const ULONG CycleTable[2][8];

	typedef VOID (CPU::*INSTRUCTION)();
	static const INSTRUCTION instructions[256];

	struct HEADER
	{
		U16 pc;
		U16 ea;
		U8  a;
		U8  x;
		U8  y;
		U8  t;
		U8  sp;
		U8  flags;
		U32 cycles;
		U8  IntLow;
		U8  IntEn;
		U8  jammed;
	};
};

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#include "NstCpu.inl"

NES_NAMESPACE_END

#endif

