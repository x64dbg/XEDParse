#include "translate.h"
#include <stdio.h>

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

        if(rc == XED_REG_CLASS_GPR32 || rci == XED_REG_CLASS_GPR32)
            xed_encoder_request_set_effective_address_size(req, 32);
        if(rc == XED_REG_CLASS_GPR16 || rci == XED_REG_CLASS_GPR16)
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
    case TYPE_REGISTER:
        return operand->u.reg.size;
    case TYPE_MEMORY:
        return operand->u.mem.size;
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
		xed_encoder_request_set_effective_operand_width(req, opsizetobits(instruction->operand1.u.reg.size));
        break;

    case TYPE_MEMORY:
        if(instruction->operand1.u.mem.size==SIZE_WORD) //example: mov word [eax], 1234
            xed_encoder_request_set_effective_operand_width(req, 16);
#ifdef _WIN64
        if(instruction->operand1.u.mem.size==SIZE_QWORD) //example: mov qword [rax], rbx
            xed_encoder_request_set_effective_operand_width(req, 64);
#endif //_WIN64
		if (instruction->operand1.u.mem.size == SIZE_DQWORD) //example: movss xmm0, [eax]
			xed_encoder_request_set_effective_operand_width(req, 128);
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
