#pragma once

enum OPSIZE
{
	SIZE_BYTE,		// Byte
	SIZE_WORD,		// Word
	SIZE_DWORD,		// Double Word
	SIZE_QWORD,		// Quad Word
	SIZE_XMMWORD,	// XMM Word
	SIZE_YMMWORD,	// YMM Word
	SIZE_ZMMWORD,	// ZMM Word
	SIZE_UNSET,		// No size set
};

typedef OPSIZE MEMSIZE;
typedef OPSIZE SCALE;
typedef OPSIZE REGSIZE;

struct OpsizeEntry
{
	const char *Name;
	OPSIZE		Size;
	int			Bytes;
	int			Bits;
};

static OpsizeEntry OpsizeEntryTable[] =
{
	{ "byte",		SIZE_BYTE,		1,	8	},
	{ "word",		SIZE_WORD,		2,	16	},
	{ "dword",		SIZE_DWORD,		4,	32	},
	{ "qword",		SIZE_QWORD,		8,	64	},
	{ "xmmword",	SIZE_XMMWORD,	16, 128 },
	{ "ymmword",	SIZE_YMMWORD,	32, 256 },
	{ "zmmword",	SIZE_ZMMWORD,	64, 512 },
	{ "UNSET",		SIZE_UNSET,		1,	8	},
};

unsigned int opsizetobits(OPSIZE opsize);
int opsizetoint(OPSIZE opsize);
OPSIZE bitstoopsize(int bits);
OPSIZE inttoopsize(int opsize);

OPSIZE StringToOpsize(const char *Value);
const char *OpsizeToString(OPSIZE Size);