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
	int			Bits;
};

static OpsizeEntry OpsizeEntryTable[] =
{
	{ "byte",		SIZE_BYTE,		8	},
	{ "word",		SIZE_WORD,		16	},
	{ "dword",		SIZE_DWORD,		32	},
	{ "qword",		SIZE_QWORD,		64	},
	{ "xmmword",	SIZE_XMMWORD,	128 },
	{ "ymmword",	SIZE_YMMWORD,	256 },
	{ "zmmword",	SIZE_ZMMWORD,	512 },
	{ "UNSET",		SIZE_UNSET,		8	},
};

unsigned int opsizetobits(OPSIZE opsize);
int opsizetoint(OPSIZE opsize);
OPSIZE bitstoopsize(int bits);
OPSIZE inttoopsize(int opsize);

OPSIZE StringToOpsize(const char *Value);
const char *OpsizeToString(OPSIZE Size);