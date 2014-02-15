#ifndef _PARSER_H
#define _PARSER_H

#include "XEDParse.h"

//enums
enum PREFIX
{
    PREFIX_NONE,
    PREFIX_LOCK,
    PREFIX_REP,
    PREFIX_REPEZ,
    PREFIX_REPNEZ
};

enum OPTYPE
{
    TYPE_NONE,
    TYPE_VALUE,
    TYPE_REGISTER,
    TYPE_MEMORY
};

enum OPSIZE
{
    SIZE_BYTE,
    SIZE_WORD,
    SIZE_DWORD,
#ifdef _WIN64
    SIZE_QWORD,
#endif //_WIN64
    SIZE_UNSET
};

typedef OPSIZE MEMSIZE;
typedef OPSIZE SCALE;
typedef OPSIZE REGSIZE;

enum REG
{
    REG_NAN,
    REG_EAX,
    REG_AX,
    REG_AH,
    REG_AL,
    REG_EBX,
    REG_BX,
    REG_BH,
    REG_BL,
    REG_ECX,
    REG_CX,
    REG_CH,
    REG_CL,
    REG_EDX,
    REG_DX,
    REG_DH,
    REG_DL,
    REG_EDI,
    REG_DI,
    REG_ESI,
    REG_SI,
    REG_EBP,
    REG_BP,
    REG_ESP,
    REG_SP,
#ifdef _WIN64
    REG_RAX,
    REG_RBX,
    REG_RCX,
    REG_RDX,
    REG_RSI,
    REG_SIL,
    REG_RDI,
    REG_DIL,
    REG_RBP,
    REG_BPL,
    REG_RSP,
    REG_SPL,
    REG_RIP,
    REG_R8,
    REG_R8D,
    REG_R8W,
    REG_R8B,
    REG_R9,
    REG_R9D,
    REG_R9W,
    REG_R9B,
    REG_R10,
    REG_R10D,
    REG_R10W,
    REG_R10B,
    REG_R11,
    REG_R11D,
    REG_R11W,
    REG_R11B,
    REG_R12,
    REG_R12D,
    REG_R12W,
    REG_R12B,
    REG_R13,
    REG_R13D,
    REG_R13W,
    REG_R13B,
    REG_R14,
    REG_R14D,
    REG_R14W,
    REG_R14B,
    REG_R15,
    REG_R15D,
    REG_R15W,
    REG_R15B
#endif //_WIN64
};

enum SEG
{
    SEG_CS,
    SEG_DS,
    SEG_ES,
    SEG_FS,
    SEG_GS,
    SEG_SS
};

//structures
struct OPVAL
{
    OPSIZE size; //value size
    ULONG_PTR val; //actual data
};

struct OPMEM //size seg:[base+index*scale+displ]
{
    MEMSIZE size; //byte/word/dword/qword
    SEG seg; //segment
    REG base; //base register
    REG index; //index register
    SCALE scale; //scale
    OPVAL displ; //displacement
};

struct OPREG
{
    REGSIZE size;
    REG reg;
};

struct OPERAND
{
    char raw[XEDPARSE_MAXBUFSIZE/2]; //raw text
    OPTYPE type; //operand type
    union
    {
        OPVAL val;
        OPREG reg;
        OPMEM mem;
    } u;
};


struct INSTRUCTION
{
    PREFIX prefix;
    char mnemonic[10];
    OPERAND operand1;
    OPERAND operand2;
};

//functions
bool parse(XEDPARSE* raw, INSTRUCTION* parsed);

#endif // _PARSER_H
