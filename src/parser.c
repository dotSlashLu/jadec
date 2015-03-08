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
static void node_attr_list(bt_nodeptr attr_tree_root, bt_nodeptr *attr_list);
static void node_attr(bt_nodeptr attr_tree_root, bt_nodeptr *attr_list);
static inline void skip_blanks();

void parse(char *in, long fsize, FILE *output)
{
        _output = output;
        lexer_init(in, fsize);
        _node_pool = pool_create(2048);
        while(1) {
                parsetok();
                jadec_pool_release(0);
                if (!tok || tok->type == tok_eof) {
                        printf("\n---[%d]\tEOF---\n", __LINE__);
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
                        printf("[%d]Unimplemented tok: %d, data: %s\n",
                        __LINE__, tok->type, (char *)tok->data);
                        break;
        }
}

static void node()
{
        // printf("[%d]\tNew dom node, type %s at level %d\n",
        // __LINE__, (char *)tok->data, _level);
        bt_nodeptr root = bt_init();

        // an array of btree node
        // btree for checking uniqueness
        // list for output and memory deallocation
        bt_nodeptr *attr_list = malloc(sizeof(bt_nodeptr) * JADEC_MAX_PROP);
        bt_nodeptr *_attr_list = attr_list;

        char *class = malloc(256), *id = malloc(256);
        bt_install(root, "class", class); bt_install(root, "id", id);
        // node type
        char *type = malloc(strlen(tok->data) + 1);
        strcpy(type, tok->data);

        // attrs
        tok = gettok();
        do {
                // printf("[%d]\ttok type: %d\n", __LINE__, tok->type);
                switch (tok->type) {
                        // class
                        case '.':
                                printf("[%d]\tbgn class\n", __LINE__);
                                // class name
                                tok = gettok();
                                if (strlen(class) > 0) strcat(class, " ");
                                strcat(class, tok->data);
                                if (!bt_find(root, "class"))
                                        *(attr_list++) = bt_install(root, "class", class);
                                break;

                        // id
                        case '#':
                                printf("[%d]\tbgn id\n", __LINE__);
                                tok = gettok();
                                if (id == NULL || strlen(id) == 0)
                                        strcpy(id, tok->data);
                                else
                                        printf("Syntax error: \
only one id can be assigned.\n");
                                printf("[%d]\tid: %s\n", __LINE__, id);
                                break;

                        // attr list
                        case '(':
                                node_attr_list(root, attr_list);
                                while (*_attr_list) {
                                        printf("[%d]\tattr name: %s, val: %s\n", __LINE__, (*_attr_list)->key, (char *)(*_attr_list)->val);
                                        _attr_list++;
                                }
                                break;

                        default:
                                // printf("[%d]\ttok type: %d\n", __LINE__, tok->type);
                                break;
                }
                tok = gettok();
        }
        while (tok->type != tok_delim &&        // begin literal
                tok->type != '|' &&             // begin text node
                tok->type != tok_id &&          // new node
                tok->type != tok_eof);          // eof

        new_node(type);
        fprintf(_output, "[%d]\t<%s \n", __LINE__, type);
        printf("[%d]\tclass = \"%s\"\n", __LINE__, class);
        printf("[%d]\tid = \"%s\"\n", __LINE__, id);
        // printf("[%d]\ttok type: %d data: %s\n", __LINE__, tok->type, (char *)tok->data);

        bt_free(root);
        free(type);
        free(class);
        free(id);
}

static void node_attr_list(bt_nodeptr root, bt_nodeptr *list)
{
        printf("[%d]\tattr list\n", __LINE__);
        tok = gettok();
        while (tok->type != ')') {
                node_attr(root, list);
                printf("[%d]\ttok type: %d, data: %s\n", __LINE__, tok->type, (char *)tok->data);
        }
        *(list++) = NULL;
}

static void node_attr(bt_nodeptr root, bt_nodeptr *list)
{
        printf("[%d]\tnode attr\n", __LINE__);
        char *attr = malloc(strlen(tok->data) + 1);
        strcpy(attr, tok->data);

/*
        switch (tok->type) { // attr name
                case tok_id:
*/
        tok = gettok(); // "=" | new attr
        skip_blanks();

        // id, another attr
        if (tok->type == tok_id) {
                printf("[%d]\ttok_id\n", __LINE__);
                *(list++) = bt_install(root, attr, attr);
        }

        // =, attr val
        else if (tok->type == '=') {
                tok = gettok();
                skip_blanks();
                if (tok->type == '"' || tok->type == '\'') {
                        char *val = get_quoted_literal(tok->type);
                        printf("[%d]\tval: %s\n", __LINE__, val);
                        *(list++) = bt_install(root, attr, val);
                        tok = gettok();
                        printf("[%d]\ttok->type: %d data: %s\n", __LINE__, tok->type, (char *)tok->data);
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
/*
                        break;
                default:
                        printf("[%d]\ttok type: %d data: %s\n", __LINE__, tok->type, (char *)tok->data);
                        break;
        }
*/
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
                if (doctype_type_tok->type == tok_id)
                        fputs((char *)doctype_type_tok->data, _output);
                else
                        fprintf(_output, "%c", doctype_type_tok->type);
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
                if (_prev->depth < _level) ret->parent = _prev;
                else
                        while (_prev && _prev->depth < _level && _prev->parent)
                                _prev = _prev->parent;
        }

        ret->depth = _level;

        // test for self closing tags
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
        if (strcmp(type, "html") == 0)
                return JADEC_DOCTYPE_HTML;
        else if (strcmp(type, "xml") == 0)
                return JADEC_DOCTYPE_XML;
        else if (strcmp(type, "strict") == 0)
                return JADEC_DOCTYPE_STRICT;
        return NULL;
}

