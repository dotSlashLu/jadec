#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "scanner.h"

tokp scan(FILE *in)
{
        char peek = ' ';
        tokp tok = calloc(1, sizeof(tok_t));

        // space
        do {
                if (peek == ' ' || peek == '\t') continue;
                else break;
        } while ((peek = fgetc(in)));

        // id
        if (isalpha(peek)) {
                char *id = malloc(sizeof(char) * MAX_ID_LEN);
                if (id == NULL) {
                        perror("malloc");
                        exit(1);
                }

                char *tmp = id;
                while (isalnum(peek)) {
                        *tmp++ = peek;
                        peek = fgetc(in);
                }
                fseek(in, -1, SEEK_CUR);
                *tmp++ = '\0';

                if (strcmp(id, "int") >= 0)
                        tok->type = tok_type_int;
                else if (strcmp(id, "float") >= 0)
                        tok->type = tok_type_float;
                else if (strcmp(id, "double") >= 0)
                        tok->type = tok_type_double;
                else if (strcmp(id, "char") >= 0)
                        tok->type = tok_type_char;
                else
                        tok->type = tok_id;

                tok->data = id;
                return tok;
        }

        // num
        if (isdigit(peek)) {
                char *num = malloc(sizeof(char) * MAX_ID_LEN);
                if (num == NULL) {
                        perror("malloc");
                        exit(1);
                }
                char *tmp = num;
                while (isdigit(peek)) {
                        *tmp++ = peek;
                        peek = fgetc(in);
                }
                fseek(in, -1, SEEK_CUR);
                *tmp++ = '\0';

                long *number = malloc(sizeof(long));
                if (number == NULL) {
                        perror("malloc");
                        exit(1);
                }
                *number = (long)atol(num);
                free(num);

                tok->type = tok_num;
                tok->data = number;
                return tok;
        }

        // other
        tok->type = peek;
        return tok;
}

void tok_free(tokp tok)
{
        if (!tok) return;
        free(tok->data);
        free(tok);
}
