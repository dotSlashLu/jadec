#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "jadec.h"
#include "lexer.h"

static char *buf0 = NULL, *buf1 = NULL;
static long seekcur = 0L, filelen = 0L;

tokp gettok(FILE *in)
{
        char peek = ' ';
        tokp tok = calloc(1, sizeof(tok_t));

}

// two-buffer lookahead
int loadbuf(FILE *in)
{
        size_t read = 0;
        char *buf = (*buf0 == -1 || *buf0 == NULL) ?  buf0 : buf1;

        if (buf == NULL) buf = malloc(JADEC_BUF_LEN);
        read = fread(buf, JADEC_BUF_LEN - 1, 1, in);
        if (!read && feof(in))
                return -1;
        *(buf + JADEC_BUF_LEN) = -1;
        return read;
}
