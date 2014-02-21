#include "translate.h"
extern "C"
{
#include "xed2\include\xed-interface.h"
}
#include <stdio.h>

static xed_reg_enum_t regtoxed(REG reg)
{
    switch(reg)
    {
    case REG_EAX:
        return XED_REG_EAX;
        break;
    case REG_AX:
        return XED_REG_AX;
        break;
    case REG_AH:
        return XED_REG_AH;
        break;
    case REG_AL:
        return XED_REG_AL;
        break;
    case REG_EBX:
        return XED_REG_EBX;
        break;
    case REG_BX:
        return XED_REG_BX;
        break;
    case REG_BH:
        return XED_REG_BH;
        break;
    case REG_BL:
        return XED_REG_BL;
        break;
    case REG_ECX:
        return XED_REG_ECX;
        break;
    case REG_CX:
        return XED_REG_CX;
        break;
    case REG_CH:
        return XED_REG_CH;
        break;
    case REG_CL:
        return XED_REG_CL;
        break;
    case REG_EDX:
        return XED_REG_EDX;
        break;
    case REG_DX:
        return XED_REG_DX;
        break;
    case REG_DH:
        return XED_REG_DH;
        break;
    case REG_DL:
        return XED_REG_DL;
        break;
    case REG_EDI:
        return XED_REG_EDI;
        break;
    case REG_DI:
        return XED_REG_DI;
        break;
    case REG_ESI:
        return XED_REG_ESI;
        break;
    case REG_SI:
        return XED_REG_SI;
        break;
    case REG_EBP:
        return XED_REG_EBP;
        break;
    case REG_BP:
        return XED_REG_BP;
        break;
    case REG_ESP:
        return XED_REG_ESP;
        break;
    case REG_SP:
        return XED_REG_SP;
        break;
#ifndef _WIN64 //x86 only
    case REG_CS:
        return XED_REG_CS;
        break;
    case REG_DS:
        return XED_REG_DS;
        break;
    case REG_ES:
        return XED_REG_ES;
        break;
    case REG_FS:
        return XED_REG_FS;
        break;
    case REG_GS:
        return XED_REG_GS;
        break;
    case REG_SS:
        return XED_REG_SS;
        break;
#endif //_WIN64
#ifdef _WIN64
    case REG_RAX:
        return XED_REG_RAX;
        break;
    case REG_RBX:
        return XED_REG_RBX;
        break;
    case REG_RCX:
        return XED_REG_RCX;
        break;
    case REG_RDX:
        return XED_REG_RDX;
        break;
    case REG_RSI:
        return XED_REG_RSI;
        break;
    case REG_SIL:
        return XED_REG_SIL;
        break;
    case REG_RDI:
        return XED_REG_RDI;
        break;
    case REG_DIL:
        return XED_REG_DIL;
        break;
    case REG_RBP:
        return XED_REG_RBP;
        break;
    case REG_BPL:
        return XED_REG_BPL;
        break;
    case REG_RSP:
        return XED_REG_RSP;
        break;
    case REG_SPL:
        return XED_REG_SPL;
        break;
    case REG_RIP:
        return XED_REG_RIP;
        break;
    case REG_R8:
        return XED_REG_R8;
        break;
    case REG_R8D:
        return XED_REG_R8D;
        break;
    case REG_R8W:
        return XED_REG_R8W;
        break;
    case REG_R8B:
        return XED_REG_R8B;
        break;
    case REG_R9:
        return XED_REG_R9;
        break;
    case REG_R9D:
        return XED_REG_R9D;
        break;
    case REG_R9W:
        return XED_REG_R9W;
        break;
    case REG_R9B:
        return XED_REG_R9B;
        break;
    case REG_R10:
        return XED_REG_R10;
        break;
    case REG_R10D:
        return XED_REG_R10D;
        break;
    case REG_R10W:
        return XED_REG_R10W;
        break;
    case REG_R10B:
        return XED_REG_R10B;
        break;
    case REG_R11:
        return XED_REG_R11;
        break;
    case REG_R11D:
        return XED_REG_R11D;
        break;
    case REG_R11W:
        return XED_REG_R11W;
        break;
    case REG_R11B:
        return XED_REG_R11B;
        break;
    case REG_R12:
        return XED_REG_R12;
        break;
    case REG_R12D:
        return XED_REG_R12D;
        break;
    case REG_R12W:
        return XED_REG_R12W;
        break;
    case REG_R12B:
        return XED_REG_R12B;
        break;
    case REG_R13:
        return XED_REG_R13;
        break;
    case REG_R13D:
        return XED_REG_R13D;
        break;
    case REG_R13W:
        return XED_REG_R13W;
        break;
    case REG_R13B:
        return XED_REG_R13B;
        break;
    case REG_R14:
        return XED_REG_R14;
        break;
    case REG_R14D:
        return XED_REG_R14D;
        break;
    case REG_R14W:
        return XED_REG_R14W;
        break;
    case REG_R14B:
        return XED_REG_R14B;
        break;
    case REG_R15:
        return XED_REG_R15;
        break;
    case REG_R15D:
        return XED_REG_R15D;
        break;
    case REG_R15W:
        return XED_REG_R15W;
        break;
    case REG_R15B:
        return XED_REG_R15B;
        break;
#endif //_WIN64
    default:
        break;
    }
    return XED_REG_INVALID;
}

