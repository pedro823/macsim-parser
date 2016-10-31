#include "error.h"
#include "stable.h"
#include "buffer.h"
#include "parser.h"
#include "optable.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

Operand checkOperand(char *line, SymbolTable alias_table, int i, Operator word, int operandNo) {
    int len = strlen(line), type;
    Operand ret;
    type = word->opd_types[operandNo];

}

int parse(const char *s, SymbolTable alias_table, Instruction **instr, const char **errptr) {
    char *line, *label;
    Buffer *analizer;
    Operator *word;
    InsertionResult result;
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
    word = optable_find(analizer->data);
    if(word == NULL) {
        // First word is label
        if(analizer[0] == '_' || (analizer[0] >= 'a' && analizer[0] <= 'z') ||
        (analizer[0] >= 'A' && analizer[0] <= 'Z')) {
            // Label is valid
            for(j = 1; j < analizer->i; j++) {
                if(!(analizer[j] == '_') && !isalnum(analizer[j])) {
                    // Label is invalid
                    //i - analizer->i + j
                    *errptr = s[i - analizer->i + j];
                    set_error_msg("Invalid label");
                    return 0;
                }
            }
            // Label is still valid -- insert it into stable
            result = stable_insert(alias_table, analizer->data);
            if(result.new == 0) {
                // Label was already declared
                // i - 1
                *errptr = s[i - 1];
                set_error_msg("Label %s was already declared\n", analizer->data);
                return 0;
            }
            label = strdup(analizer->data);
        }
        else {
            // Label is invalid
            // i - buffer->topo + 1
            *errptr = s[i - analizer->i + 1];
            set_error_msg("Invalid label\n");
            return 0;
        }
        // Label registered. Scans next word
        for(; i < len && line[i] <= 32; i++); // Takes off all spaces
        if(i >= len || line[i] == '*') {
            // Line was only a label
            // i
            *errptr = s[i];
            set_error_msg("Expected operator\n");
            return 0;
        }
        buffer_reset(analizer);
        for(; i < len && line[i] > 32 && line[i] != ';' && line[i] != '*'; i++) {
            buffer_push_back(analizer, line[i]);
        }
        word = optable_find(analizer->data);
        if(word == NULL) {
            // Word after label is not an Operator
            // i - analizer->i + 1
            *errptr = s[i - analizer->i + 1];
            set_error_msg("Expected operator\n");
            return 0;
        }
        (*instr)->label = label;
    }
    else {
        (*instr)->label = "n/a";
    }
    (*instr)->op = word;
    Operator lineOp = word;
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
            *errptr = s[i];
            return 0;
        }
        if(j > 0 && line[i] != ',') {
            set_error_msg("Expected comma");
            *errptr = s[i];
            return 0;
        }
        switch(type) {

            case BYTE1:
                //Consumes until , ; or space
                for(; i < len && line[i] > 32; i++)
                
            break;
        }
    }
}
