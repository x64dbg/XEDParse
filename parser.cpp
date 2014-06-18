#include "Parser.h"
#include "Translator.h"

static bool isinbase(char ch, const char* base)
{
    int len=strlen(base);
    for(int i=0; i<len; i++)
        if(tolower(ch)==tolower(base[i]))
            return true;
    return false;
}

static bool isbase(const char* text, const char* base)
{
    int len=strlen(text);
    for(int i=0; i<len; i++)
        if(!isinbase(text[i], base))
            return false;
    return true;
}

bool valfromstring(const char* text, ULONGLONG* value)
{
    if(*text=='x') //hexadecimal
    {
        if(!isbase(text+1, "0123456789ABCDEF"))
            return false;
        sscanf(text+1, "%llx", value);
        return true;
    }
    else if(*text=='0' && text[1]=='x') //hexadecimal
    {
        if(!isbase(text+2, "0123456789ABCDEF"))
            return false;
        sscanf(text+2, "%llx", value);
        return true;
    }
    else if(*text=='.') //decimal
    {
        int skip=1;
        bool negative=false;
        if(text[skip]=='-')
        {
            skip++;
            negative=true;
        }
        if(!isbase(text+skip, "0123456789"))
            return false;
        sscanf(text+1, "%llu", value);
        if(negative)
            *value*=~0; //*-1
        return true;
    }
    else if(*text=='o') //octal
    {
        if(!isbase(text+1, "01234567"))
            return false;
        sscanf(text+1, "%llo", value);
        return true;
    }
    else if(*text=='b') //binary
    {
        if(!isbase(text+1, "01"))
            return false;
    }
    if(!isbase(text, "0123456789ABCDEF")) //hexadecimal as default
        return false;
    sscanf(text, "%llx", value);
    return true;
}

char *GrabInstToken(char *Dest, char *Src, bool Operand)
{
	char *bufEnd = nullptr;

	if (Operand)
	{
		bufEnd	= strchr(Src, ',');

		// Skip spaces
		while (*Src == ' ' || *Src == '\t')
			Src++;
	}
	else
	{
		bufEnd = strchr(Src, ' ');
	}

	if (bufEnd)
		*bufEnd = '\0';

	strcpy(Dest, Src);

	return ((bufEnd) ? (bufEnd + 1) : nullptr);
}

int InstructionToTokens(const char *Value, char Tokens[8][64])
{
	// [PREFIX] INSTRUCTION [SEG]:[MEM/REG/IMM], [SEG]:[MEM/REG/IMM], [REG], [REG]

	char buf[XEDPARSE_MAXBUFSIZE];
	strcpy_s(buf, Value);

	// Check the length first
	if (strlen(buf) <= 0)
		return 0;

	char *bufPtr	= buf;
	int tokenIndex	= 0;

	// Grab the prefix or mnemonic
	bufPtr = GrabInstToken(Tokens[tokenIndex], bufPtr, false);

	// See if it was a prefix
	if (StringToPrefix(Tokens[tokenIndex++]) != PREFIX_NONE)
	{
		// There was a prefix but nothing after it
		if (!bufPtr)
			return 0;

		// A prefix exists, move on to the next token
		bufPtr = GrabInstToken(Tokens[tokenIndex++], bufPtr, false);
	}

	// Happens with a single mnemonic/nothing after prefix
	if (!bufPtr)
		return tokenIndex;

	// Go through each operand (use a max of 6 tokens)
	for (int i = 0; i < 6; i++)
	{
		bufPtr = GrabInstToken(Tokens[tokenIndex++], bufPtr, true);

		if (!bufPtr)
			return tokenIndex;
	}

	return tokenIndex;
}

bool ParseInstString(XEDPARSE *Parse, Inst *Instruction)
{
	char tokens[8][64];
	memset(tokens, 0, sizeof(tokens));

	int tokenIndex = 0;
	int tokenCount = InstructionToTokens(Parse->instr, tokens);

	if (tokenCount <= 0)
	{
		strcpy(Parse->error, "Malformed or invalid instruction");
		return false;
	}

	// Prefix
	Instruction->Prefix = StringToPrefix(tokens[tokenIndex]);

	if (Instruction->Prefix != PREFIX_NONE)
		tokenIndex++;

	// Mnemonic
	strcpy(Instruction->Mnemonic, tokens[tokenIndex++]);

	// Operands
	for (int i = tokenIndex; i < tokenCount; i++)
	{
		if (!AnalyzeOperand(Parse, tokens[i], &Instruction->Operands[Instruction->OperandCount++]))
			return false;
	}

	if (Instruction->OperandCount > 4)
	{
		strcpy(Parse->error, "Instruction has more than 4 operands");
		return false;
	}

	// Verify and translate the mnemonic this time
	Instruction->Class = str2xed_iclass_enum_t(TranslateInstMnemonic(Parse, Instruction));

	if (Instruction->Class == XED_ICLASS_INVALID)
	{
		sprintf(Parse->error, "Unknown instruction mnemonic '%s'", Instruction->Mnemonic);
		return false;
	}

	return true;
}