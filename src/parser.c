#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "jadec.h"
#include "lexer.h"
#include "parser.h"
#include "doctype.h"
#include "pool.h"
#include "btree.h"

/* current line */
static int line = 1;
/* current tok */
static tokp tok;
static FILE *_output;
/* current level */
static int _level = 0;
/* parent dom node */
static domnodep _prev_node = NULL;
static domnodep _node = NULL;
static poolp _node_pool;

/* get doctype string from type */
static const char *doctypestr(char *);
/* doctype node */
static void node_doctype();
static void node();
static void delim(tokp);
static inline void parsetok();
static void close_node(domnodep node);
static domnodep new_node(char *type);
static void node_attr_list(bt_nodeptr attr_tree_root, bt_nodeptr **attr_list);
static void node_attr(bt_nodeptr root, bt_nodeptr **_list);
static inline void skip_blanks();
static char *escape_html_entity(const char *src);

void parse(char *in, long fsize, FILE *output)
{
        _output = output;
        lexer_init(in, fsize);
        _node_pool = pool_create(2048);
        tok = gettok();
        while (tok && tok->type != tok_eof) {
                parsetok();
                lexer_pool_rewind(0);
        }
        printf("\n[%d]\t---EOF---\n", __LINE__);

        tok_free(tok);
        pool_release(_node_pool);
        lexer_free();
}

static inline void parsetok()
{
        switch (tok->type) {
                case tok_id:
                case '#':
                case '.':
                /*
                 * jade don't allow attr list to start a div
                 **/
                /* case '(': */
                        /* doctype */
                        if (strcmp(tok->data, "doctype") == 0) {
                                new_node(tok->data);
                                node_doctype();
                        }
                        else
                                node();
                        break;

                /* literal */
                case '|':
                        tok = gettok();
                        char *literal = get_literal_to_lf();
                        free(literal);
                        break;

                case tok_lf:
                        line++;
                        _level = 0;
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
                        printf("[%d]\tUnimplemented token: %c, data: %s\n",
                        __LINE__, tok->type, (char *)tok->data);
                        exit(1);
                        break;
        }
}

/*
static inline void escape_cpy(char *src, char *dest, int srclen, int *destlen)
{
        static int i = 0;
        printf("cpy %s, l %d\n", src, srclen);
        if (i + srclen > *destlen) {
                printf("[%d]\trealloc\n", __LINE__);
                dest = realloc(dest, *destlen + *destlen / 2);
                *destlen += *destlen / 2;
        }
        printf("[%d]\ti: %d\n", __LINE__, i);
        strncpy(dest + i, src, srclen);
        i += srclen;
}
*/

static char *escape_html_entity(const char *src)
{
        printf("[%d]\tescaping %s\n", __LINE__, src);
        int srclen = strlen(src) + 1;
        int len = srclen * 1.3;
        int i = 0;
        int cpylen = 0;
        char *dest = malloc(len);
#define cpy(src) { \
        if (i + cpylen > len) { \
                printf("[%d]\trealloc\n", __LINE__); \
                dest = realloc(dest, len + len / 2); \
                len += len / 2; \
        } \
        strncpy(dest + i, src, cpylen); \
        i += cpylen; \
}
        while (*src) {
                switch (*src) {
                        case '<':
                                cpylen = 4;
                                cpy("&lt;");
                                break;
                        case '>':
                                cpylen = 4;
                                cpy("&gt;");
                                break;
                        default:
                                cpylen = 1;
                                cpy(src);
                                break;
                }
                src += 1;
        }
        return dest;
#undef cpy
}

