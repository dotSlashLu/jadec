#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define U_MAX_BYTES 4

unsigned short u_getc(FILE *stream, char *bytes)
{
    /* mask values for bit pattern of first byte in multi-byte
     UTF-8 sequences:
       192 - 110xxxxx - for U+0080 to U+07FF
       224 - 1110xxxx - for U+0800 to U+FFFF
       240 - 11110xxx - for U+010000 to U+1FFFFF */
    static unsigned short mask[] = {192, 224, 240};

    unsigned short i, j;

    /* initialize buffer */
    memset(bytes, 0, U_MAX_BYTES + 1);

    /* read first byte into buffer */
    bytes[0] = getc(stream);
    if (bytes[0] == EOF) {
        return 0;
    }

    /* check how many more bytes need to be read for
     character */
    i = 0;
    if ((bytes[0] & mask[0]) == mask[0]) i++;
    if ((bytes[0] & mask[1]) == mask[1]) i++;
    if ((bytes[0] & mask[2]) == mask[2]) i++;

    /* read subsequent character bytes */
    j = 0;
    while (j < i) {
        j++;
        bytes[j] = getc(stream);
    }

    /* return number of bytes read into buffer */
    return i + 1;
}
