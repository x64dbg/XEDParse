#include "parser.h"
#include <stdio.h>

static bool scmp(const char* a, const char* b)
{
    if(!_strnicmp(a, b, strlen(b)))
        return true;
    return false;
}

static PREFIX getprefix(const char* text, int* i)
{
    if(scmp(text, "lock "))
    {
        *i+=5;
        return PREFIX_LOCK;
    }
    else if(scmp(text, "rep "))
    {
        *i+=4;
        return PREFIX_REP;
    }
    else if(scmp(text, "repe ") || scmp(text, "repz "))
    {
        *i+=5;
        return PREFIX_REP;
    }
    else if(scmp(text, "repne ") || scmp(text, "repnz "))
    {
        *i+=6;
        return PREFIX_REPNEZ;
    }
    return PREFIX_NONE;
}

static REG getregister(const char* text)
{
    if(scmp(text, "eax"))
        return REG_EAX;
    else if(scmp(text, "ax"))
        return REG_AX;
    else if(scmp(text, "ah"))
        return REG_AH;
    else if(scmp(text, "al"))
        return REG_AL;
    else if(scmp(text, "ebx"))
        return REG_EBX;
    else if(scmp(text, "bx"))
        return REG_BX;
    else if(scmp(text, "bh"))
        return REG_BH;
    else if(scmp(text, "bl"))
        return REG_BL;
    else if(scmp(text, "ecx"))
        return REG_ECX;
    else if(scmp(text, "cx"))
        return REG_CX;
    else if(scmp(text, "ch"))
        return REG_CH;
    else if(scmp(text, "cl"))
        return REG_CL;
    else if(scmp(text, "edx"))
        return REG_EDX;
    else if(scmp(text, "dx"))
        return REG_DX;
    else if(scmp(text, "dh"))
        return REG_DH;
    else if(scmp(text, "dl"))
        return REG_DL;
    else if(scmp(text, "edi"))
        return REG_EDI;
    else if(scmp(text, "di"))
        return REG_DI;
    else if(scmp(text, "esi"))
        return REG_ESI;
    else if(scmp(text, "si"))
        return REG_SI;
    else if(scmp(text, "ebp"))
        return REG_EBP;
    else if(scmp(text, "bp"))
        return REG_BP;
    else if(scmp(text, "esp"))
        return REG_ESP;
    else if(scmp(text, "sp"))
        return REG_SP;
#ifndef _WIN64 //x86 only
    else if(scmp(text, "cs"))
        return REG_CS;
    else if(scmp(text, "ds"))
        return REG_DS;
    else if(scmp(text, "es"))
        return REG_ES;
    else if(scmp(text, "fs"))
        return REG_FS;
    else if(scmp(text, "gs"))
        return REG_GS;
    else if(scmp(text, "ss"))
        return REG_SS;
#endif //_WIN64
#ifdef _WIN64
    else if(scmp(text, "rax"))
        return REG_RAX;
    else if(scmp(text, "rbx"))
        return REG_RBX;
    else if(scmp(text, "rcx"))
        return REG_RCX;
    else if(scmp(text, "rsi"))
        return REG_RSI;
    else if(scmp(text, "sil"))
        return REG_SIL;
    else if(scmp(text, "rdi"))
        return REG_RDI;
    else if(scmp(text, "dil"))
        return REG_DIL;
    else if(scmp(text, "rbp"))
        return REG_RBP;
    else if(scmp(text, "bpl"))
        return REG_BPL;
    else if(scmp(text, "rsp"))
        return REG_RSP;
    else if(scmp(text, "spl"))
        return REG_SPL;
    else if(scmp(text, "rip"))
        return REG_RIP;
    else if(scmp(text, "r8"))
        return REG_R8;
    else if(scmp(text, "r8d"))
        return REG_R8D;
    else if(scmp(text, "r8w"))
        return REG_R8W;
    else if(scmp(text, "r8b"))
        return REG_R8B;
    else if(scmp(text, "r9"))
        return REG_R9;
    else if(scmp(text, "r9d"))
        return REG_R9D;
    else if(scmp(text, "r9w"))
        return REG_R9W;
    else if(scmp(text, "r9b"))
        return REG_R9B;
    else if(scmp(text, "r10"))
        return REG_R10;
    else if(scmp(text, "r10d"))
        return REG_R10D;
    else if(scmp(text, "r10w"))
        return REG_R10W;
    else if(scmp(text, "r10b"))
        return REG_R10B;
    else if(scmp(text, "r11"))
        return REG_R11;
    else if(scmp(text, "r11d"))
        return REG_R11D;
    else if(scmp(text, "r11w"))
        return REG_R11W;
    else if(scmp(text, "r11b"))
        return REG_R11B;
    else if(scmp(text, "r12"))
        return REG_R12;
    else if(scmp(text, "r12d"))
        return REG_R12D;
    else if(scmp(text, "r12w"))
        return REG_R12W;
    else if(scmp(text, "r12b"))
        return REG_R12B;
    else if(scmp(text, "r13"))
        return REG_R13;
    else if(scmp(text, "r13d"))
        return REG_R13D;
    else if(scmp(text, "r13w"))
        return REG_R13W;
    else if(scmp(text, "r13b"))
        return REG_R13B;
    else if(scmp(text, "r14"))
        return REG_R14;
    else if(scmp(text, "r14d"))
        return REG_R14D;
    else if(scmp(text, "r14w"))
        return REG_R14W;
    else if(scmp(text, "r14b"))
        return REG_R14B;
    else if(scmp(text, "r15"))
        return REG_R15;
    else if(scmp(text, "r15d"))
        return REG_R15D;
    else if(scmp(text, "r15w"))
        return REG_R15W;
    else if(scmp(text, "r15b"))
        return REG_R15B;
#endif //_WIN64
    return REG_NAN;
}

