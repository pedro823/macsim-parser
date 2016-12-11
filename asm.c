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

int ispseudo2(const Operator *op) {
    return op->opcode < 0;
}

//Adiciona registradores especiais à alias_table
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
        //Coloca a linha no buffer até achar ";" ou até a linha terminar
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
                //Se o operador for um EXTERN, adiciona o operando na extern_table
                if ((*instr)->op->opcode == EXTERN && strcmp((*instr)->opds[0]->value.str, "n/a")) {
                    result = stable_insert(extern_table, (*instr)->opds[0]->value.str);
                    if (result.new == 0) {
                        //Há dois EXTERNs com a mesma label
                        print_error_msg("EXTERN label \"%s\" defined twice", (*instr)->opds[0]->value.str);
                        for (; isspace(p->data[ptr]); ptr++);
                        for (; isalpha(p->data[ptr]); ptr++);
                        for (; isspace(p->data[ptr]); ptr++);
                        fprintf(stderr, "\n%s", line->data);
                        for (int i = 0; i < ptr; i++)
                            fprintf(stderr, " ");
                        fprintf(stderr, "^\n");
                        buffer_destroy(p);
                        buffer_destroy(line);
                        stable_destroy(alias_table);
                        stable_destroy(label_table);
                        stable_destroy(extern_table);
                        return 0;
                    }
                    result.data->i = 0;
                }
                //Se a instrução tiver uma label, adiciona na label_table
                else if ((*instr)->op->opcode != IS && strcmp((*instr)->label, "n/a")) {
                    result = stable_insert(label_table, (*instr)->label);
                    if (result.new == 0) {
                        //A mesma label foi definida duas vezes
                        print_error_msg("Label \"%s\" defined twice", (*instr)->label);
                        for (; isspace(p->data[ptr]); ptr++);
                        fprintf(stderr, "\n%s", line->data);
                        for (int i = 0; i < ptr; i++)
                            fprintf(stderr, " ");
                        fprintf(stderr, "^\n");
                        buffer_destroy(p);
                        buffer_destroy(line);
                        stable_destroy(alias_table);
                        stable_destroy(label_table);
                        stable_destroy(extern_table);
                        return 0;
                    }
                    //Se a label foi definida em um EXTERN, sinaliza para a
                    //extern_table que ela foi achada
                    result.data->i = count + 1;
                    if (stable_find(extern_table, (*instr)->label) != NULL) {
                        result = stable_insert(extern_table, (*instr)->label);
                        result.data->i = 1;
                    }
                }
                (*instr)->pos = ++count;
                (*instr)->lineno = lineNo;
                //Se o operador for um pseudo-operador, pula mais linhas
                switch ((*instr)->op->opcode) {
                    case STR:
                        count += (strlen((*instr)->opds[0]->value.str) - 1)/4;
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
            else if (!parseResult){
                //Ocorreu algum erro, printa uma mensagem de erro
                print_error_msg(NULL);
                fprintf(stderr, "\n%s", line->data);
                for (int i = 0; p->data + i <= errptr; i++)
                    fprintf(stderr, " ");
                fprintf(stderr, "^\n");
                buffer_destroy(p);
                buffer_destroy(line);
                stable_destroy(alias_table);
                stable_destroy(label_table);
                stable_destroy(extern_table);
                return 0;
            }
            if (i < line->i) {
                //Lê a próxima instrução se há mais de uma instrução na linha
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
    //Imprime a lista de instruções como código de máquina
    for (instPtr = instVec; instPtr != NULL; instPtr = instPtr->next) {
        if (instPtr->op->opcode == JMP) {
            if (instPtr->opds[0]->type == LABEL) {
                data = stable_find(label_table, instPtr->opds[0]->value.str);
                if (data == NULL)
                    fprintf(output, "JMP %s\n", instPtr->opds[0]->value.str);
                else
                    fprintf(output, "%x%06lx\n", (instPtr->pos < data->i)? 0x48 : 0x49, abs(instPtr->pos - data->i));
            }
            else if (instPtr->opds[0]->type == NUMBER_TYPE) {
                if (instPtr->lineno + instPtr->opds[0]->value.num <= lineNo && instPtr->lineno + instPtr->opds[0]->value.num > 0)
                    fprintf(output, "%x%06lx\n", (instPtr->opds[0]->value.num > 0)? 0x48 : 0x49, abs(instPtr->opds[0]->value.num));
                else {
                    //O destino do JMP passa da quantidade de linhas do código,
                    //ou é negativo
                    print_error_msg("Jump destiny is not in the code");
                    return 0;
                }
            }
        }
        else if (instPtr->op->opcode > JMP && instPtr->op->opcode <= GETAB) {
            if (instPtr->opds[1]->type == LABEL) {
                data = stable_find(label_table, instPtr->opds[1]->value.str);
                if (data == NULL) {
                    //Não sei se isso é um erro
                    print_error_msg("Cannot find \"%s\" label", instPtr->opds[1]->value.str);
                    return 0;
                }
                else
                    fprintf(output, "%x%02lx%04lx\n", instPtr->op->opcode + ((instPtr->pos < data->i)? 0 : 1), instPtr->opds[0]->value.num, abs(instPtr->pos - data->i));
            }
            else if (instPtr->opds[1]->type == NUMBER_TYPE) {
                if (instPtr->pos + instPtr->opds[1]->value.num <= count && instPtr->pos + instPtr->opds[1]->value.num > 0)
                    fprintf(output, "%x%02lx%04lx\n", instPtr->op->opcode + ((instPtr->pos < data->i)? 0 : 1), instPtr->opds[0]->value.num, abs(instPtr->opds[1]->value.num));
                else {
                    //O destino do JMP passa da quantidade de linhas do código,
                    //ou é negativo
                    print_error_msg("Jump destiny is not in the code");
                    return 0;
                }
            }
        }
        else if (instPtr->op->opcode == EXTERN) {
            data = stable_find(extern_table, instPtr->opds[0]->value.str);
            if (data->i == 0) {
                //A label de um EXTERN não foi definida em nenhum lugar
                print_error_msg("EXTERN label \"%s\" wasn't defined", instPtr->opds[0]->value.str);
                return 0;
            }
            else
                fprintf(output, "__%s__\n", instPtr->opds[0]->value.str);
        }
        else if (ispseudo2(instPtr->op)) {
            //Transforma os pseudo-operadores em vários códigos de máquina
            switch (instPtr->op->opcode) {
                case TETRA:
                    fprintf(output, "%08lx\n", instPtr->opds[0]->value.num);
                break;
                case STR:
                    cptr = instPtr->opds[0]->value.str;
                    len = strlen(instPtr->opds[0]->value.str);
                    for (int i = 0; i < len; i++, cptr = cptr+1) {
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
                    fprintf(output, "58fa0004\n");
                    fprintf(output, "1ffafd00\n");
                    fprintf(output, "31fdfd08\n");
                    data = stable_find(label_table, instPtr->opds[0]->value.str);
                    if (data == NULL)
                        fprintf(output, "JMP %s\n", instPtr->opds[0]->value.str);
                    else
                        fprintf(output, "%x%06lx\n", (instPtr->pos < data->i)? 0x48 : 0x49, abs(instPtr->pos - data->i) + ((instPtr->pos < data->i)? 0 : 3));
                break;
                case RET:
                    fprintf(output, "18fdfd%02lx\n", (instPtr->opds[0]->value.num + 1)*8);
                    fprintf(output, "0ffafd%02lx\n", instPtr->opds[0]->value.num * 8);
                    fprintf(output, "56fa0000\n");
                break;
                case PUSH:
                    fprintf(output, "1f%02lxfd00\n", instPtr->opds[0]->value.num);
                    fprintf(output, "31fdfd08\n");
                break;
            }
        }
        else if (instPtr->op->opcode < JMP){
            //Cuida de todos os operadores que tem uma versão imediata
            if (instPtr->opds[2]->type == REGISTER)
                fprintf(output, "%02x%02lx%02lx%02lx\n", instPtr->op->opcode, instPtr->opds[0]->value.num, instPtr->opds[1]->value.num, instPtr->opds[2]->value.num);
            else
                fprintf(output, "%02x%02lx%02lx%02lx\n", instPtr->op->opcode+1, instPtr->opds[0]->value.num, instPtr->opds[1]->value.num, instPtr->opds[2]->value.num);
        }
        else if (instPtr->op->opcode == SETW) {
            fprintf(output, "5a%02lx%04lx\n", instPtr->opds[0]->value.num, instPtr->opds[1]->value.num);
        }
        else if (instPtr->op->opcode == INT){
            fprintf(output, "fe%06lx\n", instPtr->opds[0]->value.num);
        }
        else {
            fprintf(output, "%02x", instPtr->op->opcode);
            for (int i = 0; i < 3; i++)
                if (instPtr->op->opd_types[i] != OP_NONE)
                    fprintf(output, "%02lx", instPtr->opds[i]->value.num);
                else
                    fprintf(output, "00");
            fprintf(output, "\n");
        }
    }
    buffer_destroy(p);
    buffer_destroy(line);
    stable_destroy(alias_table);
    stable_destroy(label_table);
    stable_destroy(extern_table);
    instr_destroy(instVec);
    return 1;
}
