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
static FILE *_output;
// current level
static int _level = 0;
// parent dom node
static domnodep _parent_node = NULL;

// get doctype string from type
static const char *doctypestr(char *);
// doctype node
static void node_doctype();
static void node();
// delim tok
static void delim(tokp);
static void parsetok();

void parse(char *in, long fsize, FILE *output)
{
        _output = output;
        lexer_init(in, fsize);
        while(1) {
                parsetok();
                jadec_pool_release(0);
                if (!tok || tok->type == tok_eof) break;
        }

        tok_free(tok);
        lexer_free();
}

static void parsetok()
{
        if (!tok) tok = gettok();

        switch (tok->type) {
                case tok_id:
                // case tok_glyph:
                        // doctype
                        if (strcmp(tok->data, "doctype") >= 0)
                                node_doctype();
                        else {
                                node();
                                tok = gettok();
                        }
                        break;

                case tok_lf:
                        line++;
                        printf("[%d]\tnew line: %d\n", __LINE__, line);
                        tok = gettok();
                        break;

                case tok_delim:
                        printf("[%d]\ttok_delim, data: %d\n", __LINE__, *(int *)(tok->data));
                        _level = *(int *)tok->data;
                        tok = gettok();

                        break;

                case tok_eof:
                        return;
                        break;

                default:
                        // printf("[%d]Unimplemented tok: %d, data: %s\n", __LINE__, tok->type, *(char *)tok->data);
                        break;
        }
}


/**
 * *node*      ->  *nodeName*
 *                 *nodeAttrList*
 *                 *nodeCtnt*
 */
static void node()
{
        printf("[%d]\tNew dom node\n", __LINE__);
}

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
                fprintf(_output, "%s\n", doctypestr("html"));
                tok_free(doctype_space_tok);
                tok_free(doctype_type_tok);
                return;
        }

        tok = gettok();
        if (tok->type != tok_lf &&
                tok->type != tok_eof)
        {
                fputs("<!DOCTYPE", _output);
                delim(doctype_space_tok);
                fputs((char *)doctype_type_tok->data, _output);
                tok_free(doctype_space_tok);
                tok_free(doctype_type_tok);
                do {
                        if (tok->type == tok_id) {
                                fprintf(_output, "%s", (char *)tok->data);
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
                                fprintf(_output, "%c", (char)tok->type);

                        // tok_free(tok);
                } while ((tok = gettok()));
                fprintf(_output, ">\n");
                // tok_free(tok);
                return;
        }

        // got a type
        const char *d = doctypestr(doctype_type_tok->data);
        if (d == NULL) {
                fprintf(_output, "<!DOCTYPE %s>\n", (char *)doctype_type_tok->data);
                return;
        }
        fprintf(_output, "%s\n", d);
        tok_free(doctype_space_tok);
        tok_free(doctype_type_tok);
}

domnodep newnode(domnodep prev)
{
        domnodep ret = calloc(1, sizeof(domnode_t));

        // find parent
        if (prev && prev->depth >= _level && prev->parent) prev = prev->parent;
        if (prev->closed == -1)
                printf("Syntax error at line %d: xxx is self closed and should not have child\n", line);
        ret->parent = prev;

        ret->depth = _level;
        // if new node's level is lte than the prev node
        // close any prev node that isn't closed
        // if (parent && parent->depth >= _level)

        return ret;
}

static void delim(tokp tok)
{
        int delimlen = *(int *)tok->data;
        char *delims = malloc(delimlen + 1);
        memset(delims, ' ', delimlen);
        *(delims + delimlen) = '\0';
        fprintf(_output, "%s", delims);
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