static unsigned int opsizetobits(OPSIZE opsize)
{
    switch(opsize)
    {
    case SIZE_BYTE:
        return 8;
        break;
    case SIZE_WORD:
        return 16;
        break;
    case SIZE_DWORD:
        return 32;
        break;
#ifdef _WIN64
    case SIZE_QWORD:
        return 64;
        break;
#endif //_WIN64
    default:
        break;
    }
#ifdef _WIN64
    return 64;
#else
    return 32;
#endif //_WIN64
}

static int opsizetoint(OPSIZE opsize)
{
    switch(opsize)
    {
    case SIZE_BYTE:
        return 1;
        break;
    case SIZE_WORD:
        return 2;
        break;
    case SIZE_DWORD:
        return 4;
        break;
#ifdef _WIN64
    case SIZE_QWORD:
        return 8;
        break;
#endif //_WIN64
    }
    return 1;
}

static xed_reg_enum_t segtoxed(SEG seg)
{
    switch(seg)
    {
    case SEG_CS:
        return XED_REG_CS;
        break;
    case SEG_DS:
        return XED_REG_DS;
        break;
    case SEG_ES:
        return XED_REG_ES;
        break;
    case SEG_FS:
        return XED_REG_FS;
        break;
    case SEG_GS:
        return XED_REG_GS;
        break;
    case SEG_SS:
        return XED_REG_SS;
        break;
    }
    return XED_REG_INVALID;
}

static bool translateoperand(XEDPARSE* XEDParse, TRANSOP* transop, xed_encoder_request_t* req)
{
    //TODO: handle different instructions
    switch(transop->operand->type)
    {
    case TYPE_NONE:
    {
        return true;
    }
    break;

    case TYPE_VALUE:
    {
        xed_iclass_enum_t iclass=xed_encoder_request_get_iclass(req);
        if(iclass==XED_ICLASS_RET_NEAR || iclass==XED_ICLASS_RET_FAR) //ret imm16
            transop->operand->u.val.size=SIZE_WORD;
        xed_encoder_request_set_uimm0_bits(req, transop->operand->u.val.val, opsizetobits(transop->operand->u.val.size));
        xed_encoder_request_set_operand_order(req, transop->operand_index, XED_OPERAND_IMM0);
        transop->operand_index++;
        return true;
    }
    break;

    case TYPE_REGISTER:
    {
        xed_operand_enum_t r=XED_STATIC_CAST(xed_operand_enum_t, XED_OPERAND_REG0+transop->regop);
        xed_encoder_request_set_reg(req, r, regtoxed(transop->operand->u.reg.reg));
        xed_encoder_request_set_operand_order(req, transop->operand_index, r);
        transop->regop++;
        transop->operand_index++;
        return true;
    }
    break;

    case TYPE_MEMORY:
    {
        xed_iclass_enum_t iclass=xed_encoder_request_get_iclass(req);
        xed_reg_enum_t seg=segtoxed(transop->operand->u.mem.seg);
        if(iclass==XED_ICLASS_LEA) //LEA uses a special memory operand
        {
            xed_encoder_request_set_agen(req);
            xed_encoder_request_set_operand_order(req, transop->operand_index, XED_OPERAND_AGEN);
            seg=XED_REG_INVALID; //AGEN cannot have segment stuff
        }
        else //normal memory operand
        {
            if(!transop->memop) //first memory operand
            {
                xed_encoder_request_set_mem0(req);
                xed_encoder_request_set_operand_order(req, transop->operand_index, XED_OPERAND_MEM0);
            }
            else //second memory operand
            {
                xed_encoder_request_set_mem1(req);
                xed_encoder_request_set_operand_order(req, transop->operand_index, XED_OPERAND_MEM1);
            }
            xed_encoder_request_set_memory_operand_length(req, opsizetoint(transop->operand->u.mem.size));
            transop->memop++;
        }
        xed_reg_enum_t rbase=regtoxed(transop->operand->u.mem.base);
        xed_reg_enum_t rindex=regtoxed(transop->operand->u.mem.index);

        xed_reg_class_enum_t rc = xed_gpr_reg_class(rbase);
        xed_reg_class_enum_t rci = xed_gpr_reg_class(rindex);

        if(rc==XED_REG_CLASS_GPR32 || rci==XED_REG_CLASS_GPR32)
            xed_encoder_request_set_effective_address_size(req, 32);
        if(rc==XED_REG_CLASS_GPR16 || rci==XED_REG_CLASS_GPR16)
            xed_encoder_request_set_effective_address_size(req, 16);

        xed_encoder_request_set_base0(req, rbase);
        xed_encoder_request_set_index(req, rindex);
        xed_encoder_request_set_scale(req, opsizetoint(transop->operand->u.mem.scale));

        xed_encoder_request_set_seg0(req, seg);
        if(transop->operand->u.mem.displ.val)
            xed_encoder_request_set_memory_displacement(req, transop->operand->u.mem.displ.val, opsizetoint(transop->operand->u.mem.displ.size));
        transop->operand_index++;
        return true;
    }
    break;

    default:
    {
    }
    break;
    }
    return false;
}

