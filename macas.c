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
    if (argc != 2) {
        printUsage(argv[0]);
        return 0;
    }
    outName = emalloc((strlen(argv[1])+5)*sizeof(char));
    while (argv[1][i] != '.' && argv[1][i] != '\0') {
        outName[i] = argv[1][i];
        i++;
    }
    strcpy(&(outName[i]), ".maco\0");
    in = fopen(argv[1], "r");
    out = fopen(outName, "w+");
    if (in == NULL)
        fprintf(stderr, "ERROR: Cannot find %s\n", argv[1]);
    assemble(argv[1], in, out);
    fclose(in);
    fclose(out);
}
