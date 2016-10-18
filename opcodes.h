/*
  opcodes.h

  Opcode table for the MAC211 computer.
*/

#ifndef __OPCODES_H__
#define __OPCODES_H__

// Opcodes.
#define LDB     0x00
#define LDBI    0x01
#define LDW     0x02
#define LDWI    0x03
#define LDT     0x04
#define LDTI    0x05
#define LDO     0x06
#define LDOI    0x07

#define LDBU    0x08
#define LDBUI   0x09
#define LDWU    0x0a
#define LDWUI   0x0b
#define LDTU    0x0c
#define LDTUI   0x0d
#define LDOU    0x0e
#define LDOUI   0x0f

#define STB     0x10
#define STBI    0x11
#define STW     0x12
#define STWI    0x13
#define STT     0x14
#define STTI    0x15
#define STO     0x16
#define STOI    0x17

#define STBU    0x18
#define STBUI   0x19
#define STWU    0x1a
#define STWUI   0x1b
#define STTU    0x1c
#define STTUI   0x1d
#define STOU    0x1e
#define STOUI   0x1f

#define ADD     0x20
#define ADDI    0x21
#define SUB     0x22
#define SUBI    0x23
#define MUL     0x24
#define MULI    0x25
#define DIV     0x26
#define DIVI    0x27

#define CMP     0x28
#define CMPI    0x29
#define SL      0x2a
#define SLI     0x2b
#define SR      0x2c
#define SRI     0x2d
#define NEG     0x2e
#define NEGI    0x2f

#define ADDU    0x30
#define ADDUI   0x31
#define SUBU    0x32
#define SUBUI   0x33
#define MULU    0x34
#define MULUI   0x35
#define DIVU    0x36
#define DIVUI   0x37

#define CMPU    0x38
#define CMPUI   0x39
#define SLU     0x3a
#define SLUI    0x3b
#define SRU     0x3c
#define SRUI    0x3d
#define NEGU    0x3e
#define NEGUI   0x3f

#define AND     0x40
#define ANDI    0x41
#define OR      0x42
#define ORI     0x43
#define XOR     0x44
#define XORI    0x45
#define NXOR    0x46
#define NXORI   0x47

#define JMP     0x48
#define JMPB    0x49
#define JZ      0x4a
#define JZB     0x4b
#define JNZ     0x4c
#define JNZB    0x4d
#define JP      0x4e
#define JPB     0x4f

#define JN      0x50
#define JNB     0x51
#define JNN     0x52
#define JNNB    0x53
#define JNP     0x54
#define JNPB    0x55
#define GO      0x56
#define GOB     0x57

#define GETA    0x58
#define GETAB   0x59
#define SETW    0x5a
#define SAVE    0x5b
#define REST    0x5c

#define INT     0xfe
#define NOP     0xff

#endif
