/*
 *      Mem pool
 */

#ifndef JADEC_POOL_H
#define JADEC_POOL_H
#include <stdio.h>

typedef struct pool {
        void *addr;
        // cursor
        off_t cur;
        size_t size;
} pool_t, *poolp;

poolp pool_create();
void *pool_alloc(poolp, size_t s);
void pool_rewind(poolp, long pos);
void pool_release(poolp);

#endif
