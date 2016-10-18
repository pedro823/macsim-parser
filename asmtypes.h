/*
  asmtypes.h

  Common types for the assembler.
*/

#ifndef __ASMTYPES_H__
#define __ASMTYPES_H__

#include "mactypes.h"

// Masks of operand types.
#define OP_NONE      0     // No operand.
#define BYTE1        0x01  // One-byte number.
#define BYTE2        0x02  // Two-byte number. 
#define BYTE3        0x04  // Three-byte number.
#define TETRABYTE    0x08  // A tetrabyte.
#define LABEL        0x10  // Label.
#define REGISTER     0x20  // Register.
#define NEG_NUMBER   0x40  // Number can be negative.
#define STRING       0x80  // A quote-enclosed string.

#define IMMEDIATE    (REGISTER | BYTE1)  // Immediate constant.
#define ADDR2        (LABEL | BYTE2 | NEG_NUMBER)
#define ADDR3        (LABEL | BYTE3 | NEG_NUMBER)
#define NUMBER_TYPE  (BYTE1 | BYTE2 | BYTE3 | TETRABYTE | NEG_NUMBER)

// Type of an operand.
typedef unsigned int OperandType;

// Opcodes for psudo-operators.
#define IS           -1
#define EXTERN       -2
#define TETRA        -3
#define STR          -4
#define CALL         -5
#define RET          -6
#define PUSH         -7

// Operator.
typedef struct {
  // Name of operator.
  char name[16];

  // Opcode (of nonimmediate/nonback version) or pseudo-opcode.
  int opcode;

  // Type of each operand.
  OperandType opd_types[3];
} Operator;


// A parsed operand.
typedef union {
  octa num;
  char *label;
  char *str;
  unsigned char reg;
} OperandValue;

typedef struct {
  // Valid types are REGISTER, LABEL, NUMBER_TYPE, and STRING.
  OperandType type;
  OperandValue value;
} Operand;


// A parsed instruction.
typedef struct instruction_s {
  // Instruction position.
  int pos;

  // Line number in input file.
  int lineno;
  
  // Label associated with the instruction.
  char *label;
  
  // Operator.
  const Operator *op;

  // Operands.
  Operand *opds[3];

  // Instructions are kept in a linked list.
  struct instruction_s *next;  
} Instruction;


/*
  Return new operand with the given value.
*/
Operand *operand_create_register(unsigned char reg);
Operand *operand_create_number(octa num);
Operand *operand_create_label(const char *label);
Operand *operand_create_string(const char *str);

/*
  Return copy of given operand.
*/
Operand *operand_dup(const Operand *opd);

/*
  Destroy operand.
*/
void operand_destroy(Operand *opd);

/*
  Create a new instruction. The given operands are not duplicated, the
  label is.
*/
Instruction *instr_create(const char *label, const Operator *op, Operand *opds[3]);

/*
  Destroy an instruction, destroying also the operands.
*/
void instr_destroy(Instruction *instr);

#endif
