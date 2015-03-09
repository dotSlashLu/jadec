#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "jadec.h"
#include "lexer.h"

static long _fsize;
static short u8seq_len;
static char *cur = NULL, *forward = NULL;
static char *_in;
// current utf8 seq
static char *u8seq;

static tokp tok;
static inline char getchr();
static inline unsigned short advance();


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

// advance input and return the read utf8 seq length
static inline unsigned short advance()
{
        u8seq = forward;
        char c = getchr();
        unsigned short i, j;

        if (_fsize < 1) {
                u8seq_len = -1;
                return 0;
        }

        i = 0;
        // utf8 first byte patter mask: 192 224 240
        if ((c & 192) == 192) i++;
        if ((c & 224) == 224) i++;
        if ((c & 240) == 240) i++;

        j = 0;
        while (j < i) {
            j++;
            getchr();
        }

        u8seq_len = i + 1;
        return i + 1;
}

// test EOF and get next char
static inline char getchr()
{
        if (_fsize < 1) return EOF;
        _fsize--;
        // printf("[%d]\tremaining fsize: %d\n", __LINE__, _fsize);
        return *++forward;
}

/*
static inline void rewindchr(int i)
{
        _fsize += i;
        forward -= i;
}
*/

void lexer_init(char *input, long fsize)
{
        tok = calloc(1, sizeof(tok_t));
        _in = input;
        _fsize = fsize;
        cur = forward = input;
        pool_init();
}

void lexer_free(FILE *input)
{
        free(jadec_pool);
}

char *get_literal_to_lf()
{
        while ((*forward != '\r' && getchr() != '\n') || *forward != '\n') {
                // multibyte
                if (u8seq_len > 1) {
                        advance();
                        continue;
                }

                // lf, break
                if (*forward == '\r') {
                        advance();
                        if (*forward == '\n') {
                                if (forward > cur + 1 && *(forward - 2) == '\\')
                                        continue;
                                break;
                        }
                }
                else if (*forward == '\n' || u8seq_len < 0) {
                        if (forward > cur && *(forward - 1) == '\\')
                                continue;
                        break;
                }
        }

        int len = forward - cur;
        char *ret = malloc(len + 1);
        strncpy(ret, cur, len);
        *(ret + len) = '\0';
        cur = forward;
        return ret;
}

char *get_quoted_literal(char quote)
{
        int buflen = 512, i = 0;
        char *ret = malloc(buflen);
#define cpy(len) { \
        if (i + (len) > buflen) { \
                ret = realloc(ret, buflen + buflen / 2); \
                buflen += buflen / 2; \
        } \
        while ((len)--) { \
                *(ret + i) = *(cur + i); \
                i++; \
        } \
}

        while (1) {
                if (u8seq_len > 1) {
                        cpy(u8seq_len);
                        advance();
                        continue;
                }

                if (*forward == quote)
                        break;
                if (*forward == '\\') {
                        int escaped_len = advance();
                        if (u8seq_len == 1 && *forward == quote) {
                                cur++;
                                cpy(u8seq_len);
                        }
                        u8seq_len += escaped_len;
                        cpy(u8seq_len);
                }
                else cpy(u8seq_len);
                advance();
        }

        *(ret + i + 1) = '\0';
        // eat ending "
        getchr();
        cur = forward;
        return ret;
#undef cpy
}

tokp gettok()
{
        // printf("\n\n\n---gettok---\n");
        // id
        if (u8seq_len > 1 || isalnum(*forward)) {
                // printf("[%d]\tid\n", __LINE__);
                do advance();
                while ((u8seq_len == 1 && isalnum(*forward)) ||
                        u8seq_len > 1);
                int idlen = forward - cur;
                char *idstr = pool_alloc(idlen + 1);
                strncpy(idstr, cur, idlen);
                *(idstr + idlen) = '\0';

                tok->type = tok_id;
                tok->data = idstr;

                cur = forward;
                // rewindchr(1);
        }

        // [ \t]
        else if (isblank(*forward)) {
                // printf("[%d]\tblank\n", __LINE__);
                int *i = pool_alloc(sizeof(int));
                *i = 0;
                do {
                        (*i)++;
                        // eat current space
                        cur = forward + 1;
                        advance();
                } while (isblank(*forward));
                tok->type = tok_delim;
                tok->data = i;
        }

        // line feed
        else if ((*forward == '\r' && getchr() == '\n') ||
                *forward == '\n') {
                // printf("[%d]\tlf\n", __LINE__);
                tok->type = tok_lf;
                getchr();
                cur = forward;
        }

        // eof
        else if (_fsize < 1 ||
                u8seq_len == -1 ||
                *forward == EOF) {
                tok->type = tok_eof;
        }

        else {
                // printf("[%d]\tglyph, u8seq_len: %d\n", __LINE__, u8seq_len);
                tok->type = *forward;
                // char *data = pool_alloc(u8seq_len + 1);
                // if (u8seq_len < 2) {
                //         *data = *forward;
                //         getchr();
                //         *(data + 1) = '\0';
                //         // printf("[%d]\tdata: %s, u8seqlen: %d\n", __LINE__, data, u8seq_len);
                // }
                // else {
                //         strncpy(data, u8seq, u8seq_len);
                //         // printf("glyph len %d\n", u8seq_len);
                //         *(data + u8seq_len + 1) = '\0';
                //         advance();
                // }
                advance();
                cur = forward;
        }

        // printf("[%d]\ttok - type: %d data: [%s]\n", __LINE__, tok->type, (char *)tok->data);
        return tok;
}

void tok_free(tokp tok)
{
        if (!tok) return;
        free(tok);
}
