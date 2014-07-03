#include <windows.h>
#include <stdio.h>
#include "..\XEDParse.h"

int main(int argc, char* argv[])
{
    if(argc<2) //no arguments provided
    {
        XEDPARSE parse;
        memset(&parse, 0, sizeof(parse));
#ifdef _WIN64
        parse.x64 = true;
#else
        parse.x64 = false;
#endif
        parse.cip=0;
        char instr[256]="";
        puts("instruction:");
        fgets(instr, 256, stdin);
        instr[strlen(instr)-1]=0;
        strcpy(parse.instr, instr);
        XEDParseAssemble(&parse);
        printf("error:\n%s\n", parse.error);
        puts("bytes:");
        for(unsigned int i=0; i<parse.dest_size; i++)
            printf("%.2X ", parse.dest[i]);
        puts("");
        system("pause");
        return 0;
    }
    //parse an assembly text file with instructions
    HANDLE hFile=CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(hFile==INVALID_HANDLE_VALUE)
    {
        puts("failed to open file!");
        return 1;
    }
    unsigned int filesize=GetFileSize(hFile, 0);
    char* filedata=(char*)malloc(filesize);
    if(!filedata)
    {
        CloseHandle(hFile);
        puts("failed to allocate memory!");
        return 1;
    }
    DWORD read=0;
    if(!ReadFile(hFile, filedata, filesize, &read, 0))
    {
        free(filedata);
        CloseHandle(hFile);
        puts("failed to read file!");
        return 1;
    }
    CloseHandle(hFile);
    int len=strlen(filedata);
    char current_instr[XEDPARSE_MAXBUFSIZE]="";

    int errors=0;
    DWORD ticks=GetTickCount();
    XEDPARSE current;
    memset(&current, 0, sizeof(current));
#ifdef _WIN64
    current.x64 = true;
#else
    current.x64 = false;
#endif
    current.cip=0x10000000;
    for(int i=0,j=0; i<len; i++)
    {
        if(filedata[i]=='\n' || filedata[i+1]=='\n') //newline
        {
            strcpy(current.instr, current_instr);
            if(XEDParseAssemble(&current)==XEDPARSE_ERROR)
            {
                printf("%s\n%s\n\n", current_instr, current.error);
                errors++;
                current.cip++;
            }
            else
                current.cip+=current.dest_size;
            while(filedata[i]=='\r' || filedata[i]=='\n')
                i++;
            i--;
            j=0;
        }
        else
            j+=sprintf(current_instr+j, "%c", filedata[i]);
    }
    free(filedata);
    printf("\nerrors: %d\n%ums passed\ndone!", errors, GetTickCount()-ticks);
    return 0;
}
