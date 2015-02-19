#ifndef JADEC_LEXER_H
#define JADEC_LEXER_H

typedef struct tok {
        int type;
        void *data;
} tok_t, *tokp;


typedef struct domnode_prop {
        char *prop;
        char *val;
} prop_t, *propp;

typedef struct domnode {
        int depth;
        char *text;
        propp *proplist;
        struct domnode *parent;
} domnode_t, *domnodep;

enum {
        tok_id = 256,
        // \r?\n
        tok_lf,
        tok_level,
        tok_eof
};

tokp gettok();
void tok_free(tokp tok);
void lexer_init(FILE *);
void lexer_free();
#endif
