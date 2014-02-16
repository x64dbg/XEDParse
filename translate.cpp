#include "translate.h"

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
        "-",
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

bool translate(XEDPARSE* XEDParse, INSTRUCTION* instruction, TRANSLATION* translation)
{
    if(instruction->operand1.type==TYPE_NONE && instruction->operand2.type==TYPE_NONE) //no arguments
    {
        strcpy(translation->instr, instruction->mnemonic);
        return true;
    }
    strcpy(XEDParse->error, "could not translate instruction!");
    return false;
}
