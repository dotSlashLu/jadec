#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "jadec.h"
#include "lexer.h"

static char *buf0 = NULL, *buf1 = NULL;
static char *cur = NULL, *forward = NULL;
static FILE *_in;

static long _loadbuf(char *curbuf);
static char _advance();
static tokp tok;
// check for buffer then peek for one char
static char _advance()
{
        // sentinel or eof
        if (*forward++ == -1) {
                // end of buf0
                if (forward == buf0 + JADEC_BUF_LEN) {
                        _loadbuf(buf1);
                        forward = buf1;
                }

                // end of buf1
                else if (forward == buf1 + JADEC_BUF_LEN) {
                        _loadbuf(buf0);
                        forward = buf0;
                }

                else {
                        // eof, cleanup
                        *cur = *forward = EOF;
                }
        }

        return *forward;
}

void lexer_init(FILE *input)
{
        tok = calloc(1, sizeof(tok_t));
        _in = input;
        buf0 = malloc(sizeof(char) * JADEC_BUF_LEN);
        buf1 = malloc(sizeof(char) * JADEC_BUF_LEN);
        _loadbuf(buf0);
        cur = forward = buf0;
}

void lexer_free(FILE *input)
{
        free(buf0);
        free(buf1);
}

tokp gettok()
{
//        free(tok->data);
        // id
        if (isalnum(*forward)) {
                // while (isalnum(*forward++));
                while (isalnum(*forward)) {forward++;}
                int idlen = forward - cur;
                char *idstr = malloc(idlen + 1);
                strncpy(idstr, cur, idlen);
                *(idstr + idlen) = '\0';
                cur = forward;

                // skip trailing spaces
                while (isblank(*forward)) forward++;

                tok->type = tok_id;
                tok->data = idstr;
        }

        // level
        else if (isblank(*forward)) {
                fputs("lvl\n",stdout);
                int *i = malloc(sizeof(int));
                *i = 0;
                while (isblank(*forward)) {(*i)++;forward++;}

                tok->type = tok_level;
                tok->data = i;
        }

        // Windows line feed
        else if (*forward == '\r' && _advance() == '\n') {
                tok->type = tok_lf;
        }
        // Unix line feed
        else if (*forward == '\n') {
                tok->type = tok_lf;
                _advance();
        }

        // eof
        else if (*forward == EOF) {
                tok->type = tok_eof;
        }

        else {
                tok->type = *forward;
                _advance();
        }

        // printf("tok(%p) - type: %d data: %s\n", tok, tok->type, (char *)tok->data);
        return tok;
}

// two-buffer lookahead
static long _loadbuf(char *curbuf)
{
        size_t readlen = 0;
        // char *buf = (*buf0 == -1 || *buf0 == NULL) ?  buf0 : buf1;

        if (curbuf == NULL) curbuf = malloc(sizeof(char) * JADEC_BUF_LEN);
        readlen = fread(curbuf, 1, JADEC_BUF_LEN - 1, _in);
        // eof
        if (!readlen && feof(_in)) {
                return -1;
        }
        *(curbuf + readlen) = -1;
        return readlen;
}

void tok_free(tokp tok)
{
        if (!tok) return;
        free(tok->data);
        free(tok);
}
