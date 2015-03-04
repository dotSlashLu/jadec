#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jadec.h"
#include "lexer.h"
#include "parser.h"
#include "doctype.h"
#include "pool.h"

// current line
static int line = 1;
// current tok
static tokp tok;
static FILE *_output;
// current level
static int _level = 0;
// parent dom node
static domnodep _prev_node = NULL;
static domnodep _node = NULL;
static poolp _node_pool;

// get doctype string from type
static const char *doctypestr(char *);
// doctype node
static void node_doctype();
static void node();
static void delim(tokp);                        // tok_delim: [ \t]
static void parsetok();
static void close_node(domnodep node);
static domnodep new_node(char *type);

void parse(char *in, long fsize, FILE *output)
{
        _output = output;
        lexer_init(in, fsize);
        _node_pool = pool_create(256);
        while(1) {
                parsetok();
                jadec_pool_release(0);
                if (!tok || tok->type == tok_eof) {
                        printf("[%d]\tEOF\n", __LINE__);
                        break;
                }
        }

        tok_free(tok);
        pool_release(_node_pool);
        lexer_free();
}

static void parsetok()
{
        if (!tok) tok = gettok();

        switch (tok->type) {
                case tok_id:
                // case tok_glyph:
                        // doctype
                        if (strcmp(tok->data, "doctype") == 0) {
                                new_node(tok->data);
                                node_doctype();
                        }
                        else {
                                node();
                                tok = gettok();
                        }
                        break;

                case tok_lf:
                        line++;
                        tok = gettok();
                        break;

                case tok_delim:
                        _level = *(int *)tok->data;
                        tok = gettok();

                        break;

                case tok_eof:
                        printf("[%d]\tEOF\n", __LINE__);
                        return;
                        break;

                default:
                        // printf("[%d]Unimplemented tok: %d, data: %s\n",
                        // __LINE__, tok->type, (char *)tok->data);
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
        printf("[%d]\tNew dom node, type %s at level %d\n",
        __LINE__, (char *)tok->data, _level);
        fprintf(_output, "<%s ", (char *)tok->data);
        tok = gettok();
        switch (tok->type) {
                case '.':
                        printf("[%d]\tbgn class\n", __LINE__);
                        break;
                case '#':
                        printf("[%d]\tbgn id\n", __LINE__);
                        break;
        }
        new_node(tok->data);
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
        if (d == NULL)
                fprintf(_output, "<!DOCTYPE %s>\n", (char *)doctype_type_tok->data);
        else
                fprintf(_output, "%s\n", d);
        tok_free(doctype_space_tok);
        tok_free(doctype_type_tok);
}

static domnodep new_node(char *type)
{
        domnodep ret = pool_alloc(_node_pool, sizeof(domnode_t));
        domnodep _prev = _prev_node;

        // find parent
        if (_prev) {
                if (_prev->depth >= _level && _prev->parent)
                        _prev = _prev->parent;
                /*
                if (_prev->depth < _level && _prev->closed == -1)
                        printf("Syntax error in line %d: %s is self closed and should not \
contain any child\n", line, _prev->type);
                */
        }
        ret->parent = _prev;

        ret->depth = _level;

        // test for self closing tags
#define cmp(str)(!strcmp(str, type))
        if (cmp("doctype") || cmp("br") || cmp("img") ||
                cmp("input") || cmp("area") || cmp("col") ||
                cmp("base") || cmp("link") || cmp("hr") ||
                cmp("embed") || cmp("keygen") || cmp("meta") ||
                cmp("param") || cmp("wrb") || cmp("track") ||
                cmp("source") || cmp("command")
        ) {
#undef cmp
                // printf("[%d]\tself closing tag\n", __LINE__);
                ret->closed = -1;
        }
        else
                ret->closed = 0;

        ret->type = pool_alloc(_node_pool, strlen(type) + 1);
        strcpy(ret->type, type);
        *(ret->type + strlen(type) + 1) = '\0';

        // if new node's level is lte than the prev node
        // close any prev node that isn't closed
        _prev = _prev_node;
        while (_prev && _prev->depth <= _level) {
                close_node(_prev);
                _prev = _prev->parent;
        }

        if (_prev_node) _prev_node = _node;
        else _prev_node = ret;
        _node = ret;
        printf("[%d]\tnew node - type: %s, depth: %d, closed: %d, parent: %p\n",
                __LINE__, ret->type, ret->depth, ret->closed, ret->parent);
        return ret;
}

static void close_node(domnodep node)
{
        switch(node->closed) {
                case 0:
                        fprintf(_output, "</%s>", node->type);
                        node->closed = 1;
                        break;

                case -1:
                        if (node->depth == _level) return;
                        else
                                printf("Syntax error in line %d: %s is self closed and should not \
contain any child\n", line, node->type);
                        break;

                default:
                        return;
                        break;
        }
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

