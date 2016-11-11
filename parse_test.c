#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "buffer.h"
#include "stable.h"
#include "asmtypes.h"
#include "error.h"

void printUsage() {
    printf("Usage: parse <file>\n");
}

char* decodeOperand(unsigned int optype) {
    switch (optype){
        case NUMBER_TYPE:
            return "Number";
            break;
        case REGISTER:
            return "Register";
            break;
        case LABEL:
            return "Label";
            break;
        case STRING:
            return "String";
            break;
    }
    return NULL;
}

void specialcopy(char *newcopy, char *copyable, int begin) {
    int i;
    for(i = 0; i < strlen(copyable); i++) {
        newcopy[i] = copyable[begin + i];
        if(newcopy[i] == ';')
            break;
    }
    printf("specialcopy read %d characters.\n", i);
}

int main(int argc, char *argv[]) {
    FILE* f;
    Buffer* line;
    SymbolTable bob;
    Instruction *instr;
    int count = 0, lineNo = 0, i;
    const char *errptr;
    char* type;

    if(argc != 2) {
        printUsage();
        return 0;
    }
    f = fopen(argv[1], "r");
    if(f == NULL) {
        printf("ERROR: parse: Invalid file");
        return -1;
    }
    set_prog_name("parse_test.c");
    line = buffer_create();
    bob = stable_create();
    while(read_line(f, line)) {
        char *p = malloc(strlen(line->data) * sizeof(char));
        lineNo++;
        printf("line = ");
        for(i = 0; i < line->i; i++) {
            if(line->data[i] != ';') {
                printf("%c", line->data[i]);
                p[i] = line->data[i];
            }
            else {
                i++;
                printf("\n");
                break;
            }
        }
        int ini = 0;
        while(1) {
            int parseResult = parse(p, bob, &instr, &errptr);
            if(parseResult && instr != NULL) {
                instr->pos = ++count;
                instr->lineno = lineNo;
                printf("label = \"%s\"\n", instr->label);
                printf("operator = %s\n", instr->op->name);
                printf("operands = ");
                for(int i = 0; i < 3 && instr->opds[i]; i++) {
                    type = decodeOperand(instr->opds[i]->type);
                    if(instr->opds[i]->type == NUMBER_TYPE)
                        printf("%s(%lld)", type, instr->opds[i]->value.num);
                    else if(instr->opds[i]->type == REGISTER)
                        printf("%s(%d)", type, instr->opds[i]->value.reg);
                    else if(instr->opds[i]->type == LABEL)
                        printf("%s(%s)", type, instr->opds[i]->value.label);
                    else
                        printf("%s(%s)", type, instr->opds[i]->value.str);
                    if(i < 2 && !(instr->opds[i + 1]))
                        printf("\n\n");
                    else if(i == 2)
                        printf("\n\n");
                    else
                        printf(", ");
                }
                instr = instr->next;
            }
            else if (parseResult && instr == NULL) {
                printf("Blank line\n\n");
            }
            else {
                //Some error ocurred: print error message
                print_error_msg(NULL);
                printf("\n%s", line->data);
                printf("line = %p\nerrptr = %p\n", line+1, errptr);
                for (int i = 0; line + i <= errptr; i++)
                    printf(" ");
                printf("^\n");
                fclose(f);
                return 0;
            }
            if(i < line->i) {
                printf("line = ");
                ini = i;
                for(int a = 0; a < line->i; a++)
                    p[a] = 0;
                for(; i < line->i; i++) {
                    if(line->data[i] != ';') {
                        printf("%c", line->data[i]);
                        p[i-ini] = line->data[i];
                    }
                    else {
                        i++;
                        printf("\n");
                        break;
                    }
                }
            }
            else
                break;
        }
    }
    fclose(f);
    return 0;
}
