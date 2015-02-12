#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "jadec.h"
#include "lexer.h"

static char *buf0 = NULL, *buf1 = NULL;
static char *cur = NULL, *forward = NULL;
static int line = 0;
static FILE *in;
static int depth = 0;

static long loadbuf(FILE *in, char *curbuf);

tokp gettok(FILE *in)
{
        tokp tok = calloc(1, sizeof(tok_t));

        if (!buf0) loadbuf(in, buf0);
        if (!forward) forward = buf0;
        cur = forward;

        // sentinel or eof
        if (*forward++ == -1) {
                // end of buf0
                if (forward == buf0 + JADEC_BUF_LEN) {
                        loadbuf(in, buf1);
                        forward = buf1;
                }

                // end of buf1
                else if (forward == buf1 + JADEC_BUF_LEN) {
                        loadbuf(in, buf0);
                        forward = buf0;
                }

                else {
                        // eof, cleanup
                }
        }

        else if (isalnum(*forward)) {
                while (isalnum(*forward++));
                *forward = '\0';
                char *word = cur;

                // skip spaces
                while (*forward == '\t' || *forward == ' ') { forward++; }

                // node(propList)
                if (*forward == '(') {
                        tok->type = tok_node;
                        domnodep node = calloc(1, sizeof(domnode_t));
                        node->depth = depth;
                }
        }
}

// two-buffer lookahead
static long loadbuf(FILE *in, char *curbuf)
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
