#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "btree.h"

static bt_nodeptr _setup_node(char *key, void *val, unsigned long hash);

bt_nodeptr bt_init()
{
        bt_nodeptr n = NULL;
        n = bt_install(n, "", "");
        return n;
}

bt_nodeptr bt_install(bt_nodeptr n, char *key, void *val)
{
        unsigned long hash = bt_hash(key);
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

static bt_nodeptr _setup_node(char *key, void *val, unsigned long hash)
{
        bt_nodeptr n = calloc(1, sizeof(bt_node_t));
        if (n == NULL) return NULL;
        n->key = key;
        n->val = val;
        n->hash = hash;
        return n;
}

bt_nodeptr bt_find(bt_nodeptr n, char *key)
{
        unsigned long hash = bt_hash(key);
        while (n != NULL) {
                if (n->hash == hash) return n;

                if (hash <= n->hash) n = n->L;

                else if (hash > n->hash) n = n->R;
        }
        return NULL;
}

void bt_free(bt_nodeptr n)
{
        if (n == NULL) return;
        bt_free(n->L);
        bt_free(n->R);
        free(n);
}

unsigned long bt_hash(char *str)
{
        unsigned long hash = 5381;
        int c;

        while ((c = *str++))
                /* hash * 33 + c */
                hash = ((hash << 5) + hash) + c;

        return hash;
}
