#include <windows.h>
#include <stdio.h>
#include "..\src\XEDParse.h"
#include "Tests.h"

static void printTest(const XED_TestEntry & test, const XEDPARSE & result)
{
    printf("  \"%s\", IP: 0x%llX, Mode: %s\n", test.Asm, test.Ip, test.LongMode ? "x64" : "x32");
    printf("  Expected (%i) ", test.DataLen);
    for (int i = 0; i < test.DataLen; i++)
        printf("%02X ", (unsigned char)test.Data[i]);
    puts("");
    printf("  Actual (%i)   ", result.dest_size);
    for (unsigned int i = 0; i < result.dest_size; i++)
        printf("%02X ", result.dest[i]);
    puts("");
}

void PrintTests()
{
    int testCount = ARRAYSIZE(XED_AllTests);
    for (auto i = 0; i < testCount; i++)
    {
        auto & test = XED_AllTests[i];
        printf("runTest(%d, %s, 0x%llX, [", i, test.LongMode ? "True" : "False", test.Ip);
        if (test.DataLen != -1)
        {
            for (auto j = 0; j < test.DataLen; j++)
            {
                if (j)
                    printf(", ");
                printf("0x%02X", (unsigned char)test.Data[j]);
            }
        }
        printf("], \"%s\")\n", test.Asm);
    }
}

void RunTests()
{
    int testCount = ARRAYSIZE(XED_AllTests);
    int successCount = 0;
    for(int i = 0; i < testCount; i++)
    {
        XEDPARSE parse;
        memset(&parse, 0, sizeof(parse));
        auto & test = XED_AllTests[i];

        // Copy input parameters
        parse.x64 = test.LongMode;
        parse.cip = test.Ip;

        strcpy_s(parse.instr, test.Asm);

        // Try to assemble the string
        if(XEDParseAssemble(&parse) != XEDPARSE_OK)
        {
            // Did the test expect a failure?
            if (test.DataLen == -1)
                successCount++;
            else
            {
                printf("Test %i failed: %s\n", i, parse.error);
                printTest(test, parse);
            }

            continue;
        }

        // Compare output data with the predetermined struct
        // Compare output length
        if (test.DataLen != parse.dest_size)
        {
            printf("Test %i failed: Output hex length mismatch (expected %d, actual %d)\n", i, test.DataLen, parse.dest_size);
            printTest(test, parse);
            continue;
        }

        // Compare pure data
        if (memcmp(test.Data, parse.dest, parse.dest_size) != 0)
        {
            printf("Test %i failed: Output hex mismatch\n", i);
            printTest(test, parse);
            continue;
        }

        successCount++;
    }

    printf("%i/%i tests were successful!\n", successCount, testCount);
}

int main(int argc, char* argv[])
{
    if(argc < 2) // No arguments provided
    {
        /*PrintTests();
        return 0;*/
        // Run tests first
        RunTests();

        // Now prompt the user to assemble an instruction
        XEDPARSE parse;
        memset(&parse, 0, sizeof(parse));
#ifdef _WIN64
        parse.x64 = true;
#else
        parse.x64 = false;
#endif
        parse.cip = 0;
        char instr[256] = "";
        puts("instruction (cip = 0):");
        fgets(instr, 256, stdin);
        instr[strlen(instr) - 1] = 0;
        strcpy(parse.instr, instr);
        XEDParseAssemble(&parse);
        printf("error:\n%s\n", parse.error);
        puts("bytes:");
        for(unsigned int i = 0; i < parse.dest_size; i++)
            printf("%.2X ", parse.dest[i]);
        puts("");
        system("pause");
        return 0;
    }

    // Parse an assembly text file with instructions
    HANDLE hFile = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        puts("failed to open file!");
        return 1;
    }
    unsigned int filesize = GetFileSize(hFile, 0);
    char* filedata = (char*)malloc(filesize);
    if(!filedata)
    {
        CloseHandle(hFile);
        puts("failed to allocate memory!");
        return 1;
    }
    DWORD read = 0;
    if(!ReadFile(hFile, filedata, filesize, &read, 0))
    {
        free(filedata);
        CloseHandle(hFile);
        puts("failed to read file!");
        return 1;
    }
    CloseHandle(hFile);
    int len = strlen(filedata);
    char current_instr[XEDPARSE_MAXBUFSIZE] = "";

    int errors = 0;
    DWORD ticks = GetTickCount();
    XEDPARSE current;
    memset(&current, 0, sizeof(current));
#ifdef _WIN64
    current.x64 = true;
#else
    current.x64 = false;
#endif
    current.cip = 0x180001000;
    for(int i = 0, j = 0; i < len; i++)
    {
        if(filedata[i] == '\n' || filedata[i + 1] == '\n') //newline
        {
            if(strstr(current_instr, "base:"))
                sscanf(current_instr, "base:0x%llX", &current.cip);
            else
            {
                strcpy(current.instr, current_instr);
                if(XEDParseAssemble(&current) == XEDPARSE_ERROR)
                {
                    printf("%s\n%s\n\n", current_instr, current.error);
                    errors++;
                    current.cip++;
                }
                else
                    current.cip += current.dest_size;
            }
            while(filedata[i] == '\r' || filedata[i] == '\n')
                i++;
            i--;
            j = 0;
        }
        else
            j += sprintf(current_instr + j, "%c", filedata[i]);
    }
    free(filedata);
    printf("\nerrors: %d\n%ums passed\ndone!", errors, GetTickCount() - ticks);
    return 0;
}