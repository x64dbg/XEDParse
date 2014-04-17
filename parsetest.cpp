#include "parsetest.h"
#include "regtable.h"
#include <stdio.h>

static const char* sizetostring(OPSIZE size)
{
    const char* sizelist[]=
    {
        "byte",
        "word",
        "dword",
        "qword",
		"dqword",
		"yword",
		"zword",
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
        "8",
		"16",
		"32",
		"64",
    };
    return sizelist[size];
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
        sprintf(str, "%s", regtostring(operand->u.reg.reg));
    }
    break;

    case TYPE_VALUE:
    {
#ifdef _WIN64
        sprintf(str, "%llX/%s",
#else
        sprintf(str, "%X/%s",
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
        sprintf(str, "%s ptr %s:[%s%s%llX/%s]",
#else
        sprintf(str, "%s ptr %s:[%s%s%X/%s]",
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

void parsedisasm(INSTRUCTION* parsed, char* string)
{
    char operand1[256]="";
    operandtostring(&parsed->operand1, operand1);
    _strlwr(operand1);
    char operand2[256]="";
    operandtostring(&parsed->operand2, operand2);
    _strlwr(operand2);
    char operand3[256]="";
    operandtostring(&parsed->operand3, operand3);
    _strlwr(operand3);
    int j=sprintf(string, "%s%s", prefixtostring(parsed->prefix), parsed->mnemonic);
    if(*operand1)
        j+=sprintf(string+j, " %s", operand1);
    if(*operand2)
        j+=sprintf(string+j, ",%s", operand2);
    if(*operand3)
        sprintf(string+j, ",%s", operand3);
}
