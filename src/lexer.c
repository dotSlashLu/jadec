#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "jadec.h"
#include "lexer.h"

static char *buf0 = NULL, *buf1 = NULL;
static char *cur = NULL, *forward = NULL;
static char *_in;

static long _loadbuf(char *curbuf);
static char _advance();
static tokp tok;


void *jadec_pool;
static size_t pool_size = JADEC_POOL_SIZE;
static int pool_cur = 0;
static void *pool_init()
{
        jadec_pool = malloc(JADEC_POOL_SIZE);
        if (!jadec_pool) {
                perror("malloc");
                exit(1);
        }
        return jadec_pool;
}

static void *pool_alloc(size_t s)
{
        if (pool_cur + s >= pool_size) {
                jadec_pool = realloc(jadec_pool, pool_size + JADEC_POOL_SIZE);
                pool_size += JADEC_POOL_SIZE;
        }

        void *ret = jadec_pool + pool_cur;
        pool_cur += s;
        return ret;
}

// reset cursor for reuse of memory
void jadec_pool_release(int i)
{
        pool_cur = i;
}

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

void lexer_init(char *input)
{
        tok = calloc(1, sizeof(tok_t));
        _in = input;
        buf0 = malloc(JADEC_BUF_LEN);
        buf1 = malloc(JADEC_BUF_LEN);
        _loadbuf(buf0);
        cur = forward = buf0;
        pool_init();
}

void lexer_free(FILE *input)
{
        free(buf0);
        free(buf1);
        free(jadec_pool);
}

tokp gettok()
{
        // id
        if (isalnum(*forward)) {
                // while (isalnum(*forward++));
                while (!isspace(*forward)) {forward++;}
                int idlen = forward - cur;
                char *idstr = pool_alloc(idlen + 1);
                strncpy(idstr, cur, idlen);
                *(idstr + idlen) = '\0';

                // skip trailing spaces
                // while (isblank(*forward)) forward++;

                tok->type = tok_id;
                tok->data = idstr;

                cur = forward;
        }

        // [ \t]
        else if (isblank(*forward)) {
                int *i = pool_alloc(sizeof(int));
                *i = 0;
                while (isblank(*forward)) {
                        (*i)++;
                        forward++;
                }

                tok->type = tok_delim;
                tok->data = i;

                cur = forward;
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

        if (curbuf == NULL) curbuf = malloc(JADEC_BUF_LEN);
        readlen = fread(curbuf, 1, JADEC_BUF_LEN - 1, _in);
        // eof
        if (!readlen && feof(_in))
                return -1;
        *(curbuf + readlen) = -1;
        return readlen;
}

void tok_free(tokp tok)
{
        if (!tok) return;
        // free(tok->data);
        free(tok);
}
