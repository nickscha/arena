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

## "nostdlib" Motivation & Purpose

nostdlib is a lightweight, minimalistic approach to C development that removes dependencies on the standard library. The motivation behind this project is to provide developers with greater control over their code by eliminating unnecessary overhead, reducing binary size, and enabling deployment in resource-constrained environments.

Many modern development environments rely heavily on the standard library, which, while convenient, introduces unnecessary bloat, security risks, and unpredictable dependencies. nostdlib aims to give developers fine-grained control over memory management, execution flow, and system calls by working directly with the underlying platform.

### Benefits

#### Minimal overhead
By removing the standard library, nostdlib significantly reduces runtime overhead, allowing for faster execution and smaller binary sizes.

#### Increased security
Standard libraries often include unnecessary functions that increase the attack surface of an application. nostdlib mitigates security risks by removing unused and potentially vulnerable components.

#### Reduced binary size
Without linking to the standard library, binaries are smaller, making them ideal for embedded systems, bootloaders, and operating systems where storage is limited.

#### Enhanced performance
Direct control over system calls and memory management leads to performance gains by eliminating abstraction layers imposed by standard libraries.

#### Better portability
By relying only on fundamental system interfaces, nostdlib allows for easier porting across different platforms without worrying about standard library availability.
