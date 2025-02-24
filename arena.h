/* arena.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) arena/bump allocator

COMPILE-TIME OPTIONS

  #define ARENA_ALIGNMENT

    This global flag set the memory alignment for the arena allocations for better memory consistency.

    Default: 16

  #define ARENA_COLLECT_STATISTICS

    This global flag needs to be set if some statistics should be gathered
    during invocations of this library which can be accessed by the user
    for some additional information.

    If defined the following statistic counts are available:

    arena_stats_init;
    arena_stats_malloc;
    arena_stats_realloc;
    arena_stats_realloc_move_mem;
    arena_stats_resetf;
    rena_stats_free;

    Example:
    #define ARENA_COLLECT_STATISTICS
    #include "arena.h"


LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef ARENA_H
#define ARENA_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define ARENA_INLINE inline
#elif defined(__GNUC__) || defined(__clang__)
#define ARENA_INLINE __inline__
#elif defined(_MSC_VER)
#define ARENA_INLINE __inline
#else
#define ARENA_INLINE
#endif

#define ARENA_NULL ((void *)0)

#ifndef ARENA_ALIGNMENT
#define ARENA_ALIGNMENT 16 /* Align allocations to 16 bytes */
#endif

#define ARENA_ALIGN(size) (((size) + (ARENA_ALIGNMENT - 1)) & ~(ARENA_ALIGNMENT - 1))

#ifdef ARENA_COLLECT_STATISTICS
#define ARENA_STATS(x) x

unsigned int arena_stats_init;
unsigned int arena_stats_malloc;
unsigned int arena_stats_realloc;
unsigned int arena_stats_realloc_move_mem;
unsigned int arena_stats_resetf;
unsigned int arena_stats_free;

ARENA_INLINE void arena_stats_reset(void)
{
    arena_stats_init = 0;
    arena_stats_malloc = 0;
    arena_stats_realloc = 0;
    arena_stats_realloc_move_mem = 0;
    arena_stats_resetf = 0;
    arena_stats_free = 0;
}

#else
#define ARENA_STATS(x)
#endif

#ifdef _WIN32
/* Windows prototypes since include windows.h is immensily slow !!! */
#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_RELEASE 0x00008000
#define PAGE_READWRITE 0x04
void *VirtualAlloc(void *lpAddress, unsigned long dwSize, unsigned long flAllocationType, unsigned long flProtect);
int VirtualFree(void *lpAddress, unsigned long dwSize, unsigned long dwFreeType);

