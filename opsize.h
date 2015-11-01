#pragma once

#define EOSZ_64 3
#define EOSZ_32 2
#define EOSZ_64_32(Is64) ((Is64) ? EOSZ_64 : EOSZ_32)

enum OPSIZE
{
    SIZE_UNSET,     // No size set
    SIZE_BYTE,      // Byte
    SIZE_WORD,      // Word
    SIZE_DWORD,     // Double Word
    SIZE_FWORD,     // Far word 48bits
    SIZE_QWORD,     // Quad Word
    SIZE_TBYTE,     // FPU 80bits
    SIZE_DQWORD,    // Double quad word
    SIZE_XMMWORD,   // XMM Word
    SIZE_YMMWORD,   // YMM Word
    SIZE_ZMMWORD,   // ZMM Word
    SIZE_32_64,     // 32 or 64 bits
};

typedef OPSIZE MEMSIZE;
typedef OPSIZE REGSIZE;

struct OpsizeEntry
{
    const char* Name;
    OPSIZE      Size;
    int         Bytes;
    int         Bits;
};

static OpsizeEntry OpsizeEntryTable[] =
{
    // See: enum OPSIZE
    { "UNSET",      SIZE_UNSET,     0,  0   },
    { "byte",       SIZE_BYTE,      1,  8   },
    { "word",       SIZE_WORD,      2,  16  },
    { "dword",      SIZE_DWORD,     4,  32  },
    { "fword",      SIZE_FWORD,     6,  48  },
    { "qword",      SIZE_QWORD,     8,  64  },
    { "tbyte",      SIZE_TBYTE,     10, 80  },
    { "dqword",     SIZE_DQWORD,    16, 128 },
    { "xmmword",    SIZE_XMMWORD,   16, 128 },
    { "ymmword",    SIZE_YMMWORD,   32, 256 },
    { "zmmword",    SIZE_ZMMWORD,   64, 512 },
    { "32_64",      SIZE_32_64,     0,  0   },
};

int OpsizeToBits(OPSIZE opsize);
int OpsizeToInt(OPSIZE opsize);
OPSIZE OpsizeFromBits(int bits);
OPSIZE OpsizeFromInt(int opsize);
int OpsizeToEosz(OPSIZE Value);

OPSIZE OpsizeFromString(const char* Value);
const char* OpsizeToString(OPSIZE Size);

OPSIZE OpsizeFromValue(LONGLONG Value);