static SEG getsegment(const char* text)
{
    if(strstr(text, "cs"))
        return SEG_CS;
    else if(strstr(text, "ds"))
        return SEG_DS;
    else if(strstr(text, "es"))
        return SEG_ES;
    else if(strstr(text, "fs"))
        return SEG_FS;
    else if(strstr(text, "gs"))
        return SEG_GS;
    else if(strstr(text, "ss"))
        return SEG_SS;
    return SEG_DS;
}

static bool isinbase(char ch, const char* base)
{
    int len=strlen(base);
    for(int i=0; i<len; i++)
        if(tolower(ch)==tolower(base[i]))
            return true;
    return false;
}

static bool isbase(const char* text, const char* base)
{
    int len=strlen(text);
    for(int i=0; i<len; i++)
        if(!isinbase(text[i], base))
            return false;
    return true;
}

static bool valfromstring(const char* text, ULONG_PTR* value)
{
    if(*text=='x') //hexadecimal
    {
        if(!isbase(text+1, "0123456789ABCDEF"))
            return false;
#ifdef _WIN64
        sscanf(text+1, "%llx", value);
#else
        sscanf(text+1, "%x", value);
#endif // _WIN64
        return true;
    }
    else if(*text=='0' && text[1]=='x') //hexadecimal
    {
        if(!isbase(text+2, "0123456789ABCDEF"))
            return false;
#ifdef _WIN64
        sscanf(text+2, "%llx", value);
#else
        sscanf(text+2, "%x", value);
#endif // _WIN64
        return true;
    }
    else if(*text=='.') //decimal
    {
        int skip=1;
        bool negative=false;
        if(text[skip]=='-')
        {
            skip++;
            negative=true;
        }
        if(!isbase(text+skip, "0123456789"))
            return false;
#ifdef _WIN64
        sscanf(text+1, "%llu", value);
#else
        sscanf(text+1, "%u", value);
#endif // _WIN64
        if(negative)
            *value*=~0; //*-1
        return true;
    }
    else if(*text=='o') //octal
    {
        if(!isbase(text+1, "01234567"))
            return false;
#ifdef _WIN64
        sscanf(text+1, "%llo", value);
#else
        sscanf(text+1, "%o", value);
#endif // _WIN64
        return true;
    }
    else if(*text=='b') //binary
    {
        if(!isbase(text+1, "01"))
            return false;
    }
    if(!isbase(text, "0123456789ABCDEF")) //hexadecimal as default
        return false;
#ifdef _WIN64
    sscanf(text, "%llx", value);
#else
    sscanf(text, "%x", value);
#endif // _WIN64
    return true;
}

