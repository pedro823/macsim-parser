#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "parser.h"
#include "buffer.h"
#include "stable.h"
#include "asmtypes.h"
#include "error.h"
#include "opcodes.h"
#include "asm.h"

void specialcopy(char *newcopy, char *copyable, int begin) {
    int i;
    for(i = 0; i < strlen(copyable); i++) {
        newcopy[i] = copyable[begin + i];
        if(newcopy[i] == ';')
            break;
    }
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

int ispseudo2(const Operator *op) {
    return op->opcode < 0;
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

int assemble(const char* filename, FILE* input, FILE* output) {
    Buffer *line, *p;
    SymbolTable alias_table, label_table, extern_table;
    Instruction **instr, *instVec, *instPtr;
    InsertionResult result;
    EntryData *data;
    int count = 0, lineNo = 0, i, ptr = 0, len;
    const char *errptr;
    char *cptr;
    set_prog_name(filename);
    line = buffer_create();
    p = buffer_create();
    alias_table = stable_create();
    label_table = stable_create();
    extern_table = stable_create();
    addSpecialRegisters(alias_table);
    instr = &instVec;
    while(read_line(input, line)) {
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
            int parseResult = parse(p->data, alias_table, instr, &errptr);
            if (parseResult && *instr != NULL) {
                if ((*instr)->op->opcode == EXTERN && strcmp((*instr)->opds[0]->value.str, "n/a")) {
                    printf("Inserting %s at extern_table\n", (*instr)->opds[0]->value.str);
                    result = stable_insert(extern_table, (*instr)->opds[0]->value.str);
                    if (result.new == 0) {
                        //EXTERN lable defined two times
                        print_error_msg("EXTERN label \"%s\" defined twice\n", (*instr)->opds[0]->value.str);
                        for (; isspace(p->data[ptr]); ptr++);
                        for (; isalpha(p->data[ptr]); ptr++);
                        for (; isspace(p->data[ptr]); ptr++);
                        printf("\n%s", line->data);
                        for (int i = 0; i < ptr; i++)
                            printf(" ");
                        printf("^\n");
                        buffer_destroy(p);
                        buffer_destroy(line);
                        stable_destroy(alias_table);
                        stable_destroy(label_table);
                        stable_destroy(extern_table);
                        return 0;
                    }
                    printf("Setting it to zero\n");
                    result.data->i = 0;
                }
                else if ((*instr)->op->opcode != IS && strcmp((*instr)->label, "n/a")) {
                    result = stable_insert(label_table, (*instr)->label);
                    printf("Inserting %s at label_table\n", (*instr)->label);
                    if (result.new == 0) {
                        //lable defined two times
                        print_error_msg("Label \"%s\" defined twice\n", (*instr)->label);
                        for (; isspace(p->data[ptr]); ptr++);
                        printf("\n%s", line->data);
                        for (int i = 0; i < ptr; i++)
                            printf(" ");
                        printf("^\n");
                        buffer_destroy(p);
                        buffer_destroy(line);
                        stable_destroy(alias_table);
                        stable_destroy(label_table);
                        stable_destroy(extern_table);
                        return 0;
                    }
                    result.data->i = (*instr)->pos;
                    printf("Setting it to %d\n", (*instr)->pos);
                    if (stable_find(extern_table, (*instr)->label) != NULL) {
                        result = stable_insert(extern_table, (*instr)->label);
                        result.data->i = 1;
                        printf("Setting %s to one\n", (*instr)->label);
                    }
                }
                //stable_visit(alias_table, printTable);
                /*
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
                */
                (*instr)->pos = ++count;
                (*instr)->lineno = lineNo;
                switch ((*instr)->op->opcode) {
                    case STR:
                        count += strlen((*instr)->opds[0]->value.str)/4 - 1;
                        break;
                    case CALL:
                        count += 3;
                        break;
                    case RET:
                        count += 2;
                        break;
                    case PUSH:
                        count += 1;
                        break;
                }
                instr = &((*instr)->next);
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
                stable_destroy(alias_table);
                stable_destroy(label_table);
                stable_destroy(extern_table);
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
    printf("HEY\n");
    printf("NULL\n");
    for (instPtr = instVec; instPtr != NULL; instPtr = instPtr->next) {
        if (instPtr->op->opcode == JMP) {
            if (instPtr->opds[0]->type == STRING) {
                data = stable_find(label_table, instPtr->opds[0]->value.str);
                if (data == NULL)
                    fprintf(output, "JMP %s\n", instPtr->opds[0]->value.str);
                else
                    fprintf(output, "%x%02x0000\n", (instPtr->pos < data->i)? 0x48 : 0x49, abs(instPtr->pos - data->i));
            }
            else if (instPtr->opds[0]->type == NUMBER_TYPE) {
                if (instPtr->lineno + instPtr->opds[0]->value.num <= lineNo && instPtr->lineno + instPtr->opds[0]->value.num > 0)
                    fprintf(output, "%x%02x0000\n", (instPtr->opds[0]->value.num > 0)? 0x48 : 0x49, abs(instPtr->opds[0]->value.num));
                else {
                    //ERRO
                    fprintf(stderr, "Jumping out of the code\n");
                }
            }
        }
        else if (instPtr->op->opcode > JMP && instPtr->op->opcode <= GETAB) {
            if (instPtr->opds[1]->type == STRING) {
                data = stable_find(label_table, instPtr->opds[1]->value.str);
                if (data == NULL) {
                    //ERRO (acho)
                    fprintf(stderr, "Cannot find %s label\n", instPtr->opds[1]->value.str);
                }
                else
                    fprintf(output, "%x%02x0000\n", instPtr->op->opcode + ((instPtr->pos < data->i)? 0 : 1), abs(instPtr->pos - data->i));
            }
            else if (instPtr->opds[1]->type == NUMBER_TYPE) {
                if (instPtr->pos + instPtr->opds[1]->value.num <= count && instPtr->pos + instPtr->opds[1]->value.num > 0)
                    fprintf(output, "%x%02x0000\n", instPtr->op->opcode + ((instPtr->pos < data->i)? 0 : 1), abs(instPtr->opds[0]->value.num));
                else {
                    //ERRO
                    fprintf(stderr, "Jumping out of the code\n");
                }
            }
        }
        else if (instPtr->op->opcode == EXTERN) {
            printf("Trying to find %s at extern table\n", instPtr->opds[0]->value.str);
            data = stable_find(extern_table, instPtr->opds[0]->value.str);
            if (data->i == 0) {
                //EXTERN label not defined
                fprintf(stderr, "Label %s wasn't defined\n", instPtr->opds[0]->value.str);
            }
            else
                fprintf(output, "__%s__\n", instPtr->opds[0]->value.str);
        }
        else if (ispseudo2(instPtr->op)) {
            switch (instPtr->op->opcode) {
                case TETRA:
                    fprintf(output, "%08lx\n", instPtr->opds[0]->value.num);
                break;
                case STR:
                    cptr = instPtr->opds[0]->value.str;
                    len = strlen(instPtr->opds[0]->value.str);
                    for (int i = 0; i < len; i++) {
                        fprintf(output, "%02x", *cptr);
                        if ((i+1)%4 == 0)
                            fprintf(output, "\n");
                    }
                    while (len%4 != 0) {
                        fprintf(output, "00");
                        len++;
                    }
                    fprintf(output, "\n");
                break;
                case CALL:
                    fprintf(output, "58fa0400\n");
                    fprintf(output, "1ffafd00\n");
                    fprintf(output, "31fdfd08\n");
                    data = stable_find(label_table, instPtr->opds[0]->value.str);
                    if (data == NULL)
                        fprintf(output, "JMP %s\n", instPtr->opds[0]->value.str);
                    else
                        fprintf(output, "%x%02x0000\n", (instPtr->pos < data->i)? 0x48 : 0x49, abs(instPtr->pos - data->i));
                break;
                case RET:
                    fprintf(output, "18fdfd%02lx\n", (instPtr->opds[0]->value.num + 1)*8);
                    fprintf(output, "0efafd%02lx\n", instPtr->opds[0]->value.num * 8);
                    fprintf(output, "56fa0000\n");
                break;
                case PUSH:
                    fprintf(output, "1f%02lxfd00\n", instPtr->opds[0]->value.num);
                    fprintf(output, "31fdfd08\n");
                break;
            }
        }
        else if (instPtr->op->opcode < JMP){
            if (instPtr->opds[2]->type == REGISTER)
                fprintf(output, "%02x%02lx%02lx%02lx\n", instPtr->op->opcode, instPtr->opds[0]->value.num, instPtr->opds[1]->value.num, instPtr->opds[2]->value.num);
            else
                fprintf(output, "%02x%02lx%02lx%02lx\n", instPtr->op->opcode+1, instPtr->opds[0]->value.num, instPtr->opds[1]->value.num, instPtr->opds[2]->value.num);
        }
        else {
            fprintf(output, "%02x", instPtr->op->opcode);
            for (int i = 0; i < 2; i++)
                if (instPtr->op->opd_types[i] != OP_NONE)
                    fprintf(output, "%02lx", instPtr->opds[i]->value.num);
                else
                    fprintf(output, "00");
            fprintf(output, "\n");
        }
    }
    verify(alias_table);
    buffer_destroy(p);
    buffer_destroy(line);
    stable_destroy(alias_table);
    stable_destroy(label_table);
    stable_destroy(extern_table);
    //Destroy instr linked list
    return 1;
}
