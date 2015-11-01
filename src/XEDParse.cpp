#include "XEDParse.h"
#include "..\xed2\include\xed-interface.h"
#include "Parser.h"
#include "Translator.h"
#include "ParseTest.h"

void XEDParseSetMode(bool X64, xed_state_t* State)
{
    if(X64)
    {
        State->mmode            = XED_MACHINE_MODE_LONG_64;
        State->stack_addr_width = XED_ADDRESS_WIDTH_32b;
    }
    else
    {
        State->mmode            = XED_MACHINE_MODE_LEGACY_32;
        State->stack_addr_width = XED_ADDRESS_WIDTH_32b;
    }
}

XEDPARSE_EXPORT XEDPARSE_STATUS XEDPARSE_CALL XEDParseAssemble(XEDPARSE* XEDParse)
{
    // Initialize all of the tables only once.
    // This is mainly independent to everything else.
    LookupTableInit();

    Inst instruction;
    memset(&instruction, 0, sizeof(Inst));

    if(!ParseInstString(XEDParse, &instruction))
        return XEDPARSE_ERROR;

    xed_state_t state;
    XEDParseSetMode(XEDParse->x64, &state);

    if(!Translate(XEDParse, state, &instruction))
        return XEDPARSE_ERROR;

    char instString[XEDPARSE_MAXBUFSIZE];
    memset(instString, 0, sizeof(instString));

    InstructionToString(instString, &instruction);
    strcpy(XEDParse->error, instString);
    _strlwr(XEDParse->error);

    return XEDPARSE_OK;
}

#ifndef XEDPARSE_STATIC
extern "C" __declspec(dllexport) BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}
#endif // XEDPARSE_STATIC