#include <windows.h>
#include "..\XEDParse.h"
#include <stdio.h>

int main()
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
