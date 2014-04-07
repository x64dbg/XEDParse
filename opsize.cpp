#include "translate.h"

unsigned int opsizetobits(OPSIZE opsize)
{
	switch (opsize)
	{
	case SIZE_BYTE:		return 8;
	case SIZE_WORD:		return 16;
	case SIZE_DWORD:	return 32;
	case SIZE_QWORD:	return 64;
	case SIZE_DQWORD:	return 128;
	case SIZE_YWORD:	return 256;
	case SIZE_ZWORD:	return 512;
	default:
		break;
	}

#ifdef _WIN64
	return 64;
#else
	return 32;
#endif //_WIN64
} 

int opsizetoint(OPSIZE opsize)
{
	switch (opsize)
	{
	case SIZE_BYTE:		return 1;
	case SIZE_WORD:		return 2;
	case SIZE_DWORD:	return 4;
	case SIZE_QWORD:	return 8;
	case SIZE_DQWORD:	return 16;
	case SIZE_YWORD:	return 32;
	case SIZE_ZWORD:	return 64;
	}

	return 1;
}

OPSIZE inttoopsize(int opsize)
{
	switch (opsize)
	{
	case 1:		return SIZE_BYTE;
	case 2:		return SIZE_WORD;
	case 4:		return SIZE_DWORD;
	case 8:		return SIZE_QWORD;
	case 16:	return SIZE_DQWORD;
	case 32:	return SIZE_YWORD;
	case 64:	return SIZE_ZWORD;
	}

	return SIZE_BYTE;
}