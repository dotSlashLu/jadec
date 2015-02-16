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

// check for buffer then peek for one char
static char _advance()
{
        // sentinel or eof
        if (*forward++ == -1) {
                // end of buf0
                if (forward == buf0 + JADEC_BUF_LEN) {
                        fputs("switch to buf1\n", stderr);
                        _loadbuf(buf1);
                        forward = buf1;
                }

                // end of buf1
                else if (forward == buf1 + JADEC_BUF_LEN) {
                        fputs("switch to buf0\n", stderr);
                        _loadbuf(buf0);
                        forward = buf0;
                }

                else {
                        // eof, cleanup
                        fputs("eof", stdout);
                }
        }

        return *forward;
}

void buf_init(FILE *input)
{
        _in = input;
        buf0 = malloc(sizeof(char) * JADEC_BUF_LEN);
        buf1 = malloc(sizeof(char) * JADEC_BUF_LEN);
        _loadbuf(buf0);
}

tokp gettok()
{
        tokp tok = calloc(1, sizeof(tok_t));

        if (!forward) forward = buf0;
        cur = forward;

        _advance();

        // id
        if (isalnum(*forward)) {
                while (isalnum(*forward++)){printf("%c\t", *forward);};
                *forward = '\0';
                char *idstr = malloc(sizeof(char) * (strlen(cur) + 1));
                strcpy(idstr, cur);
                printf("idstr: %s\n", idstr);

                // skip trailing spaces
                while (*forward == '\t' || *forward == ' ')
                        _advance();

                tok->type = tok_id;
                tok->data = idstr;
        }

        // level
        else if (isblank(*forward)) {
                int *i = malloc(sizeof(int));
                *i = 0;
                while (isblank(_advance())) (*i)++;
                tok->type = tok_level;
                tok->data = i;

        }

        // Windows line feed
        else if (*forward == '\r' && _advance() == '\n')
                tok->type = tok_lf;
        // Unix line feed
        else if (*forward == '\n')
                tok->type = tok_lf;

        else
                tok->type = *forward;

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
                printf("wtf, %ld\n", readlen);
                return -1;
        }
        *(curbuf + readlen) = -1;
        printf("*curbuf: %p\n", curbuf);
        return readlen;
}

void tok_free(tokp tok)
{
        if (!tok) return;
        free(tok->data);
        free(tok);
}
