#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "btree.h"
#include "symtbl.h"

static void _st_free_root(symtbl root);

symtbl st_createScope(symtbl parent)
{
        nodeptr scope = bt_init();
        symtbl env = calloc(1, sizeof(symtbl_t));
        env->scope = scope;
        env->parent = parent;
        env->nchild = 0;
        env->children = calloc(1, sizeof(symtbl) * DS_SYMTBL_MAX_CHILD);
        if (parent != NULL) {
                *(parent->children + parent->nchild) = env;
                parent->nchild += 1;
                // max child limit exceeded
                if (parent->nchild > DS_SYMTBL_MAX_CHILD) return NULL;
        }
        return env;
}

symtbl st_put(symtbl to, char *key, void *data)
{
        nodeptr scope = to->scope;
        bt_install(scope, key, data);
        return to;
}

nodeptr st_search(symtbl from, char *key)
{
        nodeptr scope;
        nodeptr result;
        while ((scope = from->scope) != NULL) {
                result = bt_find(scope, key);
                if (result != NULL) return result;
                // search upper scope
                from = from->parent;
                if (from == NULL) return NULL;
        }
        return NULL;
}

void st_free(symtbl node)
{
        _st_free_root(node);
}

void st_free_deep(symtbl node)
{
        // trace up to root
        while (node->parent != NULL) node = node->parent;
        _st_free_root(node);
}

void _st_free_root(symtbl root)
{
        int n = 0;
        if (root->scope != NULL)
                bt_free(root->scope);
        while (n < root->nchild)
                _st_free_root(*(root->children + n++));
        free(root->children);
        free(root);
}
