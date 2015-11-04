#include <windows.h>
#include <stdio.h>
#include "..\src\XEDParse.h"
#include "Tests.h"

void RunTests()
{
    int testCount = ARRAYSIZE(XED_AllTests);
    int successCount = 0;
    for(int i = 0; i < testCount; i++)
    {
        XEDPARSE parse;
        memset(&parse, 0, sizeof(parse));

        // Copy input parameters
        parse.x64 = XED_AllTests[i].LongMode;
        parse.cip = XED_AllTests[i].Ip;

        strcpy_s(parse.instr, XED_AllTests[i].Asm);

        // Try to assemble the string
        if(XEDParseAssemble(&parse) != XEDPARSE_OK)
        {
            printf("Test %i failed: %s\n", i, parse.error);
            continue;
        }

        // Compare output data with the predetermined struct
        // Compare output length
        if(parse.dest_size != XED_AllTests[i].DataLen)
        {
            printf("Test %i failed: Output hex length mismatch (%d != %d)\n", i, parse.dest_size, XED_AllTests[i].DataLen);
            continue;
        }

        // Compare pure data
        if(memcmp(XED_AllTests[i].Data, parse.dest, parse.dest_size) != 0)
        {
            printf("Test %i failed: Output hex mismatch\n", i);
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