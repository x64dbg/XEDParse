#include "XEDParse.h"
#include "xed2\include\xed-interface.h"
#include "Parser.h"
#include "Translator.h"
#include "ParseTest.h"

void XEDParseSetMode(bool X64, xed_state_t *State)
{
	if (X64)
	{
		State->mmode			= XED_MACHINE_MODE_LONG_64;
		State->stack_addr_width = XED_ADDRESS_WIDTH_32b;
	}
	else
	{
		State->mmode			= XED_MACHINE_MODE_LEGACY_32;
		State->stack_addr_width = XED_ADDRESS_WIDTH_32b;
	}
}

XEDPARSE_EXPORT XEDPARSE_STATUS XEDPARSE_CALL XEDParseAssemble(XEDPARSE* XEDParse)
{
	Inst instruction;
	memset(&instruction, 0, sizeof(Inst));

	if (!ParseInstString(XEDParse, &instruction))
	{
		printf("%s\n", XEDParse->error);
		return XEDPARSE_ERROR;
	}

	xed_state_t state;
	XEDParseSetMode(XEDParse->x64, &state);

	if (!Translate(XEDParse, state, &instruction))
	{
		printf("%s\n", XEDParse->error);
		return XEDPARSE_ERROR;
	}

	char instString[XEDPARSE_MAXBUFSIZE];
	memset(instString, 0, sizeof(instString));

	InstructionToString(instString, &instruction);
	printf("\nParsed instruction:\n%s\n", instString);

	return XEDPARSE_OK;
}

extern "C" __declspec(dllexport) BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:
		// Initialize all of the tables only once
		// This is mainly independent to everything else
		LookupTableInit();
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
