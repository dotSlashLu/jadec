#ifndef JADEC_LEXER_H
#define JADEC_LEXER_H

typedef struct tok {
        int type;
        void *data;
} tok_t, *tokp;

enum {
        tok_id          = 256,
        tok_glyph,
        // \r?\n
        tok_lf,
        tok_delim       = 259,
        tok_level,
        tok_eof
};

tokp gettok();
void tok_free(tokp tok);
void lexer_init(char *);
void lexer_free();
void jadec_pool_release(int);
#endif
