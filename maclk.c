#include <stdio.h>
#include "buffer.h"
#include "taglist.h"
#include <string.h>

void printUsage(char* name) {
    printf("Usage: %s <Outfile> [objects...]\n", name);
}

int isJump(char* line) {
    return line[0] == 'J';
}

int isTag(char* line) {
    return line[0] == '_' && line[1] && line[1] == '_';
}

// converts int to hex
char* itoh(int num) {
    char ret[6];
    int i;
    for(i = 0; i < 6; i++)
        ret[i] = '0';
    i = 0;
    while(num) {
        ret[i++] = n % 16 + '0';
        num >>= 4;
    }
    return ret;
}

void printError(char* name, int error) {
    switch(error) {
        case 0:
            // Nenhum arquivo de entrada especificado
            fprintf(stderr, "ERROR: %s: No object files specified.\n", name);
            break;

        case 1:
            // Main foi definida duas vezes!
            fprintf(stderr, "ERROR: %s: main was defined twice.\n", name);
            break;

        case 2:
            // Referência não definida, para ser usado na
            // checagem final de se não há nenhuma referência
            // faltando.
            fprintf(stderr, "ERROR: %s: Undefined reference to ", name);

        default:
            fprintf(stderr, "ERROR: %s: Unknown error.\n", name);
    }
}

// Volta no arquivo e corrige qualquer pulo que não foi registrado
// retorna 1 se funcionou ou -error se não funcionou
int correctJump(FILE* outfile, char* outfileName, char* jumpName, int line) {
    fpos_t last;
    fgetpos(outfile, &last);
    fflush(outfile);
    Buffer aux = buffer_create();
    fseek(outfile, 0, SEEK_SET);
    if(strcmp(jumpName, "main")) {
        read_line(outfile, aux)
        if(strcmp(aux->data, "JMPTOMAIN  \n" != 0)) {
            buffer_destroy(aux);
            return -1;
        }
        fsetpos(outfile, -aux->i, SEEK_CUR)
        fprintf(outfile, "48");
        char n[6] = itoh(line);
        fprintf(outfile, "%s\n", n);
    }
    else {
        char jumpFind[300];
        int findLine = 0;
        sprintf(jumpFind, "JMP %s\n", jumpName);
        while(read_line(outfile, aux)) {
            if(strcmp(jumpFind, aux->data) == 0) {
                int diff = line - findLine;
                if(diff < 0) {
                    // TODO: CHECAR SE O READ_LINE AVANÇA
                    // O APONTADOR DE PROXIMO CARACTERE
                    fprintf(outfile, "49");
                    diff = -diff;
                }
                else {
                    fprintf(outfile, "48");
                }
                char toPrint[6] = itoh(diff);
                // Pula para a linha correta
                fprintf(outfile, "%s\n", toPrint);
            }
            findLine++;
        }
    }
    fsetpos(outfile, last);
    buffer_destroy(aux);
    return 1;
}

int main(int argc, char **argv) {
    if(argc < 2) {
        printUsage(argv[0]);
        return -1;
    }
    // Quantidade de arquivos
    int noFiles = argc - 2;
    if(noFiles == 0) {
        printError(argv[0], 0);
        printUsage();
        return -1;
    }
    Buffer *b = buffer_create();
    FILE* outfile = fopen(argv[1], "w+"); // Cria a Outfile
    FILE** files = malloc(noFiles * sizeof(FILE*));
    for(int i = 0; i < noFiles; i++) {
        files[i] = fopen(argv[i + 2]);
    }
    //Deixa um JMPTOMAIN temporário
    fprintf(outfile, "JMPTOMAIN  \n");
    int line = 1, aux;
    for(int i = 0; i < noFiles; i++) {
        while(read_line(files[i], b) != 0) {
            if(isTag(b->data)) {
                // guarda todas as tags em um buffer
            }
            else {
            }
            fprintf(outfile, "%s", b->data);
            line++;
        }
    }
    fclose(outfile);
    for(i = 0; i < noFiles; i++) {
        fclose(files[i]);
    }
    return 0;
}
