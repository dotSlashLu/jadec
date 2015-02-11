#ifndef DS_BTREE_H
#define DS_BTREE_H

typedef struct node {
        struct node *L;
        struct node *R;
        char *key;
        unsigned long hash;
        void *val;
} node_t, *nodeptr;

nodeptr bt_init();
nodeptr bt_install(nodeptr root, char *key, void *val);
nodeptr bt_find(nodeptr from, char *key);
void    bt_free(nodeptr root);

#endif
