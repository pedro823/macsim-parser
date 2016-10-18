/*
  optable.h

  Table of operations for the assembler.
*/

#ifndef __OPTABLE_H__
#define __OPTABLE_H__

#include "asmtypes.h"

/*
  Return pointer to operator given its name.

  If the operator is in the table, returns NULL.
*/
const Operator *optable_find(const char *name);

#endif
