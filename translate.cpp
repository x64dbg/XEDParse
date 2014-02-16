#include "translate.h"
extern "C"
{
#include "xed2\include\xed-interface.h"
}
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
    //initialize encoder request
    xed_state_t dstate;
#ifdef _WIN64
    dstate.mmode=XED_MACHINE_MODE_LONG_64;
#else
    dstate.mmode=XED_MACHINE_MODE_LEGACY_32;
#endif //_WIN64
    dstate.stack_addr_width=XED_ADDRESS_WIDTH_32b;
    xed_encoder_request_t req;
    xed_encoder_request_zero_set_mode(&req, &dstate);
    //set prefix
    switch(instruction->prefix)
    {
    case PREFIX_LOCK:
        xed_encoder_request_set_lock(&req);
        break;
    case PREFIX_REP:
        xed_encoder_request_set_rep(&req);
        break;
    case PREFIX_REPNEZ:
        xed_encoder_request_set_repne(&req);
        break;
    }
    //override instruction mode (for x64 mainly)
#ifdef _WIN64
    switch(instruction->operand1.type)
    {
    case TYPE_REGISTER:
        if(instruction->operand1.u.reg.size==SIZE_QWORD) //example: mov rax, rbx
            xed_encoder_request_set_effective_operand_width(&req, 64);
        break;
    case TYPE_MEMORY:
        if(instruction->operand1.u.mem.size==SIZE_QWORD) //example: mov qword [rax], rbx
            xed_encoder_request_set_effective_operand_width(&req, 64);
        break;
    default:
        break;
    }
#endif //_WIN64
    //get instruction class
    _strupr(instruction->mnemonic);
    xed_iclass_enum_t iclass=str2xed_iclass_enum_t(instruction->mnemonic);
    if(iclass==XED_ICLASS_INVALID) //unknown instruction
    {
        sprintf(XEDParse->error, "unknown instruction \"%s\"!", instruction->mnemonic);
        return false;
    }
    //set instruction class
    xed_encoder_request_set_iclass(&req, iclass);
    return true;
}
