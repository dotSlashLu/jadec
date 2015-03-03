#include <stdlib.h>
#include "pool.h"
#include "jadec.h"

poolp pool_create()
{
        poolp pool = calloc(1, sizeof(pool_t));
        pool->addr = malloc(JADEC_POOL_SIZE);
        if (!pool->addr) return NULL;
        pool->cur = 0;
        pool->size = JADEC_POOL_SIZE;
        return pool;
}

void *pool_alloc(poolp pool, size_t s)
{
        if (pool->cur + s >= pool->size) {
                pool = realloc(pool, pool->size + JADEC_POOL_SIZE);
                pool->size += JADEC_POOL_SIZE;
        }

        void *ret = pool->addr + pool->cur;
        pool->cur += s;
        return ret;
}

void pool_rewind(poolp pool, long pos)
{
        pool->cur = pos;
}

void pool_release(poolp pool)
{
        free(pool->addr);
        free(pool);
}
