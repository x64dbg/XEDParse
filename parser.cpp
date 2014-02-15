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
        return PREFIX_REPEZ;
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

static bool parseoperand(XEDPARSE* raw, OPERAND* operand)
{
    if(!raw || !operand)
        return false;
    if(!strlen(operand->raw)) //nothing inside the operand
    {
        operand->type=TYPE_NONE;
        return true;
    }
    ULONG_PTR value;
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
        char brackets[XEDPARSE_MAXBUFSIZE/2]="";
        strcpy(brackets, strstr(operand->raw, "[")+1);
        *strstr(brackets, "]")=0;
        puts(brackets);
        //TODO: handle stuff inside brackets
        //default segment + size
#ifdef _WIN64
        operand->u.mem.size=SIZE_QWORD;
#else
        operand->u.mem.size=SIZE_DWORD;
#endif //_WIN64
        operand->u.mem.seg=SEG_DS;
        //get segment + size
        char other[XEDPARSE_MAXBUFSIZE/2]="";
        strcpy(other, operand->raw);
        _strlwr(other); //lowercase
        *strstr(other, "[")=0;
        puts(other);
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

        printf("%d ptr %d:[%d+%d*%d+0x%p]\n", operand->u.mem.size, operand->u.mem.seg, operand->u.mem.base, operand->u.mem.index, operand->u.mem.scale, operand->u.mem.displ.val);

        return true;
    }
    else if(reg!=REG_NAN) //register
    {
        operand->u.reg=reg;
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
    else if(raw->cbUnknown) //unknown operand
    {
        if(raw->cbUnknown(operand->raw, &value))
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
    }
    operand->type=TYPE_NONE;
    return false;
}

bool parse(XEDPARSE* raw, INSTRUCTION* parsed)
{
    if(!raw || !parsed)
        return false;
    int len=strlen(raw->instr);
    if(!len)
    {
        strcpy(raw->error, "empty instruction");
        return false;
    }
    //skip initial spaces
    int skip=0;
    while(raw->instr[skip]==' ' && skip<len)
        skip++;
    //get prefix
    parsed->prefix=getprefix(raw->instr+skip, &skip);
    len=strlen(raw->instr+skip);
    if(!len)
    {
        strcpy(raw->error, "only a prefix");
        return false;
    }
    while(raw->instr[skip]==' ' && skip<len) //skip spaces
        skip++;
    if(!sscanf(raw->instr+skip, "%s", parsed->mnemonic))
    {
        strcpy(raw->error, "no mnemonic");
        return false;
    }
    while(raw->instr[skip]!=' ' && skip<len)
        skip++;
    char instr[XEDPARSE_MAXBUFSIZE]="";
    strcpy(instr, raw->instr+skip+1);
    len=strlen(instr);
    if(!len)
        return true; //we have a mnemonic already
    int commacount=0;
    for(int i=0; i<len; i++)
        if(instr[i]==',')
            commacount++;
    if(commacount>2) //invalid instruction
    {
        strcpy(raw->error, "too many arguments found");
        return false;
    }
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
    if(!parseoperand(raw, &parsed->operand1))
        return false;
    if(!parseoperand(raw, &parsed->operand2))
        return false;
    return true;
}
