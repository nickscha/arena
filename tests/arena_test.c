/* arena.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) arena/bump allocator.

This Test class defines cases to verify that we don't break the excepted behaviours in the future upon changes.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#define ARENA_COLLECT_STATISTICS
#include "../arena.h"

#include "test.h" /* Simple Testing framework */

#define NUM_ELEMENTS 10

int main(void)
{
  int i;
  int *arr;

  arena arena = {0};
  arena_init(&arena, 1024 * 1024 * 10); /* 1 MB*/

  arr = (int *)arena_malloc(&arena, NUM_ELEMENTS * sizeof(int));
  if (!arr)
  {
    TEST_FUNCTION_PRINTF("%s\n", "Memory allocation failed!");
    return 1;
  }

  for (i = 0; i < NUM_ELEMENTS; ++i)
  {
    arr[i] = i;
  }

  assert(arena_stats_realloc_move_mem == 0);
  assert(arena.offset == 48);
  assert(arena.offset_last == 0);

  arr = arena_realloc(&arena, arr, 13 * sizeof(int));

  assert(arena_stats_realloc_move_mem == 0);
  assert(arena.offset == 64);
  assert(arena.offset_last == 48);

  if (!((float *)arena_malloc(&arena, 3000 * sizeof(float))))
  {
    TEST_FUNCTION_PRINTF("%s\n", "Memory allocation failed!");
    return 1;
  }

  assert(arena.offset == 12064);
  assert(arena.offset_last == 64);

  arr = arena_realloc(&arena, arr, 20 * sizeof(int));

  /*
    We have to move memory since after int *arr another type allocation was done
  */
  assert(arena_stats_realloc_move_mem == 1);

  for (i = 0; i < NUM_ELEMENTS; ++i)
  {
    arr[i] = i;
  }

  assert(arena.offset == 12144);

  arr = arena_realloc(&arena, arr, 40 * sizeof(int));

  /*
    Since we are just extending the memory of the last allocation
    we do not need move memory and just offsetting the arena pointer
  */
  assert(arena_stats_realloc_move_mem == 1);

  assert(arena_stats_init == 1);
  assert(arena_stats_malloc == 3);
  assert(arena_stats_realloc == 3);
  assert(arena_stats_resetf == 0);
  assert(arena_stats_free == 0);
  assert(arena.offset == 12224);
  assert(arena.offset_last == 12144);
  assert(arena.size == 10485760);

  arena_reset(&arena);

  assert(arena_stats_resetf == 1);

  arena_free(&arena);

  assert(arena_stats_free == 1);

  return 0;
}

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