static void node()
{
        bt_nodeptr root = bt_init();

        /*
        * an array of btree node
        * btree for checking uniqueness
        * list for output and memory deallocation
        */
        bt_nodeptr *attr_list = calloc(1, sizeof(bt_nodeptr) * JADEC_MAX_PROP);
        bt_nodeptr *_attr_list = attr_list;

        char *class = calloc(1, 256), *id = calloc(1, 256);
        char *literal = NULL, *type;

        /* node type */
        switch (tok->type) {
                /* no node type specified, default div */
                case '.':
                case '#':
                case '(':
                        type = malloc(4);
                        strcpy(type, "div");
                        break;

                default:
                        type = malloc(strlen(tok->data) + 1);
                        strcpy(type, tok->data);
                        tok = gettok();
                        break;
        }
        /* node attrs */
        do {
                // printf("[%d]\ttok type: %d\n", __LINE__, tok->type);
                switch (tok->type) {
                        /* class */
                        case '.':
                                /* eat . */
                                tok = gettok();
                                if (strlen(class) > 0) strcat(class, " ");
                                strcat(class, tok->data);
                                if (!bt_find(root, "class"))
                                        *attr_list++ = bt_install(root, "class", class);
                                tok = gettok();
                                break;

                        /* id */
                        case '#':
                                /* eat # */
                                tok = gettok();
                                if (!bt_find(root, "id"))
                                        *attr_list++ = bt_install(root, "id", id);
                                if (id == NULL || strlen(id) == 0)
                                        strcpy(id, tok->data);
                                else
                                        printf("[%d]Syntax error: \
only one id can be assigned.\n", __LINE__);
                                tok = gettok();
                                break;

                        /* attr list */
                        case '(':
                                node_attr_list(root, &attr_list);
                                tok = gettok();
                                break;

                        case tok_start_block: {
                                char *raw_literal = get_literal_to_level(_level, &line);
                                literal = escape_html_entity(raw_literal);
                                free(raw_literal);
                                tok = gettok();
                                printf("[%d]\tblock literal: %s\n", __LINE__, literal);
                                break;
                        }
                }
        }
        while (tok->type != tok_delim &&        /* begin literal */
                tok->type != tok_lf &&
                tok->type != '|' &&             /* begin text node */
                tok->type != tok_eof);          /* eof */

        /* text node */
        if (tok->type == '|' || tok->type == tok_delim) {
                literal = get_literal_to_lf();
                printf("[%d]\t%s\n", __LINE__, literal);
        }

        new_node(type);
        fprintf(_output, "[%d]\t<%s \n", __LINE__, type);
        if (*attr_list) *++attr_list = NULL;
        while (*_attr_list) {
                printf("[%d]\t%s = \"%s\"\n", __LINE__, (*_attr_list)->key, (char *)(*_attr_list)->val);
                _attr_list++;
        }
        printf("[%d]\t>\n", __LINE__);

        // if (literal) printf("[%d]\t%s\n", __LINE__, literal);
        free(literal);

        bt_free(root);
        free(type);
        free(class);
        free(id);
}

static void node_attr_list(bt_nodeptr root, bt_nodeptr **_list)
{
        bt_nodeptr *list = *_list;
        tok = gettok();
        while (tok->type != ')')
                node_attr(root, &list);
}

static void node_attr(bt_nodeptr root, bt_nodeptr **_list)
{
        bt_nodeptr *list = *_list;
        char *attr = malloc(strlen(tok->data) + 1);
        strcpy(attr, tok->data);

        tok = gettok();
        skip_blanks();


        /* =, attr val */
        if (tok->type == '=') {
                tok = gettok();
                skip_blanks();
                if (tok->type == '"' || tok->type == '\'') {
                        char *val = get_quoted_literal(tok->type);
                        bt_nodeptr attrnode;
                        /*
                        * new attr,
                        * install in the btree and record in the list
                        */
                        if (!(attrnode = bt_find(root, attr))) {
                                *list++ = bt_install(root, attr, val);
                                // printf("[%d]\tattr installed to list: (%p)%s -> %s\n", __LINE__, list - 1, (*(list - 1))->key, (char *)(*(list - 1))->val);
                        }
                        /*
                        *  non-id attr exists,
                        *  chain the val to the old one
                        */
                        else if (strcmp(attr, "id") || (strlen(attrnode->val) == 0)) {
                                char *oldval = attrnode->val;
                                if (strlen(oldval) > 0)
                                        strcat(oldval, " ");
                                strcat(oldval, val);
                                bt_install(root, attr, oldval);
                        }
                        /* multiple id, report syntax error */
                        else
                                printf("[%d]\tSyntax error: multiple id\n", __LINE__);
                        tok = gettok();
                        // printf("[%d]\ttok->type: %d data: %s\n", __LINE__, tok->type, (char *)tok->data);
                }

                else if (tok->type == tok_id) {
                        /*
                        * according to jade, only numeric vals
                        * can be used without quote,
                        * other types of vals are just eaten
                        * silently without warning
                        * which I think is not graceful,
                        * but i'll stick with this sementic (for now)
                        */
                        char *data = tok->data;
                        short is_numeric = 1;
                        while (data) {
                                if (!isdigit(*data++)) {
                                        is_numeric = 0;
                                        break;
                                }
                        }
                        if (is_numeric)
                                *(list++) = bt_install(root, attr, tok->data);
                        tok = gettok();
                }
        }

        /* id, another attr */
        else if (tok->type == tok_id) {
                printf("[%d]\ttok_id\n", __LINE__);
                *(list++) = bt_install(root, attr, attr);
        }
}

