/*
*       symtbl.h
*
*       Implementation of a symbol table.
*       In general, a symbol table is a (double) chain of maps.
*       It has the functionalities listed below:
*               - Create a new one onto the chain
*               - Put a new entry in the current table
*               - Get an entry for an identifier by
*               searching the chain of tables, with
*               the most-closely nested rule applied.
*/
#ifndef DS_SYMTABLE_H
#define DS_SYMTABLE_H

#include "btree.h"
#include <string.h>

#define DS_SYMTBL_MAX_CHILD 128

typedef struct env {
        nodeptr scope;
        struct env **children;
        int nchild;
        struct env *parent;
} symtbl_t, *symtbl;

symtbl st_createScope(symtbl parent);
symtbl st_put(symtbl to, char *key, void *data);
nodeptr st_search(symtbl from, char *key);
// free node and it's children
void st_free(symtbl root);
// free node and all it's ancestors and children
void st_free_deep(symtbl root);

#endif