static SCALE getscale(const char* text)
{
    if(scmp(text, "1"))
        return SIZE_BYTE;
    else if(scmp(text, "2"))
        return SIZE_WORD;
    else if(scmp(text, "4"))
        return SIZE_DWORD;
#ifdef _WIN64
    else if(scmp(text, "8"))
        return SIZE_QWORD;
#endif //_WIN64
    return SIZE_BYTE; //default scale
}

static REGSIZE getregsize(REG reg)
{
    REGSIZE regsizelist[]=
    {
        SIZE_BYTE, //REG_NAN
        SIZE_DWORD, //REG_EAX
        SIZE_WORD, //REG_AX
        SIZE_BYTE, //REG_AH
        SIZE_BYTE, //REG_AL
        SIZE_DWORD, //REG_EBX
        SIZE_WORD, //REG_BX
        SIZE_BYTE, //REG_BH
        SIZE_BYTE, //REG_BL
        SIZE_DWORD, //REG_ECX
        SIZE_WORD, //REG_CX
        SIZE_BYTE, //REG_CH
        SIZE_BYTE, //REG_CL
        SIZE_DWORD, //REG_EDX
        SIZE_WORD, //REG_DX
        SIZE_BYTE, //REG_DH
        SIZE_BYTE, //REG_DL
        SIZE_DWORD, //REG_EDI
        SIZE_WORD, //REG_DI
        SIZE_DWORD, //REG_ESI
        SIZE_WORD, //REG_SI
        SIZE_DWORD, //REG_EBP
        SIZE_WORD, //REG_BP
        SIZE_DWORD, //REG_ESP
        SIZE_WORD, //REG_SP
#ifdef _WIN64
        SIZE_QWORD, //REG_RAX
        SIZE_QWORD, //REG_RBX
        SIZE_QWORD, //REG_RCX
        SIZE_QWORD, //REG_RDX
        SIZE_QWORD, //REG_RSI
        SIZE_BYTE, //REG_SIL
        SIZE_QWORD, //REG_RDI
        SIZE_BYTE, //REG_DIL
        SIZE_QWORD, //REG_RBP
        SIZE_BYTE, //REG_BPL
        SIZE_QWORD, //REG_RSP
        SIZE_BYTE, //REG_SPL
        SIZE_QWORD, //REG_RIP
        SIZE_QWORD, //REG_R8
        SIZE_DWORD, //REG_R8D
        SIZE_WORD, //REG_R8W
        SIZE_BYTE, //REG_R8B
        SIZE_QWORD, //REG_R9
        SIZE_DWORD, //REG_R9D
        SIZE_WORD, //REG_R9W
        SIZE_BYTE, //REG_R9B
        SIZE_QWORD, //REG_R10
        SIZE_DWORD, //REG_R10D
        SIZE_WORD, //REG_R10W
        SIZE_BYTE, //REG_R10B
        SIZE_QWORD, //REG_R11
        SIZE_DWORD, //REG_R11D
        SIZE_WORD, //REG_R11W
        SIZE_BYTE, //REG_R11B
        SIZE_QWORD, //REG_R12
        SIZE_DWORD, //REG_R12D
        SIZE_WORD, //REG_R12W
        SIZE_BYTE, //REG_R12B
        SIZE_QWORD, //REG_R13
        SIZE_DWORD, //REG_R13D
        SIZE_WORD, //REG_R13W
        SIZE_BYTE, //REG_R13B
        SIZE_QWORD, //REG_R14
        SIZE_DWORD, //REG_R14D
        SIZE_WORD, //REG_R14W
        SIZE_BYTE, //REG_R14B
        SIZE_QWORD, //REG_R15
        SIZE_DWORD, //REG_R15D
        SIZE_WORD, //REG_R15W
        SIZE_BYTE, //REG_R15B
#endif //_WIN64
    };
    return regsizelist[reg];
}