static ARENA_INLINE void *arena_malloc_win32(unsigned long size)
{
    return VirtualAlloc(ARENA_NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

static ARENA_INLINE void arena_free_win32(void *ptr)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

static void *(*allocator_default)(unsigned long) = arena_malloc_win32;
static void (*deallocator_default)(void *) = arena_free_win32;

#elif defined(__linux__)

#include <sys/mman.h>
#include <unistd.h>

static ARENA_INLINE void *arena_malloc_linux(unsigned long size)
{
    return mmap(ARENA_NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static ARENA_INLINE void arena_free_linux(void *ptr)
{
    if (ptr)
    {
        long page_size = sysconf(_SC_PAGESIZE);
        if (page_size <= 0)
        {
            page_size = 4096; /* Fallback to 4KB if sysconf fails */
        }
        munmap(ptr, ((unsigned long)ptr & ~(page_size - 1)));
    }
}

static void *(*allocator_default)(unsigned long) = arena_malloc_linux;
static void (*deallocator_default)(void *) = arena_free_linux;

#else

/* Fallback to C standard library for unsupported platforms */
#include <stdlib.h>

static ARENA_INLINE void *arena_malloc_stdlib(unsigned long size)
{
    return malloc(size);
}

static ARENA_INLINE void arena_free_stdlib(void *ptr)
{
    free(ptr);
}

/* Declare function pointers for fallback */
static void *(*allocator_default)(unsigned long) = arena_malloc_stdlib;
static void (*deallocator_default)(void *) = arena_fallback_free;

#endif

typedef struct arena
{
    char *base;
    unsigned long offset;
    unsigned long offset_last;
    unsigned long size;
    void *(*allocator)(unsigned long); /* Custom malloc implementation */
    void (*deallocator)(void *);       /* Custom free implementation */
} arena;

static ARENA_INLINE int arena_init(arena *arena, unsigned long size)
{
    ARENA_STATS(++arena_stats_init);

    if (!arena->base)
    {
        if (!arena->allocator)
        {
            arena->allocator = allocator_default;
        }
        if (!arena->deallocator)
        {
            arena->deallocator = deallocator_default;
        }

        arena->base = (char *)arena->allocator(size);

        if (!arena->base)
        {
            return 0;
        }
        arena->size = size;
        arena->offset = 0;
        arena->offset_last = 0;
    }
    return 1;
}

static ARENA_INLINE void *arena_malloc(arena *arena, unsigned long size)
{
    void *_ptr;
    unsigned int _size = ARENA_ALIGN(size);

    if (arena->offset + _size > arena->size)
    {
        return ARENA_NULL; /* Out of memory */
    }

    _ptr = arena->base + arena->offset;

    ARENA_STATS(++arena_stats_malloc);

    arena->offset_last = arena->offset;
    arena->offset += _ptr ? _size : 0;

    return (_ptr);
}

static ARENA_INLINE void *arena_memcpy(void *dest, const void *src, unsigned long n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;
    unsigned long i;
    unsigned long chunk_count;
    unsigned long remaining_bytes;

    if (d == s)
    {
        return dest;
    }

    chunk_count = n / sizeof(unsigned long);
    for (i = 0; i < chunk_count; i++)
    {
        *((unsigned long *)(d + i * sizeof(unsigned long))) = *((const unsigned long *)(s + i * sizeof(unsigned long)));
    }

    remaining_bytes = n % sizeof(unsigned long);
    if (remaining_bytes > 0)
    {
        for (i = 0; i < remaining_bytes; i++)
        {
            d[chunk_count * sizeof(unsigned long) + i] = s[chunk_count * sizeof(unsigned long) + i];
        }
    }

    return dest;
}

static ARENA_INLINE void *arena_realloc(arena *arena, void *ptr, unsigned long new_size)
{
    char *cptr;
    void *new_ptr;

    new_size = ARENA_ALIGN(new_size);

    ARENA_STATS(++arena_stats_realloc);

    if (!ptr)
    {
        return arena_malloc(arena, new_size);
    }

    cptr = (char *)ptr;

    /* Fast path: If it's the last allocation, extend in place */
    if (cptr == arena->base + arena->offset_last)
    {
        unsigned long old_size = arena->offset - arena->offset_last;

        if (arena->offset + (new_size - old_size) <= arena->size)
        {
            arena->offset_last = arena->offset;
            arena->offset += (new_size - old_size);
            return ptr;
        }
    }

    /* Slow path: Allocate new arena memory and copy */
    new_ptr = arena_malloc(arena, new_size);
    if (new_ptr)
    {
        ARENA_STATS(++arena_stats_realloc_move_mem);
        arena_memcpy(new_ptr, cptr, new_size);
    }
    return (new_ptr);
}

static ARENA_INLINE void arena_reset(arena *arena)
{
    ARENA_STATS(++arena_stats_resetf);
    arena->offset = 0;
    arena->offset_last = 0;
}

static ARENA_INLINE void arena_free(arena *arena)
{
    if (arena->base)
    {
        ARENA_STATS(++arena_stats_free);

        arena->deallocator(arena->base);
        arena->base = (char *)ARENA_NULL;
        arena->offset = 0;
        arena->offset_last = 0;
        arena->size = 0;
    }
}

#endif /* ARENA_H */

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
