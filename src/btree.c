#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "btree.h"

static unsigned long _hash(char *str);
static nodeptr _setup_node(char *key, void *val, unsigned long hash);

nodeptr bt_init()
{
        nodeptr n = NULL;
        n = bt_install(n, "", "");
        return n;
}

nodeptr bt_install(nodeptr n, char *key, void *val)
{
        unsigned long hash = _hash(key);
        while (n != NULL) {
                if (n->hash == hash && n->key == key) {
                        n->val = val;
                        return n;
                }

                if (hash <= n->hash) {
                        if (n->L == NULL)
                                return n->L = _setup_node(key, val, hash);
                        n = n->L;
                }

                else if (hash > n->hash) {
                        if (n->R == NULL)
                                return n->R = _setup_node(key, val, hash);
                        n = n->R;
                }
        }

        // root
        if (n == NULL) {
                n = _setup_node(key, val, hash);
                return n;
        }

        return n;
}

static nodeptr _setup_node(char *key, void *val, unsigned long hash)
{
        nodeptr n = calloc(1, sizeof(node_t));
        if (n == NULL) return NULL;
        n->key = key;
        n->val = val;
        n->hash = hash;
        return n;
}

nodeptr bt_find(nodeptr n, char *key)
{
        unsigned long hash = _hash(key);
        while (n != NULL) {
                if (n->hash == hash) return n;

                if (hash <= n->hash) n = n->L;

                else if (hash > n->hash) n = n->R;
        }
        return NULL;
}

void bt_free(nodeptr n)
{
        if (n == NULL) return;
        bt_free(n->L);
        bt_free(n->R);
        free(n);
}

static unsigned long _hash(char *str)
{
        unsigned long hash = 5381;
        int c;

        while ((c = *str++))
                /* hash * 33 + c */
                hash = ((hash << 5) + hash) + c;

        return hash;
}
