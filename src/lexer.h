#ifndef JADEC_LEXER_H
#define JADEC_LEXER_H

typedef struct tok {
        int type;
        void *data;
} tok_t, *tokp;

enum {
        tok_doctype = 256,
        tok_node,
        tok_code,
        tok_include,
        tok_comment,
        tok_mixin,
        tok_case
};

#endif
