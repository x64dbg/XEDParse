#include <windows.h>
#include "..\XEDParse.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
    if(argc<2) //no arguments provided
    {
        XEDPARSE parse;
        memset(&parse, 0, sizeof(parse));
        char instr[256]="";
        puts("instruction:");
        fgets(instr, 256, stdin);
        instr[strlen(instr)-1]=0;
        strcpy(parse.instr, instr);
        XEDParseAssemble(&parse);
        puts("bytes:");
        for(int i=0; i<parse.dest_size; i++)
            printf("0x%.2X ", parse.dest[i]);
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
    XEDPARSE current;
    int errors=0;
    for(int i=0,j=0; i<len; i++)
    {
        if(filedata[i]=='\n' || filedata[i+1]=='\n') //newline
        {
            memset(&current, 0, sizeof(current));
            strcpy(current.instr, current_instr);
            puts(current_instr);
            if(XEDParseAssemble(&current)==XEDPARSE_ERROR)
                errors++;
            for(int k=0; k<current.dest_size; k++)
                printf("%.2X ", current.dest[k]);
            while(filedata[i]=='\r' || filedata[i]=='\n')
                i++;
            i--;
            j=0;
        }
        else
            j+=sprintf(current_instr+j, "%c", filedata[i]);
    }
    free(filedata);
    printf("\nerrors: %d\ndone!", errors);
    return 0;
}
