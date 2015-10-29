#include "Parser.h"
#include "Translator.h"
#include <stdio.h>

static bool isinbase(char ch, const char* base)
{
    for(; *base != '\0'; base++)
    {
        if(tolower(ch) == tolower(*base))
            return true;
    }

    return false;
}

static bool isbase(const char* text, const char* base)
{
    for(; *text != '\0'; text++)
    {
        if(!isinbase(*text, base))
            return false;
    }

    return true;
}

static bool parsebinary(const char* text, ULONGLONG* value)
{
    if(!isbase(text, "01"))
        return false;
    ULONGLONG result = 0;
    int len = strlen(text);
    for(int i = 0; i < len; i++)
    {
        if(i)
            result <<= 1;
        if(text[i] == '1')
            result++;
    }
    *value = result;
    return true;
}

bool valfromstring(const char* text, ULONGLONG* value)
{
    // Check if the string has any actual characters
    if(text[0] == '\0')
        return false;

    // Set the negative flag if needed
    bool negative = false;

    if(text[0] == '-')
    {
        negative = true;
        text++;
    }

    // Hexadecimal with '0x' prefix
    if(text[0] == '0' && tolower(text[1]) == 'x')
        text++;

    switch(tolower(*text++))
    {
    default:
        // Default to hexadecimal
        text--;

    case 'x':
        // Hexadecimal
        if(!isbase(text, "0123456789ABCDEF"))
            return false;

        sscanf(text, "%llx", value);
        break;

    case '.':
        if(tolower(text[0]) == 'b')  // Binary
        {
            if(!parsebinary(text + 1, value))
                return false;
        }
        else // Decimal
        {
            if(!isbase(text, "0123456789"))
                return false;

            sscanf(text, "%llu", value);
        }
        break;

    case 'o':
        // Octal
        if(!isbase(text, "01234567"))
            return false;

        sscanf(text, "%llo", value);
        break;
    }

    if(negative)
        *value *= -1;

    return true;
}

char* stristr(const char* haystack, const char* needle)
{
    // Case insensitive strstr
    // http://stackoverflow.com/questions/27303062/strstr-function-like-that-ignores-upper-or-lower-case
    do
    {
        const char* h = haystack;
        const char* n = needle;
        while(tolower((unsigned char)*h) == tolower((unsigned char)*n) && *n)
        {
            h++;
            n++;
        }

        if(*n == 0)
            return (char*)haystack;

    }
    while(*haystack++);

    // Not found
    return nullptr;
}

bool StrDel(char* Source, char* Needle, char StopAt)
{
    // Find the location in the string first
    char* loc = stristr(Source, Needle);

    if(!loc)
        return false;

    // "Delete" the word by shifting it over
    int needleLen = strlen(Needle);

    memcpy(loc, loc + needleLen, strlen(loc) - needleLen + 1);

    return true;
}

char* GrabInstToken(char* Dest, char* Src, bool Operand)
{
    char* bufEnd = nullptr;

    if(Operand)
    {
        bufEnd = strchr(Src, ',');

        // Skip spaces
        while(*Src == ' ' || *Src == '\t')
            Src++;
    }
    else
    {
        bufEnd = strchr(Src, ' ');
    }

    if(bufEnd)
        *bufEnd = '\0';

    strcpy(Dest, Src);

    return ((bufEnd) ? (bufEnd + 1) : nullptr);
}

int InstructionToTokens(const char* Value, char Tokens[8][64])
{
    // [PREFIX] INSTRUCTION [SEG]:[MEM/REG/IMM], [SEG]:[MEM/REG/IMM], [REG], [REG]

    // Copy a buffer to edit later
    // Skip spaces
    while(*Value == ' ' || *Value == '\t')
        Value++;

    char buf[XEDPARSE_MAXBUFSIZE];
    strcpy(buf, Value);

    // Check the length
    if(strlen(buf) <= 0)
        return 0;

    char* bufPtr    = buf;
    int tokenIndex  = 0;

    // Grab the prefix or mnemonic
    bufPtr = GrabInstToken(Tokens[tokenIndex], bufPtr, false);

    // See if it was a prefix
    if(PrefixFromString(Tokens[tokenIndex++]) != PREFIX_NONE)
    {
        // There was a prefix but nothing after it
        if(!bufPtr)
            return 0;

        // A prefix exists, move on to the next token
        bufPtr = GrabInstToken(Tokens[tokenIndex++], bufPtr, false);
    }

    // Happens with a single mnemonic/nothing after prefix
    if(!bufPtr)
        return tokenIndex;

    // Obliterate spaces
    {
        char* base  = bufPtr;
        char* ptr   = bufPtr;

        for(; *ptr; ptr++)
        {
            if(*ptr != ' ' && *ptr != '\t')
                *base++ = *ptr;
        }

        *base = '\0';

        // Is there anything left in the string?
        if(base == bufPtr)
            return tokenIndex;
    }

    // Go through each operand (use a max of 6 tokens)
    for(int i = 0; i < 6; i++)
    {
        bufPtr = GrabInstToken(Tokens[tokenIndex++], bufPtr, true);

        if(!bufPtr)
            return tokenIndex;
    }

    // Too many tokens
    return 0;
}

bool ParseInstString(XEDPARSE* Parse, Inst* Instruction)
{
    // Copy a buffer to edit
    char buf[XEDPARSE_MAXBUFSIZE];
    strcpy(buf, Parse->instr);

    // Find near/far modifiers
    Instruction->Near  = StrDel(buf, "near", ' ');
    Instruction->Far   = StrDel(buf, "far", ' ');
    Instruction->Short = StrDel(buf, "short", ' ');

    // Limit the number of possible modifiers to 1 at a time
    if(((int)Instruction->Near + (int)Instruction->Far + (int)Instruction->Short) > 1)
    {
        strcpy(Parse->error, "Instruction cannot have multiple modifiers (short, near, far)");
        return false;
    }

    // Parse into tokens
    char tokens[8][64];
    memset(tokens, 0, sizeof(tokens));

    int tokenIndex = 0;
    int tokenCount = InstructionToTokens(buf, tokens);

    if(tokenCount <= 0)
    {
        strcpy(Parse->error, "Malformed or invalid instruction");
        return false;
    }

    // Prefix
    Instruction->Prefix = PrefixFromString(tokens[tokenIndex]);

    if(Instruction->Prefix != PREFIX_NONE)
        tokenIndex++;

    // Mnemonic
    strcpy(Instruction->Mnemonic, tokens[tokenIndex++]);

    // Operands
    for(int i = tokenIndex; i < tokenCount; i++)
    {
        if(!AnalyzeOperand(Parse, tokens[i], &Instruction->Operands[Instruction->OperandCount++]))
            return false;
    }

    if(Instruction->OperandCount > 4)
    {
        strcpy(Parse->error, "Instruction has more than 4 operands");
        return false;
    }

    // Verify and translate the mnemonic this time
    Instruction->Class = str2xed_iclass_enum_t(MnemonicInstToXed(Parse, Instruction));

    if(Instruction->Class == XED_ICLASS_INVALID)
    {
        sprintf(Parse->error, "Unknown instruction mnemonic '%s'", Instruction->Mnemonic);
        return false;
    }

    return true;
}