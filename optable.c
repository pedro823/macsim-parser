/*
  optable.c
*/

#include "optable.h"
#include <stdlib.h>
#include <string.h>
#include "opcodes.h"

// Array with all operators.
static const int num_ops = 57;  // Number of operators.
  
static const Operator operators[] =
  {
    { .name = "ADD",  .opcode = ADD,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "ADDU", .opcode = ADDU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "AND",  .opcode = AND,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "CALL", .opcode = CALL, .opd_types = { LABEL, OP_NONE, OP_NONE } },
    { .name = "CMP",  .opcode = CMP,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "CMPU", .opcode = CMPU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "DIV",  .opcode = DIV,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "DIVU", .opcode = DIVU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "EXTERN", .opcode = EXTERN, .opd_types = { LABEL, OP_NONE, OP_NONE } },
    { .name = "GETA", .opcode = GETA, .opd_types = { REGISTER, ADDR2,    OP_NONE   } },
    { .name = "GO",   .opcode = GO,   .opd_types = { REGISTER, ADDR2,    OP_NONE   } },
    { .name = "INT",  .opcode = INT,  .opd_types = { BYTE3,    OP_NONE,  OP_NONE   } },
    { .name = "IS",   .opcode = IS,   .opd_types = { REGISTER | TETRABYTE | NEG_NUMBER, OP_NONE, OP_NONE } },
    { .name = "JMP",  .opcode = JMP,  .opd_types = { ADDR3,    OP_NONE,  OP_NONE   } },
    { .name = "JN",   .opcode = JN,   .opd_types = { REGISTER, ADDR2,    OP_NONE   } },
    { .name = "JNN",  .opcode = JNN,  .opd_types = { REGISTER, ADDR2,    OP_NONE   } },
    { .name = "JNP",  .opcode = JNP,  .opd_types = { REGISTER, ADDR2,    OP_NONE   } },
    { .name = "JNZ",  .opcode = JNZ,  .opd_types = { REGISTER, ADDR2,    OP_NONE   } },
    { .name = "JP",   .opcode = JP,   .opd_types = { REGISTER, ADDR2,    OP_NONE   } },
    { .name = "JZ",   .opcode = JZ,   .opd_types = { REGISTER, ADDR2,    OP_NONE   } },
    { .name = "LDB",  .opcode = LDB,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "LDBU", .opcode = LDBU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "LDO",  .opcode = LDO,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "LDOU", .opcode = LDOU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "LDT",  .opcode = LDT,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "LDTU", .opcode = LDTU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "LDW",  .opcode = LDW,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "LDWU", .opcode = LDWU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "MUL",  .opcode = MUL,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "MULU", .opcode = MULU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "NEG",  .opcode = NEG,  .opd_types = { REGISTER, BYTE1,    IMMEDIATE } },
    { .name = "NEGU", .opcode = NEGU, .opd_types = { REGISTER, BYTE1,    IMMEDIATE } },
    { .name = "NOP",  .opcode = NOP,  .opd_types = { OP_NONE,  OP_NONE,  OP_NONE   } },
    { .name = "NXOR", .opcode = NXOR, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "OR",   .opcode = OR,   .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "PUSH", .opcode = PUSH, .opd_types = { REGISTER, OP_NONE, OP_NONE } },
    { .name = "REST", .opcode = REST, .opd_types = { REGISTER, REGISTER, REGISTER } },
    { .name = "RET",  .opcode = RET,  .opd_types = { BYTE1, OP_NONE, OP_NONE } },
    { .name = "SAVE", .opcode = SAVE, .opd_types = { REGISTER, REGISTER, REGISTER } },
    { .name = "SETW", .opcode = SETW, .opd_types = { REGISTER, BYTE2,    OP_NONE   } },
    { .name = "SL",   .opcode = SL,   .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "SLU",  .opcode = SLU,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "SR",   .opcode = SR,   .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "SRU",  .opcode = SRU,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "STB",  .opcode = STB,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "STBU", .opcode = STBU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "STO",  .opcode = STO,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "STOU", .opcode = STOU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "STR",  .opcode = STR,  .opd_types = { STRING, OP_NONE, OP_NONE } },
    { .name = "STT",  .opcode = STT,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "STTU", .opcode = STTU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "STW",  .opcode = STW,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "STWU", .opcode = STWU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "SUB",  .opcode = SUB,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "SUBU", .opcode = SUBU, .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
    { .name = "TETRA",.opcode = TETRA,.opd_types = { TETRABYTE | NEG_NUMBER, OP_NONE, OP_NONE } },
    { .name = "XOR",  .opcode = XOR,  .opd_types = { REGISTER, REGISTER, IMMEDIATE } },
  };


static int compar(const void *key, const void *obj)
{
  return strcmp((const char *) key, ((Operator *) obj)->name);
}

const Operator *optable_find(const char *name)
{
  return (const Operator *) bsearch(name, operators, num_ops, sizeof(Operator),
                                    compar);
  
  return 0;
}
