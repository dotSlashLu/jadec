#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "jadec.h"
#include "lexer.h"
#include "parser.h"
#include "config.h"

void parsefile(const char *filename, FILE *output);

int main(int argc, char **argv)
{
        char *filename = *(++argv);
        parsefile(filename, stdout);
        return 0;
}


/**
 *      parse file
 *      params: filename, output stream
 */
void parsefile(const char *filename, FILE *output)
{
        char *filectnt;

        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
                perror("open file");
                exit(1);
        }

        // get file size
        struct stat st;
        fstat(fd, &st);
        off_t fsize = (long)st.st_size;

        size_t size = (size_t)(((size_t)fsize / JADEC_PAGE_SIZE + 1) * JADEC_PAGE_SIZE);

        filectnt = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);

        if (filectnt == MAP_FAILED) {
                perror("mmap");
                exit(1);
        }

        parse(filectnt, fsize, stdout);

        munmap(filectnt, size);
}
