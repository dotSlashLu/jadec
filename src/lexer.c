#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "jadec.h"
#include "lexer.h"

static unsigned short u8seq_len;
static char *cur = NULL, *forward = NULL;
static char *_in;
// current utf8 seq
static char *u8seq;

// static long _loadbuf(char *curbuf);
// static char _advance();
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

/**
 *      Advance input and return the read utf8 seq length
 */
static unsigned short advance()
{
        static unsigned short mask[] = {192, 224, 240};
        u8seq = forward;
        char c = *++forward;
        unsigned short i, j;

        if (c == EOF)
            return 0;

        i = 0;
        if ((c & mask[0]) == mask[0]) i++;
        if ((c & mask[1]) == mask[1]) i++;
        if ((c & mask[2]) == mask[2]) i++;

        j = 0;
        while (j < i) {
            j++;
            forward++;
        }

        u8seq_len = i + 1;
        return i + 1;
}

void lexer_init(char *input)
{
        tok = calloc(1, sizeof(tok_t));
        _in = input;
        cur = forward = input;
        pool_init();
}

void lexer_free(FILE *input)
{
        free(jadec_pool);
}

tokp gettok()
{
        advance();
        printf("95 - u8seq_len: %d\n", u8seq_len);
        // id
        if (u8seq_len > 1 || isalnum(*forward)) {
                do {advance();}
                while ((u8seq_len == 1 && !isspace(*forward)) ||
                        u8seq_len > 1);
                int idlen = forward - cur;
                // printf("idlen: %d\n", idlen);
                char *idstr = pool_alloc(idlen + 1);
                strncpy(idstr, cur, idlen);
                *(idstr + idlen) = '\0';

                tok->type = tok_id;
                tok->data = idstr;

                cur = forward;
                // printf("cur and forward is @[%c]\n", *cur);
                forward--;
        }

        // [ \t]
        else if (isblank(*forward)) {
                int *i = pool_alloc(sizeof(int));
                *i = 0;
                do {
                        (*i)++;
                        advance();
                } while (isblank(*forward));
                tok->type = tok_delim;
                tok->data = i;

                cur = forward;
                forward--;
        }

        // Windows line feed
        else if (*forward == '\r' && *forward++ == '\n') {
                tok->type = tok_lf;
        }
        // Unix line feed
        else if (*forward == '\n') {
                tok->type = tok_lf;
                forward++;
        }

        // eof
        else if (*forward == EOF) {
                tok->type = tok_eof;
        }

        else {
                tok->type = tok_glyph;
                char *data = pool_alloc(u8seq_len + 1);
                if (u8seq_len < 2) {
                        *data = *forward++;
                        *(data + 1) = '\0';
                        printf("data: %s, u8seqlen: %d\n", data, u8seq_len);
                }
                else {
                        strncpy(data, u8seq, u8seq_len);
                        printf("glyph len %d\n", u8seq_len);
                        *(data + u8seq_len + 1) = '\0';
                        advance();
                }
        }

        printf("tok - type: %d data: %s\n", tok->type, (char *)tok->data);
        return tok;
}

/*
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
*/

void tok_free(tokp tok)
{
        if (!tok) return;
        // free(tok->data);
        free(tok);
}
