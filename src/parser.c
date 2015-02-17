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
        if (in == NULL) {
                perror("fopen: ");
                exit(1);
        }

        // init lexer buffer
        buf_init(in);
        while(1) {
                parse(in, stdout);
                if (!tok || tok->type == tok_eof) break;
                // printf("%d\n", tok->type);
        }
}

void parse(FILE *input, FILE *output)
{
        in = input;
        out = stdout;
        if (!tok) tok = gettok();

        if (tok->type == tok_id) {
                // doctype
                if (strcmp(tok->data, "doctype") >= 0)
                        node_doctype();
        }

        else if (tok->type == tok_eof) {
                exit(0);
        }
}

static void node_doctype()
{
        tok_free(tok);
        tok = gettok();
        tokp doctype_type_tok = tok;

        // no type: doctype$
        if (doctype_type_tok->type == tok_lf ||
        doctype_type_tok->type == tok_eof) {
                fprintf(out, "%s\n", doctypestr("html"));
                tok_free(doctype_type_tok);
                return;
        }

        tok = gettok();
        if (tok->type != tok_lf && tok->type != tok_eof) {
                fprintf(out, "<!DOCTYPE %s", (char *)doctype_type_tok->data);
                tok_free(doctype_type_tok);
                do {
                        if (tok->type == tok_id)
                                fprintf(out, " %s", (char *)tok->data);

                        else if (tok->type == tok_lf ||
                                tok->type == tok_eof) {
                                tok_free(tok);
                                break;
                                }

                        else
                                fprintf(out, "%c", (char)tok->type);

                        tok_free(tok);
                } while ((tok = gettok()));
                fprintf(out, ">\n");
                return;
        }

        // got a type
        const char *d = doctypestr(doctype_type_tok->data);
        if (d == NULL) {
                fprintf(out, "<!DOCTYPE %s>\n", doctype_type_tok->data);
                return;
        }
        fprintf(out, "%s\n", d);
        tok_free(doctype_type_tok);
        tok_free(tok);
}

static const char *doctypestr(char *type)
{
        // will a btree be faster?
        if (strcmp(type, "html") == 0)
                return JADEC_DOCTYPE_HTML;
        else if (strcmp(type, "xml") == 0)
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

