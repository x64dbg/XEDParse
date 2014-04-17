#include "..\XEDParse.h"
#include <stdio.h>

#define CHECK(instr, code) TestInstruction(instr, code)

ULONG64 XEDAssembleString(const char *Instruction)
{
	XEDPARSE parse;
	memset(&parse, 0, sizeof(parse));

	strcpy(parse.instr, Instruction);

	// Parse the instruction
	if (XEDParseAssemble(&parse) != XEDPARSE_OK)
		return 0;

	// Zero any invalid data
	memset((PBYTE)&parse.dest + parse.dest_size, 0, sizeof(parse.dest) - parse.dest_size);

	// Convert the bytes to an unsigned long long
	return *(ULONG64 *)&parse.dest;
}

void TestInstruction(const char *Instruction, ULONG64 Expected)
{
	ULONG64 code = XEDAssembleString(Instruction);

	if (code != Expected)
		printf("'%s' => 0x%llX\t\t\tFAILED\tExpected 0x%llX\n", Instruction, code, Expected);
	else
		printf("'%s' => 0x%llX\t\t\tSUCCESS\n", Instruction, code);
}

// Tests single instructions such as ret/nop/int3/sti/hlt/leave/pushad
bool SingleInstruction()
{
	CHECK("ret",	0xC3);
	CHECK("nop",	0x90);
	CHECK("int3",	0xCC);
	CHECK("sti",	0xFB);
	CHECK("hlt",	0xF4);

#ifndef _WIN64
	CHECK("leave",	0xC9);
	CHECK("pushad", 0xF4);
#endif

	return true;
}

// Tests control registers and debug registers
bool CrDrInstruction()
{
	CHECK("mov cr0, eax", 0xC022F2);
	CHECK("mov dr0, eax", 0xC023F2);

#ifdef _WIN64
	CHECK("mov cr0, rax", 0xC0220F);
	CHECK("mov dr0, rax", 0xC0230F);
#endif

	return true;
}

// Tests mov reg, reg
bool MovRegRegInstruction()
{
	CHECK("mov eax, eax",	0xC089);
	CHECK("mov edx, eax",	0xC289);
	CHECK("mov ebp, ecx",	0xCD89);

#ifdef _WIN64
	CHECK("mov rax, rax",	0xC08948);
	CHECK("mov rdx, rax",	0xC28948);
	CHECK("mov r9, r14",	0xF1894D);
#endif

	return true;
}

// Tests mov [mem], reg
bool MovMemRegInstruction()
{
	CHECK("mov [0], eax",		0x00000000A3);
	CHECK("mov eax, [0]",		0x00000000A1);
	CHECK("mov fs:[0], eax",	0x00000000A364);
	CHECK("mov eax, fs:[0]",	0x00000000A164);

#ifdef _WIN64
	CHECK("mov [0], rax",		0x0000000025048948);
	CHECK("mov rax, [0]",		0x0000000025048B48);
	CHECK("mov gs:[0], rax",	0x000000002504894865);
	CHECK("mov rax, gs:[0]",	0x0000000025048B4865);
#endif

	return true;
}

// Tests mov reg, imm
bool MovImmRegInstruction()
{
	CHECK("mov eax, 1",			0x01B8);
	CHECK("mov ecx, 11223344",	0x11223344B9);

#ifdef _WIN64
	CHECK("mov rax, 1",			0x01B848);
	CHECK("mov rcx, 11223344",	0x11223344B948);
#endif

	return true;
}

// Tests mov xmm#, xmm#
bool MovXmmXmmInstruction()
{
	CHECK("movss xmm0, xmm5",	0xE8110FF3);
	CHECK("movss xmm1, xmm6",	0xF1110FF3);
	CHECK("mulps xmm1, xmm6",	0xCE590F);

#ifdef _WIN64
	CHECK("movss xmm14, xmm12", 0);
	CHECK("movss xmm1, xmm10",	0);
	CHECK("mulps xmm1, xmm10",	0);
#endif

	return true;
}

// Tests mov xmm#, [mem]
bool MovMemXmmInstruction()
{
	CHECK("movss xmm0, [0]",	0x0000000005100FF3);
	CHECK("movss [0], xmm7",	0x000000003D110FF3);

#ifdef _WIN64
	CHECK("movss xmm10, [0]", 0);
	CHECK("movss [0], xmm13", 0);
#endif

	return true;
}

// Tests jmp and call
bool JumpInstruction()
{
	// CHECK("jmp 0", 0x00000000E9);	// Invalid (depends on address)
	// CHECK("call 1", 0x00000000E9);	// Invalid (depends on address)
	CHECK("jmp eax", 0xE0FF);
	CHECK("call eax", 0xD0FF);
	//CHECK("jmp [0]", 0x0000000025FF);	// CRASH

#ifdef _WIN64
	CHECK("jmp rax", 0xE0FF);
	CHECK("call rax", 0xD0FF);
	//CHECK("jmp [0]", 0x000000002524FF);	// CRASH
#endif

	return true;
}