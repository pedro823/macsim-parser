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

int printTable (const char *key, EntryData *data) {
    printf("%s\n", key);
    return 1;
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

int visit(const char *key, EntryData *data) {
    if(data->opd == NULL) {
        print_error_msg("error: label \"%s\" was never declared", key);
        return 0;
    }
    return 1;
}

int verify(SymbolTable table) {
    return stable_visit(table, visit);
}

void addSpecialRegisters(SymbolTable table) {
    InsertionResult result;
    result = stable_insert(table, "rA");
    if(result.new) {
        result.data->opd = operand_create_register(255);
    }
    result = stable_insert(table, "rR");
    if(result.new) {
        result.data->opd = operand_create_register(254);
    }
    result = stable_insert(table, "rSP");
    if(result.new) {
        result.data->opd = operand_create_register(253);
    }
    result = stable_insert(table, "rX");
    if(result.new) {
        result.data->opd = operand_create_register(252);
    }
    result = stable_insert(table, "rY");
    if(result.new) {
        result.data->opd = operand_create_register(251);
    }
    result = stable_insert(table, "rZ");
    if(result.new) {
        result.data->opd = operand_create_register(250);
    }
}

int main(int argc, char *argv[]) {
    FILE* f;
    Buffer *line, *p;
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
    p = buffer_create();
    bob = stable_create();
    addSpecialRegisters(bob);
    while(read_line(f, line)) {
        lineNo++;
        buffer_reset(p);
        for(i = 0; i < line->i; i++) {
            if(line->data[i] != ';') {
                buffer_push_back(p, line->data[i]);
            }
            else {
                i++;
                break;
            }
        }
        while(1) {
            int parseResult = parse(p->data, bob, &instr, &errptr);
            if(parseResult && instr != NULL) {
                instr->pos = ++count;
                instr->lineno = lineNo;
                //stable_visit(bob, printTable);
                printf("line = %s%c", p->data, "\n\0"[(p->data[p->i-1] == '\n')]);
                if(strcmp(instr->label, "n/a") == 0)
                    printf("label = n/a\n");
                else
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
            /*else if (parseResult && instr == NULL) {
                printf("Blank line\n\n");
            }*/
            else if (!parseResult){
                //Some error ocurred: print error message
                print_error_msg(NULL);
                printf("\n%s", line->data);
                for (int i = 0; p->data + i <= errptr; i++)
                    printf(" ");
                printf("^\n");
                buffer_destroy(p);
                buffer_destroy(line);
                stable_destroy(bob);
                fclose(f);
                return 0;
            }
            if(i < line->i) {
                buffer_reset(p);
                for(; i < line->i; i++) {
                    if(line->data[i] != ';') {
                        buffer_push_back(p, line->data[i]);
                    }
                    else {
                        i++;
                        break;
                    }
                }
            }
            else
                break;
        }
    }
    verify(bob);
    buffer_destroy(p);
    buffer_destroy(line);
    stable_destroy(bob);
    fclose(f);
    return 0;
}
