#ifndef JADEC_LEXER_H
#define JADEC_LEXER_H

typedef struct tok {
        int type;
        void *data;
} tok_t, *tokp;

enum {
        tok_id          = 256,
        // \r?\n
        tok_lf,
        tok_start_block,
        tok_delim,
        tok_eof         = 260
};

tokp gettok();
char *get_quoted_literal(char quotemark);
char *get_literal_to_lf();
void tok_free(tokp tok);
void lexer_init(char *input, long fsize);
void lexer_free();
void jadec_pool_release(int);
#endif
