#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "jadec.h"
#include "lexer.h"

static char *buf0 = NULL, *buf1 = NULL;
static char *cur = NULL, *forward = NULL;
static FILE *in;

static long _loadbuf(FILE *in, char *curbuf);
static char _advance();

// check for buffer then peek for one char
static char _advance()
{
        // sentinel or eof
        if (*forward++ == -1) {
                // end of buf0
                if (forward == buf0 + JADEC_BUF_LEN) {
                        _loadbuf(in, buf1);
                        forward = buf1;
                }

                // end of buf1
                else if (forward == buf1 + JADEC_BUF_LEN) {
                        _loadbuf(in, buf0);
                        forward = buf0;
                }

                else {
                        // eof, cleanup
                }
        }

        return *forward;
}

tokp gettok(FILE *in)
{
        tokp tok = calloc(1, sizeof(tok_t));

        if (!buf0) _loadbuf(in, buf0);
        if (!forward) forward = buf0;
        cur = forward;

        _advance();

        // id
        if (isalnum(*forward)) {
                while (isalnum(forward++));
                *forward = '\0';
                char *idstr = malloc(sizeof(char) * (strlen(cur) + 1));
                strcpy(idstr, cur);

                // skip trailing spaces
                while (*forward == '\t' || *forward == ' ') {
                        forward++;
                }

                tok->type = tok_id;
                tok->data = idstr;
        }

        // level
        else if (*forward == '\n') {
                int *i = malloc(sizeof(int));
                *i = 0;
                while (_advance() == '\t' || *forward == ' ')
                        (*i)++;
                tok->type = tok_level;
                tok->data = i;
        }

        else
                tok->type = *forward;

        return tok;
}

// two-buffer lookahead
static long _loadbuf(FILE *in, char *curbuf)
{
        size_t read = 0;
        // char *buf = (*buf0 == -1 || *buf0 == NULL) ?  buf0 : buf1;

        if (curbuf == NULL) curbuf = malloc(JADEC_BUF_LEN);
        read = fread(curbuf, JADEC_BUF_LEN - 1, 1, in);
        if (!read && feof(in))
                return -1;
        *(curbuf + JADEC_BUF_LEN) = -1;
        return read;
}

void tok_free(tokp tok)
{
        if (!tok) return;
        free(tok->data);
        free(tok);
}
