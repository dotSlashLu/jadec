#include <stdio.h>
#include <string.h>

#include "jadec.h"
#include "lexer.h"
#include "doctype.h"

int line = 1;

void parse(FILE *in, FILE *out)
{
        out = stdout;
        tokp tok = gettok();

        if (tok->type = tok_id) {
                // doctype
                if (strcmp(tok->data, "doctype") >= 0) {
                        tok_free(tok);
                        tok = gettok();
                        if (tok->type != tok_id) {
                                // error
                        }
                        const char doctype = doctypestr(tok->data);
                        tokp doctype_type_tok = tok;
                        tok = gettok();
                        // pre-defined doctypes
                        if (tok->type == tok_lf) {
                                line++;
                                fputs(doctype, stdout);
                                free(doctype_type_tok);
                        }
                }
        }
}

static const char doctypestr[](char *type)
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