static bool parseoperand(XEDPARSE* raw, OPERAND* operand)
{
    if(!raw || !operand)
        return false;
    if(!strlen(operand->raw)) //nothing inside the operand
    {
        operand->type=TYPE_NONE;
        return true;
    }
    ULONG_PTR value=0;
    REG reg=getregister(operand->raw);
    if(strstr(operand->raw, "[")) //memory
    {
        operand->type=TYPE_MEMORY;
        int len=strlen(operand->raw);
        int bopen=0;
        int bclose=0;
        for(int i=0; i<len; i++)
        {
            if(operand->raw[i]=='[')
                bopen++;
            else if(operand->raw[i]==']')
                bclose++;
        }
        if(bopen!=1 || bclose!=1)
        {
            strcpy(raw->error, "invalid brackets!");
            return false;
        }
        //get stuff inside brackets
        char temp[XEDPARSE_MAXBUFSIZE/2]="";
        strcpy(temp, strstr(operand->raw, "[")+1);
        *strstr(temp, "]")=0;
        len=strlen(temp);

        char brackets[XEDPARSE_MAXBUFSIZE/2];
        for(int i=0,j=0; i<len; i++) //remove spaces
            if(temp[i]!=' ')
                j+=sprintf(brackets+j, "%c", temp[i]);
        len=strlen(brackets);
        if(!len)
        {
            strcpy(raw->error, "nothing inside brackets!");
            return false;
        }
        for(int i=0,j=0; i<len; i++)
        {
            if(brackets[i]=='+' || brackets[i]=='*')
                j+=sprintf(temp+j, " %c ", brackets[i]);
            else
                j+=sprintf(temp+j, "%c", brackets[i]);
        }

        char index[XEDPARSE_MAXBUFSIZE/2]="";
        char scale[XEDPARSE_MAXBUFSIZE/2]="";
        char other1[XEDPARSE_MAXBUFSIZE/2]="";
        char other2[XEDPARSE_MAXBUFSIZE/2]="";

        if(sscanf(temp, "%s + %s * %s + %s", other1, index, scale, other2)==4)
        {
        }
        else if(sscanf(temp, "%s + %s * %s", other1, index, scale)==3)
        {
            *other2=0;
        }
        else if(sscanf(temp, "%s * %s + %s", index, scale, other1)==3)
        {
            *other2=0;
        }
        else if(sscanf(temp, "%s * %s", index, scale)==2)
        {
            *other1=0;
            *other2=0;
        }
        else if(sscanf(temp, "%s + %s", other1, other2)==2)
        {
            *index=0;
            *scale=0;
        }
        else if(sscanf(temp, "%s", other1)==1)
        {
            *index=0;
            *scale=0;
        }
        else
        {
            strcpy(raw->error, "invalid stuff inside brackets!");
            return false;
        }
        operand->u.mem.scale=getscale(scale);
        operand->u.mem.index=getregister(index);

        REG reg1=getregister(other1);
        REG reg2=getregister(other2);
        if(reg1!=REG_NAN && reg2!=REG_NAN) //both registers
        {
            strcpy(raw->error, "invalid stuff inside brackets!");
            return false;
        }
        if(reg1!=REG_NAN)
        {
            strcpy(other1, other2);
            *other2=0;
            operand->u.mem.base=reg1;
        }
        else if(reg2!=REG_NAN)
            operand->u.mem.base=reg2;

        //get value of displacement
        ULONG_PTR value=0;
        operand->u.mem.displ.size=SIZE_DWORD; //NOTE: displacement is always DWORD
        if(!*other1) //no value
            operand->u.mem.displ.val=0;
        else if(valfromstring(other1, &value)) //normal value
            operand->u.mem.displ.val=value;
        else if(raw->cbUnknown && raw->cbUnknown(other1, &value)) //unknown value
            operand->u.mem.displ.val=value;
        else
        {
            strcpy(raw->error, "invalid displacement inside brackets!");
            return false;
        }
        operand->u.mem.displ.val&=0xFFFFFFFF; //NOTE: displacement is always DWORD

        //default segment + size
        operand->u.mem.size=SIZE_UNSET; //for later correction
        operand->u.mem.seg=SEG_DS;
        //get segment + size
        char other[XEDPARSE_MAXBUFSIZE/2]="";
        strcpy(other, operand->raw);
        _strlwr(other); //lowercase
        *strstr(other, "[")=0;
        if(scmp(other, "byte"))
            operand->u.mem.size=SIZE_BYTE;
        else if(scmp(other, "word"))
            operand->u.mem.size=SIZE_WORD;
        else if(scmp(other, "dword"))
            operand->u.mem.size=SIZE_DWORD;
#ifdef _WIN64
        else if(scmp(other, "qword"))
            operand->u.mem.size=SIZE_QWORD;
#endif // _WIN64
        operand->u.mem.seg=getsegment(other);
        return true;
    }
    else if(reg!=REG_NAN) //register
    {
        operand->u.reg.reg=reg;
        operand->u.reg.size=getregsize(reg);
        operand->type=TYPE_REGISTER;
        return true;
    }
    else if(valfromstring(operand->raw, &value)) //value
    {
        operand->u.val.val=value;
        if(value<=0xFF)
            operand->u.val.size=SIZE_BYTE;
        else if(value<=0xFFFF)
            operand->u.val.size=SIZE_WORD;
        else if(value<=0xFFFFFFFF)
            operand->u.val.size=SIZE_DWORD;
#ifdef _WIN64
        else if(value<=0xFFFFFFFFFFFFFFFF)
            operand->u.val.size=SIZE_QWORD;
#endif //_WIN64
        operand->type=TYPE_VALUE;
        return true;
    }
    else if(raw->cbUnknown && raw->cbUnknown(operand->raw, &value)) //unknown operand
    {
        operand->u.val.val=value;
        if(value<=0xFF)
            operand->u.val.size=SIZE_BYTE;
        else if(value<=0xFFFF)
            operand->u.val.size=SIZE_WORD;
        else if(value<=0xFFFFFFFF)
            operand->u.val.size=SIZE_DWORD;
#ifdef _WIN64
        else if(value<=0xFFFFFFFFFFFFFFFF)
            operand->u.val.size=SIZE_QWORD;
#endif //_WIN64
        operand->type=TYPE_VALUE;
        return true;
    }
    operand->type=TYPE_NONE;
    sprintf(raw->error, "invalid value \"%s\"!", operand->raw);
    return false;
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
    return 0;
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

static bool checkopsize(OPERAND* operand1, OPERAND* operand2)
{
    int opsize1=opsizetoint(getopsize(operand1));
    int opsize2=opsizetoint(getopsize(operand2));
    switch(operand1->type)
    {
    case TYPE_REGISTER:
        switch(operand2->type)
        {
        case TYPE_VALUE: //mov reg,value
            if(opsize1<opsize2) //example: mov al,12345678
                return false;
            if(opsize1!=opsize2) //example: 0x11 can be 0x00000011
                setopsize(operand2, opsize1);
            return true;
            break;
        case TYPE_REGISTER: //mov reg,reg
            if(opsize1>opsize2) //example: movxz eax,cl
                return true;
            if(opsize1!=opsize2) //example: mov al,eax
                return false;
            return true;
            break;
        case TYPE_MEMORY: //mov reg,[]
            if(operand2->u.mem.size==SIZE_UNSET) //unknown memory size
            {
                operand2->u.mem.size=inttoopsize(opsize1);
                opsize2=opsize1;
            }
            if(opsize1>opsize2) //example: movzx ebp, byte [esi]
                return true;
            if(opsize1!=opsize2)
                return false;
            return true;
            break;
        default:
            return true;
        }
        break;
    case TYPE_MEMORY:
        switch(operand2->type)
        {
        case TYPE_VALUE: //mov [],value
            if(operand1->u.mem.size==SIZE_UNSET) //unknown memory size
            {
#ifdef _WIN64
                opsize1=8;
#else
                opsize1=4;
#endif //_WIN64
                operand1->u.mem.size=inttoopsize(opsize1); //assume void* size
            }
            if(opsize1<opsize2) //example: mov [ax],12345678
                return false;
            if(opsize1!=opsize2) //example: 0x11 can be 0x00000011
                setopsize(operand2, opsize1);
            return true;
            break;
        case TYPE_REGISTER: //mov [],reg
            if(operand1->u.mem.size==SIZE_UNSET) //unknown memory size
            {
                operand1->u.mem.size=inttoopsize(opsize2);
                opsize1=opsize2;
            }
            if(opsize1!=opsize2) //example: mov al,eax
                return false;
            return true;
            break;
        case TYPE_MEMORY: //mov [],[]
            if(operand1->u.mem.size==SIZE_UNSET && operand2->u.mem.size==SIZE_UNSET) //size not set at all
            {
#ifdef _WIN64
                opsize1=8;
#else
                opsize1=4;
#endif //_WIN64
                operand1->u.mem.size=inttoopsize(opsize1);
            }
            if(operand1->u.mem.size==SIZE_UNSET) //unknown memory size in operand1
            {
                operand1->u.mem.size=inttoopsize(opsize2);
                opsize1=opsize2;
            }
            else if(operand2->u.mem.size==SIZE_UNSET) //unknown memory size in operand2
            {
                operand2->u.mem.size=inttoopsize(opsize1);
                opsize2=opsize1;
            }
            if(opsize1!=opsize2)
                return false;
            return true;
            break;
        default:
            return true;
        }
        break;
    default:
        return false;
    }
    return true;
}

static void formatoperand(OPERAND* operand)
{
    char temp[XEDPARSE_MAXBUFSIZE/2]="";
    //remove prepended spaces
    int len=strlen(operand->raw);
    int skip=0;
    while(operand->raw[skip]==' ' && skip<len)
        skip++;
    strcpy(temp, operand->raw+skip);
    len=strlen(temp);
    if(!len) //nothing left
        return;
    while(temp[len-1]==' ' && len-1)
        len--;
    temp[len]=0;
    strcpy(operand->raw, temp);
}

bool parse(XEDPARSE* raw, INSTRUCTION* parsed)
{
    if(!raw || !parsed)
        return false;
    char* instr=raw->instr;
    while(*instr==' ' && *instr) //skip initial spaces
        instr++;
    int len=strlen(instr);
    if(!len)
    {
        strcpy(raw->error, "empty instruction");
        return false;
    }
    //get prefix
    int skipadd=0;
    parsed->prefix=getprefix(instr, &skipadd);
    instr+=skipadd;
    while(*instr==' ' && *instr) //skip spaces
        instr++;
    len=strlen(instr);
    if(!len)
    {
        strcpy(raw->error, "only a prefix");
        return false;
    }
    if(!sscanf(instr, "%s", parsed->mnemonic))
    {
        strcpy(raw->error, "no mnemonic");
        return false;
    }
    instr=strstr(instr, " ");
    if(!instr)
        return true; //we have a mnemonic already
    instr++;
    len=strlen(instr);
    if(!len)
        return true;
    int commacount=0;
    for(int i=0; i<len; i++)
        if(instr[i]==',')
            commacount++;
    if(commacount>2) //invalid instruction
    {
        strcpy(raw->error, "too many arguments found");
        return false;
    }
    int skip=0;
    if(commacount) //two operands
    {
        char* operand2=strstr(instr, ","); //find comma
        *operand2=0;
        operand2++;
        len=strlen(operand2);
        skip=0;
        while(operand2[skip]==' ' && skip<len) //skip spaces
            skip++;
        strcpy(parsed->operand2.raw, operand2+skip);
    }
    strcpy(parsed->operand1.raw, instr);
    formatoperand(&parsed->operand1);
    formatoperand(&parsed->operand2);
    if(!parseoperand(raw, &parsed->operand1))
        return false;
    if(!parseoperand(raw, &parsed->operand2))
        return false;
    if(parsed->operand2.type==TYPE_NONE) //only one operand
        return true;
    if(parsed->operand1.type==TYPE_VALUE) //first operand is a value
    {
        strcpy(raw->error, "invalid operand detected!");
        return false;
    }
    if(!checkopsize(&parsed->operand1, &parsed->operand2))
    {
        strcpy(raw->error, "operand size mismatch!");
        return false;
    }
    return true;
}
