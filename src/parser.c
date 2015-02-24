#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jadec.h"
#include "lexer.h"
#include "parser.h"
#include "doctype.h"

// current line
static int line = 1;
// current tok
static tokp tok;
static FILE *out;
// get doctype string from type
static const char *doctypestr(char *);

// doctype node
static void node_doctype();
// delim tok
static void delim(tokp);

/*
int main(int argc, char **argv)
{
        in = fopen(*++argv, "r");
        out = stdout;
        if (in == NULL) {
                perror("open file");
                exit(1);
        }

        // init lexer buffer
        lexer_init(in);

        while(1) {
                parse(in);
                jadec_pool_release(0);
                if (!tok || tok->type == tok_eof) break;
        }

        tok_free(tok);
        lexer_free();
        fclose(in);

        return 0;
}
*/

void parse(FILE *input)
{
        if (!tok) tok = gettok();

        switch (tok->type) {
                case tok_id:
                        // doctype
                        if (strcmp(tok->data, "doctype") >= 0)
                                node_doctype();
                        else tok->type = tok_eof;
                        break;

                case tok_lf:
                        line++;
                        tok = gettok();
                        break;

                case tok_eof:
                        return;
                        break;

                case tok_delim:
                        tok->type = tok_eof;
                        break;

        }
}


/**
 * *node*      ->  *nodeName*
 *                 *nodeAttrList*
 *                 *nodeCtnt*
static void node()
{

}
 */

static void node_doctype()
{
        tok = gettok();
        tokp doctype_space_tok = calloc(1, sizeof(tok_t));
        doctype_space_tok->type = tok->type;
        doctype_space_tok->data = tok->data;

        tok = gettok();
        tokp doctype_type_tok = calloc(1, sizeof(tok_t));
        doctype_type_tok->type = tok->type;
        doctype_type_tok->data = tok->data;

        // no type: doctype$
        if (doctype_space_tok->type != tok_delim ||
                doctype_type_tok->type == tok_lf ||
                doctype_type_tok->type == tok_eof) {
                fprintf(out, "%s\n", doctypestr("html"));
                tok_free(doctype_space_tok);
                tok_free(doctype_type_tok);
                return;
        }

        tok = gettok();
        if (tok->type != tok_lf &&
                tok->type != tok_eof)
        {
                fputs("<!DOCTYPE", out);
                delim(doctype_space_tok);
                fputs((char *)doctype_type_tok->data, out);
                tok_free(doctype_space_tok);
                tok_free(doctype_type_tok);
                do {
                        if (tok->type == tok_id) {
                                fprintf(out, "%s", (char *)tok->data);
                                // free(tok->data);
                        }

                        else if (tok->type == tok_lf ||
                                tok->type == tok_eof) {
                                break;
                                }

                        else if (tok->type == tok_delim) {
                                delim(tok);
                        }

                        else
                                fprintf(out, "%c", (char)tok->type);

                        // tok_free(tok);
                } while ((tok = gettok()));
                fprintf(out, ">\n");
                // tok_free(tok);
                return;
        }

        // got a type
        const char *d = doctypestr(doctype_type_tok->data);
        if (d == NULL) {
                fprintf(out, "<!DOCTYPE %s>\n", (char *)doctype_type_tok->data);
                return;
        }
        fprintf(out, "%s\n", d);
        tok_free(doctype_space_tok);
        tok_free(doctype_type_tok);
}

static void delim(tokp tok)
{
        int delimlen = *(int *)tok->data;
        char *delims = malloc(delimlen + 1);
        memset(delims, ' ', delimlen);
        *(delims + delimlen) = '\0';
        fprintf(out, "%s", delims);
        free(delims);
}

static const char *doctypestr(char *type)
{
        // will a btree be faster?
        if (strcmp(type, "html") == 0)
                return JADEC_DOCTYPE_HTML;
        else if (strcmp(type, "xml") == 0)
                return JADEC_DOCTYPE_XML;
        else if (strcmp(type, "strict") == 0)
                return JADEC_DOCTYPE_STRICT;
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

