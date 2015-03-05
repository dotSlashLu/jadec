#ifndef JADEC_BTREE_H
#define JADEC_BTREE_H

typedef struct node {
        struct node *L;
        struct node *R;
        char *key;
        unsigned long hash;
        void *val;
} bt_node_t, *bt_nodeptr;

bt_nodeptr bt_init();
bt_nodeptr bt_install(bt_nodeptr root, char *key, void *val);
bt_nodeptr bt_find(bt_nodeptr from, char *key);
void    bt_free(bt_nodeptr root);
unsigned long bt_hash(char *str);

#endif
