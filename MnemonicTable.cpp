#include "Translator.h"

const char *MnemonicToXed(const char *Mnemonic)
{
	for (int i = 0; i < ARRAYSIZE(XedMnemonicTable); i++)
	{
		if (!_stricmp(Mnemonic, XedMnemonicTable[i].Name))
			return XedMnemonicTable[i].XedName;
	}

	return Mnemonic;
}