#include <stdio.h>
#include <string.h>
#include "error.h"
#include "asm.h"

void printUsage(const char* name) {
    printf("Usage: %s <Input file>\n", name);
}

int main (int argc, const char **argv) {
    FILE *in, *out;
    char *outName;
    int i = 0;
    /*if (argc != 2) {
        printUsage(argv[0]);
        return 0;
    }*/
    printf("HEY-1\n");
    outName = emalloc((strlen(argv[1])+5)*sizeof(char));
    printf("HEY0\n");
    while (argv[1][i] != '.' && argv[1][i] != '\0') {
        printf("%c", argv[1][i]);
        outName[i] = argv[1][i];
        i++;
    }
    printf("HEY1\n");
    strcpy(&(outName[i]), ".maco\0");
    printf("HEY2\n");
    in = fopen(argv[1], "r");
    printf("HEY3\n");
    out = fopen(outName, "w+");
    printf("HEY4\n");
    if (in == NULL)
        fprintf(stderr, "ERROR: Cannot find %s\n", argv[1]);
    assemble(argv[1], in, out);
    fclose(in);
    fclose(out);
}
