#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jadec.h"
#include "lexer.h"
#include "doctype.h"

int line = 1;
static tokp tok;
static FILE *in; static FILE *out;
static const char *doctypestr(char *);
static void node_doctype();
void parse(FILE *input, FILE *output);

int main(int argc, char **argv)
{
        in = fopen(*++argv, "r");
        printf("file: %p\n", in);
        if (in == NULL) {
                perror("fopen: ");
                exit(1);
        }

        // init lexer buffer
        buf_init(in);
        while(1) parse(in, stdout);
}

void parse(FILE *input, FILE *output)
{
        in = input;
        out = stdout;
        tok = gettok();

        if (tok->type = tok_id) {
                // doctype
                if (strcmp(tok->data, "doctype") >= 0)
                        node_doctype();
        }
}

static void node_doctype()
{
        char *doctype = malloc(sizeof(char) * 110);

        tok_free(tok);
        tok = gettok();
        printf("got tok: %p, type: %d, data: %s\n", tok, tok->type, tok->data);
        // default doctype html
        if (tok->type == tok_lf) {
                strcpy(doctype, doctypestr("html"));
                fprintf(out, "%s\n", doctype);
                free(doctype);
                tok_free(tok);
        }
        else
                strcpy(doctype, doctypestr(tok->data));

        tokp doctype_type_tok = tok;
        tok = gettok();
        // pre-defined doctypes
        if (tok->type == tok_lf) {
                line++;
                fputs(doctype, stdout);
                free(doctype_type_tok);
                tok_free(tok);
        }
        // custom doctypes
        else {
                fprintf(out, "<!DOCTYPE %s", doctype_type_tok->data);
                tok_free(doctype_type_tok);
                while ((tok = gettok()) && tok->type != tok_lf) {
                        fprintf(out, " %s", tok->data);
                        tok_free(tok);
                }
                fprintf(out, ">");
                tok_free(tok);
        }
}
static const char *doctypestr(char *type)
{
        char *t = type;

        // will a btree be faster?
        if (strcmp(type, "html") >= 0)
                return JADEC_DOCTYPE_HTML;
        else if (strcmp(type, "xml") >= 0)
                return JADEC_DOCTYPE_XML;
        return NULL;
}

/*
static void prop_list()
{
        propp prop = calloc(1, sizeof(prop_t));
        char *name = malloc(MAX_PROP_LEN);
        int i = 0;

        while (is_alnum(_advance())) {
                name[i++] = *forward;
                if (i == MAX_PROP_LEN)
                        ; // TODO: error: prop name too long
        }

        while (isspace(*forward)) _advance();

        if (*forward == '=') {
                char *val = malloc(MAX_PROP_VAL_LEN);

        }
}
*/


/*
// id
if (*forward == '(') {
        domnodep node = calloc(1, sizeof(domnode_t));
        node->depth = depth;
        tok->data = node;
        tok->proplist = malloc(1, sizeof(propp));
        prop_list();
        cur = forward;
        return tok;
}
*/

