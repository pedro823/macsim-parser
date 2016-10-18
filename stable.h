/*
  stable.h

  A symbol table associating generic data to strings.
*/

#ifndef __STABLE_H__
#define __STABLE_H__

#include "asmtypes.h"

// The symbol table.
typedef struct stable_s *SymbolTable;

// Data stored.
typedef union {
  int i;
  octa s;
  uocta u;
  char *str;
  void *p;
  Operand *opd;
} EntryData;

// Return struct for stable_insert.
typedef struct {
  int new;  // Was a new entry created?
  EntryData *data;  // Data associated with entry.
} InsertionResult;

/*
  Return a new symbol table.
*/
SymbolTable stable_create();

/*
  Destroy a given symbol table.
*/
void stable_destroy(SymbolTable table);

/*
  Insert a new entry on the symbol table given its key.

  If there is already an entry with the given key, then a struct
  InsertionResult is returned with new == 0 and data pointing to the
  data associated with the entry. Otherwise, a struct is returned with
  new != 0 and data pointing to the data field of the new entry.

  If there is not enough space on the table, or if there is a memory
  allocation error, then crashes with an error message.
*/
InsertionResult stable_insert(SymbolTable table, const char *key);

/*
  Find the data associated with a given key.

  Given a key, returns a pointer to the data associated with it, or a
  NULL pointer if the key is not found.
*/
EntryData *stable_find(SymbolTable table, const char *key);

/*
  Visit each entry on the table.

  The visit function is called on each entry, with the key and the
  data. If the visit function returns zero, then the iteration stops.

  Returns zero if the iteration was stopped by the visit function,
  nonzero otherwise.
*/
int stable_visit(SymbolTable table,
                 int (*visit)(const char *key, EntryData *data));


#endif