static void translatemnemonic(char* mnemonic)
{
    if(!_stricmp(mnemonic, "retf"))
        strcpy(mnemonic, "ret_far");
    else if(!_stricmp(mnemonic, "ret") || !_stricmp(mnemonic, "retn")) //ret/retn
        strcpy(mnemonic, "ret_near");
    else if(!_stricmp(mnemonic, "call")) //call
        strcpy(mnemonic, "call_near");
    else if(!_stricmp(mnemonic, "pushf")) //pushfd
        strcpy(mnemonic, "pushfd");
    else if(!_stricmp(mnemonic, "popf")) //popfd
        strcpy(mnemonic, "popfd");
    else if(!_stricmp(mnemonic, "pusha")) //pushad
        strcpy(mnemonic, "pushad");
    else if(!_stricmp(mnemonic, "popa")) //popad
        strcpy(mnemonic, "popad");
    _strupr(mnemonic);
}

static OPSIZE getopsize(OPERAND* operand)
{
    switch(operand->type)
    {
    case TYPE_NONE:
        break;
    case TYPE_VALUE:
        return operand->u.val.size;
        break;
    case TYPE_REGISTER:
        return operand->u.reg.size;
        break;
    case TYPE_MEMORY:
        return operand->u.mem.size;
        break;
    }
    return SIZE_BYTE;
}

static OPSIZE inttoopsize(int opsize)
{
    switch(opsize)
    {
    case 1:
        return SIZE_BYTE;
        break;
    case 2:
        return SIZE_WORD;
        break;
    case 4:
        return SIZE_DWORD;
        break;
#ifdef _WIN64
    case 8:
        return SIZE_QWORD;
        break;
#endif //_WIN64
    }
    return SIZE_BYTE;
}

static void setopsize(OPERAND* operand, int opsize)
{
    switch(operand->type)
    {
    case TYPE_NONE:
        break;
    case TYPE_VALUE:
        operand->u.val.size=inttoopsize(opsize);
        break;
    case TYPE_REGISTER:
        operand->u.reg.size=inttoopsize(opsize);
        break;
    case TYPE_MEMORY:
        operand->u.mem.size=inttoopsize(opsize);
        break;
    }
}

