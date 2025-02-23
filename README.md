# arena
A C89 standard compliant, single header, nostdlib (no C Standard Library) arena/bump allocator

For more information please look at the "arena.h" file or take a look at the "examples" or "tests" folder.

## Quick Start

Download or clone arena.h and include it in your project.

```C
#include "arena.h"

int main() {
    int   *arr;
    float *arr2;

    arena arena = {0};
    arena_init(&arena, 1024 * 1024 * 10); /* 1 MB Arena*/

    arr = (int *)arena_malloc(&arena, 10 * sizeof(int));
    if (!arr)
    {
        return 1; /* Arena Memory has not enough space to add this type */
    }

    /* Allocate another array in the arena */    
    arr2 = (float *)arena_malloc(&arena, 1024 * sizeof(float));

    if (!arr2)
    {
        return 1; /* Arena Memory has not enough space to add this type */
    }

    /* Increase size of the first array to 13 */
    arr = arena_realloc(&arena, arr, 13 * sizeof(int));

    arena_free(&arena);

    return 0;
}
```
