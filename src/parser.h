#ifndef JADEC_PARSER_H
#define JADEC_PARSER_H

// node prop
typedef struct domnode_prop {
        char *prop;
        char *val;
} prop_t, *propp;

// node
typedef struct domnode {
        int depth;
        char *text;
        propp *proplist;
        struct domnode *parent;
} domnode_t, *domnodep;

void parse(FILE *input);

#endif
