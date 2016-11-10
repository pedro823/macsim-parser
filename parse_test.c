#include <stdio.h>
#include "parser.h"
#include "buffer.h"
#include "stable.h"
#include "asmtypes.h"

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

int main(int argc, char *argv[]) {
    FILE* f;
    Buffer* line;
    SymbolTable bob;
    Instruction *instr;
    int count = 0, lineNo = 0;
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
    line = buffer_create();
    bob = stable_create();
    while(read_line(f, line)) {
        lineNo++;
        printf("line = %s\n", line->data);
        if(parse(line->data, bob, &instr, &errptr)) {
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
                    printf("\n");
                else if(i == 2)
                    printf("\n");
                else
                    printf(", ");
            }
            instr = instr->next;
        }
        else {
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    return 0;
}
