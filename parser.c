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


int parse(const char *s, SymbolTable alias_table, Instruction **instr, const char **errptr) {
    char *line, *label;
    Buffer *analizer;
    Operator *word;
    InsertionResult result;
    EntryData *findResult;
    Instruction *finalInstr;
    int i, j, len;

    line = strdup(s);
    len = strlen(line);
    for(i = 0; i < len && line[i] <= 32; i++); // Takes off all preceding spaces
    analizer = buffer_create();
    if(line[i] == '*' || i >= len) {
        // Line is composed only by comment
        return 1;
    }
    for(; line[i] > 32 && line[i] != ';' && line[i] != '*'; i++) {
        buffer_push_back(analizer, line[i]);
    }
    word = optable_find(&(analizer->data));
    if(word == NULL) {
        // First word is label
        if(analizer[0] == '_' || (analizer[0] >= 'a' && analizer[0] <= 'z') ||
        (analizer[0] >= 'A' && analizer[0] <= 'Z')) {
            // Label is valid
            for(j = 1; j < analizer->i; j++) {
                if(!(analizer[j] == '_') && !isalnum(analizer[j])) {
                    // Label is invalid
                    //i - analizer->i + j
                    errptr = &(s + i - analizer->i + j);
                    set_error_msg("Invalid label");
                    return 0;
                }
            }
            // Label is still valid -- insert it into stable
            result = stable_insert(alias_table, analizer->data);
            if(result.new == 0) {
                // Label was already declared
                // i - 1
                errptr = &(s + i - 1);
                set_error_msg("Label %s was already declared\n", analizer->data);
                return 0;
            }
            label = strdup(analizer->data);
        }
        else {
            // Label is invalid
            // i - buffer->topo + 1
            errptr = &(s + i - analizer->i + 1);
            set_error_msg("Invalid label\n");
            return 0;
        }
        // Label registered. Scans next word
        for(; i < len && line[i] <= 32; i++); // Takes off all spaces
        if(i >= len || line[i] == '*') {
            // Line was only a label
            // i
            errptr = &(s + i);
            set_error_msg("Expected operator\n");
            return 0;
        }
        buffer_reset(analizer);
        for(; i < len && line[i] > 32 && line[i] != ';' && line[i] != '*'; i++) {
            buffer_push_back(analizer, line[i]);
        }
        word = optable_find(&(analizer->data));
        if(word == NULL) {
            // Word after label is not an Operator
            // i - analizer->i + 1
            errptr = &(s + i - analizer->i + 1);
            set_error_msg("Expected operator\n");
            return 0;
        }
    }
    else {
        label = "n/a";
    }
    (*instr)->op = word;
    Operator lineOp = (*word);
    Operand *opds[3];
    for(j = 0; j < 3; j++)
        opds[j] = 0;
    for(j = 0; j < 3; j++) {
        for(; i < len && line[i] <= 32; i++); // Takes off all spaces
        buffer_reset(analizer);
        OperandType type = lineOp.opd_types[j];
        if(type == OP_NONE) {
            if(i >= len || line[i] == ';' || line[i] == '*')
                return 1;
            // There is something after OP_NONE
            // i
            set_error_msg("Unexpected character");
            errptr = &(s + i);
            return 0;
        }
        if(j > 0) {
            if(line[i] != ',') {
                set_error_msg("Expected comma");
                errptr = &(s + i);
                return 0;
            }
            //consumes until another word again
            for(i++; i < len && line[i] <= 32; i++);
        }
        //consumes next word
        for(; i < len && line[i] > 32 && line[i] != '*' && line[i] != ';' && line[i] != ','; i++)
            buffer_push_back(analizer, line[i]);
        // Is it a valid label?

        int sentinel = 0, p = 0;
        if(type & BYTE1) {
            for (p = 0; p < analizer->top && isdigit(analizer->data[p]); p++);
            if (p == analizer->top) {
                int opNumber = atoi(analizer->data);
                if(opNumber >= 0 && opNumber <= 255) {
                    // Match
                    opds[j] = operand_create_number(opNumber);
                    continue;
                }
            }
        }
        if(type & BYTE2) {
            for (p = 0; p < analizer->top && isdigit(analizer->data[p]); p++);
            if (p == analizer->top) {
                int opNumber = atoi(analizer->data);
                if(opNumber >= 0 && opNumber <= 65535) {
                    // Match
                    opds[j] = operand_create_number(opNumber);
                    continue;
                }
            }
        }
        if(type & BYTE3) {
            for (p = 0; p < analizer->top && isdigit(analizer->data[p]); p++);
            if (p == analizer->top) {
                int opNumber = atoi(analizer->data);
                if(opNumber >= 0 && opNumber <= 167772150) {
                    // Match
                    opds[j] = operand_create_number(opNumber);
                    continue;
                }
            }
        }
        if(type & TETRABYTE) {
            for (p = 0; p < analizer->top && isdigit(analizer->data[p]); p++);
            if (p == analizer->top) {
                octa opNumber = atoll(analizer->data);
                if(opNumber >= 0 && opNumber <= 4294967295) {
                    // Match
                    opds[j] = operand_create_number(opNumber);
                    continue;
                }
            }
        }
        if(type & LABEL) {
            Entrydata *ret;
            ret = stable_find(alias_table, analizer->data);
            if (ret != NULL) {
                opds[j] = operand_create_label(analizer->data);
                continue;
            }
        }
        if(type & REGISTER) {
            if (analizer[0] == '$') {
                for (p = 1; p < analizer->top && isdigit(analizer->data[p]); p++);
                if (p == analizer->top) {
                    int opNumber = atoi(analizer->data);
                    if(opNumber >= 0 && opNumber <= 255) {
                        // Match
                        opds[j] = operand_create_register((unsigned char) opNumber);
                        continue;
                    }
                }
            }
        }
        if(type & NEG_NUMBER) {
            if (analizer[0] == '-') {
                for (p = 1; p < analizer->top && isdigit(analizer->data[p]); p++);
                if (p == analizer->top) {
                    octa opNumber = atoll(analizer->data);
                    if(opNumber >= 0 && opNumber <= 4294967295) {
                        // Match
                        (*instr)->opds[j] = operand_create_number(opNumber);
                        continue;
                    }
                }
            }
        }
        if(type & STRING) {
            if (analizer[0] == '"' && analizer->data[analizer->top - 1] == '"') {
                // Match
                analizer->data[analizer->top - 1] = '\0';
                opds[j] = operand_create_string(&(analizer->data[1]));
                continue;
            }
        }
        // i - analizer->top + 1
        set_error_msg("Invalid operand");
        errptr = &(s + i - analizer->top + 1);
        return 0;
    }
    finalInstr = instr_create(label, word, opds);
    (*instr) = finalInstr;
    return 1;
}
