#include "parser.h"
#include "x86reference.h"
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
	else if (scmp(text, "16"))
		return SIZE_DQWORD;
	else if (scmp(text, "32"))
		return SIZE_YWORD;
	else if (scmp(text, "64"))
		return SIZE_ZWORD;
    return SIZE_BYTE; //default scale
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
        char other3[XEDPARSE_MAXBUFSIZE/2]="";

        bool skipcheck=false;

        if(sscanf(temp, "%s + %s * %s + %s", other1, index, scale, other2)==4)
        {
        }
        else if(sscanf(temp, "%s + %s + %s", other1, other2, other3)==3)
        {
            skipcheck=true;
            if(getregister(other1)!=REG_NAN && getregister(other2)!=REG_NAN) //reg + reg + val
            {
                operand->u.mem.base=getregister(other1);
                strcpy(index, other2);
                strcpy(other1, other3);
            }
            else if(getregister(other1)!=REG_NAN && getregister(other3)!=REG_NAN) //reg + val + reg
            {
                operand->u.mem.base=getregister(other1);
                strcpy(index, other3);
                strcpy(other1, other2);
            }
            else if(getregister(other2)!=REG_NAN && getregister(other3)!=REG_NAN) //val + reg + reg
            {
                operand->u.mem.base=getregister(other2);
                strcpy(index, other3);
            }
            else
            {
                strcpy(raw->error, "invalid stuff inside brackets!");
                return false;
            }
            *other2=0;
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

        if(!skipcheck)
        {
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
        }

        //get value of displacement
        ULONG_PTR value=0;
        operand->u.mem.displ.size=SIZE_DWORD;
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
        operand->u.mem.displ.val&=0xFFFFFFFF; //NOTE: displacement is always <= DWORD
        if(operand->u.mem.displ.val<=0xFF) //BYTE is also possible
            operand->u.mem.displ.size=SIZE_BYTE;

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

//check the operand sizes (TODO: this function needs to use a list of valid operand sizes)
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
            {
#ifdef _WIN64
                if(opsize1==8) //fix mov qword ptr ds:[rax],1
                    opsize1=4;
#endif //_WIN64                
                setopsize(operand2, opsize1);
            }
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
    if(commacount>3) //invalid instruction
    {
        strcpy(raw->error, "too many arguments found");
        return false;
    }
    int skip=0;
    if(commacount==1) //two operands
    {
        //operand2
        char* operand2=strstr(instr, ","); //find comma
        *operand2=0;
        operand2++;
        //operand2
        len=strlen(operand2);
        skip=0;
        while(operand2[skip]==' ' && skip<len) //skip spaces
            skip++;
        strcpy(parsed->operand2.raw, operand2+skip);
    }
    else if(commacount==2) //three operands
    {
        //operand2
        char* operand2=strstr(instr, ","); //find comma
        *operand2=0;
        operand2++;
        //operand3
        char* operand3=strstr(operand2, ","); //find comma
        *operand3=0;
        operand3++;
        //operand2
        len=strlen(operand2);
        skip=0;
        while(operand2[skip]==' ' && skip<len) //skip spaces
            skip++;
        strcpy(parsed->operand2.raw, operand2+skip);
        //operand3
        len=strlen(operand3);
        skip=0;
        while(operand3[skip]==' ' && skip<len) //skip spaces
            skip++;
        strcpy(parsed->operand3.raw, operand3+skip);
    }
    else if(commacount==3) //four operands (certain vm instructions)
    {
        //operand2
        char* operand2=strstr(instr, ","); //find comma
        *operand2=0;
        operand2++;
        //operand3
        char* operand3=strstr(operand2, ","); //find comma
        *operand3=0;
        operand3++;
        //operand4
        char* operand4=strstr(operand3, ","); //find comma
        *operand4=0;
        operand4++;
        //operand2
        len=strlen(operand2);
        skip=0;
        while(operand2[skip]==' ' && skip<len) //skip spaces
            skip++;
        strcpy(parsed->operand2.raw, operand2+skip);
        //operand3
        len=strlen(operand3);
        skip=0;
        while(operand3[skip]==' ' && skip<len) //skip spaces
            skip++;
        strcpy(parsed->operand3.raw, operand3+skip);
        //operand4
        len=strlen(operand4);
        skip=0;
        while(operand4[skip]==' ' && skip<len) //skip spaces
            skip++;
        strcpy(parsed->operand4.raw, operand4+skip);
    }
    strcpy(parsed->operand1.raw, instr);
    formatoperand(&parsed->operand1);
    formatoperand(&parsed->operand2);
    formatoperand(&parsed->operand3);
    formatoperand(&parsed->operand4);
    if(!parseoperand(raw, &parsed->operand1))
        return false;
    if(!parseoperand(raw, &parsed->operand2))
        return false;
    if(!parseoperand(raw, &parsed->operand3))
        return false;
    if(!parseoperand(raw, &parsed->operand4))
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
