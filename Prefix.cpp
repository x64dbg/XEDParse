#include "Translator.h"

const char* PrefixToString(PREFIX prefix)
{
    switch(prefix)
    {
    case PREFIX_LOCK:
        return "lock";
    case PREFIX_REP:
        return "rep";
    case PREFIX_REPNEZ:
        return "repne";
    }

    return "";
}

PREFIX PrefixFromString(const char* Value)
{
    if(!_stricmp(Value, "lock"))
        return PREFIX_LOCK;

    if(!_stricmp(Value, "rep"))
        return PREFIX_REP;

    if(!_stricmp(Value, "repe") || !_stricmp(Value, "repz"))
        return PREFIX_REP;

    if(!_stricmp(Value, "repne") || !_stricmp(Value, "repnz"))
        return PREFIX_REPNEZ;

    return PREFIX_NONE;
}
