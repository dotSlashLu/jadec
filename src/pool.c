#include <stdlib.h>
#include "pool.h"
#include "jadec.h"

poolp pool_create(unsigned int s)
{
        poolp pool = calloc(1, sizeof(pool_t));
        pool->addr = malloc(s);
        if (!pool->addr) return NULL;
        pool->cur = 0;
        pool->size = s;
        return pool;
}

void *pool_alloc(poolp pool, unsigned int s)
{
        if (pool->cur + s >= pool->size) {
                pool->addr = realloc(pool->addr,
                        pool->size + pool->size / 2);
                pool->size += pool->size / 2;
        }

        void *ret = pool->addr + pool->cur;
        pool->cur += s;
        return ret;
}

void pool_rewind(poolp pool, unsigned int pos)
{
        pool->cur = pos;
}

void pool_release(poolp pool)
{
        free(pool->addr);
        free(pool);
}
