#ifndef JADEC_LEXER_H
#define JADEC_LEXER_H

typedef struct tok {
        int type;
        void *data;
} tok_t, *tokp;

enum {
        tok_id = 256,
        // \r?\n
        tok_lf,
        tok_delim,
        tok_level,
        tok_eof
};

tokp gettok();
void tok_free(tokp tok);
void lexer_init(FILE *);
void lexer_free();
void jadec_pool_release(int);
#endif
