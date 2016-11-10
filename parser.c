#include "error.h"
#include "stable.h"
#include "buffer.h"
#include "parser.h"
#include "optable.h"
#include "mactypes.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

int ispseudo(Operator op) {
    return op.opcode < 0;
}

int atoh(char* s) {
    int ret = 0;
    while(*s) {
        if(*s)
    }
}

int parse(const char *s, SymbolTable alias_table, Instruction **instr, const char **errptr) {
    char *line, *label;
    Buffer *analizer;
    InsertionResult result;
    Instruction *finalInstr;
    int i, j, len;

    line = estrdup(s);
    len = strlen(line);
    for(i = 0; i < len && line[i] <= 32; i++); // Takes off all preceding spaces
    analizer = buffer_create();
    printf("parse:\n");
    printf("\tInitialized all variables\n");
    if(i >= len || line[i] == '*') {
        // Line is composed only by comment
        return 1;
    }
    printf("\tLine has stuff\n");
    for(; line[i] > 32 && line[i] != ';' && line[i] != '*'; i++) {
        buffer_push_back(analizer, line[i]);
    }
    printf("\tBuffer Read:\n\t\t%s\n", analizer->data);
    const Operator *word = optable_find(analizer->data);
    printf("\tChecked for operator\n");
    if(word == NULL) {
        // First word is label
        printf("\tFirst word is label\n");
        printf("\tlabelcheck:\n");
        if(analizer->data[0] == '_' || isalpha(analizer->data[0])) {
            // Label is valid
            printf("\t\tlabel[0] is valid\n");
            for(j = 1; j < analizer->i; j++) {
                if(!(analizer->data[j] == '_') && !isalnum(analizer->data[j])) {
                    // Label is invalid
                    //i - analizer->i + j
                    *errptr = s + i - analizer->i + j;
                    set_error_msg("Invalid label");
                    return 0;
                }
                printf("\t\tlabel[%d] is valid\n", j);
            }
            // Label is still valid -- insert it into stable
            printf("\tlabel is valid. Inserting into stable...\n");
            result = stable_insert(alias_table, analizer->data);
            printf("\tsuccess. result.new = %d\n", result.new);
            if(result.new == 0) {
                // Label was already declared
                // i - 1
                *errptr = s + i - 1;
                set_error_msg("Label %s was already declared\n", analizer->data);
                return 0;
            }
            label = estrdup(analizer->data);
            printf("\tCopied analizer->data to label\n");
        }
        else {
            // Label is invalid
            // i - buffer->topo + 1
            *errptr = s + i - analizer->i + 1;
            set_error_msg("Invalid label\n");
            return 0;
        }
        // Label registered. Scans next word
        for(; i < len && line[i] <= 32; i++); // Takes off all spaces
        if(i >= len || line[i] == '*') {
            // Line was only a label
            // i
            *errptr = s + i;
            set_error_msg("Expected operator\n");
            return 0;
        }
        buffer_reset(analizer);
        printf("\tFinished registering label, consumed spaces\n");
        for(; i < len && line[i] > 32 && line[i] != ';' && line[i] != '*'; i++) {
            buffer_push_back(analizer, line[i]);
        }
        printf("\tbuffer read:\n\t\t%s\n", analizer->data);
        word = optable_find(analizer->data);
        printf("\tWhich represents:\n\t\t%p\n", word);
        if(word == NULL) {
            // Word after label is not an Operator
            // i - analizer->i + 1
            *errptr = s + i - analizer->i + 1;
            set_error_msg("Expected operator\n");
            return 0;
        }
    }
    else {
        printf("\tFirst word isnt label\n");
        label = "n/a";
    }
    printf("\tStarted to check operands\n");
    Operator lineOp = (*word);
    Operand **opds;
    opds = (Operand**) emalloc(3 * sizeof(Operand*));
    printf("\tInitialized Operator and Operands\n");
    for(j = 0; j < 3; j++)
        opds[j] = NULL;
    printf("\topds[0:3] = 0\n");
    for(j = 0; j < 3; j++) {
        for(; i < len && line[i] <= 32; i++); // Takes off all spaces
        buffer_reset(analizer);
        OperandType type = lineOp.opd_types[j];
        printf("\toperand no. %d\n", j);
        if(type == OP_NONE) {
            printf("\t\tTYPE:OP_NONE\n");
            if(i >= len || line[i] == ';' || line[i] == '*')
                continue;
            // There is something after OP_NONE
            // i
            set_error_msg("Unexpected character");
            *errptr = s + i;
            return 0;
        }
        if(j > 0) {
            printf("\tcomma checking\n\tline[i] = %c\n", line[i]);
            if(line[i] != ',') {
                set_error_msg("Expected comma");
                *errptr = s + i;
                return 0;
            }
            //consumes until another word again
            for(i++; i < len && line[i] <= 32; i++);
        }
        //consumes next word
        for(; i < len && line[i] > 32 && line[i] != '*' && line[i] != ';' && line[i] != ','; i++)
            buffer_push_back(analizer, line[i]);
        printf("\tread operand: %s\n", analizer->data);

        int p = 0;
        if(type & BYTE1) {
            printf("\tMATCH: BYTE1\n");
            if(analizer->data[0] == '#') {
                printf("\t\tHexadecimal number!\n");

            }
            for (p = 0; p < analizer->i && isdigit(analizer->data[p]); p++);
            if (p == analizer->i) {
                int opNumber = atoi(analizer->data);
                if(opNumber >= 0 && opNumber <= 255) {
                    // Match
                    printf("\t\tIt's a BYTE1!\n");
                    opds[j] = operand_create_number(opNumber);
                    continue;
                }
            }
        }
        if(type & BYTE2) {
            printf("\tMATCH: BYTE2\n");
            for (p = 0; p < analizer->i && isdigit(analizer->data[p]); p++);
            if (p == analizer->i) {
                int opNumber = atoi(analizer->data);
                if(opNumber >= 0 && opNumber <= 65535) {
                    // Match
                    printf("\t\tIt's a BYTE2!\n");
                    opds[j] = operand_create_number(opNumber);
                    continue;
                }
            }
        }
        if(type & BYTE3) {
            printf("\tMATCH: BYTE3\n");
            for (p = 0; p < analizer->i && isdigit(analizer->data[p]); p++);
            if (p == analizer->i) {
                int opNumber = atoi(analizer->data);
                if(opNumber >= 0 && opNumber <= 167772150) {
                    // Match
                    printf("\t\tIt's a BYTE3!\n");
                    opds[j] = operand_create_number(opNumber);
                    continue;
                }
            }
        }
        if(type & TETRABYTE) {
            printf("\tMATCH: TETRABYTE\n");
            for (p = 0; p < analizer->i && isdigit(analizer->data[p]); p++);
            if (p == analizer->i) {
                uocta opNumber = atoll(analizer->data);
                if(opNumber >= 0 && opNumber <= 4294967295) {
                    // Match
                    printf("\t\tIt's a TETRABYTE!\n");
                    opds[j] = operand_create_number(opNumber);
                    continue;
                }
            }
        }
        if(type & LABEL) {
            printf("\tMATCH: LABEL\n");
            EntryData *ret;
            ret = stable_find(alias_table, analizer->data);
            if (ret != NULL) {
                printf("\t\tIt's a LABEL!\n");
                opds[j] = operand_create_label(analizer->data);
                continue;
            }
        }
        if(type & REGISTER) {
            printf("\tMATCH: REGISTER\n");
            if (analizer->data[0] == '$') {
                for (p = 1; p < analizer->i && isdigit(analizer->data[p]); p++);
                if (p == analizer->i) {
                    int opNumber = atoi(analizer->data + 1);
                    if(opNumber >= 0 && opNumber <= 255) {
                        // Match
                        printf("\t\tIt's a REGISTER!\n");
                        opds[j] = operand_create_register((unsigned char) opNumber);
                        printf("\t\topNumber = %d\n\t\topds[j] = %d\n", opNumber, opds[j]->value.reg);
                        continue;
                    }
                }
            }
        }
        if(type & NEG_NUMBER) {
            printf("\tMATCH: NEG_NUMBER\n");
            if (analizer->data[0] == '-') {
                for (p = 1; p < analizer->i && isdigit(analizer->data[p]); p++);
                if (p == analizer->i) {
                    octa opNumber = atoll(analizer->data);
                    if(opNumber <= 0 && opNumber >= -4294967295) {
                        // Match
                        printf("\t\tIt's a NEG_NUMBER!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
            }
        }
        if(type & STRING) {
            printf("\tMATCH: STRING\n");
            if (analizer->data[0] == '"' && analizer->data[analizer->i - 1] == '"') {
                // Match
                printf("\t\tIt's a STRING!\n");
                analizer->data[analizer->i - 1] = '\0';
                opds[j] = operand_create_string(&(analizer->data[1]));
                continue;
            }
        }
        // i - analizer->i + 1
        set_error_msg("Invalid operand");
        *errptr = s + i - analizer->i + 1;
        return 0;
    }
    if(strcmp(label, "n/a") != 0 && ispseudo(lineOp)) {
        printf("\tlabel is not null, command is PSEUDO\n");
        result.data->opd = opds[0];
        printf("\tinserted operand into stable\n");
    }
    else if(strcmp(label, "n/a") != 0) {
        printf("\tlabel is not null, command is ACTUAL\n");
        result.data->opd = operand_create_label(label);
        printf("\tinserted new label into stable\n");
    }
    finalInstr = instr_create(label, word, opds);
    (*instr) = finalInstr;
    return 1;
}