static void fixopsize(xed_iclass_enum_t iclass, INSTRUCTION* instruction)
{
    //TODO: fix operand sizes
    int opsize1=opsizetoint(getopsize(&instruction->operand1));
    int opsize2=opsizetoint(getopsize(&instruction->operand2));
    switch(instruction->operand1.type)
    {
    case TYPE_REGISTER:
        switch(instruction->operand2.type)
        {
        case TYPE_VALUE: //mov reg,value
            break;
        case TYPE_REGISTER: //mov reg,reg
            break;
        case TYPE_MEMORY: //mov reg,[]
            break;
        default:
            break;
        }
        break;
    case TYPE_MEMORY:
        switch(instruction->operand2.type)
        {
        case TYPE_VALUE: //mov [],value
            break;
        case TYPE_REGISTER: //mov [],reg
            break;
        case TYPE_MEMORY: //mov [],[]
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

static bool translatebase(XEDPARSE* XEDParse, INSTRUCTION* instruction, xed_encoder_request_t* req)
{
    //set instruction prefix
    switch(instruction->prefix)
    {
    case PREFIX_LOCK:
        xed_encoder_request_set_lock(req);
        break;
    case PREFIX_REP:
        xed_encoder_request_set_rep(req);
        break;
    case PREFIX_REPNEZ:
        xed_encoder_request_set_repne(req);
        break;
    }
    //override instruction mode
    switch(instruction->operand1.type)
    {
    case TYPE_REGISTER:
        if(instruction->operand1.u.reg.size==SIZE_WORD) //example: mov ax, 123
            xed_encoder_request_set_effective_operand_width(req, 16);
#ifdef _WIN64
        else if(instruction->operand1.u.reg.size==SIZE_QWORD) //example: mov rax, rbx
            xed_encoder_request_set_effective_operand_width(req, 64);
#endif //_WIN64
        break;
    case TYPE_MEMORY:
        if(instruction->operand1.u.mem.size==SIZE_WORD) //example: mov word [eax], 1234
            xed_encoder_request_set_effective_operand_width(req, 16);
#ifdef _WIN64
        if(instruction->operand1.u.mem.size==SIZE_QWORD) //example: mov qword [rax], rbx
            xed_encoder_request_set_effective_operand_width(req, 64);
#endif //_WIN64
        if(instruction->operand2.type==TYPE_MEMORY) //mov [],[]
        {
#ifdef _WIN64
            if(instruction->operand1.u.mem.base==REG_RDI && instruction->operand2.u.mem.base==REG_RSI)
#else
            if(instruction->operand1.u.mem.base==REG_EDI && instruction->operand2.u.mem.base==REG_ESI)
#endif //_WIN64
            {
                instruction->operand1.type=TYPE_NONE;
                instruction->operand2.type=TYPE_NONE;
                switch(instruction->operand1.u.mem.size)
                {
                case SIZE_BYTE:
                    strcpy(instruction->mnemonic, "movsb");
                    break;
                case SIZE_WORD:
                    strcpy(instruction->mnemonic, "movsw");
                    break;
                case SIZE_DWORD:
                    strcpy(instruction->mnemonic, "movsd");
                    break;
#ifdef _WIN64
                case SIZE_QWORD:
                    strcpy(instruction->mnemonic, "movsq");
                    break;
#endif //_WIN64
                }
            }
        }
        break;
    default:
        break;
    }
    //translate mnemonic
    translatemnemonic(instruction->mnemonic);
    //get instruction class
    xed_iclass_enum_t iclass=str2xed_iclass_enum_t(instruction->mnemonic);
    //check instruction class
    if(iclass==XED_ICLASS_INVALID) //unknown instruction
    {
        sprintf(XEDParse->error, "unknown instruction \"%s\"!", instruction->mnemonic);
        return false;
    }
    else if(iclass==XED_ICLASS_RET_NEAR || iclass==XED_ICLASS_RET_FAR) //some instructions need x32 mode
    {
        xed_state_t dstate;
        dstate.mmode=XED_MACHINE_MODE_LEGACY_32;
        dstate.stack_addr_width=XED_ADDRESS_WIDTH_32b;
        xed_encoder_request_zero_set_mode(req, &dstate);
    }
    //fix operand size (for example: add eax,12/1 is allowed, but mov eax,12/1 is not)
    fixopsize(iclass, instruction);
    //set instruction class
    xed_encoder_request_set_iclass(req, iclass);
    return true;
}

static void initrequest(xed_encoder_request_t* req)
{
    xed_tables_init(); //important stuff
    xed_state_t dstate;
#ifdef _WIN64
    dstate.mmode=XED_MACHINE_MODE_LONG_64;
#else
    dstate.mmode=XED_MACHINE_MODE_LEGACY_32;
#endif //_WIN64
    dstate.stack_addr_width=XED_ADDRESS_WIDTH_32b;
    xed_encoder_request_zero_set_mode(req, &dstate);
}

bool translate(XEDPARSE* XEDParse, INSTRUCTION* instruction)
{
    //initialize encoder request
    xed_encoder_request_t req;
    initrequest(&req);
    //translate base instruction
    if(!translatebase(XEDParse, instruction, &req))
        return false;
    //translate operands
    TRANSOP transop;
    memset(&transop, 0, sizeof(transop));
    transop.operand=&instruction->operand1;
    if(!translateoperand(XEDParse, &transop, &req))
        return false;
    transop.operand=&instruction->operand2;
    if(!translateoperand(XEDParse, &transop, &req))
        return false;
    //print request
    char buf[5000]="";
    xed_encode_request_print(&req, buf, 5000);
    puts(buf);
    //encode the final instruction
    xed_error_enum_t xed_error=xed_encode(&req, XEDParse->dest, XED_MAX_INSTRUCTION_BYTES, &XEDParse->dest_size);
    xed_bool_t encode_okay=(xed_error==XED_ERROR_NONE);
    if (!encode_okay)
    {
        strcpy(XEDParse->error, "failed to encode instruction!");
        return false;
    }
    return true;
}
