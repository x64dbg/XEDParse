#ifndef _X86REFERENCE_H
#define _X86REFERENCE_H

enum optype //operand type
{
    _none, //no operand
    _other, //other string is used
    _unknown, //unknown operand type
    _a, //Two one-word operands in memory or two double-word operands in memory, depending on operand-size attribute (only BOUND).
    _b, //Byte, regardless of operand-size attribute.
    _bcd, //Packed-BCD. Only x87 FPU instructions (for example, FBLD).
    _bs, //Byte, sign-extended to the size of the destination operand.
    _bsq, //(Byte, sign-extended to 64 bits).
    _bss, //Byte, sign-extended to the size of the stack pointer (for example, PUSH (6A)).
    _c, //Byte or word, depending on operand-size attribute. (unused even by Intel?).
    _d, //Doubleword, regardless of operand-size attribute.
    _di, //Doubleword-integer. Only x87 FPU instructions (for example, FIADD).
    _dq, //Double-quadword, regardless of operand-size attribute (for example, CMPXCHG16B).
    _dqp, //Doubleword, or quadword, promoted by REX.W in 64-bit mode (for example, MOVSXD).
    _dr, //Double-real. Only x87 FPU instructions (for example, FADD).
    _ds, //Doubleword, sign-extended to 64 bits (for example, CALL (E8).
    _e, //x87 FPU environment (for example, FSTENV).
    _er, //Extended-real. Only x87 FPU instructions (for example, FLD).
    _p, //32-bit or 48-bit pointer, depending on operand-size attribute (for example, CALLF (9A).
    _pi, //Quadword MMX technology data.
    _pd, //128-bit packed double-precision floating-point data.
    _ps, //128-bit packed single-precision floating-point data.
    _psq, //64-bit packed single-precision floating-point data.
    _pt, //(80-bit far pointer).
    _ptp, //32-bit or 48-bit pointer, depending on operand-size attribute, or 80-bit far pointer, promoted by REX.W in 64-bit mode (for example, CALLF (FF /3)).
    _q, //Quadword, regardless of operand-size attribute (for example, CALL (FF /2)).
    _qi, //Qword-integer. Only x87 FPU instructions (for example, FILD).
    _qp, //Quadword, promoted by REX.W (for example, IRETQ).
    _s, //6-byte pseudo-descriptor, or 10-byte pseudo-descriptor in 64-bit mode (for example, SGDT).
    _sd, //Scalar element of a 128-bit packed double-precision floating data.
    _si, //Doubleword integer register (e. g., eax). (unused even by Intel?).
    _sr, //Single-real. Only x87 FPU instructions (for example, FADD).
    _ss, //Scalar element of a 128-bit packed single-precision floating data.
    _st, //x87 FPU state (for example, FSAVE).
    _stx, //x87 FPU and SIMD state (FXSAVE and FXRSTOR).
    _t, //10-byte far pointer.
    _v, //Word or doubleword, depending on operand-size attribute (for example, INC (40), PUSH (50)).
    _vds, //Word or doubleword, depending on operand-size attribute, or doubleword, sign-extended to 64 bits for 64-bit operand size.
    _vq, //Quadword (default) or word if operand-size prefix is used (for example, PUSH (50)).
    _vqp, //Word or doubleword, depending on operand-size attribute, or quadword, promoted by REX.W in 64-bit mode.
    _vs, //Word or doubleword, depending on operand-size attribute, or quadword, promoted by REX.W in 64-bit mode.
    _w, //Word, regardless of operand-size attribute (for example, ENTER).
    _wi, //Word-integer. Only x87 FPU instructions (for example, FIADD).

    _va, //Word or doubleword, according to address-size attribute (only REP and LOOP families).
    _dqa, //Doubleword or quadword, according to address-size attribute (only REP and LOOP families).
    _wa, //Word, according to address-size attribute (only JCXZ instruction).
    _wo, //Word, according to current operand size (e. g., MOVSW instruction).
    _ws, //Word, according to current stack size (only PUSHF and POPF instructions in 64-bit mode).
    _da, //Doubleword, according to address-size attribute (only JECXZ instruction).
    _do, //Doubleword, according to current operand size (e. g., MOVSD instruction).
    _qa, //Quadword, according to address-size attribute (only JRCXZ instruction).
    _qs, //Quadword, according to current stack size (only PUSHFQ and POPFQ instructions).
};

