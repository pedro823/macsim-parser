#include "stable.h"
#include <stdio.h>
#include "error.h"
#include <string.h>

// The symbol table.
typedef struct {
	char *key;
	EntryData data;
} Entry;

struct stable_s {
	Entry *Table;
	int topo;
	int tmax;
};

int buscaBin (SymbolTable table, const char *key) {
	int ini = 0, fim = table->topo, meio, comp;
	meio = (fim+ini)/2;
	while (ini <= fim) {
		comp = strcmp(key, table->Table[meio].key);
		if (comp == 0){
			return meio;
		}
		if (comp < 0){
			fim = meio-1;
			meio = (fim+ini)/2;
		}
		else {
			ini = meio+1;
			meio = (fim+ini)/2;
		}
	}
	meio = (fim+ini)/2;
	comp = strcmp(key, table->Table[meio].key);
	if (meio == 0 && comp < 0){
		return meio;
	}
	if (comp > 0){
		return meio+1;
	}
	if (meio < table->topo && comp < 0){
		return meio+1;
	}
	return meio;
}

void realloc_table (SymbolTable table) {
	Entry *New;
	int i, j;
	New = emalloc(sizeof(Entry) * table->tmax * 2);
	for (i = 0; i < 2*table->tmax; i++){
		New[i].key = emalloc(256*sizeof(char));
		for (j = 0; j < 255; j++)
			New[i].key[j] = 0;
	}
	for (i = 0; i < table->tmax; i++){
		New[i] = table->Table[i];
	}
	table->tmax *= 2;
	for (; i < table->tmax; i++){
		strcpy(New[i].key, "~~\0");
	}
	free(table->Table);
	table->Table = New;
}
/*
  Return a new symbol table.
*/
SymbolTable stable_create() {
	SymbolTable tabela = emalloc(sizeof(struct stable_s));
	tabela->topo = 0;
	tabela->tmax = 20;
	tabela->Table = emalloc(20 * sizeof(Entry));
	for (int i = 0; i < tabela->tmax; i++){
		tabela->Table[i].key = emalloc(256*sizeof(char));
		strcpy(tabela->Table[i].key, "~~\0");
	}
	return tabela;
}

/*
  Destroy a given symbol table.
*/
void stable_destroy(SymbolTable table) {
	for (int i = 0; i < table->tmax; i++)
		free(table->Table[i].key);
	free(table->Table);
	free(table);
}

/*
  Insert a new entry on the symbol table given its key.

  If there is already an entry with the given key, then a struct
  InsertionResult is returned with new == 0 and data pointing to the
  data associated with the entry. Otherwise, a struct is returned with
  new != 0 and data pointing to the data field of the new entry.

  If there is not enough space on the table, or if there is a memory
  allocation error, then crashes with an error message.
*/
InsertionResult stable_insert(SymbolTable table, const char *key) {
	InsertionResult *res = emalloc(sizeof(InsertionResult));
	int pos = buscaBin(table, key);
	char *mem;
	if (strcmp(table->Table[pos].key, key)){
		if (table->topo == (table->tmax)-1){
			realloc_table(table);
		}
		for (int i = table->topo; i > pos; i--){
			table->Table[i] = table->Table[i-1];
		}
		mem = malloc(256*sizeof(char));
		strcpy(mem, key);
		table->Table[pos].key = mem;
		res->new = 1;
		res->data = &(table->Table[pos].data);
		(table->topo)++;
	}
	else {
		res->data = &(table->Table[pos].data);
		res->new = 0;
	}
	return *res;
}

/*
  Find the data associated with a given key.

  Given a key, returns a pointer to the data associated with it, or a
  NULL pointer if the key is not found.
*/
EntryData *stable_find(SymbolTable table, const char *key) {
	int pos = buscaBin(table, key);
	if (pos == -1)
		return NULL;
	EntryData *res;
	res = &(table->Table[pos].data);
	return res;
}

/*
  Visit each entry on the table.

  The visit function is called on each entry, with pointers to its key
  and data. If the visit function returns zero, then the iteration
  stops.

  Returns zero if the iteration was stopped by the visit function,
  nonzero otherwise.
*/
int stable_visit(SymbolTable table,
                 int (*visit)(const char *key, EntryData *data)){
	int i = 0;
	while (i < table->topo && visit(table->Table[i].key, &(table->Table[i].data))){
		i++;
	}
	if (i == table->topo)
		return 1;
	return 0;
}