static inline void skip_blanks()
{
        while (tok->type == tok_delim ||
                tok->type == tok_lf)
                tok = gettok();
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

        /* no type: doctype$ */
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
                if (doctype_type_tok->type == tok_id)
                        fputs((char *)doctype_type_tok->data, _output);
                else
                        fprintf(_output, "%c", doctype_type_tok->type);
                tok_free(doctype_space_tok);
                tok_free(doctype_type_tok);
                do {
                        if (tok->type == tok_id)
                                fprintf(_output, "%s", (char *)tok->data);

                        else if (tok->type == tok_lf ||
                                tok->type == tok_eof) {
                                break;
                                }

                        else if (tok->type == tok_delim) {
                                delim(tok);
                        }

                        else
                                fprintf(_output, "%c", (char)tok->type);

                } while ((tok = gettok()));
                fprintf(_output, ">\n");
                return;
        }

        /* got a type */
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
        if (_prev_node) _prev_node = _node;
        _node = ret;
        domnodep _prev = _prev_node;

        ret->depth = _level;
        /* find parent */
        if (_prev) {
                if (_prev->depth < _level) ret->parent = _prev;
                else if (_prev->depth == _level) ret->parent = _prev->parent;
                else
                        while (_prev && _prev->depth < _level && _prev->parent)
                                _prev = _prev->parent;
        }

        /* test for self closing tags */
#define cmp(str)(!strcmp(str, type))
        if (cmp("doctype") || cmp("br") || cmp("img") ||
                cmp("input") || cmp("area") || cmp("col") ||
                cmp("base") || cmp("link") || cmp("hr") ||
                cmp("embed") || cmp("keygen") || cmp("meta") ||
                cmp("param") || cmp("wrb") || cmp("track") ||
                cmp("source") || cmp("command")
        )
#undef cmp
                ret->closed = -1;
        else
                ret->closed = 0;

        ret->type = pool_alloc(_node_pool, strlen(type) + 1);
        strcpy(ret->type, type);
        *(ret->type + strlen(type) + 1) = '\0';

        /*
         * if new node's level is lte than the prev node
         * close any prev node that isn't closed
         */
        _prev = _prev_node;
        while (_prev && _level <= _prev->depth) {
                close_node(_prev);
                _prev = _prev->parent;
                // if (_prev) printf("[%d]\tprev(%p) depth: %d, current depth: %d, should close: %d\n",
                // __LINE__, _prev, _prev->depth, _level, _level <= _prev->depth);
        }

        // printf("[%d]\tnew node(%p) - type: %s, depth: %d, closed: %d, parent: %p, prev: %p\n",
        // __LINE__, ret, ret->type, ret->depth, ret->closed, ret->parent, _prev_node);
        if (!_prev_node) _prev_node = ret;
        return ret;
}

static void close_node(domnodep node)
{
        switch(node->closed) {
                case 0:
                        fprintf(_output, "[%d]\t</%s>\n", __LINE__, node->type);
                        node->closed = 1;
                        break;

                case -1:
                        if (node->depth == _level) return;
                        else
                                printf("[%d]\tSyntax error in line %d: %s is self closed and should not \
contain any child\n", __LINE__, line, node->type);
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
        if (strcmp(type, "html") == 0)
                return JADEC_DOCTYPE_HTML;
        else if (strcmp(type, "xml") == 0)
                return JADEC_DOCTYPE_XML;
        else if (strcmp(type, "strict") == 0)
                return JADEC_DOCTYPE_STRICT;
        return NULL;
}

