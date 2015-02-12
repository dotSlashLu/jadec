#ifndef JADEC_LEXER_H
#define JADEC_LEXER_H

typedef struct tok {
        int type;
        void *data;
} tok_t, *tokp;

typedef struct domnode {
        int depth;
        char *text;
        struct domnode *parent;
} domnode_t, *domnodep

enum {
        tok_doctype = 256,
        tok_node,
        tok_text,
        tok_code,
        tok_level,
        tok_include,
        tok_comment,
        tok_mixin,
        tok_case
};

void tok_free(tokp tok);
#endif
