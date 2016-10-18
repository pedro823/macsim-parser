/*
  asmtypes.c
*/

#include "asmtypes.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "error.h"


Operand *operand_create_register(unsigned char reg)
{
  Operand *ret = emalloc(sizeof(Operand));

  ret->type = REGISTER;
  ret->value.reg = reg;

  return ret;
}


Operand *operand_create_number(octa num)
{
  Operand *ret = emalloc(sizeof(Operand));

  ret->type = NUMBER_TYPE;
  ret->value.num = num;

  return ret;
}

Operand *operand_create_label(const char *label)
{
  Operand *ret = emalloc(sizeof(Operand));

  ret->type = LABEL;
  ret->value.label = estrdup(label);

  return ret;
}

Operand *operand_create_string(const char *str)
{
  Operand *ret = emalloc(sizeof(Operand));

  ret->type = STRING;
  ret->value.str = estrdup(str);

  return ret;
}


Operand *operand_dup(const Operand *opd)
{
  Operand *ret = emalloc(sizeof(Operand));

  ret->type = opd->type;

  if (ret->type == LABEL || ret->type == STRING)
    ret->value.label = estrdup(opd->value.label);
  else
    ret->value = opd->value;

  return ret;
}


void operand_destroy(Operand *opd)
{
  if (opd->type == LABEL || opd->type == STRING)
    free(opd->value.label);

  free(opd);
}


Instruction *instr_create(const char *label, const Operator *op, Operand *opds[3])
{
  Instruction *ret = emalloc(sizeof(Instruction));

  ret->pos = ret->lineno = 0;
  
  if (label)
    ret->label = estrdup(label);
  else
    ret->label = 0;

  ret->op = op;
  
  for (int i = 0; i < 3; i++)
    ret->opds[i] = opds[i];
  
  ret->next = 0;

  return ret;
}

/*
  Destroy an instruction, destroying also the operands.
*/
void instr_destroy(Instruction *instr)
{
  free(instr->label);
  
  for (int i = 0; i < 3; i++)
    if (instr->opds[i]) operand_destroy(instr->opds[i]);
  
  free(instr);
}