enum addrmethod //addressing method
{
    _NONE, //no addressing method
    _UNKNOWN, //not specified
    _A, //Direct address. The instruction has no ModR/M byte; the address of the operand is encoded in the instruction; no base register, index register, or scaling factor can be applied (for example, far JMP (EA)).
    _BA, //Memory addressed by DS:EAX, or by rAX in 64-bit mode (only 0F01C8 MONITOR).
    _BB, //Memory addressed by DS:eBX+AL, or by rBX+AL in 64-bit mode (only XLAT). (This code changed from single B in revision 1.00).
    _BD, //Memory addressed by DS:eDI or by RDI (only 0FF7 MASKMOVQ and 660FF7 MASKMOVDQU) (This code changed from YD (introduced in 1.00) in revision 1.02).
    _C, //The reg field of the ModR/M byte selects a control register (only MOV (0F20, 0F22)).
    _D, //The reg field of the ModR/M byte selects a debug register (only MOV (0F21, 0F23)).
    _E, //A ModR/M byte follows the opcode and specifies the operand. The operand is either a general-purpose register or a memory address. If it is a memory address, the address is computed from a segment register and any of the following values: a base register, an index register, a scaling factor, or a displacement.
    _ES, //(Implies original E). A ModR/M byte follows the opcode and specifies the operand. The operand is either a x87 FPU stack register or a memory address. If it is a memory address, the address is computed from a segment register and any of the following values: a base register, an index register, a scaling factor, or a displacement.
    _EST, //(Implies original E). A ModR/M byte follows the opcode and specifies the x87 FPU stack register.
    _F, //rFLAGS register.
    _G, //The reg field of the ModR/M byte selects a general register (for example, AX (000)).
    _H, //The r/m field of the ModR/M byte always selects a general register, regardless of the mod field (for example, MOV (0F20)).
    _I, //Immediate data. The operand value is encoded in subsequent bytes of the instruction.
    _J, //The instruction contains a relative offset to be added to the instruction pointer register (for example, JMP (E9), LOOP)).
    _M, //The ModR/M byte may refer only to memory: mod != 11bin (BOUND, LEA, CALLF, JMPF, LES, LDS, LSS, LFS, LGS, CMPXCHG8B, CMPXCHG16B, F20FF0 LDDQU).
    _N, //The R/M field of the ModR/M byte selects a packed quadword MMX technology register.
    _O, //The instruction has no ModR/M byte; the offset of the operand is coded as a word, double word or quad word (depending on address size attribute) in the instruction. No base register, index register, or scaling factor can be applied (only MOV  (A0, A1, A2, A3)).
    _P, //The reg field of the ModR/M byte selects a packed quadword MMX technology register.
    _Q, //A ModR/M byte follows the opcode and specifies the operand. The operand is either an MMX technology register or a memory address. If it is a memory address, the address is computed from a segment register and any of the following values: a base register, an index register, a scaling factor, and a displacement.
    _R, //The mod field of the ModR/M byte may refer only to a general register (only MOV (0F20-0F24, 0F26)).
    _S, //The reg field of the ModR/M byte selects a segment register (only MOV (8C, 8E)).
    _SC, //Stack operand, used by instructions which either push an operand to the stack or pop an operand from the stack. Pop-like instructions are, for example, POP, RET, IRET, LEAVE. Push-like are, for example, PUSH, CALL, INT. No Operand type is provided along with this method because it depends on source/destination operand(s).
    _T, //The reg field of the ModR/M byte selects a test register (only MOV (0F24, 0F26)).
    _U, //The R/M field of the ModR/M byte selects a 128-bit XMM register.
    _V, //The reg field of the ModR/M byte selects a 128-bit XMM register.
    _W, //A ModR/M byte follows the opcode and specifies the operand. The operand is either a 128-bit XMM register or a memory address. If it is a memory address, the address is computed from a segment register and any of the following values: a base register, an index register, a scaling factor, and a displacement.
    _X, //Memory addressed by the DS:eSI or by RSI (only MOVS, CMPS, OUTS, and LODS). In 64-bit mode, only 64-bit (RSI) and 32-bit (ESI) address sizes are supported. In non-64-bit modes, only 32-bit (ESI) and 16-bit (SI) address sizes are supported.
    _Y, //Memory addressed by the ES:eDI or by RDI (only MOVS, CMPS, INS, STOS, and SCAS). In 64-bit mode, only 64-bit (RDI) and 32-bit (EDI) address sizes are supported. In non-64-bit modes, only 32-bit (EDI) and 16-bit (DI) address sizes are supported. The implicit ES segment register cannot be overriden by a segment prefix.
    _Z, //The instruction has no ModR/M byte; the three least-significant bits of the opcode byte selects a general-purpose register.

    _S2, //The two bits at bit index three of the opcode byte selects one of original four segment registers (for example, PUSH ES).
    _S30, //The three least-significant bits of the opcode byte selects segment register SS, FS, or GS (for example, LSS).
    _S33 //The three bits at bit index three of the opcode byte selects segment register FS or GS (for example, PUSH FS).
};

struct operand
{
    addrmethod _a;
    optype _t;
    char other[256];
};

struct instrinfo
{
    xed_iclass_enum_t iclass;
    operand op1;
    operand op2;
    operand op3;
    operand op4;
};

#include "x86reference_data.h"

#endif //_X86REFERENCE_H