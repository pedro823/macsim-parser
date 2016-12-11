#include "error.h"
#include "stable.h"
#include "buffer.h"
#include "parser.h"
#include "optable.h"
#include "mactypes.h"
#include "opcodes.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

int ispseudo(Operator op) {
    return op.opcode < 0;
}

int parse(const char *s, SymbolTable alias_table, Instruction **instr, const char **errptr) {
    char *line, *label;
    Buffer *analizer;
    InsertionResult result;
    Instruction *finalInstr;
    int i, j, len, isExternLable = 0;

    line = estrdup(s);
    len = strlen(line);
    for(i = 0; i <= len && line[i] <= 32; i++); // Takes off all preceding spaces
    analizer = buffer_create();
    //printf("parse:\n");
    //printf("\tInitialized all variables\n");
    if(i >= len || line[i] == '*') {
        // Line is composed only by comment
        return 1;
    }
    //printf("\tLine has stuff\n");
    for(; line[i] > 32 && line[i] != ';' && line[i] != '*'; i++) {
        buffer_push_back(analizer, line[i]);
    }
    analizer->data[analizer->i] = 0;
    //printf("\tBuffer Read:\n\t\t%s\n", analizer->data);
    const Operator *word = optable_find(analizer->data);
    //printf("\tChecked for operator\n");
    if(word == NULL) {
        // First word is label
        //printf("\tFirst word is label\n");
        //printf("\tlabelcheck:\n");
        if(analizer->data[0] == '_' || isalpha(analizer->data[0])) {
            // Label is valid
            //printf("\t\tlabel[0] is valid\n");
            for(j = 1; j < analizer->i; j++) {
                if(!(analizer->data[j] == '_') && !isalnum(analizer->data[j])) {
                    // Label is invalid
                    //i - analizer->i + j
                    *errptr = s + i - analizer->i + j;
                    set_error_msg("Invalid label");
                    return 0;
                }
                //printf("\t\tlabel[%d] is valid\n", j);
            }
            // Label is still valid -- insert it into stable
            //printf("\tlabel is valid. Inserting into stable...\n");
            result = stable_insert(alias_table, analizer->data);
            //printf("\tsuccess. result.new = %d\n", result.new);
            if(result.new == 0) {
                // Label was already declared
                // i - 1
                //printf("\tresult.data->opd = %p\n", result.data->opd);
                if(result.data->opd != NULL) {
                    *errptr = s + i - 1;
                    set_error_msg("Label %s was already declared\n", analizer->data);
                    return 0;
                }
                else {
                    //Label was declared in EXTERN
                    isExternLable = 1;
                    //printf("\tLable already exists. Adding an operand to it\n");
                    result.data->opd = operand_create_label(analizer->data);
                }
            }
            label = estrdup(analizer->data);
            //printf("\tCopied analizer->data to label\n");
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
        //printf("\tFinished registering label, consumed spaces\n");
        for(; i < len && line[i] > 32 && line[i] != ';' && line[i] != '*'; i++) {
            buffer_push_back(analizer, line[i]);
        }
        //printf("\tbuffer read:\n\t\t%s\n", analizer->data);
        word = optable_find(analizer->data);
        //printf("\tWhich represents:\n\t\t%p\n", word);
        if(word == NULL) {
            // Word after label is not an Operator
            // i - analizer->i + 1
            *errptr = s + i - analizer->i + 1;
            set_error_msg("Expected operator\n");
            return 0;
        }
        else if (word->opcode == EXTERN && strcmp(label, "n/a")) {
            set_error_msg("EXTERN operator can't have a label");
            *errptr = s + i - analizer->i + 1;
            return 0;
        }
    }
    else {
        //printf("\tFirst word isn't label\n");
        label = "n/a";
    }
    //printf("\tStarted to check operands\n");
    Operator lineOp = (*word);
    if (lineOp.opcode == IS) {
        if (strcmp(label, "n/a") == 0) {
            set_error_msg("IS operator requires a lable");
            *errptr = s;
            return 0;
        }
        if (isExternLable) {
            set_error_msg("Alias is not a valid lable for EXTERN operator");
            *errptr = s;
            return 0;
        }
    }

    Operand **opds;
    opds = (Operand**) emalloc(3 * sizeof(Operand*));
    //printf("\tInitialized Operator and Operands\n");
    for(j = 0; j < 3; j++)
        opds[j] = NULL;
    //printf("\topds[0:3] = 0\n");
    for(j = 0; j < 3; j++) {
        for(; i < len && line[i] <= 32; i++); // Takes off all spaces
        buffer_reset(analizer);
        OperandType type = lineOp.opd_types[j];
        //printf("\toperand no. %d\n", j);
        if(type == OP_NONE) {
            //printf("\t\tTYPE:OP_NONE\n");
            if(i >= len || line[i] == ';' || line[i] == '*')
                continue;
            // There is something after OP_NONE
            // i
            set_error_msg("Unexpected character");
            *errptr = s + i;
            return 0;
        }
        if(j > 0) {
            //printf("\tcomma checking\n\tline[i] = %c\n", line[i]);
            if(line[i] != ',') {
                set_error_msg("Expected comma");
                *errptr = s + i - 1;
                return 0;
            }
            //consumes until another word again
            for(i++; i < len && line[i] <= 32 && line[i] != '\n'; i++);
            //printf("i = %d, len = %d\n", i, len);
            //printf("line[i] = '%s'\n", line + i);
        }
        if (i >= len || line[i] == '*' || line[i] == ';' || line[i] == '\n') {
            set_error_msg("Expected operand");
            *errptr = s + i - 1;
            return 0;
        }
        //consumes next word
        for(; i < len && line[i] > 32 && line[i] != '*' && line[i] != ';' && line[i] != ','; i++)
            buffer_push_back(analizer, line[i]);
        //analizer->data[analizer->i] = 0;
        //printf("\tread operand: %s\n", analizer->data);
        int p = 0;
        if(type & BYTE1) {
            //printf("\tMATCH: BYTE1\n");
            if (analizer->data[0] == '#') {
                for (p = 1; p < analizer->i && isdigit(analizer->data[p]); p++);
                if(p == analizer->i) {
                    char *pt;
                    int opNumber = strtol(analizer->data + 1, &pt, 16);
                    if(opNumber >= 0 && opNumber <= 255) {
                        // Match
                        //printf("\t\tIt's an hexadecimal BYTE1!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
            }
            else {
                for (p = 0; p < analizer->i && isdigit(analizer->data[p]); p++);
                if (p == analizer->i) {
                    int opNumber = atoi(analizer->data);
                    if(opNumber >= 0 && opNumber <= 255) {
                        // Match
                        //printf("\t\tIt's a BYTE1!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
                else {
                    EntryData *ret;
                    //printf("\tTrying to find %s at the stable...\n", analizer->data);
                    ret = stable_find(alias_table, analizer->data);
                    if (strcmp(label, analizer->data) == 0) {
                        set_error_msg("Line can't have the same lable and operand");
                        *errptr = s + i;
                        return 0;
                    }
                    if (ret != NULL && ret->opd->type == NUMBER_TYPE) {
                       //printf("\tFound with success!\n");
                       if (ret->opd->value.num >= 0 && ret->opd->value.num <= 255) {
                         // Match
                         //printf("\t\tIt's a label to a BYTE1!\n");
                         opds[j] = operand_create_number(ret->opd->value.num);
                         continue;
                       }
                    }
                }
            }
        }
        if(type & BYTE2) {
            //printf("\tMATCH: BYTE2\n");
            if (analizer->data[0] == '#') {
                for (p = 1; p < analizer->i && isdigit(analizer->data[p]); p++);
                if(p == analizer->i) {
                    char *pt;
                    int opNumber = strtol(analizer->data + 1, &pt, 16);
                    if(opNumber >= 0 && opNumber <= 65535) {
                        // Match
                        //printf("\t\tIt's an hexadecimal BYTE2!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
            }
            else {
                for (p = 0; p < analizer->i && isdigit(analizer->data[p]); p++);
                if (p == analizer->i) {
                    int opNumber = atoi(analizer->data);
                    if(opNumber >= 0 && opNumber <= 65535) {
                        // Match
                        //printf("\t\tIt's a BYTE2!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
                else {
                    EntryData *ret;
                    ret = stable_find(alias_table, analizer->data);
                    if (strcmp(label, analizer->data) == 0) {
                        set_error_msg("Line can't have the same lable and operand");
                        *errptr = s + i;
                        return 0;
                    }
                    if (ret != NULL && ret->opd->type == NUMBER_TYPE) {
                       if (ret->opd->value.num >= 0 && ret->opd->value.num <= 65535) {
                         // Match
                         //printf("\t\tIt's a label to a BYTE2!\n");
                         opds[j] = operand_create_number(ret->opd->value.num);
                         continue;
                       }
                    }
                }
            }
        }
        if(type & BYTE3) {
            //printf("\tMATCH: BYTE3\n");
            if (analizer->data[0] == '#') {
                for (p = 1; p < analizer->i && isdigit(analizer->data[p]); p++);
                if(p == analizer->i) {
                    char *pt;
                    int opNumber = strtol(analizer->data + 1, &pt, 16);
                    if(opNumber >= 0 && opNumber <= 167772150) {
                        // Match
                        //printf("\t\tIt's an hexadecimal BYTE3!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
            }
            else {
                for (p = 0; p < analizer->i && isdigit(analizer->data[p]); p++);
                if (p == analizer->i) {
                    int opNumber = atoi(analizer->data);
                    if(opNumber >= 0 && opNumber <= 167772150) {
                        // Match
                        //printf("\t\tIt's a BYTE3!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
                else {
                    EntryData *ret;
                    ret = stable_find(alias_table, analizer->data);
                    if (strcmp(label, analizer->data) == 0) {
                        set_error_msg("Line can't have the same lable and operand");
                        *errptr = s + i;
                        return 0;
                    }
                    if (ret != NULL && ret->opd->type == NUMBER_TYPE) {
                       if (ret->opd->value.num >= 0 && ret->opd->value.num <= 167772150) {
                         // Match
                         //printf("\t\tIt's a label to a BYTE3!\n");
                         opds[j] = operand_create_number(ret->opd->value.num);
                         continue;
                       }
                    }
                }
            }
        }
        if(type & TETRABYTE) {
            //printf("\tMATCH: TETRABYTE\n");
            if (analizer->data[0] == '#') {
                for (p = 1; p < analizer->i && isdigit(analizer->data[p]); p++);
                if(p == analizer->i) {
                    char *pt;
                    uocta opNumber = strtol(analizer->data + 1, &pt, 16);
                    if(opNumber >= 0 && opNumber <= 4294967295) {
                        // Match
                        //printf("\t\tIt's a hexadecimal TETRABYTE!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
            }
            else {
                for (p = 0; p < analizer->i && isdigit(analizer->data[p]); p++);
                if (p == analizer->i) {
                    uocta opNumber = atoll(analizer->data);
                    if(opNumber >= 0 && opNumber <= 4294967295) {
                        // Match
                        //printf("\t\tIt's a TETRABYTE!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
                else {
                    EntryData *ret;
                    ret = stable_find(alias_table, analizer->data);
                    if (strcmp(label, analizer->data) == 0) {
                        set_error_msg("Line can't have the same lable and operand");
                        *errptr = s + i;
                        return 0;
                    }
                    if (ret != NULL && ret->opd->type == NUMBER_TYPE) {
                       if (ret->opd->value.num >= 0 && ret->opd->value.num <= 4294967295) {
                         // Match
                         //printf("\t\tIt's a label to a TETRABYTE!\n");
                         opds[j] = operand_create_number(ret->opd->value.num);
                         continue;
                       }
                    }
                }
            }
        }
        if(type & REGISTER) {
            //printf("\tMATCH: REGISTER\n");
            if (analizer->data[0] == '$') {
                for (p = 1; p < analizer->i && isdigit(analizer->data[p]); p++);
                if (p == analizer->i) {
                    int opNumber = atoi(analizer->data + 1);
                    if(opNumber >= 0 && opNumber <= 255) {
                        // Match
                        //printf("\t\tIt's a REGISTER!\n");
                        opds[j] = operand_create_register((unsigned char) opNumber);
                        //printf("\t\topNumber = %d\n\t\topds[j] = %d\n", opNumber, opds[j]->value.reg);
                        continue;
                    }
                }
            }
            else {
                EntryData *ret;
                ret = stable_find(alias_table, analizer->data);
                if (strcmp(label, analizer->data) == 0) {
                    set_error_msg("Line can't have the same lable and operand");
                    *errptr = s + i;
                    return 0;
                }
                if (ret != NULL && ret->opd->type == REGISTER) {
                   // Match
                   //printf("\t\tIt's a label to a REGISTER!\n");
                   opds[j] = operand_create_register(ret->opd->value.reg);
                   continue;
                }
            }
        }
        if(type & NEG_NUMBER) {
            //printf("\tMATCH: NEG_NUMBER\n");
            if (analizer->data[0] == '#' && analizer->data[1] == '-') {
                for (p = 2; p < analizer->i && isdigit(analizer->data[p]); p++);
                if(p == analizer->i) {
                    char *pt;
                    octa opNumber = strtol(analizer->data + 1, &pt, 16);
                    if(opNumber <= 0 && opNumber >= -4294967295) {
                        // Match
                        //printf("\t\tIt's an hexadecimal NEG_NUMBER!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
            }
            else if (analizer->data[0] == '-') {
                for (p = 1; p < analizer->i && isdigit(analizer->data[p]); p++);
                if (p == analizer->i) {
                    octa opNumber = atoll(analizer->data);
                    if(opNumber <= 0 && opNumber >= -4294967295) {
                        // Match
                        //printf("\t\tIt's a NEG_NUMBER!\n");
                        opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
                else {
                    EntryData *ret;
                    ret = stable_find(alias_table, analizer->data);
                    if (strcmp(label, analizer->data) == 0) {
                        set_error_msg("Line can't have the same lable and operand");
                        *errptr = s + i;
                        return 0;
                    }
                    if (ret != NULL && ret->opd->type == NUMBER_TYPE) {
                       if (ret->opd->value.num <= 0 && ret->opd->value.num >= -4294967295) {
                           // Match
                           //printf("\t\tIt's a label to a NEG_NUMBER!\n");
                           opds[j] = operand_create_number(ret->opd->value.num);
                           continue;
                       }
                    }
                }
            }
        }
        if(type & STRING) {
            //printf("\tMATCH: STRING\n");
            if (analizer->data[0] == '"' && analizer->data[analizer->i - 1] == '"') {
                // Match
                //printf("\t\tIt's a STRING!\n");
                analizer->data[analizer->i - 1] = '\0';
                opds[j] = operand_create_string(&(analizer->data[1]));
                continue;
            }
            else {
                EntryData *ret;
                ret = stable_find(alias_table, analizer->data);
                if (strcmp(label, analizer->data) == 0) {
                    set_error_msg("Line can't have the same lable and operand");
                    *errptr = s + i;
                    return 0;
                }
                if (ret != NULL && ret->opd->type == STRING) {
                   // Match
                   //printf("\t\tIt's a label to a STRING!\n");
                   opds[j] = operand_create_string(ret->opd->value.str);
                   continue;
                }
            }
        }
        if(type & LABEL) {
            //printf("\tMATCH: LABEL\n");
            EntryData *ret;
            ret = stable_find(alias_table, analizer->data);
            if (strcmp(label, analizer->data) == 0) {
                set_error_msg("Line can't have the same lable and operand");
                *errptr = s + i;
                return 0;
            }
            if (ret != NULL || (lineOp.opcode >= JMP && lineOp.opcode <= GETAB) || lineOp.opcode == EXTERN || lineOp.opcode == CALL) {
                //printf("\t\tIt's a LABEL!\n");
                opds[j] = operand_create_label(analizer->data);
                continue;
            }
        }
        // i - analizer->i + 1
        set_error_msg("Invalid operand");
        //printf("\tInvalid operand\n");
        *errptr = s + (i - analizer->i + 1);
        //printf("analizer->i = %d\ni = %d\nerrptr = %p\ns = %p\n", analizer->i, i, *errptr, s);
        return 0;
    }
    if(strcmp(label, "n/a") != 0 && ispseudo(lineOp)) {
        //printf("\tlabel is not null, command is PSEUDO\n");
        result.data->opd = opds[0];
        //printf("\tinserted operand into stable\n");
    }
    else if(strcmp(label, "n/a") != 0) {
        //printf("\tlabel is not null, command is ACTUAL\n");
        result.data->opd = operand_create_label(label);
        //printf("\tinserted new label into stable\n");
    }
    for(; i < len && line[i] <= 32; i++); //Consumes more spaces
    if(i < len && line[i] != '*' && line[i] != ';') {
        set_error_msg("Expected end of line");
        *errptr = s + i - 1;
        return 0;
    }
    finalInstr = instr_create(label, word, opds);
    *instr = finalInstr;
    return 1;
}
