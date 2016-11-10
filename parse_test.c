#include <stdio.h>
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
    set_prog_name("parse_test.c");
    line = buffer_create();
    bob = stable_create();
    while(read_line(f, line)) {
        lineNo++;
        printf("line = %s\n", line->data);
        int parseResult = parse(line->data, bob, &instr, &errptr);
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
                    printf("\n");
                else if(i == 2)
                    printf("\n");
                else
                    printf(", ");
            }
            instr = instr->next;
        }
        else if (parseResult && instr == NULL) {
            printf("\tBlank line\n");
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
    }
    fclose(f);
    return 0;
}
