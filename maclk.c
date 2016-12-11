#include <stdio.h>
#include "buffer.h"
#include "taglist.h"
#include <string.h>
#include <stdlib.h>

void printUsage(char* name) {
    printf("Usage: %s <Outfile> [objects...]\n", name);
}

int isJump(char* line) {
    return line[0] == 'J';
}

int isTag(char* line) {
    return line[0] == '_' && line[1] && line[1] == '_';
}

// Formata tag para um JMP tag.
// DESTRUTIVA -- Não usar em dados sensíveis
char* format(char* tag) {
    int i = 2;
    while(!(tag[i] == '_' && tag[i + 1] == '_'))
        i++;
    tag[i] = '\0';
    return tag + 2;
}

// converte int para hex
char* itoh(int num) {
    char* ret = malloc(6);
    int i;
    for(i = 0; i < 6; i++)
        ret[i] = '0';
    i = 5;
    while(num) {
        if(num % 16 < 10) {
            ret[i--] = num % 16 + '0';
        }
        else {
            ret[i--] = (num % 16) - 10 + 'a';
        }
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
            break;

        case 3:
            // Main nunca foi definida!
            fprintf(stderr, "ERROR: %s: main was not defined.\n", name);
            break;

        case 4:
            // Invalid file!
            fprintf(stderr, "ERROR: %s: Invalid file specified: ", name);
            break;

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
    fseek(outfile, 0, SEEK_SET);
    Buffer* aux = buffer_create();
    if(strcmp(jumpName, "main") == 0) {
        read_line(outfile, aux);
        if(strcmp(aux->data, "JMPTOMAIN  \n") != 0) {
            buffer_destroy(aux);
            return -1;
        }
        fseek(outfile, -aux->i, SEEK_CUR);
        fprintf(outfile, "48");
        char *n = itoh(line);
        fprintf(outfile, "%s   ", n);
    }
    else {
        // Inseguro -- buffer overflow possível?
        char jumpFind[300];
        //

        int findLine = 1, tmp, tmp2;
        sprintf(jumpFind, "JMP %s\n", jumpName);
        while(tmp = read_line(outfile, aux)) {
            if((tmp2 = strcmp(jumpFind, aux->data)) == 0) {
                int diff = line - findLine;
                fseek(outfile, -tmp, SEEK_CUR);
                if(diff < 0) {
                    // read_line avança o apontador do próximo caractere
                    fprintf(outfile, "49");
                    diff = -diff;
                }
                else {
                    fprintf(outfile, "48");
                }
                char* toPrint = itoh(diff);
                // Pula para a linha correta
                fprintf(outfile, "%-300s\n", toPrint);
            }
            findLine++;
        }
    }
    fsetpos(outfile, &last);
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
        printUsage(argv[0]);
        return -1;
    }
    Buffer *b = buffer_create();
    FILE* outfile = fopen(argv[1], "w+"); // Cria a Outfile
    FILE** files = malloc(noFiles * sizeof(FILE*));
    _Tag tlist = tag_create();
    for(int i = 0; i < noFiles; i++) {
        files[i] = fopen(argv[i + 2], "r");
        if(files[i] == NULL) {
            printError(argv[0], 4);
            fprintf(stderr, "%s\n", argv[i + 2]);
            return 0;
        }
    }
    //Deixa um JMPTOMAIN temporário
    fprintf(outfile, "JMPTOMAIN  \n");
    int line = 1, aux;
    for(int i = 0; i < noFiles; i++) {
        while(read_line(files[i], b) != 0) {
            if(isTag(b->data)) {
                // guarda todas as tags em uma lista ligada
                char* temp = format(b->data); // Formata a tag
                tag_insert(&tlist, temp, line);
            }
            else {
                // Tags devem ser invisíveis
                fprintf(outfile, "%s", b->data);
                line++;
            }
        }
    }
    for(_Tag i = tlist; i != NULL; i = i->next) {
        if((aux = correctJump(outfile, argv[1], i->tagName, i->linePos)) < 0) {
            // Erro em correctJump
            printError(argv[0], -aux);
            fclose(outfile);
            tag_destroy(tlist);
            for(int j = 0; j < noFiles; j++) {
                fclose(files[j]);
            }
            return -1;
        }
    }
    rewind(outfile);
    while(read_line(outfile, b) != 0) {
        if(isJump(b->data)) {
            printError(argv[0], 2);
            fprintf(stderr, "%s", b->data + 4);
            fclose(outfile);
            tag_destroy(tlist);
            for(int i = 0; i < noFiles; i++) {
                fclose(files[i]);
            }
            return -1;
        }
    }
    fclose(outfile);
    tag_destroy(tlist);
    for(int i = 0; i < noFiles; i++) {
        fclose(files[i]);
    }
    return 0;
}
