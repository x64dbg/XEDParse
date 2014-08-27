#pragma once

enum PREFIX
{
    PREFIX_NONE,
    PREFIX_LOCK,    // LOCK F0
    PREFIX_REP,     // REP/REPE/REPZ F3
    PREFIX_REPNEZ,  // REPNE/REPNZ F2
};

const char* PrefixToString(PREFIX prefix);
PREFIX StringToPrefix(const char* Value);
