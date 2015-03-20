/*
 *      Mem pool
 */

#ifndef JADEC_POOL_H
#define JADEC_POOL_H
#include <stdio.h>

typedef struct pool {
        void *addr;
        /* cursor */
        long int cur;
        unsigned int size;
} pool_t, *poolp;

poolp pool_create(unsigned int s);
void *pool_alloc(poolp, unsigned int s);
void pool_rewind(poolp, unsigned int pos);
void pool_release(poolp);

#endif
