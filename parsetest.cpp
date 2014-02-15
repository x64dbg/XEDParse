#include "parsetest.h"
#include <stdio.h>

static const char* sizetostring(OPSIZE size)
{
    const char* sizelist[]=
    {
        "byte",
        "word",
        "dword",
#ifdef _WIN64
        "qword"
#endif // _WIN64
    };
    return sizelist[size];
}

static const char* sizedtostring(OPSIZE size)
{
    const char* sizelist[]=
    {
        "1",
        "2",
        "4",
#ifdef _WIN64
        "8"
#endif // _WIN64
    };
    return sizelist[size];
}


static const char* regtostring(REG reg)
{
    const char* reglist[]=
    {
        "nan",
        "EAX",
        "AX",
        "AH",
        "AL",
        "EBX",
        "BX",
        "BH",
        "BL",
        "ECX",
        "CX",
        "CH",
        "CL",
        "EDX",
        "DX",
        "DH",
        "DL",
        "EDI",
        "DI",
        "ESI",
        "SI",
        "EBP",
        "BP",
        "ESP",
        "SP",
#ifdef _WIN64
        "RAX",
        "RBX",
        "RCX",
        "RDX",
        "RSI",
        "SIL",
        "RDI",
        "DIL",
        "RBP",
        "BPL",
        "RSP",
        "SPL",
        "RIP",
        "R8",
        "R8D",
        "R8W",
        "R8B",
        "R9",
        "R9D",
        "R9W",
        "R9B",
        "R10",
        "R10D",
        "R10W",
        "R10B",
        "R11",
        "R11D",
        "R11W",
        "R11B",
        "R12",
        "R12D",
        "R12W",
        "R12B",
        "R13",
        "R13D",
        "R13W",
        "R13B",
        "R14",
        "R14D",
        "R14W",
        "R14B",
        "R15",
        "R15D",
        "R15W",
        "R15B"
#endif //_WIN64
    };
    return reglist[reg];
}

static const char* segtostring(SEG seg)
{
    const char* seglist[]=
    {
        "cs",
        "ds",
        "es",
        "fs",
        "gs",
        "ss"
    };
    return seglist[seg];
}

static const char* prefixtostring(PREFIX prefix)
{
    const char* prefixlist[]=
    {
        "",
        "lock ",
        "rep ",
        "repe ",
        "repne "
    };
    return prefixlist[prefix];
}

static void operandtostring(OPERAND* operand, char* str)
{
    switch(operand->type)
    {
    case TYPE_NONE:
    {
        *str=0;
    }
    break;

    case TYPE_REGISTER:
    {
        sprintf(str, "%s", regtostring(operand->u.reg));
    }
    break;

    case TYPE_VALUE:
    {
#ifdef _WIN64
        sprintf(str, "%llX.%s",
#else
        sprintf(str, "%X.%s",
#endif //_WIN64
                operand->u.val.val,
                sizedtostring(operand->u.val.size));
    }
    break;

    case TYPE_MEMORY:
    {
        char base[20]="";
        char indexscale[20]="";
        if(operand->u.mem.base!=REG_NAN)
            sprintf(base, "%s+",
                    regtostring(operand->u.mem.base));
        if(operand->u.mem.index!=REG_NAN)
            sprintf(indexscale, "%s*%s+",
                    regtostring(operand->u.mem.index),
                    sizedtostring(operand->u.mem.scale));
#ifdef _WIN64
        sprintf(str, "%s ptr %s:[%s%s%llX.%s]",
#else
        sprintf(str, "%s ptr %s:[%s%s%X.%s]",
#endif //_WIN64
                sizetostring(operand->u.mem.size),
                segtostring(operand->u.mem.seg),
                base,
                indexscale,
                operand->u.mem.displ.val,
                sizedtostring(operand->u.mem.displ.size));
    }
    break;
    }
}

void parsedisasm(INSTRUCTION* parsed)
{
    char operand1[256]="";
    operandtostring(&parsed->operand1, operand1);
    _strlwr(operand1);
    char operand2[256]="";
    operandtostring(&parsed->operand2, operand2);
    _strlwr(operand2);
    printf("%s%s", prefixtostring(parsed->prefix), parsed->mnemonic);
    if(*operand1)
        printf(" %s", operand1);
    if(*operand2)
        printf(",%s", operand2);
    puts("");
}
